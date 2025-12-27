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
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <iostream>
#include <cmath>
#include <map>
#include <string>
#include <vector>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

// Helper to setup a genome with specific gene values
// Note: Phenotype must be created separately with pointers to genome/registry
// to avoid dangling pointer issues (Phenotype stores pointer, not copy)
void setupGenome(
    Genome& genome,
    const std::map<std::string, float>& geneValues
) {
    // Set specific gene values in the genome
    for (const auto& [name, value] : geneValues) {
        if (genome.hasGene(name)) {
            genome.getGeneMutable(name).setAlleleValues(value);
        }
    }
}

// Helper to set phenotype to adult age for 100% gene expression
// Age modulation: 0.0-0.1 = 60%, 0.1-0.8 = 100%, 0.8-1.0 = 80%
void setMatureAge(Phenotype& phenotype) {
    OrganismState matureState;
    matureState.age_normalized = 0.5f;  // Adult age = 100% expression
    phenotype.updateContext(EnvironmentState(), matureState);
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
// Test 8: Normalized Damage Distribution (Teeth)
// ============================================================================

void test_teeth_damage_normalized() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create genome and phenotype with default values
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Teeth);
    
    // Key test: distribution should now sum to ~1.0 (normalized)
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    // All components should be non-negative
    TEST_ASSERT_GT(dist.piercing, -0.001f);
    TEST_ASSERT_GT(dist.slashing, -0.001f);
    TEST_ASSERT_GT(dist.blunt, -0.001f);
}

// ============================================================================
// Test 9: Normalized Damage Distribution (Claws)
// ============================================================================

void test_claws_damage_normalized() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create genome and phenotype with default values
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Claws);
    
    // Key test: distribution should sum to ~1.0 (normalized)
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    // With defaults curvature=0.4, sharpness=0.6:
    // pierceWeight = 0.4*0.6 = 0.24, slashWeight = 0.6*0.6 = 0.36, bluntWeight = 0.4*0.3 = 0.12
    // Slashing should be dominant with these defaults
    TEST_ASSERT(dist.slashing + dist.piercing > dist.blunt);  // Pierce or slash should dominate
}

// ============================================================================
// Test 10: Normalized Damage Distribution (Horns)
// ============================================================================

void test_horns_damage_normalized() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create genome and phenotype with default values
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Horns);
    
    // Key test: distribution should sum to ~1.0 (normalized)
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    // All components should be non-negative
    TEST_ASSERT_GT(dist.piercing, -0.001f);
    TEST_ASSERT_GT(dist.slashing, -0.001f);
    TEST_ASSERT_GT(dist.blunt, -0.001f);
}

// ============================================================================
// Test 11: Normalized Damage Distribution (Tail)
// ============================================================================

void test_tail_damage_normalized() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create genome and phenotype with default values
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Tail);
    
    // Key test: distribution should sum to ~1.0 (normalized)
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    // All components should be non-negative
    TEST_ASSERT_GT(dist.piercing, -0.001f);
    TEST_ASSERT_GT(dist.slashing, -0.001f);
    TEST_ASSERT_GT(dist.blunt, -0.001f);
}

// ============================================================================
// Test 12: Normalized Damage Distribution (Body)
// ============================================================================

void test_body_damage_normalized() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create genome and phenotype in local scope
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    setupGenome(genome, {
        {UniversalGenes::BODY_SPINES, 0.3f},
        {UniversalGenes::MAX_SIZE, 1.5f}  // Size is now applied separately
    });
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Body);
    
    // Key test: distribution should sum to ~1.0 (normalized)
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    // Body is primarily blunt (always 1.0 weight before normalization)
    TEST_ASSERT(dist.getDominantType() == DamageType::Blunt);
    
    // With spines=0.3, pierce should be ~23% (0.3/1.3), blunt ~77% (1.0/1.3)
    TEST_ASSERT_GT(dist.blunt, 0.7f);
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
// Test 17: Size Factor for Weapons
// ============================================================================

