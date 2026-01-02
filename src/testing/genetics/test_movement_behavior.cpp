/**
 * @file test_movement_behavior.cpp
 * @brief Tests for MovementBehavior class
 *
 * Tests cover:
 * - isApplicable: locomotion threshold and target state
 * - setTarget/clearTarget: target management
 * - calculateMovementCost: base and diagonal multiplier
 * - execute: movement toward target and stopping
 * - getMovementSpeed: phenotype-based speed
 * - priority: LOW priority value
 */

#include "test_framework.hpp"
#include "genetics/behaviors/MovementBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/interfaces/IPositionable.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <cmath>
#include <memory>

using namespace EcoSim::Testing;
using namespace EcoSim::Genetics;

class MockGeneticOrganism : public Organism {
public:
    MockGeneticOrganism()
        : Organism(10, 10, createGenome(), *getSharedRegistry())
        , worldX_(10.5f)
        , worldY_(10.5f)
    {
        EnvironmentState env;
        OrganismState state;
        state.age_normalized = 0.5f;
        state.health = 1.0f;
        state.energy_level = 0.5f;
        Organism::updatePhenotype();
    }
    
    ~MockGeneticOrganism() noexcept override = default;
    
    // IPositionable - world coordinates
    float getWorldX() const override { return worldX_; }
    float getWorldY() const override { return worldY_; }
    void setWorldPosition(float x, float y) override { worldX_ = x; worldY_ = y; }
    
    // ILifecycle
    unsigned int getMaxLifespan() const override { return 10000; }
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
    
    // Convenience method for tests
    void setPosition(float x, float y) { worldX_ = x; worldY_ = y; }
    
    void setLocomotion(float value) {
        if (getGenomeMutable().hasGene(UniversalGenes::LOCOMOTION)) {
            getGenomeMutable().getGeneMutable(UniversalGenes::LOCOMOTION).setAlleleValues(value);
            Organism::updatePhenotype();
        }
    }
    
    void setMetabolism(float value) {
        if (getGenomeMutable().hasGene(UniversalGenes::METABOLISM_RATE)) {
            getGenomeMutable().getGeneMutable(UniversalGenes::METABOLISM_RATE).setAlleleValues(value);
            Organism::updatePhenotype();
        }
    }

private:
    float worldX_;
    float worldY_;
    
    static GeneRegistry* getSharedRegistry() {
        static GeneRegistry registry;
        static bool initialized = false;
        if (!initialized) {
            UniversalGenes::registerDefaults(registry);
            initialized = true;
        }
        return &registry;
    }
    
    static Genome createGenome() {
        return UniversalGenes::createCreatureGenome(*getSharedRegistry());
    }
};

void test_isApplicable_trueWhenCanMove() {
    MockGeneticOrganism organism;
    organism.setLocomotion(0.5f);
    
    MovementBehavior behavior;
    BehaviorContext ctx;
    
    bool applicable = behavior.isApplicable(organism, ctx);
    
    TEST_ASSERT(applicable);
}

void test_isApplicable_falseWhenNoLocomotion() {
    MockGeneticOrganism organism;
    organism.setLocomotion(0.1f);
    
    MovementBehavior behavior;
    BehaviorContext ctx;
    
    bool applicable = behavior.isApplicable(organism, ctx);
    
    TEST_ASSERT(!applicable);
}

void test_setTarget_enablesMovement() {
    MovementBehavior behavior;
    
    TEST_ASSERT(!behavior.hasTarget());
    
    behavior.setTarget(20, 30);
    
    TEST_ASSERT(behavior.hasTarget());
    
    auto target = behavior.getTarget();
    TEST_ASSERT_EQ(target.first, 20);
    TEST_ASSERT_EQ(target.second, 30);
}

void test_clearTarget_disablesDirectedMovement() {
    MovementBehavior behavior;
    
    behavior.setTarget(20, 30);
    TEST_ASSERT(behavior.hasTarget());
    
    behavior.clearTarget();
    
    TEST_ASSERT(!behavior.hasTarget());
    
    auto target = behavior.getTarget();
    TEST_ASSERT_EQ(target.first, -1);
    TEST_ASSERT_EQ(target.second, -1);
}

