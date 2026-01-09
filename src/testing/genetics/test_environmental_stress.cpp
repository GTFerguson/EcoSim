/**
 * @file test_environmental_stress.cpp
 * @brief Tests for the environmental stress calculation system
 * 
 * Tests cover:
 * - Thermal adaptation extraction from phenotype
 * - Effective tolerance range calculations with asymmetric adaptations
 * - Temperature stress levels (Comfortable/Mild/Moderate/Severe/Lethal)
 * - Energy drain formula verification (5% per degree)
 * - Health damage with safety margin (no damage until 5°C outside)
 * - Moisture stress for plants
 * - Edge cases (extreme temperatures, invalid inputs)
 */

#include "test_framework.hpp"
#include "genetics/expression/EnvironmentalStress.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//=============================================================================
// Helper Functions
//=============================================================================

// Print thermal adaptations for debugging
void printAdaptations(const ThermalAdaptations& a, const char* name) {
    std::cout << "    " << name << ": fur=" << a.furDensity 
              << " fat=" << a.fatLayerThickness
              << " metabolism=" << a.metabolismRate
              << " hide=" << a.hideThickness
              << " size=" << a.bodySize
              << " thermoreg=" << a.thermoregulation << std::endl;
}

// Print effective tolerance range for debugging
void printEffectiveRange(const EffectiveToleranceRange& r, float baseLow, float baseHigh) {
    std::cout << "    Base: [" << baseLow << ", " << baseHigh << "]"
              << " -> Effective: [" << r.tempMin << ", " << r.tempMax << "]"
              << " (cold bonus=" << r.coldBonus << ", heat bonus=" << r.heatBonus << ")"
              << std::endl;
}

// Print temperature stress for debugging
void printStress(const TemperatureStress& s, float temp) {
    std::cout << "    Temp " << temp << "°C: "
              << EnvironmentalStressCalculator::stressLevelToString(s.severity)
              << " outside=" << s.degreesOutside
              << " drain=" << s.energyDrainMultiplier
              << " damage=" << s.healthDamageRate
              << (s.isHeatStress ? " (heat)" : (s.isStressed() ? " (cold)" : ""))
              << std::endl;
}

//=============================================================================
// Test: Thermal Adaptation Profiles
//=============================================================================

void test_arctic_creature_adaptations() {
    // Arctic creature: high fur, high fat, high metabolism
    // Should have excellent cold tolerance but poor heat tolerance
    
    ThermalAdaptations arctic = ThermalAdaptations::arcticProfile();
    printAdaptations(arctic, "Arctic");
    
    TEST_ASSERT_GT(arctic.furDensity, 0.7f);
    TEST_ASSERT_GT(arctic.fatLayerThickness, 0.6f);
    TEST_ASSERT_GT(arctic.metabolismRate, 1.2f);
    
    // Calculate effective range for base tolerance 5°C to 30°C
    float baseLow = 5.0f;
    float baseHigh = 30.0f;
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, arctic);
    
    printEffectiveRange(effective, baseLow, baseHigh);
    
    // Arctic creature should have significantly extended cold tolerance
    TEST_ASSERT_GT(effective.coldBonus, 10.0f);  // At least 10°C cold bonus
    TEST_ASSERT_LT(effective.tempMin, baseLow - 10.0f);  // Can survive much colder
    
    // Arctic creature should have REDUCED heat tolerance (negative heat bonus)
    TEST_ASSERT_LT(effective.heatBonus, 0.0f);  // Heat bonus should be negative
    TEST_ASSERT_LT(effective.tempMax, baseHigh);  // Heat tolerance reduced
}

