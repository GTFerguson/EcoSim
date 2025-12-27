/**
 * @file test_scent_layer.cpp
 * @brief Tests for the ScentLayer sensory system (Phase 1)
 * 
 * Tests scent deposition, retrieval, decay mechanics, genetic signature
 * computation, and sparse storage efficiency.
 */

#include "test_framework.hpp"
#include "world/ScentLayer.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneRegistry.hpp"

#include <iostream>
#include <chrono>
#include <random>

using namespace EcoSim;
using namespace EcoSim::Testing;

//================================================================================
//  Scent Deposition and Retrieval Tests
//================================================================================

void test_basic_deposit_and_retrieval() {
    ScentLayer layer(100, 100);
    
    ScentDeposit deposit;
    deposit.type = ScentType::MATE_SEEKING;
    deposit.creatureId = 42;
    deposit.intensity = 0.8f;
    deposit.signature = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    deposit.tickDeposited = 100;
    deposit.decayRate = 200;
    
    layer.deposit(50, 50, deposit);
    
    auto scents = layer.getScentsAt(50, 50);
    TEST_ASSERT_EQ(1, scents.size());
    TEST_ASSERT_EQ(ScentType::MATE_SEEKING, scents[0].type);
    TEST_ASSERT_EQ(42, scents[0].creatureId);
    TEST_ASSERT_NEAR(0.8f, scents[0].intensity, 0.001f);
}

void test_multiple_deposits_same_tile() {
    ScentLayer layer(100, 100);
    
    // Deposit from creature 1
    ScentDeposit deposit1;
    deposit1.type = ScentType::MATE_SEEKING;
    deposit1.creatureId = 1;
    deposit1.intensity = 0.6f;
    deposit1.signature = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
    deposit1.tickDeposited = 100;
    deposit1.decayRate = 100;
    
    // Deposit from creature 2
    ScentDeposit deposit2;
    deposit2.type = ScentType::TERRITORIAL;
    deposit2.creatureId = 2;
    deposit2.intensity = 0.9f;
    deposit2.signature = {0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f};
    deposit2.tickDeposited = 100;
    deposit2.decayRate = 150;
    
    layer.deposit(25, 25, deposit1);
    layer.deposit(25, 25, deposit2);
    
    auto scents = layer.getScentsAt(25, 25);
    TEST_ASSERT_EQ(2, scents.size());
}

void test_deposit_refresh_same_creature_same_type() {
    ScentLayer layer(100, 100);
    
    // First deposit
    ScentDeposit deposit1;
    deposit1.type = ScentType::MATE_SEEKING;
    deposit1.creatureId = 5;
    deposit1.intensity = 0.5f;
    deposit1.signature = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    deposit1.tickDeposited = 100;
    deposit1.decayRate = 100;
    
    layer.deposit(30, 30, deposit1);
    
    // Second deposit - same creature, same type, higher intensity
    ScentDeposit deposit2;
    deposit2.type = ScentType::MATE_SEEKING;
    deposit2.creatureId = 5;
    deposit2.intensity = 0.9f;
    deposit2.signature = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    deposit2.tickDeposited = 150;
    deposit2.decayRate = 100;
    
    layer.deposit(30, 30, deposit2);
    
    // Should only have one scent (refreshed)
    auto scents = layer.getScentsAt(30, 30);
    TEST_ASSERT_EQ(1, scents.size());
    TEST_ASSERT_NEAR(0.9f, scents[0].intensity, 0.001f);
    TEST_ASSERT_EQ(150u, scents[0].tickDeposited);
}

void test_get_scents_by_type() {
    ScentLayer layer(100, 100);
    
    // Add different types
    ScentDeposit mate;
    mate.type = ScentType::MATE_SEEKING;
    mate.creatureId = 1;
    mate.intensity = 0.7f;
    mate.tickDeposited = 100;
    mate.decayRate = 100;
    
    ScentDeposit alarm;
    alarm.type = ScentType::ALARM;
    alarm.creatureId = 2;
    alarm.intensity = 0.9f;
    alarm.tickDeposited = 100;
    alarm.decayRate = 50;
    
    ScentDeposit food;
    food.type = ScentType::FOOD_TRAIL;
    food.creatureId = 3;
    food.intensity = 0.5f;
    food.tickDeposited = 100;
    food.decayRate = 200;
    
    layer.deposit(10, 10, mate);
    layer.deposit(10, 10, alarm);
    layer.deposit(10, 10, food);
    
    // Filter by type
    auto mateScents = layer.getScentsOfType(10, 10, ScentType::MATE_SEEKING);
    TEST_ASSERT_EQ(1, mateScents.size());
    TEST_ASSERT_EQ(ScentType::MATE_SEEKING, mateScents[0].type);
    
    auto alarmScents = layer.getScentsOfType(10, 10, ScentType::ALARM);
    TEST_ASSERT_EQ(1, alarmScents.size());
    TEST_ASSERT_EQ(ScentType::ALARM, alarmScents[0].type);
}

