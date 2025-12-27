/**
 * @file test_health_system.cpp
 * @brief Comprehensive tests for the organism-agnostic HealthSystem
 *
 * Tests damage application, healing, regeneration, wound states,
 * and resistance calculations for both creatures and plants.
 *
 * The HealthSystem works through IGeneticOrganism interface using phenotype
 * traits only - no type-specific code.
 */

#include "test_framework.hpp"
#include "genetics/systems/HealthSystem.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/Chromosome.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <iostream>
#include <memory>
#include <cmath>

using namespace EcoSim;
using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//================================================================================
//  MockOrganism: Test Implementation of IGeneticOrganism
//================================================================================

/**
 * @brief Mock organism for testing HealthSystem
 *
 * Implements IGeneticOrganism interface with configurable traits.
 * Traits can be set directly for precise test control.
 */
class HealthMockOrganism : public IGeneticOrganism {
public:
    HealthMockOrganism() : genome_(), phenotype_(&genome_, &registry_) {
        initializeRegistry();
        // Set optimal organism state so modulation returns 100% values
        setOptimalState();
    }

    const Genome& getGenome() const override {
        return genome_;
    }

    Genome& getGenomeMutable() override {
        return genome_;
    }

    const Phenotype& getPhenotype() const override {
        return phenotype_;
    }

    void updatePhenotype() override {
        phenotype_.invalidateCache();
    }

    // Position and ID methods (required by IGeneticOrganism)
    int getX() const override { return 0; }
    int getY() const override { return 0; }
    int getId() const override { return 0; }

    /**
     * @brief Set a trait directly on the genome for testing
     */
    void setTrait(const std::string& name, float value) {
        // Determine appropriate chromosome
        ChromosomeType chromType = ChromosomeType::Metabolism;
        
        if (name == UniversalGenes::HARDINESS || name == UniversalGenes::MAX_SIZE) {
            chromType = ChromosomeType::Morphology;
        } else if (name == UniversalGenes::TOXIN_TOLERANCE) {
            chromType = ChromosomeType::Environmental;
        } else if (name == UniversalGenes::REGROWTH_RATE || 
                   name == UniversalGenes::REGENERATION_RATE) {
            chromType = ChromosomeType::Metabolism;
        }
        
        // Check if gene already exists
        if (genome_.hasGene(name)) {
            Gene& existingGene = genome_.getGeneMutable(name);
            existingGene.setAlleleValues(value);
        } else {
            GeneValue geneVal = value;
            Gene gene(name, geneVal);
            genome_.addGene(gene, chromType);
        }
        
        // Ensure gene is in registry
        if (!registry_.hasGene(name)) {
            float maxVal = (name == UniversalGenes::MAX_SIZE) ? 10.0f : 1.0f;
            GeneLimits limits(0.0f, maxVal, 0.05f);
            GeneDefinition def(name, chromType, limits, DominanceType::Incomplete);
            registry_.registerGene(def);
        }
        
        phenotype_.invalidateCache();
    }

    /**
     * @brief Configure as a plant-like organism (fixed max health = 1.0)
     */
    void configureAsPlant() {
        setTrait(UniversalGenes::HARDINESS, 0.5f);
        setTrait(UniversalGenes::TOXIN_TOLERANCE, 0.0f);
        setTrait(UniversalGenes::REGROWTH_RATE, 0.3f);
        // Plants don't have max_size trait - use default health
    }

    /**
     * @brief Configure as a creature-like organism
     */
    void configureAsCreature() {
        setTrait(UniversalGenes::HARDINESS, 0.5f);
        setTrait(UniversalGenes::TOXIN_TOLERANCE, 0.3f);
        setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
        setTrait(UniversalGenes::REGENERATION_RATE, 0.5f);
    }

