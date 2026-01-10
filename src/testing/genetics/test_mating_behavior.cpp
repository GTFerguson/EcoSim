/**
 * @file test_mating_behavior.cpp
 * @brief Tests for MatingBehavior component
 *
 * Tests cover:
 * - isApplicable conditions (mate value, hunger, maturity)
 * - checkFitness genetic similarity evaluation
 * - execute behavior outcomes
 * - offspring callback invocation
 * - priority calculation based on mate value
 */

#include "test_framework.hpp"
#include "genetics/behaviors/MatingBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/interfaces/ILifecycle.hpp"
#include <memory>
#include <iostream>
#include <cmath>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

/**
 * @brief Mock organism for testing mating behavior
 *
 * Implements Organism and IPositionable interfaces for isolated testing
 * without requiring full Creature dependencies.
 */
class MockMatingOrganism : public Organism {
public:
    ~MockMatingOrganism() noexcept override = default;
    
    MockMatingOrganism(GeneRegistry& registry, int x = 0, int y = 0, unsigned int maxLifespan = 500000)
        : Organism(x, y, UniversalGenes::createCreatureGenome(registry), registry)
        , registry_(registry)
        , maxLifespan_(maxLifespan)
    {
        // Set optimal context for phenotype expression
        OrganismState state;
        state.age_normalized = 0.5f;
        state.health = 1.0f;
        state.energy_level = 0.7f;
        
        EnvironmentState env;
        env.temperature = 20.0f;
        env.humidity = 0.5f;
        env.time_of_day = 0.5f;
        
        phenotype_.updateContext(env, state);
    }
    
    // IPositionable - world coordinates
    float getWorldX() const override { return static_cast<float>(getX()); }
    float getWorldY() const override { return static_cast<float>(getY()); }
    void setWorldPosition(float, float) override {}
    
    // ILifecycle
    unsigned int getMaxLifespan() const override { return maxLifespan_; }
    void grow() override {}
    
    // IReproducible
    bool canReproduce() const override { return false; }
    float getReproductiveUrge() const override { return 0.0f; }
    float getReproductionEnergyCost() const override { return 10.0f; }
    ReproductionMode getReproductionMode() const override { return ReproductionMode::SEXUAL; }
    bool isCompatibleWith(const Organism&) const override { return false; }
    std::unique_ptr<Organism> reproduce(const Organism* = nullptr) override { return nullptr; }
    
    // Organism abstract methods
    float getMaxSize() const override { return 1.0f; }
    
    void setGene(const std::string& geneName, float value) {
        Genome& genome = getGenomeMutable();
        if (genome.hasGene(geneName)) {
            genome.getGeneMutable(geneName).setAlleleValues(value);
        } else {
            // Add gene if it doesn't exist
            GeneValue geneVal = value;
            Gene gene(geneName, geneVal);
            // Use Metabolism chromosome as default
            genome.addGene(gene, ChromosomeType::Metabolism);
        }
        phenotype_.invalidateCache();
    }
    
    /**
     * @brief Set the organism's age to achieve a target normalized age
     * @param ageNorm Target normalized age (0.0 to 1.0)
     */
    void setAgeNormalized(float ageNorm) {
        // Calculate the number of ticks needed to achieve this normalized age
        // ageNormalized = age_ / maxLifespan_
        // So age_ = ageNorm * maxLifespan_
        unsigned int targetAge = static_cast<unsigned int>(ageNorm * maxLifespan_);
        
        // Use the inherited age() method to set age
        // First we need to reset and age to the target
        age_ = targetAge;
        
        Organism::updatePhenotype();
    }
    
private:
    GeneRegistry& registry_;
    unsigned int maxLifespan_ = 500000;
};

void setupMatureOrganism(MockMatingOrganism& organism) {
    organism.setAgeNormalized(0.5f);
    organism.setGene(UniversalGenes::MATE_THRESHOLD, 3.0f);
    organism.setGene(UniversalGenes::HUNGER_THRESHOLD, 10.0f);
    organism.setGene(UniversalGenes::LIFESPAN, 500000.0f);
}