void test_empty_tile_returns_empty() {
    ScentLayer layer(100, 100);
    
    auto scents = layer.getScentsAt(50, 50);
    TEST_ASSERT(scents.empty());
}

void test_boundary_conditions() {
    ScentLayer layer(100, 100);
    
    // Deposit at corners
    ScentDeposit deposit;
    deposit.type = ScentType::TERRITORIAL;
    deposit.creatureId = 1;
    deposit.intensity = 0.5f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 100;
    
    layer.deposit(0, 0, deposit);
    layer.deposit(99, 99, deposit);
    layer.deposit(0, 99, deposit);
    layer.deposit(99, 0, deposit);
    
    TEST_ASSERT_EQ(1, layer.getScentsAt(0, 0).size());
    TEST_ASSERT_EQ(1, layer.getScentsAt(99, 99).size());
    TEST_ASSERT_EQ(1, layer.getScentsAt(0, 99).size());
    TEST_ASSERT_EQ(1, layer.getScentsAt(99, 0).size());
}

//================================================================================
//  Decay Mechanics Tests
//================================================================================

void test_scent_decay_intensity() {
    ScentLayer layer(100, 100);
    
    ScentDeposit deposit;
    deposit.type = ScentType::MATE_SEEKING;
    deposit.creatureId = 1;
    deposit.intensity = 1.0f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 100;  // Should last 100 ticks
    
    layer.deposit(50, 50, deposit);
    
    // Update to tick 50 (halfway through decay)
    layer.update(50);
    
    auto scents = layer.getScentsAt(50, 50);
    TEST_ASSERT_EQ(1, scents.size());
    TEST_ASSERT_GT(scents[0].intensity, 0.0f);
    TEST_ASSERT_LT(scents[0].intensity, 1.0f);
}

void test_scent_full_decay_removal() {
    ScentLayer layer(100, 100);
    
    ScentDeposit deposit;
    deposit.type = ScentType::ALARM;
    deposit.creatureId = 1;
    deposit.intensity = 0.5f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 50;  // Short decay
    
    layer.deposit(50, 50, deposit);
    
    // Update past decay time
    layer.update(100);
    
    auto scents = layer.getScentsAt(50, 50);
    TEST_ASSERT(scents.empty());
}

void test_different_decay_rates() {
    ScentLayer layer(100, 100);
    
    // Short-lived scent
    ScentDeposit shortLived;
    shortLived.type = ScentType::ALARM;
    shortLived.creatureId = 1;
    shortLived.intensity = 0.8f;
    shortLived.tickDeposited = 0;
    shortLived.decayRate = 20;  // Very short
    
    // Long-lived scent
    ScentDeposit longLived;
    longLived.type = ScentType::TERRITORIAL;
    longLived.creatureId = 2;
    longLived.intensity = 0.8f;
    longLived.tickDeposited = 0;
    longLived.decayRate = 200;  // Long
    
    layer.deposit(50, 50, shortLived);
    layer.deposit(50, 50, longLived);
    
    // After 50 ticks, short should be gone, long should remain
    layer.update(50);
    
    auto scents = layer.getScentsAt(50, 50);
    TEST_ASSERT_EQ(1, scents.size());
    TEST_ASSERT_EQ(ScentType::TERRITORIAL, scents[0].type);
}

void test_batch_decay_processing() {
    ScentLayer layer(100, 100);
    
    // Add many scents across the map
    ScentDeposit deposit;
    deposit.type = ScentType::FOOD_TRAIL;
    deposit.intensity = 0.5f;
    deposit.decayRate = 100;
    
    for (int i = 0; i < 50; ++i) {
        deposit.creatureId = i;
        deposit.tickDeposited = 0;
        layer.deposit(i, i, deposit);
    }
    
    TEST_ASSERT_EQ(50, layer.getActiveTileCount());
    
    // Decay all
    layer.update(200);
    
    TEST_ASSERT_EQ(0, layer.getActiveTileCount());
}

//================================================================================
//  Genetic Signature Tests
//================================================================================

void test_signature_storage() {
    ScentLayer layer(100, 100);
    
    ScentDeposit deposit;
    deposit.type = ScentType::MATE_SEEKING;
    deposit.creatureId = 1;
    deposit.intensity = 0.5f;
    deposit.signature = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    deposit.tickDeposited = 0;
    deposit.decayRate = 100;
    
    layer.deposit(50, 50, deposit);
    
    auto scents = layer.getScentsAt(50, 50);
    TEST_ASSERT_EQ(1, scents.size());
    
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_NEAR(deposit.signature[i], scents[0].signature[i], 0.001f);
    }
}