    /**
     * @brief Set optimal organism state for predictable modulation
     */
    void setOptimalState() {
        EnvironmentState env;
        env.temperature = 22.0f;
        env.humidity = 0.5f;
        env.time_of_day = 0.5f;
        
        OrganismState org;
        org.age_normalized = 0.5f;
        org.health = 1.0f;
        org.energy_level = 1.0f;
        org.is_sleeping = false;
        org.is_pregnant = false;
        
        phenotype_.updateContext(env, org);
    }

private:
    Genome genome_;
    mutable GeneRegistry registry_;
    mutable Phenotype phenotype_;

    void initializeRegistry() {
        registerGeneIfNeeded(UniversalGenes::HARDINESS, ChromosomeType::Morphology, 0.0f, 1.0f);
        registerGeneIfNeeded(UniversalGenes::TOXIN_TOLERANCE, ChromosomeType::Environmental, 0.0f, 1.0f);
        registerGeneIfNeeded(UniversalGenes::MAX_SIZE, ChromosomeType::Morphology, 0.0f, 10.0f);
        registerGeneIfNeeded(UniversalGenes::REGROWTH_RATE, ChromosomeType::Metabolism, 0.0f, 1.0f);
        registerGeneIfNeeded(UniversalGenes::REGENERATION_RATE, ChromosomeType::Metabolism, 0.0f, 1.0f);
    }

    void registerGeneIfNeeded(const std::string& name, ChromosomeType chrom, 
                               float minVal, float maxVal) {
        if (!registry_.hasGene(name)) {
            GeneLimits limits(minVal, maxVal, 0.05f);
            GeneDefinition def(name, chrom, limits, DominanceType::Incomplete);
            registry_.registerGene(def);
        }
    }
};

//================================================================================
//  Test: applyDamage Operations
//================================================================================

void test_applyDamage_reducesHealth() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.configureAsCreature();
    
    float health = 10.0f;  // Start at full health
    float damage = 3.0f;
    
    auto result = healthSystem.applyDamage(organism, health, damage, DamageType::Starvation);
    
    // Starvation has no resistance, so full damage applied
    TEST_ASSERT_NEAR(3.0f, result.actualDamage, 0.01f);
    TEST_ASSERT_NEAR(7.0f, health, 0.01f);
    TEST_ASSERT(!result.fatal);
}

void test_applyDamage_respectsResistance() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    
    // Set high hardiness for resistance
    organism.setTrait(UniversalGenes::HARDINESS, 0.8f);
    
    float health = 10.0f;
    float damage = 10.0f;
    
    auto result = healthSystem.applyDamage(organism, health, damage, DamageType::Physical);
    
    // Physical resistance = hardiness * 0.5 = 0.8 * 0.5 = 0.4
    // Actual damage = 10 * (1 - 0.4) = 6.0
    TEST_ASSERT_NEAR(0.4f, result.resistanceApplied, 0.05f);
    TEST_ASSERT_NEAR(6.0f, result.actualDamage, 0.5f);
    TEST_ASSERT_NEAR(4.0f, health, 0.5f);
    TEST_ASSERT(!result.fatal);
}

void test_applyDamage_floorAtZero() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.configureAsCreature();
    
    float health = 5.0f;
    float damage = 100.0f;  // Massive overkill damage
    
    auto result = healthSystem.applyDamage(organism, health, damage, DamageType::Starvation);
    
    // Health should floor at 0, not go negative
    TEST_ASSERT_NEAR(0.0f, health, 0.01f);
    TEST_ASSERT(result.fatal);
}

void test_applyDamage_zeroDamageNoOp() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.configureAsCreature();
    
    float health = 10.0f;
    
    auto result = healthSystem.applyDamage(organism, health, 0.0f, DamageType::Physical);
    
    // Zero damage should not change health
    TEST_ASSERT_NEAR(10.0f, health, 0.01f);
    TEST_ASSERT_NEAR(0.0f, result.actualDamage, 0.01f);
    TEST_ASSERT(!result.fatal);
}

void test_applyDamage_negativeDamageIgnored() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.configureAsCreature();
    
    float health = 10.0f;
    
    auto result = healthSystem.applyDamage(organism, health, -5.0f, DamageType::Physical);
    
    // Negative damage should be ignored
    TEST_ASSERT_NEAR(10.0f, health, 0.01f);
    TEST_ASSERT_NEAR(0.0f, result.actualDamage, 0.01f);
}

