/**
 * @file test_creature_state_machine.cpp
 * @brief Tests for creature state machine and behavior decision system
 *
 * Tests cover:
 * - Behavior decision making (decideBehaviour)
 * - Main update loop (update)
 * - Profile transitions between hungry/thirsty/breed/sleep
 * - Resource drain over time
 * - Age progression
 */

#include "test_framework.hpp"
#include "objects/creature/creature.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <cmath>
#include <memory>

using namespace EcoSim::Testing;

Creature createStateMachineTestCreature() {
    Creature::initializeGeneRegistry();
    
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    Creature creature(10, 10, std::move(genome));
    
    return creature;
}

void test_decideBehaviour_hungryWhenLowFood() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(0.5f);
    creature.setThirst(10.0f);
    creature.setFatigue(0.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    
    TEST_ASSERT(creature.getProfile() == Profile::hungry);
}

void test_decideBehaviour_thirstyWhenLowWater() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(10.0f);
    creature.setThirst(0.5f);
    creature.setFatigue(0.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    
    TEST_ASSERT(creature.getProfile() == Profile::thirsty);
}

void test_decideBehaviour_breedWhenReady() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(10.0f);
    creature.setThirst(10.0f);
    creature.setFatigue(0.0f);
    creature.setMate(5.0f);
    
    creature.decideBehaviour();
    
    TEST_ASSERT(creature.getProfile() == Profile::breed);
}

void test_decideBehaviour_sleepWhenTired() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(10.0f);
    creature.setThirst(10.0f);
    creature.setFatigue(10.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    
    TEST_ASSERT(creature.getProfile() == Profile::sleep);
}

void test_decideBehaviour_migrateWhenContent() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(10.0f);
    creature.setThirst(10.0f);
    creature.setFatigue(0.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    
    TEST_ASSERT(creature.getProfile() == Profile::migrate);
}

void test_update_incrementsAge() {
    Creature creature = createStateMachineTestCreature();
    
    unsigned ageBefore = creature.getAge();
    
    creature.update();
    
    TEST_ASSERT_EQ(creature.getAge(), ageBefore + 1);
}

void test_update_reducesHunger() {
    Creature creature = createStateMachineTestCreature();
    creature.setHunger(5.0f);
    
    float hungerBefore = creature.getHunger();
    
    creature.update();
    
    TEST_ASSERT_LT(creature.getHunger(), hungerBefore);
}

void test_update_reducesThirst() {
    Creature creature = createStateMachineTestCreature();
    creature.setThirst(5.0f);
    
    float thirstBefore = creature.getThirst();
    
    creature.update();
    
    TEST_ASSERT_LT(creature.getThirst(), thirstBefore);
}

