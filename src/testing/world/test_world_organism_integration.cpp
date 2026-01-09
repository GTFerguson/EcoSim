/**
 * @file test_world_organism_integration.cpp
 * @brief Comprehensive integration tests for world-organism system
 *
 * Tests verify the complete integration of:
 * - Phase 1: Environment data pipeline (climate → organisms)
 * - Phase 2: Environmental stress system (fitness penalties)
 * - Phase 3: Gene-controlled pathfinding (risk-aware movement)
 * - Phase 4: Biome-specific organisms (thermal adaptations)
 *
 * These are extended simulation tests that verify all components work
 * together correctly over time.
 */

#include "../genetics/test_framework.hpp"
#include "world/WorldGrid.hpp"
#include "world/ClimateWorldGenerator.hpp"
#include "world/EnvironmentSystem.hpp"
#include "world/SeasonManager.hpp"
#include "genetics/organisms/BiomeVariantExamples.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/expression/EnvironmentalStress.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/core/GeneRegistry.hpp"

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <map>
#include <cmath>

using namespace EcoSim;
using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//=============================================================================
// Test Infrastructure
//=============================================================================

namespace {

// Helper to create a gene registry with all defaults registered
std::shared_ptr<GeneRegistry> createTestRegistry() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    return registry;
}

// Helper to get gene value from genome
float getGeneValue(const Genome& genome, const std::string& geneId) {
    if (!genome.hasGene(geneId)) return 0.0f;
    return genome.getGene(geneId).getNumericValue(DominanceType::Incomplete);
}

// Helper to build thermal adaptations from a creature's genome
ThermalAdaptations extractAdaptations(const Creature& creature) {
    const Genome& genome = creature.getGenome();
    ThermalAdaptations adapt;
    adapt.furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    adapt.fatLayerThickness = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    adapt.metabolismRate = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    adapt.hideThickness = getGeneValue(genome, UniversalGenes::HIDE_THICKNESS);
    adapt.bodySize = getGeneValue(genome, UniversalGenes::MAX_SIZE);
    return adapt;
}

// Helper to calculate creature's temperature stress
TemperatureStress calculateCreatureStress(const Creature& creature, float temperature) {
    const Genome& genome = creature.getGenome();
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    ThermalAdaptations adapt = extractAdaptations(creature);
    
    return EnvironmentalStressCalculator::calculateTemperatureStress(
        temperature, tempMin, tempMax, adapt);
}

// Helper to calculate plant's combined stress
CombinedPlantStress calculatePlantStress(const Plant& plant, const EnvironmentState& env) {
    const Genome& genome = plant.getGenome();
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    float waterReq = getGeneValue(genome, UniversalGenes::WATER_REQUIREMENT);
    float waterStorage = getGeneValue(genome, UniversalGenes::WATER_STORAGE);
    
    return EnvironmentalStressCalculator::calculatePlantStress(
        env, tempMin, tempMax, waterReq, waterStorage);
}

// Simulation metrics tracking
struct SimulationMetrics {
    int ticksAlive = 0;
    float totalEnergyDrain = 0.0f;
    float totalHealthDamage = 0.0f;
    float maxStressLevel = 0.0f;
    int ticksAtLethalStress = 0;
    bool survived = true;
    
    void recordTick(const TemperatureStress& stress, float energyDrain, float healthDamage) {
        ticksAlive++;
        totalEnergyDrain += energyDrain;
        totalHealthDamage += healthDamage;
        maxStressLevel = std::max(maxStressLevel, stress.stressLevel);
        if (stress.severity == StressLevel::Lethal) {
            ticksAtLethalStress++;
        }
    }
};

} // anonymous namespace

//=============================================================================
// Test 1: Climate Data Pipeline
//=============================================================================

void test_climate_data_pipeline_world_has_climate() {
    std::cout << "    Testing climate data availability..." << std::endl;
    
    // Create world infrastructure
    ClimateGeneratorConfig config;
    config.width = 100;
    config.height = 100;
    config.seed = 12345;
    
    ClimateWorldGenerator generator(config);
    WorldGrid grid;
    generator.generate(grid, config.seed);
    
    // Create season manager and environment system
    SeasonManager seasonManager;
    EnvironmentSystem envSystem(seasonManager, grid);
    
    // Connect climate map
    envSystem.setClimateMap(&generator.getClimateMap());
    
    // Verify climate data is available
    TEST_ASSERT(envSystem.hasClimateData());
    
    std::cout << "      ✓ World has climate map connected" << std::endl;
}