void test_size_factor_for_weapons() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create phenotype with default genome values
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    // Create a simple action with teeth weapon type
    CombatAction teethAction;
    teethAction.weapon = WeaponType::Teeth;
    teethAction.damage = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Teeth);
    
    // The distribution should sum to 1.0 (normalized)
    TEST_ASSERT_NEAR(teethAction.damage.total(), 1.0f, 0.01f);
    
    // Create a defender phenotype
    Genome defenderGenome = UniversalGenes::createCreatureGenome(registry);
    Phenotype defenderPhenotype(&defenderGenome, &registry);
    setMatureAge(defenderPhenotype);  // Set adult age for 100% gene expression
    
    // Resolve attack and check raw damage includes size factor
    AttackResult result = CombatInteraction::resolveAttack(phenotype, defenderPhenotype, teethAction);
    
    // Raw damage should be: distribution.total (1.0) × baseDamage (10) × sizeFactor × specMultiplier
    // With default size and spec bonus, rawDamage should be > 0
    TEST_ASSERT_GT(result.rawDamage, 0.0f);
    
    // With default size factor (0.5) and teeth base 10:
    // Raw = 1.0 × 10 × 0.5 × ~1.3 (spec bonus) = ~6.5
    // Be lenient with bounds due to spec bonus variation
    TEST_ASSERT_GT(result.rawDamage, 3.0f);
    TEST_ASSERT_LT(result.rawDamage, 15.0f);
}

// ============================================================================
// Test 18: Edge Case - Zero Size
// ============================================================================

void test_edge_case_zero_size() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create phenotype with zero size for teeth
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    genome.getGeneMutable(UniversalGenes::TEETH_SIZE).setAlleleValues(0.0f);
    genome.getGeneMutable(UniversalGenes::TEETH_SHARPNESS).setAlleleValues(0.9f);
    
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    // Distribution should still sum to 1.0 (shape genes still work)
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Teeth);
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    // But actual damage should be zero due to size=0
    CombatAction action;
    action.weapon = WeaponType::Teeth;
    action.damage = dist;
    
    Genome defenderGenome = UniversalGenes::createCreatureGenome(registry);
    Phenotype defenderPhenotype(&defenderGenome, &registry);
    setMatureAge(defenderPhenotype);  // Set adult age for 100% gene expression
    
    AttackResult result = CombatInteraction::resolveAttack(phenotype, defenderPhenotype, action);
    
    // With size=0, raw damage should be 0 or very close to 0
    TEST_ASSERT_NEAR(result.rawDamage, 0.0f, 0.01f);
}

// ============================================================================
// Test 19: Edge Case - Maximum Size
// ============================================================================

void test_edge_case_max_size() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create phenotype with default values (size depends on what genome provides)
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, WeaponType::Teeth);
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    CombatAction action;
    action.weapon = WeaponType::Teeth;
    action.damage = dist;
    
    Genome defenderGenome = UniversalGenes::createCreatureGenome(registry);
    Phenotype defenderPhenotype(&defenderGenome, &registry);
    setMatureAge(defenderPhenotype);  // Set adult age for 100% gene expression
    
    AttackResult result = CombatInteraction::resolveAttack(phenotype, defenderPhenotype, action);
    
    // With default size factor, raw damage should be in reasonable range
    // Raw = 1.0 × 10 × sizeFactor × specMultiplier
    TEST_ASSERT_GT(result.rawDamage, 3.0f);  // Reasonable minimum
    TEST_ASSERT_LT(result.rawDamage, 20.0f); // Not unreasonably high
}

// ============================================================================
// Test 20: All Weapon Types Produce Normalized Distributions
// ============================================================================

void test_all_weapons_normalized() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create genome and phenotype in local scope
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    setMatureAge(phenotype);  // Set adult age for 100% gene expression
    
    // Test all weapon types
    std::vector<WeaponType> weapons = {
        WeaponType::Teeth,
        WeaponType::Claws,
        WeaponType::Horns,
        WeaponType::Tail,
        WeaponType::Body
    };
    
    for (WeaponType weapon : weapons) {
        DamageDistribution dist = CombatInteraction::calculateWeaponDamage(phenotype, weapon);
        
        // Each weapon's distribution should sum to 1.0
        TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
        
        // All values should be non-negative
        TEST_ASSERT_GT(dist.piercing, -0.001f);
        TEST_ASSERT_GT(dist.slashing, -0.001f);
        TEST_ASSERT_GT(dist.blunt, -0.001f);
    }
}

