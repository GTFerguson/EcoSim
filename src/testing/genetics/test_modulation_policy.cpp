/**
 * @file test_modulation_policy.cpp
 * @brief Comprehensive tests for the TraitModulationPolicy system
 * 
 * Tests verify that phenotype modulation correctly follows the policy system:
 * - NEVER: Physical structure traits remain constant regardless of organism state
 * - HEALTH_ONLY: Metabolic efficiency traits only affected by health
 * - ENERGY_GATED: Production traits return raw capacity (consumer checks energy)
 * - CONSUMER_APPLIED: Performance traits return raw value (consumer applies modulation)
 * 
 * This test suite verifies the phenotype modulation refactor that fixes the issue
 * where structural traits like hide_thickness would incorrectly vary based on the
 * killer's state rather than the corpse's inherent properties.
 */

#include <iostream>
#include <cmath>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Create a standard test phenotype with creature genome
 */
G::Phenotype createTestPhenotype(G::GeneRegistry& registry, G::Genome& genome) {
    G::Phenotype phenotype(&genome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;  // Neutral temperature
    env.time_of_day = 0.5f;   // Noon (full light)
    env.humidity = 0.5f;      // Neutral humidity
    
    G::OrganismState org;
    org.age_normalized = 0.5f;   // Adult (prime)
    org.energy_level = 1.0f;     // Full energy
    org.health = 1.0f;           // Full health
    org.is_sleeping = false;
    
    phenotype.updateContext(env, org);
    return phenotype;
}

/**
 * @brief Update phenotype with specific organism state
 */
void updateOrganismState(G::Phenotype& phenotype, float energy, float health, bool sleeping) {
    G::EnvironmentState env;
    env.temperature = 20.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.5f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;  // Keep at adult age for consistent comparison
    org.energy_level = energy;
    org.health = health;
    org.is_sleeping = sleeping;
    
    phenotype.updateContext(env, org);
}

// ============================================================================
// Policy Infrastructure Tests
// ============================================================================

void testTraitModulationPolicyEnumExists() {
    // Verify all enum values exist and have distinct values
    G::TraitModulationPolicy never = G::TraitModulationPolicy::NEVER;
    G::TraitModulationPolicy healthOnly = G::TraitModulationPolicy::HEALTH_ONLY;
    G::TraitModulationPolicy energyGated = G::TraitModulationPolicy::ENERGY_GATED;
    G::TraitModulationPolicy consumerApplied = G::TraitModulationPolicy::CONSUMER_APPLIED;
    
    // They should all be different
    TEST_ASSERT(never != healthOnly);
    TEST_ASSERT(never != energyGated);
    TEST_ASSERT(never != consumerApplied);
    TEST_ASSERT(healthOnly != energyGated);
    TEST_ASSERT(healthOnly != consumerApplied);
    TEST_ASSERT(energyGated != consumerApplied);
}

void testGeneDefinitionStoresPolicy() {
    // Create a gene definition and set its policy
    G::GeneLimits limits(0.0f, 1.0f, 0.05f);
    G::GeneDefinition def("test_gene", G::ChromosomeType::Morphology, limits);
    
    // Default policy should be NEVER (as per Gene.hpp line 252)
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::NEVER);
    
    // Set and verify each policy type
    def.setModulationPolicy(G::TraitModulationPolicy::HEALTH_ONLY);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::HEALTH_ONLY);
    
    def.setModulationPolicy(G::TraitModulationPolicy::ENERGY_GATED);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::ENERGY_GATED);
    
    def.setModulationPolicy(G::TraitModulationPolicy::CONSUMER_APPLIED);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::CONSUMER_APPLIED);
    
    def.setModulationPolicy(G::TraitModulationPolicy::NEVER);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::NEVER);
}

