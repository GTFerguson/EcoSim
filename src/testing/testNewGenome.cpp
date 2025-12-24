#include <iostream>
#include <cassert>
#include <cmath>

// Include the new genetics system headers directly
// Note: We don't include LegacyGenomeAdapter.hpp to avoid Genome name conflicts
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/Chromosome.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/expression/EnergyBudget.hpp"
#include "genetics/defaults/DefaultGenes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/interactions/CoevolutionTracker.hpp"

// Namespace alias to avoid Genome name collision with legacy class
namespace G = EcoSim::Genetics;

void testGeneCreation() {
    std::cout << "Testing Gene creation..." << std::endl;
    
    // Create a gene with two alleles
    G::Allele a1(1.0f, 1.0f);
    G::Allele a2(2.0f, 0.8f);
    G::Gene gene("test_gene", a1, a2);
    
    assert(gene.getId() == "test_gene");
    assert(gene.isHeterozygous());
    
    // Test expression with different dominance types
    float incomplete = gene.getNumericValue(G::DominanceType::Incomplete);
    assert(incomplete == 1.5f); // Average of 1.0 and 2.0
    
    std::cout << "  Gene creation: PASSED" << std::endl;
}

void testChromosome() {
    std::cout << "Testing Chromosome..." << std::endl;
    
    G::Chromosome chromo(G::ChromosomeType::Metabolism);
    assert(chromo.getType() == G::ChromosomeType::Metabolism);
    assert(chromo.size() == 0);
    
    G::Gene gene1("gene1", G::Allele(1.0f), G::Allele(1.0f));
    G::Gene gene2("gene2", G::Allele(2.0f), G::Allele(2.0f));
    
    chromo.addGene(gene1);
    chromo.addGene(gene2);
    
    assert(chromo.size() == 2);
    assert(chromo.hasGene("gene1"));
    assert(chromo.hasGene("gene2"));
    assert(!chromo.hasGene("gene3"));
    
    std::cout << "  Chromosome: PASSED" << std::endl;
}

void testGenome() {
    std::cout << "Testing Genome..." << std::endl;
    
    G::Genome genome;
    
    // Add genes to different chromosomes
    G::Gene lifeGene("lifespan", G::Allele(5000.0f), G::Allele(6000.0f));
    G::Gene sightGene("sight", G::Allele(50.0f), G::Allele(60.0f));
    
    genome.addGene(lifeGene, G::ChromosomeType::Lifespan);
    genome.addGene(sightGene, G::ChromosomeType::Sensory);
    
    assert(genome.hasGene("lifespan"));
    assert(genome.hasGene("sight"));
    assert(genome.getTotalGeneCount() == 2);
    
    std::cout << "  Genome: PASSED" << std::endl;
}

void testDefaultGenes() {
    std::cout << "Testing DefaultGenes..." << std::endl;
    
    G::GeneRegistry registry;
    G::DefaultGenes::registerDefaults(registry);
    
    assert(registry.size() == 12); // 12 legacy genes
    assert(registry.hasGene(G::DefaultGenes::LIFESPAN));
    assert(registry.hasGene(G::DefaultGenes::SIGHT));
    assert(registry.hasGene(G::DefaultGenes::DIET));
    
    // Create random genome
    G::Genome randomGenome = G::DefaultGenes::createRandomGenome(registry);
    assert(randomGenome.hasGene(G::DefaultGenes::LIFESPAN));
    
    std::cout << "  DefaultGenes: PASSED" << std::endl;
}

void testGenomeCrossover() {
    std::cout << "Testing Genome crossover..." << std::endl;
    
    G::GeneRegistry registry;
    G::DefaultGenes::registerDefaults(registry);
    
    G::Genome parent1 = G::DefaultGenes::createRandomGenome(registry);
    G::Genome parent2 = G::DefaultGenes::createRandomGenome(registry);
    
    G::Genome offspring = G::Genome::crossover(parent1, parent2);
    
    // Offspring should have genes from both parents
    assert(offspring.hasGene(G::DefaultGenes::LIFESPAN));
    assert(offspring.hasGene(G::DefaultGenes::SIGHT));
    
    std::cout << "  Genome crossover: PASSED" << std::endl;
}