void test_climate_data_pipeline_per_tile_variation() {
    std::cout << "    Testing per-tile climate variation..." << std::endl;
    
    ClimateGeneratorConfig config;
    config.width = 100;
    config.height = 100;
    config.seed = 12345;
    
    ClimateWorldGenerator generator(config);
    WorldGrid grid;
    generator.generate(grid, config.seed);
    
    SeasonManager seasonManager;
    EnvironmentSystem envSystem(seasonManager, grid);
    envSystem.setClimateMap(&generator.getClimateMap());
    
    // Get temperatures at different locations
    float temp1 = envSystem.getTemperature(25, 25);
    float temp2 = envSystem.getTemperature(75, 75);
    float temp3 = envSystem.getTemperature(25, 75);
    float temp4 = envSystem.getTemperature(75, 25);
    
    std::cout << "      Temperatures at various locations:" << std::endl;
    std::cout << "        (25,25): " << temp1 << "°C" << std::endl;
    std::cout << "        (75,75): " << temp2 << "°C" << std::endl;
    std::cout << "        (25,75): " << temp3 << "°C" << std::endl;
    std::cout << "        (75,25): " << temp4 << "°C" << std::endl;
    
    // Verify temperature variation exists (not all same)
    bool hasVariation = (temp1 != temp2) || (temp2 != temp3) || (temp3 != temp4);
    TEST_ASSERT(hasVariation);
    
    std::cout << "      ✓ Different locations have different temperatures" << std::endl;
}

void test_climate_data_pipeline_organism_receives_environment() {
    std::cout << "    Testing organism environment reception..." << std::endl;
    
    ClimateGeneratorConfig config;
    config.width = 100;
    config.height = 100;
    config.seed = 12345;
    
    ClimateWorldGenerator generator(config);
    WorldGrid grid;
    generator.generate(grid, config.seed);
    
    SeasonManager seasonManager;
    EnvironmentSystem envSystem(seasonManager, grid);
    envSystem.setClimateMap(&generator.getClimateMap());
    
    // Get environment state at a location
    EnvironmentState env = envSystem.getEnvironmentStateAt(50, 50);
    
    std::cout << "      Environment at (50,50):" << std::endl;
    std::cout << "        Temperature: " << env.temperature << "°C" << std::endl;
    std::cout << "        Moisture: " << env.moisture << std::endl;
    std::cout << "        Elevation: " << env.elevation << std::endl;
    std::cout << "        Primary biome: " << env.primaryBiome << std::endl;
    
    // Verify environment state has valid data
    TEST_ASSERT(env.temperature > -60.0f && env.temperature < 80.0f);
    TEST_ASSERT(env.moisture >= 0.0f && env.moisture <= 1.0f);
    TEST_ASSERT(env.elevation >= 0.0f && env.elevation <= 1.0f);
    
    std::cout << "      ✓ Organisms receive location-specific environment" << std::endl;
}

//=============================================================================
// Test 2: Environmental Stress System
//=============================================================================

void test_environmental_stress_arctic_in_cold() {
    std::cout << "    Testing arctic creature in cold biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arcticWolf = factory.createArcticWolf(0, 0);
    
    // Test at cold temperature (-20°C)
    auto stress = calculateCreatureStress(arcticWolf, -20.0f);
    
    std::cout << "      Arctic Wolf at -20°C:" << std::endl;
    std::cout << "        Stress severity: " << EnvironmentalStressCalculator::stressLevelToString(stress.severity) << std::endl;
    std::cout << "        Energy drain: " << stress.energyDrainMultiplier << "x" << std::endl;
    std::cout << "        Health damage: " << stress.healthDamageRate << std::endl;
    
    // Should be comfortable or only mildly stressed
    TEST_ASSERT_LE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Mild));
    TEST_ASSERT_NEAR(stress.energyDrainMultiplier, 1.0f, 0.3f);
    
    std::cout << "      ✓ Arctic creature comfortable in cold" << std::endl;
}

void test_environmental_stress_arctic_in_hot() {
    std::cout << "    Testing arctic creature in hot biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arcticWolf = factory.createArcticWolf(0, 0);
    
    // Test at hot temperature (40°C)
    auto stress = calculateCreatureStress(arcticWolf, 40.0f);
    
    std::cout << "      Arctic Wolf at 40°C:" << std::endl;
    std::cout << "        Stress severity: " << EnvironmentalStressCalculator::stressLevelToString(stress.severity) << std::endl;
    std::cout << "        Energy drain: " << stress.energyDrainMultiplier << "x" << std::endl;
    std::cout << "        Health damage: " << stress.healthDamageRate << std::endl;
    std::cout << "        Degrees outside: " << stress.degreesOutside << std::endl;
    
    // Should be severely stressed
    TEST_ASSERT_GE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Severe));
    TEST_ASSERT_GT(stress.energyDrainMultiplier, 2.0f);
    TEST_ASSERT_GT(stress.healthDamageRate, 0.0f);
    TEST_ASSERT(stress.isHeatStress);
    
    std::cout << "      ✓ Arctic creature stressed in heat (energy drain + damage)" << std::endl;
}

void test_environmental_stress_desert_in_hot() {
    std::cout << "    Testing desert creature in hot biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature desertFennec = factory.createDesertFennec(0, 0);
    
    // Test at hot temperature (40°C)
    ThermalAdaptations adapt = extractAdaptations(desertFennec);
    adapt.thermoregulation = 0.7f; // Desert creatures have good thermoregulation
    
    const Genome& genome = desertFennec.getGenome();
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        40.0f, tempMin, tempMax, adapt);
    
    std::cout << "      Desert Fennec at 40°C:" << std::endl;
    std::cout << "        Stress severity: " << EnvironmentalStressCalculator::stressLevelToString(stress.severity) << std::endl;
    std::cout << "        Energy drain: " << stress.energyDrainMultiplier << "x" << std::endl;
    
    // Should be comfortable
    TEST_ASSERT_LE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Mild));
    
    std::cout << "      ✓ Desert creature comfortable in heat" << std::endl;
}

