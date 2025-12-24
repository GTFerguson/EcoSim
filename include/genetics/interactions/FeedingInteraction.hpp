#pragma once

#include "genetics/expression/Phenotype.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <string>
#include <cmath>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Result of a creature attempting to eat a plant
 * 
 * Contains detailed information about the feeding attempt including
 * success status, nutrition gained, damage taken, and seed dispersal effects.
 */
struct FeedingResult {
    bool success = false;           ///< Did creature successfully eat?
    float nutritionGained = 0.0f;   ///< Calories extracted
    float damageReceived = 0.0f;    ///< From thorns, toxins
    float plantDamage = 0.0f;       ///< Damage dealt to plant
    bool seedsConsumed = false;     ///< Did creature eat seeds?
    bool seedsDestroyed = false;    ///< Were seeds ground up?
    float seedsToDisperse = 0;      ///< Seeds passing through intact
    std::string description;        ///< For logging/UI
};

/**
 * @brief Handles feeding interactions between creatures and plants
 * 
 * This class encapsulates all the logic for creature-plant feeding interactions,
 * implementing the coevolutionary dynamics described in the design document.
 * 
 * The feeding process involves four phases:
 * 1. Detection - Can the creature find/identify the plant as food?
 * 2. Access - Can the creature overcome plant defenses?
 * 3. Extraction - How much nutrition can be extracted?
 * 4. Seed fate - What happens to consumed seeds?
 * 
 * Usage:
 * @code
 *   FeedingInteraction interaction;
 *   FeedingResult result = interaction.attemptToEatPlant(
 *       creaturePhenotype, plant, 0.8f  // 80% hungry
 *   );
 *   if (result.success) {
 *       creature.addEnergy(result.nutritionGained);
 *       creature.takeDamage(result.damageReceived);
 *       plant.takeDamage(result.plantDamage);
 *   }
 * @endcode
 * 
 * @see FeedingResult
 * @see Phenotype
 * @see Plant
 */
class FeedingInteraction {
public:
    /**
     * @brief Default constructor
     */
    FeedingInteraction() = default;
    
    /**
     * @brief Destructor
     */
    ~FeedingInteraction() = default;
    
    // ========================================================================
    // Main interaction method
    // ========================================================================
    
    /**
     * @brief Attempt to have a creature eat a plant
     * @param creaturePhenotype The creature's expressed traits
     * @param plant The target plant
     * @param creatureHunger Hunger level 0-1 (affects desperation/risk tolerance)
     * @return FeedingResult with all interaction outcomes
     * 
     * The interaction follows four phases:
     * 1. Detection phase - uses scent_detection, color_vision vs fruit_appeal
     * 2. Access phase - hide_thickness vs thorn_density, toxin_tolerance vs toxins
     * 3. Extraction phase - plant_digestion_efficiency * nutrient_value
     * 4. Seed fate - seed_destruction_rate vs seed_coat_durability
     */
    FeedingResult attemptToEatPlant(
        const Phenotype& creaturePhenotype,
        const Plant& plant,
        float creatureHunger = 1.0f
    ) const;
    
    // ========================================================================
    // Detection phase
    // ========================================================================
    
    /**
     * @brief Check if creature can detect the plant as food
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @param distance Distance between creature and plant
     * @return true if creature can detect the plant
     */
    bool canDetectPlant(const Phenotype& creature, const Plant& plant, float distance) const;
    
    /**
     * @brief Get the maximum distance at which creature can detect plant
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @return Maximum detection range in tiles
     */
    float getDetectionRange(const Phenotype& creature, const Plant& plant) const;
    
    // ========================================================================
    // Access phase
    // ========================================================================
    
    /**
     * @brief Calculate thorn damage creature would receive
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @return Damage amount (reduced by hide_thickness)
     */
    float calculateThornDamage(const Phenotype& creature, const Plant& plant) const;
    