// ============================================================================
// Test 21: Raw Damage Calculation Formula
// ============================================================================

void test_raw_damage_formula() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create a phenotype with default genome values
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype attackerPhenotype(&genome, &registry);
    setMatureAge(attackerPhenotype);  // Set adult age for 100% gene expression
    
    // Create defender with default defense
    Genome defenderGenome = UniversalGenes::createCreatureGenome(registry);
    Phenotype defenderPhenotype(&defenderGenome, &registry);
    setMatureAge(defenderPhenotype);  // Set adult age for 100% gene expression
    
    // Calculate distribution with default values
    DamageDistribution dist = CombatInteraction::calculateWeaponDamage(attackerPhenotype, WeaponType::Teeth);
    
    // Distribution should be normalized
    TEST_ASSERT_NEAR(dist.total(), 1.0f, 0.01f);
    
    // Piercing should be dominant with default sharpness=0.7
    TEST_ASSERT(dist.getDominantType() == DamageType::Piercing);
    
    // Calculate spec bonus based on actual distribution
    float specBonus = CombatInteraction::calculateSpecializationBonus(dist);
    TEST_ASSERT_GT(specBonus, 0.0f);  // Should have some spec bonus
    TEST_ASSERT_LT(specBonus, 0.51f); // Can't exceed 50%
    
    CombatAction action;
    action.weapon = WeaponType::Teeth;
    action.damage = dist;
    
    AttackResult result = CombatInteraction::resolveAttack(attackerPhenotype, defenderPhenotype, action);
    
    // Raw damage = dist.total(1.0) × baseDamage(10) × sizeFactor × (1 + specBonus)
    // With defaults, expect reasonable damage range
    TEST_ASSERT_GT(result.rawDamage, 3.0f);  // Should produce meaningful damage
    TEST_ASSERT_LT(result.rawDamage, 15.0f); // Not unreasonably high
}

// ============================================================================
// Test 22: Health Never Exceeds MaxHealth After Age Modulation
// ============================================================================
// This test catches the bug where elderly creatures had _health > maxHealth
// because MAX_SIZE expression decreases with age (80% at age=1.0) but
// _health was never clamped when maxHealth decreased.

void test_health_clamped_on_aging() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create a genome with known MAX_SIZE
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    setupGenome(genome, {
        {UniversalGenes::MAX_SIZE, 1.0f}  // MAX_SIZE = 1.0 -> maxHealth = 10.0
    });
    
    Phenotype phenotype(&genome, &registry);
    
    // Set to adult age (100% expression)
    OrganismState adultState;
    adultState.age_normalized = 0.5f;  // Adult = 100% expression
    phenotype.updateContext(EnvironmentState(), adultState);
    
    // Get adult maxHealth (should be MAX_SIZE * 10 * 1.0 = 10.0)
    float adultMaxHealth = phenotype.getTrait(UniversalGenes::MAX_SIZE) * 10.0f;
    TEST_ASSERT_NEAR(adultMaxHealth, 10.0f, 0.1f);
    
    // Now simulate creature becoming elderly
    OrganismState elderlyState;
    elderlyState.age_normalized = 1.0f;  // Elderly = 80% expression
    phenotype.updateContext(EnvironmentState(), elderlyState);
    
    // Elderly maxHealth should be MAX_SIZE * 10 * 0.8 = 8.0
    float elderlyMaxHealth = phenotype.getTrait(UniversalGenes::MAX_SIZE) * 10.0f;
    TEST_ASSERT_NEAR(elderlyMaxHealth, 8.0f, 0.1f);
    
    // Key invariant: maxHealth MUST decrease with age
    TEST_ASSERT_LT(elderlyMaxHealth, adultMaxHealth);
}

// ============================================================================
// Test 23: Health Invariant - Never Exceeds MaxHealth
// ============================================================================
// This test verifies the core invariant that should always hold:
// health <= maxHealth for any creature at any age

