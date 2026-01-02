/**
 * @file test_reproducible_interface.cpp
 * @brief Tests for the IReproducible interface implementation
 * 
 * Tests both Plant (asexual) and Creature (sexual) implementations
 * of the IReproducible interface.
 */

#include <iostream>
#include <memory>
#include "test_framework.hpp"

#include "genetics/interfaces/IReproducible.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "objects/creature/creature.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Plant IReproducible Tests (Asexual Reproduction)
// ============================================================================

void testPlantReproductionMode() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(10, 20, registry);
    
    // Plants should always use asexual reproduction
    TEST_ASSERT_EQ(G::ReproductionMode::ASEXUAL, plant.getReproductionMode());
}

void testPlantIsCompatibleWith() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant1(0, 0, registry);
    G::Plant plant2(5, 5, registry);
    
    // Plants don't use partners for reproduction - always returns false
    TEST_ASSERT(!plant1.isCompatibleWith(plant2));
    TEST_ASSERT(!plant2.isCompatibleWith(plant1));
}

void testPlantCanReproduce() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    
    // Fresh plants are seedlings, not mature - typically can't reproduce
    // The exact behavior depends on the plant's state
    // Just verify method doesn't crash and returns bool
    bool canRep = plant.canReproduce();
    (void)canRep;  // Suppress unused warning
}

void testPlantGetReproductiveUrge() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    
    float urge = plant.getReproductiveUrge();
    TEST_ASSERT_GE(urge, 0.0f);
    TEST_ASSERT_LE(urge, 1.0f);
}

void testPlantGetReproductionEnergyCost() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    
    float cost = plant.getReproductionEnergyCost();
    TEST_ASSERT_GE(cost, 0.0f);
}

// Helper to mature a plant by simulating growth cycles
void maturePlant(G::Plant& plant) {
    G::EnvironmentState env;
    env.humidity = 0.7f;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;  // Noon for optimal light
    
    // Run many update cycles to grow the plant to maturity
    for (int i = 0; i < 1500; ++i) {
        plant.update(env);
    }
}

void testPlantReproduce() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant parent = factory.createFromTemplate("berry_bush", 10, 10);
    
    // Mature the plant so it can reproduce
    maturePlant(parent);
    
    // Verify it can now reproduce
    TEST_ASSERT_MSG(parent.canReproduce(), "Parent plant should be mature and able to reproduce");
    
    // Plant asexual reproduction (no partner needed)
    auto offspring = parent.reproduce(nullptr);
    
    // Offspring should be a valid Organism
    TEST_ASSERT(offspring != nullptr);
    
    // Verify offspring is a Plant via dynamic_cast
    // @todo This dynamic_cast will be unnecessary once Creature/Plant are unified
    auto* offspringPlant = dynamic_cast<G::Plant*>(offspring.get());
    TEST_ASSERT(offspringPlant != nullptr);
    
    // Offspring should be alive
    TEST_ASSERT(offspringPlant->isAlive());
}

void testPlantReproduceIgnoresPartner() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Use berry_bush for both since it matures quickly
    G::Plant parent = factory.createFromTemplate("berry_bush", 10, 10);
    G::Plant fakePartner = factory.createFromTemplate("berry_bush", 20, 20);
    
    // Mature both plants
    maturePlant(parent);
    maturePlant(fakePartner);
    
    // Verify parent can reproduce
    TEST_ASSERT_MSG(parent.canReproduce(), "Parent plant should be mature and able to reproduce");
    
    // Even if partner is provided, asexual reproduction ignores it
    auto offspring = parent.reproduce(&fakePartner);
    
    TEST_ASSERT(offspring != nullptr);
    
    auto* offspringPlant = dynamic_cast<G::Plant*>(offspring.get());
    TEST_ASSERT(offspringPlant != nullptr);
}

// ============================================================================
// Creature IReproducible Tests (Sexual Reproduction)
// ============================================================================