void test_applyDamage_toxinResistance() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    
    // Set high toxin tolerance
    organism.setTrait(UniversalGenes::TOXIN_TOLERANCE, 0.7f);
    
    float health = 10.0f;
    float damage = 10.0f;
    
    auto result = healthSystem.applyDamage(organism, health, damage, DamageType::Toxin);
    
    // Toxin resistance = toxin_tolerance = 0.7
    // Actual damage = 10 * (1 - 0.7) = 3.0
    TEST_ASSERT_NEAR(0.7f, result.resistanceApplied, 0.05f);
    TEST_ASSERT_NEAR(3.0f, result.actualDamage, 0.5f);
}

void test_applyDamage_starvationNoResistance() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    
    // Even with high hardiness, starvation has no resistance
    organism.setTrait(UniversalGenes::HARDINESS, 1.0f);
    
    float health = 10.0f;
    float damage = 5.0f;
    
    auto result = healthSystem.applyDamage(organism, health, damage, DamageType::Starvation);
    
    // Starvation ignores all resistance
    TEST_ASSERT_NEAR(0.0f, result.resistanceApplied, 0.01f);
    TEST_ASSERT_NEAR(5.0f, result.actualDamage, 0.01f);
}

//================================================================================
//  Test: Heal Operations
//================================================================================

void test_heal_increasesHealth() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
    
    float health = 5.0f;  // Injured
    float healAmount = 3.0f;
    
    auto result = healthSystem.heal(organism, health, healAmount);
    
    TEST_ASSERT_NEAR(3.0f, result.actualHealing, 0.01f);
    TEST_ASSERT_NEAR(8.0f, health, 0.01f);
    TEST_ASSERT_NEAR(8.0f, result.newHealth, 0.01f);
}

void test_heal_cappedAtMax() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
    
    float health = 8.0f;  // Nearly full
    float healAmount = 5.0f;  // More than needed
    
    auto result = healthSystem.heal(organism, health, healAmount);
    
    // Should cap at max health (10.0)
    TEST_ASSERT_NEAR(2.0f, result.actualHealing, 0.01f);  // Only healed 2
    TEST_ASSERT_NEAR(10.0f, health, 0.01f);
    TEST_ASSERT_NEAR(10.0f, result.newHealth, 0.01f);
}

void test_heal_zeroHealingNoOp() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.configureAsCreature();
    
    float health = 5.0f;
    
    auto result = healthSystem.heal(organism, health, 0.0f);
    
    TEST_ASSERT_NEAR(5.0f, health, 0.01f);
    TEST_ASSERT_NEAR(0.0f, result.actualHealing, 0.01f);
}

void test_heal_negativeHealingIgnored() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.configureAsCreature();
    
    float health = 5.0f;
    
    auto result = healthSystem.heal(organism, health, -3.0f);
    
    // Negative healing should be ignored
    TEST_ASSERT_NEAR(5.0f, health, 0.01f);
    TEST_ASSERT_NEAR(0.0f, result.actualHealing, 0.01f);
}

void test_heal_plantFixedMaxHealth() {
    HealthSystem healthSystem;
    HealthMockOrganism plant;
    plant.configureAsPlant();  // No MAX_SIZE trait = default max health (10.0)
    
    float health = 5.0f;
    float healAmount = 3.0f;
    
    auto result = healthSystem.heal(plant, health, healAmount);
    
    // Should heal normally using default max health
    TEST_ASSERT_NEAR(3.0f, result.actualHealing, 0.01f);
    TEST_ASSERT_NEAR(8.0f, health, 0.01f);
}

//================================================================================
//  Test: Wound State
//================================================================================