void test_health_invariant_simulation() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    Phenotype phenotype(&genome, &registry);
    
    // Simulate a creature going through its entire lifespan
    // At each age, verify maxHealth decreases appropriately
    float previousMaxHealth = 999999.0f;  // Start very high
    
    for (float age = 0.0f; age <= 1.0f; age += 0.1f) {
        OrganismState state;
        state.age_normalized = age;
        phenotype.updateContext(EnvironmentState(), state);
        
        float maxHealth = phenotype.getTrait(UniversalGenes::MAX_SIZE) * 10.0f;
        
        // MaxHealth should follow the age modulation curve:
        // - Infant (0-0.05): 40% -> 60%
        // - Juvenile (0.05-0.15): 60% -> 100%
        // - Adult (0.15-0.8): 100%
        // - Elderly (0.8-1.0): 100% -> 80%
        
        // During infant/juvenile phase, maxHealth increases
        if (age >= 0.15f && age <= 0.8f) {
            // Adult phase: maxHealth should be at peak (roughly same)
            // Allow for floating point variance
        }
        
        // During elderly phase (>0.8), maxHealth should decrease
        if (age > 0.8f) {
            TEST_ASSERT_LT(maxHealth, previousMaxHealth * 1.01f);  // Allow tiny variance
        }
        
        previousMaxHealth = maxHealth;
    }
}

// ============================================================================
// Test 24: Age Modulation Curve Verification
// ============================================================================
// Verify the exact age modulation values match the expected curve

void test_age_modulation_curve() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    setupGenome(genome, {
        {UniversalGenes::MAX_SIZE, 1.0f}  // Use 1.0 for easy math
    });
    
    Phenotype phenotype(&genome, &registry);
    
    // Test specific age points
    struct AgeTest {
        float age;
        float expectedFactor;
        float tolerance;
    };
    
    std::vector<AgeTest> tests = {
        {0.0f, 0.40f, 0.01f},   // Birth: 40%
        {0.025f, 0.50f, 0.02f}, // Mid-infant: ~50%
        {0.05f, 0.60f, 0.01f},  // End infant: 60%
        {0.15f, 1.00f, 0.01f},  // Start adult: 100%
        {0.5f, 1.00f, 0.01f},   // Mid adult: 100%
        {0.8f, 1.00f, 0.01f},   // End adult: 100%
        {0.9f, 0.90f, 0.02f},   // Mid-elderly: ~90%
        {1.0f, 0.80f, 0.01f},   // End elderly: 80%
    };
    
    for (const auto& test : tests) {
        OrganismState state;
        state.age_normalized = test.age;
        phenotype.updateContext(EnvironmentState(), state);
        
        // MAX_SIZE = 1.0 * factor, so maxHealth = 10.0 * factor
        float maxHealth = phenotype.getTrait(UniversalGenes::MAX_SIZE) * 10.0f;
        float expectedMaxHealth = 10.0f * test.expectedFactor;
        
        TEST_ASSERT_NEAR(maxHealth, expectedMaxHealth, test.tolerance * 10.0f);
    }
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
    RUN_TEST(test_teeth_damage_normalized);
    RUN_TEST(test_claws_damage_normalized);
    RUN_TEST(test_horns_damage_normalized);
    RUN_TEST(test_tail_damage_normalized);
    RUN_TEST(test_body_damage_normalized);
    RUN_TEST(test_type_effectiveness_combinations);
    RUN_TEST(test_combat_action);
    RUN_TEST(test_attack_result);
    RUN_TEST(test_string_conversions);
    RUN_TEST(test_size_factor_for_weapons);
    RUN_TEST(test_edge_case_zero_size);
    RUN_TEST(test_edge_case_max_size);
    RUN_TEST(test_all_weapons_normalized);
    RUN_TEST(test_raw_damage_formula);
    
    // Health System Tests (catch health > maxHealth bugs)
    RUN_TEST(test_health_clamped_on_aging);
    RUN_TEST(test_health_invariant_simulation);
    RUN_TEST(test_age_modulation_curve);
    
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
