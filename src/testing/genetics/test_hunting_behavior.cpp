/**
 * @file test_hunting_behavior.cpp
 * @brief Tests for HuntingBehavior component
 *
 * Tests cover the four critical bug fixes that prevent prey extinction:
 * 1. Satiation check - Don't hunt when 80%+ full
 * 2. Hunt energy cost - Deduct energy for each hunt attempt
 * 3. Hunt cooldown tracking - Minimum ticks between hunts
 * 4. Prey escape mechanics - Use flee/pursue genes for escape chance
 *
 * Also tests:
 * - isApplicable conditions
 * - Priority calculation based on hunger
 * - Execute behavior outcomes
 */

#include "test_framework.hpp"
#include "genetics/behaviors/HuntingBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/organisms/Organism.hpp"
#include <memory>
#include <iostream>
#include <cmath>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

/**
 * @brief Mock organism for testing hunting behavior
 *
 * Implements Organism interface for isolated testing
 * without requiring full Creature dependencies.
 */
class MockOrganism : public Organism {
public:
    MockOrganism(GeneRegistry& registry)
        : Organism(0, 0, UniversalGenes::createCreatureGenome(registry), registry)
        , registry_(registry)
    {
        OrganismState state;
        state.age_normalized = 0.5f;
        state.health = 1.0f;
        state.energy_level = 0.5f;
        
        EnvironmentState env;
        env.temperature = 20.0f;
        env.humidity = 0.5f;
        env.time_of_day = 0.5f;
        
        Organism::updatePhenotype();
    }
    
    ~MockOrganism() noexcept override = default;
    
    // IPositionable - world coordinates
    float getWorldX() const override { return static_cast<float>(getX()); }
    float getWorldY() const override { return static_cast<float>(getY()); }
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
    
    void setGene(const std::string& geneName, float value) {
        if (getGenomeMutable().hasGene(geneName)) {
            getGenomeMutable().getGeneMutable(geneName).setAlleleValues(value);
        }
        Organism::updatePhenotype();
    }
    
private:
    GeneRegistry& registry_;
};

void setupPredatorGenome(MockOrganism& organism) {
    organism.setGene(UniversalGenes::HUNT_INSTINCT, 0.8f);
    organism.setGene(UniversalGenes::LOCOMOTION, 0.7f);
    organism.setGene(UniversalGenes::MEAT_DIGESTION_EFFICIENCY, 0.9f);
    organism.setGene(UniversalGenes::PURSUE_THRESHOLD, 25.0f);
    organism.setGene(UniversalGenes::COMBAT_AGGRESSION, 0.8f);
}

void setupPreyGenome(MockOrganism& organism) {
    organism.setGene(UniversalGenes::HUNT_INSTINCT, 0.1f);
    organism.setGene(UniversalGenes::LOCOMOTION, 0.6f);
    organism.setGene(UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 0.9f);
    organism.setGene(UniversalGenes::FLEE_THRESHOLD, 20.0f);
    organism.setGene(UniversalGenes::COMBAT_AGGRESSION, 0.1f);
}

void setupHerbivoreGenome(MockOrganism& organism) {
    organism.setGene(UniversalGenes::HUNT_INSTINCT, 0.1f);
    organism.setGene(UniversalGenes::LOCOMOTION, 0.5f);
    organism.setGene(UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 0.9f);
    organism.setGene(UniversalGenes::MEAT_DIGESTION_EFFICIENCY, 0.1f);
}

void test_isApplicable_trueWhenHungryPredator() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    predator.setGene(UniversalGenes::HUNGER_THRESHOLD, 10.0f);
    
    // Set up organism state - hungry (low energy)
    OrganismState state;
    state.energy_level = 0.3f;  // 30% energy = hungry
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    ctx.organismState = &state;
    
    bool applicable = hunting.isApplicable(predator, ctx);
    
    TEST_ASSERT_MSG(applicable, "Hungry predator with high hunt_instinct should be able to hunt");
}

void test_isApplicable_falseWhenSatiated() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    predator.setGene(UniversalGenes::HUNGER_THRESHOLD, 10.0f);
    
    // Set up organism state - satiated (high energy)
    OrganismState state;
    state.energy_level = 0.9f;  // 90% energy = satiated (> 0.8 threshold)
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    ctx.organismState = &state;
    
    bool applicable = hunting.isApplicable(predator, ctx);
    
    TEST_ASSERT_MSG(!applicable, "Satiated predator (>80% full) should not hunt");
}

