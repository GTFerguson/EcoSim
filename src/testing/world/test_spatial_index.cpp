/**
 * @file test_spatial_index.cpp
 * @brief Comprehensive unit tests for SpatialIndex
 * 
 * Tests cover:
 * - Basic insert/remove operations
 * - Radius queries with boundary precision
 * - Sub-tile position accuracy (fractional coordinates)
 * - Cell boundary edge cases
 * - Position update correctness
 * - findNearest accuracy
 * - Empty cell handling
 */

#include "world/SpatialIndex.hpp"
#include "objects/creature/creature.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "../genetics/test_framework.hpp"

#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>

using namespace EcoSim;
using namespace EcoSim::Testing;

namespace {

// Factory instance for creating test creatures
std::unique_ptr<EcoSim::Genetics::CreatureFactory> g_testFactory;

// Helper to create a creature at a specific position for testing
std::unique_ptr<Creature> createTestCreature(float x, float y) {
    // Ensure registry is initialized
    Creature::initializeGeneRegistry();
    
    // Lazy-initialize the factory
    if (!g_testFactory) {
        auto& registry = Creature::getGeneRegistry();
        g_testFactory = std::make_unique<EcoSim::Genetics::CreatureFactory>(
            std::shared_ptr<EcoSim::Genetics::GeneRegistry>(&registry, [](auto*){}));
        g_testFactory->registerDefaultTemplates();
    }
    
    // Create a fleet runner (herbivore) - lightweight for testing
    auto creature = std::make_unique<Creature>(
        g_testFactory->createFleetRunner(static_cast<int>(x), static_cast<int>(y)));
    
    // Set precise world position
    creature->setWorldPosition(x, y);
    
    return creature;
}

// Helper to calculate distance between two points
float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

//==============================================================================
// Basic Operations Tests
//==============================================================================

void test_constructor() {
    SpatialIndex index(500, 500, 32);
    
    TEST_ASSERT(index.empty());
    TEST_ASSERT_EQ(0u, index.size());
    TEST_ASSERT_EQ(32, index.getCellSize());
}

void test_insert_single() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(50.0f, 50.0f);
    
    index.insert(creature.get());
    
    TEST_ASSERT(!index.empty());
    TEST_ASSERT_EQ(1u, index.size());
}

void test_insert_multiple() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    for (int i = 0; i < 10; ++i) {
        creatures.push_back(createTestCreature(i * 10.0f, i * 10.0f));
        index.insert(creatures.back().get());
    }
    
    TEST_ASSERT_EQ(10u, index.size());
}

void test_remove() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(50.0f, 50.0f);
    
    index.insert(creature.get());
    TEST_ASSERT_EQ(1u, index.size());
    
    index.remove(creature.get());
    TEST_ASSERT(index.empty());
    TEST_ASSERT_EQ(0u, index.size());
}

void test_clear() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    for (int i = 0; i < 5; ++i) {
        creatures.push_back(createTestCreature(i * 20.0f, i * 20.0f));
        index.insert(creatures.back().get());
    }
    
    TEST_ASSERT_EQ(5u, index.size());
    
    index.clear();
    TEST_ASSERT(index.empty());
}

void test_null_handling() {
    SpatialIndex index(100, 100, 10);
    
    // Should not crash on null
    index.insert(nullptr);
    TEST_ASSERT(index.empty());
    
    index.remove(nullptr);
    TEST_ASSERT(index.empty());
    
    index.update(nullptr, 0.0f, 0.0f);
    TEST_ASSERT(index.empty());
}

//==============================================================================
// Cell Coordinate Tests
//==============================================================================

void test_getCellCoords_basic() {
    SpatialIndex index(100, 100, 10);
    
    auto [cellX, cellY] = index.getCellCoords(5.0f, 5.0f);
    TEST_ASSERT_EQ(0, cellX);
    TEST_ASSERT_EQ(0, cellY);
    
    auto [cellX2, cellY2] = index.getCellCoords(15.0f, 25.0f);
    TEST_ASSERT_EQ(1, cellX2);
    TEST_ASSERT_EQ(2, cellY2);
}

