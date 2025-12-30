/**
 * @file test_environment_system.cpp
 * @brief Unit tests for EnvironmentSystem component
 */

#include "world/EnvironmentSystem.hpp"
#include "../genetics/test_framework.hpp"
#include <cmath>

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
    
    TEST_ASSERT(std::abs(light - EnvironmentSystem::DEFAULT_LIGHT_LEVEL) < 0.001f);
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
// Test: TileEnvironment Struct
//==============================================================================

void test_getEnvironmentAt_returns_all_defaults() {
    SeasonManager seasons;
    WorldGrid grid = createTestGrid();
    EnvironmentSystem env(seasons, grid);
    
    auto tileEnv = env.getEnvironmentAt(50, 50);
    
    TEST_ASSERT(std::abs(tileEnv.temperature - EnvironmentSystem::DEFAULT_TEMPERATURE) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.humidity - EnvironmentSystem::DEFAULT_HUMIDITY) < 0.001f);
    TEST_ASSERT(std::abs(tileEnv.lightLevel - EnvironmentSystem::DEFAULT_LIGHT_LEVEL) < 0.001f);
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
}