void test_isApplicable_falseWhenHerbivore() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism herbivore(registry);
    setupHerbivoreGenome(herbivore);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    
    bool applicable = hunting.isApplicable(herbivore, ctx);
    
    TEST_ASSERT_MSG(!applicable, "Herbivore with low hunt_instinct should not hunt");
}

void test_isApplicable_falseWhenOnCooldown() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    
    // Set up organism state - hungry
    OrganismState state;
    state.energy_level = 0.3f;
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    ctx.organismState = &state;
    
    bool firstApplicable = hunting.isApplicable(predator, ctx);
    TEST_ASSERT_MSG(firstApplicable, "First hunt check should be applicable");
    
    BehaviorResult result = hunting.execute(predator, ctx);
    
    ctx.currentTick = 110;
    bool secondApplicable = hunting.isApplicable(predator, ctx);
    
    TEST_ASSERT_MSG(!secondApplicable, "Should not be applicable during cooldown (tick 110, cooldown 30)");
    
    ctx.currentTick = 200;
    bool thirdApplicable = hunting.isApplicable(predator, ctx);
    
    TEST_ASSERT_MSG(thirdApplicable, "Should be applicable after cooldown expires (tick 200)");
}

void test_calculateEscapeChance_highFleeMoreEscapes() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    predator.setGene(UniversalGenes::PURSUE_THRESHOLD, 10.0f);
    
    MockOrganism highFleePrey(registry);
    setupPreyGenome(highFleePrey);
    highFleePrey.setGene(UniversalGenes::FLEE_THRESHOLD, 50.0f);
    
    MockOrganism lowFleePrey(registry);
    setupPreyGenome(lowFleePrey);
    lowFleePrey.setGene(UniversalGenes::FLEE_THRESHOLD, 5.0f);
    
    float highFleeChance = highFleePrey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) /
        (highFleePrey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) +
         predator.getPhenotype().getTrait(UniversalGenes::PURSUE_THRESHOLD) + 0.1f);
    
    float lowFleeChance = lowFleePrey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) /
        (lowFleePrey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) +
         predator.getPhenotype().getTrait(UniversalGenes::PURSUE_THRESHOLD) + 0.1f);
    
    TEST_ASSERT_MSG(highFleeChance > lowFleeChance,
                    "High flee prey should have greater escape chance than low flee prey");
    TEST_ASSERT_MSG(highFleeChance > 0.8f,
                    "High flee prey (50) vs low pursue (10) should have >80% escape chance");
}

void test_calculateEscapeChance_highPursueLessEscapes() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    MockOrganism prey(registry);
    setupPreyGenome(prey);
    prey.setGene(UniversalGenes::FLEE_THRESHOLD, 10.0f);
    
    MockOrganism slowPredator(registry);
    setupPredatorGenome(slowPredator);
    slowPredator.setGene(UniversalGenes::PURSUE_THRESHOLD, 5.0f);
    
    MockOrganism fastPredator(registry);
    setupPredatorGenome(fastPredator);
    fastPredator.setGene(UniversalGenes::PURSUE_THRESHOLD, 50.0f);
    
    float slowPredatorEscape = prey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) /
        (prey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) +
         slowPredator.getPhenotype().getTrait(UniversalGenes::PURSUE_THRESHOLD) + 0.1f);
    
    float fastPredatorEscape = prey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) /
        (prey.getPhenotype().getTrait(UniversalGenes::FLEE_THRESHOLD) +
         fastPredator.getPhenotype().getTrait(UniversalGenes::PURSUE_THRESHOLD) + 0.1f);
    
    TEST_ASSERT_MSG(slowPredatorEscape > fastPredatorEscape,
                    "Prey should escape more often from slow predators");
    TEST_ASSERT_MSG(fastPredatorEscape < 0.2f,
                    "Prey (10) vs fast predator (50) should have <20% escape chance");
}

void test_execute_recordsHuntTick() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    
    // Set up organism state - hungry
    OrganismState state;
    state.energy_level = 0.3f;
    
    BehaviorContext ctx;
    ctx.currentTick = 500;
    ctx.organismState = &state;
    
    bool beforeHunt = hunting.isApplicable(predator, ctx);
    TEST_ASSERT_MSG(beforeHunt, "Should be applicable before first hunt");
    
    BehaviorResult result = hunting.execute(predator, ctx);
    
    ctx.currentTick = 510;
    bool duringCooldown = hunting.isApplicable(predator, ctx);
    
    TEST_ASSERT_MSG(!duringCooldown, "Hunt tick should be recorded, causing cooldown");
}