    /**
     * @brief Calculate toxin damage creature would receive
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @return Damage amount (reduced by toxin_tolerance and toxin_metabolism)
     */
    float calculateToxinDamage(const Phenotype& creature, const Plant& plant) const;
    
    /**
     * @brief Check if creature can overcome all plant defenses
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @return true if creature can access the plant
     */
    bool canOvercomeDefenses(const Phenotype& creature, const Plant& plant) const;
    
    // ========================================================================
    // Digestion phase
    // ========================================================================
    
    /**
     * @brief Calculate total nutrition creature can extract
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @return Nutrition amount including specialist bonus
     */
    float calculateNutritionExtracted(const Phenotype& creature, const Plant& plant) const;
    
    /**
     * @brief Get effective digestion efficiency for this plant type
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @return Efficiency multiplier 0.0-1.0+
     */
    float getDigestionEfficiency(const Phenotype& creature, const Plant& plant) const;
    
    // ========================================================================
    // Seed dispersal
    // ========================================================================
    
    /**
     * @brief Calculate what fraction of seeds survive gut passage
     * @param creature The creature's phenotype
     * @param plant The target plant
     * @return Survival rate 0.0-1.0
     */
    float calculateSeedSurvivalRate(const Phenotype& creature, const Plant& plant) const;
    
    /**
     * @brief Calculate expected dispersal distance based on creature mobility
     * @param creature The creature's phenotype
     * @return Distance in tiles
     */
    float calculateDispersalDistance(const Phenotype& creature) const;
    
    /**
     * @brief Check if creature has caching instinct (squirrel behavior)
     * @param creature The creature's phenotype
     * @return true if creature will cache seeds/nuts
     */
    bool willCacheSeeds(const Phenotype& creature) const;

private:
    // ========================================================================
    // Internal helper methods
    // ========================================================================
    
    /**
     * @brief Safe trait getter with default value
     */
    float getTraitSafe(const Phenotype& phenotype, const char* traitName, float defaultValue = 0.0f) const;
    
    /**
     * @brief Calculate risk-benefit ratio for desperation check
     */
    float calculateRiskBenefitRatio(float expectedNutrition, float expectedDamage) const;
    
    /**
     * @brief Build description string for the feeding result
     */
    std::string buildDescription(const FeedingResult& result, const std::string& plantInfo) const;
    
    // ========================================================================
    // Constants (GENETICS-MIGRATION: Lowered for initial feeding viability)
    // ========================================================================
    
    /// Threshold below which plant is not detected (lowered from 0.3)
    static constexpr float DETECTION_THRESHOLD = 0.1f;
    
    /// Threshold below which plant is not attractive (lowered from 0.2)
    static constexpr float ATTRACTION_THRESHOLD = 0.05f;
    
    /// Minimum nutrition to be worth eating (lowered from 0.5)
    static constexpr float MIN_NUTRITION_THRESHOLD = 0.1f;
    
    /// Damage/benefit ratio threshold for risk assessment (raised from 0.5)
    static constexpr float RISK_THRESHOLD = 1.0f;
    
    /// Base thorn damage multiplier
    static constexpr float THORN_DAMAGE_BASE = 10.0f;
    
    /// Base toxin damage multiplier
    static constexpr float TOXIN_DAMAGE_BASE = 15.0f;
    
    /// Hide thickness protection multiplier
    static constexpr float HIDE_PROTECTION_FACTOR = 2.0f;
    
    /// Seed nutrition value per seed
    static constexpr float SEED_ENERGY_VALUE = 5.0f;
    
    /// Optimal gut transit time for seed scarification (hours)
    static constexpr float OPTIMAL_TRANSIT_LOW = 4.0f;
    static constexpr float OPTIMAL_TRANSIT_HIGH = 12.0f;
    
    /// Caching instinct threshold
    static constexpr float CACHING_THRESHOLD = 0.5f;
};

} // namespace Genetics
} // namespace EcoSim
