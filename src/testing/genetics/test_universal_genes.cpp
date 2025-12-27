/**
 * @file test_universal_genes.cpp
 * @brief Tests for UniversalGenes 
 * 
 * Tests pleiotropy effects and emergent diet type calculation.
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
#include "genetics/defaults/UniversalGenes.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// UniversalGenes Registration Tests
// ============================================================================

void testUniversalGenesRegistration() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    size_t geneCount = registry.size();
    // Updated: 70 base genes + 19 combat genes (Phase 1c) = 89
    TEST_ASSERT_GE(geneCount, 85u);
    TEST_ASSERT_LE(geneCount, 100u);
}

void testUniversalGenesPresent() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Universal genes
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::LIFESPAN));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::MAX_SIZE));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::METABOLISM_RATE));
    
    // Mobility genes
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::LOCOMOTION));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::SIGHT_RANGE));
    
    // Autotrophy genes
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::PHOTOSYNTHESIS));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::ROOT_DEPTH));
    
    // Heterotrophy genes
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::HUNT_INSTINCT));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::DIGESTIVE_EFFICIENCY));
    
    // Phase 2.1 genes
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::GUT_LENGTH));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::TOXIN_PRODUCTION));
}

void testUniversalGenesCategories() {
    TEST_ASSERT_EQ(G::GeneCategory::Universal, G::UniversalGenes::getCategory(G::UniversalGenes::LIFESPAN));
    TEST_ASSERT_EQ(G::GeneCategory::Mobility, G::UniversalGenes::getCategory(G::UniversalGenes::LOCOMOTION));
    TEST_ASSERT_EQ(G::GeneCategory::Autotrophy, G::UniversalGenes::getCategory(G::UniversalGenes::PHOTOSYNTHESIS));
    TEST_ASSERT_EQ(G::GeneCategory::Heterotrophy, G::UniversalGenes::getCategory(G::UniversalGenes::HUNT_INSTINCT));
    TEST_ASSERT_EQ(G::GeneCategory::Morphology, G::UniversalGenes::getCategory(G::UniversalGenes::GUT_LENGTH));
    TEST_ASSERT_EQ(G::GeneCategory::PlantDefense, G::UniversalGenes::getCategory(G::UniversalGenes::TOXIN_PRODUCTION));
}

// ============================================================================
// Creature/Plant Genome Tests
// ============================================================================

void testCreatureGenomeCreation() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    TEST_ASSERT_EQ(registry.size(), creatureGenome.getTotalGeneCount());
    TEST_ASSERT(creatureGenome.hasGene(G::UniversalGenes::LOCOMOTION));
    TEST_ASSERT(creatureGenome.hasGene(G::UniversalGenes::PHOTOSYNTHESIS));
}

void testCreatureGenomeExpression() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Creatures should have high locomotion expression
    const G::Gene& locomotionGene = creatureGenome.getGene(G::UniversalGenes::LOCOMOTION);
    float locomotionExpr = locomotionGene.getAllele1().expression_strength;
    TEST_ASSERT_GE(locomotionExpr, 0.9f);
    
    // Creatures should have dormant photosynthesis
    const G::Gene& photoGene = creatureGenome.getGene(G::UniversalGenes::PHOTOSYNTHESIS);
    float photoExpr = photoGene.getAllele1().expression_strength;
    TEST_ASSERT_LE(photoExpr, 0.2f);
}

void testPlantGenomeCreation() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    TEST_ASSERT_EQ(registry.size(), plantGenome.getTotalGeneCount());
}

void testPlantGenomeExpression() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    
    // Plants should have high photosynthesis expression
    const G::Gene& photoGene = plantGenome.getGene(G::UniversalGenes::PHOTOSYNTHESIS);
    float photoExpr = photoGene.getAllele1().expression_strength;
    TEST_ASSERT_GE(photoExpr, 0.9f);
    
    // Plants should have dormant locomotion
    const G::Gene& locomotionGene = plantGenome.getGene(G::UniversalGenes::LOCOMOTION);
    float locomotionExpr = locomotionGene.getAllele1().expression_strength;
    TEST_ASSERT_LE(locomotionExpr, 0.2f);
}

void testCreaturePlantCrossover() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    
    G::Genome hybridGenome = G::Genome::crossover(creatureGenome, plantGenome);
    TEST_ASSERT_EQ(registry.size(), hybridGenome.getTotalGeneCount());
}

// ============================================================================
// Pleiotropy Tests
// ============================================================================

void testPleiotropyPlantMeatInhibition() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    const G::GeneDefinition& plantDigestDef = registry.getDefinition(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    const auto& effects = plantDigestDef.getEffects();
    
    bool hasInhibition = false;
    for (const auto& effect : effects) {
        if (effect.target_trait == "meat_digestion_efficiency" && effect.scale_factor < 0) {
            hasInhibition = true;
            break;
        }
    }
    TEST_ASSERT(hasInhibition);
}

void testPleiotropyMeatPlantInhibition() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    const G::GeneDefinition& meatDigestDef = registry.getDefinition(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    const auto& effects = meatDigestDef.getEffects();
    
    bool hasInhibition = false;
    for (const auto& effect : effects) {
        if (effect.target_trait == "plant_digestion_efficiency" && effect.scale_factor < 0) {
            hasInhibition = true;
            break;
        }
    }
    TEST_ASSERT(hasInhibition);
}

void testPleiotropyGutLength() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    const G::GeneDefinition& gutLengthDef = registry.getDefinition(G::UniversalGenes::GUT_LENGTH);
    const auto& effects = gutLengthDef.getEffects();
    
    bool hasSpeedEffect = false;
    for (const auto& effect : effects) {
        if (effect.target_trait == "speed_multiplier" && effect.scale_factor < 0) {
            hasSpeedEffect = true;
            break;
        }
    }
    TEST_ASSERT(hasSpeedEffect);
}

// ============================================================================
// Emergent Diet Type Tests
// ============================================================================

void testEmergentDietOmnivore() {
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
    
    G::DietType diet = phenotype.calculateDietType();
    TEST_ASSERT_EQ(G::DietType::OMNIVORE, diet);
}

void testEmergentDietCarnivore() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome carnivoreGenome;
    
    G::Allele meatHigh(0.9f, 1.0f);
    G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatHigh, meatHigh);
    carnivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
    
    G::Allele plantLow(0.1f, 1.0f);
    G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantLow, plantLow);
    carnivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
    
    G::Allele celluloseLow(0.1f, 1.0f);
    G::Gene cellulose(G::UniversalGenes::CELLULOSE_BREAKDOWN, celluloseLow, celluloseLow);
    carnivoreGenome.addGene(cellulose, G::ChromosomeType::Metabolism);
    
    G::Allele colorLow(0.3f, 1.0f);
    G::Gene colorVision(G::UniversalGenes::COLOR_VISION, colorLow, colorLow);
    carnivoreGenome.addGene(colorVision, G::ChromosomeType::Sensory);
    
    G::Phenotype phenotype(&carnivoreGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    G::DietType diet = phenotype.calculateDietType();
    TEST_ASSERT_EQ(G::DietType::CARNIVORE, diet);
}

void testEmergentDietHerbivore() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome herbivoreGenome;
    
    G::Allele plantHigh(0.9f, 1.0f);
    G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantHigh, plantHigh);
    herbivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
    
    G::Allele meatLow(0.2f, 1.0f);
    G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatLow, meatLow);
    herbivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
    
    G::Allele celluloseHigh(0.8f, 1.0f);
    G::Gene cellulose(G::UniversalGenes::CELLULOSE_BREAKDOWN, celluloseHigh, celluloseHigh);
    herbivoreGenome.addGene(cellulose, G::ChromosomeType::Metabolism);
    
    G::Allele colorMed(0.4f, 1.0f);
    G::Gene colorVision(G::UniversalGenes::COLOR_VISION, colorMed, colorMed);
    herbivoreGenome.addGene(colorVision, G::ChromosomeType::Sensory);
    
    G::Phenotype phenotype(&herbivoreGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    G::DietType diet = phenotype.calculateDietType();
    TEST_ASSERT_EQ(G::DietType::HERBIVORE, diet);
}

void testEmergentDietFrugivore() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome frugivoreGenome;
    
    G::Allele plantMed(0.6f, 1.0f);
    G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantMed, plantMed);
    frugivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
    
    G::Allele meatLow(0.3f, 1.0f);
    G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatLow, meatLow);
    frugivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
    
    G::Allele celluloseLow(0.2f, 1.0f);
    G::Gene cellulose(G::UniversalGenes::CELLULOSE_BREAKDOWN, celluloseLow, celluloseLow);
    frugivoreGenome.addGene(cellulose, G::ChromosomeType::Metabolism);
    
    G::Allele colorHigh(0.8f, 1.0f);
    G::Gene colorVision(G::UniversalGenes::COLOR_VISION, colorHigh, colorHigh);
    frugivoreGenome.addGene(colorVision, G::ChromosomeType::Sensory);
    
    G::Phenotype phenotype(&frugivoreGenome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    phenotype.updateContext(env, org);
    
    G::DietType diet = phenotype.calculateDietType();
    TEST_ASSERT_EQ(G::DietType::FRUGIVORE, diet);
}

// ============================================================================
// Test Runner
// ============================================================================

void runUniversalGenesTests() {
    BEGIN_TEST_GROUP("UniversalGenes Registration Tests");
    RUN_TEST(testUniversalGenesRegistration);
    RUN_TEST(testUniversalGenesPresent);
    RUN_TEST(testUniversalGenesCategories);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Creature/Plant Genome Tests");
    RUN_TEST(testCreatureGenomeCreation);
    RUN_TEST(testCreatureGenomeExpression);
    RUN_TEST(testPlantGenomeCreation);
    RUN_TEST(testPlantGenomeExpression);
    RUN_TEST(testCreaturePlantCrossover);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Pleiotropy Tests (Phase 2.1)");
    RUN_TEST(testPleiotropyPlantMeatInhibition);
    RUN_TEST(testPleiotropyMeatPlantInhibition);
    RUN_TEST(testPleiotropyGutLength);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Emergent Diet Type Tests (Phase 2.1)");
    RUN_TEST(testEmergentDietOmnivore);
    RUN_TEST(testEmergentDietCarnivore);
    RUN_TEST(testEmergentDietHerbivore);
    RUN_TEST(testEmergentDietFrugivore);
    END_TEST_GROUP();
}

#ifdef TEST_UNIVERSAL_GENES_STANDALONE
int main() {
    std::cout << "=== Universal Genes Tests ===" << std::endl;
    runUniversalGenesTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