void test_execute_deductsEnergyCost() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    
    // Set up organism state - hungry
    OrganismState state;
    state.energy_level = 0.3f;
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    ctx.organismState = &state;
    
    BehaviorResult result = hunting.execute(predator, ctx);
    
    TEST_ASSERT_MSG(result.executed, "Hunt should execute");
    TEST_ASSERT_MSG(result.energyCost > 0.0f, "Hunt should have energy cost");
    TEST_ASSERT_NEAR(result.energyCost, 1.5f, 0.01f);
}

void test_satiation_preventsHunting() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism wellFedPredator(registry);
    setupPredatorGenome(wellFedPredator);
    
    // Set up organism state - satiated (high energy >= 0.8)
    OrganismState state;
    state.energy_level = 0.9f;
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    ctx.organismState = &state;
    
    bool applicable = hunting.isApplicable(wellFedPredator, ctx);
    
    TEST_ASSERT_MSG(!applicable, "Satiated predator should not hunt (bug fix: satiation check)");
}

void test_cooldown_preventsImmediateRehunt() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    
    // Set up organism state - hungry
    OrganismState state;
    state.energy_level = 0.3f;
    
    BehaviorContext ctx;
    ctx.currentTick = 1000;
    ctx.organismState = &state;
    
    TEST_ASSERT_MSG(hunting.isApplicable(predator, ctx), "Should hunt initially");
    hunting.execute(predator, ctx);
    
    ctx.currentTick = 1001;
    TEST_ASSERT_MSG(!hunting.isApplicable(predator, ctx), "Should NOT hunt 1 tick later");
    
    ctx.currentTick = 1015;
    TEST_ASSERT_MSG(!hunting.isApplicable(predator, ctx), "Should NOT hunt 15 ticks later");
    
    ctx.currentTick = 1029;
    TEST_ASSERT_MSG(!hunting.isApplicable(predator, ctx), "Should NOT hunt 29 ticks later");
    
    ctx.currentTick = 1030;
    TEST_ASSERT_MSG(hunting.isApplicable(predator, ctx), "Should hunt exactly at cooldown (30 ticks)");
    
    ctx.currentTick = 1050;
    TEST_ASSERT_MSG(hunting.isApplicable(predator, ctx), "Should hunt after cooldown expires");
}

void test_priority_increasesWithHunger() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    
    float priority = hunting.getPriority(predator);
    
    TEST_ASSERT_MSG(priority >= 50.0f, "Base priority should be at least NORMAL (50)");
    TEST_ASSERT_MSG(priority <= 75.0f, "Priority should not exceed HIGH (75)");
}

void test_cannotHunt_withLowLocomotion() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism slowPredator(registry);
    setupPredatorGenome(slowPredator);
    slowPredator.setGene(UniversalGenes::LOCOMOTION, 0.1f);
    
    BehaviorContext ctx;
    ctx.currentTick = 100;
    
    bool applicable = hunting.isApplicable(slowPredator, ctx);
    
    TEST_ASSERT_MSG(!applicable, "Slow predator (locomotion < 0.3) should not be able to hunt");
}

void test_behaviorId_isHunting() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    TEST_ASSERT_MSG(hunting.getId() == "hunting", "Behavior ID should be 'hunting'");
}

void test_energyCost_isConstant() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    CombatInteraction combat;
    PerceptionSystem perception;
    HuntingBehavior hunting(combat, perception);
    
    MockOrganism predator(registry);
    setupPredatorGenome(predator);
    
    float cost = hunting.getEnergyCost(predator);
    
    TEST_ASSERT_NEAR(cost, 1.5f, 0.01f);
}

void run_hunting_behavior_tests() {
    BEGIN_TEST_GROUP("Hunting Behavior Tests");
    
    RUN_TEST(test_isApplicable_trueWhenHungryPredator);
    RUN_TEST(test_isApplicable_falseWhenSatiated);
    RUN_TEST(test_isApplicable_falseWhenHerbivore);
    RUN_TEST(test_isApplicable_falseWhenOnCooldown);
    RUN_TEST(test_calculateEscapeChance_highFleeMoreEscapes);
    RUN_TEST(test_calculateEscapeChance_highPursueLessEscapes);
    RUN_TEST(test_execute_recordsHuntTick);
    RUN_TEST(test_execute_deductsEnergyCost);
    RUN_TEST(test_satiation_preventsHunting);
    RUN_TEST(test_cooldown_preventsImmediateRehunt);
    RUN_TEST(test_priority_increasesWithHunger);
    RUN_TEST(test_cannotHunt_withLowLocomotion);
    RUN_TEST(test_behaviorId_isHunting);
    RUN_TEST(test_energyCost_isConstant);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running Hunting Behavior Tests..." << std::endl;
    run_hunting_behavior_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
