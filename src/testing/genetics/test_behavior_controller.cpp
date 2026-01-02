/**
 * @file test_behavior_controller.cpp
 * @brief Tests for BehaviorController
 *
 * Tests cover:
 * - Adding and removing behaviors
 * - Behavior existence checking
 * - Priority-based behavior execution
 * - Non-applicable behavior filtering
 * - Current behavior tracking
 */

#include "test_framework.hpp"
#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"
#include <memory>
#include <iostream>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//==============================================================================
// Mock Classes
//==============================================================================

/**
 * @brief Mock behavior for testing
 * 
 * Configurable applicability, priority, and execution results.
 */
class MockBehavior : public IBehavior {
public:
    MockBehavior(const std::string& id, float priority, bool applicable = true)
        : id_(id), priority_(priority), applicable_(applicable) {}
    
    std::string getId() const override { return id_; }
    
    bool isApplicable(const Organism& /*organism*/,
                      const BehaviorContext& /*ctx*/) const override {
        return applicable_;
    }
    
    float getPriority(const Organism& /*organism*/) const override {
        return priority_;
    }
    
    BehaviorResult execute(Organism& /*organism*/,
                          BehaviorContext& /*ctx*/) override {
        executionCount_++;
        return BehaviorResult{true, true, 1.0f, "MockBehavior executed: " + id_};
    }
    
    float getEnergyCost(const Organism& /*organism*/) const override {
        return 1.0f;
    }
    
    int getExecutionCount() const { return executionCount_; }
    void setApplicable(bool applicable) { applicable_ = applicable; }
    void setPriority(float priority) { priority_ = priority; }
    
private:
    std::string id_;
    float priority_;
    bool applicable_;
    int executionCount_ = 0;
};

/**
 * @brief Minimal mock organism for testing
 */
class MockOrganism : public Organism {
public:
    MockOrganism()
        : Organism(0, 0, Genome(), registry_)
        , phenotype_(&getGenomeMutable(), &registry_)
    {}
    ~MockOrganism() override = default;
    
    // IGenetic
    const Phenotype& getPhenotype() const override { return phenotype_; }
    void updatePhenotype() override {}

    // IPositionable - world coordinates
    float getWorldX() const override { return 0.0f; }
    float getWorldY() const override { return 0.0f; }
    void setWorldPosition(float, float) override {}
    
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
    
private:
    static inline GeneRegistry registry_;
    Phenotype phenotype_;
};

//==============================================================================
// Test Functions
//==============================================================================

void test_addBehavior_increasesBehaviorCount() {
    BehaviorController controller;
    
    TEST_ASSERT_EQ(0u, controller.getBehaviorCount());
    
    controller.addBehavior(std::make_unique<MockBehavior>("test1", 50.0f));
    TEST_ASSERT_EQ(1u, controller.getBehaviorCount());
    
    controller.addBehavior(std::make_unique<MockBehavior>("test2", 50.0f));
    TEST_ASSERT_EQ(2u, controller.getBehaviorCount());
}

void test_removeBehavior_decreasesBehaviorCount() {
    BehaviorController controller;
    
    controller.addBehavior(std::make_unique<MockBehavior>("test1", 50.0f));
    controller.addBehavior(std::make_unique<MockBehavior>("test2", 50.0f));
    TEST_ASSERT_EQ(2u, controller.getBehaviorCount());
    
    controller.removeBehavior("test1");
    TEST_ASSERT_EQ(1u, controller.getBehaviorCount());
    
    // Removing non-existent should not change count
    controller.removeBehavior("nonexistent");
    TEST_ASSERT_EQ(1u, controller.getBehaviorCount());
}

void test_hasBehavior_returnsTrueWhenPresent() {
    BehaviorController controller;
    
    controller.addBehavior(std::make_unique<MockBehavior>("hunting", 50.0f));
    
    TEST_ASSERT(controller.hasBehavior("hunting"));
}

void test_hasBehavior_returnsFalseWhenAbsent() {
    BehaviorController controller;
    
    controller.addBehavior(std::make_unique<MockBehavior>("hunting", 50.0f));
    
    TEST_ASSERT(!controller.hasBehavior("mating"));
    TEST_ASSERT(!controller.hasBehavior(""));
}

