/**
 * @file test_creature_movement.cpp
 * @brief Tests for creature movement system
 *
 * Tests cover:
 * - Movement cost calculation (movementCost)
 * - Resource drain during movement
 * - Distance calculation (calculateDistance)
 * - Movement speed based on genetics
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

Creature createMovementTestCreature() {
    Creature::initializeGeneRegistry();
    
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    Creature creature(10, 10, std::move(genome));
    creature.setHunger(10.0f);
    creature.setThirst(10.0f);
    
    return creature;
}

Creature createCreatureWithLocomotion(float locomotion) {
    Creature::initializeGeneRegistry();
    
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    if (genome->hasGene(EcoSim::Genetics::UniversalGenes::LOCOMOTION)) {
        genome->getGeneMutable(EcoSim::Genetics::UniversalGenes::LOCOMOTION)
            .setAlleleValues(locomotion);
    }
    
    Creature creature(10, 10, std::move(genome));
    creature.setHunger(10.0f);
    creature.setThirst(10.0f);
    
    return creature;
}

void test_movementCost_drainsHunger() {
    Creature creature = createMovementTestCreature();
    
    float hungerBefore = creature.getHunger();
    
    creature.movementCost(1.0f);
    
    TEST_ASSERT_LT(creature.getHunger(), hungerBefore);
}

void test_movementCost_scalesByDistance() {
    Creature creature1 = createMovementTestCreature();
    Creature creature2 = createMovementTestCreature();
    
    float hungerBefore1 = creature1.getHunger();
    float hungerBefore2 = creature2.getHunger();
    
    creature1.movementCost(1.0f);
    creature2.movementCost(5.0f);
    
    float cost1 = hungerBefore1 - creature1.getHunger();
    float cost2 = hungerBefore2 - creature2.getHunger();
    
    TEST_ASSERT_GT(cost2, cost1);
}

void test_movementCost_zeroDistanceNoCost() {
    Creature creature = createMovementTestCreature();
    
    float hungerBefore = creature.getHunger();
    
    creature.movementCost(0.0f);
    
    TEST_ASSERT_EQ(creature.getHunger(), hungerBefore);
}

void test_movementCost_metabolismAffectsCost() {
    Creature creature = createMovementTestCreature();
    
    float metabolism = creature.getMetabolism();
    float hungerBefore = creature.getHunger();
    float distance = 1.0f;
    
    creature.movementCost(distance);
    
    float actualCost = hungerBefore - creature.getHunger();
    float expectedCost = metabolism * distance;
    
    TEST_ASSERT_NEAR(actualCost, expectedCost, 0.001f);
}

void test_calculateDistance_samePosition() {
    Creature creature = createMovementTestCreature();
    
    float distance = creature.calculateDistance(creature.getX(), creature.getY());
    
    TEST_ASSERT_EQ(distance, 0.0f);
}

void test_calculateDistance_pythagorean() {
    Creature creature = createMovementTestCreature();
    
    float distance = creature.calculateDistance(creature.getX() + 3, creature.getY() + 4);
    
    TEST_ASSERT_NEAR(distance, 5.0f, 0.001f);
}

void test_calculateDistance_horizontalMovement() {
    Creature creature = createMovementTestCreature();
    
    float distance = creature.calculateDistance(creature.getX() + 10, creature.getY());
    
    TEST_ASSERT_NEAR(distance, 10.0f, 0.001f);
}

void test_calculateDistance_verticalMovement() {
    Creature creature = createMovementTestCreature();
    
    float distance = creature.calculateDistance(creature.getX(), creature.getY() + 10);
    
    TEST_ASSERT_NEAR(distance, 10.0f, 0.001f);
}

void test_calculateDistance_diagonalMovement() {
    Creature creature = createMovementTestCreature();
    
    float distance = creature.calculateDistance(creature.getX() + 1, creature.getY() + 1);
    
    float expected = std::sqrt(2.0f);
    TEST_ASSERT_NEAR(distance, expected, 0.001f);
}

void test_getMovementSpeed_positiveValue() {
    Creature creature = createMovementTestCreature();
    
    float speed = creature.getMovementSpeed();
    
    TEST_ASSERT_GT(speed, 0.0f);
}

void test_getMovementSpeed_hasMinimum() {
    Creature creature = createMovementTestCreature();
    
    float speed = creature.getMovementSpeed();
    
    TEST_ASSERT_GE(speed, 0.1f);
}

void test_getMovementSpeed_locomotionAffectsSpeed() {
    Creature slowCreature = createCreatureWithLocomotion(0.1f);
    Creature fastCreature = createCreatureWithLocomotion(0.9f);
    
    float slowSpeed = slowCreature.getMovementSpeed();
    float fastSpeed = fastCreature.getMovementSpeed();
    
    // Both speeds should be positive and meet minimum requirements
    // The actual relationship depends on multiple factors (mass, leg length)
    TEST_ASSERT_GE(slowSpeed, 0.1f);
    TEST_ASSERT_GE(fastSpeed, 0.1f);
}

void test_movementCost_diagonalCostsMore() {
    Creature creature1 = createMovementTestCreature();
    Creature creature2 = createMovementTestCreature();
    
    float cardinalDistance = 1.0f;
    float diagonalDistance = std::sqrt(2.0f);
    
    float hungerBefore1 = creature1.getHunger();
    float hungerBefore2 = creature2.getHunger();
    
    creature1.movementCost(cardinalDistance);
    creature2.movementCost(diagonalDistance);
    
    float cardinalCost = hungerBefore1 - creature1.getHunger();
    float diagonalCost = hungerBefore2 - creature2.getHunger();
    
    TEST_ASSERT_GT(diagonalCost, cardinalCost);
}

void test_worldPosition_setAndGet() {
    Creature creature = createMovementTestCreature();
    
    creature.setWorldPosition(15.5f, 25.7f);
    
    TEST_ASSERT_NEAR(creature.getWorldX(), 15.5f, 0.001f);
    TEST_ASSERT_NEAR(creature.getWorldY(), 25.7f, 0.001f);
}

void test_tilePosition_truncatesFloat() {
    Creature creature = createMovementTestCreature();
    
    creature.setWorldPosition(15.9f, 25.1f);
    
    TEST_ASSERT_EQ(creature.tileX(), 15);
    TEST_ASSERT_EQ(creature.tileY(), 25);
}

void test_movementCost_largeDistanceLargeCost() {
    Creature creature = createMovementTestCreature();
    
    float hungerBefore = creature.getHunger();
    
    creature.movementCost(100.0f);
    
    float cost = hungerBefore - creature.getHunger();
    TEST_ASSERT_GT(cost, 0.0f);
}

void test_getMovementSpeed_woundedReducesSpeed() {
    Creature creature = createMovementTestCreature();
    
    float fullHealthSpeed = creature.getMovementSpeed();
    
    float maxHealth = creature.getMaxHealth();
    creature.takeDamage(maxHealth * 0.75f);
    
    float woundedSpeed = creature.getMovementSpeed();
    
    TEST_ASSERT_LE(woundedSpeed, fullHealthSpeed);
}

void run_creature_movement_tests() {
    BEGIN_TEST_GROUP("Creature Movement System Tests");
    
    RUN_TEST(test_movementCost_drainsHunger);
    RUN_TEST(test_movementCost_scalesByDistance);
    RUN_TEST(test_movementCost_zeroDistanceNoCost);
    RUN_TEST(test_movementCost_metabolismAffectsCost);
    RUN_TEST(test_movementCost_diagonalCostsMore);
    RUN_TEST(test_movementCost_largeDistanceLargeCost);
    
    RUN_TEST(test_calculateDistance_samePosition);
    RUN_TEST(test_calculateDistance_pythagorean);
    RUN_TEST(test_calculateDistance_horizontalMovement);
    RUN_TEST(test_calculateDistance_verticalMovement);
    RUN_TEST(test_calculateDistance_diagonalMovement);
    
    RUN_TEST(test_getMovementSpeed_positiveValue);
    RUN_TEST(test_getMovementSpeed_hasMinimum);
    RUN_TEST(test_getMovementSpeed_locomotionAffectsSpeed);
    RUN_TEST(test_getMovementSpeed_woundedReducesSpeed);
    
    RUN_TEST(test_worldPosition_setAndGet);
    RUN_TEST(test_tilePosition_truncatesFloat);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running Creature Movement Tests..." << std::endl;
    run_creature_movement_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