void test_calculateMovementCost_baseCase() {
    MockGeneticOrganism organism;
    organism.setMetabolism(0.5f);
    
    MovementBehavior behavior;
    
    float cost = behavior.getEnergyCost(organism);
    
    TEST_ASSERT_GT(cost, 0.0f);
}

void test_calculateMovementCost_diagonalMultiplier() {
    MockGeneticOrganism organism;
    organism.setLocomotion(0.5f);
    organism.setMetabolism(0.5f);
    organism.setPosition(0.5f, 0.5f);
    
    MovementBehavior behavior;
    BehaviorContext ctx;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    behavior.setTarget(1, 1);
    BehaviorResult diagonalResult = behavior.execute(organism, ctx);
    float diagonalCost = diagonalResult.energyCost;
    
    organism.setPosition(0.5f, 0.5f);
    behavior.setTarget(1, 0);
    BehaviorResult cardinalResult = behavior.execute(organism, ctx);
    float cardinalCost = cardinalResult.energyCost;
    
    TEST_ASSERT_GT(diagonalCost, cardinalCost);
}

void test_execute_movesTowardTarget() {
    MockGeneticOrganism organism;
    organism.setLocomotion(0.8f);
    organism.setPosition(0.5f, 0.5f);
    
    MovementBehavior behavior;
    BehaviorContext ctx;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    behavior.setTarget(10, 0);
    
    float startX = organism.getWorldX();
    BehaviorResult result = behavior.execute(organism, ctx);
    
    TEST_ASSERT(result.executed);
    TEST_ASSERT_GT(organism.getWorldX(), startX);
}

void test_execute_stopsAtTarget() {
    MockGeneticOrganism organism;
    organism.setLocomotion(0.8f);
    organism.setPosition(10.5f, 10.5f);
    
    MovementBehavior behavior;
    BehaviorContext ctx;
    ctx.worldRows = 100;
    ctx.worldCols = 100;
    
    behavior.setTarget(10, 10);
    
    BehaviorResult result = behavior.execute(organism, ctx);
    
    TEST_ASSERT(result.completed);
    TEST_ASSERT(!behavior.hasTarget());
}

void test_getMovementSpeed_fromPhenotype() {
    MockGeneticOrganism slowOrganism;
    slowOrganism.setLocomotion(0.4f);
    
    MockGeneticOrganism fastOrganism;
    fastOrganism.setLocomotion(0.9f);
    
    MovementBehavior behavior;
    BehaviorContext ctx;
    
    float slowCost = behavior.getEnergyCost(slowOrganism);
    float fastCost = behavior.getEnergyCost(fastOrganism);
    
    TEST_ASSERT_GT(slowCost, 0.0f);
    TEST_ASSERT_GT(fastCost, 0.0f);
}

void test_priority_isLow() {
    MockGeneticOrganism organism;
    organism.setLocomotion(0.5f);
    
    MovementBehavior behavior;
    
    float priority = behavior.getPriority(organism);
    
    TEST_ASSERT_EQ(priority, static_cast<float>(BehaviorPriority::LOW));
}

void run_movement_behavior_tests() {
    BEGIN_TEST_GROUP("MovementBehavior Tests");
    
    RUN_TEST(test_isApplicable_trueWhenCanMove);
    RUN_TEST(test_isApplicable_falseWhenNoLocomotion);
    RUN_TEST(test_setTarget_enablesMovement);
    RUN_TEST(test_clearTarget_disablesDirectedMovement);
    RUN_TEST(test_calculateMovementCost_baseCase);
    RUN_TEST(test_calculateMovementCost_diagonalMultiplier);
    RUN_TEST(test_execute_movesTowardTarget);
    RUN_TEST(test_execute_stopsAtTarget);
    RUN_TEST(test_getMovementSpeed_fromPhenotype);
    RUN_TEST(test_priority_isLow);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running MovementBehavior Tests..." << std::endl;
    run_movement_behavior_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