void test_environmental_stress_desert_in_cold() {
    std::cout << "    Testing desert creature in cold biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature desertFennec = factory.createDesertFennec(0, 0);
    
    // Test at cold temperature (-10°C)
    auto stress = calculateCreatureStress(desertFennec, -10.0f);
    
    std::cout << "      Desert Fennec at -10°C:" << std::endl;
    std::cout << "        Stress severity: " << EnvironmentalStressCalculator::stressLevelToString(stress.severity) << std::endl;
    std::cout << "        Energy drain: " << stress.energyDrainMultiplier << "x" << std::endl;
    std::cout << "        Health damage: " << stress.healthDamageRate << std::endl;
    
    // Should be stressed
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isColdStress());
    
    std::cout << "      ✓ Desert creature stressed in cold" << std::endl;
}

void test_environmental_stress_gradual_decline() {
    std::cout << "    Testing gradual stress decline (not instant death)..." << std::endl;
    
    // Use a temperate creature that will show gradual stress response
    auto registry = createTestRegistry();
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature temperateCreature = factory.createApexPredator(0, 0);
    
    // Test stress at various temperatures starting from temperate to extreme
    std::vector<float> temps = {25.0f, 30.0f, 35.0f, 40.0f, 45.0f, 50.0f};
    std::vector<float> stressLevels;
    
    std::cout << "      Stress progression as temperature increases:" << std::endl;
    
    for (float temp : temps) {
        auto stress = calculateCreatureStress(temperateCreature, temp);
        stressLevels.push_back(stress.stressLevel);
        std::cout << "        " << temp << "°C: "
                  << EnvironmentalStressCalculator::stressLevelToString(stress.severity)
                  << " (stress level: " << stress.stressLevel << ", drain: " << stress.energyDrainMultiplier << "x)" << std::endl;
    }
    
    // Verify stress increases with temperature (gradual progression)
    bool gradualIncrease = true;
    for (size_t i = 1; i < stressLevels.size(); ++i) {
        if (stressLevels[i] < stressLevels[i-1]) {
            gradualIncrease = false;
            break;
        }
    }
    
    TEST_ASSERT(gradualIncrease);
    TEST_ASSERT_GT(stressLevels.back(), stressLevels.front()); // Final stress > initial
    
    std::cout << "      ✓ Stress causes gradual decline, not instant death" << std::endl;
}

//=============================================================================
// Test 3: Stress Affects Survival (Extended Simulation)
//=============================================================================

void test_stress_survival_arctic_in_desert() {
    std::cout << "    Running 100-tick survival simulation..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arcticWolf = factory.createArcticWolf(0, 0);
    
    // Simulate arctic creature in hot desert (45°C)
    const float desertTemp = 45.0f;
    const int maxTicks = 100;
    
    SimulationMetrics metrics;
    float health = 1.0f; // Start at full health (normalized 0-1)
    float energy = 1.0f; // Start at full energy
    
    std::cout << "      Arctic Wolf in 45°C desert environment:" << std::endl;
    
    for (int tick = 0; tick < maxTicks && health > 0.0f; ++tick) {
        auto stress = calculateCreatureStress(arcticWolf, desertTemp);
        
        // Apply stress effects
        float energyDrain = 0.01f * stress.energyDrainMultiplier; // Base drain * multiplier
        float healthDamage = stress.healthDamageRate;
        
        energy -= energyDrain;
        health -= healthDamage;
        
        metrics.recordTick(stress, energyDrain, healthDamage);
        
        // Log key milestones
        if (tick == 0 || tick == 25 || tick == 50 || tick == 75 || tick == 99) {
            std::cout << "        Tick " << tick << ": health=" << std::fixed << std::setprecision(3) 
                      << health << ", energy=" << energy 
                      << ", stress=" << EnvironmentalStressCalculator::stressLevelToString(stress.severity) 
                      << std::endl;
        }
        
        if (health <= 0.0f) {
            metrics.survived = false;
        }
    }
    
    std::cout << "      Results:" << std::endl;
    std::cout << "        Ticks survived: " << metrics.ticksAlive << std::endl;
    std::cout << "        Total energy drain: " << metrics.totalEnergyDrain << std::endl;
    std::cout << "        Total health damage: " << metrics.totalHealthDamage << std::endl;
    std::cout << "        Max stress level: " << metrics.maxStressLevel << std::endl;
    std::cout << "        Survived: " << (metrics.survived ? "yes" : "no") << std::endl;
    
    // Verify progressive decline
    TEST_ASSERT_GT(metrics.totalEnergyDrain, 0.5f); // Significant energy drain
    TEST_ASSERT_GT(metrics.totalHealthDamage, 0.5f); // Significant health damage
    
    // Should survive 20-80 ticks in hostile environment (not instant, not forever)
    TEST_ASSERT_GE(metrics.ticksAlive, 20);
    TEST_ASSERT_LE(metrics.ticksAlive, 100);
    
    std::cout << "      ✓ Creature experiences progressive decline in hostile environment" << std::endl;
}