void testPhenotype() {
    std::cout << "Testing Phenotype..." << std::endl;
    
    G::GeneRegistry registry;
    G::DefaultGenes::registerDefaults(registry);
    
    G::Genome genome = G::DefaultGenes::createDefaultGenome(registry);
    G::Phenotype phenotype(&genome, &registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f; // Adult
    org.energy_level = 1.0f;
    
    phenotype.updateContext(env, org);
    
    // Get a trait value
    float lifespan = phenotype.getTrait(G::DefaultGenes::LIFESPAN);
    assert(lifespan > 0.0f);
    
    std::cout << "  Phenotype: PASSED" << std::endl;
    std::cout << "  Lifespan trait value: " << lifespan << std::endl;
}

// ============================================================================
// UNIVERSAL GENES TESTS
// ============================================================================

void testUniversalGenesRegistration() {
    std::cout << "Testing UniversalGenes registration..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Phase 2.1: Should have ~58 genes total (29 original + 29 new)
    size_t geneCount = registry.size();
    std::cout << "  Total genes registered: " << geneCount << std::endl;
    assert(geneCount >= 55 && geneCount <= 70); // Allow some flexibility
    
    // Test Universal genes exist
    assert(registry.hasGene(G::UniversalGenes::LIFESPAN));
    assert(registry.hasGene(G::UniversalGenes::MAX_SIZE));
    assert(registry.hasGene(G::UniversalGenes::METABOLISM_RATE));
    assert(registry.hasGene(G::UniversalGenes::COLOR_HUE));
    assert(registry.hasGene(G::UniversalGenes::HARDINESS));
    assert(registry.hasGene(G::UniversalGenes::TEMP_TOLERANCE_LOW));
    assert(registry.hasGene(G::UniversalGenes::TEMP_TOLERANCE_HIGH));
    
    // Test Mobility genes exist
    assert(registry.hasGene(G::UniversalGenes::LOCOMOTION));
    assert(registry.hasGene(G::UniversalGenes::SIGHT_RANGE));
    assert(registry.hasGene(G::UniversalGenes::NAVIGATION_ABILITY));
    assert(registry.hasGene(G::UniversalGenes::FLEE_THRESHOLD));
    assert(registry.hasGene(G::UniversalGenes::PURSUE_THRESHOLD));
    
    // Test Autotrophy genes exist
    assert(registry.hasGene(G::UniversalGenes::PHOTOSYNTHESIS));
    assert(registry.hasGene(G::UniversalGenes::ROOT_DEPTH));
    assert(registry.hasGene(G::UniversalGenes::WATER_STORAGE));
    assert(registry.hasGene(G::UniversalGenes::LIGHT_REQUIREMENT));
    assert(registry.hasGene(G::UniversalGenes::WATER_REQUIREMENT));
    
    // Test Heterotrophy genes exist (original)
    assert(registry.hasGene(G::UniversalGenes::HUNT_INSTINCT));
    assert(registry.hasGene(G::UniversalGenes::HUNGER_THRESHOLD));
    assert(registry.hasGene(G::UniversalGenes::THIRST_THRESHOLD));
    assert(registry.hasGene(G::UniversalGenes::DIGESTIVE_EFFICIENCY));
    assert(registry.hasGene(G::UniversalGenes::NUTRIENT_VALUE));
    
    // Test NEW Heterotrophy genes for coevolution (Phase 2.1)
    assert(registry.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY));
    assert(registry.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY));
    assert(registry.hasGene(G::UniversalGenes::CELLULOSE_BREAKDOWN));
    assert(registry.hasGene(G::UniversalGenes::TOXIN_TOLERANCE));
    assert(registry.hasGene(G::UniversalGenes::TOXIN_METABOLISM));
    assert(registry.hasGene(G::UniversalGenes::SCENT_DETECTION));
    assert(registry.hasGene(G::UniversalGenes::COLOR_VISION));
    
    // Test NEW Morphology genes (Phase 2.1)
    assert(registry.hasGene(G::UniversalGenes::GUT_LENGTH));
    assert(registry.hasGene(G::UniversalGenes::TOOTH_SHARPNESS));
    assert(registry.hasGene(G::UniversalGenes::TOOTH_GRINDING));
    assert(registry.hasGene(G::UniversalGenes::STOMACH_ACIDITY));
    assert(registry.hasGene(G::UniversalGenes::JAW_STRENGTH));
    assert(registry.hasGene(G::UniversalGenes::JAW_SPEED));
    assert(registry.hasGene(G::UniversalGenes::HIDE_THICKNESS));
    assert(registry.hasGene(G::UniversalGenes::FUR_DENSITY));
    assert(registry.hasGene(G::UniversalGenes::MUCUS_PROTECTION));
    
    // Test NEW Behavior genes (Phase 2.1)
    assert(registry.hasGene(G::UniversalGenes::SWEETNESS_PREFERENCE));
    assert(registry.hasGene(G::UniversalGenes::CACHING_INSTINCT));
    assert(registry.hasGene(G::UniversalGenes::SPATIAL_MEMORY));
    assert(registry.hasGene(G::UniversalGenes::GROOMING_FREQUENCY));
    assert(registry.hasGene(G::UniversalGenes::PAIN_SENSITIVITY));
    
    // Test NEW Seed Interaction genes (Phase 2.1)
    assert(registry.hasGene(G::UniversalGenes::GUT_TRANSIT_TIME));
    assert(registry.hasGene(G::UniversalGenes::SEED_DESTRUCTION_RATE));
    
    // Test NEW Plant Defense genes (Phase 2.1)
    assert(registry.hasGene(G::UniversalGenes::TOXIN_PRODUCTION));
    assert(registry.hasGene(G::UniversalGenes::THORN_DENSITY));
    assert(registry.hasGene(G::UniversalGenes::REGROWTH_RATE));
    assert(registry.hasGene(G::UniversalGenes::FRUIT_PRODUCTION_RATE));
    assert(registry.hasGene(G::UniversalGenes::SEED_COAT_DURABILITY));
    assert(registry.hasGene(G::UniversalGenes::FRUIT_APPEAL));
    
    // Test Reproduction genes exist
    assert(registry.hasGene(G::UniversalGenes::OFFSPRING_COUNT));
    assert(registry.hasGene(G::UniversalGenes::MATE_THRESHOLD));
    assert(registry.hasGene(G::UniversalGenes::SPREAD_DISTANCE));
    assert(registry.hasGene(G::UniversalGenes::FATIGUE_THRESHOLD));
    assert(registry.hasGene(G::UniversalGenes::COMFORT_INCREASE));
    assert(registry.hasGene(G::UniversalGenes::COMFORT_DECREASE));
    
    std::cout << "  UniversalGenes registration: PASSED" << std::endl;
}

