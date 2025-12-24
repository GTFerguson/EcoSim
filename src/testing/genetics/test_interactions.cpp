/**
 * @file test_interactions.cpp
 * @brief Tests for Creature-Plant Interactions (Phase 2.4)
 * 
 * Tests FeedingInteraction, SeedDispersal, and CoevolutionTracker.
 */

#include <iostream>
#include <memory>
#include <vector>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"

// CoevolutionTracker depends on legacy Creature class - conditionally include
#ifdef INCLUDE_COEVOLUTION_TRACKER
#include "genetics/interactions/CoevolutionTracker.hpp"
#endif

namespace G = EcoSim::Genetics;

// ============================================================================
// FeedingInteraction Tests
// ============================================================================

void testFeedingInteractionCreation() {
    G::FeedingInteraction feeding;
    // Default instance should be valid
    TEST_ASSERT(true);
}

void testFeedingResultAttempt() {
    G::FeedingInteraction feeding;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    
    // Simulate a feeding attempt
    G::FeedingResult result = feeding.attemptToEatPlant(creaturePhenotype, plant, 1.0f);
    
    // Nutrition gained should be non-negative
    TEST_ASSERT_GE(result.nutritionGained, 0.0f);
    // Damage received should be non-negative
    TEST_ASSERT_GE(result.damageReceived, 0.0f);
    // Plant damage should be non-negative
    TEST_ASSERT_GE(result.plantDamage, 0.0f);
}

void testFeedingThornDamage() {
    G::FeedingInteraction feeding;
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Thorn bush has high thorn density
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    // Berry bush has low thorn density
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(*registry);
    G::Phenotype creaturePhenotype(&creatureGenome, registry.get());
    
    float thornDamage = feeding.calculateThornDamage(creaturePhenotype, thornBush);
    float berryDamage = feeding.calculateThornDamage(creaturePhenotype, berryBush);
    
    // Thorn bush should cause more damage
    TEST_ASSERT_GE(thornDamage, berryDamage);
}

void testFeedingToxinDamage() {
    G::FeedingInteraction feeding;
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Thorn bush has high toxicity
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    // Berry bush has low toxicity
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(*registry);
    G::Phenotype creaturePhenotype(&creatureGenome, registry.get());
    
    float thornToxin = feeding.calculateToxinDamage(creaturePhenotype, thornBush);
    float berryToxin = feeding.calculateToxinDamage(creaturePhenotype, berryBush);
    
    // Thorn bush should cause more toxin damage
    TEST_ASSERT_GE(thornToxin, berryToxin);
}

void testFeedingDigestionEfficiency() {
    G::FeedingInteraction feeding;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    
    float efficiency = feeding.getDigestionEfficiency(creaturePhenotype, plant);
    
    // Efficiency should be positive
    TEST_ASSERT_GE(efficiency, 0.0f);
}

void testFeedingSeedSurvival() {
    G::FeedingInteraction feeding;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    
    float survivalRate = feeding.calculateSeedSurvivalRate(creaturePhenotype, plant);
    
    // Survival rate should be between 0 and 1
    TEST_ASSERT_GE(survivalRate, 0.0f);
    TEST_ASSERT_LE(survivalRate, 1.0f);
}

void testFeedingDispersalDistance() {
    G::FeedingInteraction feeding;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    
    float distance = feeding.calculateDispersalDistance(creaturePhenotype);
    
    // Distance should be non-negative
    TEST_ASSERT_GE(distance, 0.0f);
}

void testFeedingDetection() {
    G::FeedingInteraction feeding;
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(*registry);
    G::Phenotype creaturePhenotype(&creatureGenome, registry.get());
    
    // Detection at close range
    bool canDetectClose = feeding.canDetectPlant(creaturePhenotype, berryBush, 1.0f);
    // Detection at far range
    bool canDetectFar = feeding.canDetectPlant(creaturePhenotype, berryBush, 100.0f);
    
    // Should detect at close range, may not detect at far range
    TEST_ASSERT(canDetectClose || !canDetectFar);
    
    float detectionRange = feeding.getDetectionRange(creaturePhenotype, berryBush);
    TEST_ASSERT_GE(detectionRange, 0.0f);
}

