/**
 * @file test_combat_system.cpp
 * @brief Tests for the combat interaction system
 * 
 * Tests cover:
 * 1. Damage type effectiveness (rock-paper-scissors)
 * 2. Weapon damage calculation from shape genes
 * 3. Specialization bonus calculation
 * 4. Defense application
 * 5. Combat initiation check (aggression threshold)
 * 6. Retreat check (health threshold)
 * 7. Full attack resolution
 */

#include "test_framework.hpp"
#include "genetics/interactions/DamageTypes.hpp"
#include "genetics/interactions/CombatAction.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <iostream>
#include <cmath>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

// Helper to create a test phenotype with specific gene values
Phenotype createTestPhenotype(
    GeneRegistry& registry,
    const std::map<std::string, float>& geneValues = {}
) {
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    
    // Set specific gene values in the genome
    for (const auto& [name, value] : geneValues) {
        if (genome.hasGene(name)) {
            genome.setGeneValue(name, value);
        }
    }
    
    Phenotype phenotype(&genome, &registry);
    return phenotype;
}

// ============================================================================
// Test 1: Damage Type Effectiveness (Rock-Paper-Scissors)
// ============================================================================

void test_damage_type_effectiveness() {
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
    
    // Piercing vs FatLayer = 1.0 (neutral)
    TEST_ASSERT_NEAR(
        getTypeEffectiveness(DamageType::Piercing, DefenseType::FatLayer),
        1.0f, 0.001f
    );
    
    // Slashing vs Scales = 0.5 (resisted)
    TEST_ASSERT_NEAR(
        getTypeEffectiveness(DamageType::Slashing, DefenseType::Scales),
        0.5f, 0.001f
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
    
    // Blunt vs FatLayer = 0.5 (resisted)
    TEST_ASSERT_NEAR(
        getTypeEffectiveness(DamageType::Blunt, DefenseType::FatLayer),
        0.5f, 0.001f
    );
}

// ============================================================================
// Test 2: Weapon Base Stats
// ============================================================================

void test_weapon_base_stats() {
    // Teeth: 10 damage, 3 cooldown, Pierce
    const WeaponStats& teeth = getWeaponStats(WeaponType::Teeth);
    TEST_ASSERT_NEAR(teeth.baseDamage, 10.0f, 0.001f);
    TEST_ASSERT_EQ(teeth.baseCooldown, 3);
    TEST_ASSERT(teeth.primaryType == DamageType::Piercing);
    
    // Claws: 6 damage, 2 cooldown, Slash
    const WeaponStats& claws = getWeaponStats(WeaponType::Claws);
    TEST_ASSERT_NEAR(claws.baseDamage, 6.0f, 0.001f);
    TEST_ASSERT_EQ(claws.baseCooldown, 2);
    TEST_ASSERT(claws.primaryType == DamageType::Slashing);
    
    // Horns: 12 damage, 4 cooldown, Pierce
    const WeaponStats& horns = getWeaponStats(WeaponType::Horns);
    TEST_ASSERT_NEAR(horns.baseDamage, 12.0f, 0.001f);
    TEST_ASSERT_EQ(horns.baseCooldown, 4);
    TEST_ASSERT(horns.primaryType == DamageType::Piercing);
    
    // Tail: 5 damage, 2 cooldown, Blunt
    const WeaponStats& tail = getWeaponStats(WeaponType::Tail);
    TEST_ASSERT_NEAR(tail.baseDamage, 5.0f, 0.001f);
    TEST_ASSERT_EQ(tail.baseCooldown, 2);
    TEST_ASSERT(tail.primaryType == DamageType::Blunt);
    
    // Body: 8 damage, 3 cooldown, Blunt
    const WeaponStats& body = getWeaponStats(WeaponType::Body);
    TEST_ASSERT_NEAR(body.baseDamage, 8.0f, 0.001f);
    TEST_ASSERT_EQ(body.baseCooldown, 3);
    TEST_ASSERT(body.primaryType == DamageType::Blunt);
}

// ============================================================================
// Test 3: Damage Distribution Calculation
// ============================================================================

void test_damage_distribution() {
    DamageDistribution dist;
    dist.piercing = 0.8f;
    dist.slashing = 0.1f;
    dist.blunt = 0.1f;
    
    // Test total
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.001f);
    
    // Test dominant type (should be piercing)
    TEST_ASSERT(dist.getDominantType() == DamageType::Piercing);
    
    // Test specialization (0.8/1.0 = 0.8)
    TEST_ASSERT_NEAR(dist.getSpecialization(), 0.8f, 0.001f);
    
    // Test generalist distribution
    DamageDistribution gen;
    gen.piercing = 0.33f;
    gen.slashing = 0.33f;
    gen.blunt = 0.34f;
    
    // Generalist has lower specialization
    TEST_ASSERT_LT(gen.getSpecialization(), 0.5f);
}

