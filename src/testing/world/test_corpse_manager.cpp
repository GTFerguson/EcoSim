/**
 * @file test_corpse_manager.cpp
 * @brief Unit tests for CorpseManager component
 */

#include "world/CorpseManager.hpp"
#include "../genetics/test_framework.hpp"

using namespace EcoSim;
using namespace EcoSim::Testing;

namespace {

//==============================================================================
// Test: Construction
//==============================================================================

void test_default_construction() {
    CorpseManager manager;
    
    TEST_ASSERT_EQ(manager.count(), 0u);
    TEST_ASSERT(manager.empty());
}

void test_construction_with_max_corpses() {
    CorpseManager manager(50);
    
    TEST_ASSERT_EQ(manager.count(), 0u);
    TEST_ASSERT(manager.empty());
}

//==============================================================================
// Test: Adding Corpses
//==============================================================================

void test_add_single_corpse() {
    CorpseManager manager;
    
    manager.addCorpse(10.5f, 20.5f, 2.0f, "TestSpecies", 0.5f);
    
    TEST_ASSERT_EQ(manager.count(), 1u);
    TEST_ASSERT(!manager.empty());
}

void test_add_multiple_corpses() {
    CorpseManager manager;
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "Species1");
    manager.addCorpse(20.0f, 20.0f, 2.0f, "Species2");
    manager.addCorpse(30.0f, 30.0f, 3.0f, "Species3");
    
    TEST_ASSERT_EQ(manager.count(), 3u);
}

void test_add_corpse_replaces_most_decayed_at_capacity() {
    CorpseManager manager(3);  // Small capacity for testing
    
    // Add 3 corpses
    manager.addCorpse(10.0f, 10.0f, 1.0f, "Fresh1");
    manager.addCorpse(20.0f, 20.0f, 1.0f, "Fresh2");
    manager.addCorpse(30.0f, 30.0f, 1.0f, "Fresh3");
    
    TEST_ASSERT_EQ(manager.count(), 3u);
    
    // Decay the first one more by ticking many times
    for (int i = 0; i < 500; ++i) {
        // Only tick decay on the first corpse manually
        auto& corpses = manager.getAll();
        if (!corpses.empty()) {
            const_cast<world::Corpse*>(corpses[0].get())->tick();
        }
    }
    
    // Add a 4th corpse - should replace the most decayed
    manager.addCorpse(40.0f, 40.0f, 1.0f, "New");
    
    // Still 3 corpses
    TEST_ASSERT_EQ(manager.count(), 3u);
}

//==============================================================================
// Test: Decay Processing
//==============================================================================

void test_tick_advances_decay() {
    CorpseManager manager;
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "Test");
    
    float initialDecay = manager.getAll()[0]->getDecayProgress();
    
    // Tick a few times
    for (int i = 0; i < 100; ++i) {
        manager.tick();
    }
    
    float afterDecay = manager.getAll()[0]->getDecayProgress();
    
    TEST_ASSERT(afterDecay > initialDecay);
}

void test_tick_removes_fully_decayed() {
    CorpseManager manager;
    
    // Add a very small corpse (decays quickly: DECAY_TIME_PER_SIZE * 0.1 = 100 ticks)
    manager.addCorpse(10.0f, 10.0f, 0.1f, "TinyCorpse");
    
    TEST_ASSERT_EQ(manager.count(), 1u);
    
    // Tick until it's gone
    for (int i = 0; i < 200; ++i) {
        manager.tick();
    }
    
    TEST_ASSERT(manager.empty());
}

void test_removeExpiredCorpses() {
    CorpseManager manager;
    
    // Add a very small corpse
    manager.addCorpse(10.0f, 10.0f, 0.1f, "TinyCorpse");
    
    // Manually tick decay without calling removeExpiredCorpses
    auto& corpses = manager.getAll();
    for (int i = 0; i < 200; ++i) {
        if (!corpses.empty()) {
            const_cast<world::Corpse*>(corpses[0].get())->tick();
        }
    }
    
    // Should still have 1 corpse (not removed yet)
    TEST_ASSERT_EQ(manager.count(), 1u);
    
    // Now remove expired
    manager.removeExpiredCorpses();
    
    TEST_ASSERT(manager.empty());
}

//==============================================================================
// Test: Corpse Removal
//==============================================================================

void test_remove_specific_corpse() {
    CorpseManager manager;
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "A");
    manager.addCorpse(20.0f, 20.0f, 1.0f, "B");
    manager.addCorpse(30.0f, 30.0f, 1.0f, "C");
    
    // Get pointer to middle corpse
    world::Corpse* middleCorpse = const_cast<world::Corpse*>(manager.getAll()[1].get());
    
    manager.removeCorpse(middleCorpse);
    
    TEST_ASSERT_EQ(manager.count(), 2u);
}

