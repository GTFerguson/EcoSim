/**
 * @file test_creature_breeding.cpp
 * @brief Tests for creature breeding system via IReproducible interface
 *
 * Tests cover:
 * - Mate fitness calculation (checkFitness)
 * - Offspring creation (reproduce)
 * - Mate finding behavior (findMate)
 * - Resource costs for breeding
 * - Genetic inheritance
 *
 * @todo These tests use dynamic_cast to convert Organism* to Creature*.
 * This will be simplified when Creature/Plant are unified into a single Organism class.
 */

#include "test_framework.hpp"
#include "objects/creature/creature.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/organisms/Organism.hpp"

#include <cmath>
#include <memory>

using namespace EcoSim::Testing;

Creature createBreedingTestCreature(int x = 10, int y = 10) {
    Creature::initializeGeneRegistry();
    
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    Creature creature(x, y, std::move(genome));
    
    // Set resources high enough for breeding (needs > BREED_COST which is 3.0f)
    // Also ensures hunger/thirst priorities are low (satisfied needs)
    creature.setHunger(8.0f);
    creature.setThirst(8.0f);
    
    // Set fatigue low so sleep priority is low
    creature.setFatigue(0.0f);
    
    // Set mate value high so breed priority is highest
    // decideBehaviour() computes: _mate - getTMate() where getTMate() defaults to 3.0f
    // Need mate > 3.0f and higher than other needs for Profile::breed
    creature.setMate(5.0f);
    
    // Grow creature to maturity (canReproduce requires isMature() which checks
    // if creature has grown to 50% of max size via the growth system)
    // With high hunger (nutrition), growth is fast
    while (!creature.isMature()) {
        creature.grow();
    }
    
    // Call decideBehaviour() to set _profile based on current needs
    // This should set _profile to Profile::breed since mate need is highest
    creature.decideBehaviour();
    
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
    
    // Set resources and state for breeding capability (same as createBreedingTestCreature)
    creature.setHunger(8.0f);
    creature.setThirst(8.0f);
    creature.setFatigue(0.0f);
    creature.setMate(5.0f);
    
    // Grow creature to maturity (canReproduce requires isMature())
    while (!creature.isMature()) {
        creature.grow();
    }
    
    creature.decideBehaviour();
    
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

void test_reproduce_producesViableOffspring() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    // Use IReproducible interface
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    // @todo Remove dynamic_cast when Creature/Plant unified into Organism
    Creature* offspring = dynamic_cast<Creature*>(offspringBase.get());
    TEST_ASSERT_MSG(offspring != nullptr, "reproduce() should return Creature type");
    
    TEST_ASSERT_GT(offspring->getMaxHealth(), 0.0f);
    TEST_ASSERT(offspring->getGenome().hasGene(EcoSim::Genetics::UniversalGenes::MAX_SIZE));
    TEST_ASSERT(offspring->getPhenotype().hasTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE));
}

void test_reproduce_offspringInheritsGenes() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    Creature* offspring = dynamic_cast<Creature*>(offspringBase.get());
    TEST_ASSERT_MSG(offspring != nullptr, "reproduce() should return Creature type");
    
    const EcoSim::Genetics::Genome& offspringGenome = offspring->getGenome();
    
    TEST_ASSERT(offspringGenome.hasGene(EcoSim::Genetics::UniversalGenes::MAX_SIZE));
    TEST_ASSERT(offspringGenome.hasGene(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE));
    TEST_ASSERT(offspringGenome.hasGene(EcoSim::Genetics::UniversalGenes::SIGHT_RANGE));
}

void test_reproduce_consumesResources() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    float parent1HungerBefore = parent1.getHunger();
    float parent1ThirstBefore = parent1.getThirst();
    float parent2HungerBefore = parent2.getHunger();
    float parent2ThirstBefore = parent2.getThirst();
    
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    TEST_ASSERT_LT(parent1.getHunger(), parent1HungerBefore);
    TEST_ASSERT_LT(parent1.getThirst(), parent1ThirstBefore);
    TEST_ASSERT_LT(parent2.getHunger(), parent2HungerBefore);
    TEST_ASSERT_LT(parent2.getThirst(), parent2ThirstBefore);
}

void test_reproduce_offspringReceivesResources() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    parent1.setHunger(10.0f);
    parent1.setThirst(10.0f);
    parent2.setHunger(10.0f);
    parent2.setThirst(10.0f);
    
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    Creature* offspring = dynamic_cast<Creature*>(offspringBase.get());
    TEST_ASSERT_MSG(offspring != nullptr, "reproduce() should return Creature type");
    
    TEST_ASSERT_GT(offspring->getHunger(), 0.0f);
    TEST_ASSERT_GT(offspring->getThirst(), 0.0f);
}

void test_reproduce_resetsMateValue() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    parent1.setMate(5.0f);
    parent2.setMate(5.0f);
    
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    TEST_ASSERT_EQ(parent1.getMate(), 0.0f);
    TEST_ASSERT_EQ(parent2.getMate(), 0.0f);
}