void test_desert_creature_adaptations() {
    // Desert creature: low fur, low fat, low metabolism, high thermoregulation
    // Should have poor cold tolerance but better heat tolerance
    
    ThermalAdaptations desert = ThermalAdaptations::desertProfile();
    printAdaptations(desert, "Desert");
    
    TEST_ASSERT_LT(desert.furDensity, 0.4f);
    TEST_ASSERT_LT(desert.fatLayerThickness, 0.3f);
    TEST_ASSERT_LT(desert.metabolismRate, 1.0f);
    TEST_ASSERT_GT(desert.thermoregulation, 0.6f);
    
    // Calculate effective range for base tolerance 5°C to 30°C
    float baseLow = 5.0f;
    float baseHigh = 30.0f;
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, desert);
    
    printEffectiveRange(effective, baseLow, baseHigh);
    
    // Desert creature has minimal insulation - small cold bonus
    TEST_ASSERT_LT(effective.coldBonus, 5.0f);  // Limited cold adaptation
    
    // Desert creature should have better heat tolerance than arctic
    // (less negative or possibly positive heat bonus due to thermoregulation)
    ThermalAdaptations arctic = ThermalAdaptations::arcticProfile();
    auto arcticEffective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, arctic);
    
    TEST_ASSERT_GT(effective.heatBonus, arcticEffective.heatBonus);  // Better heat adaptation
    TEST_ASSERT_GT(effective.tempMax, arcticEffective.tempMax);  // Higher heat tolerance
}

void test_temperate_creature_adaptations() {
    // Balanced creature: moderate everything
    // Should have decent tolerance in both directions
    
    ThermalAdaptations temperate = ThermalAdaptations::defaults();
    printAdaptations(temperate, "Temperate");
    
    float baseLow = 5.0f;
    float baseHigh = 30.0f;
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, temperate);
    
    printEffectiveRange(effective, baseLow, baseHigh);
    
    // Moderate adaptations - some cold bonus, smaller heat penalty
    TEST_ASSERT_GT(effective.coldBonus, 0.0f);  // Some cold benefit
    
    // Heat bonus between arctic (very negative) and desert (less negative/positive)
    ThermalAdaptations arctic = ThermalAdaptations::arcticProfile();
    ThermalAdaptations desert = ThermalAdaptations::desertProfile();
    auto arcticEffective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, arctic);
    auto desertEffective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, desert);
    
    TEST_ASSERT_GT(effective.heatBonus, arcticEffective.heatBonus);
    TEST_ASSERT_LT(effective.heatBonus, desertEffective.heatBonus);
}

//=============================================================================
// Test: Stress Level Classification
//=============================================================================

void test_stress_level_comfortable() {
    // Temperature within tolerance should be Comfortable
    float baseLow = 5.0f;
    float baseHigh = 30.0f;
    ThermalAdaptations adapt = ThermalAdaptations::defaults();
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        20.0f, baseLow, baseHigh, adapt);
    
    printStress(stress, 20.0f);
    
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Comfortable));
    TEST_ASSERT(!stress.isStressed());
    TEST_ASSERT_NEAR(stress.energyDrainMultiplier, 1.0f, 0.001f);
    TEST_ASSERT_NEAR(stress.healthDamageRate, 0.0f, 0.0001f);
}

void test_stress_level_mild() {
    // 0-5°C outside effective tolerance = Mild stress
    // Only energy drain, no health damage
    float baseLow = 10.0f;
    float baseHigh = 30.0f;
    ThermalAdaptations adapt = ThermalAdaptations::forPlant();  // Minimal adaptations
    
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, adapt);
    
    // Test at 3°C below effective minimum
    float testTemp = effective.tempMin - 3.0f;
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        testTemp, baseLow, baseHigh, adapt);
    
    printStress(stress, testTemp);
    
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Mild));
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isColdStress());
    TEST_ASSERT_GT(stress.energyDrainMultiplier, 1.0f);  // Energy drain
    TEST_ASSERT_NEAR(stress.healthDamageRate, 0.0f, 0.0001f);  // No damage yet
}