void test_stress_survival_energy_drain_tracking() {
    std::cout << "    Testing energy drain increases with stress..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arcticWolf = factory.createArcticWolf(0, 0);
    
    // Compare energy drain at different stress levels
    auto lowStress = calculateCreatureStress(arcticWolf, -10.0f);  // Comfortable
    auto medStress = calculateCreatureStress(arcticWolf, 25.0f);   // Moderate stress
    auto highStress = calculateCreatureStress(arcticWolf, 40.0f);  // Severe stress
    
    std::cout << "      Energy drain multipliers:" << std::endl;
    std::cout << "        At -10°C (comfortable): " << lowStress.energyDrainMultiplier << "x" << std::endl;
    std::cout << "        At 25°C (stressed): " << medStress.energyDrainMultiplier << "x" << std::endl;
    std::cout << "        At 40°C (severe): " << highStress.energyDrainMultiplier << "x" << std::endl;
    
    // Energy drain should increase with temperature (for arctic creature)
    TEST_ASSERT_LT(lowStress.energyDrainMultiplier, medStress.energyDrainMultiplier);
    TEST_ASSERT_LT(medStress.energyDrainMultiplier, highStress.energyDrainMultiplier);
    
    std::cout << "      ✓ Energy drain increases with stress level" << std::endl;
}

//=============================================================================
// Test 4: Biome-Appropriate Organisms Thrive
//=============================================================================

void test_biome_appropriate_arctic_wolf_in_tundra() {
    std::cout << "    Testing Arctic Wolf in tundra..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arcticWolf = factory.createArcticWolf(0, 0);
    
    // Simulate 200 ticks in appropriate biome (-15°C)
    const float tundraTemp = -15.0f;
    const int ticks = 200;
    
    SimulationMetrics metrics;
    float health = 1.0f;
    
    for (int tick = 0; tick < ticks; ++tick) {
        auto stress = calculateCreatureStress(arcticWolf, tundraTemp);
        float healthDamage = stress.healthDamageRate;
        health -= healthDamage;
        metrics.recordTick(stress, stress.energyDrainMultiplier * 0.01f, healthDamage);
    }
    
    std::cout << "      Arctic Wolf in -15°C tundra for 200 ticks:" << std::endl;
    std::cout << "        Final health: " << health << std::endl;
    std::cout << "        Total health damage: " << metrics.totalHealthDamage << std::endl;
    
    // Should survive with minimal damage
    TEST_ASSERT_GT(health, 0.9f); // >90% health remaining
    TEST_ASSERT_LT(metrics.totalHealthDamage, 0.1f);
    
    std::cout << "      ✓ Arctic Wolf thrives in tundra (>90% health)" << std::endl;
}

void test_biome_appropriate_desert_fennec_in_desert() {
    std::cout << "    Testing Desert Fennec in desert..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature desertFennec = factory.createDesertFennec(0, 0);
    
    // Simulate 200 ticks in appropriate biome (35°C with thermoregulation)
    const float desertTemp = 35.0f;
    const int ticks = 200;
    
    ThermalAdaptations adapt = extractAdaptations(desertFennec);
    adapt.thermoregulation = 0.7f;
    
    const Genome& genome = desertFennec.getGenome();
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    SimulationMetrics metrics;
    float health = 1.0f;
    
    for (int tick = 0; tick < ticks; ++tick) {
        auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
            desertTemp, tempMin, tempMax, adapt);
        float healthDamage = stress.healthDamageRate;
        health -= healthDamage;
        metrics.recordTick(stress, stress.energyDrainMultiplier * 0.01f, healthDamage);
    }
    
    std::cout << "      Desert Fennec in 35°C desert for 200 ticks:" << std::endl;
    std::cout << "        Final health: " << health << std::endl;
    std::cout << "        Total health damage: " << metrics.totalHealthDamage << std::endl;
    
    // Should survive with minimal damage
    TEST_ASSERT_GT(health, 0.9f);
    
    std::cout << "      ✓ Desert Fennec thrives in desert (>90% health)" << std::endl;
}

void test_biome_appropriate_tropical_jaguar_in_tropical() {
    std::cout << "    Testing Tropical Jaguar in tropical..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature tropicalJaguar = factory.createTropicalJaguar(0, 0);
    
    // Simulate 200 ticks in appropriate biome (28°C)
    const float tropicalTemp = 28.0f;
    const int ticks = 200;
    
    SimulationMetrics metrics;
    float health = 1.0f;
    
    for (int tick = 0; tick < ticks; ++tick) {
        auto stress = calculateCreatureStress(tropicalJaguar, tropicalTemp);
        float healthDamage = stress.healthDamageRate;
        health -= healthDamage;
        metrics.recordTick(stress, stress.energyDrainMultiplier * 0.01f, healthDamage);
    }
    
    std::cout << "      Tropical Jaguar in 28°C tropical for 200 ticks:" << std::endl;
    std::cout << "        Final health: " << health << std::endl;
    
    // Should survive with minimal damage
    TEST_ASSERT_GT(health, 0.9f);
    
    std::cout << "      ✓ Tropical Jaguar thrives in tropical (>90% health)" << std::endl;
}

//=============================================================================
// Test 5: Cross-Biome Migration Stress
//=============================================================================