void test_update_sleepReducesFatigue() {
    Creature creature = createStateMachineTestCreature();
    creature.setFatigue(5.0f);
    creature.setHunger(10.0f);
    creature.setThirst(10.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    TEST_ASSERT(creature.getProfile() == Profile::sleep);
    
    float fatigueBefore = creature.getFatigue();
    creature.update();
    
    TEST_ASSERT_LT(creature.getFatigue(), fatigueBefore);
}

void test_update_awakeFatigueIncreases() {
    Creature creature = createStateMachineTestCreature();
    creature.setFatigue(0.0f);
    creature.setHunger(10.0f);
    creature.setThirst(10.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    TEST_ASSERT(creature.getProfile() != Profile::sleep);
    
    float fatigueBefore = creature.getFatigue();
    creature.update();
    
    TEST_ASSERT_GT(creature.getFatigue(), fatigueBefore);
}

void test_decideBehaviour_thirstPriorityOverHunger() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(0.5f);
    creature.setThirst(0.3f);
    creature.setFatigue(0.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    
    TEST_ASSERT(creature.getProfile() == Profile::thirsty);
}

void test_decideBehaviour_profilePersistenceWhileSeeking() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(0.5f);
    creature.setThirst(10.0f);
    creature.setFatigue(0.0f);
    creature.setMate(0.0f);
    
    creature.decideBehaviour();
    Profile initialProfile = creature.getProfile();
    TEST_ASSERT(initialProfile == Profile::hungry);
    
    creature.decideBehaviour();
    
    TEST_ASSERT(creature.getProfile() == initialProfile);
}

void test_update_multipleTicksProgressAge() {
    Creature creature = createStateMachineTestCreature();
    
    unsigned initialAge = creature.getAge();
    
    for (int i = 0; i < 10; i++) {
        creature.update();
    }
    
    TEST_ASSERT_EQ(creature.getAge(), initialAge + 10);
}

void test_update_resourceDrainIsMetabolismBased() {
    Creature creature = createStateMachineTestCreature();
    creature.setHunger(5.0f);
    creature.setThirst(5.0f);
    
    float metabolism = creature.getMetabolism();
    float hungerBefore = creature.getHunger();
    
    creature.update();
    
    float hungerLoss = hungerBefore - creature.getHunger();
    TEST_ASSERT_GT(hungerLoss, 0.0f);
    TEST_ASSERT_LE(hungerLoss, metabolism * 2.0f);
}

void test_deathCheck_oldAge() {
    Creature creature = createStateMachineTestCreature();
    
    unsigned lifespan = creature.getLifespan();
    creature.setAge(lifespan + 1);
    
    short deathCode = creature.deathCheck();
    
    TEST_ASSERT_EQ(deathCode, 1);
}

void test_deathCheck_starvation() {
    Creature creature = createStateMachineTestCreature();
    creature.setHunger(-1.0f);
    
    short deathCode = creature.deathCheck();
    
    TEST_ASSERT_EQ(deathCode, 2);
}

void test_deathCheck_dehydration() {
    Creature creature = createStateMachineTestCreature();
    creature.setThirst(-1.0f);
    
    short deathCode = creature.deathCheck();
    
    TEST_ASSERT_EQ(deathCode, 3);
}

void test_deathCheck_alive() {
    Creature creature = createStateMachineTestCreature();
    creature.setHunger(5.0f);
    creature.setThirst(5.0f);
    creature.setMate(0.0f);
    
    short deathCode = creature.deathCheck();
    
    TEST_ASSERT_EQ(deathCode, 0);
}

void test_getMotivation_mapsFromProfile() {
    Creature creature = createStateMachineTestCreature();
    
    creature.setHunger(0.5f);
    creature.setThirst(10.0f);
    creature.setFatigue(0.0f);
    creature.setMate(0.0f);
    creature.decideBehaviour();
    TEST_ASSERT(creature.getMotivation() == Motivation::Hungry);
    
    creature.setHunger(10.0f);
    creature.setThirst(0.5f);
    creature.decideBehaviour();
    TEST_ASSERT(creature.getMotivation() == Motivation::Thirsty);
}

void run_creature_state_machine_tests() {
    BEGIN_TEST_GROUP("Creature State Machine Tests");
    
    RUN_TEST(test_decideBehaviour_hungryWhenLowFood);
    RUN_TEST(test_decideBehaviour_thirstyWhenLowWater);
    RUN_TEST(test_decideBehaviour_breedWhenReady);
    RUN_TEST(test_decideBehaviour_sleepWhenTired);
    RUN_TEST(test_decideBehaviour_migrateWhenContent);
    RUN_TEST(test_decideBehaviour_thirstPriorityOverHunger);
    RUN_TEST(test_decideBehaviour_profilePersistenceWhileSeeking);
    
    RUN_TEST(test_update_incrementsAge);
    RUN_TEST(test_update_reducesHunger);
    RUN_TEST(test_update_reducesThirst);
    RUN_TEST(test_update_sleepReducesFatigue);
    RUN_TEST(test_update_awakeFatigueIncreases);
    RUN_TEST(test_update_multipleTicksProgressAge);
    RUN_TEST(test_update_resourceDrainIsMetabolismBased);
    
    RUN_TEST(test_deathCheck_oldAge);
    RUN_TEST(test_deathCheck_starvation);
    RUN_TEST(test_deathCheck_dehydration);
    RUN_TEST(test_deathCheck_alive);
    
    RUN_TEST(test_getMotivation_mapsFromProfile);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running Creature State Machine Tests..." << std::endl;
    run_creature_state_machine_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
