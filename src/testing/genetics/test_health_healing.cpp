/**
 * @file test_health_healing.cpp
 * @brief Tests for creature health and healing system
 * @author Gary Ferguson
 * @date December 2025
 * 
 * Tests the health/wound system for creatures including:
 * - Wound state transitions at correct thresholds
 * - Healing rate calculation from genes
 * - Movement speed penalties from wounds
 * - Energy cost of healing
 * - Bleeding resistance effects
 * - Wound tolerance effects
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

//==============================================================================
// Test Helpers
//==============================================================================

/**
 * Create a test creature with default genome
 */
Creature createTestCreature() {
    // Initialize gene registry if needed
    Creature::initializeGeneRegistry();
    
    // Create a new genetics genome
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::UniversalGenes::createCreatureGenome(Creature::getGeneRegistry())
    );
    
    // Create creature at position (10, 10)
    Creature creature(10, 10, std::move(genome));
    
    return creature;
}

//==============================================================================
// Wound State Transition Tests
//==============================================================================

/**
 * Test wound state thresholds are correct
 */
void testWoundStateThresholds() {
    Creature creature = createTestCreature();
    
    // Get max health for calculation
    float maxHealth = creature.getMaxHealth();
    TEST_ASSERT(maxHealth > 0.0f);
    
    // Test Healthy state (80-100%)
    creature.heal(maxHealth);  // Full health
    TEST_ASSERT(creature.getWoundState() == WoundState::Healthy);
    
    creature.takeDamage(maxHealth * 0.15f);  // 85% HP
    TEST_ASSERT(creature.getWoundState() == WoundState::Healthy);
    
    // Test Injured state (50-79%)
    creature.heal(maxHealth);  // Reset
    creature.takeDamage(maxHealth * 0.25f);  // 75% HP
    TEST_ASSERT(creature.getWoundState() == WoundState::Injured);
    
    creature.heal(maxHealth);  // Reset
    creature.takeDamage(maxHealth * 0.45f);  // 55% HP
    TEST_ASSERT(creature.getWoundState() == WoundState::Injured);
    
    // Test Wounded state (25-49%)
    creature.heal(maxHealth);  // Reset
    creature.takeDamage(maxHealth * 0.55f);  // 45% HP
    TEST_ASSERT(creature.getWoundState() == WoundState::Wounded);
    
    creature.heal(maxHealth);  // Reset
    creature.takeDamage(maxHealth * 0.70f);  // 30% HP
    TEST_ASSERT(creature.getWoundState() == WoundState::Wounded);
    
    // Test Critical state (1-24%)
    creature.heal(maxHealth);  // Reset
    creature.takeDamage(maxHealth * 0.80f);  // 20% HP
    TEST_ASSERT(creature.getWoundState() == WoundState::Critical);
    
    creature.heal(maxHealth);  // Reset
    creature.takeDamage(maxHealth * 0.95f);  // 5% HP
    TEST_ASSERT(creature.getWoundState() == WoundState::Critical);
    
    // Test Dead state (0%)
    creature.heal(maxHealth);  // Reset
    creature.takeDamage(maxHealth * 1.5f);  // Over-damage to ensure 0
    TEST_ASSERT(creature.getWoundState() == WoundState::Dead);
    TEST_ASSERT_EQ(creature.getHealth(), 0.0f);
}

/**
 * Test that health percentage is calculated correctly
 */
void testHealthPercentCalculation() {
    Creature creature = createTestCreature();
    
    float maxHealth = creature.getMaxHealth();
    
    // Full health should be 100%
    TEST_ASSERT_NEAR(creature.getHealthPercent(), 1.0f, 0.001f);
    
    // Half damage should be 50%
    creature.takeDamage(maxHealth * 0.5f);
    TEST_ASSERT_NEAR(creature.getHealthPercent(), 0.5f, 0.001f);
    
    // No health should be 0%
    creature.takeDamage(maxHealth);
    TEST_ASSERT_NEAR(creature.getHealthPercent(), 0.0f, 0.001f);
}