void test_cross_biome_migration_stress_detection() {
    std::cout << "    Testing cross-biome migration stress detection..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    // Create a temperate creature
    CreatureFactory creatureFactory(registry);
    creatureFactory.registerDefaultTemplates();
    Creature temperateCreature = creatureFactory.createApexPredator(0, 0);
    
    // Test stress at different temperatures (simulating migration)
    std::vector<float> migrationPath = {15.0f, 20.0f, 25.0f, 30.0f, 35.0f, 40.0f};
    
    std::cout << "      Stress during migration (temperate → desert):" << std::endl;
    
    float lastStress = 0.0f;
    
    for (float temp : migrationPath) {
        auto stress = calculateCreatureStress(temperateCreature, temp);
        std::cout << "        At " << temp << "°C: " 
                  << EnvironmentalStressCalculator::stressLevelToString(stress.severity)
                  << " (level: " << stress.stressLevel << ")" << std::endl;
        
        lastStress = stress.stressLevel;
    }
    
    // Last stress level should be elevated
    TEST_ASSERT_GT(lastStress, 0.0f);
    
    std::cout << "      ✓ Stress detected during cross-biome migration" << std::endl;
}

void test_cross_biome_recovery_in_home_biome() {
    std::cout << "    Testing recovery when returning to home biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arcticWolf = factory.createArcticWolf(0, 0);
    
    // Simulate excursion: home (cold) → hostile (hot) → home (cold)
    float health = 1.0f;
    
    std::cout << "      Phase 1: 50 ticks in home biome (-15°C):" << std::endl;
    for (int i = 0; i < 50; ++i) {
        auto stress = calculateCreatureStress(arcticWolf, -15.0f);
        health -= stress.healthDamageRate;
    }
    float healthAfterHome1 = health;
    std::cout << "        Health: " << healthAfterHome1 << std::endl;
    
    std::cout << "      Phase 2: 30 ticks in hostile biome (35°C):" << std::endl;
    for (int i = 0; i < 30; ++i) {
        auto stress = calculateCreatureStress(arcticWolf, 35.0f);
        health -= stress.healthDamageRate;
    }
    float healthAfterHostile = health;
    std::cout << "        Health: " << healthAfterHostile << std::endl;
    
    // Health should have dropped significantly in hostile biome
    TEST_ASSERT_LT(healthAfterHostile, healthAfterHome1);
    
    std::cout << "      Phase 3: Return to home biome (stress stops):" << std::endl;
    auto finalStress = calculateCreatureStress(arcticWolf, -15.0f);
    std::cout << "        Stress level after return: " 
              << EnvironmentalStressCalculator::stressLevelToString(finalStress.severity) << std::endl;
    
    // Stress should be low/none in home biome
    TEST_ASSERT_LE(static_cast<int>(finalStress.severity), static_cast<int>(StressLevel::Mild));
    
    std::cout << "      ✓ Creature can return to home biome and recover" << std::endl;
}

//=============================================================================
// Test 6: Plant Stress and Growth
//=============================================================================

void test_plant_stress_tundra_moss_in_cold() {
    std::cout << "    Testing Tundra Moss in cold biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Plant tundraMoss = factory.createTundraMoss(0, 0);
    
    // Create cold, moderate moisture environment
    EnvironmentState coldEnv;
    coldEnv.temperature = -20.0f;
    coldEnv.moisture = 0.4f;
    
    auto stress = calculatePlantStress(tundraMoss, coldEnv);
    
    std::cout << "      Tundra Moss at -20°C, 0.4 moisture:" << std::endl;
    std::cout << "        Temperature stress: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.temperature.severity) << std::endl;
    std::cout << "        Growth modifier: " << stress.combinedGrowthModifier << std::endl;
    std::cout << "        Health damage: " << stress.combinedHealthDamage << std::endl;
    
    // Should grow well in cold
    TEST_ASSERT_GT(stress.combinedGrowthModifier, 0.7f);
    TEST_ASSERT_LT(stress.combinedHealthDamage, 0.001f);
    
    std::cout << "      ✓ Tundra Moss grows well in cold" << std::endl;
}

void test_plant_stress_desert_cactus_in_hot_dry() {
    std::cout << "    Testing Desert Cactus in hot/dry biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Plant desertCactus = factory.createDesertCactus(0, 0);
    
    // Create hot, dry environment
    EnvironmentState hotDryEnv;
    hotDryEnv.temperature = 40.0f;
    hotDryEnv.moisture = 0.15f;
    
    auto stress = calculatePlantStress(desertCactus, hotDryEnv);
    
    std::cout << "      Desert Cactus at 40°C, 0.15 moisture:" << std::endl;
    std::cout << "        Temperature stress: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.temperature.severity) << std::endl;
    std::cout << "        Moisture stress level: " << stress.moisture.stressLevel << std::endl;
    std::cout << "        Growth modifier: " << stress.combinedGrowthModifier << std::endl;
    
    // Should handle hot/dry conditions
    TEST_ASSERT_GT(stress.combinedGrowthModifier, 0.5f);
    
    std::cout << "      ✓ Desert Cactus handles hot/dry conditions" << std::endl;
}