void test_stress_level_moderate() {
    // 5-15°C outside = Moderate stress
    // Energy drain + health damage
    float baseLow = 10.0f;
    float baseHigh = 30.0f;
    ThermalAdaptations adapt = ThermalAdaptations::forPlant();
    
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, adapt);
    
    // Test at 10°C above effective maximum (heat stress)
    float testTemp = effective.tempMax + 10.0f;
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        testTemp, baseLow, baseHigh, adapt);
    
    printStress(stress, testTemp);
    
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Moderate));
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isHeatStress);
    TEST_ASSERT_GT(stress.energyDrainMultiplier, 1.0f);
    TEST_ASSERT_GT(stress.healthDamageRate, 0.0f);  // Health damage now active
}

void test_stress_level_severe() {
    // 15-25°C outside = Severe stress
    float baseLow = 10.0f;
    float baseHigh = 30.0f;
    ThermalAdaptations adapt = ThermalAdaptations::forPlant();
    
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, adapt);
    
    // Test at 20°C below effective minimum
    float testTemp = effective.tempMin - 20.0f;
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        testTemp, baseLow, baseHigh, adapt);
    
    printStress(stress, testTemp);
    
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Severe));
    TEST_ASSERT_GT(stress.energyDrainMultiplier, 1.5f);  // Significant drain
    TEST_ASSERT_GT(stress.healthDamageRate, 0.001f);  // Notable damage
}

void test_stress_level_lethal() {
    // 25°C+ outside = Lethal stress
    float baseLow = 10.0f;
    float baseHigh = 30.0f;
    ThermalAdaptations adapt = ThermalAdaptations::forPlant();
    
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, adapt);
    
    // Test at 30°C above effective maximum
    float testTemp = effective.tempMax + 30.0f;
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        testTemp, baseLow, baseHigh, adapt);
    
    printStress(stress, testTemp);
    
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Lethal));
    TEST_ASSERT_GT(stress.energyDrainMultiplier, 2.0f);  // Heavy drain
    TEST_ASSERT_NEAR(stress.healthDamageRate, 0.05f, 0.001f);  // 5% lethal damage rate
}

//=============================================================================
// Test: Energy Drain Formula
//=============================================================================

void test_energy_drain_formula() {
    // Verify 5% per degree energy drain formula
    // energyDrainMultiplier = 1.0 + 0.05 * degreesOutside
    
    float baseLow = 10.0f;
    float baseHigh = 30.0f;
    ThermalAdaptations adapt = ThermalAdaptations::forPlant();
    
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, adapt);
    
    std::cout << "    Testing energy drain formula (5% per degree):" << std::endl;
    
    // Test various degrees outside
    float testPoints[] = {2.0f, 5.0f, 10.0f, 20.0f};
    for (float degrees : testPoints) {
        float testTemp = effective.tempMax + degrees;
        auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
            testTemp, baseLow, baseHigh, adapt);
        
        float expectedDrain = 1.0f + 0.05f * degrees;
        std::cout << "      " << degrees << "°C outside: expected=" 
                  << expectedDrain << " actual=" << stress.energyDrainMultiplier << std::endl;
        
        TEST_ASSERT_NEAR(stress.energyDrainMultiplier, expectedDrain, 0.01f);
    }
}

//=============================================================================
// Test: Health Damage Safety Margin
//=============================================================================