void test_update_executesHighestPriority() {
    BehaviorController controller;
    MockOrganism organism;
    BehaviorContext ctx;
    
    // Create behaviors with different priorities
    auto lowPriority = std::make_unique<MockBehavior>("low", 25.0f);
    auto medPriority = std::make_unique<MockBehavior>("medium", 50.0f);
    auto highPriority = std::make_unique<MockBehavior>("high", 75.0f);
    
    // Keep raw pointers before moving
    MockBehavior* lowPtr = lowPriority.get();
    MockBehavior* medPtr = medPriority.get();
    MockBehavior* highPtr = highPriority.get();
    
    // Add in random order
    controller.addBehavior(std::move(medPriority));
    controller.addBehavior(std::move(lowPriority));
    controller.addBehavior(std::move(highPriority));
    
    // Execute
    BehaviorResult result = controller.update(organism, ctx);
    
    // High priority should execute
    TEST_ASSERT(result.executed);
    TEST_ASSERT_EQ(1, highPtr->getExecutionCount());
    TEST_ASSERT_EQ(0, medPtr->getExecutionCount());
    TEST_ASSERT_EQ(0, lowPtr->getExecutionCount());
}

void test_update_skipsNonApplicable() {
    BehaviorController controller;
    MockOrganism organism;
    BehaviorContext ctx;
    
    // High priority but not applicable
    auto highNotApplicable = std::make_unique<MockBehavior>("high", 100.0f, false);
    // Lower priority but applicable
    auto lowApplicable = std::make_unique<MockBehavior>("low", 25.0f, true);
    
    MockBehavior* highPtr = highNotApplicable.get();
    MockBehavior* lowPtr = lowApplicable.get();
    
    controller.addBehavior(std::move(highNotApplicable));
    controller.addBehavior(std::move(lowApplicable));
    
    BehaviorResult result = controller.update(organism, ctx);
    
    // Low priority should execute because high is not applicable
    TEST_ASSERT(result.executed);
    TEST_ASSERT_EQ(0, highPtr->getExecutionCount());
    TEST_ASSERT_EQ(1, lowPtr->getExecutionCount());
}

void test_update_returnsNotExecutedWhenNoBehaviors() {
    BehaviorController controller;
    MockOrganism organism;
    BehaviorContext ctx;
    
    BehaviorResult result = controller.update(organism, ctx);
    
    TEST_ASSERT(!result.executed);
    TEST_ASSERT(!result.completed);
}

void test_update_returnsNotExecutedWhenNoneApplicable() {
    BehaviorController controller;
    MockOrganism organism;
    BehaviorContext ctx;
    
    // Add behaviors that are all non-applicable
    controller.addBehavior(std::make_unique<MockBehavior>("test1", 50.0f, false));
    controller.addBehavior(std::make_unique<MockBehavior>("test2", 75.0f, false));
    
    BehaviorResult result = controller.update(organism, ctx);
    
    TEST_ASSERT(!result.executed);
}

void test_update_updatesCurrentBehaviorId() {
    BehaviorController controller;
    MockOrganism organism;
    BehaviorContext ctx;
    
    TEST_ASSERT(controller.getCurrentBehaviorId().empty());
    
    controller.addBehavior(std::make_unique<MockBehavior>("hunting", 50.0f));
    controller.update(organism, ctx);
    
    TEST_ASSERT_EQ(std::string("hunting"), controller.getCurrentBehaviorId());
    
    // Add higher priority and update
    controller.addBehavior(std::make_unique<MockBehavior>("fleeing", 100.0f));
    controller.update(organism, ctx);
    
    TEST_ASSERT_EQ(std::string("fleeing"), controller.getCurrentBehaviorId());
}

