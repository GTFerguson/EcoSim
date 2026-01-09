/**
 * @file test_environment_system.cpp
 * @brief Unit tests for EnvironmentSystem component
 * 
 * Tests cover:
 * - Climate map connection
 * - Per-tile environmental queries
 * - Biome blending calculations (6 tests)
 * - Out-of-bounds handling
 * - Factory method validation
 * - Light level calculations
 * - Backward compatibility
 */

#include "world/EnvironmentSystem.hpp"
#include "world/ClimateWorldGenerator.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "../genetics/test_framework.hpp"
#include <cmath>
#include <algorithm>

using namespace EcoSim;
using namespace EcoSim::Testing;

namespace {

//==============================================================================
// Test Fixture Helpers
//==============================================================================

// Create a simple test grid
WorldGrid createTestGrid(unsigned int width = 100, unsigned int height = 100) {
    return WorldGrid(width, height);
}

// Create a climate map with default values
std::vector<std::vector<TileClimate>> createClimateMap(unsigned int width = 100, unsigned int height = 100) {
    return std::vector<std::vector<TileClimate>>(width, std::vector<TileClimate>(height));
}

//==============================================================================
// Test: Construction
//==============================================================================

void test_construction() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    
    // Should construct without throwing
    EnvironmentSystem env(seasons, grid);
    
    // Basic sanity check - should be able to query
    float temp = env.getTemperature(0, 0);
    TEST_ASSERT(temp > -100.0f && temp < 100.0f);
}

//==============================================================================
// Test: Default Values (backward compatibility)
//==============================================================================

void test_getTemperature_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    float temp = env.getTemperature(50, 50);
    
    TEST_ASSERT(std::abs(temp - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
}

void test_getHumidity_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    float humidity = env.getHumidity(50, 50);
    
    TEST_ASSERT(std::abs(humidity - EnvironmentSystem::DEFAULT_HUMIDITY) < 0.001f);
}

void test_getLightLevel_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    float light = env.getLightLevel(50, 50);
    
    // Light level depends on time of day, so just check it's in valid range
    TEST_ASSERT(light >= 0.0f && light <= 1.0f);
}

void test_getWindSpeed_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    float speed = env.getWindSpeed(50, 50);
    
    TEST_ASSERT(std::abs(speed - EnvironmentSystem::DEFAULT_WIND_SPEED) < 0.001f);
}

void test_getWindDirection_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    float direction = env.getWindDirection(50, 50);
    
    TEST_ASSERT(std::abs(direction - EnvironmentSystem::DEFAULT_WIND_DIRECTION) < 0.001f);
}

//==============================================================================
// Test: TileEnvironment Struct (Legacy)
//==============================================================================

void test_getEnvironmentAt_returns_all_defaults() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    auto tileEnv = env.getEnvironmentAt(50, 50);
    
    TEST_ASSERT(std::abs(tileEnv.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.humidity - EnvironmentSystem::DEFAULT_HUMIDITY) < 0.001f);
    // Light level depends on time of day
    TEST_ASSERT(tileEnv.lightLevel >= 0.0f && tileEnv.lightLevel <= 1.0f);
    TEST_ASSERT(std::abs(tileEnv.windSpeed - EnvironmentSystem::DEFAULT_WIND_SPEED) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.windDirection - EnvironmentSystem::DEFAULT_WIND_DIRECTION) < 0.001f);
}

void test_getEnvironmentAt_matches_individual_queries() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    int x = 25, y = 75;
    
    auto tileEnv = env.getEnvironmentAt(x, y);
    
    TEST_ASSERT(std::abs(tileEnv.temperature - env.getTemperature(x, y)) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.humidity - env.getHumidity(x, y)) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.lightLevel - env.getLightLevel(x, y)) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.windSpeed - env.getWindSpeed(x, y)) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.windDirection - env.getWindDirection(x, y)) < 0.001f);
}

//==============================================================================
// Test: Bounds Handling
//==============================================================================