void test_health_damage_safety_margin() {
    // Verify 5°C safety margin before health damage begins
    // No damage for 0-5°C outside, damage starts after 5°C
    
    float baseLow = 10.0f;
    float baseHigh = 30.0f;
    ThermalAdaptations adapt = ThermalAdaptations::forPlant();
    
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        baseLow, baseHigh, adapt);
    
    std::cout << "    Testing 5°C safety margin:" << std::endl;
    
    // At 3°C outside - within safety margin, no damage
    float temp1 = effective.tempMax + 3.0f;
    auto stress1 = EnvironmentalStressCalculator::calculateTemperatureStress(
        temp1, baseLow, baseHigh, adapt);
    
    std::cout << "      3°C outside: damage=" << stress1.healthDamageRate << std::endl;
    TEST_ASSERT_NEAR(stress1.healthDamageRate, 0.0f, 0.0001f);
    
    // At 5°C outside - exactly at margin boundary, still no damage
    float temp2 = effective.tempMax + 5.0f;
    auto stress2 = EnvironmentalStressCalculator::calculateTemperatureStress(
        temp2, baseLow, baseHigh, adapt);
    
    std::cout << "      5°C outside: damage=" << stress2.healthDamageRate << std::endl;
    TEST_ASSERT_NEAR(stress2.healthDamageRate, 0.0f, 0.0001f);
    
    // At 10°C outside - 5°C beyond margin, should have damage
    // Expected: (10 - 5) * 0.00025 = 0.00125
    float temp3 = effective.tempMax + 10.0f;
    auto stress3 = EnvironmentalStressCalculator::calculateTemperatureStress(
        temp3, baseLow, baseHigh, adapt);
    
    float expectedDamage = (10.0f - 5.0f) * 0.00025f;
    std::cout << "      10°C outside: expected=" << expectedDamage 
              << " actual=" << stress3.healthDamageRate << std::endl;
    TEST_ASSERT_NEAR(stress3.healthDamageRate, expectedDamage, 0.0001f);
}

//=============================================================================
// Test: Moisture Stress for Plants
//=============================================================================

void test_moisture_stress_adequate() {
    // Plant with adequate moisture - no stress
    float moisture = 0.6f;
    float waterReq = 0.5f;
    float waterStorage = 0.0f;
    
    auto stress = EnvironmentalStressCalculator::calculateMoistureStress(
        moisture, waterReq, waterStorage);
    
    std::cout << "    Adequate moisture: deficit=" << stress.deficit 
              << " growth=" << stress.energyDrainMultiplier << std::endl;
    
    TEST_ASSERT(!stress.isStressed());
    TEST_ASSERT_NEAR(stress.deficit, 0.0f, 0.001f);
    TEST_ASSERT_NEAR(stress.energyDrainMultiplier, 1.0f, 0.001f);
}

void test_moisture_stress_drought() {
    // Plant in drought conditions
    float moisture = 0.2f;
    float waterReq = 0.7f;
    float waterStorage = 0.0f;
    
    auto stress = EnvironmentalStressCalculator::calculateMoistureStress(
        moisture, waterReq, waterStorage);
    
    std::cout << "    Drought: deficit=" << stress.deficit 
              << " stress=" << stress.stressLevel
              << " growth=" << stress.energyDrainMultiplier << std::endl;
    
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT_GT(stress.deficit, 0.0f);
    TEST_ASSERT_LT(stress.energyDrainMultiplier, 1.0f);  // Growth reduction
}

void test_moisture_stress_succulent_adaptation() {
    // Succulent plant (high water storage) in drought
    // Should have reduced effective water need
    float moisture = 0.2f;
    float waterReq = 0.7f;
    
    // Without storage
    auto stressNoStorage = EnvironmentalStressCalculator::calculateMoistureStress(
        moisture, waterReq, 0.0f);
    
    // With high storage (succulent)
    auto stressWithStorage = EnvironmentalStressCalculator::calculateMoistureStress(
        moisture, waterReq, 0.8f);
    
    std::cout << "    Succulent adaptation:" << std::endl;
    std::cout << "      Without storage: deficit=" << stressNoStorage.deficit << std::endl;
    std::cout << "      With storage: deficit=" << stressWithStorage.deficit << std::endl;
    
    // Succulent should have lower deficit
    TEST_ASSERT_LT(stressWithStorage.deficit, stressNoStorage.deficit);
    TEST_ASSERT_GT(stressWithStorage.energyDrainMultiplier, stressNoStorage.energyDrainMultiplier);
}

//=============================================================================
// Test: Edge Cases
//=============================================================================