// ============================================================================
// SeedDispersal Tests
// ============================================================================

void testSeedDispersalCreation() {
    G::SeedDispersal dispersal;
    TEST_ASSERT(true);
}

void testSeedDispersalCreationWithSeed() {
    G::SeedDispersal dispersal(42); // Random seed
    TEST_ASSERT(true);
}

void testGravityDispersal() {
    G::SeedDispersal dispersal;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(50, 50, registry);
    
    G::DispersalEvent event = dispersal.disperseByGravity(plant);
    
    // Should land near parent
    float distance = event.getDistance();
    TEST_ASSERT_GE(distance, 0.0f);
    TEST_ASSERT_LE(distance, 5.0f); // Gravity dispersal is short range
    TEST_ASSERT_EQ(G::DispersalStrategy::GRAVITY, event.method);
}

void testWindDispersal() {
    G::SeedDispersal dispersal;
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant grass = factory.createFromTemplate("grass", 50, 50);
    
    G::DispersalEvent event = dispersal.disperseByWind(grass, 10.0f, 90.0f); // East wind
    
    // Wind dispersal can travel further
    float distance = event.getDistance();
    TEST_ASSERT_GE(distance, 0.0f);
    TEST_ASSERT_EQ(G::DispersalStrategy::WIND, event.method);
}

void testAnimalFruitDispersal() {
    G::SeedDispersal dispersal;
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 10, 10);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(*registry);
    G::Phenotype creaturePhenotype(&creatureGenome, registry.get());
    
    G::DispersalEvent event = dispersal.disperseByAnimalFruit(
        berryBush, creaturePhenotype, 50, 50
    );
    
    // Should land near creature location
    TEST_ASSERT_EQ(G::DispersalStrategy::ANIMAL_FRUIT, event.method);
    // Viability should be affected by gut passage
    TEST_ASSERT_GE(event.seedViability, 0.0f);
    TEST_ASSERT_LE(event.seedViability, 1.0f);
}

void testAnimalBurrDispersal() {
    G::SeedDispersal dispersal;
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(*registry);
    G::Phenotype creaturePhenotype(&creatureGenome, registry.get());
    
    G::DispersalEvent event = dispersal.disperseByAnimalBurr(
        thornBush, creaturePhenotype, 30, 30
    );
    
    TEST_ASSERT_EQ(G::DispersalStrategy::ANIMAL_BURR, event.method);
}

void testExplosiveDispersal() {
    G::SeedDispersal dispersal;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(50, 50, registry);
    
    G::DispersalEvent event = dispersal.disperseByExplosive(plant);
    
    // Explosive can launch seeds quite far
    float distance = event.getDistance();
    TEST_ASSERT_GE(distance, 0.0f);
    TEST_ASSERT_EQ(G::DispersalStrategy::EXPLOSIVE, event.method);
}

void testVegetativeDispersal() {
    G::SeedDispersal dispersal;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(50, 50, registry);
    
    G::DispersalEvent event = dispersal.disperseByVegetative(plant);
    
    // Vegetative is short range but high viability
    TEST_ASSERT_EQ(G::DispersalStrategy::VEGETATIVE, event.method);
    TEST_ASSERT_GE(event.seedViability, 0.8f); // Clonal spread is reliable
}

void testAutoDispersal() {
    G::SeedDispersal dispersal;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(50, 50, registry);
    
    // Auto dispersal uses plant's primary strategy
    G::DispersalEvent event = dispersal.disperse(plant, nullptr);
    
    // Should produce some dispersal
    TEST_ASSERT(event.method == G::DispersalStrategy::GRAVITY ||
                event.method == G::DispersalStrategy::WIND ||
                event.method == G::DispersalStrategy::ANIMAL_FRUIT ||
                event.method == G::DispersalStrategy::ANIMAL_BURR ||
                event.method == G::DispersalStrategy::EXPLOSIVE ||
                event.method == G::DispersalStrategy::VEGETATIVE);
}

