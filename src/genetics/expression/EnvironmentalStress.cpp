#include "genetics/expression/EnvironmentalStress.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <algorithm>
#include <cmath>

namespace EcoSim {
namespace Genetics {

//=============================================================================
// Thermal Adaptation Extraction
//=============================================================================

ThermalAdaptations EnvironmentalStressCalculator::extractThermalAdaptations(
    const Phenotype& phenotype)
{
    ThermalAdaptations adaptations;
    
    // Extract morphology genes that affect thermoregulation
    if (phenotype.hasTrait(UniversalGenes::FUR_DENSITY)) {
        adaptations.furDensity = phenotype.getTrait(UniversalGenes::FUR_DENSITY);
    }
    
    if (phenotype.hasTrait(UniversalGenes::FAT_LAYER_THICKNESS)) {
        adaptations.fatLayerThickness = phenotype.getTrait(UniversalGenes::FAT_LAYER_THICKNESS);
    }
    
    if (phenotype.hasTrait(UniversalGenes::METABOLISM_RATE)) {
        adaptations.metabolismRate = phenotype.getTrait(UniversalGenes::METABOLISM_RATE);
    }
    
    if (phenotype.hasTrait(UniversalGenes::HIDE_THICKNESS)) {
        adaptations.hideThickness = phenotype.getTrait(UniversalGenes::HIDE_THICKNESS);
    }
    
    if (phenotype.hasTrait(UniversalGenes::MAX_SIZE)) {
        adaptations.bodySize = phenotype.getTrait(UniversalGenes::MAX_SIZE);
    }
    
    // Thermoregulation gene is optional - use default if not present
    // This gene represents active temperature management (sweating, panting, shivering)
    if (phenotype.hasTrait("thermoregulation")) {
        adaptations.thermoregulation = phenotype.getTrait("thermoregulation");
    } else {
        adaptations.thermoregulation = 0.5f;  // Moderate default
    }
    
    return adaptations;
}

//=============================================================================
// Tolerance Calculation
//=============================================================================

EffectiveToleranceRange EnvironmentalStressCalculator::calculateEffectiveTempRange(
    float baseTolLow,
    float baseTolHigh,
    const ThermalAdaptations& adaptations)
{
    EffectiveToleranceRange result;
    
    // Size scaling factor: smaller creatures lose heat faster, larger retain it
    // Normalized so size=1.0 gives factor=1.6, size=4.0 gives factor=0.4
    float sizeScaling = std::max(0.2f, 2.0f - adaptations.bodySize * 0.4f);
    
    // Cold Adaptation Bonus: fur and fat help, high metabolism generates heat
    // Formula: (fur*12 + fat*10 + metabolism_deviation*6 + hide*3) * sizeScaling
    // Larger creatures have better heat retention, so they get less benefit from insulation
    float coldBonus = 0.0f;
    coldBonus += adaptations.furDensity * FUR_COLD_BONUS;
    coldBonus += adaptations.fatLayerThickness * FAT_COLD_BONUS;
    // Metabolism above 1.0 helps in cold (generates body heat)
    coldBonus += (adaptations.metabolismRate - 1.0f) * METABOLISM_COLD_BONUS;
    coldBonus += adaptations.hideThickness * HIDE_COLD_BONUS;
    coldBonus *= sizeScaling;
    
    // Heat Adaptation Bonus: OPPOSITE of cold adaptations!
    // Fur and fat REDUCE heat tolerance (can't dissipate heat)
    // Low metabolism = less internal heat generation
    // Thermoregulation (sweating/panting) helps dissipate heat
    float heatBonus = 0.0f;
    heatBonus -= adaptations.furDensity * FUR_HEAT_PENALTY;
    heatBonus -= adaptations.fatLayerThickness * FAT_HEAT_PENALTY;
    // High metabolism generates more internal heat = harder to stay cool
    heatBonus -= (adaptations.metabolismRate - 1.0f) * METABOLISM_HEAT_PENALTY;
    heatBonus *= sizeScaling;
    // Thermoregulation helps in heat (not scaled by size - it's active regulation)
    heatBonus += adaptations.thermoregulation * THERMOREG_HEAT_BONUS;
    
    // Store bonuses for debugging/inspection
    result.coldBonus = coldBonus;
    result.heatBonus = heatBonus;
    
    // Apply modifiers to base tolerance
    // Cold bonus is SUBTRACTED from minimum (lower = can survive colder)
    // Heat bonus is ADDED to maximum (higher = can survive hotter, but can be negative!)
    result.tempMin = baseTolLow - coldBonus;
    result.tempMax = baseTolHigh + heatBonus;
    
    // Clamp to reasonable physical limits
    result.tempMin = std::max(MIN_EFFECTIVE_TEMP, result.tempMin);
    result.tempMax = std::min(MAX_EFFECTIVE_TEMP, result.tempMax);
    
    // Ensure minimum tolerance range (at least 5 degrees)
    if (result.tempMax < result.tempMin + 5.0f) {
        float midpoint = (result.tempMin + result.tempMax) / 2.0f;
        result.tempMin = midpoint - 2.5f;
        result.tempMax = midpoint + 2.5f;
    }
    
    return result;
}

//=============================================================================
// Temperature Stress Calculation
//=============================================================================

TemperatureStress EnvironmentalStressCalculator::calculateTemperatureStress(
    float currentTemp,
    float baseTolLow,
    float baseTolHigh,
    const ThermalAdaptations& adaptations)
{
    TemperatureStress result;
    result.energyDrainMultiplier = 1.0f;
    result.healthDamageRate = 0.0f;
    result.severity = StressLevel::Comfortable;
    result.degreesOutside = 0.0f;
    result.rawDegreesOutside = 0.0f;
    result.stressLevel = 0.0f;
    result.isHeatStress = false;
    
    // Handle invalid input
    if (std::isnan(currentTemp) || std::isinf(currentTemp)) {
        return result;  // Return no-stress state for invalid input
    }
    
    // Auto-correct inverted ranges
    if (baseTolLow > baseTolHigh) {
        std::swap(baseTolLow, baseTolHigh);
    }
    
    // Calculate effective tolerance range with adaptations
    auto effective = calculateEffectiveTempRange(baseTolLow, baseTolHigh, adaptations);
    
    // Calculate distance from tolerance ranges
    if (currentTemp < effective.tempMin) {
        // Cold stress
        result.isHeatStress = false;
        result.degreesOutside = effective.tempMin - currentTemp;
        result.rawDegreesOutside = std::max(0.0f, baseTolLow - currentTemp);
    } else if (currentTemp > effective.tempMax) {
        // Heat stress
        result.isHeatStress = true;
        result.degreesOutside = currentTemp - effective.tempMax;
        result.rawDegreesOutside = std::max(0.0f, currentTemp - baseTolHigh);
    } else {
        // Within effective tolerance - no stress
        return result;
    }
    
    // Normalize stress level (25°C outside = 1.0, lethal threshold)
    result.stressLevel = result.degreesOutside / LETHAL_THRESHOLD;
    
    // Energy drain: +5% per degree outside tolerance
    // This is multiplicative with base metabolism
    result.energyDrainMultiplier = 1.0f + (result.degreesOutside * ENERGY_DRAIN_PER_DEGREE);
    
    // Determine severity and health damage based on degrees outside
    if (result.degreesOutside <= MODERATE_THRESHOLD) {
        // Mild stress: 0-5°C outside
        // Energy drain only, no health damage (safety margin)
        result.severity = StressLevel::Mild;
        result.healthDamageRate = 0.0f;
    }
    else if (result.degreesOutside <= SEVERE_THRESHOLD) {
        // Moderate stress: 5-15°C outside
        // Energy drain + slow health damage
        result.severity = StressLevel::Moderate;
        // Damage starts after safety margin
        result.healthDamageRate = (result.degreesOutside - SAFETY_MARGIN_DEGREES) 
                                  * HEALTH_DAMAGE_PER_DEGREE;
    }
    else if (result.degreesOutside <= LETHAL_THRESHOLD) {
        // Severe stress: 15-25°C outside
        // High energy drain + moderate health damage
        result.severity = StressLevel::Severe;
        result.healthDamageRate = (result.degreesOutside - SAFETY_MARGIN_DEGREES) 
                                  * HEALTH_DAMAGE_PER_DEGREE;
    }
    else {
        // Lethal stress: 25°C+ outside
        // Extreme drain + rapid health damage (5% max health per tick)
        result.severity = StressLevel::Lethal;
        result.healthDamageRate = LETHAL_DAMAGE_RATE;
    }
    
    return result;
}

TemperatureStress EnvironmentalStressCalculator::calculateTemperatureStress(
    float currentTemp,
    float toleranceLow,
    float toleranceHigh)
{
    // Use default/plant adaptations for simple version
    return calculateTemperatureStress(currentTemp, toleranceLow, toleranceHigh, 
                                      ThermalAdaptations::forPlant());
}

//=============================================================================
// Moisture Stress Calculation
//=============================================================================

MoistureStress EnvironmentalStressCalculator::calculateMoistureStress(
    float currentMoisture,
    float waterRequirement,
    float waterStorage)
{
    MoistureStress result;
    result.energyDrainMultiplier = 1.0f;
    result.healthDamageRate = 0.0f;
    result.deficit = 0.0f;
    result.stressLevel = 0.0f;
    
    // Clamp inputs to valid ranges (using min/max for C++11/14 compatibility)
    currentMoisture = std::max(0.0f, std::min(1.0f, currentMoisture));
    waterRequirement = std::max(0.0f, std::min(1.0f, waterRequirement));
    waterStorage = std::max(0.0f, std::min(1.0f, waterStorage));
    
    // Effective water need reduced by storage capability
    // Higher storage = better drought tolerance (succulent adaptation)
    // Formula: effectiveNeed = requirement * (1.0 - storage * 0.5)
    // At max storage (1.0), need is halved
    float effectiveNeed = waterRequirement * (1.0f - waterStorage * 0.5f);
    
    // Calculate deficit
    result.deficit = std::max(0.0f, effectiveNeed - currentMoisture);
    
    if (result.deficit <= 0.0f) {
        // Adequate moisture - no stress
        return result;
    }
    
    // Stress increases with deficit
    // Stress 1.0 = deficit equal to half effective need
    result.stressLevel = result.deficit * 2.0f;
    
    // Energy/growth modifier: inverse relationship with stress
    // At stress 1.0: growth = 50%, at stress 2.0: growth = 33%
    result.energyDrainMultiplier = 1.0f / (1.0f + result.stressLevel);
    
    // Health damage only for severe drought (stress > 1.0)
    // This represents wilting and cellular damage
    if (result.stressLevel > 1.0f) {
        result.healthDamageRate = (result.stressLevel - 1.0f) * 0.001f;
    }
    
    return result;
}

//=============================================================================
// Combined Plant Stress
//=============================================================================

CombinedPlantStress EnvironmentalStressCalculator::calculatePlantStress(
    const EnvironmentState& env,
    float tempLow, float tempHigh,
    float waterReq, float waterStorage)
{
    CombinedPlantStress result;
    
    // Plants have minimal thermal adaptations (no fur/fat/active regulation)
    ThermalAdaptations plantAdaptations = ThermalAdaptations::forPlant();
    
    // Calculate temperature stress
    result.temperature = calculateTemperatureStress(
        env.temperature, tempLow, tempHigh, plantAdaptations);
    
    // Calculate moisture stress
    result.moisture = calculateMoistureStress(
        env.moisture, waterReq, waterStorage);
    
    // Combined growth modifier
    // Temperature stress reduces growth inversely (like moisture)
    float tempGrowthMod = 1.0f / (1.0f + result.temperature.stressLevel);
    result.combinedGrowthModifier = tempGrowthMod * result.moisture.energyDrainMultiplier;
    
    // Combined health damage: sum of both sources
    result.combinedHealthDamage = result.temperature.healthDamageRate 
                                + result.moisture.healthDamageRate;
    
    return result;
}

//=============================================================================
// Utility
//=============================================================================

const char* EnvironmentalStressCalculator::stressLevelToString(StressLevel level) {
    switch (level) {
        case StressLevel::Comfortable: return "Comfortable";
        case StressLevel::Mild:        return "Mild";
        case StressLevel::Moderate:    return "Moderate";
        case StressLevel::Severe:      return "Severe";
        case StressLevel::Lethal:      return "Lethal";
        default:                       return "Unknown";
    }
}

} // namespace Genetics
} // namespace EcoSim