void testUniversalGenesCategories() {
    std::cout << "Testing UniversalGenes categories..." << std::endl;
    
    // Test category lookup
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::LIFESPAN) == G::GeneCategory::Universal);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::MAX_SIZE) == G::GeneCategory::Universal);
    
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::LOCOMOTION) == G::GeneCategory::Mobility);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::SIGHT_RANGE) == G::GeneCategory::Mobility);
    
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::PHOTOSYNTHESIS) == G::GeneCategory::Autotrophy);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::ROOT_DEPTH) == G::GeneCategory::Autotrophy);
    
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::HUNT_INSTINCT) == G::GeneCategory::Heterotrophy);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::HUNGER_THRESHOLD) == G::GeneCategory::Heterotrophy);
    
    // Test new gene categories (Phase 2.1)
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY) == G::GeneCategory::Heterotrophy);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY) == G::GeneCategory::Heterotrophy);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::GUT_LENGTH) == G::GeneCategory::Morphology);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::TOOTH_SHARPNESS) == G::GeneCategory::Morphology);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::SWEETNESS_PREFERENCE) == G::GeneCategory::Behavior);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::TOXIN_PRODUCTION) == G::GeneCategory::PlantDefense);
    
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::OFFSPRING_COUNT) == G::GeneCategory::Reproduction);
    assert(G::UniversalGenes::getCategory(G::UniversalGenes::MATE_THRESHOLD) == G::GeneCategory::Reproduction);
    
    std::cout << "  UniversalGenes categories: PASSED" << std::endl;
}

void testCreatureGenome() {
    std::cout << "Testing UniversalGenes::createCreatureGenome()..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Should have all genes
    assert(creatureGenome.getTotalGeneCount() == registry.size());
    
    // Verify creature has expected genes
    assert(creatureGenome.hasGene(G::UniversalGenes::LIFESPAN));
    assert(creatureGenome.hasGene(G::UniversalGenes::LOCOMOTION));
    assert(creatureGenome.hasGene(G::UniversalGenes::PHOTOSYNTHESIS));  // Present but dormant
    assert(creatureGenome.hasGene(G::UniversalGenes::HUNT_INSTINCT));
    
    // Verify new genes are present (Phase 2.1)
    assert(creatureGenome.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY));
    assert(creatureGenome.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY));
    assert(creatureGenome.hasGene(G::UniversalGenes::GUT_LENGTH));
    assert(creatureGenome.hasGene(G::UniversalGenes::TOOTH_SHARPNESS));
    assert(creatureGenome.hasGene(G::UniversalGenes::TOXIN_PRODUCTION));  // Present but dormant in creatures
    
    // Check that mobility genes have high expression (>= 1.0 value)
    const G::Gene& locomotionGene = creatureGenome.getGene(G::UniversalGenes::LOCOMOTION);
    float locomotionExpr = locomotionGene.getAllele1().expression_strength;
    assert(locomotionExpr >= 0.9f);  // High expression for creatures
    std::cout << "  Creature locomotion expression: " << locomotionExpr << std::endl;
    
    // Check that autotrophy genes have low expression (dormant)
    const G::Gene& photoGene = creatureGenome.getGene(G::UniversalGenes::PHOTOSYNTHESIS);
    float photoExpr = photoGene.getAllele1().expression_strength;
    assert(photoExpr <= 0.2f);  // Dormant in creatures
    std::cout << "  Creature photosynthesis expression: " << photoExpr << std::endl;
    
    // Check that digestion genes are balanced (omnivore default)
    const G::Gene& plantDigestGene = creatureGenome.getGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    const G::Gene& meatDigestGene = creatureGenome.getGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float plantDigest = plantDigestGene.getNumericValue(G::DominanceType::Incomplete);
    float meatDigest = meatDigestGene.getNumericValue(G::DominanceType::Incomplete);
    std::cout << "  Creature plant digestion: " << plantDigest << std::endl;
    std::cout << "  Creature meat digestion: " << meatDigest << std::endl;
    assert(plantDigest >= 0.4f && plantDigest <= 0.6f);  // Balanced
    assert(meatDigest >= 0.4f && meatDigest <= 0.6f);    // Balanced
    
    std::cout << "  createCreatureGenome(): PASSED" << std::endl;
}