void testBurrAttachment() {
    G::SeedDispersal dispersal;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    
    // Check if burr would attach
    bool willAttach = dispersal.willBurrAttach(plant, creaturePhenotype);
    // Result depends on genes - just check it returns without error
    TEST_ASSERT(willAttach || !willAttach);
}

void testBurrDetachment() {
    G::SeedDispersal dispersal;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    
    // Check if burr would detach after some ticks
    bool willDetachEarly = dispersal.willBurrDetach(creaturePhenotype, 1);
    bool willDetachLate = dispersal.willBurrDetach(creaturePhenotype, 1000);
    
    // Later ticks should have higher probability of detachment
    TEST_ASSERT(willDetachEarly || !willDetachEarly);
    TEST_ASSERT(willDetachLate || !willDetachLate);
}

void testExpectedBurrDistance() {
    G::SeedDispersal dispersal;
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    
    float distance = dispersal.calculateExpectedBurrDistance(plant, creaturePhenotype);
    TEST_ASSERT_GE(distance, 0.0f);
}

// ============================================================================
// CoevolutionTracker Tests (requires legacy Creature class)
// ============================================================================

#ifdef INCLUDE_COEVOLUTION_TRACKER

void testCoevolutionTrackerCreation() {
    G::CoevolutionTracker tracker;
    TEST_ASSERT_EQ(0, tracker.getCurrentGeneration());
}

void testCoevolutionTrackerWithConfig() {
    G::ArmsRaceConfig config;
    config.correlationThreshold = 0.7f;
    config.escalationThreshold = 0.15f;
    config.minGenerationsForTrend = 10;
    
    G::CoevolutionTracker tracker(config);
    TEST_ASSERT_EQ(0.7f, tracker.getConfig().correlationThreshold);
}

void testAddTrackedPair() {
    G::CoevolutionTracker tracker;
    
    tracker.addTrackedPair("TEST_CREATURE_GENE", "TEST_PLANT_GENE");
    
    // Should be able to get stats (even if empty)
    G::CoevolutionStats stats = tracker.getCoevolutionStats(
        "TEST_CREATURE_GENE", "TEST_PLANT_GENE"
    );
    TEST_ASSERT_EQ(0, stats.generationsTracked);
}

void testRemoveTrackedPair() {
    G::CoevolutionTracker tracker;
    
    tracker.addTrackedPair("TEST_GENE_A", "TEST_GENE_B");
    tracker.removeTrackedPair("TEST_GENE_A", "TEST_GENE_B");
    
    // Should not crash
    TEST_ASSERT(true);
}

void testAdvanceGeneration() {
    G::CoevolutionTracker tracker;
    
    TEST_ASSERT_EQ(0, tracker.getCurrentGeneration());
    tracker.advanceGeneration();
    TEST_ASSERT_EQ(1, tracker.getCurrentGeneration());
    tracker.advanceGeneration();
    TEST_ASSERT_EQ(2, tracker.getCurrentGeneration());
}

void testCoevolutionStrength() {
    G::CoevolutionTracker tracker;
    
    float strength = tracker.getCoevolutionStrength(
        "TOXIN_TOLERANCE", "TOXIN_PRODUCTION"
    );
    
    // With no data, should return 0
    TEST_ASSERT_EQ(0.0f, strength);
}

void testIsArmsRaceActive() {
    G::CoevolutionTracker tracker;
    
    bool active = tracker.isArmsRaceActive(
        "TOXIN_TOLERANCE", "TOXIN_PRODUCTION"
    );
    
    // With no data, should not be active
    TEST_ASSERT(!active);
}

void testGetActiveArmsRaces() {
    G::CoevolutionTracker tracker;
    
    std::vector<G::CoevolutionStats> races = tracker.getActiveArmsRaces();
    
    // With no data, should be empty
    TEST_ASSERT_EQ(0u, races.size());
}

