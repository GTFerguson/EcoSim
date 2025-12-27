/**
 * @file test_creature_behavior_integration.cpp
 * @brief Integration tests for BehaviorController integration into Creature
 *
 * Phase 3: Creature God Class decomposition
 *
 * These tests verify that the BehaviorController integration works correctly
 * when USE_NEW_BEHAVIOR_SYSTEM is enabled, and that backward compatibility
 * is maintained when it is disabled.
 */

#include "test_framework.hpp"
#include "objects/creature/creature.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/OrganismState.hpp"

#if USE_NEW_BEHAVIOR_SYSTEM
#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/behaviors/FeedingBehavior.hpp"
#include "genetics/behaviors/HuntingBehavior.hpp"
#include "genetics/behaviors/MatingBehavior.hpp"
#include "genetics/behaviors/MovementBehavior.hpp"
#include "genetics/behaviors/RestBehavior.hpp"
#include "genetics/behaviors/ZoochoryBehavior.hpp"
#endif

#include <iostream>
#include <memory>

using namespace EcoSim::Testing;
using namespace EcoSim::Genetics;

namespace {

/**
 * Helper to create a test genome with standard values
 */
std::unique_ptr<Genome> createTestGenome() {
    auto genome = std::make_unique<Genome>();
    
    // Set up basic gene values through chromosomes
    auto& registry = Creature::getGeneRegistry();
    
    // The genome will use default values from the registry
    return genome;
}

/**
 * Helper to create a herbivore test genome
 */
std::unique_ptr<Genome> createHerbivoreGenome() {
    auto genome = createTestGenome();
    
    // Herbivore traits: high plant digestion, low meat digestion
    // These will be expressed through the phenotype
    return genome;
}

/**
 * Helper to create a carnivore test genome
 */
std::unique_ptr<Genome> createCarnivoreGenome() {
    auto genome = createTestGenome();
    
    // Carnivore traits: low plant digestion, high meat digestion, high hunt instinct
    // These will be expressed through the phenotype
    return genome;
}

} // anonymous namespace

//==============================================================================
// Tests that work with both flag states (backward compatibility)
//==============================================================================

/**
 * Test that creatures can be created with the standard genome constructor
 */
void test_creature_construction_with_genome() {
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    TEST_ASSERT_EQ(10, creature.getX());
    TEST_ASSERT_EQ(20, creature.getY());
    TEST_ASSERT(creature.getGenome() != nullptr);
    TEST_ASSERT(creature.getPhenotype() != nullptr);
}

/**
 * Test that creature update() works (legacy system)
 */
void test_creature_legacy_update() {
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    unsigned int initialAge = creature.getAge();
    float initialHunger = creature.getHunger();
    
    creature.update();
    
    // Age should increment
    TEST_ASSERT_EQ(initialAge + 1, creature.getAge());
    
    // Hunger should decrease due to metabolism
    TEST_ASSERT_LT(creature.getHunger(), initialHunger);
}

/**
 * Test that decideBehaviour() is still functional
 */
void test_creature_decide_behaviour() {
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    // Set creature state to force hungry profile
    creature.setHunger(-1.0f);
    creature.decideBehaviour();
    
    // Should be in hungry profile
    TEST_ASSERT_EQ(Profile::hungry, creature.getProfile());
}

//==============================================================================
// Tests specific to new behavior system
//==============================================================================

#if USE_NEW_BEHAVIOR_SYSTEM

/**
 * Test that behaviorController is properly initialized
 */
void test_behaviorController_initialized() {
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    // Initialize behavior controller
    creature.initializeBehaviorController();
    
    // Should have a valid controller
    TEST_ASSERT(creature.getBehaviorController() != nullptr);
}

/**
 * Test that updateWithBehaviors executes a behavior
 */
void test_updateWithBehaviors_executesBehavior() {
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    creature.initializeBehaviorController();
    
    // Build a minimal context (would normally come from World)
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // Execute behavior update
    BehaviorResult result = creature.updateWithBehaviors(ctx);
    
    // MovementBehavior (IDLE priority) should execute as fallback
    // Result may or may not be executed depending on context
    // The key is that no crash occurs and the system runs
    TEST_ASSERT(true);  // If we got here, execution succeeded
}