void test_signature_used_for_identification() {
    // Different creatures with different signatures
    ScentDeposit depositA;
    depositA.signature = {0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f};
    
    ScentDeposit depositB;
    depositB.signature = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
    
    // Calculate signature similarity (simple dot product normalized)
    float similarity = 0.0f;
    for (int i = 0; i < 8; ++i) {
        similarity += depositA.signature[i] * depositB.signature[i];
    }
    similarity /= 8.0f;  // Normalize
    
    // Very different signatures should have low similarity
    TEST_ASSERT_LT(similarity, 0.2f);
    
    // Same signature should have high similarity
    float selfSimilarity = 0.0f;
    for (int i = 0; i < 8; ++i) {
        selfSimilarity += depositA.signature[i] * depositA.signature[i];
    }
    selfSimilarity /= 8.0f;
    
    TEST_ASSERT_GT(selfSimilarity, 0.8f);
}

//================================================================================
//  Sparse Storage Efficiency Tests
//================================================================================

void test_sparse_storage_empty_map() {
    ScentLayer layer(500, 500);  // Large map
    
    // Empty map should have zero active count
    TEST_ASSERT_EQ(0, layer.getActiveTileCount());
}

void test_sparse_storage_few_scents() {
    ScentLayer layer(500, 500);  // Large map
    
    ScentDeposit deposit;
    deposit.type = ScentType::MATE_SEEKING;
    deposit.creatureId = 1;
    deposit.intensity = 0.5f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 100;
    
    // Add only 10 scents to a 250,000 tile map
    for (int i = 0; i < 10; ++i) {
        layer.deposit(i * 50, i * 50, deposit);
    }
    
    TEST_ASSERT_EQ(10, layer.getActiveTileCount());
}

void test_sparse_storage_performance() {
    ScentLayer layer(500, 500);  // 250,000 tiles
    
    ScentDeposit deposit;
    deposit.type = ScentType::TERRITORIAL;
    deposit.creatureId = 1;
    deposit.intensity = 0.7f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 500;
    
    // Add 1000 scents (0.4% of tiles)
    for (int i = 0; i < 1000; ++i) {
        deposit.creatureId = i;
        layer.deposit(i % 500, i / 2, deposit);
    }
    
    // Time the update operation
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int tick = 0; tick < 10; ++tick) {
        layer.update(tick * 10);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in under 100ms total (10ms per update on average)
    TEST_ASSERT_LT(duration.count(), 100);
    
    std::cout << "    Performance: 10 updates on 1000 scents took " 
              << duration.count() << "ms" << std::endl;
}

void test_clear_operation() {
    ScentLayer layer(100, 100);
    
    ScentDeposit deposit;
    deposit.type = ScentType::FOOD_TRAIL;
    deposit.creatureId = 1;
    deposit.intensity = 0.5f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 100;
    
    for (int i = 0; i < 50; ++i) {
        layer.deposit(i, i, deposit);
    }
    
    TEST_ASSERT_EQ(50, layer.getActiveTileCount());
    
    layer.clear();
    
    TEST_ASSERT_EQ(0, layer.getActiveTileCount());
}

void test_remove_scents_from_creature() {
    ScentLayer layer(100, 100);
    
    ScentDeposit deposit;
    deposit.intensity = 0.5f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 100;
    deposit.type = ScentType::MATE_SEEKING;
    
    // Creature 1 deposits at multiple locations
    deposit.creatureId = 1;
    layer.deposit(10, 10, deposit);
    layer.deposit(20, 20, deposit);
    layer.deposit(30, 30, deposit);
    
    // Creature 2 deposits
    deposit.creatureId = 2;
    layer.deposit(40, 40, deposit);
    layer.deposit(50, 50, deposit);
    
    TEST_ASSERT_EQ(5, layer.getActiveTileCount());
    
    // Remove creature 1's scents
    layer.removeScentsFromCreature(1);
    
    TEST_ASSERT_EQ(2, layer.getActiveTileCount());
    TEST_ASSERT(layer.getScentsAt(10, 10).empty());
    TEST_ASSERT(!layer.getScentsAt(40, 40).empty());
}

//================================================================================
//  Strongest Scent Search Tests
//================================================================================

