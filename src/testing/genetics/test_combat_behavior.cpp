/**
 * @file test_combat_behavior.cpp
 * @brief Tests for combat behavior system
 *
 * Tests cover:
 * - Combat initiation and targeting
 * - Attack damage application
 * - Retreat threshold based on health
 * - Corpse creation on death
 * - Scavenging behavior
 * - Body condition affecting corpse nutrition
 */

#include "test_framework.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/interactions/CombatAction.hpp"
#include "genetics/interactions/DamageTypes.hpp"
#include "world/Corpse.hpp"
#include <memory>
#include <iostream>
#include <cmath>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//==============================================================================
// Test Helpers
//==============================================================================

/**
 * Create a phenotype for testing combat calculations.
 */
Phenotype createCombatPhenotype(
    GeneRegistry& registry,
    float aggression,
    float meatDigestion,
    float toothSharpness = 0.5f
) {
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    
    // Set combat-relevant genes
    if (genome.hasGene(UniversalGenes::COMBAT_AGGRESSION)) {
        genome.setGeneValue(UniversalGenes::COMBAT_AGGRESSION, aggression);
    }
    if (genome.hasGene(UniversalGenes::MEAT_DIGESTION_EFFICIENCY)) {
        genome.setGeneValue(UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatDigestion);
    }
    if (genome.hasGene(UniversalGenes::TOOTH_SHARPNESS)) {
        genome.setGeneValue(UniversalGenes::TOOTH_SHARPNESS, toothSharpness);
    }
    
    Phenotype phenotype(&genome, &registry);
    return phenotype;
}

//==============================================================================
// Combat Initiation Tests
//==============================================================================

void test_combat_initiation() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // High aggression predator
    Phenotype predatorPhenotype = createCombatPhenotype(registry, 0.9f, 0.9f, 0.8f);
    
    // Low aggression prey
    Phenotype preyPhenotype = createCombatPhenotype(registry, 0.1f, 0.2f, 0.2f);
    
    // Test 1: Hungry predator should initiate combat
    float hungerLevel = 0.8f;  // Very hungry (low food)
    bool shouldAttack = CombatInteraction::shouldInitiateCombat(
        predatorPhenotype, preyPhenotype, hungerLevel);
    
    TEST_ASSERT_MSG(shouldAttack, "Hungry predator with high aggression should initiate combat");
}

//==============================================================================
// Attack Damage Tests
//==============================================================================

void test_attack_damage() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create attacker with high damage traits
    Phenotype attackerPhenotype = createCombatPhenotype(registry, 0.9f, 0.9f, 0.9f);
    
    // Create defender with moderate defense
    Phenotype defenderPhenotype = createCombatPhenotype(registry, 0.3f, 0.3f, 0.3f);
    
    // Select best attack action
    CombatAction action = CombatInteraction::selectBestAction(
        attackerPhenotype, defenderPhenotype);
    
    // Resolve attack
    AttackResult result = CombatInteraction::resolveAttack(
        attackerPhenotype, defenderPhenotype, action);
    
    TEST_ASSERT_MSG(result.finalDamage > 0.0f, "Attack should deal positive damage");
}

//==============================================================================
// Retreat Threshold Tests
//==============================================================================

void test_retreat_threshold() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create phenotype with specific retreat threshold
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    
    // Set retreat threshold to 30%
    if (genome.hasGene(UniversalGenes::RETREAT_THRESHOLD)) {
        genome.setGeneValue(UniversalGenes::RETREAT_THRESHOLD, 0.3f);
    }
    
    Phenotype phenotype(&genome, &registry);
    
    // Test at various health levels
    bool shouldRetreat80 = CombatInteraction::shouldRetreat(phenotype, 0.8f);
    bool shouldRetreat20 = CombatInteraction::shouldRetreat(phenotype, 0.2f);
    
    TEST_ASSERT_MSG(!shouldRetreat80, "Should not retreat at 80% health");
    TEST_ASSERT_MSG(shouldRetreat20, "Should retreat at 20% health (below 30% threshold)");
}

//==============================================================================
// Corpse Creation Tests
//==============================================================================