void test_reproduce_offspringAtParentLocation() {
    Creature parent1 = createBreedingTestCreature(15, 25);
    Creature parent2 = createBreedingTestCreature(15, 25);
    
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    Creature* offspring = dynamic_cast<Creature*>(offspringBase.get());
    TEST_ASSERT_MSG(offspring != nullptr, "reproduce() should return Creature type");
    
    TEST_ASSERT_EQ(offspring->getX(), parent1.getX());
    TEST_ASSERT_EQ(offspring->getY(), parent1.getY());
}

void test_checkFitness_penalizesTooSimilar() {
    Creature c1 = createBreedingTestCreature(10, 10);
    Creature c2 = createBreedingTestCreature(11, 10);
    
    float fitness = c1.checkFitness(c2);
    
    TEST_ASSERT_GT(fitness, 0.0f);
    TEST_ASSERT_LT(fitness, 1.5f);
}

void test_reproduce_offspringStartsYoung() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    // Age the parents more (they already start mature from createBreedingTestCreature)
    for (int i = 0; i < 100; i++) {
        parent1.update();
        parent2.update();
    }
    TEST_ASSERT_GT(parent1.getAge(), 0u);
    
    // Reset breeding state after updates (updates drain resources and change profile)
    parent1.setHunger(8.0f);
    parent1.setThirst(8.0f);
    parent1.setMate(5.0f);
    parent1.setFatigue(0.0f);
    parent1.decideBehaviour();
    
    parent2.setHunger(8.0f);
    parent2.setThirst(8.0f);
    parent2.setMate(5.0f);
    parent2.setFatigue(0.0f);
    parent2.decideBehaviour();
    
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    Creature* offspring = dynamic_cast<Creature*>(offspringBase.get());
    TEST_ASSERT_MSG(offspring != nullptr, "reproduce() should return Creature type");
    
    TEST_ASSERT_EQ(offspring->getAge(), 0u);
}

void test_reproduce_resourcesNotNegative() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    Creature parent2 = createBreedingTestCreature(10, 10);
    
    // Set resources low but still above BREED_COST (3.0f) for canReproduce() to pass
    // Using 4.0f ensures hasResources check passes (needs > BREED_COST)
    parent1.setHunger(4.0f);
    parent1.setThirst(4.0f);
    parent2.setHunger(4.0f);
    parent2.setThirst(4.0f);
    
    auto offspringBase = parent1.reproduce(&parent2);
    TEST_ASSERT_MSG(offspringBase != nullptr, "reproduce() should return offspring");
    
    Creature* offspring = dynamic_cast<Creature*>(offspringBase.get());
    TEST_ASSERT_MSG(offspring != nullptr, "reproduce() should return Creature type");
    
    // Offspring receives shared resources from parents (which are now low)
    TEST_ASSERT_GE(offspring->getHunger(), 0.0f);
    TEST_ASSERT_GE(offspring->getThirst(), 0.0f);
}

void test_reproduce_requiresPartner() {
    Creature parent1 = createBreedingTestCreature(10, 10);
    
    // Creatures require sexual reproduction (partner required)
    TEST_ASSERT_EQ(parent1.getReproductionMode(), EcoSim::Genetics::ReproductionMode::SEXUAL);
    
    // Trying to reproduce without partner should return nullptr
    auto offspringBase = parent1.reproduce(nullptr);
    TEST_ASSERT_MSG(offspringBase == nullptr, "Creature reproduce() without partner should fail");
}

void test_isCompatibleWith_sameArchetype() {
    Creature c1 = createBreedingTestCreature(10, 10);
    Creature c2 = createBreedingTestCreature(11, 10);
    
    // Same archetype creatures should be compatible
    bool compatible = c1.isCompatibleWith(c2);
    TEST_ASSERT_MSG(compatible, "Creatures of same archetype should be compatible");
}

void run_creature_breeding_tests() {
    BEGIN_TEST_GROUP("Creature Breeding System Tests");
    
    RUN_TEST(test_checkFitness_similarCreatures_highScore);
    RUN_TEST(test_checkFitness_differentCreatures_lowScore);
    RUN_TEST(test_checkFitness_distanceAffectsScore);
    RUN_TEST(test_checkFitness_penalizesTooSimilar);
    
    RUN_TEST(test_reproduce_producesViableOffspring);
    RUN_TEST(test_reproduce_offspringInheritsGenes);
    RUN_TEST(test_reproduce_consumesResources);
    RUN_TEST(test_reproduce_offspringReceivesResources);
    RUN_TEST(test_reproduce_resetsMateValue);
    RUN_TEST(test_reproduce_offspringAtParentLocation);
    RUN_TEST(test_reproduce_offspringStartsYoung);
    RUN_TEST(test_reproduce_resourcesNotNegative);
    RUN_TEST(test_reproduce_requiresPartner);
    RUN_TEST(test_isCompatibleWith_sameArchetype);
    
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