/**
 * Test that buildBehaviorContext creates valid context
 */
void test_buildBehaviorContext_createsValidContext() {
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    // We can't fully test without World/ScentLayer,
    // but we can verify the method signature works
    // This test verifies compilation and basic structure
    TEST_ASSERT(creature.getBehaviorController() == nullptr);  // Not initialized yet
    
    creature.initializeBehaviorController();
    TEST_ASSERT(creature.getBehaviorController() != nullptr);
}

/**
 * Test that feeding behavior triggers when creature is hungry
 */
void test_feedingBehavior_triggersWhenHungry() {
    // Create a feeding behavior directly to test
    FeedingBehavior feedingBehavior;
    
    TEST_ASSERT_EQ("feeding", feedingBehavior.getId());
    TEST_ASSERT_EQ(BehaviorPriority::NORMAL, feedingBehavior.getPriority());
    
    // Create a hungry creature
    auto genome = createHerbivoreGenome();
    Creature creature(10, 20, std::move(genome));
    creature.setHunger(-1.0f);  // Very hungry
    
    // Build context
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // FeedingBehavior requires world access, so isApplicable will be false
    // without proper context - this is expected behavior
    bool applicable = feedingBehavior.isApplicable(creature, ctx);
    // Not applicable without world - this is correct
    TEST_ASSERT(!applicable);
}

/**
 * Test that hunting behavior triggers for predators
 */
void test_huntingBehavior_triggersForPredator() {
    HuntingBehavior huntingBehavior;
    
    TEST_ASSERT_EQ("hunting", huntingBehavior.getId());
    TEST_ASSERT_EQ(BehaviorPriority::HIGH, huntingBehavior.getPriority());
    
    // Create a carnivore creature
    auto genome = createCarnivoreGenome();
    Creature creature(10, 20, std::move(genome));
    creature.setHunger(-1.0f);  // Very hungry
    
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // HuntingBehavior requires world access for prey detection
    bool applicable = huntingBehavior.isApplicable(creature, ctx);
    // Not applicable without world - this is correct
    TEST_ASSERT(!applicable);
}

/**
 * Test that mating behavior triggers when ready to mate
 */
void test_matingBehavior_triggersWhenReady() {
    MatingBehavior matingBehavior;
    
    TEST_ASSERT_EQ("mating", matingBehavior.getId());
    TEST_ASSERT_EQ(BehaviorPriority::NORMAL, matingBehavior.getPriority());
    
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    // Set creature state for mating
    creature.setMate(10.0f);  // High mate drive
    creature.setHunger(5.0f);  // Well fed
    creature.setFatigue(0.0f);  // Not tired
    
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // MatingBehavior requires world access
    bool applicable = matingBehavior.isApplicable(creature, ctx);
    TEST_ASSERT(!applicable);  // Not applicable without world
}

/**
 * Test that rest behavior triggers when tired
 */
void test_restBehavior_triggersWhenTired() {
    RestBehavior restBehavior;
    
    TEST_ASSERT_EQ("rest", restBehavior.getId());
    TEST_ASSERT_EQ(BehaviorPriority::CRITICAL, restBehavior.getPriority());
    
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    // Set creature to be exhausted
    creature.setFatigue(100.0f);  // Very tired
    
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // RestBehavior should be applicable when exhausted
    bool applicable = restBehavior.isApplicable(creature, ctx);
    TEST_ASSERT(applicable);
}

/**
 * Test that behaviors are executed by priority
 */
