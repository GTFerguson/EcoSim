/**
 * @file test_plant_manager.cpp
 * @brief Unit tests for PlantManager component
 */

#include "world/PlantManager.hpp"
#include "world/PlantSpatialIndex.hpp"
#include "world/WorldGrid.hpp"
#include "world/ScentLayer.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "../genetics/test_framework.hpp"

#include <unordered_set>

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

//=============================================================================
// Tests: Spatial Index Integrity
//
// The PlantSpatialIndex stores raw Plant* grouped by cell. Those pointers are
// owned by the tile's shared_ptr<Plant>. If a plant's shared_ptr is destroyed
// without the index being updated, queryRadius walks a dangling pointer and
// segfaults. These tests pin the invariant: every pointer in the index must
// address a currently-alive plant that lives in some tile.
//=============================================================================

// Collect all raw Plant* currently held by any tile. Used as the "truth set"
// to check the index against.
static std::unordered_set<const G::Plant*> collectLivePlantPointers(WorldGrid& grid) {
    std::unordered_set<const G::Plant*> live;
    for (unsigned x = 0; x < grid.width(); ++x) {
        for (unsigned y = 0; y < grid.height(); ++y) {
            for (const auto& p : grid(x, y).getPlants()) {
                if (p) live.insert(p.get());
            }
        }
    }
    return live;
}

// Walk the spatial index and verify every pointer is in the live set.
// Returns number of dangling entries found.
static int countDanglingIndexEntries(PlantManager& manager, WorldGrid& grid) {
    auto live = collectLivePlantPointers(grid);
    int dangling = 0;
    const PlantSpatialIndex* idx = manager.getPlantIndex();
    if (!idx) return 0;

    for (unsigned cx = 0; cx * 32u < grid.width() + 32; ++cx) {
        for (unsigned cy = 0; cy * 32u < grid.height() + 32; ++cy) {
            auto cellPlants = idx->queryCell(static_cast<int>(cx), static_cast<int>(cy));
            for (const G::Plant* p : cellPlants) {
                if (p && live.find(p) == live.end()) {
                    ++dangling;
                }
            }
        }
    }
    return dangling;
}

void test_index_clean_after_add_and_query() {
    Tile passableTile(100, '.', 1, true, false, 180, TerrainType::PLAINS);
    WorldGrid grid(64, 64, passableTile);
    ScentLayer scents(64, 64);

    PlantManager manager(grid, scents);
    manager.initialize();
    manager.addPlants(150, 200, 30, "grass");

    // Force a rebuild via query.
    auto results = manager.queryPlantsInRadius(32, 32, 10.0f);
    (void)results;

    TEST_ASSERT_EQ(0, countDanglingIndexEntries(manager, grid));
}

void test_index_clean_after_plant_deaths_via_takeDamage() {
    Tile passableTile(100, '.', 1, true, false, 180, TerrainType::PLAINS);
    WorldGrid grid(64, 64, passableTile);
    ScentLayer scents(64, 64);

    PlantManager manager(grid, scents);
    manager.initialize();
    manager.addPlants(150, 200, 50, "grass");

    // Prime the index.
    manager.queryPlantsInRadius(32, 32, 10.0f);

    // Kill every plant via takeDamage (mimics FeedingBehavior eating to death).
    for (unsigned x = 0; x < grid.width(); ++x) {
        for (unsigned y = 0; y < grid.height(); ++y) {
            for (auto& p : grid(x, y).getPlants()) {
                if (p) p->takeDamage(1e6f);  // overkill damage
            }
        }
    }

    // Tick PlantManager — this should detect dead plants and clean them from
    // both the tile and the spatial index.
    manager.tick(1);

    // Any stale Plant* in the index here is a bug.
    int dangling = countDanglingIndexEntries(manager, grid);
    TEST_ASSERT_EQ(0, dangling);
}

// Dispersal-path invariant: if the target tile rejects the offspring
// (e.g. tile is already occupied), the spatial index must not retain a
// pointer to the destroyed plant. PlantManager's dispersal code must
// addPlant-first, insert-second — never insert a pointer it doesn't
// know will survive.
void test_dispersal_into_full_tile_does_not_leak_stale_pointer() {
    Tile passableTile(100, '.', 1, true, false, 180, TerrainType::PLAINS);
    WorldGrid grid(64, 64, passableTile);
    ScentLayer scents(64, 64);

    PlantManager manager(grid, scents);
    manager.initialize();

    // Occupy the target tile so any would-be dispersed plant is rejected.
    manager.addPlant(32, 32, "grass");
    TEST_ASSERT_EQ(size_t{1}, grid(32, 32).getPlants().size());
    TEST_ASSERT(grid(32, 32).getPlants()[0]->isAlive());

    // Prime index (rebuilds, registers the one plant).
    manager.queryPlantsInRadius(32, 32, 5.0f);

    // Mirror the dispersal code's (post-fix) order: addPlant first, only
    // insert into the index if addPlant accepted the plant. Rejected
    // plants let their local shared_ptr drop and never enter the index.
    {
        auto p = manager.factory()->createFromTemplate("grass", 32, 32);
        auto pPtr = std::make_shared<G::Plant>(std::move(p));
        bool added = grid(32, 32).addPlant(pPtr);
        TEST_ASSERT(!added);  // rejected because tile already has a plant
        if (added) {
            const_cast<PlantSpatialIndex*>(manager.getPlantIndex())->insert(
                pPtr.get(), 32, 32);
        }
    }  // pPtr dies here — but index never got the pointer.

    int dangling = countDanglingIndexEntries(manager, grid);
    TEST_ASSERT_EQ(0, dangling);
}

void test_index_query_after_deaths_does_not_return_stale_pointers() {
    Tile passableTile(100, '.', 1, true, false, 180, TerrainType::PLAINS);
    WorldGrid grid(64, 64, passableTile);
    ScentLayer scents(64, 64);

    PlantManager manager(grid, scents);
    manager.initialize();
    manager.addPlants(150, 200, 50, "grass");

    // Prime the index.
    manager.queryPlantsInRadius(32, 32, 10.0f);
    auto liveBefore = collectLivePlantPointers(grid);

    // Kill every plant.
    for (unsigned x = 0; x < grid.width(); ++x) {
        for (unsigned y = 0; y < grid.height(); ++y) {
            for (auto& p : grid(x, y).getPlants()) {
                if (p) p->takeDamage(1e6f);
            }
        }
    }
    manager.tick(1);

    // Plants that are still in tiles (shouldn't be any, but just in case).
    auto liveAfter = collectLivePlantPointers(grid);

    // Every pointer the query returns must point into liveAfter.
    auto results = manager.queryPlantsInRadius(32, 32, 100.0f);
    for (const G::Plant* p : results) {
        TEST_ASSERT(liveAfter.find(p) != liveAfter.end());
    }
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

    BEGIN_TEST_GROUP("PlantManager - Spatial Index Integrity");
    RUN_TEST(test_index_clean_after_add_and_query);
    RUN_TEST(test_index_clean_after_plant_deaths_via_takeDamage);
    RUN_TEST(test_dispersal_into_full_tile_does_not_leak_stale_pointer);
    RUN_TEST(test_index_query_after_deaths_does_not_return_stale_pointers);
    END_TEST_GROUP();
}