void test_getWoundState_correctThresholds() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
    
    // Healthy: > 75%
    TEST_ASSERT(healthSystem.getWoundState(organism, 8.0f) == WoundState::Healthy);
    TEST_ASSERT(healthSystem.getWoundState(organism, 10.0f) == WoundState::Healthy);
    
    // Injured: 50-75%
    TEST_ASSERT(healthSystem.getWoundState(organism, 7.0f) == WoundState::Injured);
    TEST_ASSERT(healthSystem.getWoundState(organism, 5.5f) == WoundState::Injured);
    
    // Wounded: 25-50%
    TEST_ASSERT(healthSystem.getWoundState(organism, 4.0f) == WoundState::Wounded);
    TEST_ASSERT(healthSystem.getWoundState(organism, 3.0f) == WoundState::Wounded);
    
    // Critical: > 0 and <= 25%
    TEST_ASSERT(healthSystem.getWoundState(organism, 2.0f) == WoundState::Critical);
    TEST_ASSERT(healthSystem.getWoundState(organism, 0.5f) == WoundState::Critical);
    
    // Dead: <= 0
    TEST_ASSERT(healthSystem.getWoundState(organism, 0.0f) == WoundState::Dead);
}

void test_getWoundSeverity_inverseOfHealthPercent() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
    
    // Full health = 0% severity
    float severity1 = healthSystem.getWoundSeverity(organism, 10.0f);
    TEST_ASSERT_NEAR(0.0f, severity1, 0.01f);
    
    // Half health = 50% severity
    float severity2 = healthSystem.getWoundSeverity(organism, 5.0f);
    TEST_ASSERT_NEAR(0.5f, severity2, 0.01f);
    
    // No health = 100% severity
    float severity3 = healthSystem.getWoundSeverity(organism, 0.0f);
    TEST_ASSERT_NEAR(1.0f, severity3, 0.01f);
}

void test_checkDeathCondition() {
    HealthSystem healthSystem;
    
    TEST_ASSERT(healthSystem.checkDeathCondition(0.0f));
    TEST_ASSERT(healthSystem.checkDeathCondition(-1.0f));  // Should be dead
    TEST_ASSERT(!healthSystem.checkDeathCondition(0.1f));
    TEST_ASSERT(!healthSystem.checkDeathCondition(10.0f));
}

//================================================================================
//  Test: Regeneration
//================================================================================

void test_canRegenerate_requiresEnergy() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.5f);  // Can regenerate
    
    float health = 5.0f;  // Injured, not at max
    
    // With energy surplus, can regenerate
    TEST_ASSERT(healthSystem.canRegenerate(organism, health, 10.0f));
    
    // With no energy surplus, cannot regenerate
    TEST_ASSERT(!healthSystem.canRegenerate(organism, health, 0.0f));
    
    // With negative energy, cannot regenerate
    TEST_ASSERT(!healthSystem.canRegenerate(organism, health, -5.0f));
}

void test_canRegenerate_requiresRegenerationAbility() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);
    
    float health = 5.0f;
    float energySurplus = 10.0f;
    
    // No regeneration traits set = 0.0 < 0.1 threshold
    TEST_ASSERT(!healthSystem.canRegenerate(organism, health, energySurplus));
    
    // Set low regrowth rate (below threshold)
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.05f);
    TEST_ASSERT(!healthSystem.canRegenerate(organism, health, energySurplus));
    
    // Set sufficient regrowth rate
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.2f);
    TEST_ASSERT(healthSystem.canRegenerate(organism, health, energySurplus));
}

void test_canRegenerate_notWhenDead() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.5f);
    
    // Dead organisms cannot regenerate
    float deadHealth = 0.0f;
    TEST_ASSERT(!healthSystem.canRegenerate(organism, deadHealth, 10.0f));
}

void test_canRegenerate_notWhenAtFullHealth() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max = 10.0
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.5f);
    
    // Already at full health
    float fullHealth = 10.0f;
    TEST_ASSERT(!healthSystem.canRegenerate(organism, fullHealth, 10.0f));
}

