/**
 * @file test_energy_budget.cpp
 * @brief Tests for Energy Budget System (Phase 2.2)
 * 
 * Tests maintenance costs, specialist bonuses, and metabolic overhead.
 */

#include <iostream>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/expression/EnergyBudget.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Maintenance Cost Tests
// ============================================================================

void testMaintenanceCostCreature() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype phenotype(&creatureGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    float maintenanceCost = phenotype.getTotalMaintenanceCost();
    TEST_ASSERT_GT(maintenanceCost, 0.0f);
    TEST_ASSERT_GT(maintenanceCost, 1.0f);
}

void testMaintenanceCostPlant() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    G::Phenotype phenotype(&plantGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    float maintenanceCost = phenotype.getTotalMaintenanceCost();
    TEST_ASSERT_GT(maintenanceCost, 0.0f);
}

void testMaintenanceCostScaling() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    const G::GeneDefinition& locomotionDef = registry.getDefinition(G::UniversalGenes::LOCOMOTION);
    
    float fullCost = locomotionDef.calculateMaintenanceCost(1.0f);
    float halfCost = locomotionDef.calculateMaintenanceCost(0.5f);
    float zeroCost = locomotionDef.calculateMaintenanceCost(0.0f);
    
    TEST_ASSERT_GT(fullCost, 0.0f);
    TEST_ASSERT_LT(halfCost, fullCost);
    TEST_ASSERT_GT(halfCost, 0.0f);
    TEST_ASSERT_NEAR(0.0f, zeroCost, 0.001f);
}

// ============================================================================
// Specialist Bonus Tests
// ============================================================================

void testSpecialistBonusOmnivore() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome omnivoreGenome;
    
    G::Allele plantMed(0.5f, 1.0f);
    G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantMed, plantMed);
    omnivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
    
    G::Allele meatMed(0.5f, 1.0f);
    G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatMed, meatMed);
    omnivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
    
    G::Phenotype phenotype(&omnivoreGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    float bonus = phenotype.getSpecialistBonus();
    TEST_ASSERT_GE(bonus, 1.0f);
    TEST_ASSERT_LE(bonus, 1.05f);
}

void testSpecialistBonusCarnivore() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome carnivoreGenome;
    
    G::Allele plantLow(0.1f, 1.0f);
    G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantLow, plantLow);
    carnivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
    
    G::Allele meatHigh(0.9f, 1.0f);
    G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatHigh, meatHigh);
    carnivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
    
    G::Phenotype phenotype(&carnivoreGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    float bonus = phenotype.getSpecialistBonus();
    TEST_ASSERT_GT(bonus, 1.2f);
    TEST_ASSERT_LE(bonus, 1.3f);
}

// ============================================================================
// EnergyBudget Update Tests
// ============================================================================

void testEnergyBudgetGain() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype phenotype(&creatureGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    const G::EnergyBudget& energyBudget = phenotype.getEnergyBudget();
    
    G::EnergyState state;
    state.currentEnergy = 50.0f;
    state.maxEnergy = 100.0f;
    state.baseMetabolism = 1.0f;
    state.maintenanceCost = 2.0f;
    state.activityCost = 0.0f;
    
    float income = 5.0f;
    G::EnergyState newState = energyBudget.updateEnergy(state, income, 0.0f);
    
    TEST_ASSERT_GT(newState.currentEnergy, 50.0f);
}

void testEnergyBudgetStarvation() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype phenotype(&creatureGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    const G::EnergyBudget& energyBudget = phenotype.getEnergyBudget();
    
    G::EnergyState state;
    state.currentEnergy = 5.0f;
    state.maxEnergy = 100.0f;
    
    TEST_ASSERT(energyBudget.isStarving(state));
    
    state.currentEnergy = 15.0f;
    TEST_ASSERT(!energyBudget.isStarving(state));
}

// ============================================================================
// Test Runner
// ============================================================================

void runEnergyBudgetTests() {
    BEGIN_TEST_GROUP("Maintenance Cost Tests");
    RUN_TEST(testMaintenanceCostCreature);
    RUN_TEST(testMaintenanceCostPlant);
    RUN_TEST(testMaintenanceCostScaling);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Specialist Bonus Tests");
    RUN_TEST(testSpecialistBonusOmnivore);
    RUN_TEST(testSpecialistBonusCarnivore);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("EnergyBudget Update Tests");
    RUN_TEST(testEnergyBudgetGain);
    RUN_TEST(testEnergyBudgetStarvation);
    END_TEST_GROUP();
}

#ifdef TEST_ENERGY_BUDGET_STANDALONE
int main() {
    std::cout << "=== Energy Budget Tests (Phase 2.2) ===" << std::endl;
    runEnergyBudgetTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