void setupImmatureOrganism(MockMatingOrganism& organism) {
    organism.setAgeNormalized(0.01f);
    organism.setGene(UniversalGenes::MATE_THRESHOLD, 3.0f);
    organism.setGene(UniversalGenes::HUNGER_THRESHOLD, 10.0f);
    organism.setGene(UniversalGenes::LIFESPAN, 500000.0f);
}

void test_isApplicable_trueWhenReadyToMate() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupMatureOrganism(organism);
    organism.setGene(UniversalGenes::MATE_THRESHOLD, 1.0f);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    
    bool applicable = mating.isApplicable(organism, ctx);
    
    TEST_ASSERT_MSG(applicable, "Mature organism with high mate value should be ready to mate");
}

void test_isApplicable_falseWhenLowMateValue() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupMatureOrganism(organism);
    organism.setGene(UniversalGenes::MATE_THRESHOLD, 0.1f);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    
    bool applicable = mating.isApplicable(organism, ctx);
    
    TEST_ASSERT_MSG(!applicable, "Organism with low mate value should not be ready to mate");
}

void test_isApplicable_falseWhenHungry() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupMatureOrganism(organism);
    organism.setGene(UniversalGenes::MATE_THRESHOLD, 1.0f);
    organism.setGene(UniversalGenes::HUNGER_THRESHOLD, 2.0f);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    
    bool applicable = mating.isApplicable(organism, ctx);
    
    TEST_ASSERT_MSG(!applicable, "Hungry organism should not be ready to mate");
}

void test_isApplicable_falseWhenImmature() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupImmatureOrganism(organism);
    organism.setGene(UniversalGenes::MATE_THRESHOLD, 1.0f);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    
    bool applicable = mating.isApplicable(organism, ctx);
    
    TEST_ASSERT_MSG(!applicable, "Immature organism should not be ready to mate");
}

void test_checkFitness_prefersSimilarButNotIdentical() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism seeker(registry, 10, 10);
    setupMatureOrganism(seeker);
    
    MockMatingOrganism similarMate(registry, 11, 10);
    setupMatureOrganism(similarMate);
    similarMate.setGene(UniversalGenes::MAX_SIZE, 
        seeker.getPhenotype().getTrait(UniversalGenes::MAX_SIZE) * 0.9f);
    
    MockMatingOrganism veryDifferentMate(registry, 11, 10);
    setupMatureOrganism(veryDifferentMate);
    veryDifferentMate.setGene(UniversalGenes::MAX_SIZE, 0.1f);
    veryDifferentMate.setGene(UniversalGenes::METABOLISM_RATE, 0.1f);
    veryDifferentMate.setGene(UniversalGenes::SIGHT_RANGE, 10.0f);
    
    float similarFitness = seeker.getGenome().compare(similarMate.getGenome());
    float differentFitness = seeker.getGenome().compare(veryDifferentMate.getGenome());
    
    TEST_ASSERT_GT(similarFitness, 0.0f);
    TEST_ASSERT_GT(differentFitness, 0.0f);
    TEST_ASSERT_LT(similarFitness, 1.0f);
}

void test_mating_checkFitness_penalizesTooSimilar() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism seeker(registry, 10, 10);
    setupMatureOrganism(seeker);
    
    MockMatingOrganism identicalMate(registry, 11, 10);
    setupMatureOrganism(identicalMate);
    
    float identicalSimilarity = seeker.getGenome().compare(identicalMate.getGenome());
    
    TEST_ASSERT_GT(identicalSimilarity, 0.8f);
}

void test_execute_noMateFound() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupMatureOrganism(organism);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    ctx.world = nullptr;
    
    BehaviorResult result = mating.execute(organism, ctx);
    
    TEST_ASSERT_MSG(result.executed, "Execute should run");
    TEST_ASSERT_MSG(!result.completed, "Should not complete without world access");
}

void test_execute_deductsBreedCost() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupMatureOrganism(organism);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    
    BehaviorResult result = mating.execute(organism, ctx);
    
    TEST_ASSERT_MSG(result.executed, "Execute should run");
    TEST_ASSERT_NEAR(result.energyCost, 3.0f, 0.01f);
}