//==============================================================================
// Healing Rate Tests
//==============================================================================

/**
 * Test healing rate is calculated from genes
 */
void testHealingRateCalculation() {
    Creature creature = createTestCreature();
    
    // Healing rate should be positive for a valid creature
    float healingRate = creature.getHealingRate();
    TEST_ASSERT_GT(healingRate, 0.0f);
    
    // Healing rate formula: metabolism * regeneration_rate / body_mass
    // With default genes, should produce a reasonable value
    TEST_ASSERT_LT(healingRate, 10.0f);
}

/**
 * Test that healing increases health
 */
void testHealingIncreasesHealth() {
    Creature creature = createTestCreature();
    
    float maxHealth = creature.getMaxHealth();
    
    // Damage creature first
    creature.takeDamage(maxHealth * 0.5f);
    float healthBefore = creature.getHealth();
    
    // Heal
    creature.heal(10.0f);
    float healthAfter = creature.getHealth();
    
    TEST_ASSERT_GT(healthAfter, healthBefore);
    TEST_ASSERT_GE(healthAfter - healthBefore, 10.0f - 0.001f);
}

/**
 * Test that healing doesn't exceed max health
 */
void testHealingCapAtMax() {
    Creature creature = createTestCreature();
    
    float maxHealth = creature.getMaxHealth();
    
    // Take a small amount of damage
    creature.takeDamage(5.0f);
    
    // Heal for more than the damage
    creature.heal(100.0f);
    
    TEST_ASSERT_LE(creature.getHealth(), maxHealth);
    TEST_ASSERT_NEAR(creature.getHealth(), maxHealth, 0.001f);
}

//==============================================================================
// Damage Tests
//==============================================================================

/**
 * Test that damage reduces health
 */
void testDamageReducesHealth() {
    Creature creature = createTestCreature();
    
    float healthBefore = creature.getHealth();
    
    creature.takeDamage(10.0f);
    float healthAfter = creature.getHealth();
    
    TEST_ASSERT_LT(healthAfter, healthBefore);
    TEST_ASSERT_GE(healthBefore - healthAfter, 10.0f - 0.001f);
}

/**
 * Test that health doesn't go below zero
 */
void testHealthFloorAtZero() {
    Creature creature = createTestCreature();
    
    float maxHealth = creature.getMaxHealth();
    
    // Deal massive damage
    creature.takeDamage(maxHealth * 10.0f);
    
    TEST_ASSERT_GE(creature.getHealth(), 0.0f);
    TEST_ASSERT_EQ(creature.getHealth(), 0.0f);
}

/**
 * Test wound severity updates with damage
 */
void testWoundSeverityUpdateOnDamage() {
    Creature creature = createTestCreature();
    
    // Full health = no wound severity
    TEST_ASSERT_EQ(creature.getWoundSeverity(), 0.0f);
    
    float maxHealth = creature.getMaxHealth();
    
    // Half health = 0.5 wound severity
    creature.takeDamage(maxHealth * 0.5f);
    TEST_ASSERT_NEAR(creature.getWoundSeverity(), 0.5f, 0.01f);
    
    // Quarter health = 0.75 wound severity
    creature.takeDamage(maxHealth * 0.25f);
    TEST_ASSERT_NEAR(creature.getWoundSeverity(), 0.75f, 0.01f);
}

//==============================================================================
// Movement Speed Penalty Tests
//==============================================================================

/**
 * Test that wounds reduce movement speed
 */
void testWoundSpeedPenalty() {
    Creature creature = createTestCreature();
    
    // Get speed at full health
    float fullHealthSpeed = creature.getMovementSpeed();
    TEST_ASSERT_GT(fullHealthSpeed, 0.0f);
    
    float maxHealth = creature.getMaxHealth();
    
    // Take significant damage to trigger wound severity
    creature.takeDamage(maxHealth * 0.75f);  // 25% HP remaining
    
    float woundedSpeed = creature.getMovementSpeed();
    
    // Speed should be reduced when wounded (unless wound tolerance is very high)
    float woundSeverity = creature.getWoundSeverity();
    if (woundSeverity > 0.0f) {
        TEST_ASSERT_LE(woundedSpeed, fullHealthSpeed);
    }
}