void test_edge_case_extreme_cold() {
    // Extreme cold (-50°C)
    ThermalAdaptations arctic = ThermalAdaptations::arcticProfile();
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        -50.0f, 5.0f, 30.0f, arctic);
    
    printStress(stress, -50.0f);
    
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isColdStress());
    // Even arctic creature should be stressed at -50°C
}

void test_edge_case_extreme_heat() {
    // Extreme heat (+60°C)
    ThermalAdaptations desert = ThermalAdaptations::desertProfile();
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        60.0f, 5.0f, 30.0f, desert);
    
    printStress(stress, 60.0f);
    
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isHeatStress);
    // Even desert creature should be severely stressed at 60°C
    TEST_ASSERT_GE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Severe));
}

void test_edge_case_inverted_tolerance_range() {
    // Inverted tolerance range (low > high) should be auto-corrected
    ThermalAdaptations adapt = ThermalAdaptations::defaults();
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        20.0f, 30.0f, 5.0f, adapt);  // Note: 30 > 5 (inverted)
    
    printStress(stress, 20.0f);
    
    // 20°C should be comfortable in a 5-30°C range
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Comfortable));
}

void test_edge_case_zero_adaptations() {
    // Creature with zero adaptations
    ThermalAdaptations zero;
    zero.furDensity = 0.0f;
    zero.fatLayerThickness = 0.0f;
    zero.metabolismRate = 1.0f;  // Neutral metabolism
    zero.hideThickness = 0.0f;
    zero.bodySize = 1.0f;
    zero.thermoregulation = 0.0f;
    
    printAdaptations(zero, "Zero adaptations");
    
    auto effective = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        5.0f, 30.0f, zero);
    
    printEffectiveRange(effective, 5.0f, 30.0f);
    
    // With zero adaptations, effective range should be close to base
    TEST_ASSERT_NEAR(effective.coldBonus, 0.0f, 1.0f);
}

void test_edge_case_nan_temperature() {
    // NaN temperature should return no-stress state
    ThermalAdaptations adapt = ThermalAdaptations::defaults();
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        std::nan(""), 5.0f, 30.0f, adapt);
    
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Comfortable));
    TEST_ASSERT_NEAR(stress.energyDrainMultiplier, 1.0f, 0.001f);
}

//=============================================================================
// Test: Arctic vs Desert in Different Environments
//=============================================================================

void test_arctic_in_desert_environment() {
    // Arctic creature placed in desert environment (40°C)
    // Should be severely stressed due to overheating
    
    ThermalAdaptations arctic = ThermalAdaptations::arcticProfile();
    float baseLow = 5.0f;
    float baseHigh = 30.0f;
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        40.0f, baseLow, baseHigh, arctic);
    
    std::cout << "    Arctic creature at 40°C (desert):" << std::endl;
    printStress(stress, 40.0f);
    
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isHeatStress);
    TEST_ASSERT_GE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Moderate));
}

void test_desert_in_arctic_environment() {
    // Desert creature placed in arctic environment (-20°C)
    // Should be severely stressed due to cold
    
    ThermalAdaptations desert = ThermalAdaptations::desertProfile();
    float baseLow = 5.0f;
    float baseHigh = 30.0f;
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        -20.0f, baseLow, baseHigh, desert);
    
    std::cout << "    Desert creature at -20°C (arctic):" << std::endl;
    printStress(stress, -20.0f);
    
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isColdStress());
    TEST_ASSERT_GE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Severe));
}

void test_arctic_in_arctic_environment() {
    // Arctic creature in arctic environment (-20°C)
    // Should be comfortable or only mildly stressed
    
    ThermalAdaptations arctic = ThermalAdaptations::arcticProfile();
    float baseLow = 5.0f;
    float baseHigh = 30.0f;
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        -20.0f, baseLow, baseHigh, arctic);
    
    std::cout << "    Arctic creature at -20°C (home):" << std::endl;
    printStress(stress, -20.0f);
    
    // Arctic creature should be comfortable or only mildly stressed at -20°C
    TEST_ASSERT_LE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Moderate));
}