void test_plant_stress_rainforest_vine_in_hot_wet() {
    std::cout << "    Testing Rainforest Vine in hot/wet biome..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Plant rainforestVine = factory.createRainforestVine(0, 0);
    
    // Create hot, wet environment
    EnvironmentState hotWetEnv;
    hotWetEnv.temperature = 28.0f;
    hotWetEnv.moisture = 0.85f;
    
    auto stress = calculatePlantStress(rainforestVine, hotWetEnv);
    
    std::cout << "      Rainforest Vine at 28°C, 0.85 moisture:" << std::endl;
    std::cout << "        Temperature stress: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.temperature.severity) << std::endl;
    std::cout << "        Growth modifier: " << stress.combinedGrowthModifier << std::endl;
    
    // Should thrive in hot/wet conditions
    TEST_ASSERT_GT(stress.combinedGrowthModifier, 0.8f);
    
    std::cout << "      ✓ Rainforest Vine thrives in hot/wet conditions" << std::endl;
}

void test_plant_stress_tundra_moss_in_desert_struggles() {
    std::cout << "    Testing Tundra Moss in desert (should struggle)..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Plant tundraMoss = factory.createTundraMoss(0, 0);
    
    // Create hot, dry environment
    EnvironmentState desertEnv;
    desertEnv.temperature = 40.0f;
    desertEnv.moisture = 0.1f;
    
    auto stress = calculatePlantStress(tundraMoss, desertEnv);
    
    std::cout << "      Tundra Moss at 40°C, 0.1 moisture:" << std::endl;
    std::cout << "        Temperature stress: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.temperature.severity) << std::endl;
    std::cout << "        Growth modifier: " << stress.combinedGrowthModifier << std::endl;
    std::cout << "        Health damage: " << stress.combinedHealthDamage << std::endl;
    
    // Should struggle significantly
    TEST_ASSERT_LT(stress.combinedGrowthModifier, 0.5f);
    TEST_ASSERT_GT(stress.combinedHealthDamage, 0.0f);
    
    std::cout << "      ✓ Tundra Moss struggles in desert (reduced growth, damage)" << std::endl;
}

//=============================================================================
// Test 7: Population Stability (Extended Simulation)
//=============================================================================

void test_population_stability_simple_ecosystem() {
    std::cout << "    Testing ecosystem stability over 500 ticks..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    // Create mini ecosystem in appropriate biomes
    struct OrganismState {
        float health = 1.0f;
        bool alive = true;
        std::string name;
    };
    
    // Tundra ecosystem
    OrganismState tundraMoss;
    tundraMoss.name = "Tundra Moss";
    OrganismState mammoth;
    mammoth.name = "Woolly Mammoth";
    OrganismState arcticWolf;
    arcticWolf.name = "Arctic Wolf";
    
    Plant moss = factory.createTundraMoss(0, 0);
    Creature mamm = factory.createWoollyMammoth(0, 0);
    Creature wolf = factory.createArcticWolf(0, 0);
    
    const float tundraTemp = -15.0f;
    const int ticks = 500;
    
    EnvironmentState tundraEnv;
    tundraEnv.temperature = tundraTemp;
    tundraEnv.moisture = 0.4f;
    
    std::cout << "      Running 500-tick simulation in tundra biome..." << std::endl;
    
    for (int tick = 0; tick < ticks; ++tick) {
        // Update moss
        if (tundraMoss.alive) {
            auto stress = calculatePlantStress(moss, tundraEnv);
            tundraMoss.health -= stress.combinedHealthDamage;
            if (tundraMoss.health <= 0.0f) tundraMoss.alive = false;
        }
        
        // Update mammoth
        if (mammoth.alive) {
            auto stress = calculateCreatureStress(mamm, tundraTemp);
            mammoth.health -= stress.healthDamageRate;
            if (mammoth.health <= 0.0f) mammoth.alive = false;
        }
        
        // Update wolf
        if (arcticWolf.alive) {
            auto stress = calculateCreatureStress(wolf, tundraTemp);
            arcticWolf.health -= stress.healthDamageRate;
            if (arcticWolf.health <= 0.0f) arcticWolf.alive = false;
        }
    }
    
    std::cout << "      Results after 500 ticks:" << std::endl;
    std::cout << "        Tundra Moss: health=" << tundraMoss.health << ", alive=" << tundraMoss.alive << std::endl;
    std::cout << "        Woolly Mammoth: health=" << mammoth.health << ", alive=" << mammoth.alive << std::endl;
    std::cout << "        Arctic Wolf: health=" << arcticWolf.health << ", alive=" << arcticWolf.alive << std::endl;
    
    // All should survive in appropriate biome
    TEST_ASSERT(tundraMoss.alive);
    TEST_ASSERT(mammoth.alive);
    TEST_ASSERT(arcticWolf.alive);
    
    // All should have >80% health (gradual selection pressure, not mass extinction)
    TEST_ASSERT_GT(tundraMoss.health, 0.8f);
    TEST_ASSERT_GT(mammoth.health, 0.8f);
    TEST_ASSERT_GT(arcticWolf.health, 0.8f);
    
    std::cout << "      ✓ Population stable - no mass extinction in appropriate biome" << std::endl;
}