void test_processNaturalHealing_appliesHealingRate() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.5f);  // Healing rate = 0.5 * 0.01 = 0.005
    
    float health = 5.0f;
    float energySurplus = 10.0f;
    
    auto result = healthSystem.processNaturalHealing(organism, health, energySurplus);
    
    // Should apply healing based on regrowth rate
    // Expected: 0.5 * 0.01 = 0.005
    TEST_ASSERT_GT(result.actualHealing, 0.0f);
    TEST_ASSERT_GT(health, 5.0f);
}

void test_processNaturalHealing_noHealingWithoutEnergy() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.5f);
    
    float health = 5.0f;
    float energySurplus = 0.0f;  // No energy
    
    auto result = healthSystem.processNaturalHealing(organism, health, energySurplus);
    
    // No healing should occur
    TEST_ASSERT_NEAR(0.0f, result.actualHealing, 0.01f);
    TEST_ASSERT_NEAR(5.0f, health, 0.01f);
}

void test_getHealingRate_usesRegrowthRate() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::REGROWTH_RATE, 0.5f);
    
    float rate = healthSystem.getHealingRate(organism);
    
    // Rate = regrowth_rate * 0.01 = 0.5 * 0.01 = 0.005
    TEST_ASSERT_NEAR(0.005f, rate, 0.001f);
}

void test_getHealingRate_usesRegenerationRate() {
    HealthSystem healthSystem;
    HealthMockOrganism creature;
    creature.setTrait(UniversalGenes::REGENERATION_RATE, 0.8f);
    
    float rate = healthSystem.getHealingRate(creature);
    
    // Rate = regeneration_rate * 0.001 = 0.8 * 0.001 = 0.0008
    TEST_ASSERT_NEAR(0.0008f, rate, 0.0001f);
}

void test_getHealingRate_defaultWhenNoTrait() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    // No healing traits set
    
    float rate = healthSystem.getHealingRate(organism);
    
    // Should return default (0.005)
    TEST_ASSERT_NEAR(0.005f, rate, 0.001f);
}

//================================================================================
//  Test: Capacity Operations
//================================================================================

void test_getMaxHealth_fromMaxSizeTrait() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 2.0f);  // Max health = 2.0 * 10 = 20.0
    
    float maxHealth = healthSystem.getMaxHealth(organism);
    
    TEST_ASSERT_NEAR(20.0f, maxHealth, 0.1f);
}

void test_getMaxHealth_defaultWithoutTrait() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    // No MAX_SIZE trait
    
    float maxHealth = healthSystem.getMaxHealth(organism);
    
    // Should return default (10.0)
    TEST_ASSERT_NEAR(10.0f, maxHealth, 0.1f);
}

void test_getHealthPercent_correctRatio() {
    HealthSystem healthSystem;
    HealthMockOrganism organism;
    organism.setTrait(UniversalGenes::MAX_SIZE, 1.0f);  // Max health = 10.0
    
    // Test various health levels
    TEST_ASSERT_NEAR(1.0f, healthSystem.getHealthPercent(organism, 10.0f), 0.01f);
    TEST_ASSERT_NEAR(0.5f, healthSystem.getHealthPercent(organism, 5.0f), 0.01f);
    TEST_ASSERT_NEAR(0.0f, healthSystem.getHealthPercent(organism, 0.0f), 0.01f);
}

//================================================================================
//  Test: Organism-Agnostic Behavior
//================================================================================

void test_plant_and_creature_use_same_damage_system() {
    HealthSystem healthSystem;
    
    HealthMockOrganism plant;
    plant.configureAsPlant();
    plant.setTrait(UniversalGenes::HARDINESS, 0.6f);
    
    HealthMockOrganism creature;
    creature.configureAsCreature();
    creature.setTrait(UniversalGenes::HARDINESS, 0.6f);
    
    float plantHealth = 10.0f;
    float creatureHealth = 10.0f;
    
    // Same hardiness, same damage type = same resistance
    auto plantResult = healthSystem.applyDamage(plant, plantHealth, 10.0f, DamageType::Physical);
    auto creatureResult = healthSystem.applyDamage(creature, creatureHealth, 10.0f, DamageType::Physical);
    
    // Resistance should be the same
    TEST_ASSERT_NEAR(plantResult.resistanceApplied, creatureResult.resistanceApplied, 0.01f);
}