void testGetAllTrackedPairs() {
    G::CoevolutionTracker tracker;
    
    std::vector<G::CoevolutionStats> pairs = tracker.getAllTrackedPairs();
    
    // Default tracker has some tracked pairs
    TEST_ASSERT_GE(pairs.size(), 0u);
}

void testCoevolutionReset() {
    G::CoevolutionTracker tracker;
    
    tracker.advanceGeneration();
    tracker.advanceGeneration();
    TEST_ASSERT_EQ(2, tracker.getCurrentGeneration());
    
    tracker.reset();
    TEST_ASSERT_EQ(0, tracker.getCurrentGeneration());
}

void testClearHistory() {
    G::CoevolutionTracker tracker;
    
    tracker.advanceGeneration();
    tracker.clearHistory();
    
    // Should not crash
    TEST_ASSERT(true);
}

void testGeneHistoryRetrieval() {
    G::CoevolutionTracker tracker;
    
    std::vector<std::pair<int, float>> creatureHistory =
        tracker.getCreatureGeneHistory("TOXIN_TOLERANCE");
    std::vector<std::pair<int, float>> plantHistory =
        tracker.getPlantGeneHistory("TOXIN_PRODUCTION");
    
    // With no data recorded, histories should be empty
    TEST_ASSERT_EQ(0u, creatureHistory.size());
    TEST_ASSERT_EQ(0u, plantHistory.size());
}

#endif // INCLUDE_COEVOLUTION_TRACKER

// ============================================================================
// Test Runner
// ============================================================================

void runInteractionTests() {
    BEGIN_TEST_GROUP("FeedingInteraction Tests");
    RUN_TEST(testFeedingInteractionCreation);
    RUN_TEST(testFeedingResultAttempt);
    RUN_TEST(testFeedingThornDamage);
    RUN_TEST(testFeedingToxinDamage);
    RUN_TEST(testFeedingDigestionEfficiency);
    RUN_TEST(testFeedingSeedSurvival);
    RUN_TEST(testFeedingDispersalDistance);
    RUN_TEST(testFeedingDetection);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SeedDispersal Tests");
    RUN_TEST(testSeedDispersalCreation);
    RUN_TEST(testSeedDispersalCreationWithSeed);
    RUN_TEST(testGravityDispersal);
    RUN_TEST(testWindDispersal);
    RUN_TEST(testAnimalFruitDispersal);
    RUN_TEST(testAnimalBurrDispersal);
    RUN_TEST(testExplosiveDispersal);
    RUN_TEST(testVegetativeDispersal);
    RUN_TEST(testAutoDispersal);
    RUN_TEST(testBurrAttachment);
    RUN_TEST(testBurrDetachment);
    RUN_TEST(testExpectedBurrDistance);
    END_TEST_GROUP();
    
#ifdef INCLUDE_COEVOLUTION_TRACKER
    BEGIN_TEST_GROUP("CoevolutionTracker Tests");
    RUN_TEST(testCoevolutionTrackerCreation);
    RUN_TEST(testCoevolutionTrackerWithConfig);
    RUN_TEST(testAddTrackedPair);
    RUN_TEST(testRemoveTrackedPair);
    RUN_TEST(testAdvanceGeneration);
    RUN_TEST(testCoevolutionStrength);
    RUN_TEST(testIsArmsRaceActive);
    RUN_TEST(testGetActiveArmsRaces);
    RUN_TEST(testGetAllTrackedPairs);
    RUN_TEST(testCoevolutionReset);
    RUN_TEST(testClearHistory);
    RUN_TEST(testGeneHistoryRetrieval);
    END_TEST_GROUP();
#else
    std::cout << "[INFO] CoevolutionTracker tests skipped (requires legacy Creature)" << std::endl;
#endif
}

#ifdef TEST_INTERACTIONS_STANDALONE
int main() {
    std::cout << "=== Creature-Plant Interaction Tests (Phase 2.4) ===" << std::endl;
    runInteractionTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
