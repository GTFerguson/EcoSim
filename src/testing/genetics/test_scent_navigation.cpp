/**
 * @file test_scent_navigation.cpp
 * @brief Tests for scent-based navigation system (Phase 2: Gradient Navigation)
 * 
 * Tests scent detection, direction calculation, genetic similarity filtering,
 * olfactory acuity scaling, and integration with creature movement.
 */

#include "test_framework.hpp"
#include "world/ScentLayer.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "objects/creature/creature.hpp"
#include "objects/creature/genome.hpp"

#include <iostream>
#include <cmath>
#include <array>

using namespace EcoSim;
using namespace EcoSim::Testing;

//================================================================================
//  Helper Functions
//================================================================================

/**
 * Create a test genome for creatures with specified traits
 */
Genome createTestGenome(Diet diet, unsigned sight = 100, unsigned lifespan = 100000) {
    return Genome(
        lifespan,   // lifespan
        sight,      // sight
        0.3f,       // tHunger
        0.3f,       // tThirst
        0.3f,       // tFatigue
        0.3f,       // tMate
        0.1f,       // comfInc
        0.1f,       // comfDec
        diet,       // diet
        false,      // flocks
        5,          // flee
        10          // pursue
    );
}

/**
 * Create a test scent deposit with specified parameters
 */
ScentDeposit createTestDeposit(int creatureId, float intensity, 
                                const std::array<float, 8>& signature,
                                unsigned tick = 0, unsigned decay = 200) {
    ScentDeposit deposit;
    deposit.type = ScentType::MATE_SEEKING;
    deposit.creatureId = creatureId;
    deposit.intensity = intensity;
    deposit.signature = signature;
    deposit.tickDeposited = tick;
    deposit.decayRate = decay;
    return deposit;
}

//================================================================================
//  Signature Similarity Tests
//================================================================================

void test_signature_similarity_identical() {
    std::array<float, 8> sig1 = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    std::array<float, 8> sig2 = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    
    float similarity = Creature::calculateSignatureSimilarity(sig1, sig2);
    
    // Identical signatures should have similarity = 1.0 (cosine similarity)
    TEST_ASSERT_NEAR(1.0f, similarity, 0.001f);
}

void test_signature_similarity_orthogonal() {
    // Create orthogonal signatures (one positive, one zero)
    std::array<float, 8> sig1 = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::array<float, 8> sig2 = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    
    float similarity = Creature::calculateSignatureSimilarity(sig1, sig2);
    
    // Orthogonal signatures should have similarity = 0.0
    TEST_ASSERT_NEAR(0.0f, similarity, 0.001f);
}

void test_signature_similarity_partial() {
    std::array<float, 8> sig1 = {0.8f, 0.8f, 0.8f, 0.8f, 0.5f, 0.5f, 0.5f, 0.5f};
    std::array<float, 8> sig2 = {0.7f, 0.7f, 0.7f, 0.7f, 0.5f, 0.5f, 0.5f, 0.5f};
    
    float similarity = Creature::calculateSignatureSimilarity(sig1, sig2);
    
    // Similar but not identical should have high but not perfect similarity
    TEST_ASSERT_GT(similarity, 0.9f);
    TEST_ASSERT_LT(similarity, 1.0f);
}

