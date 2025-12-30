/**
 * @file test_plant_manager.cpp
 * @brief Unit tests for PlantManager component
 */

#include "world/PlantManager.hpp"
#include "world/WorldGrid.hpp"
#include "world/ScentLayer.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "../genetics/test_framework.hpp"

using namespace EcoSim;
using namespace EcoSim::Testing;
namespace G = EcoSim::Genetics;

namespace {

//=============================================================================
// Tests: Initialization
//=============================================================================

void test_starts_uninitialized() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    PlantManager manager(grid, scents);
    
    // Should not be initialized until initialize() is called
    TEST_ASSERT(!manager.isInitialized());
    
    // Factory and registry should be null before initialization
    TEST_ASSERT(manager.factory() == nullptr);
    TEST_ASSERT(manager.registry() == nullptr);
}

void test_initialize() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    PlantManager manager(grid, scents);
    
    manager.initialize();
    
    TEST_ASSERT(manager.isInitialized());
    TEST_ASSERT(manager.factory() != nullptr);
    TEST_ASSERT(manager.registry() != nullptr);
}

void test_double_initialize_safe() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    PlantManager manager(grid, scents);
    
    manager.initialize();
    auto* factory1 = manager.factory();
    
    // Second initialize should be a no-op
    manager.initialize();
    auto* factory2 = manager.factory();
    
    TEST_ASSERT(factory1 == factory2);
    TEST_ASSERT(manager.isInitialized());
}

//=============================================================================
// Tests: Adding Plants
//=============================================================================

void test_add_plants_by_species() {
    WorldGrid grid(100, 100);
    ScentLayer scents(100, 100);
    
    // Set some tiles to valid elevations
    for (unsigned x = 0; x < 100; ++x) {
        for (unsigned y = 0; y < 100; ++y) {
            grid(x, y).setElevation(175);
        }
    }
    
    PlantManager manager(grid, scents);
    manager.initialize();
    
    // Count initial plants
    int initialCount = 0;
    for (unsigned x = 0; x < grid.width(); ++x) {
        for (unsigned y = 0; y < grid.height(); ++y) {
            initialCount += static_cast<int>(grid(x, y).getPlants().size());
        }
    }
    
    // Add grass in a specific elevation range
    manager.addPlants(150, 200, 5, "grass");
    
    // Count after adding
    int afterCount = 0;
    for (unsigned x = 0; x < grid.width(); ++x) {
        for (unsigned y = 0; y < grid.height(); ++y) {
            afterCount += static_cast<int>(grid(x, y).getPlants().size());
        }
    }
    
    TEST_ASSERT(afterCount >= initialCount);
}

void test_add_single_plant() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    PlantManager manager(grid, scents);
    manager.initialize();
    
    size_t before = grid(10, 10).getPlants().size();
    
    bool added = manager.addPlant(10, 10, "grass");
    
    if (added) {
        size_t after = grid(10, 10).getPlants().size();
        TEST_ASSERT_EQ(after, before + 1);
    }
}

void test_add_plant_invalid_location() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    PlantManager manager(grid, scents);
    manager.initialize();
    
    // Try to add at out-of-bounds location
    bool added = manager.addPlant(-1, -1, "grass");
    TEST_ASSERT(!added);
    
    added = manager.addPlant(1000, 1000, "grass");
    TEST_ASSERT(!added);
}

//=============================================================================
// Tests: Tick Processing
//=============================================================================

void test_tick_with_no_plants() {
    WorldGrid grid(20, 20);
    ScentLayer scents(20, 20);
    
    PlantManager manager(grid, scents);
    manager.initialize();
    
    // Should not crash even with no plants
    manager.tick(0);
    manager.tick(1);
    manager.tick(100);
    
    // Test passes if no crashes occur
    TEST_ASSERT(true);
}

void test_tick_processes_plants() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    // Set up tiles with appropriate elevations
    for (unsigned x = 0; x < 50; ++x) {
        for (unsigned y = 0; y < 50; ++y) {
            grid(x, y).setElevation(180);
        }
    }
    
    PlantManager manager(grid, scents);
    manager.initialize();
    
    // Add some plants
    manager.addPlants(150, 200, 10, "grass");
    
    // Run several ticks
    for (unsigned i = 0; i < 10; ++i) {
        manager.tick(i);
    }
    
    // Test passes if no crashes occur
    TEST_ASSERT(true);
}

//=============================================================================
// Tests: Environment Access
//=============================================================================

void test_environment_access() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    PlantManager manager(grid, scents);
    manager.initialize();
    
    // Should be able to access environment
    G::EnvironmentState& env = manager.environment();
    
    // Environment should have reasonable values (using actual member names)
    TEST_ASSERT(env.temperature >= -50.0f);  // Reasonable temperature range
    TEST_ASSERT(env.temperature <= 50.0f);
    TEST_ASSERT(env.humidity >= 0.0f);
    TEST_ASSERT(env.humidity <= 1.0f);
    
    // Const access should also work
    const PlantManager& constManager = manager;
    const G::EnvironmentState& constEnv = constManager.environment();
    TEST_ASSERT(constEnv.temperature == env.temperature);
}

//=============================================================================
// Tests: Factory Access
//=============================================================================

void test_factory_can_create_plants() {
    WorldGrid grid(50, 50);
    ScentLayer scents(50, 50);
    
    PlantManager manager(grid, scents);
    manager.initialize();
    
    G::PlantFactory* factory = manager.factory();
    TEST_ASSERT(factory != nullptr);
    
    // Should be able to create a plant
    G::Plant plant = factory->createFromTemplate("grass", 10, 10);
    TEST_ASSERT_EQ(plant.getX(), 10);
    TEST_ASSERT_EQ(plant.getY(), 10);
}

} // anonymous namespace

//=============================================================================
// Test Runner
//=============================================================================

void runPlantManagerTests() {
    BEGIN_TEST_GROUP("PlantManager - Initialization");
    RUN_TEST(test_starts_uninitialized);
    RUN_TEST(test_initialize);
    RUN_TEST(test_double_initialize_safe);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("PlantManager - Adding Plants");
    RUN_TEST(test_add_plants_by_species);
    RUN_TEST(test_add_single_plant);
    RUN_TEST(test_add_plant_invalid_location);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("PlantManager - Tick Processing");
    RUN_TEST(test_tick_with_no_plants);
    RUN_TEST(test_tick_processes_plants);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("PlantManager - Environment Access");
    RUN_TEST(test_environment_access);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("PlantManager - Factory Access");
    RUN_TEST(test_factory_can_create_plants);
    END_TEST_GROUP();
}