void test_find_strongest_scent_in_radius() {
    ScentLayer layer(100, 100);
    
    // Add weak scent nearby
    ScentDeposit weak;
    weak.type = ScentType::MATE_SEEKING;
    weak.creatureId = 1;
    weak.intensity = 0.3f;
    weak.tickDeposited = 0;
    weak.decayRate = 100;
    layer.deposit(48, 48, weak);
    
    // Add strong scent further
    ScentDeposit strong;
    strong.type = ScentType::MATE_SEEKING;
    strong.creatureId = 2;
    strong.intensity = 0.9f;
    strong.tickDeposited = 0;
    strong.decayRate = 100;
    layer.deposit(55, 55, strong);
    
    // Add different type scent (should be ignored)
    ScentDeposit different;
    different.type = ScentType::ALARM;
    different.creatureId = 3;
    different.intensity = 1.0f;
    different.tickDeposited = 0;
    different.decayRate = 100;
    layer.deposit(49, 49, different);
    
    int foundX = -1, foundY = -1;
    ScentDeposit found = layer.getStrongestScentInRadius(50, 50, 10, ScentType::MATE_SEEKING, foundX, foundY);
    
    TEST_ASSERT(found.intensity > 0.0f);
    TEST_ASSERT_EQ(55, foundX);
    TEST_ASSERT_EQ(55, foundY);
}

void test_no_scent_in_radius() {
    ScentLayer layer(100, 100);
    
    ScentDeposit deposit;
    deposit.type = ScentType::MATE_SEEKING;
    deposit.creatureId = 1;
    deposit.intensity = 0.8f;
    deposit.tickDeposited = 0;
    deposit.decayRate = 100;
    
    // Place scent far from search center
    layer.deposit(90, 90, deposit);
    
    int foundX = -1, foundY = -1;
    ScentDeposit found = layer.getStrongestScentInRadius(10, 10, 5, ScentType::MATE_SEEKING, foundX, foundY);
    
    TEST_ASSERT(found.intensity <= 0.0f);
}

//================================================================================
//  Olfactory Genes Registration Test
//================================================================================

void test_olfactory_genes_registered() {
    EcoSim::Genetics::GeneRegistry registry;
    EcoSim::Genetics::UniversalGenes::registerDefaults(registry);
    
    // Check that olfactory genes are registered
    TEST_ASSERT(registry.hasGene(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION));
    TEST_ASSERT(registry.hasGene(EcoSim::Genetics::UniversalGenes::SCENT_SIGNATURE_VARIANCE));
    TEST_ASSERT(registry.hasGene(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY));
    TEST_ASSERT(registry.hasGene(EcoSim::Genetics::UniversalGenes::SCENT_MASKING));
    
    // Check gene definitions are valid
    const auto& scentProd = registry.getDefinition(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION);
    TEST_ASSERT_GE(scentProd.getLimits().max_value, scentProd.getLimits().min_value);
    
    const auto& olfactory = registry.getDefinition(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY);
    TEST_ASSERT_GE(olfactory.getLimits().max_value, olfactory.getLimits().min_value);
}

//================================================================================
//  Main Entry Point
//================================================================================

void runScentLayerTests() {
    BEGIN_TEST_GROUP("ScentLayer Deposition and Retrieval");
    RUN_TEST(test_basic_deposit_and_retrieval);
    RUN_TEST(test_multiple_deposits_same_tile);
    RUN_TEST(test_deposit_refresh_same_creature_same_type);
    RUN_TEST(test_get_scents_by_type);
    RUN_TEST(test_empty_tile_returns_empty);
    RUN_TEST(test_boundary_conditions);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("ScentLayer Decay Mechanics");
    RUN_TEST(test_scent_decay_intensity);
    RUN_TEST(test_scent_full_decay_removal);
    RUN_TEST(test_different_decay_rates);
    RUN_TEST(test_batch_decay_processing);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Genetic Signature Tests");
    RUN_TEST(test_signature_storage);
    RUN_TEST(test_signature_used_for_identification);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Sparse Storage Efficiency");
    RUN_TEST(test_sparse_storage_empty_map);
    RUN_TEST(test_sparse_storage_few_scents);
    RUN_TEST(test_sparse_storage_performance);
    RUN_TEST(test_clear_operation);
    RUN_TEST(test_remove_scents_from_creature);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Strongest Scent Search");
    RUN_TEST(test_find_strongest_scent_in_radius);
    RUN_TEST(test_no_scent_in_radius);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Olfactory Genes Registration");
    RUN_TEST(test_olfactory_genes_registered);
    END_TEST_GROUP();
}

// Standalone main for running just scent layer tests
#ifdef SCENT_LAYER_TEST_MAIN
int main() {
    std::cout << "=== EcoSim ScentLayer Tests ===" << std::endl;
    
    runScentLayerTests();
    
    TestSuite::instance().printSummary();
    
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