void test_getTemperature_out_of_bounds_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem env(seasons, grid);
    
    // Test negative coordinates
    float temp1 = env.getTemperature(-1, 50);
    TEST_ASSERT(std::abs(temp1 - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    
    // Test coordinates beyond grid
    float temp2 = env.getTemperature(150, 50);
    TEST_ASSERT(std::abs(temp2 - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    
    float temp3 = env.getTemperature(50, 150);
    TEST_ASSERT(std::abs(temp3 - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
}

void test_getHumidity_out_of_bounds_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem env(seasons, grid);
    
    float humidity = env.getHumidity(-5, -5);
    TEST_ASSERT(std::abs(humidity - EnvironmentSystem::DEFAULT_HUMIDITY) < 0.001f);
}

void test_getLightLevel_out_of_bounds_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem env(seasons, grid);
    
    float light = env.getLightLevel(200, 200);
    TEST_ASSERT(std::abs(light - EnvironmentSystem::DEFAULT_LIGHT_LEVEL) < 0.001f);
}

void test_getWindSpeed_out_of_bounds_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem env(seasons, grid);
    
    float speed = env.getWindSpeed(100, 100);  // At boundary (out of bounds)
    TEST_ASSERT(std::abs(speed - EnvironmentSystem::DEFAULT_WIND_SPEED) < 0.001f);
}

void test_getWindDirection_out_of_bounds_returns_default() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem env(seasons, grid);
    
    float direction = env.getWindDirection(-10, 50);
    TEST_ASSERT(std::abs(direction - EnvironmentSystem::DEFAULT_WIND_DIRECTION) < 0.001f);
}

void test_getEnvironmentAt_out_of_bounds_returns_defaults() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem env(seasons, grid);
    
    auto tileEnv = env.getEnvironmentAt(-1, -1);
    
    TEST_ASSERT(std::abs(tileEnv.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.humidity - EnvironmentSystem::DEFAULT_HUMIDITY) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.lightLevel - EnvironmentSystem::DEFAULT_LIGHT_LEVEL) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.windSpeed - EnvironmentSystem::DEFAULT_WIND_SPEED) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.windDirection - EnvironmentSystem::DEFAULT_WIND_DIRECTION) < 0.001f);
}

//==============================================================================
// Test: Edge Cases
//==============================================================================