void test_traits_determine_all_calculations() {
    HealthSystem healthSystem;
    
    // Two organisms with identical traits should have identical behavior
    HealthMockOrganism org1;
    org1.setTrait(UniversalGenes::HARDINESS, 0.4f);
    org1.setTrait(UniversalGenes::TOXIN_TOLERANCE, 0.3f);
    org1.setTrait(UniversalGenes::MAX_SIZE, 1.5f);
    
    HealthMockOrganism org2;
    org2.setTrait(UniversalGenes::HARDINESS, 0.4f);
    org2.setTrait(UniversalGenes::TOXIN_TOLERANCE, 0.3f);
    org2.setTrait(UniversalGenes::MAX_SIZE, 1.5f);
    
    float health1 = 10.0f, health2 = 10.0f;
    
    auto result1 = healthSystem.applyDamage(org1, health1, 5.0f, DamageType::Physical);
    auto result2 = healthSystem.applyDamage(org2, health2, 5.0f, DamageType::Physical);
    
    TEST_ASSERT_NEAR(result1.actualDamage, result2.actualDamage, 0.001f);
    TEST_ASSERT_NEAR(result1.resistanceApplied, result2.resistanceApplied, 0.001f);
    TEST_ASSERT_NEAR(health1, health2, 0.001f);
}

//================================================================================
//  Main Entry Point
//================================================================================

void runHealthSystemTests() {
    BEGIN_TEST_GROUP("applyDamage Tests");
    RUN_TEST(test_applyDamage_reducesHealth);
    RUN_TEST(test_applyDamage_respectsResistance);
    RUN_TEST(test_applyDamage_floorAtZero);
    RUN_TEST(test_applyDamage_zeroDamageNoOp);
    RUN_TEST(test_applyDamage_negativeDamageIgnored);
    RUN_TEST(test_applyDamage_toxinResistance);
    RUN_TEST(test_applyDamage_starvationNoResistance);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Heal Tests");
    RUN_TEST(test_heal_increasesHealth);
    RUN_TEST(test_heal_cappedAtMax);
    RUN_TEST(test_heal_zeroHealingNoOp);
    RUN_TEST(test_heal_negativeHealingIgnored);
    RUN_TEST(test_heal_plantFixedMaxHealth);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Wound State Tests");
    RUN_TEST(test_getWoundState_correctThresholds);
    RUN_TEST(test_getWoundSeverity_inverseOfHealthPercent);
    RUN_TEST(test_checkDeathCondition);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Regeneration Tests");
    RUN_TEST(test_canRegenerate_requiresEnergy);
    RUN_TEST(test_canRegenerate_requiresRegenerationAbility);
    RUN_TEST(test_canRegenerate_notWhenDead);
    RUN_TEST(test_canRegenerate_notWhenAtFullHealth);
    RUN_TEST(test_processNaturalHealing_appliesHealingRate);
    RUN_TEST(test_processNaturalHealing_noHealingWithoutEnergy);
    RUN_TEST(test_getHealingRate_usesRegrowthRate);
    RUN_TEST(test_getHealingRate_usesRegenerationRate);
    RUN_TEST(test_getHealingRate_defaultWhenNoTrait);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Capacity Tests");
    RUN_TEST(test_getMaxHealth_fromMaxSizeTrait);
    RUN_TEST(test_getMaxHealth_defaultWithoutTrait);
    RUN_TEST(test_getHealthPercent_correctRatio);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Organism-Agnostic Tests");
    RUN_TEST(test_plant_and_creature_use_same_damage_system);
    RUN_TEST(test_traits_determine_all_calculations);
    END_TEST_GROUP();
}

// Standalone main for running just HealthSystem tests
#ifdef HEALTH_SYSTEM_TEST_MAIN
int main() {
    std::cout << "=== EcoSim HealthSystem Tests ===" << std::endl;
    
    runHealthSystemTests();
    
    TestSuite::instance().printSummary();
    
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