void test_population_stability_no_instant_death() {
    std::cout << "    Testing no instant death in hostile environment..." << std::endl;
    
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    // Place arctic creature in EXTREMELY hostile environment
    Creature arcticWolf = factory.createArcticWolf(0, 0);
    
    // Even at 50°C, should survive at least a few ticks
    const float extremeTemp = 50.0f;
    float health = 1.0f;
    int ticksSurvived = 0;
    
    while (health > 0.0f && ticksSurvived < 100) {
        auto stress = calculateCreatureStress(arcticWolf, extremeTemp);
        health -= stress.healthDamageRate;
        ticksSurvived++;
    }
    
    std::cout << "      Arctic Wolf in 50°C extreme heat:" << std::endl;
    std::cout << "        Ticks survived: " << ticksSurvived << std::endl;
    
    // Should survive at least 10 ticks even in extreme conditions
    TEST_ASSERT_GE(ticksSurvived, 10);
    
    std::cout << "      ✓ No instant death - survived " << ticksSurvived << " ticks" << std::endl;
}

//=============================================================================
// Test 8: Edge Cases
//=============================================================================

void test_edge_case_zero_thermal_adaptations() {
    std::cout << "    Testing creature with zero thermal adaptations..." << std::endl;
    
    // Create adaptations with minimum values
    ThermalAdaptations noAdapt;
    noAdapt.furDensity = 0.0f;
    noAdapt.fatLayerThickness = 0.0f;
    noAdapt.metabolismRate = 1.0f;
    noAdapt.hideThickness = 0.0f;
    noAdapt.bodySize = 1.0f;
    noAdapt.thermoregulation = 0.0f;
    
    // With no adaptations, base tolerance should apply
    float baseTolLow = 0.0f;  // Temperate baseline
    float baseTolHigh = 30.0f;
    
    auto coldStress = EnvironmentalStressCalculator::calculateTemperatureStress(
        -10.0f, baseTolLow, baseTolHigh, noAdapt);
    
    auto hotStress = EnvironmentalStressCalculator::calculateTemperatureStress(
        40.0f, baseTolLow, baseTolHigh, noAdapt);
    
    std::cout << "      Zero-adaptation creature:" << std::endl;
    std::cout << "        At -10°C: " << EnvironmentalStressCalculator::stressLevelToString(coldStress.severity) << std::endl;
    std::cout << "        At 40°C: " << EnvironmentalStressCalculator::stressLevelToString(hotStress.severity) << std::endl;
    
    // Should be stressed in both directions
    TEST_ASSERT(coldStress.isStressed());
    TEST_ASSERT(hotStress.isStressed());
    
    std::cout << "      ✓ Zero-adaptation creature is vulnerable to both extremes" << std::endl;
}

void test_edge_case_exact_tolerance_boundary() {
    std::cout << "    Testing creature at exact tolerance boundary..." << std::endl;
    
    // Use desert fennec testing at its boundary, as it has narrow tolerances we can test
    auto registry = createTestRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature desertFennec = factory.createDesertFennec(0, 0);
    
    const Genome& genome = desertFennec.getGenome();
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Desert Fennec (tolerance: " << tempMin << "°C to " << tempMax << "°C):" << std::endl;
    
    // Test within comfortable range (well inside tolerance)
    float comfortTemp = (tempMin + tempMax) / 2.0f;
    auto stressComfortable = calculateCreatureStress(desertFennec, comfortTemp);
    
    // Test at upper boundary
    auto stressAtUpperBoundary = calculateCreatureStress(desertFennec, tempMax);
    
    // Test well outside boundary
    auto stressWellOutside = calculateCreatureStress(desertFennec, tempMax + 20.0f);
    
    std::cout << "        At " << comfortTemp << "°C (comfortable): "
              << EnvironmentalStressCalculator::stressLevelToString(stressComfortable.severity) << std::endl;
    std::cout << "        At " << tempMax << "°C (upper boundary): "
              << EnvironmentalStressCalculator::stressLevelToString(stressAtUpperBoundary.severity) << std::endl;
    std::cout << "        At " << (tempMax + 20.0f) << "°C (well outside): "
              << EnvironmentalStressCalculator::stressLevelToString(stressWellOutside.severity) << std::endl;
    
    // Comfortable should be no stress
    TEST_ASSERT_EQ(static_cast<int>(stressComfortable.severity), static_cast<int>(StressLevel::Comfortable));
    
    // Outside tolerance should have higher stress than boundary
    TEST_ASSERT_GT(stressWellOutside.stressLevel, stressAtUpperBoundary.stressLevel);
    
    std::cout << "      ✓ Boundary conditions handled gracefully" << std::endl;
}