void test_corpse_creation() {
    float x = 15.5f;
    float y = 22.3f;
    float size = 3.0f;
    std::string species = "TestCreature";
    float bodyCondition = 0.7f;
    
    world::Corpse corpse(x, y, size, species, bodyCondition);
    
    TEST_ASSERT_NEAR(corpse.getX(), x, 0.001f);
    TEST_ASSERT_NEAR(corpse.getY(), y, 0.001f);
    TEST_ASSERT_MSG(!corpse.isExhausted(), "Fresh corpse should not be exhausted");
    
    // Extract some nutrition
    float extracted = corpse.extractNutrition(10.0f);
    TEST_ASSERT_MSG(extracted > 0.0f, "Should be able to extract nutrition from fresh corpse");
}

//==============================================================================
// Scavenging Tests
//==============================================================================

void test_scavenging() {
    // Create a corpse
    float corpseX = 10.0f;
    float corpseY = 10.0f;
    float corpseSize = 2.0f;
    
    world::Corpse corpse(corpseX, corpseY, corpseSize, "DeadCreature", 0.5f);
    
    // Fresh corpse should have low toxicity
    TEST_ASSERT_MSG(corpse.getToxicity() < 1.0f, "Fresh corpse should have low toxicity");
    
    // Age the corpse
    for (int i = 0; i < 100; i++) {
        corpse.tick();
    }
    
    // Aged corpse should have higher toxicity
    TEST_ASSERT_MSG(corpse.getToxicity() > 0.0f, "Aged corpse should have some toxicity");
    
    // Extract nutrition
    float nutrition = corpse.extractNutrition(5.0f);
    TEST_ASSERT_MSG(nutrition >= 0.0f, "Should be able to extract nutrition from corpse");
}

//==============================================================================
// Body Condition Nutrition Tests
//==============================================================================

void test_body_condition_nutrition() {
    float x = 10.0f;
    float y = 10.0f;
    float size = 2.0f;
    std::string species = "TestCreature";
    
    // Create well-fed corpse (bodyCondition = 1.0)
    world::Corpse wellFedCorpse(x, y, size, species, 1.0f);
    
    // Create starved corpse (bodyCondition = 0.0)
    world::Corpse starvedCorpse(x, y, size, species, 0.0f);
    
    // Create normal corpse (bodyCondition = 0.5, default)
    world::Corpse normalCorpse(x, y, size, species, 0.5f);
    
    // Extract large amounts to get total nutrition
    float wellFedNutrition = wellFedCorpse.extractNutrition(1000.0f);
    float starvedNutrition = starvedCorpse.extractNutrition(1000.0f);
    float normalNutrition = normalCorpse.extractNutrition(1000.0f);
    
    // Well-fed corpse should provide more nutrition
    TEST_ASSERT_MSG(wellFedNutrition > starvedNutrition,
                    "Well-fed corpse should have more nutrition than starved");
    
    // Normal should be between starved and well-fed
    TEST_ASSERT_MSG(normalNutrition > starvedNutrition,
                    "Normal corpse should have more nutrition than starved");
    TEST_ASSERT_MSG(wellFedNutrition >= normalNutrition,
                    "Well-fed corpse should have at least as much nutrition as normal");
    
    // Verify the formula: nutrition = size * NUTRITION_PER_SIZE * (0.5 + bodyCondition * 0.5)
    // Well-fed: multiplier = 0.5 + 1.0 * 0.5 = 1.0
    // Starved: multiplier = 0.5 + 0.0 * 0.5 = 0.5
    // Ratio should be 2:1
    float ratio = wellFedNutrition / starvedNutrition;
    TEST_ASSERT_MSG(ratio > 1.9f && ratio < 2.1f,
                    "Well-fed to starved nutrition ratio should be approximately 2:1");
}

//==============================================================================
// Combat Weapon Selection Tests
//==============================================================================

void test_weapon_selection() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create attacker with high tooth sharpness
    Genome attackerGenome = UniversalGenes::createCreatureGenome(registry);
    if (attackerGenome.hasGene(UniversalGenes::TOOTH_SHARPNESS)) {
        attackerGenome.setGeneValue(UniversalGenes::TOOTH_SHARPNESS, 0.9f);
    }
    Phenotype attackerPhenotype(&attackerGenome, &registry);
    
    // Create defender
    Genome defenderGenome = UniversalGenes::createCreatureGenome(registry);
    Phenotype defenderPhenotype(&defenderGenome, &registry);
    
    // Select best action - should choose a weapon
    CombatAction action = CombatInteraction::selectBestAction(attackerPhenotype, defenderPhenotype);
    
    // The weapon selection should choose some weapon for attack
    // Teeth, Claws, Horns, Tail, or Body are valid weapons
    bool validWeapon = (action.weapon == WeaponType::Teeth ||
                        action.weapon == WeaponType::Claws ||
                        action.weapon == WeaponType::Horns ||
                        action.weapon == WeaponType::Tail ||
                        action.weapon == WeaponType::Body);
    TEST_ASSERT_MSG(validWeapon, "Should select a valid weapon for attack");
}

