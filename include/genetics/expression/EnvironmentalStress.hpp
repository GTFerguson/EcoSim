#pragma once

#include <algorithm>
#include <cmath>

namespace EcoSim {
namespace Genetics {

// Forward declarations
struct EnvironmentState;
class Phenotype;

/**
 * @brief Stress severity levels for categorization
 * 
 * Maps degrees outside tolerance range to severity categories.
 * Each level has distinct effects on energy drain and health damage.
 */
enum class StressLevel {
    Comfortable,  // Within tolerance range - no penalties
    Mild,         // 0-5°C outside (energy drain only, no health damage)
    Moderate,     // 5-15°C outside (drain + slow damage)
    Severe,       // 15-25°C outside (high drain + moderate damage)
    Lethal        // 25°C+ outside (extreme drain + rapid damage)
};

/**
 * @brief Morphology traits that affect thermoregulation
 *
 * These values are extracted from creature genes and modify how
 * organisms respond to temperature extremes. Real-world analogs:
 * - Fur: Arctic fox (high) vs desert fox (low)
 * - Fat: Whale/seal (high) vs lizard (low)
 * - Metabolism: Shrew (high) vs tortoise (low)
 *
 * Key design: Insulation (fur, fat) helps in COLD but HURTS in HEAT.
 * This creates realistic evolutionary tradeoffs - arctic creatures
 * struggle in deserts, desert creatures struggle in cold.
 */
struct ThermalAdaptations {
    float furDensity = 0.0f;           ///< [0-1]: sparse to thick (FUR_DENSITY gene)
    float fatLayerThickness = 0.0f;    ///< [0-1]: lean to heavily insulated (FAT_LAYER_THICKNESS gene)
    float metabolismRate = 1.0f;       ///< [0.1-3.0]: base metabolism (METABOLISM_RATE gene)
    float hideThickness = 0.0f;        ///< [0-1]: skin protection (HIDE_THICKNESS gene)
    float bodySize = 1.0f;             ///< [0.5-20.0]: body mass affects heat retention (MAX_SIZE gene)
    float thermoregulation = 0.5f;     ///< [0-1]: active temp control (optional THERMOREGULATION gene)
    
    /// Create default adaptations (balanced creature)
    static ThermalAdaptations defaults() {
        return ThermalAdaptations{0.5f, 0.2f, 1.0f, 0.3f, 1.0f, 0.5f};
    }
    
    /// Create adaptations for plants (minimal - no fur/fat/active regulation)
    static ThermalAdaptations forPlant() {
        return ThermalAdaptations{0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f};
    }
    
    /// Create arctic-adapted profile (polar bear, arctic fox)
    static ThermalAdaptations arcticProfile() {
        return ThermalAdaptations{0.9f, 0.8f, 1.5f, 0.4f, 2.0f, 0.6f};
    }
    
    /// Create desert-adapted profile (fennec fox, camel)
    static ThermalAdaptations desertProfile() {
        return ThermalAdaptations{0.2f, 0.1f, 0.7f, 0.3f, 1.0f, 0.8f};
    }
};

/**
 * @brief Effective tolerance range after applying adaptation modifiers
 * 
 * Adaptations modify the base tolerance range asymmetrically:
 * - Cold adaptations (fur, fat) extend tolerance to lower temperatures
 * - Heat adaptations can be negative (high fur HURTS heat tolerance)
 */
struct EffectiveToleranceRange {
    float tempMin;      ///< Effective minimum survivable temperature (may be lower than base)
    float tempMax;      ///< Effective maximum survivable temperature (may be lower than base if over-insulated)
    float coldBonus;    ///< Degrees of cold tolerance extension (positive = better cold survival)
    float heatBonus;    ///< Degrees of heat tolerance change (can be NEGATIVE for heavily insulated creatures)
};

/**
 * @brief Temperature stress calculation result
 * 
 * Contains all stress metrics calculated from temperature deviation:
 * - Energy costs increase proportionally to stress
 * - Health damage begins after safety margin (5°C)
 * - Severity classification for behavioral responses
 */
struct TemperatureStress {
    float degreesOutside = 0.0f;       ///< Distance from EFFECTIVE tolerance range
    float rawDegreesOutside = 0.0f;    ///< Distance from BASE tolerance (before adaptations)
    float stressLevel = 0.0f;          ///< Normalized 0.0-1.0+ (0 = none, 1 = lethal threshold)
    float energyDrainMultiplier = 1.0f;///< Multiplier for energy costs (1.0 = normal, 2.0 = double)
    float healthDamageRate = 0.0f;     ///< Fraction of max health damage per tick
    StressLevel severity = StressLevel::Comfortable;
    bool isHeatStress = false;         ///< True if overheating, false if freezing
    
