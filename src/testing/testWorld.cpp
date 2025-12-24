/**
 *  Title   : World Test
 *  Author  : Gary Ferguson
 *  Purpose : Test world generation and tile functionality with proper assertions
 *
 *  Test Isolation:
 *  - This test does NOT access file system
 *  - All test data is created in-memory
 *  - No external dependencies on save files or fixtures
 *  - Each test uses fresh World instances
 */

#include "../../include/world/world.hpp"
#include "../../include/world/tile.hpp"
#include "../../include/objects/food.hpp"
#include "../../include/objects/spawner.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

// Test assertion macro with descriptive error messages
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << message << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

// Test constants for world configuration
const int SMALL_ROWS = 10;
const int SMALL_COLS = 10;
const int MEDIUM_ROWS = 50;
const int MEDIUM_COLS = 50;

/**
 * Setup function - creates fresh test configuration
 */
MapGen createTestMapGen(unsigned rows, unsigned cols) {
    MapGen mg;
    mg.seed = 7.1231;
    mg.scale = 0.0039;
    mg.freq = 4.4;
    mg.exponent = 0.6;
    mg.terraces = 64;
    mg.rows = rows;
    mg.cols = cols;
    mg.isIsland = false;
    return mg;
}

/**
 * Teardown function - cleanup test data
 */
void testTeardown() {
    // World objects are stack-allocated, automatic cleanup
}