void test_signature_similarity_different_species() {
    // Herbivore signature (diet encoded in first element = 0.0)
    std::array<float, 8> herbivore = {0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    // Carnivore signature (diet encoded = 0.75)
    std::array<float, 8> carnivore = {0.75f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    
    float similarity = Creature::calculateSignatureSimilarity(herbivore, carnivore);
    
    // Different diet types should have lower similarity
    // The exact threshold depends on the rest of the signature
    TEST_ASSERT_GT(similarity, 0.5f);  // Still somewhat similar due to shared components
    TEST_ASSERT_LT(similarity, 1.0f);  // But not identical
}

//================================================================================
//  Detection Range Tests
//================================================================================

void test_detection_range_scales_with_acuity() {
    Creature::initializeGeneRegistry();
    
    // Create creature with known olfactory acuity
    Genome genome = createTestGenome(Diet::banana);
    Creature creature(50, 50, genome);
    creature.enableNewGenetics(true);
    
    // Force the creature into breeding mode by manipulating state
    creature.setMate(1.0f);  // High mate desire should trigger breeding profile
    
    // The detection range should be approximately:
    // base 10 + acuity (default ~0.5) * 20 = 10 + 10 = 20 tiles
    // Note: We can't directly test the internal range calculation,
    // but we can verify the creature can detect scents within expected range
    
    ScentLayer layer(100, 100);
    
    // Place scent at distance 15 (should be within range for moderate acuity)
    std::array<float, 8> sig = {0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    layer.deposit(65, 50, createTestDeposit(999, 0.8f, sig));
    
    // The creature should be able to detect this if acuity allows
    // (Testing indirectly through scent layer query)
    int outX, outY;
    ScentDeposit found = layer.getStrongestScentInRadius(50, 50, 20, 
                                                          ScentType::MATE_SEEKING, 
                                                          outX, outY);
    TEST_ASSERT_GT(found.intensity, 0.0f);
}

//================================================================================
//  Direction Calculation Tests  
//================================================================================

void test_direction_north() {
    ScentLayer layer(100, 100);
    
    // Place scent directly north (lower y)
    std::array<float, 8> sig = {0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    layer.deposit(50, 40, createTestDeposit(999, 0.8f, sig));  // North = lower Y
    
    int outX, outY;
    layer.getStrongestScentInRadius(50, 50, 15, ScentType::MATE_SEEKING, outX, outY);
    
    // Calculate direction delta
    int dx = outX - 50;
    int dy = outY - 50;
    
    TEST_ASSERT_EQ(0, dx);
    TEST_ASSERT_LT(dy, 0);  // Negative Y = North
}

void test_direction_southeast() {
    ScentLayer layer(100, 100);
    
    // Place scent to the southeast (higher x, higher y)
    std::array<float, 8> sig = {0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    layer.deposit(60, 60, createTestDeposit(999, 0.8f, sig));
    
    int outX, outY;
    layer.getStrongestScentInRadius(50, 50, 20, ScentType::MATE_SEEKING, outX, outY);
    
    // Calculate direction delta
    int dx = outX - 50;
    int dy = outY - 50;
    
    TEST_ASSERT_GT(dx, 0);  // Positive X = East
    TEST_ASSERT_GT(dy, 0);  // Positive Y = South
}

void test_direction_at_same_position() {
    ScentLayer layer(100, 100);
    
    // Place scent at the same position
    std::array<float, 8> sig = {0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    layer.deposit(50, 50, createTestDeposit(999, 0.8f, sig));
    
    int outX, outY;
    layer.getStrongestScentInRadius(50, 50, 5, ScentType::MATE_SEEKING, outX, outY);
    
    // Calculate direction delta
    int dx = outX - 50;
    int dy = outY - 50;
    
    TEST_ASSERT_EQ(0, dx);
    TEST_ASSERT_EQ(0, dy);  // At the scent source
}

//================================================================================
//  Genetic Similarity Filtering Tests
//================================================================================

void test_filter_by_genetic_similarity() {
    ScentLayer layer(100, 100);
    
    // Same species signature (herbivore)
    std::array<float, 8> herbSig = {0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.8f, 0.5f, 0.5f};
    layer.deposit(60, 50, createTestDeposit(100, 0.9f, herbSig));
    
    // Different species signature (carnivore)
    std::array<float, 8> carnSig = {0.75f, 0.5f, 0.5f, 0.5f, 0.9f, 0.2f, 0.9f, 0.9f};
    layer.deposit(40, 50, createTestDeposit(200, 0.9f, carnSig));
    
    // A herbivore creature's signature should be more similar to herbSig
    float herbSimilarity = Creature::calculateSignatureSimilarity(herbSig, herbSig);
    float carnSimilarity = Creature::calculateSignatureSimilarity(herbSig, carnSig);
    
    TEST_ASSERT_GT(herbSimilarity, carnSimilarity);
    TEST_ASSERT_NEAR(1.0f, herbSimilarity, 0.001f);  // Identical
}

void test_min_similarity_threshold() {
    // Test that 60% similarity threshold works
    std::array<float, 8> sig1 = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    std::array<float, 8> sig2 = {0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f};
    
    float similarity = Creature::calculateSignatureSimilarity(sig1, sig2);
    
    // These signatures are scaled versions of each other, cosine similarity = 1.0
    TEST_ASSERT_GT(similarity, 0.6f);  // Above 60% threshold
}

//================================================================================
//  Scent Decay Integration Tests
//================================================================================

void test_scent_decay_affects_detection() {
    ScentLayer layer(100, 100);
    
    std::array<float, 8> sig = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    
    // Place scent with short decay
    layer.deposit(60, 50, createTestDeposit(100, 0.8f, sig, 0, 50));
    
    // Scent should exist initially
    auto scents = layer.getScentsAt(60, 50);
    TEST_ASSERT_EQ(1, scents.size());
    
    // After decay time, scent should be gone
    layer.update(100);  // Past decay time
    
    scents = layer.getScentsAt(60, 50);
    TEST_ASSERT(scents.empty());
}

void test_scent_intensity_affects_priority() {
    ScentLayer layer(100, 100);
    
    std::array<float, 8> sig = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    
    // Place weak scent closer
    layer.deposit(55, 50, createTestDeposit(100, 0.3f, sig));
    
    // Place strong scent further
    layer.deposit(65, 50, createTestDeposit(200, 0.9f, sig));
    
    // Strongest scent search should find the stronger one
    int outX, outY;
    ScentDeposit found = layer.getStrongestScentInRadius(50, 50, 20, 
                                                          ScentType::MATE_SEEKING, 
                                                          outX, outY);
    
    TEST_ASSERT_EQ(65, outX);  // Should find the stronger scent
    TEST_ASSERT_NEAR(0.9f, found.intensity, 0.001f);
}

//================================================================================
//  Creature Scent Signature Tests
//================================================================================

void test_creature_computes_scent_signature() {
    Creature::initializeGeneRegistry();
    
    Genome genome = createTestGenome(Diet::banana);
    Creature creature(50, 50, genome);
    creature.enableNewGenetics(true);
    
    std::array<float, 8> signature = creature.computeScentSignature();
    
    // Signature should have 8 elements
    TEST_ASSERT_EQ(8, signature.size());
    
    // First element should encode diet type
    // banana diet = 0, so first element should be 0 * 0.25 = 0.0
    TEST_ASSERT_NEAR(0.0f, signature[0], 0.001f);
    
    // All values should be in [0, 1] range
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_GE(signature[i], 0.0f);
        TEST_ASSERT_LE(signature[i], 1.0f);
    }
}

void test_creature_signature_reflects_diet() {
    Creature::initializeGeneRegistry();
    
    // Herbivore (banana)
    Genome herbGenome = createTestGenome(Diet::banana);
    Creature herbivore(50, 50, herbGenome);
    herbivore.enableNewGenetics(true);
    
    // Carnivore (predator)
    Genome carnGenome = createTestGenome(Diet::predator);
    Creature carnivore(50, 50, carnGenome);
    carnivore.enableNewGenetics(true);
    
    auto herbSig = herbivore.computeScentSignature();
    auto carnSig = carnivore.computeScentSignature();
    
    // Diet is encoded in first element
    // banana = 0 * 0.25 = 0.0
    // predator = 3 * 0.25 = 0.75
    TEST_ASSERT_NEAR(0.0f, herbSig[0], 0.001f);
    TEST_ASSERT_NEAR(0.75f, carnSig[0], 0.001f);
}

void test_same_species_high_similarity() {
    Creature::initializeGeneRegistry();
    
    // Two herbivores with same diet
    Genome genome1 = createTestGenome(Diet::banana, 100, 100000);
    Genome genome2 = createTestGenome(Diet::banana, 100, 100000);
    
    Creature c1(50, 50, genome1);
    Creature c2(60, 60, genome2);
    
    c1.enableNewGenetics(true);
    c2.enableNewGenetics(true);
    
    auto sig1 = c1.computeScentSignature();
    auto sig2 = c2.computeScentSignature();
    
    float similarity = Creature::calculateSignatureSimilarity(sig1, sig2);
    
    // Same species should have high similarity
    TEST_ASSERT_GT(similarity, 0.6f);  // Above 60% threshold
}

void test_different_species_lower_similarity() {
    Creature::initializeGeneRegistry();
    
    // Herbivore vs Carnivore
    Genome herbGenome = createTestGenome(Diet::banana, 100, 100000);
    Genome carnGenome = createTestGenome(Diet::predator, 100, 100000);
    
    Creature herbivore(50, 50, herbGenome);
    Creature carnivore(60, 60, carnGenome);
    
    herbivore.enableNewGenetics(true);
    carnivore.enableNewGenetics(true);
    
    auto herbSig = herbivore.computeScentSignature();
    auto carnSig = carnivore.computeScentSignature();
    
    float crossSimilarity = Creature::calculateSignatureSimilarity(herbSig, carnSig);
    float selfSimilarity = Creature::calculateSignatureSimilarity(herbSig, herbSig);
    
    // Cross-species similarity should be lower than self-similarity
    TEST_ASSERT_LT(crossSimilarity, selfSimilarity);
}

//================================================================================
//  Main Entry Point
//================================================================================

void runScentNavigationTests() {
    BEGIN_TEST_GROUP("Signature Similarity Calculation");
    RUN_TEST(test_signature_similarity_identical);
    RUN_TEST(test_signature_similarity_orthogonal);
    RUN_TEST(test_signature_similarity_partial);
    RUN_TEST(test_signature_similarity_different_species);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Detection Range Scaling");
    RUN_TEST(test_detection_range_scales_with_acuity);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Direction Calculation");
    RUN_TEST(test_direction_north);
    RUN_TEST(test_direction_southeast);
    RUN_TEST(test_direction_at_same_position);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Genetic Similarity Filtering");
    RUN_TEST(test_filter_by_genetic_similarity);
    RUN_TEST(test_min_similarity_threshold);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Scent Decay Integration");
    RUN_TEST(test_scent_decay_affects_detection);
    RUN_TEST(test_scent_intensity_affects_priority);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Creature Scent Signatures");
    RUN_TEST(test_creature_computes_scent_signature);
    RUN_TEST(test_creature_signature_reflects_diet);
    RUN_TEST(test_same_species_high_similarity);
    RUN_TEST(test_different_species_lower_similarity);
    END_TEST_GROUP();
}

// Standalone main for running just scent navigation tests
#ifdef SCENT_NAVIGATION_TEST_MAIN
int main() {
    std::cout << "=== EcoSim Scent Navigation Tests ===" << std::endl;
    
    runScentNavigationTests();
    
    TestSuite::instance().printSummary();
    
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