void test_getCellCoords_boundaries() {
    SpatialIndex index(100, 100, 10);
    
    // Exactly on cell boundary (should go to the cell containing that boundary)
    auto [cellX, cellY] = index.getCellCoords(10.0f, 10.0f);
    TEST_ASSERT_EQ(1, cellX);
    TEST_ASSERT_EQ(1, cellY);
    
    // Just before boundary
    auto [cellX2, cellY2] = index.getCellCoords(9.999f, 9.999f);
    TEST_ASSERT_EQ(0, cellX2);
    TEST_ASSERT_EQ(0, cellY2);
}

void test_getCellCoords_clamping() {
    SpatialIndex index(100, 100, 10);
    
    // Negative coordinates should clamp to 0
    auto [cellX, cellY] = index.getCellCoords(-5.0f, -10.0f);
    TEST_ASSERT_EQ(0, cellX);
    TEST_ASSERT_EQ(0, cellY);
    
    // Coordinates beyond world should clamp to max cell
    auto [cellX2, cellY2] = index.getCellCoords(150.0f, 200.0f);
    TEST_ASSERT_EQ(9, cellX2);  // 100/10 - 1 = 9
    TEST_ASSERT_EQ(9, cellY2);
}

//==============================================================================
// Radius Query Tests - Basic
//==============================================================================

void test_queryRadius_empty() {
    SpatialIndex index(100, 100, 10);
    
    auto results = index.queryRadius(50.0f, 50.0f, 20.0f);
    TEST_ASSERT(results.empty());
}

void test_queryRadius_single_inside() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(50.0f, 50.0f);
    index.insert(creature.get());
    
    // Query centered on creature
    auto results = index.queryRadius(50.0f, 50.0f, 10.0f);
    TEST_ASSERT_EQ(1u, results.size());
    TEST_ASSERT_EQ(creature.get(), results[0]);
}

void test_queryRadius_single_outside() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(50.0f, 50.0f);
    index.insert(creature.get());
    
    // Query far from creature
    auto results = index.queryRadius(10.0f, 10.0f, 5.0f);
    TEST_ASSERT(results.empty());
}

void test_queryRadius_multiple() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    // Create 4 creatures in a 2x2 pattern around center
    creatures.push_back(createTestCreature(48.0f, 48.0f));
    creatures.push_back(createTestCreature(52.0f, 48.0f));
    creatures.push_back(createTestCreature(48.0f, 52.0f));
    creatures.push_back(createTestCreature(52.0f, 52.0f));
    
    for (auto& c : creatures) {
        index.insert(c.get());
    }
    
    // Query should find all 4
    auto results = index.queryRadius(50.0f, 50.0f, 10.0f);
    TEST_ASSERT_EQ(4u, results.size());
}

//==============================================================================
// Radius Query Tests - Boundary Precision (Critical for sight range!)
//==============================================================================

void test_queryRadius_exact_boundary_included() {
    SpatialIndex index(100, 100, 10);
    
    // Creature exactly at radius distance should be INCLUDED
    float radius = 10.0f;
    float creatureX = 50.0f + radius;  // Exactly 10 units away
    float creatureY = 50.0f;
    
    auto creature = createTestCreature(creatureX, creatureY);
    index.insert(creature.get());
    
    auto results = index.queryRadius(50.0f, 50.0f, radius);
    TEST_ASSERT_MSG(results.size() == 1, 
        "Creature exactly at radius boundary should be included");
}

void test_queryRadius_just_outside_excluded() {
    SpatialIndex index(100, 100, 10);
    
    // Creature just beyond radius should be EXCLUDED
    float radius = 10.0f;
    float epsilon = 0.001f;
    float creatureX = 50.0f + radius + epsilon;
    float creatureY = 50.0f;
    
    auto creature = createTestCreature(creatureX, creatureY);
    index.insert(creature.get());
    
    auto results = index.queryRadius(50.0f, 50.0f, radius);
    TEST_ASSERT_MSG(results.empty(), 
        "Creature just outside radius should be excluded");
}