void testPlantGenome() {
    std::cout << "Testing UniversalGenes::createPlantGenome()..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    
    // Should have all genes
    assert(plantGenome.getTotalGeneCount() == registry.size());
    
    // Verify plant has expected genes
    assert(plantGenome.hasGene(G::UniversalGenes::LIFESPAN));
    assert(plantGenome.hasGene(G::UniversalGenes::LOCOMOTION));  // Present but dormant
    assert(plantGenome.hasGene(G::UniversalGenes::PHOTOSYNTHESIS));
    assert(plantGenome.hasGene(G::UniversalGenes::HUNT_INSTINCT));  // Present but dormant
    
    // Verify new genes are present (Phase 2.1)
    assert(plantGenome.hasGene(G::UniversalGenes::TOXIN_PRODUCTION));  // Active in plants
    assert(plantGenome.hasGene(G::UniversalGenes::THORN_DENSITY));     // Active in plants
    assert(plantGenome.hasGene(G::UniversalGenes::FRUIT_PRODUCTION_RATE));
    assert(plantGenome.hasGene(G::UniversalGenes::SEED_COAT_DURABILITY));
    
    // Check that autotrophy genes have high expression
    const G::Gene& photoGene = plantGenome.getGene(G::UniversalGenes::PHOTOSYNTHESIS);
    float photoExpr = photoGene.getAllele1().expression_strength;
    assert(photoExpr >= 0.9f);  // High expression for plants
    std::cout << "  Plant photosynthesis expression: " << photoExpr << std::endl;
    
    // Check that mobility genes have low expression (dormant)
    const G::Gene& locomotionGene = plantGenome.getGene(G::UniversalGenes::LOCOMOTION);
    float locomotionExpr = locomotionGene.getAllele1().expression_strength;
    assert(locomotionExpr <= 0.2f);  // Dormant in plants
    std::cout << "  Plant locomotion expression: " << locomotionExpr << std::endl;
    
    // Check that plant defense genes are active
    const G::Gene& toxinProdGene = plantGenome.getGene(G::UniversalGenes::TOXIN_PRODUCTION);
    float toxinProdExpr = toxinProdGene.getAllele1().expression_strength;
    assert(toxinProdExpr >= 0.9f);  // Active in plants
    std::cout << "  Plant toxin production expression: " << toxinProdExpr << std::endl;
    
    std::cout << "  createPlantGenome(): PASSED" << std::endl;
}

void testRandomGenome() {
    std::cout << "Testing UniversalGenes::createRandomGenome()..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome randomGenome = G::UniversalGenes::createRandomGenome(registry);
    
    // Should have all genes
    assert(randomGenome.getTotalGeneCount() == registry.size());
    
    // All genes should exist
    assert(randomGenome.hasGene(G::UniversalGenes::LIFESPAN));
    assert(randomGenome.hasGene(G::UniversalGenes::LOCOMOTION));
    assert(randomGenome.hasGene(G::UniversalGenes::PHOTOSYNTHESIS));
    assert(randomGenome.hasGene(G::UniversalGenes::OFFSPRING_COUNT));
    
    // New genes should exist too
    assert(randomGenome.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY));
    assert(randomGenome.hasGene(G::UniversalGenes::GUT_LENGTH));
    
    // Create a second random genome - should be different
    G::Genome randomGenome2 = G::UniversalGenes::createRandomGenome(registry);
    
    // They should have different values (extremely unlikely to be identical)
    const G::Gene& gene1 = randomGenome.getGene(G::UniversalGenes::LIFESPAN);
    const G::Gene& gene2 = randomGenome2.getGene(G::UniversalGenes::LIFESPAN);
    
    float val1 = gene1.getNumericValue(G::DominanceType::Incomplete);
    float val2 = gene2.getNumericValue(G::DominanceType::Incomplete);
    
    std::cout << "  Random genome 1 lifespan: " << val1 << std::endl;
    std::cout << "  Random genome 2 lifespan: " << val2 << std::endl;
    
    std::cout << "  createRandomGenome(): PASSED" << std::endl;
}

void testUniversalGenomeCrossover() {
    std::cout << "Testing Universal Genome crossover..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Cross a creature with a plant - should create a hybrid!
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    
    G::Genome hybridGenome = G::Genome::crossover(creatureGenome, plantGenome);
    
    // Hybrid should have all genes
    assert(hybridGenome.getTotalGeneCount() == registry.size());
    
    // Hybrid should have genes from both parents
    assert(hybridGenome.hasGene(G::UniversalGenes::LOCOMOTION));
    assert(hybridGenome.hasGene(G::UniversalGenes::PHOTOSYNTHESIS));
    
    // Should also have new Phase 2.1 genes
    assert(hybridGenome.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY));
    assert(hybridGenome.hasGene(G::UniversalGenes::TOXIN_PRODUCTION));
    
    std::cout << "  Universal Genome crossover: PASSED" << std::endl;
}

// ============================================================================
// PHASE 2.1 TESTS - Pleiotropy and Emergent Diet
// ============================================================================

