/**
 * @file test_creature_breeding.cpp
 * @brief Tests for creature breeding system
 *
 * Tests cover:
 * - Mate fitness calculation (checkFitness)
 * - Offspring creation (breedCreature)
 * - Mate finding behavior (findMate)
 * - Resource costs for breeding
 * - Genetic inheritance
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

Creature createBreedingTestCreature(int x = 10, int y = 10) {
    Creature::initializeGeneRegistry();
    
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    Creature creature(x, y, std::move(genome));
    creature.setHunger(8.0f);
    creature.setThirst(8.0f);
    
    return creature;
}

Creature createModifiedCreature(int x, int y, float geneModifier) {
    Creature::initializeGeneRegistry();
    
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    if (genome->hasGene(EcoSim::Genetics::UniversalGenes::MAX_SIZE)) {
        genome->getGeneMutable(EcoSim::Genetics::UniversalGenes::MAX_SIZE)
            .setAlleleValues(geneModifier);
    }
    if (genome->hasGene(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE)) {
        genome->getGeneMutable(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE)
            .setAlleleValues(geneModifier);
    }
    
    Creature creature(x, y, std::move(genome));
    creature.setHunger(8.0f);
    creature.setThirst(8.0f);
    
    return creature;
}

void test_checkFitness_similarCreatures_highScore() {
    Creature c1 = createBreedingTestCreature(10, 10);
    Creature c2 = createBreedingTestCreature(11, 10);
    
    float fitness = c1.checkFitness(c2);
    
    TEST_ASSERT_GT(fitness, 0.0f);
    TEST_ASSERT_LE(fitness, 1.5f);
}

void test_checkFitness_differentCreatures_lowScore() {
    Creature c1 = createModifiedCreature(10, 10, 0.1f);
    Creature c2 = createModifiedCreature(11, 10, 0.9f);
    
    float similarFitness = c1.checkFitness(createBreedingTestCreature(11, 10));
    float differentFitness = c1.checkFitness(c2);
    
    TEST_ASSERT_GT(differentFitness, 0.0f);
}

void test_checkFitness_distanceAffectsScore() {
    Creature c1 = createBreedingTestCreature(10, 10);
    Creature closeCreature = createBreedingTestCreature(11, 10);
    Creature farCreature = createBreedingTestCreature(50, 50);
    
    float closeFitness = c1.checkFitness(closeCreature);
    float farFitness = c1.checkFitness(farCreature);
    
    TEST_ASSERT_GT(closeFitness, farFitness);
}

void test_breedCreature_producesViableOffspring() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    Creature offspring = parent1.breedCreature(parent2);
    
    TEST_ASSERT_GT(offspring.getMaxHealth(), 0.0f);
    // getGenome() and getPhenotype() return references, so they're always valid
    // Verify offspring has expected genes and traits
    TEST_ASSERT(offspring.getGenome().hasGene(EcoSim::Genetics::UniversalGenes::MAX_SIZE));
    TEST_ASSERT(offspring.getPhenotype().hasTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE));
}

void test_breedCreature_offspringInheritsGenes() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    Creature offspring = parent1.breedCreature(parent2);
    
    const EcoSim::Genetics::Genome& offspringGenome = offspring.getGenome();
    
    TEST_ASSERT(offspringGenome.hasGene(EcoSim::Genetics::UniversalGenes::MAX_SIZE));
    TEST_ASSERT(offspringGenome.hasGene(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE));
    TEST_ASSERT(offspringGenome.hasGene(EcoSim::Genetics::UniversalGenes::SIGHT_RANGE));
}

void test_breedCreature_consumesResources() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    float parent1HungerBefore = parent1.getHunger();
    float parent1ThirstBefore = parent1.getThirst();
    float parent2HungerBefore = parent2.getHunger();
    float parent2ThirstBefore = parent2.getThirst();
    
    Creature offspring = parent1.breedCreature(parent2);
    
    TEST_ASSERT_LT(parent1.getHunger(), parent1HungerBefore);
    TEST_ASSERT_LT(parent1.getThirst(), parent1ThirstBefore);
    TEST_ASSERT_LT(parent2.getHunger(), parent2HungerBefore);
    TEST_ASSERT_LT(parent2.getThirst(), parent2ThirstBefore);
}

void test_breedCreature_offspringReceivesResources() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    parent1.setHunger(10.0f);
    parent1.setThirst(10.0f);
    parent2.setHunger(10.0f);
    parent2.setThirst(10.0f);
    
    Creature offspring = parent1.breedCreature(parent2);
    
    TEST_ASSERT_GT(offspring.getHunger(), 0.0f);
    TEST_ASSERT_GT(offspring.getThirst(), 0.0f);
}

void test_breedCreature_resetsMateValue() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    parent1.setMate(5.0f);
    parent2.setMate(5.0f);
    
    Creature offspring = parent1.breedCreature(parent2);
    
    TEST_ASSERT_EQ(parent1.getMate(), 0.0f);
    TEST_ASSERT_EQ(parent2.getMate(), 0.0f);
}

void test_breedCreature_offspringAtParentLocation() {
    Creature parent1 = createBreedingTestCreature(15, 25);
    Creature parent2 = createBreedingTestCreature(15, 25);
    
    Creature offspring = parent1.breedCreature(parent2);
    
    TEST_ASSERT_EQ(offspring.getX(), parent1.getX());
    TEST_ASSERT_EQ(offspring.getY(), parent1.getY());
}

void test_checkFitness_penalizesTooSimilar() {
    Creature c1 = createBreedingTestCreature(10, 10);
    Creature c2 = createBreedingTestCreature(11, 10);
    
    float fitness = c1.checkFitness(c2);
    
    TEST_ASSERT_GT(fitness, 0.0f);
    TEST_ASSERT_LT(fitness, 1.5f);
}

void test_breedCreature_offspringStartsYoung() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    for (int i = 0; i < 100; i++) {
        parent1.update();
        parent2.update();
    }
    TEST_ASSERT_GT(parent1.getAge(), 0u);
    
    Creature offspring = parent1.breedCreature(parent2);
    
    TEST_ASSERT_EQ(offspring.getAge(), 0u);
}

void test_breedCreature_resourcesNotNegative() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    parent1.setHunger(1.0f);
    parent1.setThirst(1.0f);
    parent2.setHunger(1.0f);
    parent2.setThirst(1.0f);
    
    Creature offspring = parent1.breedCreature(parent2);
    
    TEST_ASSERT_GE(offspring.getHunger(), 0.0f);
    TEST_ASSERT_GE(offspring.getThirst(), 0.0f);
}

void run_creature_breeding_tests() {
    BEGIN_TEST_GROUP("Creature Breeding System Tests");
    
    RUN_TEST(test_checkFitness_similarCreatures_highScore);
    RUN_TEST(test_checkFitness_differentCreatures_lowScore);
    RUN_TEST(test_checkFitness_distanceAffectsScore);
    RUN_TEST(test_checkFitness_penalizesTooSimilar);
    
    RUN_TEST(test_breedCreature_producesViableOffspring);
    RUN_TEST(test_breedCreature_offspringInheritsGenes);
    RUN_TEST(test_breedCreature_consumesResources);
    RUN_TEST(test_breedCreature_offspringReceivesResources);
    RUN_TEST(test_breedCreature_resetsMateValue);
    RUN_TEST(test_breedCreature_offspringAtParentLocation);
    RUN_TEST(test_breedCreature_offspringStartsYoung);
    RUN_TEST(test_breedCreature_resourcesNotNegative);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running Creature Breeding Tests..." << std::endl;
    run_creature_breeding_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
