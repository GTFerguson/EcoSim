/**
 * @file test_expression.cpp
 * @brief Tests for expression system: Phenotype, EnvironmentState, OrganismState
 * 
 * Phase 1 tests for phenotype expression from genotype.
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
#include "genetics/defaults/DefaultGenes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// EnvironmentState Tests
// ============================================================================

void testEnvironmentStateDefaults() {
    G::EnvironmentState env;
    // Default values should be reasonable
    TEST_ASSERT_GE(env.temperature, -50.0f);
    TEST_ASSERT_LE(env.temperature, 50.0f);
}

void testEnvironmentStateCustom() {
    G::EnvironmentState env;
    env.temperature = 25.0f;
    env.time_of_day = 0.8f;
    env.humidity = 0.6f;
    
    TEST_ASSERT_NEAR(25.0f, env.temperature, 0.01f);
    TEST_ASSERT_NEAR(0.8f, env.time_of_day, 0.01f);
}

// ============================================================================
// OrganismState Tests
// ============================================================================

void testOrganismStateDefaults() {
    G::OrganismState org;
    // Check reasonable defaults
    TEST_ASSERT_GE(org.energy_level, 0.0f);
    TEST_ASSERT_LE(org.energy_level, 1.0f);
}

void testOrganismStateCustom() {
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 0.75f;
    org.health = 0.9f;
    
    TEST_ASSERT_NEAR(0.5f, org.age_normalized, 0.01f);
    TEST_ASSERT_NEAR(0.75f, org.energy_level, 0.01f);
}

// ============================================================================
// Phenotype Tests
// ============================================================================

void testPhenotypeCreation() {
    G::GeneRegistry registry;
    G::DefaultGenes::registerDefaults(registry);
    
    G::Genome genome = G::DefaultGenes::createDefaultGenome(registry);
    G::Phenotype phenotype(&genome, &registry);
    
    // Should not throw during creation
    TEST_ASSERT(true);
}

void testPhenotypeContextUpdate() {
    G::GeneRegistry registry;
    G::DefaultGenes::registerDefaults(registry);
    
    G::Genome genome = G::DefaultGenes::createDefaultGenome(registry);
    G::Phenotype phenotype(&genome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    
    phenotype.updateContext(env, org);
    
    // Context should be stored
    TEST_ASSERT(true);
}

void testPhenotypeGetTrait() {
    G::GeneRegistry registry;
    G::DefaultGenes::registerDefaults(registry);
    
    G::Genome genome = G::DefaultGenes::createDefaultGenome(registry);
    G::Phenotype phenotype(&genome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    
    phenotype.updateContext(env, org);
    
    float lifespan = phenotype.getTrait(G::DefaultGenes::LIFESPAN);
    TEST_ASSERT_GT(lifespan, 0.0f);
}

void testPhenotypeTraitWithUniversalGenes() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    G::Phenotype phenotype(&genome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    // Test various traits
    float lifespan = phenotype.getTrait(G::UniversalGenes::LIFESPAN);
    float locomotion = phenotype.getTrait(G::UniversalGenes::LOCOMOTION);
    float photosynthesis = phenotype.getTrait(G::UniversalGenes::PHOTOSYNTHESIS);
    
    TEST_ASSERT_GT(lifespan, 0.0f);
    TEST_ASSERT_GT(locomotion, 0.0f);  // High for creatures
    // Photosynthesis may be low for creatures (dormant)
}

void testPhenotypeHasTrait() {
    G::GeneRegistry registry;
    G::DefaultGenes::registerDefaults(registry);
    
    G::Genome genome = G::DefaultGenes::createDefaultGenome(registry);
    G::Phenotype phenotype(&genome, &registry);
    
    TEST_ASSERT(phenotype.hasTrait(G::DefaultGenes::LIFESPAN));
    TEST_ASSERT(!phenotype.hasTrait("nonexistent_trait"));
}

// ============================================================================
// Test Runner
// ============================================================================

void runExpressionTests() {
    BEGIN_TEST_GROUP("EnvironmentState Tests");
    RUN_TEST(testEnvironmentStateDefaults);
    RUN_TEST(testEnvironmentStateCustom);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("OrganismState Tests");
    RUN_TEST(testOrganismStateDefaults);
    RUN_TEST(testOrganismStateCustom);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Phenotype Tests");
    RUN_TEST(testPhenotypeCreation);
    RUN_TEST(testPhenotypeContextUpdate);
    RUN_TEST(testPhenotypeGetTrait);
    RUN_TEST(testPhenotypeTraitWithUniversalGenes);
    RUN_TEST(testPhenotypeHasTrait);
    END_TEST_GROUP();
}

#ifdef TEST_EXPRESSION_STANDALONE
int main() {
    std::cout << "=== Expression System Tests ===" << std::endl;
    runExpressionTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