void testPleiotropyEffects() {
    std::cout << "Testing Pleiotropy effects (Phase 2.1)..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Check that pleiotropy effects are registered for key genes
    
    // Plant digestion should affect meat digestion
    const G::GeneDefinition& plantDigestDef = registry.getDefinition(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    const auto& plantDigestEffects = plantDigestDef.getEffects();
    bool hasPlantMeatInhibition = false;
    for (const auto& effect : plantDigestEffects) {
        if (effect.target_trait == "meat_digestion_efficiency" && effect.scale_factor < 0) {
            hasPlantMeatInhibition = true;
            break;
        }
    }
    assert(hasPlantMeatInhibition);
    std::cout << "  Plant digestion inhibits meat digestion: PASSED" << std::endl;
    
    // Meat digestion should affect plant digestion
    const G::GeneDefinition& meatDigestDef = registry.getDefinition(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    const auto& meatDigestEffects = meatDigestDef.getEffects();
    bool hasMeatPlantInhibition = false;
    for (const auto& effect : meatDigestEffects) {
        if (effect.target_trait == "plant_digestion_efficiency" && effect.scale_factor < 0) {
            hasMeatPlantInhibition = true;
            break;
        }
    }
    assert(hasMeatPlantInhibition);
    std::cout << "  Meat digestion inhibits plant digestion: PASSED" << std::endl;
    
    // Gut length should affect movement speed
    const G::GeneDefinition& gutLengthDef = registry.getDefinition(G::UniversalGenes::GUT_LENGTH);
    const auto& gutLengthEffects = gutLengthDef.getEffects();
    bool hasGutSpeedEffect = false;
    for (const auto& effect : gutLengthEffects) {
        if (effect.target_trait == "speed_multiplier" && effect.scale_factor < 0) {
            hasGutSpeedEffect = true;
            break;
        }
    }
    assert(hasGutSpeedEffect);
    std::cout << "  Gut length reduces movement speed: PASSED" << std::endl;
    
    // Tooth sharpness should inhibit tooth grinding
    const G::GeneDefinition& toothSharpDef = registry.getDefinition(G::UniversalGenes::TOOTH_SHARPNESS);
    const auto& toothSharpEffects = toothSharpDef.getEffects();
    bool hasToothInhibition = false;
    for (const auto& effect : toothSharpEffects) {
        if (effect.target_trait == "tooth_grinding" && effect.scale_factor < 0) {
            hasToothInhibition = true;
            break;
        }
    }
    assert(hasToothInhibition);
    std::cout << "  Tooth sharpness inhibits tooth grinding: PASSED" << std::endl;
    
    // Stomach acidity should inhibit cellulose breakdown
    const G::GeneDefinition& stomachAcidDef = registry.getDefinition(G::UniversalGenes::STOMACH_ACIDITY);
    const auto& stomachAcidEffects = stomachAcidDef.getEffects();
    bool hasAcidCelluloseInhibition = false;
    for (const auto& effect : stomachAcidEffects) {
        if (effect.target_trait == "cellulose_breakdown" && effect.scale_factor < 0) {
            hasAcidCelluloseInhibition = true;
            break;
        }
    }
    assert(hasAcidCelluloseInhibition);
    std::cout << "  Stomach acidity inhibits cellulose breakdown: PASSED" << std::endl;
    
    std::cout << "  Pleiotropy effects: PASSED" << std::endl;
}

void testEmergentDietCalculation() {
    std::cout << "Testing Emergent Diet Type calculation (Phase 2.1)..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    // Test 1: Default creature genome should be OMNIVORE (balanced digestion)
    {
        G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
        G::Phenotype phenotype(&creatureGenome, &registry);
        phenotype.updateContext(env, org);
        
        G::DietType diet = phenotype.calculateDietType();
        std::cout << "  Creature diet type: " << G::Phenotype::dietTypeToString(diet) << std::endl;
        assert(diet == G::DietType::OMNIVORE);
        std::cout << "  Default creature is OMNIVORE: PASSED" << std::endl;
    }
    
    // Test 2: Create a carnivore genome (high meat, low plant digestion)
    {
        G::Genome carnivoreGenome;
        
        // Set high meat digestion, low plant digestion
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
        phenotype.updateContext(env, org);
        
        G::DietType diet = phenotype.calculateDietType();
        std::cout << "  Carnivore diet type: " << G::Phenotype::dietTypeToString(diet) << std::endl;
        assert(diet == G::DietType::CARNIVORE);
        std::cout << "  High meat/low plant creature is CARNIVORE: PASSED" << std::endl;
    }
    
    // Test 3: Create a herbivore genome (high plant, high cellulose breakdown)
    {
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
        phenotype.updateContext(env, org);
        
        G::DietType diet = phenotype.calculateDietType();
        std::cout << "  Herbivore diet type: " << G::Phenotype::dietTypeToString(diet) << std::endl;
        assert(diet == G::DietType::HERBIVORE);
        std::cout << "  High plant/cellulose creature is HERBIVORE: PASSED" << std::endl;
    }
    
    // Test 4: Create a frugivore genome (moderate plant, high color vision, low cellulose)
    {
        G::Genome frugivoreGenome;
        
        G::Allele plantMed(0.6f, 1.0f);
        G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantMed, plantMed);
        frugivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
        
        G::Allele meatLow(0.3f, 1.0f);
        G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatLow, meatLow);
        frugivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
        
        G::Allele celluloseLow(0.2f, 1.0f);  // Cannot digest tough plant material
        G::Gene cellulose(G::UniversalGenes::CELLULOSE_BREAKDOWN, celluloseLow, celluloseLow);
        frugivoreGenome.addGene(cellulose, G::ChromosomeType::Metabolism);
        
        G::Allele colorHigh(0.8f, 1.0f);  // High color vision for fruit detection
        G::Gene colorVision(G::UniversalGenes::COLOR_VISION, colorHigh, colorHigh);
        frugivoreGenome.addGene(colorVision, G::ChromosomeType::Sensory);
        
        G::Phenotype phenotype(&frugivoreGenome, &registry);
        phenotype.updateContext(env, org);
        
        G::DietType diet = phenotype.calculateDietType();
        std::cout << "  Frugivore diet type: " << G::Phenotype::dietTypeToString(diet) << std::endl;
        assert(diet == G::DietType::FRUGIVORE);
        std::cout << "  Color vision + low cellulose creature is FRUGIVORE: PASSED" << std::endl;
    }
    
    std::cout << "  Emergent Diet Type calculation: PASSED" << std::endl;
}