void test_queryRadius_diagonal_boundary() {
    SpatialIndex index(100, 100, 10);
    
    // Test diagonal - creature at exactly radius distance diagonally
    float radius = 10.0f;
    // At 45 degrees, x = y = radius / sqrt(2)
    float offset = radius / std::sqrt(2.0f);
    
    auto creature = createTestCreature(50.0f + offset, 50.0f + offset);
    index.insert(creature.get());
    
    float actualDist = distance(50.0f, 50.0f, 
                                creature->getWorldX(), creature->getWorldY());
    
    auto results = index.queryRadius(50.0f, 50.0f, radius);
    TEST_ASSERT_MSG(results.size() == 1,
        "Creature at diagonal boundary should be included");
}

void test_queryRadius_sub_tile_precision() {
    SpatialIndex index(100, 100, 10);
    
    // Test that sub-tile positions are correctly handled
    // Two creatures within same tile but different distances
    auto nearCreature = createTestCreature(50.3f, 50.0f);
    auto farCreature = createTestCreature(50.8f, 50.0f);
    
    index.insert(nearCreature.get());
    index.insert(farCreature.get());
    
    // Radius that includes near but not far
    float radius = 0.5f;
    auto results = index.queryRadius(50.0f, 50.0f, radius);
    
    TEST_ASSERT_EQ(1u, results.size());
    TEST_ASSERT_MSG(results[0] == nearCreature.get(),
        "Should only find creature at 0.3 units, not at 0.8 units");
}

void test_queryRadius_very_small_radius() {
    SpatialIndex index(100, 100, 10);
    
    auto creature = createTestCreature(50.0f, 50.0f);
    index.insert(creature.get());
    
    // Very small radius - should still find creature at exact position
    auto results = index.queryRadius(50.0f, 50.0f, 0.01f);
    TEST_ASSERT_EQ(1u, results.size());
    
    // But not if we query slightly offset
    auto results2 = index.queryRadius(50.1f, 50.1f, 0.01f);
    TEST_ASSERT(results2.empty());
}

void test_queryRadius_zero_radius() {
    SpatialIndex index(100, 100, 10);
    
    auto creature = createTestCreature(50.0f, 50.0f);
    index.insert(creature.get());
    
    // Zero radius should return empty
    auto results = index.queryRadius(50.0f, 50.0f, 0.0f);
    TEST_ASSERT(results.empty());
}

void test_queryRadius_negative_radius() {
    SpatialIndex index(100, 100, 10);
    
    auto creature = createTestCreature(50.0f, 50.0f);
    index.insert(creature.get());
    
    // Negative radius should return empty
    auto results = index.queryRadius(50.0f, 50.0f, -10.0f);
    TEST_ASSERT(results.empty());
}

//==============================================================================
// Vision Range Simulation Tests
//==============================================================================

void test_sight_range_simulation() {
    SpatialIndex index(500, 500, 32);
    
    // Simulate typical sight range scenario
    float sightRange = 100.0f;  // Typical sight range
    float observerX = 250.0f;
    float observerY = 250.0f;
    
    std::vector<std::unique_ptr<Creature>> creatures;
    
    // Place creatures at various distances
    std::vector<std::pair<float, bool>> testCases = {
        {50.0f, true},    // Well within range
        {99.0f, true},    // Just inside
        {99.99f, true},   // Very close to boundary
        {100.0f, true},   // Exactly at boundary
        {100.01f, false}, // Just outside
        {150.0f, false},  // Well outside
    };
    
    for (auto& [dist, shouldSee] : testCases) {
        creatures.push_back(createTestCreature(observerX + dist, observerY));
        index.insert(creatures.back().get());
    }
    
    auto visible = index.queryRadius(observerX, observerY, sightRange);
    
    // Verify correct count
    int expectedVisible = 0;
    for (auto& [dist, shouldSee] : testCases) {
        if (shouldSee) expectedVisible++;
    }
    
    TEST_ASSERT_EQ(static_cast<size_t>(expectedVisible), visible.size());
}