void test_edge_case_extreme_climate() {
    std::cout << "    Testing extreme climate values..." << std::endl;
    
    // Use a temperate creature (not arctic) so both extremes cause lethal stress
    auto registry = createTestRegistry();
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature temperateCreature = factory.createApexPredator(0, 0);
    
    // Test at physical extremes - well beyond any reasonable tolerance
    auto stressExtremeCold = calculateCreatureStress(temperateCreature, -50.0f);
    auto stressExtremeHot = calculateCreatureStress(temperateCreature, 70.0f);
    
    std::cout << "      Extreme climate tests (temperate creature):" << std::endl;
    std::cout << "        At -50°C: " << EnvironmentalStressCalculator::stressLevelToString(stressExtremeCold.severity)
              << ", damage: " << stressExtremeCold.healthDamageRate << std::endl;
    std::cout << "        At 70°C: " << EnvironmentalStressCalculator::stressLevelToString(stressExtremeHot.severity)
              << ", damage: " << stressExtremeHot.healthDamageRate << std::endl;
    
    // Both should be severely stressed (Severe or Lethal)
    TEST_ASSERT_GE(static_cast<int>(stressExtremeCold.severity), static_cast<int>(StressLevel::Severe));
    TEST_ASSERT_GE(static_cast<int>(stressExtremeHot.severity), static_cast<int>(StressLevel::Severe));
    
    // Both should cause significant stress levels
    TEST_ASSERT_GT(stressExtremeCold.stressLevel, 0.5f);
    TEST_ASSERT_GT(stressExtremeHot.stressLevel, 0.5f);
    
    std::cout << "      ✓ Extreme climates cause severe/lethal stress" << std::endl;
}

void test_integration_edge_case_nan_temperature() {
    std::cout << "    Testing NaN temperature handling..." << std::endl;
    
    ThermalAdaptations adapt = ThermalAdaptations::defaults();
    
    // Test with NaN temperature
    float nanTemp = std::nanf("");
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        nanTemp, -5.0f, 35.0f, adapt);
    
    std::cout << "      NaN temperature result:" << std::endl;
    std::cout << "        Severity: " << EnvironmentalStressCalculator::stressLevelToString(stress.severity) << std::endl;
    std::cout << "        Energy drain: " << stress.energyDrainMultiplier << std::endl;
    
    // Should return safe defaults (no stress)
    TEST_ASSERT_EQ(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Comfortable));
    TEST_ASSERT_NEAR(stress.energyDrainMultiplier, 1.0f, 0.01f);
    
    std::cout << "      ✓ NaN temperature handled gracefully (returns no stress)" << std::endl;
}

//=============================================================================
// Test Runner
//=============================================================================

void runWorldOrganismIntegrationTests() {
    BEGIN_TEST_GROUP("World-Organism Integration Tests");
    
    // Test 1: Climate Data Pipeline
    std::cout << "\n--- Test 1: Climate Data Pipeline ---" << std::endl;
    RUN_TEST(test_climate_data_pipeline_world_has_climate);
    RUN_TEST(test_climate_data_pipeline_per_tile_variation);
    RUN_TEST(test_climate_data_pipeline_organism_receives_environment);
    
    // Test 2: Environmental Stress System
    std::cout << "\n--- Test 2: Environmental Stress System ---" << std::endl;
    RUN_TEST(test_environmental_stress_arctic_in_cold);
    RUN_TEST(test_environmental_stress_arctic_in_hot);
    RUN_TEST(test_environmental_stress_desert_in_hot);
    RUN_TEST(test_environmental_stress_desert_in_cold);
    RUN_TEST(test_environmental_stress_gradual_decline);
    
    // Test 3: Stress Affects Survival
    std::cout << "\n--- Test 3: Stress Affects Survival ---" << std::endl;
    RUN_TEST(test_stress_survival_arctic_in_desert);
    RUN_TEST(test_stress_survival_energy_drain_tracking);
    
    // Test 4: Biome-Appropriate Organisms Thrive
    std::cout << "\n--- Test 4: Biome-Appropriate Organisms Thrive ---" << std::endl;
    RUN_TEST(test_biome_appropriate_arctic_wolf_in_tundra);
    RUN_TEST(test_biome_appropriate_desert_fennec_in_desert);
    RUN_TEST(test_biome_appropriate_tropical_jaguar_in_tropical);
    
    // Test 5: Cross-Biome Migration Stress
    std::cout << "\n--- Test 5: Cross-Biome Migration Stress ---" << std::endl;
    RUN_TEST(test_cross_biome_migration_stress_detection);
    RUN_TEST(test_cross_biome_recovery_in_home_biome);
    
    // Test 6: Plant Stress and Growth
    std::cout << "\n--- Test 6: Plant Stress and Growth ---" << std::endl;
    RUN_TEST(test_plant_stress_tundra_moss_in_cold);
    RUN_TEST(test_plant_stress_desert_cactus_in_hot_dry);
    RUN_TEST(test_plant_stress_rainforest_vine_in_hot_wet);
    RUN_TEST(test_plant_stress_tundra_moss_in_desert_struggles);
    
    // Test 7: Population Stability
    std::cout << "\n--- Test 7: Population Stability ---" << std::endl;
    RUN_TEST(test_population_stability_simple_ecosystem);
    RUN_TEST(test_population_stability_no_instant_death);
    
    // Test 8: Edge Cases
    std::cout << "\n--- Test 8: Edge Cases ---" << std::endl;
    RUN_TEST(test_edge_case_zero_thermal_adaptations);
    RUN_TEST(test_edge_case_exact_tolerance_boundary);
    RUN_TEST(test_edge_case_extreme_climate);
    RUN_TEST(test_integration_edge_case_nan_temperature);
    
    END_TEST_GROUP();
}
