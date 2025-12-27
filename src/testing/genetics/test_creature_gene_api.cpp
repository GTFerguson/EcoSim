/**
 * @file test_creature_gene_api.cpp
 * @brief Diagnostic test to debug why creature gene values show as 0 in inspector
 * 
 * This test simulates exactly what the inspector does to read gene values,
 * to identify where the data flow breaks.
 */

#include "test_framework.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <iostream>
#include <iomanip>

using namespace EcoSim::Genetics;
namespace G = EcoSim::Genetics;
using namespace EcoSim::Testing;

/**
 * Test that diagnoses the issue by checking the entire data flow
 */
void testCreatureGeneAPI_Diagnosis() {
    std::cout << "\n    ========================================" << std::endl;
    std::cout << "    DIAGNOSTIC: Creature Gene API Flow" << std::endl;
    std::cout << "    ========================================\n" << std::endl;
    
    // Step 1: Create registry like Creature::initializeGeneRegistry() does
    std::cout << "    Step 1: Creating registry with G::UniversalGenes::registerDefaults()..." << std::endl;
    GeneRegistry registryWithDefaults;
    G::UniversalGenes::registerDefaults(registryWithDefaults);
    
    // Check which genes are registered
    std::cout << "\n    Checking if key genes exist in registry (UniversalGenes):" << std::endl;
    std::cout << "      - lifespan: " << (registryWithDefaults.hasGene("lifespan") ? "YES" : "NO") << std::endl;
    std::cout << "      - sight: " << (registryWithDefaults.hasGene("sight") ? "YES" : "NO") << std::endl;
    std::cout << "      - plant_digestion_efficiency: " << (registryWithDefaults.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - meat_digestion_efficiency: " << (registryWithDefaults.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - cellulose_breakdown: " << (registryWithDefaults.hasGene(G::UniversalGenes::CELLULOSE_BREAKDOWN) ? "YES" : "NO") << std::endl;
    
    // Step 2: Create genome using UniversalGenes (like enableNewGenetics does)
    std::cout << "\n    Step 2: Creating genome with G::UniversalGenes::createCreatureGenome()..." << std::endl;
    Genome genomeFromDefaults = G::UniversalGenes::createCreatureGenome(registryWithDefaults);
    
    // Check which genes exist in the genome
    std::cout << "\n    Genes in genome created with UniversalGenes registry:" << std::endl;
    std::cout << "      - plant_digestion_efficiency: " << (genomeFromDefaults.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - meat_digestion_efficiency: " << (genomeFromDefaults.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - sight_range: " << (genomeFromDefaults.hasGene(G::UniversalGenes::SIGHT_RANGE) ? "YES" : "NO") << std::endl;
    
    // Step 3: Create phenotype (like enableNewGenetics does)
    std::cout << "\n    Step 3: Creating phenotype..." << std::endl;
    Phenotype phenotypeFromDefaults(&genomeFromDefaults, &registryWithDefaults);
    
    // Check if phenotype is valid
    std::cout << "      Phenotype is valid: " << (phenotypeFromDefaults.isValid() ? "YES" : "NO") << std::endl;
    
    // Step 4: Try to read trait values like inspector does
    std::cout << "\n    Step 4: Reading trait values (same API as inspector):" << std::endl;
    float plantDigestion = phenotypeFromDefaults.getTrait(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    float meatDigestion = phenotypeFromDefaults.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float sightRange = phenotypeFromDefaults.getTrait(G::UniversalGenes::SIGHT_RANGE);
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "      plant_digestion_efficiency: " << plantDigestion << std::endl;
    std::cout << "      meat_digestion_efficiency: " << meatDigestion << std::endl;
    std::cout << "      sight_range: " << sightRange << std::endl;
    
    std::cout << "\n    ========================================" << std::endl;
    std::cout << "    NOW TRYING WITH G::UniversalGenes::registerDefaults()" << std::endl;
    std::cout << "    ========================================\n" << std::endl;
    
    // Step 5: Create registry with UniversalGenes (the correct way)
    std::cout << "    Step 5: Creating registry with G::UniversalGenes::registerDefaults()..." << std::endl;
    GeneRegistry registryWithUniversal;
    G::UniversalGenes::registerDefaults(registryWithUniversal);
    
    // Check which genes are registered
    std::cout << "\n    Checking if key genes exist in registry (UniversalGenes):" << std::endl;
    std::cout << "      - lifespan: " << (registryWithUniversal.hasGene(G::UniversalGenes::LIFESPAN) ? "YES" : "NO") << std::endl;
    std::cout << "      - sight_range: " << (registryWithUniversal.hasGene(G::UniversalGenes::SIGHT_RANGE) ? "YES" : "NO") << std::endl;
    std::cout << "      - plant_digestion_efficiency: " << (registryWithUniversal.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - meat_digestion_efficiency: " << (registryWithUniversal.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - cellulose_breakdown: " << (registryWithUniversal.hasGene(G::UniversalGenes::CELLULOSE_BREAKDOWN) ? "YES" : "NO") << std::endl;
    
    // Step 6: Create genome using UniversalGenes with proper registry
    std::cout << "\n    Step 6: Creating genome with G::UniversalGenes::createCreatureGenome()..." << std::endl;
    Genome genomeFromUniversal = G::UniversalGenes::createCreatureGenome(registryWithUniversal);
    
    // Check which genes exist in the genome
    std::cout << "\n    Genes in genome created with UniversalGenes registry:" << std::endl;
    std::cout << "      - plant_digestion_efficiency: " << (genomeFromUniversal.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - meat_digestion_efficiency: " << (genomeFromUniversal.hasGene(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY) ? "YES" : "NO") << std::endl;
    std::cout << "      - sight_range: " << (genomeFromUniversal.hasGene(G::UniversalGenes::SIGHT_RANGE) ? "YES" : "NO") << std::endl;
    
    // Direct gene value check
    if (genomeFromUniversal.hasGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY)) {
        const Gene& gene = genomeFromUniversal.getGene(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
        std::cout << "\n      Direct plant_digestion gene value: " << gene.getNumericValue(DominanceType::Incomplete) << std::endl;
    }
    
    // Step 7: Create phenotype with correct registry
    std::cout << "\n    Step 7: Creating phenotype..." << std::endl;
    Phenotype phenotypeFromUniversal(&genomeFromUniversal, &registryWithUniversal);
    
    // Check if phenotype is valid
    std::cout << "      Phenotype is valid: " << (phenotypeFromUniversal.isValid() ? "YES" : "NO") << std::endl;
    
    // Step 8: Read trait values
    std::cout << "\n    Step 8: Reading trait values (same API as inspector):" << std::endl;
    float plantDigestion2 = phenotypeFromUniversal.getTrait(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    float meatDigestion2 = phenotypeFromUniversal.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float sightRange2 = phenotypeFromUniversal.getTrait(G::UniversalGenes::SIGHT_RANGE);
    float celluloseBreakdown = phenotypeFromUniversal.getTrait(G::UniversalGenes::CELLULOSE_BREAKDOWN);
    float toxinTolerance = phenotypeFromUniversal.getTrait(G::UniversalGenes::TOXIN_TOLERANCE);
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "      plant_digestion_efficiency: " << plantDigestion2 << std::endl;
    std::cout << "      meat_digestion_efficiency: " << meatDigestion2 << std::endl;
    std::cout << "      sight_range: " << sightRange2 << std::endl;
    std::cout << "      cellulose_breakdown: " << celluloseBreakdown << std::endl;
    std::cout << "      toxin_tolerance: " << toxinTolerance << std::endl;
    
    std::cout << "\n    ========================================" << std::endl;
    std::cout << "    DIAGNOSIS SUMMARY" << std::endl;
    std::cout << "    ========================================\n" << std::endl;
    
    bool defaultsWork = (plantDigestion > 0.0f && meatDigestion > 0.0f);
    bool universalWorks = (plantDigestion2 > 0.0f && meatDigestion2 > 0.0f);
    
    std::cout << "    With UniversalGenes registry: " << (defaultsWork ? "VALUES NON-ZERO" : "VALUES ARE ZERO (BUG!)") << std::endl;
    std::cout << "    With UniversalGenes registry: " << (universalWorks ? "VALUES NON-ZERO (EXPECTED)" : "VALUES ARE ZERO (UNEXPECTED!)") << std::endl;
    
    if (!defaultsWork && universalWorks) {
        std::cout << "\n    >>> ROOT CAUSE IDENTIFIED <<<" << std::endl;
        std::cout << "    Creature::initializeGeneRegistry() uses G::UniversalGenes::registerDefaults()" << std::endl;
        std::cout << "    But enableNewGenetics() uses G::UniversalGenes::createCreatureGenome()" << std::endl;
        std::cout << "    The UniversalGenes (like plant_digestion_efficiency) are NOT in the registry!" << std::endl;
        std::cout << "\n    FIX: Change Creature::initializeGeneRegistry() to use:" << std::endl;
        std::cout << "         G::UniversalGenes::registerDefaults(*s_geneRegistry);" << std::endl;
    }
    
    // Assertions
    TEST_ASSERT_MSG(universalWorks, "UniversalGenes registry should produce non-zero values");
}

/**
 * Test that verifies the fix would work
 */
void testCreatureGeneAPI_VerifyFix() {
    std::cout << "\n    ========================================" << std::endl;
    std::cout << "    VERIFY FIX: Using UniversalGenes registry" << std::endl;
    std::cout << "    ========================================\n" << std::endl;
    
    // Create registry with UniversalGenes (the fix)
    GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Create creature genome
    Genome genome = G::UniversalGenes::createCreatureGenome(registry);
    
    // Create phenotype
    Phenotype phenotype(&genome, &registry);
    
    // Verify all inspector traits have non-zero values
    float plantDigestion = phenotype.getTrait(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    float meatDigestion = phenotype.getTrait(G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float celluloseBreakdown = phenotype.getTrait(G::UniversalGenes::CELLULOSE_BREAKDOWN);
    float toxinTolerance = phenotype.getTrait(G::UniversalGenes::TOXIN_TOLERANCE);
    float sightRange = phenotype.getTrait(G::UniversalGenes::SIGHT_RANGE);
    float locomotion = phenotype.getTrait(G::UniversalGenes::LOCOMOTION);
    float hideThickness = phenotype.getTrait(G::UniversalGenes::HIDE_THICKNESS);
    float furDensity = phenotype.getTrait(G::UniversalGenes::FUR_DENSITY);
    float toothSharpness = phenotype.getTrait(G::UniversalGenes::TOOTH_SHARPNESS);
    float toothGrinding = phenotype.getTrait(G::UniversalGenes::TOOTH_GRINDING);
    float gutLength = phenotype.getTrait(G::UniversalGenes::GUT_LENGTH);
    float jawStrength = phenotype.getTrait(G::UniversalGenes::JAW_STRENGTH);
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "    Trait values with fixed registry:" << std::endl;
    std::cout << "      plant_digestion_efficiency: " << plantDigestion << std::endl;
    std::cout << "      meat_digestion_efficiency: " << meatDigestion << std::endl;
    std::cout << "      cellulose_breakdown: " << celluloseBreakdown << std::endl;
    std::cout << "      toxin_tolerance: " << toxinTolerance << std::endl;
    std::cout << "      sight_range: " << sightRange << std::endl;
    std::cout << "      locomotion: " << locomotion << std::endl;
    std::cout << "      hide_thickness: " << hideThickness << std::endl;
    std::cout << "      fur_density: " << furDensity << std::endl;
    std::cout << "      tooth_sharpness: " << toothSharpness << std::endl;
    std::cout << "      tooth_grinding: " << toothGrinding << std::endl;
    std::cout << "      gut_length: " << gutLength << std::endl;
    std::cout << "      jaw_strength: " << jawStrength << std::endl;
    
    // All traits should be non-zero (creature defaults from createCreatureGenome)
    TEST_ASSERT_MSG(plantDigestion > 0.0f, "plant_digestion_efficiency should be > 0");
    TEST_ASSERT_MSG(meatDigestion > 0.0f, "meat_digestion_efficiency should be > 0");
    TEST_ASSERT_MSG(celluloseBreakdown > 0.0f, "cellulose_breakdown should be > 0");
    TEST_ASSERT_MSG(toxinTolerance > 0.0f, "toxin_tolerance should be > 0");
    TEST_ASSERT_MSG(sightRange > 0.0f, "sight_range should be > 0");
    TEST_ASSERT_MSG(locomotion > 0.0f, "locomotion should be > 0");
    TEST_ASSERT_MSG(hideThickness > 0.0f, "hide_thickness should be > 0");
    TEST_ASSERT_MSG(furDensity > 0.0f, "fur_density should be > 0");
    TEST_ASSERT_MSG(toothSharpness > 0.0f, "tooth_sharpness should be > 0");
    TEST_ASSERT_MSG(toothGrinding > 0.0f, "tooth_grinding should be > 0");
    TEST_ASSERT_MSG(gutLength > 0.0f, "gut_length should be > 0");
    TEST_ASSERT_MSG(jawStrength > 0.0f, "jaw_strength should be > 0");
}

/**
 * Run all creature gene API tests
 */
void runCreatureGeneAPITests() {
    BEGIN_TEST_GROUP("Creature Gene API Tests");
    RUN_TEST(testCreatureGeneAPI_Diagnosis);
    RUN_TEST(testCreatureGeneAPI_VerifyFix);
    END_TEST_GROUP();
}
