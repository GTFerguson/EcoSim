/**
 * @file test_creature_state_machine.cpp
 * @brief Tests for creature death checks and state queries
 *
 * The legacy Profile-based decision system (decideBehaviour, update) has been
 * replaced by BehaviorController. These tests cover the remaining Creature
 * state methods that are still active.
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

void test_motivation_defaultsToContent() {
    Creature creature = createStateMachineTestCreature();

    TEST_ASSERT(creature.getMotivation() == Motivation::Content);
}

void test_motivation_setAndGet() {
    Creature creature = createStateMachineTestCreature();

    creature.setMotivation(Motivation::Hungry);
    TEST_ASSERT(creature.getMotivation() == Motivation::Hungry);

    creature.setMotivation(Motivation::Thirsty);
    TEST_ASSERT(creature.getMotivation() == Motivation::Thirsty);

    creature.setMotivation(Motivation::Amorous);
    TEST_ASSERT(creature.getMotivation() == Motivation::Amorous);

    creature.setMotivation(Motivation::Tired);
    TEST_ASSERT(creature.getMotivation() == Motivation::Tired);
}

void run_creature_state_machine_tests() {
    BEGIN_TEST_GROUP("Creature State Machine Tests");

    RUN_TEST(test_deathCheck_oldAge);
    RUN_TEST(test_deathCheck_starvation);
    RUN_TEST(test_deathCheck_dehydration);
    RUN_TEST(test_deathCheck_alive);

    RUN_TEST(test_motivation_defaultsToContent);
    RUN_TEST(test_motivation_setAndGet);

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