    bool isStressed() const { return severity != StressLevel::Comfortable; }
    bool isColdStress() const { return isStressed() && !isHeatStress; }
};

/**
 * @brief Moisture stress calculation result (primarily for plants)
 * 
 * Plants experience drought stress when water availability is below needs.
 * Water storage gene (succulent adaptation) reduces effective water needs.
 */
struct MoistureStress {
    float deficit = 0.0f;             ///< Moisture shortfall (0 = adequate)
    float stressLevel = 0.0f;         ///< Normalized stress (0.0-2.0 typical)
    float energyDrainMultiplier = 1.0f; ///< Multiplier for energy/growth costs
    float healthDamageRate = 0.0f;    ///< Drought damage per tick (only for severe stress)
    
    bool isStressed() const { return deficit > 0.0f; }
};

/**
 * @brief Combined stress for plants (temperature + moisture)
 */
struct CombinedPlantStress {
    TemperatureStress temperature;
    MoistureStress moisture;
    float combinedGrowthModifier = 1.0f;  ///< Product of both stress modifiers
    float combinedHealthDamage = 0.0f;    ///< Sum of both damage rates
};

/**
 * @brief Environmental stress calculation utility
 *
 * Calculates fitness penalties for organisms outside their tolerance ranges.
 * Uses gene-defined tolerance thresholds modified by morphological adaptations
 * (fur, fat, metabolism) and applies progressive penalties.
 *
 * Key design principle: Adaptations have ASYMMETRIC effects on cold vs heat.
 * - High fur/fat: HELPS in cold (+tolerance), HURTS in heat (-tolerance)
 * - High metabolism: Generates body heat (helps cold, hurts heat)
 * - Thermoregulation: Helps in BOTH directions (sweating/panting/shivering)
 * - Large body size: Better heat retention (helps cold via surface-to-volume ratio)
 *
 * Energy drain formula: 1.0 + 0.05 * degreesOutside (5% per degree)
 * Health damage formula: max(0, degreesOutside - 5.0) * 0.00025 (0.025% per degree beyond margin)
 * 
 * @see plans/environmental-stress-system.md for detailed design rationale
 */
class EnvironmentalStressCalculator {
public:
    //=========================================================================
    // Thermal Adaptation Extraction
    //=========================================================================
    
    /**
     * @brief Extract thermal adaptations from a creature's phenotype
     * @param phenotype The creature's expressed traits
     * @return ThermalAdaptations struct populated from phenotype genes
     *
     * Extracts: FUR_DENSITY, FAT_LAYER_THICKNESS, METABOLISM_RATE, 
     *           HIDE_THICKNESS, MAX_SIZE, and optionally THERMOREGULATION
     */
    static ThermalAdaptations extractThermalAdaptations(const Phenotype& phenotype);
    
    //=========================================================================
    // Tolerance Calculation
    //=========================================================================
    
    /**
     * @brief Calculate effective tolerance range with adaptation modifiers
     * @param baseTolLow Base minimum tolerable temperature (from gene)
     * @param baseTolHigh Base maximum tolerable temperature (from gene)
     * @param adaptations Morphological adaptation traits (from genes)
     * @return EffectiveToleranceRange with modified thresholds
     *
     * Cold adaptation bonus formula (degrees extended):
     *   (fur*12 + fat*10 + metabolism*6 + hide*3) * (2.0 - size*0.4)
     * 
     * Heat adaptation bonus formula (can be NEGATIVE):
     *   -(fur*10 + fat*8 + metabolism*5) * (2.0 - size*0.4) + thermoreg*8
     *
     * Example: Arctic creature (fur=0.9, fat=0.7, metabolism=1.5, size=2.0):
     * - Cold bonus: ~+15°C (can survive much colder)
     * - Heat bonus: ~-12°C (overheats easily in warm weather)
     */
    static EffectiveToleranceRange calculateEffectiveTempRange(
        float baseTolLow,
        float baseTolHigh,
        const ThermalAdaptations& adaptations);
    
    //=========================================================================
    // Temperature Stress
    //=========================================================================
    
    /**
     * @brief Calculate temperature stress with adaptation modifiers (full version)
     * @param currentTemp Current environmental temperature (Celsius)
     * @param baseTolLow Base minimum tolerable temperature (from gene)
     * @param baseTolHigh Base maximum tolerable temperature (from gene)
     * @param adaptations Morphological adaptation traits (from genes)
     * @return TemperatureStress with energy drain and health damage rates
     */
    static TemperatureStress calculateTemperatureStress(
        float currentTemp,
        float baseTolLow,
        float baseTolHigh,
        const ThermalAdaptations& adaptations);
    