void testRegistryStoresGenePolicy() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Verify HIDE_THICKNESS has NEVER policy
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::HIDE_THICKNESS));
    const G::GeneDefinition& hideDef = registry.getDefinition(G::UniversalGenes::HIDE_THICKNESS);
    TEST_ASSERT(hideDef.getModulationPolicy() == G::TraitModulationPolicy::NEVER);
    
    // Verify MEAT_DIGESTION_EFFICIENCY has HEALTH_ONLY policy
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY));
    const G::GeneDefinition& meatDef = registry.getDefinition(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    TEST_ASSERT(meatDef.getModulationPolicy() == G::TraitModulationPolicy::HEALTH_ONLY);
    
    // Verify TOXIN_PRODUCTION has ENERGY_GATED policy
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::TOXIN_PRODUCTION));
    const G::GeneDefinition& toxinDef = registry.getDefinition(G::UniversalGenes::TOXIN_PRODUCTION);
    TEST_ASSERT(toxinDef.getModulationPolicy() == G::TraitModulationPolicy::ENERGY_GATED);
    
    // Verify LOCOMOTION has CONSUMER_APPLIED policy
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::LOCOMOTION));
    const G::GeneDefinition& locoDef = registry.getDefinition(G::UniversalGenes::LOCOMOTION);
    TEST_ASSERT(locoDef.getModulationPolicy() == G::TraitModulationPolicy::CONSUMER_APPLIED);
}

// ============================================================================
// NEVER Policy Tests - Physical Structure Traits
// ============================================================================

void testNeverPolicyHideThicknessUnaffectedByEnergy() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float baseValue = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    
    // Low energy should NOT affect hide thickness
    updateOrganismState(phenotype, 0.1f, 1.0f, false);
    phenotype.invalidateCache();
    float lowEnergyValue = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    
    TEST_ASSERT_NEAR(baseValue, lowEnergyValue, 0.001f);
}

void testNeverPolicyHideThicknessUnaffectedByHealth() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float baseValue = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    
    // Low health should NOT affect hide thickness
    updateOrganismState(phenotype, 1.0f, 0.2f, false);
    phenotype.invalidateCache();
    float lowHealthValue = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    
    TEST_ASSERT_NEAR(baseValue, lowHealthValue, 0.001f);
}

void testNeverPolicyHideThicknessUnaffectedBySleep() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float baseValue = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    
    // Sleeping should NOT affect hide thickness
    updateOrganismState(phenotype, 1.0f, 1.0f, true);
    phenotype.invalidateCache();
    float sleepingValue = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    
    TEST_ASSERT_NEAR(baseValue, sleepingValue, 0.001f);
}

void testNeverPolicyToothSharpnessConstant() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float baseValue = phenotype.getTrait(G::UniversalGenes::TOOTH_SHARPNESS);
    
    // Test various stressed states
    updateOrganismState(phenotype, 0.1f, 0.2f, false);  // Low energy, low health
    phenotype.invalidateCache();
    float stressedValue = phenotype.getTrait(G::UniversalGenes::TOOTH_SHARPNESS);
    
    TEST_ASSERT_NEAR(baseValue, stressedValue, 0.001f);
}

void testNeverPolicyTeethSharpnessConstant() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    
    // TEETH_SHARPNESS (combat weapon gene) should have NEVER policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::TEETH_SHARPNESS);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::NEVER);
    
    float baseValue = phenotype.getTrait(G::UniversalGenes::TEETH_SHARPNESS);
    
    updateOrganismState(phenotype, 0.1f, 0.2f, true);  // Worst case state
    phenotype.invalidateCache();
    float stressedValue = phenotype.getTrait(G::UniversalGenes::TEETH_SHARPNESS);
    
    TEST_ASSERT_NEAR(baseValue, stressedValue, 0.001f);
}

void testNeverPolicyClawSharpnessConstant() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float baseValue = phenotype.getTrait(G::UniversalGenes::CLAW_SHARPNESS);
    
    updateOrganismState(phenotype, 0.1f, 0.2f, true);  // Worst case state
    phenotype.invalidateCache();
    float stressedValue = phenotype.getTrait(G::UniversalGenes::CLAW_SHARPNESS);
    
    TEST_ASSERT_NEAR(baseValue, stressedValue, 0.001f);
}

void testNeverPolicyEqualsRawValue() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    
    // For NEVER policy traits, getTrait() should equal computeTraitRaw()
    // (except for age modulation which both apply)
    float traitValue = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::HIDE_THICKNESS);
    
    // At age 0.5 (adult prime), age modulation factor is 1.0
    // So trait value should equal raw value
    TEST_ASSERT_NEAR(traitValue, rawValue, 0.001f);
}

// ============================================================================
// HEALTH_ONLY Policy Tests - Metabolic Efficiency Traits
// ============================================================================

