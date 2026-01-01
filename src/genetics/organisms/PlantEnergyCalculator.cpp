/**
 * @file PlantEnergyCalculator.cpp
 * @brief Implementation of plant energy calculations
 *
 * Extracted from Plant.cpp to improve code organization and testability.
 * Contains calculations for environmental factors affecting plant growth.
 */

#include "genetics/organisms/PlantEnergyCalculator.hpp"
#include <cmath>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Environmental Factor Calculations
// ============================================================================

float PlantEnergyCalculator::calculateLightFactor(float timeOfDay, float lightNeed) {
    // time_of_day 0.5 = noon = full light
    // Uses sine curve: sin(0) = 0, sin(π/2) = 1, sin(π) = 0
    float availableLight = std::sin(timeOfDay * 3.14159f);
    
    if (availableLight < lightNeed) {
        // Reduced growth when light is below plant's needs
        // Add small epsilon to prevent division by zero
        return availableLight / (lightNeed + 0.01f);
    }
    
    return 1.0f;
}

float PlantEnergyCalculator::calculateWaterFactor(float humidity, float waterNeed) {
    if (humidity < waterNeed) {
        // Reduced growth when humidity is below plant's needs
        // Add small epsilon to prevent division by zero
        return humidity / (waterNeed + 0.01f);
    }
    
    return 1.0f;
}

float PlantEnergyCalculator::calculateTemperatureFactor(float temperature, 
                                                         bool canSurvive,
                                                         float optimalTemp) {
    if (!canSurvive) {
        // Outside survival range - no growth
        return 0.0f;
    }
    
    // Growth scales based on distance from optimal temperature
    // Maximum reduction to 50% at extreme (but survivable) temperatures
    float tempDiff = std::abs(temperature - optimalTemp);
    return std::max(0.5f, 1.0f - (tempDiff / 30.0f));
}

// ============================================================================
// Combined Energy Calculations
// ============================================================================

float PlantEnergyCalculator::calculateEffectiveGrowth(float baseGrowthRate,
                                                       float lightFactor,
                                                       float waterFactor,
                                                       float tempFactor,
                                                       float growthMultiplier) {
    return baseGrowthRate * lightFactor * waterFactor * tempFactor * growthMultiplier;
}

float PlantEnergyCalculator::calculatePhotosynthesisGrowth(
    const EnvironmentState& environment,
    float lightNeed,
    float waterNeed,
    float growthRate,
    bool canSurviveTemp) {
    
    float lightFactor = calculateLightFactor(environment.time_of_day, lightNeed);
    float waterFactor = calculateWaterFactor(environment.humidity, waterNeed);
    float tempFactor = calculateTemperatureFactor(environment.temperature, canSurviveTemp);
    
    return calculateEffectiveGrowth(growthRate, lightFactor, waterFactor, tempFactor);
}

// ============================================================================
// Energy Cost Calculations
// ============================================================================

float PlantEnergyCalculator::calculateMetabolicCost(float currentSize, 
                                                     float metabolismFactor) {
    // Larger plants have proportionally higher maintenance costs
    return currentSize * metabolismFactor;
}

float PlantEnergyCalculator::calculateGrowthEnergy(float currentSize,
                                                    float targetSize,
                                                    float energyPerSize) {
    float sizeIncrease = targetSize - currentSize;
    if (sizeIncrease <= 0.0f) {
        return 0.0f;
    }
    return sizeIncrease * energyPerSize;
}

float PlantEnergyCalculator::calculateReproductionEnergy(int seedCount,
                                                          float baseCostPerSeed) {
    return static_cast<float>(seedCount) * baseCostPerSeed;
}

// ============================================================================
// Utility Methods
// ============================================================================

float PlantEnergyCalculator::calculateTemperatureStressDamage(bool canSurviveTemp,
                                                               float baseDamage) {
    if (canSurviveTemp) {
        return 0.0f;
    }
    return baseDamage;
}

} // namespace Genetics
} // namespace EcoSim
