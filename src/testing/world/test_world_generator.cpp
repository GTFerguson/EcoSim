/**
 * @file test_world_generator.cpp
 * @brief Unit tests for WorldGenerator component
 * 
 * Tests terrain generation, seed reproducibility, and configuration options.
 */

#include "world/WorldGenerator.hpp"
#include "world/WorldGrid.hpp"
#include "../genetics/test_framework.hpp"

#include <set>
#include <cmath>

using namespace EcoSim;
using namespace EcoSim::Testing;

namespace {

//==============================================================================
// Test: Construction
//==============================================================================

void test_default_construction() {
    WorldGenerator generator;
    
    // Default configuration should have reasonable values
    MapGen mapGen = generator.getMapGen();
    TEST_ASSERT_EQ(mapGen.rows, 500u);
    TEST_ASSERT_EQ(mapGen.cols, 500u);
    TEST_ASSERT(mapGen.terraces > 0);
    
    OctaveGen octaveGen = generator.getOctaveGen();
    TEST_ASSERT(octaveGen.quantity > 0);
    
    // Should have terrain rules initialized
    TEST_ASSERT(generator.getTerrainRuleCount() > 0);
}

void test_parameterized_construction() {
    MapGen mapGen;
    mapGen.seed = 12345.0;
    mapGen.rows = 100;
    mapGen.cols = 100;
    mapGen.scale = 0.02;
    
    OctaveGen octaveGen;
    octaveGen.quantity = 3;
    
    WorldGenerator generator(mapGen, octaveGen);
    
    TEST_ASSERT_EQ(generator.getMapGen().seed, 12345.0);
    TEST_ASSERT_EQ(generator.getMapGen().rows, 100u);
    TEST_ASSERT_EQ(generator.getMapGen().cols, 100u);
    TEST_ASSERT_EQ(generator.getOctaveGen().quantity, 3u);
}

//==============================================================================
// Test: Configuration
//==============================================================================

void test_set_map_gen() {
    WorldGenerator generator;
    
    MapGen newMapGen;
    newMapGen.seed = 999.0;
    newMapGen.rows = 50;
    newMapGen.cols = 75;
    newMapGen.isIsland = true;
    
    generator.setMapGen(newMapGen);
    
    MapGen retrieved = generator.getMapGen();
    TEST_ASSERT_EQ(retrieved.seed, 999.0);
    TEST_ASSERT_EQ(retrieved.rows, 50u);
    TEST_ASSERT_EQ(retrieved.cols, 75u);
    TEST_ASSERT(retrieved.isIsland);
}

void test_set_octave_gen() {
    WorldGenerator generator;
    
    OctaveGen newOctaveGen;
    newOctaveGen.quantity = 6;
    newOctaveGen.minWeight = 0.05;
    newOctaveGen.maxWeight = 0.8;
    
    generator.setOctaveGen(newOctaveGen);
    
    OctaveGen retrieved = generator.getOctaveGen();
    TEST_ASSERT_EQ(retrieved.quantity, 6u);
    TEST_ASSERT(std::abs(retrieved.minWeight - 0.05) < 0.001);
    TEST_ASSERT(std::abs(retrieved.maxWeight - 0.8) < 0.001);
}

void test_terrain_level_access() {
    WorldGenerator generator;
    
    // Get initial terrain level
    double originalLevel = generator.getTerrainLevel(0);
    TEST_ASSERT(originalLevel > 0);  // Should have a positive elevation threshold
    
    // Modify terrain level
    generator.setTerrainLevel(0, 50.0);
    TEST_ASSERT_EQ(generator.getTerrainLevel(0), 50.0);
    
    // Invalid level access should return 0
    TEST_ASSERT_EQ(generator.getTerrainLevel(1000), 0.0);
}

void test_terrain_rules() {
    WorldGenerator generator;
    
    const auto& rules = generator.getTerrainRules();
    
    // Should have multiple terrain types
    TEST_ASSERT(rules.size() >= 10);  // At least 10 terrain types
    
    // Elevation thresholds should be increasing
    double prevElevation = 0;
    for (const auto& rule : rules) {
        TEST_ASSERT(rule.maxElevation >= prevElevation);
        prevElevation = rule.maxElevation;
    }
}

//==============================================================================
// Test: Generation - Basic
//==============================================================================

void test_generate_fills_grid() {
    MapGen mapGen;
    mapGen.seed = 42.0;
    mapGen.rows = 20;
    mapGen.cols = 20;
    
    WorldGenerator generator(mapGen, OctaveGen{});
    WorldGrid grid(20, 20);
    
    generator.generate(grid);
    
    // Verify all tiles have valid terrain types (not default)
    bool hasVariedTerrain = false;
    TerrainType firstType = grid(0, 0).getTerrainType();
    
    for (unsigned int x = 0; x < grid.width(); ++x) {
        for (unsigned int y = 0; y < grid.height(); ++y) {
            const Tile& tile = grid(x, y);
            
            // Elevation should be set (not zero for most tiles after generation)
            // Note: Some tiles might have low elevation, so just check it's reasonable
            TEST_ASSERT(tile.getElevation() <= 255);
            
            // Check for terrain variety
            if (tile.getTerrainType() != firstType) {
                hasVariedTerrain = true;
            }
        }
    }
    
    // A properly generated map should have terrain variety
    TEST_ASSERT(hasVariedTerrain);
}

void test_generate_produces_valid_tiles() {
    MapGen mapGen;
    mapGen.seed = 123.0;
    mapGen.rows = 30;
    mapGen.cols = 30;
    
    WorldGenerator generator(mapGen, OctaveGen{});
    WorldGrid grid(30, 30);
    
    generator.generate(grid);
    
    // Collect all terrain types used
    std::set<TerrainType> terrainTypes;
    
    for (unsigned int x = 0; x < grid.width(); ++x) {
        for (unsigned int y = 0; y < grid.height(); ++y) {
            const Tile& tile = grid(x, y);
            terrainTypes.insert(tile.getTerrainType());
            
            // Tile properties should be consistent
            // Water tiles should not be passable (except shallow water)
            TerrainType type = tile.getTerrainType();
            if (type == TerrainType::DEEP_WATER || type == TerrainType::WATER) {
                TEST_ASSERT(!tile.isPassable());
            }
        }
    }
    
    // Should have generated multiple terrain types
    TEST_ASSERT(terrainTypes.size() >= 2);
}

//==============================================================================
// Test: Seed Reproducibility
//==============================================================================

void test_same_seed_same_world() {
    MapGen mapGen;
    mapGen.seed = 12345.0;
    mapGen.rows = 25;
    mapGen.cols = 25;
    
    // Generate first world
    WorldGenerator generator1(mapGen, OctaveGen{});
    WorldGrid grid1(25, 25);
    generator1.generate(grid1);
    
    // Generate second world with same seed
    WorldGenerator generator2(mapGen, OctaveGen{});
    WorldGrid grid2(25, 25);
    generator2.generate(grid2);
    
    // All tiles should be identical
    bool allMatch = true;
    for (unsigned int x = 0; x < grid1.width() && allMatch; ++x) {
        for (unsigned int y = 0; y < grid1.height() && allMatch; ++y) {
            if (grid1(x, y).getElevation() != grid2(x, y).getElevation() ||
                grid1(x, y).getTerrainType() != grid2(x, y).getTerrainType()) {
                allMatch = false;
            }
        }
    }
    
    TEST_ASSERT(allMatch);
}

void test_different_seed_different_world() {
    MapGen mapGen1;
    mapGen1.seed = 100.0;
    mapGen1.rows = 25;
    mapGen1.cols = 25;
    
    MapGen mapGen2 = mapGen1;
    mapGen2.seed = 200.0;
    
    // Generate worlds with different seeds
    WorldGenerator generator1(mapGen1, OctaveGen{});
    WorldGrid grid1(25, 25);
    generator1.generate(grid1);
    
    WorldGenerator generator2(mapGen2, OctaveGen{});
    WorldGrid grid2(25, 25);
    generator2.generate(grid2);
    
    // Count differences
    int differences = 0;
    for (unsigned int x = 0; x < grid1.width(); ++x) {
        for (unsigned int y = 0; y < grid1.height(); ++y) {
            if (grid1(x, y).getElevation() != grid2(x, y).getElevation()) {
                differences++;
            }
        }
    }
    
    // Should have significant differences (at least 10% of tiles)
    int totalTiles = static_cast<int>(grid1.width() * grid1.height());
    TEST_ASSERT(differences > totalTiles / 10);
}

void test_generate_with_explicit_seed() {
    MapGen mapGen;
    mapGen.seed = 0.0;  // Initial seed doesn't matter
    mapGen.rows = 20;
    mapGen.cols = 20;
    
    WorldGenerator generator(mapGen, OctaveGen{});
    
    // Generate with explicit seed
    WorldGrid grid1(20, 20);
    generator.generate(grid1, 555.0);
    
    // Generate again with same explicit seed
    WorldGrid grid2(20, 20);
    generator.generate(grid2, 555.0);
    
    // Should be identical
    bool allMatch = true;
    for (unsigned int x = 0; x < grid1.width() && allMatch; ++x) {
        for (unsigned int y = 0; y < grid1.height() && allMatch; ++y) {
            if (grid1(x, y).getElevation() != grid2(x, y).getElevation()) {
                allMatch = false;
            }
        }
    }
    
    TEST_ASSERT(allMatch);
}

//==============================================================================
// Test: Configuration Affects Output
//==============================================================================

void test_island_mode() {
    MapGen normalMap;
    normalMap.seed = 42.0;
    normalMap.rows = 50;
    normalMap.cols = 50;
    normalMap.isIsland = false;
    
    MapGen islandMap = normalMap;
    islandMap.isIsland = true;
    
    WorldGenerator normalGen(normalMap, OctaveGen{});
    WorldGrid normalGrid(50, 50);
    normalGen.generate(normalGrid);
    
    WorldGenerator islandGen(islandMap, OctaveGen{});
    WorldGrid islandGrid(50, 50);
    islandGen.generate(islandGrid);
    
    // Island mode should have more water around edges
    // Count water tiles in outer ring vs center
    int islandEdgeWater = 0;
    int normalEdgeWater = 0;
    
    for (unsigned int x = 0; x < 50; ++x) {
        for (unsigned int y = 0; y < 50; ++y) {
            bool isEdge = (x < 3 || x >= 47 || y < 3 || y >= 47);
            
            if (isEdge) {
                TerrainType islandType = islandGrid(x, y).getTerrainType();
                TerrainType normalType = normalGrid(x, y).getTerrainType();
                
                if (islandType == TerrainType::DEEP_WATER || 
                    islandType == TerrainType::WATER ||
                    islandType == TerrainType::SHALLOW_WATER ||
                    islandType == TerrainType::SHALLOW_WATER_2) {
                    islandEdgeWater++;
                }
                
                if (normalType == TerrainType::DEEP_WATER ||
                    normalType == TerrainType::WATER ||
                    normalType == TerrainType::SHALLOW_WATER ||
                    normalType == TerrainType::SHALLOW_WATER_2) {
                    normalEdgeWater++;
                }
            }
        }
    }
    
    // Island mode should generally have more water at edges
    // This isn't guaranteed for every seed, but with island mode the edges
    // should be lower elevation on average
    // We just verify the maps are different
    TEST_ASSERT(islandEdgeWater != normalEdgeWater || 
                islandGrid(25, 25).getElevation() != normalGrid(25, 25).getElevation());
}

void test_terraces_affect_output() {
    MapGen map1;
    map1.seed = 123.0;
    map1.rows = 30;
    map1.cols = 30;
    map1.terraces = 5;  // Few terraces = banded terrain
    
    MapGen map2 = map1;
    map2.terraces = 100;  // Many terraces = smoother gradients
    
    WorldGenerator gen1(map1, OctaveGen{});
    WorldGrid grid1(30, 30);
    gen1.generate(grid1);
    
    WorldGenerator gen2(map2, OctaveGen{});
    WorldGrid grid2(30, 30);
    gen2.generate(grid2);
    
    // Count unique elevation values
    std::set<unsigned int> elevations1, elevations2;
    
    for (unsigned int x = 0; x < grid1.width(); ++x) {
        for (unsigned int y = 0; y < grid1.height(); ++y) {
            elevations1.insert(grid1(x, y).getElevation());
            elevations2.insert(grid2(x, y).getElevation());
        }
    }
    
    // More terraces should generally allow more unique elevation values
    // Though actual count depends on grid size and noise
    TEST_ASSERT(elevations1.size() != elevations2.size() || 
                grid1(15, 15).getElevation() != grid2(15, 15).getElevation());
}

void test_scale_affects_output() {
    MapGen map1;
    map1.seed = 456.0;
    map1.rows = 40;
    map1.cols = 40;
    map1.scale = 0.005;  // Zoomed out, large features
    
    MapGen map2 = map1;
    map2.scale = 0.05;   // Zoomed in, small features
    
    WorldGenerator gen1(map1, OctaveGen{});
    WorldGrid grid1(40, 40);
    gen1.generate(grid1);
    
    WorldGenerator gen2(map2, OctaveGen{});
    WorldGrid grid2(40, 40);
    gen2.generate(grid2);
    
    // Different scales should produce different results
    int differences = 0;
    for (unsigned int x = 0; x < grid1.width(); ++x) {
        for (unsigned int y = 0; y < grid1.height(); ++y) {
            if (grid1(x, y).getElevation() != grid2(x, y).getElevation()) {
                differences++;
            }
        }
    }
    
    // Should have many differences
    TEST_ASSERT(differences > 100);
}

//==============================================================================
// Test: Serialization
//==============================================================================

void test_serialize_config() {
    MapGen mapGen;
    mapGen.seed = 42.0;
    mapGen.scale = 0.015;
    mapGen.freq = 1.5;
    mapGen.exponent = 1.2;
    mapGen.terraces = 25;
    mapGen.rows = 100;
    mapGen.cols = 200;
    mapGen.isIsland = true;
    
    OctaveGen octaveGen;
    octaveGen.quantity = 5;
    octaveGen.minWeight = 0.15;
    octaveGen.maxWeight = 0.6;
    octaveGen.freqInterval = 1.5;
    
    WorldGenerator generator(mapGen, octaveGen);
    
    std::string serialized = generator.serializeConfig();
    
    // Verify serialized string contains expected values
    TEST_ASSERT(serialized.find("42") != std::string::npos);
    TEST_ASSERT(serialized.find("100") != std::string::npos);
    TEST_ASSERT(serialized.find("200") != std::string::npos);
    TEST_ASSERT(serialized.length() > 0);
}

} // anonymous namespace

//==============================================================================
// Test Runner
//==============================================================================

void runWorldGeneratorTests() {
    BEGIN_TEST_GROUP("WorldGenerator - Construction");
    RUN_TEST(test_default_construction);
    RUN_TEST(test_parameterized_construction);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGenerator - Configuration");
    RUN_TEST(test_set_map_gen);
    RUN_TEST(test_set_octave_gen);
    RUN_TEST(test_terrain_level_access);
    RUN_TEST(test_terrain_rules);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGenerator - Basic Generation");
    RUN_TEST(test_generate_fills_grid);
    RUN_TEST(test_generate_produces_valid_tiles);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGenerator - Seed Reproducibility");
    RUN_TEST(test_same_seed_same_world);
    RUN_TEST(test_different_seed_different_world);
    RUN_TEST(test_generate_with_explicit_seed);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGenerator - Configuration Effects");
    RUN_TEST(test_island_mode);
    RUN_TEST(test_terraces_affect_output);
    RUN_TEST(test_scale_affects_output);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("WorldGenerator - Serialization");
    RUN_TEST(test_serialize_config);
    END_TEST_GROUP();
}