//==============================================================================
// Combat State Tests
//==============================================================================

void test_combat_state() {
    CombatState state;
    
    // Initial state
    TEST_ASSERT_MSG(!state.inCombat, "Should not be in combat initially");
    TEST_ASSERT_EQ(state.ticksSinceCombatStart, 0);
    
    // All weapons should be ready initially
    TEST_ASSERT_MSG(state.isWeaponReady(WeaponType::Teeth), "Teeth should be ready initially");
    TEST_ASSERT_MSG(state.isWeaponReady(WeaponType::Claws), "Claws should be ready initially");
    
    // Start cooldown on teeth (3 ticks)
    state.startCooldown(WeaponType::Teeth);
    TEST_ASSERT_MSG(!state.isWeaponReady(WeaponType::Teeth), "Teeth should be on cooldown");
    
    // Other weapons still ready
    TEST_ASSERT_MSG(state.isWeaponReady(WeaponType::Claws), "Claws should still be ready");
    
    // Tick down cooldowns
    state.tickAllCooldowns();
    state.tickAllCooldowns();
    TEST_ASSERT_MSG(!state.isWeaponReady(WeaponType::Teeth), "Teeth should still have 1 tick left");
    
    state.tickAllCooldowns();
    TEST_ASSERT_MSG(state.isWeaponReady(WeaponType::Teeth), "Teeth should be ready after 3 ticks");
    
    // Reset should clear everything
    state.reset();
    TEST_ASSERT_MSG(!state.inCombat, "Should not be in combat after reset");
}

//==============================================================================
// Defense Profile Tests
//==============================================================================

void test_defense_profile() {
    DefenseProfile profile;
    profile.thickHide = 0.8f;
    profile.scales = 0.3f;
    profile.fatLayer = 0.5f;
    
    // Test get defense for type
    TEST_ASSERT_NEAR(profile.getDefenseForType(DefenseType::ThickHide), 0.8f, 0.001f);
    TEST_ASSERT_NEAR(profile.getDefenseForType(DefenseType::Scales), 0.3f, 0.001f);
    TEST_ASSERT_NEAR(profile.getDefenseForType(DefenseType::FatLayer), 0.5f, 0.001f);
    
    // Test strongest defense
    TEST_ASSERT(profile.getStrongestDefense() == DefenseType::ThickHide);
    
    // Test total investment
    TEST_ASSERT_NEAR(profile.totalInvestment(), 1.6f, 0.001f);
}

//==============================================================================
// Type Effectiveness Tests
//==============================================================================

void test_type_effectiveness() {
    // Piercing vs ThickHide = 0.5 (resisted)
    TEST_ASSERT_NEAR(
        getTypeEffectiveness(DamageType::Piercing, DefenseType::ThickHide),
        0.5f, 0.001f
    );
    
    // Piercing vs Scales = 1.5 (strong)
    TEST_ASSERT_NEAR(
        getTypeEffectiveness(DamageType::Piercing, DefenseType::Scales),
        1.5f, 0.001f
    );
    
    // Slashing vs FatLayer = 1.5 (strong)
    TEST_ASSERT_NEAR(
        getTypeEffectiveness(DamageType::Slashing, DefenseType::FatLayer),
        1.5f, 0.001f
    );
    
    // Blunt vs ThickHide = 1.5 (strong)
    TEST_ASSERT_NEAR(
        getTypeEffectiveness(DamageType::Blunt, DefenseType::ThickHide),
        1.5f, 0.001f
    );
}

//==============================================================================
// Main test runner
//==============================================================================

void run_combat_behavior_tests() {
    BEGIN_TEST_GROUP("Combat Behavior Tests");
    
    RUN_TEST(test_combat_initiation);
    RUN_TEST(test_attack_damage);
    RUN_TEST(test_retreat_threshold);
    RUN_TEST(test_corpse_creation);
    RUN_TEST(test_scavenging);
    RUN_TEST(test_body_condition_nutrition);
    RUN_TEST(test_weapon_selection);
    RUN_TEST(test_combat_state);
    RUN_TEST(test_defense_profile);
    RUN_TEST(test_type_effectiveness);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running Combat Behavior Tests..." << std::endl;
    run_combat_behavior_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