void test_sight_across_cell_boundaries() {
    // Cell size is 32, sight range is 100
    // This means creatures in cells up to 4 away could be visible
    SpatialIndex index(500, 500, 32);
    
    float observerX = 100.0f;  // In cell (3, 3) 
    float observerY = 100.0f;
    float sightRange = 100.0f;
    
    std::vector<std::unique_ptr<Creature>> creatures;
    
    // Creature in same cell
    creatures.push_back(createTestCreature(110.0f, 110.0f));
    
    // Creature in adjacent cell (should be visible)
    creatures.push_back(createTestCreature(140.0f, 100.0f));
    
    // Creature 3 cells away but within sight range
    creatures.push_back(createTestCreature(190.0f, 100.0f));  // 90 units away
    
    // Creature 4 cells away, just at boundary
    creatures.push_back(createTestCreature(200.0f, 100.0f));  // 100 units away
    
    // Creature beyond sight range
    creatures.push_back(createTestCreature(250.0f, 100.0f));  // 150 units away
    
    for (auto& c : creatures) {
        index.insert(c.get());
    }
    
    auto visible = index.queryRadius(observerX, observerY, sightRange);
    
    // Should see 4 creatures (all except the one at 150 units)
    TEST_ASSERT_EQ(4u, visible.size());
}

//==============================================================================
// Cell Query Tests
//==============================================================================

void test_queryCell_basic() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(5.0f, 5.0f);  // In cell (0, 0)
    index.insert(creature.get());
    
    auto results = index.queryCell(0, 0);
    TEST_ASSERT_EQ(1u, results.size());
    
    auto results2 = index.queryCell(1, 1);
    TEST_ASSERT(results2.empty());
}

void test_queryNearbyCells() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    // Create creatures in a 3x3 cell area
    for (int cy = 0; cy < 3; ++cy) {
        for (int cx = 0; cx < 3; ++cx) {
            float x = cx * 10.0f + 5.0f;
            float y = cy * 10.0f + 5.0f;
            creatures.push_back(createTestCreature(x, y));
            index.insert(creatures.back().get());
        }
    }
    
    // Query from center cell
    auto results = index.queryNearbyCells(15.0f, 15.0f);
    TEST_ASSERT_EQ(9u, results.size());
}

//==============================================================================
// Position Update Tests
//==============================================================================

void test_update_same_cell() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(5.0f, 5.0f);
    index.insert(creature.get());
    
    // Move within same cell
    float oldX = creature->getWorldX();
    float oldY = creature->getWorldY();
    creature->setWorldPosition(8.0f, 8.0f);
    
    index.update(creature.get(), oldX, oldY);
    
    // Should still be findable
    auto results = index.queryRadius(8.0f, 8.0f, 1.0f);
    TEST_ASSERT_EQ(1u, results.size());
    TEST_ASSERT_EQ(1u, index.size());
}

void test_update_different_cell() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(5.0f, 5.0f);  // Cell (0, 0)
    index.insert(creature.get());
    
    // Move to different cell
    float oldX = creature->getWorldX();
    float oldY = creature->getWorldY();
    creature->setWorldPosition(15.0f, 15.0f);  // Cell (1, 1)
    
    index.update(creature.get(), oldX, oldY);
    
    // Should not be in old cell
    auto oldCellResults = index.queryCell(0, 0);
    TEST_ASSERT(oldCellResults.empty());
    
    // Should be in new cell
    auto newCellResults = index.queryCell(1, 1);
    TEST_ASSERT_EQ(1u, newCellResults.size());
    
    // Total count unchanged
    TEST_ASSERT_EQ(1u, index.size());
}

void test_update_to_boundary() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(5.0f, 5.0f);
    index.insert(creature.get());
    
    // Move exactly to cell boundary
    float oldX = creature->getWorldX();
    float oldY = creature->getWorldY();
    creature->setWorldPosition(10.0f, 10.0f);  // On boundary, should be in cell (1,1)
    
    index.update(creature.get(), oldX, oldY);
    
    auto results = index.queryCell(1, 1);
    TEST_ASSERT_EQ(1u, results.size());
}

//==============================================================================
// findNearest Tests
//==============================================================================

void test_findNearest_basic() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    creatures.push_back(createTestCreature(55.0f, 50.0f));  // 5 units away
    creatures.push_back(createTestCreature(60.0f, 50.0f));  // 10 units away
    creatures.push_back(createTestCreature(70.0f, 50.0f));  // 20 units away
    
    for (auto& c : creatures) {
        index.insert(c.get());
    }
    
    auto nearest = index.findNearest(50.0f, 50.0f, 100.0f, 
        [](const Creature*) { return true; });
    
    TEST_ASSERT(nearest != nullptr);
    TEST_ASSERT_EQ(creatures[0].get(), nearest);
}