void test_remove_nonexistent_corpse_safe() {
    CorpseManager manager;
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "A");
    
    // Try to remove nullptr (should be safe)
    manager.removeCorpse(nullptr);
    
    TEST_ASSERT_EQ(manager.count(), 1u);
}

void test_clear() {
    CorpseManager manager;
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "A");
    manager.addCorpse(20.0f, 20.0f, 1.0f, "B");
    
    manager.clear();
    
    TEST_ASSERT(manager.empty());
}

//==============================================================================
// Test: Spatial Queries
//==============================================================================

void test_getCorpsesAt() {
    CorpseManager manager;
    
    // Add corpses at different tile positions
    manager.addCorpse(5.5f, 5.5f, 1.0f, "At5");   // Tile (5, 5)
    manager.addCorpse(5.9f, 5.1f, 1.0f, "At5b");  // Also Tile (5, 5)
    manager.addCorpse(10.0f, 10.0f, 1.0f, "At10"); // Tile (10, 10)
    
    auto corpsesAt5 = manager.getCorpsesAt(5, 5);
    auto corpsesAt10 = manager.getCorpsesAt(10, 10);
    auto corpsesAt0 = manager.getCorpsesAt(0, 0);
    
    TEST_ASSERT_EQ(corpsesAt5.size(), 2u);
    TEST_ASSERT_EQ(corpsesAt10.size(), 1u);
    TEST_ASSERT_EQ(corpsesAt0.size(), 0u);
}

void test_getCorpsesAt_const() {
    CorpseManager manager;
    
    manager.addCorpse(5.5f, 5.5f, 1.0f, "At5");
    
    const CorpseManager& constManager = manager;
    auto corpses = constManager.getCorpsesAt(5, 5);
    
    TEST_ASSERT_EQ(corpses.size(), 1u);
}

void test_getCorpsesInRadius() {
    CorpseManager manager;
    
    // Add corpses at various distances from origin
    manager.addCorpse(1.0f, 0.0f, 1.0f, "Close1");   // Distance 1.0
    manager.addCorpse(0.0f, 2.0f, 1.0f, "Close2");   // Distance 2.0
    manager.addCorpse(3.0f, 4.0f, 1.0f, "Medium");   // Distance 5.0
    manager.addCorpse(10.0f, 10.0f, 1.0f, "Far");    // Distance ~14.14
    
    auto inRadius3 = manager.getCorpsesInRadius(0.0f, 0.0f, 3.0f);
    auto inRadius6 = manager.getCorpsesInRadius(0.0f, 0.0f, 6.0f);
    auto inRadius1 = manager.getCorpsesInRadius(0.0f, 0.0f, 1.0f);
    
    TEST_ASSERT_EQ(inRadius1.size(), 1u);  // Only Close1
    TEST_ASSERT_EQ(inRadius3.size(), 2u);  // Close1 and Close2
    TEST_ASSERT_EQ(inRadius6.size(), 3u);  // Close1, Close2, and Medium
}

void test_getCorpsesInRadius_const() {
    CorpseManager manager;
    
    manager.addCorpse(1.0f, 0.0f, 1.0f, "Close");
    
    const CorpseManager& constManager = manager;
    auto corpses = constManager.getCorpsesInRadius(0.0f, 0.0f, 5.0f);
    
    TEST_ASSERT_EQ(corpses.size(), 1u);
}

void test_findNearest() {
    CorpseManager manager;
    
    manager.addCorpse(5.0f, 0.0f, 1.0f, "FarCorpse");
    manager.addCorpse(1.0f, 0.0f, 1.0f, "NearCorpse");
    manager.addCorpse(3.0f, 0.0f, 1.0f, "MidCorpse");
    
    world::Corpse* nearest = manager.findNearest(0.0f, 0.0f, 10.0f);
    
    TEST_ASSERT(nearest != nullptr);
    TEST_ASSERT_EQ(nearest->getSpeciesName(), "NearCorpse");
}

void test_findNearest_no_corpses_in_range() {
    CorpseManager manager;
    
    manager.addCorpse(100.0f, 100.0f, 1.0f, "FarAway");
    
    world::Corpse* nearest = manager.findNearest(0.0f, 0.0f, 5.0f);
    
    TEST_ASSERT(nearest == nullptr);
}

void test_findNearest_skips_exhausted() {
    CorpseManager manager;
    
    // Add two corpses
    manager.addCorpse(1.0f, 0.0f, 0.5f, "Near");   // Closer but will be exhausted
    manager.addCorpse(5.0f, 0.0f, 0.5f, "Far");    // Farther but has nutrition
    
    // Exhaust the nearer corpse
    world::Corpse* nearCorpse = manager.findNearest(0.0f, 0.0f, 10.0f);
    if (nearCorpse) {
        // Extract all nutrition
        while (!nearCorpse->isExhausted()) {
            nearCorpse->extractNutrition(10.0f);
        }
    }
    
    // Now findNearest should skip the exhausted one
    world::Corpse* nearest = manager.findNearest(0.0f, 0.0f, 10.0f);
    
    TEST_ASSERT(nearest != nullptr);
    TEST_ASSERT_EQ(nearest->getSpeciesName(), "Far");
}