void test_clearBehaviors_removesAll() {
    BehaviorController controller;
    
    controller.addBehavior(std::make_unique<MockBehavior>("test1", 50.0f));
    controller.addBehavior(std::make_unique<MockBehavior>("test2", 50.0f));
    controller.addBehavior(std::make_unique<MockBehavior>("test3", 50.0f));
    
    TEST_ASSERT_EQ(3u, controller.getBehaviorCount());
    
    controller.clearBehaviors();
    
    TEST_ASSERT_EQ(0u, controller.getBehaviorCount());
    TEST_ASSERT(!controller.hasBehavior("test1"));
    TEST_ASSERT(!controller.hasBehavior("test2"));
    TEST_ASSERT(!controller.hasBehavior("test3"));
}

void test_getBehaviorIds_returnsAllIds() {
    BehaviorController controller;
    
    controller.addBehavior(std::make_unique<MockBehavior>("alpha", 50.0f));
    controller.addBehavior(std::make_unique<MockBehavior>("beta", 50.0f));
    controller.addBehavior(std::make_unique<MockBehavior>("gamma", 50.0f));
    
    std::vector<std::string> ids = controller.getBehaviorIds();
    
    TEST_ASSERT_EQ(3u, ids.size());
    
    // Check all expected IDs are present (order matches insertion order)
    TEST_ASSERT_EQ(std::string("alpha"), ids[0]);
    TEST_ASSERT_EQ(std::string("beta"), ids[1]);
    TEST_ASSERT_EQ(std::string("gamma"), ids[2]);
}

void test_getStatusString_returnsFormattedString() {
    BehaviorController controller;
    MockOrganism organism;
    BehaviorContext ctx;
    
    controller.addBehavior(std::make_unique<MockBehavior>("hunting", 50.0f));
    controller.update(organism, ctx);
    
    std::string status = controller.getStatusString();
    
    // Should contain count and current behavior
    TEST_ASSERT(status.find("1") != std::string::npos);
    TEST_ASSERT(status.find("hunting") != std::string::npos);
}

void test_stableSortMaintainsInsertionOrder() {
    BehaviorController controller;
    MockOrganism organism;
    BehaviorContext ctx;
    
    // Add behaviors with equal priority - first added should win
    auto first = std::make_unique<MockBehavior>("first", 50.0f);
    auto second = std::make_unique<MockBehavior>("second", 50.0f);
    auto third = std::make_unique<MockBehavior>("third", 50.0f);
    
    MockBehavior* firstPtr = first.get();
    MockBehavior* secondPtr = second.get();
    MockBehavior* thirdPtr = third.get();
    
    controller.addBehavior(std::move(first));
    controller.addBehavior(std::move(second));
    controller.addBehavior(std::move(third));
    
    controller.update(organism, ctx);
    
    // First one added should execute due to stable sort
    TEST_ASSERT_EQ(1, firstPtr->getExecutionCount());
    TEST_ASSERT_EQ(0, secondPtr->getExecutionCount());
    TEST_ASSERT_EQ(0, thirdPtr->getExecutionCount());
    TEST_ASSERT_EQ(std::string("first"), controller.getCurrentBehaviorId());
}

//==============================================================================
// Main test runner
//==============================================================================

void run_behavior_controller_tests() {
    BEGIN_TEST_GROUP("BehaviorController Tests");
    
    RUN_TEST(test_addBehavior_increasesBehaviorCount);
    RUN_TEST(test_removeBehavior_decreasesBehaviorCount);
    RUN_TEST(test_hasBehavior_returnsTrueWhenPresent);
    RUN_TEST(test_hasBehavior_returnsFalseWhenAbsent);
    RUN_TEST(test_update_executesHighestPriority);
    RUN_TEST(test_update_skipsNonApplicable);
    RUN_TEST(test_update_returnsNotExecutedWhenNoBehaviors);
    RUN_TEST(test_update_returnsNotExecutedWhenNoneApplicable);
    RUN_TEST(test_update_updatesCurrentBehaviorId);
    RUN_TEST(test_clearBehaviors_removesAll);
    RUN_TEST(test_getBehaviorIds_returnsAllIds);
    RUN_TEST(test_getStatusString_returnsFormattedString);
    RUN_TEST(test_stableSortMaintainsInsertionOrder);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running BehaviorController Tests..." << std::endl;
    run_behavior_controller_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