void testHealthOnlyPolicyFullHealth() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    
    // At full health (1.0), no modulation should occur
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float traitValue = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    // At adult age and full health, trait should equal raw
    TEST_ASSERT_NEAR(traitValue, rawValue, 0.001f);
}

void testHealthOnlyPolicyModerateHealth() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    // At health = 0.6 (above 0.5 threshold), no modulation should occur
    updateOrganismState(phenotype, 1.0f, 0.6f, false);
    phenotype.invalidateCache();
    float traitValue = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    TEST_ASSERT_NEAR(traitValue, rawValue, 0.001f);
}

void testHealthOnlyPolicyLowHealth() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    // At health = 0.3 (below 0.5 threshold), modulation should occur
    // health_factor = 0.7 + (0.3/0.5) * 0.3 = 0.7 + 0.6 * 0.3 = 0.7 + 0.18 = 0.88
    updateOrganismState(phenotype, 1.0f, 0.3f, false);
    phenotype.invalidateCache();
    float traitValue = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    float expectedFactor = 0.7f + (0.3f / 0.5f) * 0.3f;  // 0.88
    float expectedValue = rawValue * expectedFactor;
    
    TEST_ASSERT_NEAR(traitValue, expectedValue, 0.01f);
}

void testHealthOnlyPolicyVeryLowHealth() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    
    // At health = 0.1, modulation should be more severe
    // health_factor = 0.7 + (0.1/0.5) * 0.3 = 0.7 + 0.2 * 0.3 = 0.7 + 0.06 = 0.76
    updateOrganismState(phenotype, 1.0f, 0.1f, false);
    phenotype.invalidateCache();
    float traitValue = phenotype.getTrait(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    
    float expectedFactor = 0.7f + (0.1f / 0.5f) * 0.3f;  // 0.76
    float expectedValue = rawValue * expectedFactor;
    
    TEST_ASSERT_NEAR(traitValue, expectedValue, 0.01f);
}

void testHealthOnlyPolicyUnaffectedByEnergy() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float baseValue = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    // Low energy should NOT affect HEALTH_ONLY traits
    updateOrganismState(phenotype, 0.1f, 1.0f, false);
    phenotype.invalidateCache();
    float lowEnergyValue = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    TEST_ASSERT_NEAR(baseValue, lowEnergyValue, 0.001f);
}

void testHealthOnlyPolicyCelluloseBreakdown() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify CELLULOSE_BREAKDOWN has HEALTH_ONLY policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::CELLULOSE_BREAKDOWN);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::HEALTH_ONLY);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::CELLULOSE_BREAKDOWN);
    
    // At health = 0.25
    updateOrganismState(phenotype, 1.0f, 0.25f, false);
    phenotype.invalidateCache();
    float traitValue = phenotype.getTrait(G::UniversalGenes::CELLULOSE_BREAKDOWN);
    
    float expectedFactor = 0.7f + (0.25f / 0.5f) * 0.3f;  // 0.85
    float expectedValue = rawValue * expectedFactor;
    
    TEST_ASSERT_NEAR(traitValue, expectedValue, 0.01f);
}

// ============================================================================
// ENERGY_GATED Policy Tests - Production Traits
// ============================================================================

void testEnergyGatedPolicyToxinProduction() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify TOXIN_PRODUCTION has ENERGY_GATED policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::TOXIN_PRODUCTION);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::ENERGY_GATED);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::TOXIN_PRODUCTION);
    
    // ENERGY_GATED returns unmodified value
    float traitValue = phenotype.getTrait(G::UniversalGenes::TOXIN_PRODUCTION);
    TEST_ASSERT_NEAR(traitValue, rawValue, 0.001f);
}

void testEnergyGatedPolicyUnaffectedByState() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::TOXIN_PRODUCTION);
    
    // Low energy, low health, sleeping - should still return raw value
    updateOrganismState(phenotype, 0.1f, 0.2f, true);
    phenotype.invalidateCache();
    float stressedValue = phenotype.getTrait(G::UniversalGenes::TOXIN_PRODUCTION);
    
    TEST_ASSERT_NEAR(rawValue, stressedValue, 0.001f);
}