void testPhase21Integration() {
    std::cout << "Testing Phase 2.1 integration..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Create creature and plant genomes with all new genes
    G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    
    // Count genes
    size_t expectedGeneCount = registry.size();
    std::cout << "  Expected gene count: " << expectedGeneCount << std::endl;
    std::cout << "  Creature genome genes: " << creatureGenome.getTotalGeneCount() << std::endl;
    std::cout << "  Plant genome genes: " << plantGenome.getTotalGeneCount() << std::endl;
    
    assert(creatureGenome.getTotalGeneCount() == expectedGeneCount);
    assert(plantGenome.getTotalGeneCount() == expectedGeneCount);
    
    // Cross creature and plant to create hybrid
    G::Genome hybridGenome = G::Genome::crossover(creatureGenome, plantGenome);
    assert(hybridGenome.getTotalGeneCount() == expectedGeneCount);
    std::cout << "  Hybrid genome genes: " << hybridGenome.getTotalGeneCount() << std::endl;
    
    // Create phenotypes and test diet calculation
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    G::Phenotype creaturePhenotype(&creatureGenome, &registry);
    creaturePhenotype.updateContext(env, org);
    G::DietType creatureDiet = creaturePhenotype.calculateDietType();
    
    std::cout << "  Creature diet: " << G::Phenotype::dietTypeToString(creatureDiet) << std::endl;
    
    // Test that traits can be retrieved for new genes
    float plantDigest = creaturePhenotype.getTrait(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    float meatDigest = creaturePhenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float gutLength = creaturePhenotype.getTrait(G::UniversalGenes::GUT_LENGTH);
    
    std::cout << "  Plant digestion trait: " << plantDigest << std::endl;
    std::cout << "  Meat digestion trait: " << meatDigest << std::endl;
    std::cout << "  Gut length trait: " << gutLength << std::endl;
    
    assert(plantDigest >= 0.0f);
    assert(meatDigest >= 0.0f);
    assert(gutLength >= 0.0f);
    
    std::cout << "  Phase 2.1 integration: PASSED" << std::endl;
}

// ============================================================================
// PHASE 2.2 TESTS - Energy Budget System
// ============================================================================

void testMaintenanceCostCalculation() {
    std::cout << "Testing Maintenance Cost calculation (Phase 2.2)..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    // Test creature genome maintenance cost
    {
        G::Genome creatureGenome = G::UniversalGenes::createCreatureGenome(registry);
        G::Phenotype phenotype(&creatureGenome, &registry);
        phenotype.updateContext(env, org);
        
        float maintenanceCost = phenotype.getTotalMaintenanceCost();
        std::cout << "  Creature total maintenance cost: " << maintenanceCost << std::endl;
        
        // Should have a positive maintenance cost
        assert(maintenanceCost > 0.0f);
        
        // Creature has many active genes, should have substantial cost
        assert(maintenanceCost > 1.0f);  // At least 1.0 per tick
        
        std::cout << "  Creature maintenance cost > 0: PASSED" << std::endl;
    }
    
    // Test plant genome maintenance cost (lower than creature due to dormant mobility genes)
    {
        G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
        G::Phenotype phenotype(&plantGenome, &registry);
        phenotype.updateContext(env, org);
        
        float maintenanceCost = phenotype.getTotalMaintenanceCost();
        std::cout << "  Plant total maintenance cost: " << maintenanceCost << std::endl;
        
        // Should have a positive maintenance cost
        assert(maintenanceCost > 0.0f);
        
        std::cout << "  Plant maintenance cost > 0: PASSED" << std::endl;
    }
    
    // Test GeneDefinition::calculateMaintenanceCost directly
    {
        const G::GeneDefinition& locomotionDef = registry.getDefinition(G::UniversalGenes::LOCOMOTION);
        
        // Full expression should give full cost
        float fullCost = locomotionDef.calculateMaintenanceCost(1.0f);
        std::cout << "  Locomotion full expression cost: " << fullCost << std::endl;
        assert(fullCost > 0.0f);
        
        // Half expression should give less cost (due to scaling)
        float halfCost = locomotionDef.calculateMaintenanceCost(0.5f);
        std::cout << "  Locomotion half expression cost: " << halfCost << std::endl;
        assert(halfCost < fullCost);
        assert(halfCost > 0.0f);
        
        // Zero expression should give zero cost
        float zeroCost = locomotionDef.calculateMaintenanceCost(0.0f);
        std::cout << "  Locomotion zero expression cost: " << zeroCost << std::endl;
        assert(zeroCost == 0.0f);
        
        std::cout << "  GeneDefinition::calculateMaintenanceCost: PASSED" << std::endl;
    }
    
    std::cout << "  Maintenance Cost calculation: PASSED" << std::endl;
}

void testSpecialistBonus() {
    std::cout << "Testing Specialist Bonus calculation (Phase 2.2)..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    // Test 1: Balanced omnivore should have low/no specialist bonus
    {
        G::Genome omnivoreGenome;
        
        G::Allele plantMed(0.5f, 1.0f);
        G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantMed, plantMed);
        omnivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
        
        G::Allele meatMed(0.5f, 1.0f);
        G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatMed, meatMed);
        omnivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
        
        G::Phenotype phenotype(&omnivoreGenome, &registry);
        phenotype.updateContext(env, org);
        
        float bonus = phenotype.getSpecialistBonus();
        std::cout << "  Omnivore specialist bonus: " << bonus << std::endl;
        
        // Balanced digestion = low bonus (close to 1.0)
        assert(bonus >= 1.0f);
        assert(bonus <= 1.05f);  // Near 1.0 for balanced
        
        std::cout << "  Balanced omnivore has low bonus: PASSED" << std::endl;
    }
    
    // Test 2: Specialist carnivore should have high bonus
    {
        G::Genome carnivoreGenome;
        
        G::Allele plantLow(0.1f, 1.0f);
        G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantLow, plantLow);
        carnivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
        
        G::Allele meatHigh(0.9f, 1.0f);
        G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatHigh, meatHigh);
        carnivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
        
        G::Phenotype phenotype(&carnivoreGenome, &registry);
        phenotype.updateContext(env, org);
        
        float bonus = phenotype.getSpecialistBonus();
        std::cout << "  Carnivore specialist bonus: " << bonus << std::endl;
        
        // Large difference = high bonus (up to 30%)
        assert(bonus > 1.2f);
        assert(bonus <= 1.3f);
        
        std::cout << "  Specialist carnivore has high bonus: PASSED" << std::endl;
    }
    
    // Test 3: Specialist herbivore should also have high bonus
    {
        G::Genome herbivoreGenome;
        
        G::Allele plantHigh(0.9f, 1.0f);
        G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantHigh, plantHigh);
        herbivoreGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
        
        G::Allele meatLow(0.1f, 1.0f);
        G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatLow, meatLow);
        herbivoreGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
        
        G::Phenotype phenotype(&herbivoreGenome, &registry);
        phenotype.updateContext(env, org);
        
        float bonus = phenotype.getSpecialistBonus();
        std::cout << "  Herbivore specialist bonus: " << bonus << std::endl;
        
        // Large difference = high bonus (up to 30%)
        assert(bonus > 1.2f);
        assert(bonus <= 1.3f);
        
        std::cout << "  Specialist herbivore has high bonus: PASSED" << std::endl;
    }
    
    std::cout << "  Specialist Bonus calculation: PASSED" << std::endl;
}