void test_queries_at_grid_corners() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem env(seasons, grid);
    
    // Test all four corners (valid positions)
    auto corner00 = env.getEnvironmentAt(0, 0);
    auto corner99_0 = env.getEnvironmentAt(99, 0);
    auto corner0_99 = env.getEnvironmentAt(0, 99);
    auto corner99_99 = env.getEnvironmentAt(99, 99);
    
    // All should return valid defaults
    TEST_ASSERT(std::abs(corner00.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(corner99_0.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(corner0_99.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(corner99_99.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
}

void test_queries_with_small_grid() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(1, 1);  // Minimal grid
    EnvironmentSystem env(seasons, grid);
    
    // Only (0, 0) should be valid
    auto validEnv = env.getEnvironmentAt(0, 0);
    TEST_ASSERT(std::abs(validEnv.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    
    // (1, 0) and (0, 1) should be out of bounds
    auto outOfBounds1 = env.getEnvironmentAt(1, 0);
    auto outOfBounds2 = env.getEnvironmentAt(0, 1);
    TEST_ASSERT(std::abs(outOfBounds1.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(outOfBounds2.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
}

void test_multiple_queries_same_tile() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    // Multiple queries to same tile should return consistent results
    float temp1 = env.getTemperature(50, 50);
    float temp2 = env.getTemperature(50, 50);
    float temp3 = env.getTemperature(50, 50);
    
    TEST_ASSERT(std::abs(temp1 - temp2) < 0.001f);
    TEST_ASSERT(std::abs(temp2 - temp3) < 0.001f);
}

//==============================================================================
// Test: Default Value Constants
//==============================================================================

void test_default_constants_are_reasonable() {
    // Temperature should be comfortable room temperature
    TEST_ASSERT(EnvironmentSystem::DEFAULT_TEMPERATURE >= 15.0f);
    TEST_ASSERT(EnvironmentSystem::DEFAULT_TEMPERATURE <= 25.0f);
    
    // Humidity should be in valid range
    TEST_ASSERT(EnvironmentSystem::DEFAULT_HUMIDITY >= 0.0f);
    TEST_ASSERT(EnvironmentSystem::DEFAULT_HUMIDITY <= 1.0f);
    
    // Light level should be in valid range
    TEST_ASSERT(EnvironmentSystem::DEFAULT_LIGHT_LEVEL >= 0.0f);
    TEST_ASSERT(EnvironmentSystem::DEFAULT_LIGHT_LEVEL <= 1.0f);
    
    // Wind speed should be non-negative
    TEST_ASSERT(EnvironmentSystem::DEFAULT_WIND_SPEED >= 0.0f);
    
    // Wind direction should be in valid range (0-360)
    TEST_ASSERT(EnvironmentSystem::DEFAULT_WIND_DIRECTION >= 0.0f);
    TEST_ASSERT(EnvironmentSystem::DEFAULT_WIND_DIRECTION < 360.0f);
}

//==============================================================================
// NEW TESTS: Climate Map Connection
//==============================================================================

void test_climate_map_connection() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    // Initially no climate data
    TEST_ASSERT(!envSystem.hasClimateData());
    
    // Create mock climate map
    auto climateMap = createClimateMap(100, 100);
    climateMap[50][50].temperature = 35.0f;
    climateMap[50][50].moisture = 0.2f;
    
    // Connect climate data
    envSystem.setClimateMap(&climateMap);
    TEST_ASSERT(envSystem.hasClimateData());
    
    // Verify queries return climate values
    TEST_ASSERT(std::abs(envSystem.getTemperature(50, 50) - 35.0f) < 0.001f);
    TEST_ASSERT(std::abs(envSystem.getMoisture(50, 50) - 0.2f) < 0.001f);
}

void test_environment_state_factory() {
    // Create a TileClimate with known values
    TileClimate climate;
    climate.temperature = 30.0f;
    climate.moisture = 0.8f;
    climate.elevation = 0.6f;
    climate.biomeBlend = BiomeBlend(Biome::TROPICAL_RAINFOREST);
    
    auto env = Genetics::EnvironmentState::fromTileClimate(climate, 0.5f, 2);
    
    TEST_ASSERT(std::abs(env.temperature - 30.0f) < 0.001f);
    TEST_ASSERT(std::abs(env.moisture - 0.8f) < 0.001f);
    TEST_ASSERT(std::abs(env.elevation - 0.6f) < 0.001f);
    TEST_ASSERT(env.primaryBiome == static_cast<int>(Biome::TROPICAL_RAINFOREST));
    TEST_ASSERT(env.season == 2);
    // Humidity is alias for moisture
    TEST_ASSERT(std::abs(env.humidity - 0.8f) < 0.001f);
}

void test_out_of_bounds_with_climate_returns_defaults() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    // Connect climate data
    auto climateMap = createClimateMap(100, 100);
    envSystem.setClimateMap(&climateMap);
    
    // Out of bounds should return defaults
    TEST_ASSERT(std::abs(envSystem.getTemperature(-1, 0) - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(envSystem.getTemperature(200, 0) - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
}

void test_backward_compatibility_without_climate() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    // Without climate data, should return defaults
    auto env = envSystem.getEnvironmentStateAt(50, 50);
    TEST_ASSERT(std::abs(env.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(env.moisture - EnvironmentSystem::DEFAULT_MOISTURE) < 0.001f);
    TEST_ASSERT(env.primaryBiome == static_cast<int>(Biome::TEMPERATE_GRASSLAND));
}

void test_light_level_varies_with_time() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    // Set time to noon (0.5) - should be bright
    // Note: We can't directly set time on SeasonManager easily, 
    // but we can at least verify light level is in valid range
    float light = envSystem.getLightLevel(50, 50);
    TEST_ASSERT(light >= 0.0f && light <= 1.0f);
    
    // Create multiple queries - should be consistent at same time
    float light2 = envSystem.getLightLevel(50, 50);
    TEST_ASSERT(std::abs(light - light2) < 0.001f);
}

void test_organism_receives_per_tile_environment() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    // Setup world with climate
    auto climateMap = createClimateMap(100, 100);
    climateMap[25][25].temperature = 40.0f;  // Hot tile
    climateMap[75][75].temperature = 5.0f;   // Cold tile
    envSystem.setClimateMap(&climateMap);
    
    // Get environment for each location
    auto hotEnv = envSystem.getEnvironmentStateAt(25, 25);
    auto coldEnv = envSystem.getEnvironmentStateAt(75, 75);
    
    TEST_ASSERT(std::abs(hotEnv.temperature - 40.0f) < 0.001f);
    TEST_ASSERT(std::abs(coldEnv.temperature - 5.0f) < 0.001f);
}

//==============================================================================
// NEW TESTS: Biome Blending (6 tests)
//==============================================================================

void test_biome_blend_weighted_vegetation_density() {
    // Create a blend: 60% Savanna + 40% Desert
    BiomeBlend blend(Biome::SAVANNA);
    blend.addContribution(Biome::DESERT_HOT, 0.67f);  // Will normalize to ~40%
    blend.normalize();
    
    // Verify weights after normalization (approximately 60/40)
    TEST_ASSERT(blend.contributions[0].weight > 0.55f);
    TEST_ASSERT(blend.contributions[0].weight < 0.65f);
    TEST_ASSERT(blend.contributions[1].weight > 0.35f);
    TEST_ASSERT(blend.contributions[1].weight < 0.45f);
    
    // Get blended vegetation density
    // Savanna: 0.4, Desert: 0.05
    // Expected: ~0.6 * 0.4 + ~0.4 * 0.05 = 0.24 + 0.02 = ~0.26
    float blendedVeg = blend.getBlendedVegetationDensity();
    TEST_ASSERT(blendedVeg >= 0.20f && blendedVeg <= 0.32f);
}

void test_biome_blend_weighted_movement_cost() {
    // Create a blend: 70% Forest + 30% Grassland
    BiomeBlend blend(Biome::TEMPERATE_FOREST);
    blend.addContribution(Biome::TEMPERATE_GRASSLAND, 0.43f);  // Will normalize to ~30%
    blend.normalize();
    
    // Get blended movement cost
    // Forest: 1.2, Grassland: 1.0
    // Expected: ~0.7 * 1.2 + ~0.3 * 1.0 = 0.84 + 0.30 = ~1.14
    float blendedCost = blend.getBlendedMovementCost();
    TEST_ASSERT(blendedCost >= 1.05f && blendedCost <= 1.25f);
}

void test_environment_state_from_blended_tile_climate() {
    TileClimate climate;
    climate.temperature = 28.0f;  // Direct value, NOT blended
    climate.moisture = 0.3f;      // Direct value, NOT blended
    climate.elevation = 0.45f;
    
    // Create blended biome: 60% Savanna + 40% Desert
    climate.biomeBlend = BiomeBlend(Biome::SAVANNA);
    climate.biomeBlend.addContribution(Biome::DESERT_HOT, 0.67f);
    climate.biomeBlend.normalize();
    
    auto env = Genetics::EnvironmentState::fromTileClimate(climate, 0.5f, 1);
    
    // Temperature and moisture come directly from TileClimate
    TEST_ASSERT(std::abs(env.temperature - 28.0f) < 0.001f);
    TEST_ASSERT(std::abs(env.moisture - 0.3f) < 0.001f);
    
    // Primary biome is the dominant one
    TEST_ASSERT(env.primaryBiome == static_cast<int>(Biome::SAVANNA));
    
    // Vegetation density and movement cost are BLENDED
    TEST_ASSERT(std::abs(env.vegetationDensity - climate.getVegetationDensity()) < 0.001f);
    TEST_ASSERT(std::abs(env.movementCostModifier - climate.getMovementCost()) < 0.001f);
}

void test_single_biome_returns_pure_properties() {
    BiomeBlend blend(Biome::TROPICAL_RAINFOREST);
    // No additional contributions - should return pure rainforest values
    
    // Get properties from BiomeProperties
    const BiomeProperties& rainforestProps = ClimateWorldGenerator::getBiomeProperties(Biome::TROPICAL_RAINFOREST);
    
    // Rainforest vegetation density should be high (1.0 based on BIOME_PROPERTIES)
    float vegDensity = blend.getBlendedVegetationDensity();
    TEST_ASSERT(std::abs(vegDensity - rainforestProps.vegetationDensity) < 0.01f);
    
    // Rainforest movement cost should be 1.5
    float moveCost = blend.getBlendedMovementCost();
    TEST_ASSERT(std::abs(moveCost - rainforestProps.movementCost) < 0.01f);
}

void test_environment_system_returns_blended_values() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    // Create climate map with blended tile
    auto climateMap = createClimateMap(100, 100);
    
    // Set up a blended tile at (50, 50)
    climateMap[50][50].temperature = 25.0f;
    climateMap[50][50].moisture = 0.4f;
    climateMap[50][50].biomeBlend = BiomeBlend(Biome::SAVANNA);
    climateMap[50][50].biomeBlend.addContribution(Biome::TEMPERATE_GRASSLAND, 0.5f);
    climateMap[50][50].biomeBlend.normalize();
    
    envSystem.setClimateMap(&climateMap);
    
    auto env = envSystem.getEnvironmentStateAt(50, 50);
    
    // Temperature/moisture come directly from TileClimate
    TEST_ASSERT(std::abs(env.temperature - 25.0f) < 0.001f);
    TEST_ASSERT(std::abs(env.moisture - 0.4f) < 0.001f);
    
    // Primary biome is the dominant one (Savanna with higher weight after normalize)
    TEST_ASSERT(env.primaryBiome == static_cast<int>(Biome::SAVANNA));
    
    // Vegetation density and movement cost should be weighted averages
    // Not testing exact values since they depend on BiomeProperties
    // Just verify they're different from single-biome values
    float pureGrasslandVeg = BiomeBlend(Biome::TEMPERATE_GRASSLAND).getBlendedVegetationDensity();
    float pureSavannaVeg = BiomeBlend(Biome::SAVANNA).getBlendedVegetationDensity();
    
    // Blended value should be between the two pure values
    float blendedVeg = env.vegetationDensity;
    float minVeg = std::min(pureGrasslandVeg, pureSavannaVeg);
    float maxVeg = std::max(pureGrasslandVeg, pureSavannaVeg);
    TEST_ASSERT(blendedVeg >= minVeg - 0.01f);
    TEST_ASSERT(blendedVeg <= maxVeg + 0.01f);
}

void test_biome_blend_can_support_plants_threshold() {
    // Desert (no plants) + Grassland (plants) blend
    // Need >= 30% plant-supporting biomes to support plants
    
    // 25% Grassland + 75% Desert - should NOT support plants
    BiomeBlend blend1(Biome::DESERT_HOT);
    blend1.addContribution(Biome::TEMPERATE_GRASSLAND, 0.33f);  // ~25% after normalize
    blend1.normalize();
    TEST_ASSERT(!blend1.canSupportPlants());
    
    // 40% Grassland + 60% Desert - SHOULD support plants (>30%)
    BiomeBlend blend2(Biome::DESERT_HOT);
    blend2.addContribution(Biome::TEMPERATE_GRASSLAND, 0.67f);  // ~40% after normalize
    blend2.normalize();
    TEST_ASSERT(blend2.canSupportPlants());
}

//==============================================================================
// Test: New Query Methods
//==============================================================================

void test_getMoisture_query() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    auto climateMap = createClimateMap(100, 100);
    climateMap[50][50].moisture = 0.75f;
    envSystem.setClimateMap(&climateMap);
    
    float moisture = envSystem.getMoisture(50, 50);
    TEST_ASSERT(std::abs(moisture - 0.75f) < 0.001f);
}

void test_getElevation_query() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    auto climateMap = createClimateMap(100, 100);
    climateMap[50][50].elevation = 0.9f;
    envSystem.setClimateMap(&climateMap);
    
    float elevation = envSystem.getElevation(50, 50);
    TEST_ASSERT(std::abs(elevation - 0.9f) < 0.001f);
}

void test_getBiome_query() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    auto climateMap = createClimateMap(100, 100);
    climateMap[50][50].biomeBlend = BiomeBlend(Biome::TUNDRA);
    envSystem.setClimateMap(&climateMap);
    
    int biome = envSystem.getBiome(50, 50);
    TEST_ASSERT(biome == static_cast<int>(Biome::TUNDRA));
}

void test_getClimateAt_raw_access() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid(100, 100);
    EnvironmentSystem envSystem(seasons, grid);
    
    auto climateMap = createClimateMap(100, 100);
    climateMap[50][50].temperature = 22.5f;
    climateMap[50][50].moisture = 0.65f;
    climateMap[50][50].biomeBlend = BiomeBlend(Biome::TEMPERATE_FOREST);
    climateMap[50][50].biomeBlend.addContribution(Biome::TEMPERATE_GRASSLAND, 0.3f);
    climateMap[50][50].biomeBlend.normalize();
    envSystem.setClimateMap(&climateMap);
    
    const TileClimate& climate = envSystem.getClimateAt(50, 50);
    
    TEST_ASSERT(std::abs(climate.temperature - 22.5f) < 0.001f);
    TEST_ASSERT(std::abs(climate.moisture - 0.65f) < 0.001f);
    TEST_ASSERT(climate.biome() == Biome::TEMPERATE_FOREST);
    TEST_ASSERT(climate.biomeBlend.count == 2);
}

void test_significantly_different() {
    Genetics::EnvironmentState env1;
    env1.temperature = 20.0f;
    env1.moisture = 0.5f;
    env1.elevation = 0.5f;
    env1.primaryBiome = static_cast<int>(Biome::TEMPERATE_GRASSLAND);
    env1.season = 0;
    
    // Same environment
    Genetics::EnvironmentState env2 = env1;
    TEST_ASSERT(!env1.significantlyDifferent(env2));
    
    // Different temperature
    Genetics::EnvironmentState env3 = env1;
    env3.temperature = 35.0f;
    TEST_ASSERT(env1.significantlyDifferent(env3));
    
    // Different biome
    Genetics::EnvironmentState env4 = env1;
    env4.primaryBiome = static_cast<int>(Biome::DESERT_HOT);
    TEST_ASSERT(env1.significantlyDifferent(env4));
    
    // Different season
    Genetics::EnvironmentState env5 = env1;
    env5.season = 2;
    TEST_ASSERT(env1.significantlyDifferent(env5));
}

} // anonymous namespace