void testEnergyGatedPolicyScentProduction() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify SCENT_PRODUCTION has ENERGY_GATED policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::SCENT_PRODUCTION);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::ENERGY_GATED);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::SCENT_PRODUCTION);
    
    // Various states should not affect the returned value
    updateOrganismState(phenotype, 0.1f, 0.1f, true);
    phenotype.invalidateCache();
    float traitValue = phenotype.getTrait(G::UniversalGenes::SCENT_PRODUCTION);
    
    TEST_ASSERT_NEAR(traitValue, rawValue, 0.001f);
}

void testEnergyGatedPolicyRegenerationRate() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify REGENERATION_RATE has ENERGY_GATED policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::REGENERATION_RATE);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::ENERGY_GATED);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::REGENERATION_RATE);
    float traitValue = phenotype.getTrait(G::UniversalGenes::REGENERATION_RATE);
    
    TEST_ASSERT_NEAR(traitValue, rawValue, 0.001f);
}

// ============================================================================
// CONSUMER_APPLIED Policy Tests - Performance Traits
// ============================================================================

void testConsumerAppliedPolicyLocomotion() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify LOCOMOTION has CONSUMER_APPLIED policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::LOCOMOTION);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::CONSUMER_APPLIED);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::LOCOMOTION);
    
    // CONSUMER_APPLIED returns unmodified value from phenotype
    float traitValue = phenotype.getTrait(G::UniversalGenes::LOCOMOTION);
    TEST_ASSERT_NEAR(traitValue, rawValue, 0.001f);
}

void testConsumerAppliedPolicyUnaffectedByOrganismState() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::LOCOMOTION);
    
    // Stressed state should not affect CONSUMER_APPLIED traits
    updateOrganismState(phenotype, 0.1f, 0.2f, true);
    phenotype.invalidateCache();
    float stressedValue = phenotype.getTrait(G::UniversalGenes::LOCOMOTION);
    
    TEST_ASSERT_NEAR(rawValue, stressedValue, 0.001f);
}

void testConsumerAppliedPolicySightRange() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify SIGHT_RANGE has CONSUMER_APPLIED policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::SIGHT_RANGE);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::CONSUMER_APPLIED);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    
    updateOrganismState(phenotype, 0.1f, 0.2f, false);
    phenotype.invalidateCache();
    float stressedValue = phenotype.getTrait(G::UniversalGenes::SIGHT_RANGE);
    
    // SIGHT_RANGE may be affected by environment (time of day for vision)
    // but should NOT be affected by organism state via modulation policy
    // The raw value represents the genetic potential
    TEST_ASSERT_GT(stressedValue, 0.0f);
}

void testConsumerAppliedPolicyHuntInstinct() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify HUNT_INSTINCT has CONSUMER_APPLIED policy
    const G::GeneDefinition& def = registry.getDefinition(G::UniversalGenes::HUNT_INSTINCT);
    TEST_ASSERT(def.getModulationPolicy() == G::TraitModulationPolicy::CONSUMER_APPLIED);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    float rawValue = phenotype.computeTraitRaw(G::UniversalGenes::HUNT_INSTINCT);
    
    updateOrganismState(phenotype, 0.1f, 0.2f, true);
    phenotype.invalidateCache();
    float stressedValue = phenotype.getTrait(G::UniversalGenes::HUNT_INSTINCT);
    
    TEST_ASSERT_NEAR(rawValue, stressedValue, 0.001f);
}

// ============================================================================
// Integration Tests - Corpse Value Scenario
// ============================================================================