//=============================================================================
// Test: Combined Plant Stress
//=============================================================================

void test_combined_plant_stress() {
    // Test combined temperature + moisture stress for plants
    EnvironmentState env;
    env.temperature = 38.0f;  // Hot
    env.moisture = 0.2f;      // Dry
    
    auto stress = EnvironmentalStressCalculator::calculatePlantStress(
        env, 5.0f, 30.0f, 0.6f, 0.1f);
    
    std::cout << "    Combined plant stress (hot + dry):" << std::endl;
    std::cout << "      Temperature: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.temperature.severity)
              << std::endl;
    std::cout << "      Moisture deficit: " << stress.moisture.deficit << std::endl;
    std::cout << "      Combined growth modifier: " << stress.combinedGrowthModifier << std::endl;
    std::cout << "      Combined health damage: " << stress.combinedHealthDamage << std::endl;
    
    // Should have combined effects
    TEST_ASSERT_LT(stress.combinedGrowthModifier, 1.0f);  // Reduced growth
    TEST_ASSERT(stress.temperature.isStressed() || stress.moisture.isStressed());
}

//=============================================================================
// Test: Stress Level String Conversion
//=============================================================================

void test_stress_level_to_string() {
    TEST_ASSERT_EQ(std::string(EnvironmentalStressCalculator::stressLevelToString(StressLevel::Comfortable)), 
                   std::string("Comfortable"));
    TEST_ASSERT_EQ(std::string(EnvironmentalStressCalculator::stressLevelToString(StressLevel::Mild)), 
                   std::string("Mild"));
    TEST_ASSERT_EQ(std::string(EnvironmentalStressCalculator::stressLevelToString(StressLevel::Moderate)), 
                   std::string("Moderate"));
    TEST_ASSERT_EQ(std::string(EnvironmentalStressCalculator::stressLevelToString(StressLevel::Severe)), 
                   std::string("Severe"));
    TEST_ASSERT_EQ(std::string(EnvironmentalStressCalculator::stressLevelToString(StressLevel::Lethal)), 
                   std::string("Lethal"));
}

//=============================================================================
// Test Runner
//=============================================================================

void runEnvironmentalStressTests() {
    BEGIN_TEST_GROUP("Environmental Stress System");
    
    // Thermal adaptation tests
    RUN_TEST(test_arctic_creature_adaptations);
    RUN_TEST(test_desert_creature_adaptations);
    RUN_TEST(test_temperate_creature_adaptations);
    
    // Stress level tests
    RUN_TEST(test_stress_level_comfortable);
    RUN_TEST(test_stress_level_mild);
    RUN_TEST(test_stress_level_moderate);
    RUN_TEST(test_stress_level_severe);
    RUN_TEST(test_stress_level_lethal);
    
    // Formula verification tests
    RUN_TEST(test_energy_drain_formula);
    RUN_TEST(test_health_damage_safety_margin);
    
    // Moisture stress tests
    RUN_TEST(test_moisture_stress_adequate);
    RUN_TEST(test_moisture_stress_drought);
    RUN_TEST(test_moisture_stress_succulent_adaptation);
    
    // Edge case tests
    RUN_TEST(test_edge_case_extreme_cold);
    RUN_TEST(test_edge_case_extreme_heat);
    RUN_TEST(test_edge_case_inverted_tolerance_range);
    RUN_TEST(test_edge_case_zero_adaptations);
    RUN_TEST(test_edge_case_nan_temperature);
    
    // Cross-environment tests
    RUN_TEST(test_arctic_in_desert_environment);
    RUN_TEST(test_desert_in_arctic_environment);
    RUN_TEST(test_arctic_in_arctic_environment);
    
    // Combined stress tests
    RUN_TEST(test_combined_plant_stress);
    
    // Utility tests
    RUN_TEST(test_stress_level_to_string);
    
    END_TEST_GROUP();
}