// Helper function to create test creatures with proper constructor
Creature createReproducibleTestCreature(int x = 10, int y = 10) {
    Creature::initializeGeneRegistry();
    
    auto genome = std::make_unique<G::Genome>(
        G::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    Creature creature(x, y, std::move(genome));
    creature.setHunger(8.0f);
    creature.setThirst(8.0f);
    
    return creature;
}

void testCreatureReproductionMode() {
    Creature creature = createReproducibleTestCreature(10, 20);
    
    // Creatures should always use sexual reproduction
    TEST_ASSERT_EQ(G::ReproductionMode::SEXUAL, creature.getReproductionMode());
}

void testCreatureCanReproduce() {
    Creature creature = createReproducibleTestCreature(0, 0);
    
    // canReproduce checks maturity, resources, and other conditions
    // Just verify method doesn't crash and returns bool
    bool canRep = creature.canReproduce();
    (void)canRep;  // Suppress unused warning
}

void testCreatureGetReproductiveUrge() {
    Creature creature = createReproducibleTestCreature(0, 0);
    
    float urge = creature.getReproductiveUrge();
    TEST_ASSERT_GE(urge, 0.0f);
    TEST_ASSERT_LE(urge, 1.0f);
}

void testCreatureGetReproductionEnergyCost() {
    Creature creature = createReproducibleTestCreature(0, 0);
    
    float cost = creature.getReproductionEnergyCost();
    TEST_ASSERT_GE(cost, 0.0f);
}

void testCreatureIsCompatibleWithSameArchetype() {
    Creature creature1 = createReproducibleTestCreature(0, 0);
    Creature creature2 = createReproducibleTestCreature(5, 5);
    
    // Creatures with similar genomes should typically be compatible
    // The actual compatibility depends on archetype matching logic
    // Just verify method doesn't crash and returns bool
    bool compatible = creature1.isCompatibleWith(creature2);
    (void)compatible;  // Suppress unused warning
}

void testCreatureIsCompatibleWithPlantReturnsFalse() {
    Creature creature = createReproducibleTestCreature(0, 0);
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Plant plant(10, 10, registry);
    
    // Creatures cannot mate with plants
    bool compatible = creature.isCompatibleWith(plant);
    TEST_ASSERT(!compatible);
}

// ============================================================================
// Interface Contract Tests
// ============================================================================

void testReproducibleInterfaceViaPointer() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    
    // Test accessing through interface pointer
    G::IReproducible* reproducible = &plant;
    
    TEST_ASSERT_EQ(G::ReproductionMode::ASEXUAL, reproducible->getReproductionMode());
    
    float urge = reproducible->getReproductiveUrge();
    TEST_ASSERT_GE(urge, 0.0f);
    
    float cost = reproducible->getReproductionEnergyCost();
    TEST_ASSERT_GE(cost, 0.0f);
}

void testReproductionModeEnum() {
    // Verify enum values are distinct
    TEST_ASSERT(G::ReproductionMode::SEXUAL != G::ReproductionMode::ASEXUAL);
}

// ============================================================================
// Test Runner
// ============================================================================

void runReproducibleInterfaceTests() {
    BEGIN_TEST_GROUP("Plant IReproducible Tests (Asexual)");
    RUN_TEST(testPlantReproductionMode);
    RUN_TEST(testPlantIsCompatibleWith);
    RUN_TEST(testPlantCanReproduce);
    RUN_TEST(testPlantGetReproductiveUrge);
    RUN_TEST(testPlantGetReproductionEnergyCost);
    RUN_TEST(testPlantReproduce);
    RUN_TEST(testPlantReproduceIgnoresPartner);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Creature IReproducible Tests (Sexual)");
    RUN_TEST(testCreatureReproductionMode);
    RUN_TEST(testCreatureCanReproduce);
    RUN_TEST(testCreatureGetReproductiveUrge);
    RUN_TEST(testCreatureGetReproductionEnergyCost);
    RUN_TEST(testCreatureIsCompatibleWithSameArchetype);
    RUN_TEST(testCreatureIsCompatibleWithPlantReturnsFalse);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Interface Contract Tests");
    RUN_TEST(testReproducibleInterfaceViaPointer);
    RUN_TEST(testReproductionModeEnum);
    END_TEST_GROUP();
}

#ifdef TEST_REPRODUCIBLE_STANDALONE
int main() {
    std::cout << "=== IReproducible Interface Tests ===" << std::endl;
    runReproducibleInterfaceTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