void testCorpseValueUnaffectedByKillerState() {
    // This test verifies the core issue that the modulation policy system fixes:
    // A creature's corpse value (hide, teeth, nutrient value) should NOT vary
    // based on who is accessing it or their state.
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome corpseGenome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Create phenotype representing the corpse
    G::Phenotype corpsePhenotype(&corpseGenome, &registry);
    
    // Set corpse state (dead creature - state doesn't matter for structural traits)
    G::EnvironmentState env;
    env.temperature = 20.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.5f;
    
    G::OrganismState corpseState;
    corpseState.age_normalized = 0.5f;  // Adult when it died
    corpseState.energy_level = 0.0f;    // Dead - no energy
    corpseState.health = 0.0f;          // Dead - no health
    corpseState.is_sleeping = false;
    
    corpsePhenotype.updateContext(env, corpseState);
    
    // Get corpse structural values - these should be constant
    float hideThickness = corpsePhenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    float toothSharpness = corpsePhenotype.getTrait(G::UniversalGenes::TOOTH_SHARPNESS);
    float clawSharpness = corpsePhenotype.getTrait(G::UniversalGenes::CLAW_SHARPNESS);
    
    // Get raw values for comparison
    float hideRaw = corpsePhenotype.computeTraitRaw(G::UniversalGenes::HIDE_THICKNESS);
    float toothRaw = corpsePhenotype.computeTraitRaw(G::UniversalGenes::TOOTH_SHARPNESS);
    float clawRaw = corpsePhenotype.computeTraitRaw(G::UniversalGenes::CLAW_SHARPNESS);
    
    // Structural traits should equal raw values (at adult age)
    TEST_ASSERT_NEAR(hideThickness, hideRaw, 0.001f);
    TEST_ASSERT_NEAR(toothSharpness, toothRaw, 0.001f);
    TEST_ASSERT_NEAR(clawSharpness, clawRaw, 0.001f);
}

void testStructuralVsPerformanceTraitBehavior() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    
    // Get baseline values at full health/energy
    float hideBase = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    float locoBase = phenotype.getTrait(G::UniversalGenes::LOCOMOTION);
    float meatDigestBase = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    // Update to low health (below 0.5 threshold)
    updateOrganismState(phenotype, 1.0f, 0.3f, false);
    phenotype.invalidateCache();
    
    float hideLowHealth = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    float locoLowHealth = phenotype.getTrait(G::UniversalGenes::LOCOMOTION);
    float meatDigestLowHealth = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    // NEVER policy (hide): Should be unchanged
    TEST_ASSERT_NEAR(hideBase, hideLowHealth, 0.001f);
    
    // CONSUMER_APPLIED policy (locomotion): Should be unchanged (consumer applies modulation)
    TEST_ASSERT_NEAR(locoBase, locoLowHealth, 0.001f);
    
    // HEALTH_ONLY policy (meat digestion): Should be reduced
    TEST_ASSERT_LT(meatDigestLowHealth, meatDigestBase);
}

void testPolicyCategorization() {
    // Verify that the 92 genes have been properly categorized
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    int neverCount = 0;
    int healthOnlyCount = 0;
    int energyGatedCount = 0;
    int consumerAppliedCount = 0;
    
    const auto& allDefs = registry.getAllDefinitions();
    for (const auto& [id, def] : allDefs) {
        switch (def.getModulationPolicy()) {
            case G::TraitModulationPolicy::NEVER:
                neverCount++;
                break;
            case G::TraitModulationPolicy::HEALTH_ONLY:
                healthOnlyCount++;
                break;
            case G::TraitModulationPolicy::ENERGY_GATED:
                energyGatedCount++;
                break;
            case G::TraitModulationPolicy::CONSUMER_APPLIED:
                consumerAppliedCount++;
                break;
        }
    }
    
    // We should have genes in each category
    TEST_ASSERT_GT(neverCount, 0);
    TEST_ASSERT_GT(healthOnlyCount, 0);
    TEST_ASSERT_GT(energyGatedCount, 0);
    TEST_ASSERT_GT(consumerAppliedCount, 0);
    
    // Total should match registry size
    int total = neverCount + healthOnlyCount + energyGatedCount + consumerAppliedCount;
    TEST_ASSERT_EQ(static_cast<int>(registry.size()), total);
    
    std::cout << "    Gene policy distribution:" << std::endl;
    std::cout << "      NEVER: " << neverCount << std::endl;
    std::cout << "      HEALTH_ONLY: " << healthOnlyCount << std::endl;
    std::cout << "      ENERGY_GATED: " << energyGatedCount << std::endl;
    std::cout << "      CONSUMER_APPLIED: " << consumerAppliedCount << std::endl;
}