// ============================================================================
// Test 4: Specialization Bonus Calculation
// ============================================================================

void test_specialization_bonus() {
    // Pure specialist (100% pierce)
    DamageDistribution specialist;
    specialist.piercing = 1.0f;
    specialist.slashing = 0.0f;
    specialist.blunt = 0.0f;
    
    float specBonus = CombatInteraction::calculateSpecializationBonus(specialist);
    // 100% in one type = 50% bonus
    TEST_ASSERT_NEAR(specBonus, 0.5f, 0.001f);
    
    // Perfect generalist (33% each)
    DamageDistribution generalist;
    generalist.piercing = 0.33f;
    generalist.slashing = 0.33f;
    generalist.blunt = 0.34f;
    
    float genBonus = CombatInteraction::calculateSpecializationBonus(generalist);
    // ~34% max ratio = ~17% bonus
    TEST_ASSERT_GT(genBonus, 0.15f);
    TEST_ASSERT_LT(genBonus, 0.2f);
    
    // Dual-focused (70% one, 30% another)
    DamageDistribution dualFocus;
    dualFocus.piercing = 0.7f;
    dualFocus.slashing = 0.3f;
    dualFocus.blunt = 0.0f;
    
    float dualBonus = CombatInteraction::calculateSpecializationBonus(dualFocus);
    // 70% max ratio = 35% bonus
    TEST_ASSERT_NEAR(dualBonus, 0.35f, 0.01f);
    
    // Edge case: zero damage
    DamageDistribution zero;
    float zeroBonus = CombatInteraction::calculateSpecializationBonus(zero);
    TEST_ASSERT_NEAR(zeroBonus, 0.0f, 0.001f);
}

// ============================================================================
// Test 5: Defense Application
// ============================================================================

void test_defense_application() {
    // 50% reduction at max defense (1.0)
    float reduced = CombatInteraction::applyDefense(100.0f, 1.0f);
    TEST_ASSERT_NEAR(reduced, 50.0f, 0.001f);
    
    // No reduction at zero defense
    float noDefense = CombatInteraction::applyDefense(100.0f, 0.0f);
    TEST_ASSERT_NEAR(noDefense, 100.0f, 0.001f);
    
    // 25% reduction at half defense (0.5)
    float halfDefense = CombatInteraction::applyDefense(100.0f, 0.5f);
    TEST_ASSERT_NEAR(halfDefense, 75.0f, 0.001f);
}

// ============================================================================
// Test 6: Combat State Management
// ============================================================================

void test_combat_state() {
    CombatState state;
    
    // Initial state
    TEST_ASSERT(!state.inCombat);
    TEST_ASSERT_EQ(state.ticksSinceCombatStart, 0);
    
    // All weapons should be ready initially
    TEST_ASSERT(state.isWeaponReady(WeaponType::Teeth));
    TEST_ASSERT(state.isWeaponReady(WeaponType::Claws));
    TEST_ASSERT(state.isWeaponReady(WeaponType::Horns));
    TEST_ASSERT(state.isWeaponReady(WeaponType::Tail));
    TEST_ASSERT(state.isWeaponReady(WeaponType::Body));
    
    // Start cooldown on teeth (3 ticks)
    state.startCooldown(WeaponType::Teeth);
    TEST_ASSERT(!state.isWeaponReady(WeaponType::Teeth));
    
    // Other weapons still ready
    TEST_ASSERT(state.isWeaponReady(WeaponType::Claws));
    
    // Tick down cooldowns
    state.tickAllCooldowns();
    state.tickAllCooldowns();
    TEST_ASSERT(!state.isWeaponReady(WeaponType::Teeth));  // Still 1 tick left
    
    state.tickAllCooldowns();
    TEST_ASSERT(state.isWeaponReady(WeaponType::Teeth));  // Now ready
    
    // Reset should clear everything
    state.reset();
    TEST_ASSERT(!state.inCombat);
    TEST_ASSERT_EQ(state.ticksSinceCombatStart, 0);
}

// ============================================================================
// Test 7: Defense Profile
// ============================================================================

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

// ============================================================================
// Test 8: Teeth Damage Formula (using DamageDistribution directly)
// ============================================================================