void test_findNearest_with_predicate() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    creatures.push_back(createTestCreature(55.0f, 50.0f));  // Closest
    creatures.push_back(createTestCreature(60.0f, 50.0f));  // Second closest
    creatures.push_back(createTestCreature(70.0f, 50.0f));  // Third closest
    
    for (auto& c : creatures) {
        index.insert(c.get());
    }
    
    // Find nearest that is NOT the closest one
    auto nearest = index.findNearest(50.0f, 50.0f, 100.0f, 
        [&](const Creature* c) { return c != creatures[0].get(); });
    
    TEST_ASSERT(nearest != nullptr);
    TEST_ASSERT_EQ(creatures[1].get(), nearest);
}

void test_findNearest_none_matching() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(55.0f, 50.0f);
    index.insert(creature.get());
    
    // Predicate that matches nothing
    auto nearest = index.findNearest(50.0f, 50.0f, 100.0f, 
        [](const Creature*) { return false; });
    
    TEST_ASSERT(nearest == nullptr);
}

void test_findNearest_beyond_radius() {
    SpatialIndex index(100, 100, 10);
    auto creature = createTestCreature(80.0f, 50.0f);  // 30 units away
    index.insert(creature.get());
    
    // Search with small radius
    auto nearest = index.findNearest(50.0f, 50.0f, 10.0f, 
        [](const Creature*) { return true; });
    
    TEST_ASSERT(nearest == nullptr);
}

void test_findNearest_boundary_precision() {
    SpatialIndex index(100, 100, 10);
    
    // Creature exactly at max radius
    float maxRadius = 20.0f;
    auto creature = createTestCreature(50.0f + maxRadius, 50.0f);
    index.insert(creature.get());
    
    auto nearest = index.findNearest(50.0f, 50.0f, maxRadius, 
        [](const Creature*) { return true; });
    
    TEST_ASSERT_MSG(nearest != nullptr,
        "Creature exactly at max radius should be found");
}

//==============================================================================
// queryWithFilter Tests
//==============================================================================

void test_queryWithFilter_basic() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    for (int i = 0; i < 5; ++i) {
        creatures.push_back(createTestCreature(50.0f + i * 2.0f, 50.0f));
        index.insert(creatures.back().get());
    }
    
    // Filter to find only specific creatures
    auto results = index.queryWithFilter(50.0f, 50.0f, 20.0f,
        [&](const Creature* c) { 
            return c == creatures[0].get() || c == creatures[2].get(); 
        });
    
    TEST_ASSERT_EQ(2u, results.size());
}

//==============================================================================
// Edge Cases and Stress Tests
//==============================================================================

void test_world_corner_positions() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    // Corners
    creatures.push_back(createTestCreature(0.0f, 0.0f));
    creatures.push_back(createTestCreature(99.0f, 0.0f));
    creatures.push_back(createTestCreature(0.0f, 99.0f));
    creatures.push_back(createTestCreature(99.0f, 99.0f));
    
    for (auto& c : creatures) {
        index.insert(c.get());
    }
    
    TEST_ASSERT_EQ(4u, index.size());
    
    // Query from corner
    auto results = index.queryRadius(0.0f, 0.0f, 5.0f);
    TEST_ASSERT_EQ(1u, results.size());
}

void test_many_creatures_same_cell() {
    SpatialIndex index(100, 100, 10);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    // 100 creatures in same cell
    for (int i = 0; i < 100; ++i) {
        float x = 5.0f + (i % 10) * 0.1f;
        float y = 5.0f + (i / 10) * 0.1f;
        creatures.push_back(createTestCreature(x, y));
        index.insert(creatures.back().get());
    }
    
    TEST_ASSERT_EQ(100u, index.size());
    
    auto results = index.queryCell(0, 0);
    TEST_ASSERT_EQ(100u, results.size());
}