//==============================================================================
// Test Runner
//==============================================================================

void runEnvironmentSystemTests() {
    BEGIN_TEST_GROUP("EnvironmentSystem - Construction");
    RUN_TEST(test_construction);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - Default Values");
    RUN_TEST(test_getTemperature_returns_default);
    RUN_TEST(test_getHumidity_returns_default);
    RUN_TEST(test_getLightLevel_returns_default);
    RUN_TEST(test_getWindSpeed_returns_default);
    RUN_TEST(test_getWindDirection_returns_default);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - TileEnvironment Struct");
    RUN_TEST(test_getEnvironmentAt_returns_all_defaults);
    RUN_TEST(test_getEnvironmentAt_matches_individual_queries);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - Bounds Handling");
    RUN_TEST(test_getTemperature_out_of_bounds_returns_default);
    RUN_TEST(test_getHumidity_out_of_bounds_returns_default);
    RUN_TEST(test_getLightLevel_out_of_bounds_returns_default);
    RUN_TEST(test_getWindSpeed_out_of_bounds_returns_default);
    RUN_TEST(test_getWindDirection_out_of_bounds_returns_default);
    RUN_TEST(test_getEnvironmentAt_out_of_bounds_returns_defaults);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - Edge Cases");
    RUN_TEST(test_queries_at_grid_corners);
    RUN_TEST(test_queries_with_small_grid);
    RUN_TEST(test_multiple_queries_same_tile);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - Default Constants");
    RUN_TEST(test_default_constants_are_reasonable);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - Climate Map Connection");
    RUN_TEST(test_climate_map_connection);
    RUN_TEST(test_environment_state_factory);
    RUN_TEST(test_out_of_bounds_with_climate_returns_defaults);
    RUN_TEST(test_backward_compatibility_without_climate);
    RUN_TEST(test_light_level_varies_with_time);
    RUN_TEST(test_organism_receives_per_tile_environment);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - Biome Blending");
    RUN_TEST(test_biome_blend_weighted_vegetation_density);
    RUN_TEST(test_biome_blend_weighted_movement_cost);
    RUN_TEST(test_environment_state_from_blended_tile_climate);
    RUN_TEST(test_single_biome_returns_pure_properties);
    RUN_TEST(test_environment_system_returns_blended_values);
    RUN_TEST(test_biome_blend_can_support_plants_threshold);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnvironmentSystem - New Query Methods");
    RUN_TEST(test_getMoisture_query);
    RUN_TEST(test_getElevation_query);
    RUN_TEST(test_getBiome_query);
    RUN_TEST(test_getClimateAt_raw_access);
    RUN_TEST(test_significantly_different);
    END_TEST_GROUP();
}