    /**
     * @brief Calculate temperature stress (simple version, no adaptations)
     * @param currentTemp Current environmental temperature (Celsius)
     * @param toleranceLow Minimum tolerable temperature (from gene)
     * @param toleranceHigh Maximum tolerable temperature (from gene)
     * @return TemperatureStress with energy drain and health damage rates
     *
     * Uses default ThermalAdaptations. Suitable for plants or simple creatures.
     */
    static TemperatureStress calculateTemperatureStress(
        float currentTemp,
        float toleranceLow,
        float toleranceHigh);
    
    //=========================================================================
    // Moisture Stress (Plants)
    //=========================================================================
    
    /**
     * @brief Calculate moisture stress for plants
     * @param currentMoisture Current tile moisture (0.0-1.0)
     * @param waterRequirement Plant's water need gene value
     * @param waterStorage Plant's water storage gene value (succulent adaptation)
     * @return MoistureStress with growth modifier and damage rate
     *
     * Effective water need = waterRequirement * (1.0 - waterStorage * 0.5)
     * Deficit = max(0, effectiveNeed - currentMoisture)
     */
    static MoistureStress calculateMoistureStress(
        float currentMoisture,
        float waterRequirement,
        float waterStorage);
    
    //=========================================================================
    // Combined Stress
    //=========================================================================
    
    /**
     * @brief Calculate combined temperature + moisture stress for plants
     * @param env Current environment state
     * @param tempLow Temperature tolerance minimum
     * @param tempHigh Temperature tolerance maximum
     * @param waterReq Water requirement gene value
     * @param waterStorage Water storage gene value
     * @return CombinedPlantStress with all stress factors
     */
    static CombinedPlantStress calculatePlantStress(
        const EnvironmentState& env,
        float tempLow, float tempHigh,
        float waterReq, float waterStorage);
    
    //=========================================================================
    // Utility
    //=========================================================================
    
    /**
     * @brief Convert StressLevel enum to human-readable string
     * @param level The stress level to convert
     * @return String representation ("Comfortable", "Mild", etc.)
     */
    static const char* stressLevelToString(StressLevel level);
    
    //=========================================================================
    // Constants - Base Stress Parameters
    //=========================================================================
    
    /// Energy drain: +5% per degree outside tolerance
    static constexpr float ENERGY_DRAIN_PER_DEGREE = 0.05f;
    
    /// Health damage: 0.025% max health per tick per degree beyond safety margin
    static constexpr float HEALTH_DAMAGE_PER_DEGREE = 0.00025f;
    
    /// Safety margin before health damage begins (degrees)
    static constexpr float SAFETY_MARGIN_DEGREES = 5.0f;
    
    /// Threshold for moderate stress (degrees outside)
    static constexpr float MODERATE_THRESHOLD = 5.0f;
    
    /// Threshold for severe stress (degrees outside)
    static constexpr float SEVERE_THRESHOLD = 15.0f;
    
    /// Lethal threshold (degrees outside effective tolerance)
    static constexpr float LETHAL_THRESHOLD = 25.0f;
    
    /// Lethal damage rate (fraction of max health per tick)
    static constexpr float LETHAL_DAMAGE_RATE = 0.05f;
    
    //=========================================================================
    // Constants - Adaptation Bonuses (per unit of gene value)
    //=========================================================================
    
    // Cold adaptation bonuses (degrees of extended tolerance per 1.0 gene value)
    static constexpr float FUR_COLD_BONUS = 12.0f;       ///< Max +12°C from thick fur
    static constexpr float FAT_COLD_BONUS = 10.0f;       ///< Max +10°C from blubber
    static constexpr float METABOLISM_COLD_BONUS = 6.0f; ///< Max +6°C from high metabolism
    static constexpr float HIDE_COLD_BONUS = 3.0f;       ///< Max +3°C from thick hide
    static constexpr float THERMOREG_COLD_BONUS = 0.0f;  ///< Thermoregulation doesn't extend cold tolerance directly
    
    // Heat adaptation penalties (degrees of REDUCED tolerance per 1.0 gene value)
    static constexpr float FUR_HEAT_PENALTY = 10.0f;       ///< Max -10°C penalty from thick fur
    static constexpr float FAT_HEAT_PENALTY = 8.0f;        ///< Max -8°C penalty from fat layer
    static constexpr float METABOLISM_HEAT_PENALTY = 5.0f; ///< High metabolism = more internal heat
    static constexpr float THERMOREG_HEAT_BONUS = 8.0f;    ///< Max +8°C from sweating/panting
    
    /// Maximum/minimum temperature bounds (prevent extreme values)
    static constexpr float MIN_EFFECTIVE_TEMP = -60.0f;
    static constexpr float MAX_EFFECTIVE_TEMP = 80.0f;
};

} // namespace Genetics
} // namespace EcoSim