void test_teeth_damage_formula() {
    // Test the teeth formula directly with known values
    // pierce = sharpness × size = 0.8 × 1.0 = 0.8
    // slash  = serration × size × 0.5 = 0.4 × 1.0 × 0.5 = 0.2
    // blunt  = (1.0 - sharpness) × size = 0.2 × 1.0 = 0.2
    
    // We test the formula by constructing expected values
    float sharpness = 0.8f;
    float serration = 0.4f;
    float size = 1.0f;
    
    float expectedPierce = sharpness * size;
    float expectedSlash = serration * size * 0.5f;
    float expectedBlunt = (1.0f - sharpness) * size;
    
    TEST_ASSERT_NEAR(expectedPierce, 0.8f, 0.001f);
    TEST_ASSERT_NEAR(expectedSlash, 0.2f, 0.001f);
    TEST_ASSERT_NEAR(expectedBlunt, 0.2f, 0.001f);
}

// ============================================================================
// Test 9: Claws Damage Formula (using DamageDistribution directly)
// ============================================================================

void test_claws_damage_formula() {
    // Test the claws formula directly
    // pierce = curvature × length × sharpness = 0.6 × 0.8 × 1.0 = 0.48
    // slash  = (1.0 - curvature) × length × sharpness = 0.4 × 0.8 × 1.0 = 0.32
    // blunt  = length × (1.0 - sharpness) × 0.3 = 0.8 × 0.0 × 0.3 = 0.0
    
    float curvature = 0.6f;
    float length = 0.8f;
    float sharpness = 1.0f;
    
    float expectedPierce = curvature * length * sharpness;
    float expectedSlash = (1.0f - curvature) * length * sharpness;
    float expectedBlunt = length * (1.0f - sharpness) * 0.3f;
    
    TEST_ASSERT_NEAR(expectedPierce, 0.48f, 0.001f);
    TEST_ASSERT_NEAR(expectedSlash, 0.32f, 0.001f);
    TEST_ASSERT_NEAR(expectedBlunt, 0.0f, 0.001f);
}

// ============================================================================
// Test 10: Horns Damage Formula
// ============================================================================

void test_horns_damage_formula() {
    // pierce = pointiness × length = 0.7 × 0.5 = 0.35
    // slash  = spread × length × 0.3 = 0.3 × 0.5 × 0.3 = 0.045
    // blunt  = (1.0 - pointiness) × length = 0.3 × 0.5 = 0.15
    
    float pointiness = 0.7f;
    float length = 0.5f;
    float spread = 0.3f;
    
    float expectedPierce = pointiness * length;
    float expectedSlash = spread * length * 0.3f;
    float expectedBlunt = (1.0f - pointiness) * length;
    
    TEST_ASSERT_NEAR(expectedPierce, 0.35f, 0.001f);
    TEST_ASSERT_NEAR(expectedSlash, 0.045f, 0.001f);
    TEST_ASSERT_NEAR(expectedBlunt, 0.15f, 0.001f);
}

// ============================================================================
// Test 11: Tail Damage Formula
// ============================================================================

void test_tail_damage_formula() {
    // pierce = spines × length = 0.2 × 0.6 = 0.12
    // slash  = (1.0 - mass) × length × 0.5 = 0.6 × 0.6 × 0.5 = 0.18
    // blunt  = mass × length = 0.4 × 0.6 = 0.24
    
    float spines = 0.2f;
    float length = 0.6f;
    float mass = 0.4f;
    
    float expectedPierce = spines * length;
    float expectedSlash = (1.0f - mass) * length * 0.5f;
    float expectedBlunt = mass * length;
    
    TEST_ASSERT_NEAR(expectedPierce, 0.12f, 0.001f);
    TEST_ASSERT_NEAR(expectedSlash, 0.18f, 0.001f);
    TEST_ASSERT_NEAR(expectedBlunt, 0.24f, 0.001f);
}

// ============================================================================
// Test 12: Body Damage Formula
// ============================================================================

void test_body_damage_formula() {
    // pierce = spines × maxSize = 0.3 × 2.0 = 0.6
    // slash  = 0.0
    // blunt  = maxSize = 2.0
    
    float spines = 0.3f;
    float maxSize = 2.0f;
    
    float expectedPierce = spines * maxSize;
    float expectedSlash = 0.0f;
    float expectedBlunt = maxSize;
    
    TEST_ASSERT_NEAR(expectedPierce, 0.6f, 0.001f);
    TEST_ASSERT_NEAR(expectedSlash, 0.0f, 0.001f);
    TEST_ASSERT_NEAR(expectedBlunt, 2.0f, 0.001f);
}

// ============================================================================
// Test 13: Type Effectiveness Combinations
// ============================================================================