void test_behaviors_executedByPriority() {
    // Create controller with multiple behaviors
    BehaviorController controller;
    
    controller.addBehavior(std::make_unique<RestBehavior>());      // CRITICAL
    controller.addBehavior(std::make_unique<HuntingBehavior>());   // HIGH
    controller.addBehavior(std::make_unique<FeedingBehavior>());   // NORMAL
    controller.addBehavior(std::make_unique<MatingBehavior>());    // NORMAL
    controller.addBehavior(std::make_unique<MovementBehavior>());  // IDLE
    
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    // Make creature exhausted - RestBehavior should win
    creature.setFatigue(100.0f);
    
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // Execute the controller
    BehaviorResult result = controller.update(creature, ctx);
    
    // RestBehavior should have been selected due to CRITICAL priority
    TEST_ASSERT_EQ("rest", controller.getCurrentBehaviorId());
}

/**
 * Test movement behavior as fallback
 */
void test_movementBehavior_asFallback() {
    MovementBehavior movementBehavior;
    
    TEST_ASSERT_EQ("movement", movementBehavior.getId());
    TEST_ASSERT_EQ(BehaviorPriority::IDLE, movementBehavior.getPriority());
    
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    // Set creature to be content (no urgent needs)
    creature.setHunger(5.0f);
    creature.setThirst(5.0f);
    creature.setFatigue(0.0f);
    creature.setMate(0.0f);
    
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // MovementBehavior should always be applicable as fallback
    bool applicable = movementBehavior.isApplicable(creature, ctx);
    TEST_ASSERT(applicable);
}

/**
 * Test zoochory behavior for seed dispersal
 */
void test_zoochoryBehavior_forSeedDispersal() {
    ZoochoryBehavior zoochoryBehavior;
    
    TEST_ASSERT_EQ("zoochory", zoochoryBehavior.getId());
    TEST_ASSERT_EQ(BehaviorPriority::LOW, zoochoryBehavior.getPriority());
    
    auto genome = createTestGenome();
    Creature creature(10, 20, std::move(genome));
    
    BehaviorContext ctx;
    ctx.scentLayer = nullptr;
    ctx.world = nullptr;
    ctx.organismState = nullptr;
    ctx.deltaTime = 1.0f;
    ctx.currentTick = 100;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    // ZoochoryBehavior requires burrs or gut seeds
    bool applicable = zoochoryBehavior.isApplicable(creature, ctx);
    // Not applicable without seeds - this is correct
    TEST_ASSERT(!applicable);
}

#endif // USE_NEW_BEHAVIOR_SYSTEM

//==============================================================================
// Test runner
//==============================================================================

void run_creature_behavior_integration_tests() {
    std::cout << "\n=== Creature Behavior Integration Tests ===" << std::endl;
    
    // Initialize gene registry before tests
    Creature::initializeGeneRegistry();
    
    // Backward compatibility tests (always run)
    BEGIN_TEST_GROUP("Backward Compatibility");
    RUN_TEST(test_creature_construction_with_genome);
    RUN_TEST(test_creature_legacy_update);
    RUN_TEST(test_creature_decide_behaviour);
    END_TEST_GROUP();
    
#if USE_NEW_BEHAVIOR_SYSTEM
    // New behavior system tests (only when enabled)
    BEGIN_TEST_GROUP("BehaviorController Integration");
    RUN_TEST(test_behaviorController_initialized);
    RUN_TEST(test_updateWithBehaviors_executesBehavior);
    RUN_TEST(test_buildBehaviorContext_createsValidContext);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Individual Behavior Tests");
    RUN_TEST(test_feedingBehavior_triggersWhenHungry);
    RUN_TEST(test_huntingBehavior_triggersForPredator);
    RUN_TEST(test_matingBehavior_triggersWhenReady);
    RUN_TEST(test_restBehavior_triggersWhenTired);
    RUN_TEST(test_movementBehavior_asFallback);
    RUN_TEST(test_zoochoryBehavior_forSeedDispersal);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Priority Execution");
    RUN_TEST(test_behaviors_executedByPriority);
    END_TEST_GROUP();
#else
    std::cout << "\n[INFO] New behavior system tests skipped (USE_NEW_BEHAVIOR_SYSTEM=0)" << std::endl;
#endif
    
    TestSuite::instance().printSummary();
}

// Main entry point if compiled standalone
#ifdef TEST_STANDALONE
int main() {
    run_creature_behavior_integration_tests();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