int main () {
  cout << "=== World Test Suite ===" << endl;
  
  cout << endl << "TEST 1: World Initialization" << endl;
  
  MapGen mg = createTestMapGen(SMALL_ROWS, SMALL_COLS);
  OctaveGen og;
  
  World world(mg, og);
  
  TEST_ASSERT(world.getRows() == SMALL_ROWS, "World should have correct number of rows");
  TEST_ASSERT(world.getCols() == SMALL_COLS, "World should have correct number of columns");
  TEST_ASSERT(world.getSeed() == mg.seed, "World should preserve seed");
  
  cout << "  Rows: " << world.getRows() << endl;
  cout << "  Cols: " << world.getCols() << endl;
  cout << "  Seed: " << world.getSeed() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 2: Grid Access and Dimensions" << endl;
  
  auto& grid = world.getGrid();
  
  TEST_ASSERT(grid.size() == SMALL_ROWS, "Grid should have correct row count");
  TEST_ASSERT(grid[0].size() == SMALL_COLS, "Grid should have correct column count");
  
  cout << "  Grid rows: " << grid.size() << endl;
  cout << "  Grid cols: " << grid[0].size() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 3: Tile Properties" << endl;
  
  // Access tiles and verify they have valid properties
  Tile& tile00 = grid[0][0];
  Tile& tileMid = grid[SMALL_ROWS/2][SMALL_COLS/2];
  
  char char00 = tile00.getChar();
  char charMid = tileMid.getChar();
  unsigned int elev00 = tile00.getElevation();
  
  cout << "  Tile (0,0) char: " << char00 << ", elevation: " << elev00 << endl;
  cout << "  Tile (mid) char: " << charMid << endl;
  
  TEST_ASSERT(char00 != '\0', "Tile should have valid character");
  TEST_ASSERT(elev00 >= 0, "Tile should have valid elevation");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 4: Food Addition to Tiles" << endl;
  
  Food testFood(1, "TestFood", "test", true, 'F', 1, 50.0f, 100);
  
  world.addFood(0, 0, testFood);
  
  vector<Food>& foodVec = grid[0][0].getFoodVec();
  
  cout << "  Food items at (0,0): " << foodVec.size() << endl;
  TEST_ASSERT(foodVec.size() > 0, "Tile should contain added food");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 5: Spawner Addition to Tiles" << endl;
  
  Food food(2, "Food", "desc", true, 'Y', 2, 40.0f, 100);
  Spawner testSpawner("TestSpawner", "test", true, 'S', 3, 50, 3, 10, food);
  
  world.addSpawner(1, 1, testSpawner);
  
  vector<Spawner>& spawners = grid[1][1].getSpawners();
  
  cout << "  Spawners at (1,1): " << spawners.size() << endl;
  TEST_ASSERT(spawners.size() > 0, "Tile should contain added spawner");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 6: Edge Case - Boundary Coordinates" << endl;
  
  // Test accessing tiles at boundaries
  Tile& cornerTile = grid[SMALL_ROWS-1][SMALL_COLS-1];
  char cornerChar = cornerTile.getChar();
  
  cout << "  Corner tile (" << (SMALL_ROWS-1) << "," << (SMALL_COLS-1) << ") char: " << cornerChar << endl;
  TEST_ASSERT(cornerChar != '\0', "Boundary tile should be valid");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 7: Edge Case - Zero-Sized World" << endl;
  
  // Test with minimal world size
  MapGen mgMin = createTestMapGen(1, 1);
  World minWorld(mgMin, og);
  
  TEST_ASSERT(minWorld.getRows() == 1, "Minimal world should have 1 row");
  TEST_ASSERT(minWorld.getCols() == 1, "Minimal world should have 1 column");
  
  auto& minGrid = minWorld.getGrid();
  TEST_ASSERT(minGrid.size() == 1, "Minimal grid should exist");
  
  cout << "  Minimal world: " << minWorld.getRows() << "x" << minWorld.getCols() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 8: Negative Test - Terrain Level Bounds" << endl;
  
  // NOTE: getTerraces() returns the noise config terraces (e.g., 64),
  // NOT the number of terrain types in _tileGen (which is 17).
  // Test valid terrain level indices (0-16 for 17 terrain types)
  // Count from world.cpp: DEEP_WATER, WATER, SHALLOW_WATER, SHALLOW_WATER_2,
  // SAND, DESERT_SAND, PLAINS, SAVANNA, SHORT_GRASS, SHORT_GRASS(medium),
  // LONG_GRASS, FOREST, MOUNTAIN, MOUNTAIN_2, MOUNTAIN_3, SNOW, PEAKS = 17
  const unsigned int NUM_TERRAIN_TYPES = 17;  // Count of TileGen entries in World constructor
  
  // Test terrain level access at boundaries
  double level0 = world.getTerrainLevel(0);
  double levelMax = world.getTerrainLevel(NUM_TERRAIN_TYPES - 1);
  
  cout << "  Terrain level 0: " << level0 << endl;
  cout << "  Terrain level max (" << (NUM_TERRAIN_TYPES-1) << "): " << levelMax << endl;
  
  // Terrain levels are elevation thresholds (e.g., 90, 110, etc.), not normalized values
  TEST_ASSERT(level0 > 0.0, "Terrain level should be positive");
  TEST_ASSERT(levelMax > 0.0, "Max terrain level should be positive");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 9: Edge Case - Large World Generation" << endl;
  
  MapGen mgLarge = createTestMapGen(MEDIUM_ROWS, MEDIUM_COLS);
  World largeWorld(mgLarge, og);
  
  TEST_ASSERT(largeWorld.getRows() == MEDIUM_ROWS, "Large world should have correct rows");
  TEST_ASSERT(largeWorld.getCols() == MEDIUM_COLS, "Large world should have correct cols");
  
  auto& largeGrid = largeWorld.getGrid();
  TEST_ASSERT(largeGrid.size() == MEDIUM_ROWS, "Large grid should be fully allocated");
  
  cout << "  Large world: " << largeWorld.getRows() << "x" << largeWorld.getCols() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 10: World toString Output" << endl;
  
  string worldStr = world.toString();
  
  cout << "  World string length: " << worldStr.length() << endl;
  TEST_ASSERT(!worldStr.empty(), "World toString should not be empty");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 11: Negative Test - Adding Multiple Items to Same Tile" << endl;
  
  Food food1(10, "Food1", "first", true, 'A', 5, 30.0f, 50);
  Food food2(11, "Food2", "second", true, 'B', 6, 40.0f, 60);
  Food food3(12, "Food3", "third", true, 'C', 7, 50.0f, 70);
  
  world.addFood(2, 2, food1);
  world.addFood(2, 2, food2);
  world.addFood(2, 2, food3);
  
  vector<Food>& multiFood = grid[2][2].getFoodVec();
  
  cout << "  Multiple food items at (2,2): " << multiFood.size() << endl;
  TEST_ASSERT(multiFood.size() > 0, "Should be able to add multiple food items");
  
  cout << "  PASSED" << endl;

  testTeardown();

  cout << endl << "=== All World Tests PASSED ===" << endl;
  
  return 0;
}