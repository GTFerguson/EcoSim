/**
 * Test creature classification system
 * Diagnoses why all creatures are "Omnivore Generalist"
 */

// Enable debug output in CreatureFactory
#define DEBUG_CLASSIFICATION

#include "test_framework.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Gene.hpp"
#include "objects/creature/creature.hpp"
#include <iostream>
#include <iomanip>
#include <memory>

using namespace EcoSim::Genetics;

// Print BOTH raw genome value and phenotype expressed value
void printGeneComparison(Creature& creature, const char* geneId, const std::string& label) {
    float expressedValue = creature.getExpressedValue(geneId);
    
    // Try to get raw value directly from genome
    float rawValue = -1.0f;
    const EcoSim::Genetics::Genome* genome = creature.getGenome();
    if (genome && genome->hasGene(geneId)) {
        const EcoSim::Genetics::Gene& gene = genome->getGene(geneId);
        rawValue = gene.getNumericValue(DominanceType::Incomplete);
    }
    
    std::cout << "  " << std::left << std::setw(35) << label
              << ": Raw=" << std::setw(8) << rawValue
              << " Expressed=" << expressedValue << std::endl;
}

void printGeneValue(Creature& creature, const char* geneId, const std::string& label) {
    float value = creature.getExpressedValue(geneId);
    std::cout << "  " << std::left << std::setw(35) << label << ": " << value << std::endl;
}

void testClassification(CreatureFactory& factory, const std::string& templateName) {
    std::cout << "\n=== Testing Template: " << templateName << " ===" << std::endl;
    
    Creature creature = factory.createFromTemplate(templateName, 0, 0);
    
    std::cout << "Archetype Label: " << creature.getArchetypeLabel() << std::endl;
    
    // Check if genome exists
    if (!creature.getGenome()) {
        std::cout << "ERROR: Genome not initialized!" << std::endl;
        return;
    }
    
    std::cout << "\nKey Genes (comparing Raw genome vs Expressed phenotype):" << std::endl;
    printGeneComparison(creature, UniversalGenes::MEAT_DIGESTION_EFFICIENCY, "MEAT_DIGESTION_EFFICIENCY");
    printGeneComparison(creature, UniversalGenes::PLANT_DIGESTION_EFFICIENCY, "PLANT_DIGESTION_EFFICIENCY");
    printGeneComparison(creature, UniversalGenes::MAX_SIZE, "MAX_SIZE");
    printGeneComparison(creature, UniversalGenes::COMBAT_AGGRESSION, "COMBAT_AGGRESSION");
    printGeneComparison(creature, UniversalGenes::PACK_COORDINATION, "PACK_COORDINATION");
    printGeneComparison(creature, UniversalGenes::LOCOMOTION, "LOCOMOTION");
    printGeneComparison(creature, UniversalGenes::HIDE_THICKNESS, "HIDE_THICKNESS");
    printGeneComparison(creature, UniversalGenes::SCENT_MASKING, "SCENT_MASKING");
    
    // Check what the template actually has
    const CreatureTemplate* tmpl = factory.getTemplate(templateName);
    if (tmpl) {
        std::cout << "\nTemplate Gene Ranges (should be applied):" << std::endl;
        for (const auto& [geneId, range] : tmpl->geneRanges) {
            if (geneId == UniversalGenes::MAX_SIZE ||
                geneId == UniversalGenes::COMBAT_AGGRESSION ||
                geneId == UniversalGenes::MEAT_DIGESTION_EFFICIENCY ||
                geneId == UniversalGenes::PLANT_DIGESTION_EFFICIENCY ||
                geneId == UniversalGenes::PACK_COORDINATION ||
                geneId == UniversalGenes::LOCOMOTION ||
                geneId == UniversalGenes::HIDE_THICKNESS ||
                geneId == UniversalGenes::SCENT_MASKING) {
                std::cout << "  " << std::left << std::setw(35) << geneId
                          << ": [" << range.first << ", " << range.second << "]" << std::endl;
            }
        }
    } else {
        std::cout << "\nWARNING: Template '" << templateName << "' not found!" << std::endl;
    }
}

int main() {
    std::cout << "=== Creature Classification Diagnostic ===" << std::endl;
    
    // Initialize gene registry
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    
    // Create factory and register templates
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // List available templates
    std::cout << "\nAvailable templates:" << std::endl;
    for (const auto& name : factory.getTemplateNames()) {
        std::cout << "  - " << name << std::endl;
    }
    
    // Test each template type
    testClassification(factory, "apex_predator");
    testClassification(factory, "pack_hunter");
    testClassification(factory, "ambush_predator");
    testClassification(factory, "tank_herbivore");
    testClassification(factory, "fleet_runner");
    testClassification(factory, "herd_grazer");
    
    // Also test a creature without template to see default behavior
    std::cout << "\n=== Testing Creature Without Template ===" << std::endl;
    Creature creature = factory.createFromTemplate("nonexistent_template", 0, 0);
    std::cout << "Archetype Label: " << creature.getArchetypeLabel() << std::endl;
    printGeneValue(creature, UniversalGenes::MEAT_DIGESTION_EFFICIENCY, "MEAT_DIGESTION_EFFICIENCY");
    printGeneValue(creature, UniversalGenes::PLANT_DIGESTION_EFFICIENCY, "PLANT_DIGESTION_EFFICIENCY");
    printGeneValue(creature, UniversalGenes::MAX_SIZE, "MAX_SIZE");
    
    return 0;
}
