/**
 * @file test_world_grid.cpp
 * @brief Unit tests for WorldGrid component
 */

#include "world/WorldGrid.hpp"
#include "../genetics/test_framework.hpp"

using namespace EcoSim;
using namespace EcoSim::Testing;

namespace {

//==============================================================================
// Test: Construction
//==============================================================================

void test_default_construction() {
    WorldGrid grid;
    
    TEST_ASSERT_EQ(grid.width(), 0u);
    TEST_ASSERT_EQ(grid.height(), 0u);
    TEST_ASSERT(!grid.isInitialized());
}

void test_sized_construction() {
    WorldGrid grid(100, 50);
    
    TEST_ASSERT_EQ(grid.width(), 100u);
    TEST_ASSERT_EQ(grid.height(), 50u);
    TEST_ASSERT(grid.isInitialized());
}

void test_construction_with_default_tile() {
    Tile defaultTile(100, '.', 1, true, false, TerrainType::PLAINS);
    WorldGrid grid(10, 10, defaultTile);
    
    TEST_ASSERT_EQ(grid.width(), 10u);
    TEST_ASSERT_EQ(grid.height(), 10u);
    
    // Check that tiles have the expected terrain type
    TEST_ASSERT_EQ(static_cast<int>(grid(5, 5).getTerrainType()), 
                   static_cast<int>(TerrainType::PLAINS));
}

//==============================================================================
// Test: Bounds Checking
//==============================================================================

void test_inBounds_valid() {
    WorldGrid grid(100, 50);
    
    // Corners
    TEST_ASSERT(grid.inBounds(0, 0));
    TEST_ASSERT(grid.inBounds(99, 0));
    TEST_ASSERT(grid.inBounds(0, 49));
    TEST_ASSERT(grid.inBounds(99, 49));
    
    // Center
    TEST_ASSERT(grid.inBounds(50, 25));
}

void test_inBounds_invalid() {
    WorldGrid grid(100, 50);
    
    // Negative coordinates
    TEST_ASSERT(!grid.inBounds(-1, 0));
    TEST_ASSERT(!grid.inBounds(0, -1));
    TEST_ASSERT(!grid.inBounds(-1, -1));
    
    // Beyond bounds
    TEST_ASSERT(!grid.inBounds(100, 0));
    TEST_ASSERT(!grid.inBounds(0, 50));
    TEST_ASSERT(!grid.inBounds(100, 50));
    TEST_ASSERT(!grid.inBounds(1000, 1000));
}

//==============================================================================
// Test: Tile Access
//==============================================================================

void test_at_bounds_checked() {
    WorldGrid grid(10, 10);
    
    // Valid access should not throw
    bool validAccessWorks = true;
    try {
        Tile& tile = grid.at(5, 5);
        (void)tile;  // Suppress unused warning
    } catch (...) {
        validAccessWorks = false;
    }
    TEST_ASSERT(validAccessWorks);
    
    // Invalid access should throw
    bool throwsOnInvalid = false;
    try {
        grid.at(-1, 0);
    } catch (const std::out_of_range&) {
        throwsOnInvalid = true;
    }
    TEST_ASSERT(throwsOnInvalid);
    
    throwsOnInvalid = false;
    try {
        grid.at(10, 10);
    } catch (const std::out_of_range&) {
        throwsOnInvalid = true;
    }
    TEST_ASSERT(throwsOnInvalid);
}

void test_operator_unchecked_access() {
    WorldGrid grid(10, 10);
    
    // Set a value using operator()
    Tile& tile = grid(5, 5);
    tile.setElevation(150);
    
    // Read it back
    TEST_ASSERT_EQ(grid(5, 5).getElevation(), 150u);
    
    // Const access
    const WorldGrid& constGrid = grid;
    TEST_ASSERT_EQ(constGrid(5, 5).getElevation(), 150u);
}

void test_at_const_access() {
    WorldGrid grid(10, 10);
    grid(5, 5).setElevation(200);
    
    const WorldGrid& constGrid = grid;
    TEST_ASSERT_EQ(constGrid.at(5, 5).getElevation(), 200u);
    
    // Const at() should also throw on invalid
    bool throwsOnInvalid = false;
    try {
        constGrid.at(-1, 0);
    } catch (const std::out_of_range&) {
        throwsOnInvalid = true;
    }
    TEST_ASSERT(throwsOnInvalid);
}

//==============================================================================
// Test: Resize
//==============================================================================

void test_resize() {
    WorldGrid grid;
    
    TEST_ASSERT(!grid.isInitialized());
    
    grid.resize(50, 25);
    
    TEST_ASSERT(grid.isInitialized());
    TEST_ASSERT_EQ(grid.width(), 50u);
    TEST_ASSERT_EQ(grid.height(), 25u);
    TEST_ASSERT(grid.inBounds(49, 24));
    TEST_ASSERT(!grid.inBounds(50, 25));
}

void test_resize_with_default() {
    WorldGrid grid;
    Tile waterTile(100, '~', 2, false, true, TerrainType::WATER);
    
    grid.resize(20, 20, waterTile);
    
    TEST_ASSERT_EQ(static_cast<int>(grid(10, 10).getTerrainType()), 
                   static_cast<int>(TerrainType::WATER));
    TEST_ASSERT(!grid(10, 10).isPassable());
}

//==============================================================================
// Test: Raw Access (backward compatibility)
//==============================================================================

void test_raw_access() {
    WorldGrid grid(10, 10);
    
    // Set via raw access - note: raw uses [x][y] indexing
    auto& raw = grid.raw();
    raw[3][4].setElevation(100);
    
    // Verify via normal access - grid(x, y) maps to raw[x][y]
    TEST_ASSERT_EQ(grid(3, 4).getElevation(), 100u);
    
    // Const raw access
    const WorldGrid& constGrid = grid;
    const auto& constRaw = constGrid.raw();
    TEST_ASSERT_EQ(constRaw[3][4].getElevation(), 100u);
}

//==============================================================================
// Test: Iteration
//==============================================================================

void test_iteration() {
    WorldGrid grid(5, 5);
    
    // Set all tiles to a specific elevation
    for (auto ref : grid) {
        ref.tile.setElevation(50);
    }
    
    // Verify all tiles were set
    for (unsigned int x = 0; x < grid.width(); ++x) {
        for (unsigned int y = 0; y < grid.height(); ++y) {
            TEST_ASSERT_EQ(grid(x, y).getElevation(), 50u);
        }
    }
}

void test_iteration_with_coordinates() {
    WorldGrid grid(3, 3);
    
    // Set elevation to x + y * 10 for each tile
    for (auto ref : grid) {
        ref.tile.setElevation(ref.x + ref.y * 10);
    }
    
    // Verify
    TEST_ASSERT_EQ(grid(0, 0).getElevation(), 0u);
    TEST_ASSERT_EQ(grid(1, 0).getElevation(), 1u);
    TEST_ASSERT_EQ(grid(2, 0).getElevation(), 2u);
    TEST_ASSERT_EQ(grid(0, 1).getElevation(), 10u);
    TEST_ASSERT_EQ(grid(1, 1).getElevation(), 11u);
    TEST_ASSERT_EQ(grid(2, 2).getElevation(), 22u);
}

void test_const_iteration() {
    WorldGrid grid(3, 3);
    grid(1, 1).setElevation(100);
    
    const WorldGrid& constGrid = grid;
    
    unsigned int count = 0;
    unsigned int elevationSum = 0;
    for (auto ref : constGrid) {
        count++;
        elevationSum += ref.tile.getElevation();
    }
    
    TEST_ASSERT_EQ(count, 9u);
    TEST_ASSERT_EQ(elevationSum, 100u);  // Only (1,1) has elevation 100
}

} // anonymous namespace

//==============================================================================
// Test Runner
//==============================================================================

void runWorldGridTests() {
    BEGIN_TEST_GROUP("WorldGrid - Construction");
    RUN_TEST(test_default_construction);
    RUN_TEST(test_sized_construction);
    RUN_TEST(test_construction_with_default_tile);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGrid - Bounds Checking");
    RUN_TEST(test_inBounds_valid);
    RUN_TEST(test_inBounds_invalid);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGrid - Tile Access");
    RUN_TEST(test_at_bounds_checked);
    RUN_TEST(test_operator_unchecked_access);
    RUN_TEST(test_at_const_access);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGrid - Resize");
    RUN_TEST(test_resize);
    RUN_TEST(test_resize_with_default);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGrid - Raw Access");
    RUN_TEST(test_raw_access);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGrid - Iteration");
    RUN_TEST(test_iteration);
    RUN_TEST(test_iteration_with_coordinates);
    RUN_TEST(test_const_iteration);
    END_TEST_GROUP();
}
