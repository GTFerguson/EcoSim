#pragma once

#include "genetics/expression/EnvironmentState.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * @brief Calculates plant energy-related values
 *
 * Extracts complex energy calculations from Plant.cpp into a dedicated class
 * for improved readability, testability, and separation of concerns.
 *
 * All methods are static to allow usage without instantiation.
 * Gene values are passed as parameters to avoid coupling with Plant class.
 */
class PlantEnergyCalculator {
public:
    // ========================================================================
    // Environmental Factor Calculations
    // ========================================================================

    /**
     * @brief Calculate light factor affecting photosynthesis
     * @param timeOfDay Time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
     * @param lightNeed Plant's light requirement (0.0 = shade tolerant, 1.0 = full sun)
     * @return Light factor (0.0 to 1.0) for growth calculation
     *
     * Uses sine curve to model daylight availability:
     * - Noon (0.5) provides maximum light
     * - Night provides zero light
     * - High lightNeed plants need more daylight to grow at full rate
     */
    static float calculateLightFactor(float timeOfDay, float lightNeed);

    /**
     * @brief Calculate water/humidity factor affecting growth
     * @param humidity Current humidity level (0.0 to 1.0)
     * @param waterNeed Plant's water requirement (0.0 = drought resistant, 1.0 = water loving)
     * @return Water factor (0.0 to 1.0) for growth calculation
     *
     * Plants with high water needs grow slower in dry conditions.
     */
    static float calculateWaterFactor(float humidity, float waterNeed);

    /**
     * @brief Calculate temperature factor affecting growth
     * @param temperature Current temperature in Celsius
     * @param canSurvive Whether the plant can survive at this temperature
     * @param optimalTemp Optimal temperature for growth (default: 22.5°C)
     * @return Temperature factor (0.0 to 1.0) for growth calculation
     *
     * Returns 0.0 if plant cannot survive the temperature.
     * Otherwise scales from 0.5 to 1.0 based on distance from optimal temperature.
     */
    static float calculateTemperatureFactor(float temperature, bool canSurvive, 
                                           float optimalTemp = 22.5f);

    // ========================================================================
    // Combined Energy Calculations
    // ========================================================================

    /**
     * @brief Calculate effective growth rate combining all environmental factors
     * @param baseGrowthRate Base growth rate from genes
     * @param lightFactor Result from calculateLightFactor()
     * @param waterFactor Result from calculateWaterFactor()
     * @param tempFactor Result from calculateTemperatureFactor()
     * @param growthMultiplier Base multiplier for growth speed (default: 0.003)
     * @return Effective growth increment per tick
     *
     * The growth multiplier is tuned so berry bushes mature in ~900 ticks.
     */
    static float calculateEffectiveGrowth(float baseGrowthRate,
                                          float lightFactor,
                                          float waterFactor,
                                          float tempFactor,
                                          float growthMultiplier = 0.003f);

    /**
     * @brief Calculate all environmental factors and return effective growth
     * @param environment Current environment state
     * @param lightNeed Plant's light requirement
     * @param waterNeed Plant's water requirement
     * @param growthRate Base growth rate
     * @param canSurviveTemp Whether plant can survive current temperature
     * @return Effective growth increment per tick
     *
     * Convenience method that combines all factor calculations.
     */
    static float calculatePhotosynthesisGrowth(
        const EnvironmentState& environment,
        float lightNeed,
        float waterNeed,
        float growthRate,
        bool canSurviveTemp);

    // ========================================================================
    // Energy Cost Calculations
    // ========================================================================

    /**
     * @brief Calculate base metabolic cost for a plant
     * @param currentSize Current plant size
     * @param metabolismFactor Base metabolism multiplier (default: 0.001)
     * @return Energy cost per tick for basic metabolism
     *
     * Larger plants have higher maintenance costs.
     */
    static float calculateMetabolicCost(float currentSize, 
                                        float metabolismFactor = 0.001f);

    /**
     * @brief Calculate energy required for growth increment
     * @param currentSize Current plant size
     * @param targetSize Target size after growth
     * @param energyPerSize Energy cost per unit size (default: 1.0)
     * @return Energy required for the growth
     */
    static float calculateGrowthEnergy(float currentSize,
                                       float targetSize,
                                       float energyPerSize = 1.0f);

    /**
     * @brief Calculate energy cost for seed production
     * @param seedCount Number of seeds to produce
     * @param baseCostPerSeed Energy cost per seed (default: 0.1)
     * @return Total energy cost for reproduction
     */
    static float calculateReproductionEnergy(int seedCount,
                                             float baseCostPerSeed = 0.1f);

    // ========================================================================
    // Utility Methods
    // ========================================================================

    /**
     * @brief Calculate damage from temperature stress
     * @param canSurviveTemp Whether plant can survive current temperature
     * @param baseDamage Base damage per tick when stressed (default: 0.01)
     * @return Damage amount (0.0 if temperature is survivable)
     */
    static float calculateTemperatureStressDamage(bool canSurviveTemp,
                                                  float baseDamage = 0.01f);
};

} // namespace Genetics
} // namespace EcoSim
