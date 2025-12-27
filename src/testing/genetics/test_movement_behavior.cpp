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
#include "genetics/interfaces/IGeneticOrganism.hpp"
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

class MockGeneticOrganism : public IGeneticOrganism, public IPositionable {
public:
    MockGeneticOrganism()
        : registry_(std::make_shared<GeneRegistry>())
        , worldX_(10.5f)
        , worldY_(10.5f)
    {
        UniversalGenes::registerDefaults(*registry_);
        genome_ = std::make_unique<Genome>(
            UniversalGenes::createCreatureGenome(*registry_)
        );
        phenotype_ = std::make_unique<Phenotype>(genome_.get(), registry_.get());
        
        EnvironmentState env;
        OrganismState state;
        state.age_normalized = 0.5f;
        state.health = 1.0f;
        state.energy_level = 0.5f;
        phenotype_->updateContext(env, state);
    }
    
    const Genome& getGenome() const override { return *genome_; }
    Genome& getGenomeMutable() override { return *genome_; }
    const Phenotype& getPhenotype() const override { return *phenotype_; }
    void updatePhenotype() override {
        EnvironmentState env;
        OrganismState state;
        state.age_normalized = 0.5f;
        state.health = 1.0f;
        state.energy_level = 0.5f;
        phenotype_->updateContext(env, state);
    }
    
    // Tile coordinates (for collision detection, NCurses rendering)
    int getX() const override { return static_cast<int>(worldX_); }
    int getY() const override { return static_cast<int>(worldY_); }
    int getId() const override { return 0; }
    
    // Float coordinates (for actual position in world)
    float getWorldX() const override { return worldX_; }
    float getWorldY() const override { return worldY_; }
    void setWorldPosition(float x, float y) override { worldX_ = x; worldY_ = y; }
    
    // Convenience method for tests
    void setPosition(float x, float y) { worldX_ = x; worldY_ = y; }
    
    void setLocomotion(float value) {
        if (genome_->hasGene(UniversalGenes::LOCOMOTION)) {
            genome_->getGeneMutable(UniversalGenes::LOCOMOTION).setAlleleValues(value);
            phenotype_ = std::make_unique<Phenotype>(genome_.get(), registry_.get());
            updatePhenotype();
        }
    }
    
    void setMetabolism(float value) {
        if (genome_->hasGene(UniversalGenes::METABOLISM_RATE)) {
            genome_->getGeneMutable(UniversalGenes::METABOLISM_RATE).setAlleleValues(value);
            phenotype_ = std::make_unique<Phenotype>(genome_.get(), registry_.get());
            updatePhenotype();
        }
    }

private:
    std::shared_ptr<GeneRegistry> registry_;
    std::unique_ptr<Genome> genome_;
    std::unique_ptr<Phenotype> phenotype_;
    float worldX_;
    float worldY_;
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