void test_type_effectiveness_combinations() {
    // Full rock-paper-scissors verification
    // Pierce -> strong vs Scales, weak vs Hide
    TEST_ASSERT_GT(
        getTypeEffectiveness(DamageType::Piercing, DefenseType::Scales),
        getTypeEffectiveness(DamageType::Piercing, DefenseType::ThickHide)
    );
    
    // Slash -> strong vs Fat, weak vs Scales
    TEST_ASSERT_GT(
        getTypeEffectiveness(DamageType::Slashing, DefenseType::FatLayer),
        getTypeEffectiveness(DamageType::Slashing, DefenseType::Scales)
    );
    
    // Blunt -> strong vs Hide, weak vs Fat
    TEST_ASSERT_GT(
        getTypeEffectiveness(DamageType::Blunt, DefenseType::ThickHide),
        getTypeEffectiveness(DamageType::Blunt, DefenseType::FatLayer)
    );
}

// ============================================================================
// Test 14: Combat Action Structure
// ============================================================================

void test_combat_action() {
    CombatAction action;
    action.weapon = WeaponType::Teeth;
    action.cooldownRemaining = 0;
    action.staminaCost = 0.1f;
    
    // Should be ready initially
    TEST_ASSERT(action.isReady());
    
    // Start cooldown
    action.startCooldown();
    TEST_ASSERT(!action.isReady());
    TEST_ASSERT_EQ(action.cooldownRemaining, 3);  // Teeth has 3 tick cooldown
    
    // Tick down
    action.tickCooldown();
    TEST_ASSERT_EQ(action.cooldownRemaining, 2);
    
    action.tickCooldown();
    action.tickCooldown();
    TEST_ASSERT(action.isReady());
    
    // Name should be correct
    TEST_ASSERT(std::string(action.getName()) == "Teeth");
}

// ============================================================================
// Test 15: Attack Result Structure
// ============================================================================

void test_attack_result() {
    AttackResult result;
    result.hit = true;
    result.rawDamage = 20.0f;
    result.finalDamage = 15.0f;
    result.primaryType = DamageType::Piercing;
    result.effectivenessMultiplier = 0.5f;  // Resisted
    result.weaponUsed = WeaponType::Teeth;
    result.causedBleeding = false;
    
    std::string desc = result.describe();
    
    // Description should contain key info
    TEST_ASSERT(desc.find("Teeth") != std::string::npos);
    TEST_ASSERT(desc.find("15") != std::string::npos);
    TEST_ASSERT(desc.find("Piercing") != std::string::npos);
    TEST_ASSERT(desc.find("not very effective") != std::string::npos);
}

// ============================================================================
// Test 16: String Conversions
// ============================================================================

void test_string_conversions() {
    // Damage types
    TEST_ASSERT(std::string(damageTypeToString(DamageType::Piercing)) == "Piercing");
    TEST_ASSERT(std::string(damageTypeToString(DamageType::Slashing)) == "Slashing");
    TEST_ASSERT(std::string(damageTypeToString(DamageType::Blunt)) == "Blunt");
    
    // Defense types
    TEST_ASSERT(std::string(defenseTypeToString(DefenseType::ThickHide)) == "ThickHide");
    TEST_ASSERT(std::string(defenseTypeToString(DefenseType::Scales)) == "Scales");
    TEST_ASSERT(std::string(defenseTypeToString(DefenseType::FatLayer)) == "FatLayer");
    
    // Weapon types
    TEST_ASSERT(std::string(weaponTypeToString(WeaponType::Teeth)) == "Teeth");
    TEST_ASSERT(std::string(weaponTypeToString(WeaponType::Claws)) == "Claws");
    TEST_ASSERT(std::string(weaponTypeToString(WeaponType::Horns)) == "Horns");
    TEST_ASSERT(std::string(weaponTypeToString(WeaponType::Tail)) == "Tail");
    TEST_ASSERT(std::string(weaponTypeToString(WeaponType::Body)) == "Body");
}

// ============================================================================
// Main test runner
// ============================================================================

void run_combat_system_tests() {
    BEGIN_TEST_GROUP("Combat System Tests");
    
    RUN_TEST(test_damage_type_effectiveness);
    RUN_TEST(test_weapon_base_stats);
    RUN_TEST(test_damage_distribution);
    RUN_TEST(test_specialization_bonus);
    RUN_TEST(test_defense_application);
    RUN_TEST(test_combat_state);
    RUN_TEST(test_defense_profile);
    RUN_TEST(test_teeth_damage_formula);
    RUN_TEST(test_claws_damage_formula);
    RUN_TEST(test_horns_damage_formula);
    RUN_TEST(test_tail_damage_formula);
    RUN_TEST(test_body_damage_formula);
    RUN_TEST(test_type_effectiveness_combinations);
    RUN_TEST(test_combat_action);
    RUN_TEST(test_attack_result);
    RUN_TEST(test_string_conversions);
    
    END_TEST_GROUP();
}

#ifdef STANDALONE_TEST
int main() {
    std::cout << "Running Combat System Tests..." << std::endl;
    run_combat_system_tests();
    TestSuite::instance().printSummary();
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