void testMetabolicOverhead() {
    std::cout << "Testing Metabolic Overhead calculation (Phase 2.2)..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::OrganismState org;
    org.age_normalized = 0.5f;
    org.energy_level = 1.0f;
    org.health = 1.0f;
    
    // Test 1: Minimal systems active should have low overhead
    {
        G::Genome specialistGenome;
        
        // Only one system active (above 0.3 threshold)
        G::Allele plantHigh(0.8f, 1.0f);
        G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantHigh, plantHigh);
        specialistGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
        
        G::Allele meatLow(0.1f, 1.0f);  // Below threshold
        G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatLow, meatLow);
        specialistGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
        
        G::Allele celluloseLow(0.1f, 1.0f);  // Below threshold
        G::Gene cellulose(G::UniversalGenes::CELLULOSE_BREAKDOWN, celluloseLow, celluloseLow);
        specialistGenome.addGene(cellulose, G::ChromosomeType::Metabolism);
        
        G::Allele toxinLow(0.1f, 1.0f);  // Below threshold
        G::Gene toxinMetab(G::UniversalGenes::TOXIN_METABOLISM, toxinLow, toxinLow);
        specialistGenome.addGene(toxinMetab, G::ChromosomeType::Metabolism);
        
        G::Phenotype phenotype(&specialistGenome, &registry);
        phenotype.updateContext(env, org);
        
        float overhead = phenotype.getMetabolicOverhead();
        std::cout << "  Specialist (1 system) overhead: " << overhead << std::endl;
        
        // One system = 1.08^1 = 1.08
        assert(overhead >= 1.07f);
        assert(overhead <= 1.09f);
        
        std::cout << "  Single system overhead ~1.08: PASSED" << std::endl;
    }
    
    // Test 2: Generalist with all systems active should have high overhead
    {
        G::Genome generalistGenome;
        
        // All four systems active (above 0.3 threshold)
        G::Allele plantHigh(0.5f, 1.0f);
        G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantHigh, plantHigh);
        generalistGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
        
        G::Allele meatHigh(0.5f, 1.0f);
        G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatHigh, meatHigh);
        generalistGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
        
        G::Allele celluloseHigh(0.5f, 1.0f);
        G::Gene cellulose(G::UniversalGenes::CELLULOSE_BREAKDOWN, celluloseHigh, celluloseHigh);
        generalistGenome.addGene(cellulose, G::ChromosomeType::Metabolism);
        
        G::Allele toxinHigh(0.5f, 1.0f);
        G::Gene toxinMetab(G::UniversalGenes::TOXIN_METABOLISM, toxinHigh, toxinHigh);
        generalistGenome.addGene(toxinMetab, G::ChromosomeType::Metabolism);
        
        G::Phenotype phenotype(&generalistGenome, &registry);
        phenotype.updateContext(env, org);
        
        float overhead = phenotype.getMetabolicOverhead();
        std::cout << "  Generalist (4 systems) overhead: " << overhead << std::endl;
        
        // Four systems = 1.08^4 ≈ 1.36
        assert(overhead >= 1.34f);
        assert(overhead <= 1.38f);
        
        std::cout << "  Four system overhead ~1.36: PASSED" << std::endl;
    }
    
    // Test 3: No systems active should have no overhead
    {
        G::Genome minimalGenome;
        
        // All systems below threshold
        G::Allele plantLow(0.2f, 1.0f);
        G::Gene plantDigest(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantLow, plantLow);
        minimalGenome.addGene(plantDigest, G::ChromosomeType::Metabolism);
        
        G::Allele meatLow(0.2f, 1.0f);
        G::Gene meatDigest(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatLow, meatLow);
        minimalGenome.addGene(meatDigest, G::ChromosomeType::Metabolism);
        
        G::Allele celluloseLow(0.2f, 1.0f);
        G::Gene cellulose(G::UniversalGenes::CELLULOSE_BREAKDOWN, celluloseLow, celluloseLow);
        minimalGenome.addGene(cellulose, G::ChromosomeType::Metabolism);
        
        G::Allele toxinLow(0.2f, 1.0f);
        G::Gene toxinMetab(G::UniversalGenes::TOXIN_METABOLISM, toxinLow, toxinLow);
        minimalGenome.addGene(toxinMetab, G::ChromosomeType::Metabolism);
        
        G::Phenotype phenotype(&minimalGenome, &registry);
        phenotype.updateContext(env, org);
        
        float overhead = phenotype.getMetabolicOverhead();
        std::cout << "  Minimal (0 systems) overhead: " << overhead << std::endl;
        
        // No systems = 1.08^0 = 1.0
        assert(overhead == 1.0f);
        
        std::cout << "  Zero system overhead = 1.0: PASSED" << std::endl;
    }
    
    std::cout << "  Metabolic Overhead calculation: PASSED" << std::endl;
}