void test_rebuild() {
    SpatialIndex index(100, 100, 10);
    std::vector<Creature> creatures;
    creatures.reserve(10);
    
    // Initialize gene registry and factory
    Creature::initializeGeneRegistry();
    if (!g_testFactory) {
        auto& registry = Creature::getGeneRegistry();
        g_testFactory = std::make_unique<EcoSim::Genetics::CreatureFactory>(
            std::shared_ptr<EcoSim::Genetics::GeneRegistry>(&registry, [](auto*){}));
        g_testFactory->registerDefaultTemplates();
    }
    
    // Create creatures using factory
    for (int i = 0; i < 10; ++i) {
        creatures.push_back(g_testFactory->createFleetRunner(i * 10, i * 10));
    }
    
    // Rebuild index from vector
    index.rebuild(creatures);
    
    TEST_ASSERT_EQ(10u, index.size());
}

void test_large_radius_query() {
    SpatialIndex index(500, 500, 32);
    std::vector<std::unique_ptr<Creature>> creatures;
    
    // Scatter creatures across world
    for (int i = 0; i < 50; ++i) {
        float x = (i % 10) * 50.0f + 25.0f;
        float y = (i / 10) * 100.0f + 25.0f;
        creatures.push_back(createTestCreature(x, y));
        index.insert(creatures.back().get());
    }
    
    // Large radius query from center
    auto results = index.queryRadius(250.0f, 250.0f, 200.0f);
    
    // Should find multiple creatures
    TEST_ASSERT(!results.empty());
    
    // Verify all returned creatures are actually within radius
    for (Creature* c : results) {
        float dist = distance(250.0f, 250.0f, c->getWorldX(), c->getWorldY());
        TEST_ASSERT_LE(dist, 200.0f);
    }
}

} // anonymous namespace

//==============================================================================
// Test Runner
//==============================================================================

void runSpatialIndexTests() {
    BEGIN_TEST_GROUP("SpatialIndex - Basic Operations");
    RUN_TEST(test_constructor);
    RUN_TEST(test_insert_single);
    RUN_TEST(test_insert_multiple);
    RUN_TEST(test_remove);
    RUN_TEST(test_clear);
    RUN_TEST(test_null_handling);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - Cell Coordinates");
    RUN_TEST(test_getCellCoords_basic);
    RUN_TEST(test_getCellCoords_boundaries);
    RUN_TEST(test_getCellCoords_clamping);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - Radius Queries");
    RUN_TEST(test_queryRadius_empty);
    RUN_TEST(test_queryRadius_single_inside);
    RUN_TEST(test_queryRadius_single_outside);
    RUN_TEST(test_queryRadius_multiple);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - Boundary Precision");
    RUN_TEST(test_queryRadius_exact_boundary_included);
    RUN_TEST(test_queryRadius_just_outside_excluded);
    RUN_TEST(test_queryRadius_diagonal_boundary);
    RUN_TEST(test_queryRadius_sub_tile_precision);
    RUN_TEST(test_queryRadius_very_small_radius);
    RUN_TEST(test_queryRadius_zero_radius);
    RUN_TEST(test_queryRadius_negative_radius);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - Vision Simulation");
    RUN_TEST(test_sight_range_simulation);
    RUN_TEST(test_sight_across_cell_boundaries);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - Cell Queries");
    RUN_TEST(test_queryCell_basic);
    RUN_TEST(test_queryNearbyCells);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - Position Updates");
    RUN_TEST(test_update_same_cell);
    RUN_TEST(test_update_different_cell);
    RUN_TEST(test_update_to_boundary);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - findNearest");
    RUN_TEST(test_findNearest_basic);
    RUN_TEST(test_findNearest_with_predicate);
    RUN_TEST(test_findNearest_none_matching);
    RUN_TEST(test_findNearest_beyond_radius);
    RUN_TEST(test_findNearest_boundary_precision);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - queryWithFilter");
    RUN_TEST(test_queryWithFilter_basic);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SpatialIndex - Edge Cases");
    RUN_TEST(test_world_corner_positions);
    RUN_TEST(test_many_creatures_same_cell);
    RUN_TEST(test_rebuild);
    RUN_TEST(test_large_radius_query);
    END_TEST_GROUP();
}