void testMultipleTraitsSimultaneously() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    G::Phenotype phenotype = createTestPhenotype(registry, genome);
    
    // Get baseline values for multiple traits from each policy category
    float hideBase = phenotype.computeTraitRaw(G::UniversalGenes::HIDE_THICKNESS);    // NEVER
    float meatBase = phenotype.computeTraitRaw(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY); // HEALTH_ONLY
    float toxinBase = phenotype.computeTraitRaw(G::UniversalGenes::TOXIN_PRODUCTION); // ENERGY_GATED
    float locoBase = phenotype.computeTraitRaw(G::UniversalGenes::LOCOMOTION);        // CONSUMER_APPLIED
    
    // Set to stressed state (low health triggers HEALTH_ONLY modulation)
    updateOrganismState(phenotype, 0.2f, 0.3f, true);
    phenotype.invalidateCache();
    
    float hideStressed = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    float meatStressed = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float toxinStressed = phenotype.getTrait(G::UniversalGenes::TOXIN_PRODUCTION);
    float locoStressed = phenotype.getTrait(G::UniversalGenes::LOCOMOTION);
    
    // NEVER: unchanged
    TEST_ASSERT_NEAR(hideBase, hideStressed, 0.001f);
    
    // HEALTH_ONLY: reduced (health 0.3 < 0.5)
    float healthFactor = 0.7f + (0.3f / 0.5f) * 0.3f;
    TEST_ASSERT_NEAR(meatBase * healthFactor, meatStressed, 0.01f);
    
    // ENERGY_GATED: unchanged
    TEST_ASSERT_NEAR(toxinBase, toxinStressed, 0.001f);
    
    // CONSUMER_APPLIED: unchanged
    TEST_ASSERT_NEAR(locoBase, locoStressed, 0.001f);
}

// ============================================================================
// Test Runner
// ============================================================================

void runModulationPolicyTests() {
    BEGIN_TEST_GROUP("Policy Infrastructure Tests");
    RUN_TEST(testTraitModulationPolicyEnumExists);
    RUN_TEST(testGeneDefinitionStoresPolicy);
    RUN_TEST(testRegistryStoresGenePolicy);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("NEVER Policy Tests (Physical Structure Traits)");
    RUN_TEST(testNeverPolicyHideThicknessUnaffectedByEnergy);
    RUN_TEST(testNeverPolicyHideThicknessUnaffectedByHealth);
    RUN_TEST(testNeverPolicyHideThicknessUnaffectedBySleep);
    RUN_TEST(testNeverPolicyToothSharpnessConstant);
    RUN_TEST(testNeverPolicyTeethSharpnessConstant);
    RUN_TEST(testNeverPolicyClawSharpnessConstant);
    RUN_TEST(testNeverPolicyEqualsRawValue);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("HEALTH_ONLY Policy Tests (Metabolic Efficiency Traits)");
    RUN_TEST(testHealthOnlyPolicyFullHealth);
    RUN_TEST(testHealthOnlyPolicyModerateHealth);
    RUN_TEST(testHealthOnlyPolicyLowHealth);
    RUN_TEST(testHealthOnlyPolicyVeryLowHealth);
    RUN_TEST(testHealthOnlyPolicyUnaffectedByEnergy);
    RUN_TEST(testHealthOnlyPolicyCelluloseBreakdown);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("ENERGY_GATED Policy Tests (Production Traits)");
    RUN_TEST(testEnergyGatedPolicyToxinProduction);
    RUN_TEST(testEnergyGatedPolicyUnaffectedByState);
    RUN_TEST(testEnergyGatedPolicyScentProduction);
    RUN_TEST(testEnergyGatedPolicyRegenerationRate);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("CONSUMER_APPLIED Policy Tests (Performance Traits)");
    RUN_TEST(testConsumerAppliedPolicyLocomotion);
    RUN_TEST(testConsumerAppliedPolicyUnaffectedByOrganismState);
    RUN_TEST(testConsumerAppliedPolicySightRange);
    RUN_TEST(testConsumerAppliedPolicyHuntInstinct);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Integration Tests");
    RUN_TEST(testCorpseValueUnaffectedByKillerState);
    RUN_TEST(testStructuralVsPerformanceTraitBehavior);
    RUN_TEST(testPolicyCategorization);
    RUN_TEST(testMultipleTraitsSimultaneously);
    END_TEST_GROUP();
}

#ifdef TEST_MODULATION_POLICY_STANDALONE
int main() {
    std::cout << "=== TraitModulationPolicy Tests ===" << std::endl;
    runModulationPolicyTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
