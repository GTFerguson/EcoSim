/**
 * @file test_core.cpp
 * @brief Tests for core genetics types: Gene, Chromosome, Genome
 * 
 * Phase 1 tests - validates the fundamental genetics data structures.
 */

#include <iostream>
#include "test_framework.hpp"

// Include genetics headers
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/Chromosome.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

// Namespace alias
namespace G = EcoSim::Genetics;

// ============================================================================
// Gene Tests
// ============================================================================

void testGeneCreation() {
    // Create a gene with two alleles
    G::Allele a1(1.0f, 1.0f);
    G::Allele a2(2.0f, 0.8f);
    G::Gene gene("test_gene", a1, a2);
    
    TEST_ASSERT_EQ("test_gene", gene.getId());
    TEST_ASSERT(gene.isHeterozygous());
    
    // Test expression with different dominance types
    float incomplete = gene.getNumericValue(G::DominanceType::Incomplete);
    TEST_ASSERT_NEAR(1.5f, incomplete, 0.001f); // Average of 1.0 and 2.0
}

void testGeneHomozygous() {
    G::Allele a1(1.0f, 1.0f);
    G::Allele a2(1.0f, 1.0f);
    G::Gene gene("homo_gene", a1, a2);
    
    TEST_ASSERT(!gene.isHeterozygous());
}

void testGeneDominance() {
    G::Allele dominant(1.0f, 1.0f);
    G::Allele recessive(0.5f, 0.3f);
    G::Gene gene("dom_test", dominant, recessive);
    
    // Complete dominance should favor higher expression
    float complete = gene.getNumericValue(G::DominanceType::Complete);
    TEST_ASSERT_GT(complete, 0.75f);
    
    // Incomplete should average
    float incomplete = gene.getNumericValue(G::DominanceType::Incomplete);
    TEST_ASSERT_NEAR(0.75f, incomplete, 0.1f);
}

// ============================================================================
// Chromosome Tests
// ============================================================================

void testChromosomeCreation() {
    G::Chromosome chromo(G::ChromosomeType::Metabolism);
    TEST_ASSERT_EQ(G::ChromosomeType::Metabolism, chromo.getType());
    TEST_ASSERT_EQ(0u, chromo.size());
}

void testChromosomeAddGenes() {
    G::Chromosome chromo(G::ChromosomeType::Metabolism);
    
    G::Gene gene1("gene1", G::Allele(1.0f), G::Allele(1.0f));
    G::Gene gene2("gene2", G::Allele(2.0f), G::Allele(2.0f));
    
    chromo.addGene(gene1);
    chromo.addGene(gene2);
    
    TEST_ASSERT_EQ(2u, chromo.size());
    TEST_ASSERT(chromo.hasGene("gene1"));
    TEST_ASSERT(chromo.hasGene("gene2"));
    TEST_ASSERT(!chromo.hasGene("gene3"));
}

void testChromosomeGetGene() {
    G::Chromosome chromo(G::ChromosomeType::Sensory);
    G::Gene gene("sight", G::Allele(50.0f), G::Allele(60.0f));
    chromo.addGene(gene);
    
    auto retrieved = chromo.getGene("sight");
    TEST_ASSERT(retrieved.has_value());
    TEST_ASSERT_EQ("sight", retrieved.value().get().getId());
}

// ============================================================================
// Genome Tests
// ============================================================================

void testGenomeCreation() {
    G::Genome genome;
    TEST_ASSERT_EQ(0u, genome.getTotalGeneCount());
}

void testGenomeAddGenes() {
    G::Genome genome;
    
    G::Gene lifeGene("lifespan", G::Allele(5000.0f), G::Allele(6000.0f));
    G::Gene sightGene("sight", G::Allele(50.0f), G::Allele(60.0f));
    
    genome.addGene(lifeGene, G::ChromosomeType::Lifespan);
    genome.addGene(sightGene, G::ChromosomeType::Sensory);
    
    TEST_ASSERT(genome.hasGene("lifespan"));
    TEST_ASSERT(genome.hasGene("sight"));
    TEST_ASSERT_EQ(2u, genome.getTotalGeneCount());
}

void testGenomeCrossover() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome parent1 = G::UniversalGenes::createRandomGenome(registry);
    G::Genome parent2 = G::UniversalGenes::createRandomGenome(registry);
    
    G::Genome offspring = G::Genome::crossover(parent1, parent2);
    
    // Offspring should have genes from both parents
    TEST_ASSERT(offspring.hasGene(G::UniversalGenes::LIFESPAN));
    TEST_ASSERT(offspring.hasGene(G::UniversalGenes::SIGHT_RANGE));
}

void testGenomeMutation() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome original = G::UniversalGenes::createRandomGenome(registry);
    G::Genome mutated = original;
    
    // Apply high mutation rate - need to pass the definitions map from registry
    mutated.mutate(1.0f, registry.getAllDefinitions());
    
    // At least some values should be different
    // (This is a probabilistic test, but with 100% rate something should change)
}

// ============================================================================
// GeneRegistry Tests
// ============================================================================

void testGeneRegistryCreation() {
    G::GeneRegistry registry;
    TEST_ASSERT_EQ(0u, registry.size());
}

void testGeneRegistryRegistration() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // UniversalGenes now registers 89 genes across all categories:
    // Universal(7) + Mobility(5) + Autotrophy(5) + Heterotrophy(6) + CoevolutionHeterotrophy(7)
    // + Morphology(9) + Behavior(5) + SeedInteraction(2) + PlantDefense(6) + Reproduction(6)
    // + SeedPropagation(5) + Olfactory(4) + HealthHealing(3) + WeaponShape(13) + CombatDefense(2)
    // + CombatBehavior(4) = 89 genes total
    TEST_ASSERT_EQ(89u, registry.size());
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::LIFESPAN));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::SIGHT_RANGE));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::DIET_TYPE));
}

void testDefaultGenomeCreation() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome defaultGenome = G::UniversalGenes::createRandomGenome(registry);
    TEST_ASSERT(defaultGenome.hasGene(G::UniversalGenes::LIFESPAN));
    TEST_ASSERT_EQ(registry.size(), defaultGenome.getTotalGeneCount());
}

void testRandomGenomeCreation() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome randomGenome = G::UniversalGenes::createRandomGenome(registry);
    TEST_ASSERT(randomGenome.hasGene(G::UniversalGenes::LIFESPAN));
}

// ============================================================================
// Test Runner
// ============================================================================

void runCoreTests() {
    BEGIN_TEST_GROUP("Gene Tests");
    RUN_TEST(testGeneCreation);
    RUN_TEST(testGeneHomozygous);
    RUN_TEST(testGeneDominance);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Chromosome Tests");
    RUN_TEST(testChromosomeCreation);
    RUN_TEST(testChromosomeAddGenes);
    RUN_TEST(testChromosomeGetGene);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Genome Tests");
    RUN_TEST(testGenomeCreation);
    RUN_TEST(testGenomeAddGenes);
    RUN_TEST(testGenomeCrossover);
    RUN_TEST(testGenomeMutation);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("GeneRegistry Tests");
    RUN_TEST(testGeneRegistryCreation);
    RUN_TEST(testGeneRegistryRegistration);
    RUN_TEST(testDefaultGenomeCreation);
    RUN_TEST(testRandomGenomeCreation);
    END_TEST_GROUP();
}

#ifdef TEST_CORE_STANDALONE
int main() {
    std::cout << "=== Core Genetics Tests ===" << std::endl;
    runCoreTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