void test_findNearest_const() {
    CorpseManager manager;
    
    manager.addCorpse(1.0f, 0.0f, 1.0f, "Corpse");
    
    const CorpseManager& constManager = manager;
    const world::Corpse* nearest = constManager.findNearest(0.0f, 0.0f, 10.0f);
    
    TEST_ASSERT(nearest != nullptr);
}

void test_getAll() {
    CorpseManager manager;
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "A");
    manager.addCorpse(20.0f, 20.0f, 1.0f, "B");
    
    const auto& all = manager.getAll();
    
    TEST_ASSERT_EQ(all.size(), 2u);
}

//==============================================================================
// Test: Statistics
//==============================================================================

void test_count() {
    CorpseManager manager;
    
    TEST_ASSERT_EQ(manager.count(), 0u);
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "A");
    TEST_ASSERT_EQ(manager.count(), 1u);
    
    manager.addCorpse(20.0f, 20.0f, 1.0f, "B");
    TEST_ASSERT_EQ(manager.count(), 2u);
}

void test_empty() {
    CorpseManager manager;
    
    TEST_ASSERT(manager.empty());
    
    manager.addCorpse(10.0f, 10.0f, 1.0f, "A");
    
    TEST_ASSERT(!manager.empty());
    
    manager.clear();
    
    TEST_ASSERT(manager.empty());
}

void test_getTotalNutrition() {
    CorpseManager manager;
    
    // Initial total should be 0
    TEST_ASSERT(manager.getTotalNutrition() < 0.001f);
    
    // Add a corpse with size 2.0
    // Nutrition = NUTRITION_PER_SIZE * size * bodyCondition = 50 * 2.0 * 0.5 = 50
    manager.addCorpse(10.0f, 10.0f, 2.0f, "A", 0.5f);
    
    float total = manager.getTotalNutrition();
    TEST_ASSERT(total > 0.0f);
}

void test_getTotalNutritionAt() {
    CorpseManager manager;
    
    // Add corpses at different tiles
    manager.addCorpse(5.5f, 5.5f, 2.0f, "AtTile5", 0.5f);
    manager.addCorpse(10.0f, 10.0f, 2.0f, "AtTile10", 0.5f);
    
    float nutritionAt5 = manager.getTotalNutritionAt(5, 5);
    float nutritionAt10 = manager.getTotalNutritionAt(10, 10);
    float nutritionAt0 = manager.getTotalNutritionAt(0, 0);
    
    TEST_ASSERT(nutritionAt5 > 0.0f);
    TEST_ASSERT(nutritionAt10 > 0.0f);
    TEST_ASSERT(nutritionAt0 < 0.001f);  // No corpses at (0, 0)
}

} // anonymous namespace

//==============================================================================
// Test Runner
//==============================================================================

void runCorpseManagerTests() {
    BEGIN_TEST_GROUP("CorpseManager - Construction");
    RUN_TEST(test_default_construction);
    RUN_TEST(test_construction_with_max_corpses);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("CorpseManager - Adding Corpses");
    RUN_TEST(test_add_single_corpse);
    RUN_TEST(test_add_multiple_corpses);
    RUN_TEST(test_add_corpse_replaces_most_decayed_at_capacity);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("CorpseManager - Decay Processing");
    RUN_TEST(test_tick_advances_decay);
    RUN_TEST(test_tick_removes_fully_decayed);
    RUN_TEST(test_removeExpiredCorpses);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("CorpseManager - Corpse Removal");
    RUN_TEST(test_remove_specific_corpse);
    RUN_TEST(test_remove_nonexistent_corpse_safe);
    RUN_TEST(test_clear);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("CorpseManager - Spatial Queries");
    RUN_TEST(test_getCorpsesAt);
    RUN_TEST(test_getCorpsesAt_const);
    RUN_TEST(test_getCorpsesInRadius);
    RUN_TEST(test_getCorpsesInRadius_const);
    RUN_TEST(test_findNearest);
    RUN_TEST(test_findNearest_no_corpses_in_range);
    RUN_TEST(test_findNearest_skips_exhausted);
    RUN_TEST(test_findNearest_const);
    RUN_TEST(test_getAll);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("CorpseManager - Statistics");
    RUN_TEST(test_count);
    RUN_TEST(test_empty);
    RUN_TEST(test_getTotalNutrition);
    RUN_TEST(test_getTotalNutritionAt);
    END_TEST_GROUP();
}