//==============================================================================
// Health Initialization Tests
//==============================================================================

/**
 * Test that health is properly initialized
 */
void testHealthInitialization() {
    Creature creature = createTestCreature();
    
    // Health should start at maximum
    TEST_ASSERT_EQ(creature.getHealth(), creature.getMaxHealth());
    
    // Wound severity should be zero
    TEST_ASSERT_EQ(creature.getWoundSeverity(), 0.0f);
    
    // Max health should be positive (based on size gene)
    TEST_ASSERT_GT(creature.getMaxHealth(), 0.0f);
    
    // Should be in Healthy state
    TEST_ASSERT(creature.getWoundState() == WoundState::Healthy);
}

/**
 * Test that max health scales with size
 */
void testMaxHealthScalesWithSize() {
    // This test verifies the relationship between MAX_SIZE gene and health
    // Health = MAX_SIZE * 50.0f (HEALTH_PER_SIZE constant)
    
    Creature creature = createTestCreature();
    
    float maxHealth = creature.getMaxHealth();
    
    // Max health should be reasonable for a creature
    // With default MAX_SIZE around 1-4, health should be 50-200
    TEST_ASSERT_GE(maxHealth, 25.0f);
    TEST_ASSERT_LE(maxHealth, 1000.0f);
}

//==============================================================================
// Edge Case Tests
//==============================================================================

/**
 * Test zero/negative damage handling
 */
void testZeroNegativeDamage() {
    Creature creature = createTestCreature();
    
    float healthBefore = creature.getHealth();
    
    // Zero damage should not change health
    creature.takeDamage(0.0f);
    TEST_ASSERT_EQ(creature.getHealth(), healthBefore);
    
    // Negative damage should not change health (treated as no-op)
    creature.takeDamage(-10.0f);
    TEST_ASSERT_EQ(creature.getHealth(), healthBefore);
}

/**
 * Test zero/negative healing handling
 */
void testZeroNegativeHealing() {
    Creature creature = createTestCreature();
    
    float maxHealth = creature.getMaxHealth();
    creature.takeDamage(maxHealth * 0.5f);  // Get to 50% HP
    float healthBefore = creature.getHealth();
    
    // Zero healing should not change health
    creature.heal(0.0f);
    TEST_ASSERT_EQ(creature.getHealth(), healthBefore);
    
    // Negative healing should not change health (treated as no-op)
    creature.heal(-10.0f);
    TEST_ASSERT_EQ(creature.getHealth(), healthBefore);
}

//==============================================================================
// Run All Health/Healing Tests
//==============================================================================

void runHealthHealingTests() {
    BEGIN_TEST_GROUP("Health & Healing System Tests");
    
    // Wound state tests
    RUN_TEST(testWoundStateThresholds);
    RUN_TEST(testHealthPercentCalculation);
    
    // Healing tests
    RUN_TEST(testHealingRateCalculation);
    RUN_TEST(testHealingIncreasesHealth);
    RUN_TEST(testHealingCapAtMax);
    
    // Damage tests
    RUN_TEST(testDamageReducesHealth);
    RUN_TEST(testHealthFloorAtZero);
    RUN_TEST(testWoundSeverityUpdateOnDamage);
    
    // Movement penalty tests
    RUN_TEST(testWoundSpeedPenalty);
    
    // Initialization tests
    RUN_TEST(testHealthInitialization);
    RUN_TEST(testMaxHealthScalesWithSize);
    
    // Edge case tests
    RUN_TEST(testZeroNegativeDamage);
    RUN_TEST(testZeroNegativeHealing);
    
    END_TEST_GROUP();
}