void testEnergyBudgetUpdate() {
    std::cout << "Testing EnergyBudget::updateEnergy (Phase 2.2)..." << std::endl;
    
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
    
    // Test 1: Energy gain with sufficient income
    {
        G::EnergyState state;
        state.currentEnergy = 50.0f;
        state.maxEnergy = 100.0f;
        state.baseMetabolism = 1.0f;
        state.maintenanceCost = 2.0f;
        state.activityCost = 0.0f;
        
        float income = 5.0f;  // More than base + maintenance
        float activity = 0.0f;
        
        G::EnergyState newState = energyBudget.updateEnergy(state, income, activity);
        
        std::cout << "  Energy after update (gain): " << newState.currentEnergy << std::endl;
        
        // Should gain energy: 50 + 5 - 1 - 2 - 0 = 52
        assert(newState.currentEnergy > 50.0f);
        assert(newState.totalExpenditure > 0.0f);
        
        std::cout << "  Energy gain with sufficient income: PASSED" << std::endl;
    }
    
    // Test 2: Energy loss with insufficient income
    {
        G::EnergyState state;
        state.currentEnergy = 50.0f;
        state.maxEnergy = 100.0f;
        state.baseMetabolism = 1.0f;
        state.maintenanceCost = 2.0f;
        state.activityCost = 0.0f;
        
        float income = 1.0f;  // Less than base + maintenance
        float activity = 0.0f;
        
        G::EnergyState newState = energyBudget.updateEnergy(state, income, activity);
        
        std::cout << "  Energy after update (loss): " << newState.currentEnergy << std::endl;
        
        // Should lose energy: 50 + 1 - 1 - 2 - 0 = 48
        assert(newState.currentEnergy < 50.0f);
        
        std::cout << "  Energy loss with insufficient income: PASSED" << std::endl;
    }
    
    // Test 3: Energy capped at maximum
    {
        G::EnergyState state;
        state.currentEnergy = 95.0f;
        state.maxEnergy = 100.0f;
        state.baseMetabolism = 1.0f;
        state.maintenanceCost = 1.0f;
        state.activityCost = 0.0f;
        
        float income = 20.0f;  // Way more than needed
        float activity = 0.0f;
        
        G::EnergyState newState = energyBudget.updateEnergy(state, income, activity);
        
        std::cout << "  Energy after update (capped): " << newState.currentEnergy << std::endl;
        
        // Should be capped at maxEnergy
        assert(newState.currentEnergy <= state.maxEnergy);
        
        std::cout << "  Energy capped at maximum: PASSED" << std::endl;
    }
    
    // Test 4: Starvation detection
    {
        G::EnergyState state;
        state.currentEnergy = 5.0f;  // 5% of max
        state.maxEnergy = 100.0f;
        
        bool starving = energyBudget.isStarving(state);
        std::cout << "  Is starving (5%): " << (starving ? "yes" : "no") << std::endl;
        assert(starving == true);
        
        state.currentEnergy = 15.0f;  // 15% of max
        starving = energyBudget.isStarving(state);
        std::cout << "  Is starving (15%): " << (starving ? "yes" : "no") << std::endl;
        assert(starving == false);
        
        std::cout << "  Starvation detection: PASSED" << std::endl;
    }
    
    // Test 5: Reproduction check
    {
        G::EnergyState state;
        state.currentEnergy = 80.0f;
        state.maxEnergy = 100.0f;
        
        float reproductionCost = 30.0f;
        
        bool canReproduce = energyBudget.canReproduce(state, reproductionCost);
        std::cout << "  Can reproduce (80 energy, 30 cost): " << (canReproduce ? "yes" : "no") << std::endl;
        assert(canReproduce == true);
        
        state.currentEnergy = 40.0f;  // Not enough after reproduction
        canReproduce = energyBudget.canReproduce(state, reproductionCost);
        std::cout << "  Can reproduce (40 energy, 30 cost): " << (canReproduce ? "yes" : "no") << std::endl;
        assert(canReproduce == false);  // Would leave < 20% energy
        
        std::cout << "  Reproduction check: PASSED" << std::endl;
    }
    
    std::cout << "  EnergyBudget::updateEnergy: PASSED" << std::endl;
}

void testPhase22Integration() {
    std::cout << "Testing Phase 2.2 integration..." << std::endl;
    
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::EnvironmentState env;
    env.temperature = 20.0f;
    
    G::Organism