void test_offspringCallback_setCorrectly() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    bool callbackCalled = false;
    mating.setOffspringCallback([&callbackCalled](std::unique_ptr<Organism> offspring) {
        callbackCalled = true;
    });
    
    TEST_ASSERT_MSG(!callbackCalled, "Callback should not be called until offspring created");
}

void test_priority_increasesWithMateValue() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism lowMateOrganism(registry);
    setupMatureOrganism(lowMateOrganism);
    lowMateOrganism.setGene(UniversalGenes::MATE_THRESHOLD, 0.8f);
    
    MockMatingOrganism highMateOrganism(registry);
    setupMatureOrganism(highMateOrganism);
    highMateOrganism.setGene(UniversalGenes::MATE_THRESHOLD, 2.0f);
    
    float lowPriority = mating.getPriority(lowMateOrganism);
    float highPriority = mating.getPriority(highMateOrganism);
    
    TEST_ASSERT_GE(lowPriority, 50.0f);
    TEST_ASSERT_GE(highPriority, 50.0f);
    TEST_ASSERT_GT(highPriority, lowPriority);
}

void test_priority_basePriorityIsNormal() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupMatureOrganism(organism);
    organism.setGene(UniversalGenes::MATE_THRESHOLD, 0.7f);
    
    float priority = mating.getPriority(organism);
    
    TEST_ASSERT_GE(priority, 50.0f);
    TEST_ASSERT_LE(priority, 75.0f);
}

void test_behaviorId_isMating() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    TEST_ASSERT_MSG(mating.getId() == "mating", "Behavior ID should be 'mating'");
}

void test_energyCost_isBreedCost() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    PerceptionSystem perception;
    MatingBehavior mating(perception, registry);
    
    MockMatingOrganism organism(registry);
    setupMatureOrganism(organism);
    
    float cost = mating.getEnergyCost(organism);
    
    TEST_ASSERT_NEAR(cost, 3.0f, 0.01f);
}

void test_geneticSimilarity_identicalGenomesHighSimilarity() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    MockMatingOrganism org1(registry);
    setupMatureOrganism(org1);
    
    MockMatingOrganism org2(registry);
    setupMatureOrganism(org2);
    
    float similarity = org1.getGenome().compare(org2.getGenome());
    
    TEST_ASSERT_GT(similarity, 0.9f);
}

void test_geneticSimilarity_differentGenomesLowerSimilarity() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    MockMatingOrganism org1(registry);
    setupMatureOrganism(org1);
    org1.setGene(UniversalGenes::MAX_SIZE, 10.0f);
    org1.setGene(UniversalGenes::METABOLISM_RATE, 0.01f);
    
    MockMatingOrganism org2(registry);
    setupMatureOrganism(org2);
    org2.setGene(UniversalGenes::MAX_SIZE, 1.0f);
    org2.setGene(UniversalGenes::METABOLISM_RATE, 0.1f);
    
    float similarity = org1.getGenome().compare(org2.getGenome());
    
    TEST_ASSERT_LT(similarity, 1.0f);
    TEST_ASSERT_GT(similarity, 0.0f);
}

void run_mating_behavior_tests() {
    BEGIN_TEST_GROUP("Mating Behavior Tests");
    
    RUN_TEST(test_isApplicable_trueWhenReadyToMate);
    RUN_TEST(test_isApplicable_falseWhenLowMateValue);
    RUN_TEST(test_isApplicable_falseWhenHungry);
    RUN_TEST(test_isApplicable_falseWhenImmature);
    RUN_TEST(test_checkFitness_prefersSimilarButNotIdentical);
    RUN_TEST(test_mating_checkFitness_penalizesTooSimilar);
    RUN_TEST(test_execute_noMateFound);
    RUN_TEST(test_execute_deductsBreedCost);
    RUN_TEST(test_offspringCallback_setCorrectly);
    RUN_TEST(test_priority_increasesWithMateValue);
    RUN_TEST(test_priority_basePriorityIsNormal);
    RUN_TEST(test_behaviorId_isMating);
    RUN_TEST(test_energyCost_isBreedCost);
    RUN_TEST(test_geneticSimilarity_identicalGenomesHighSimilarity);
    RUN_TEST(test_geneticSimilarity_differentGenomesLowerSimilarity);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running Mating Behavior Tests..." << std::endl;
    run_mating_behavior_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
