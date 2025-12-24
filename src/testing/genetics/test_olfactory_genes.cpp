/**
 * @file test_olfactory_genes.cpp
 * @brief Quick diagnostic to verify olfactory genes are set in creature prefabs
 */

#include "test_framework.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <iostream>
#include <iomanip>

using namespace EcoSim::Genetics;

void testOlfactoryGenes() {
    std::cout << "\n========== OLFACTORY GENE DIAGNOSTIC ==========\n\n";
    
    auto registry = std::make_shared<GeneRegistry>();
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::string> templates = {"grazer", "browser", "hunter", "forager", "scavenger"};
    
    const char* olfactoryGenes[] = {
        UniversalGenes::SCENT_PRODUCTION,
        UniversalGenes::SCENT_SIGNATURE_VARIANCE,
        UniversalGenes::OLFACTORY_ACUITY,
        UniversalGenes::SCENT_MASKING
    };
    
    bool allPassed = true;
    
    for (const auto& templateName : templates) {
        const CreatureTemplate* tmpl = factory.getTemplate(templateName);
        
        std::cout << "Template: " << templateName << "\n";
        std::cout << std::string(40, '-') << "\n";
        
        if (!tmpl) {
            std::cout << "  ERROR: Template not found!\n\n";
            allPassed = false;
            continue;
        }
        
        for (const char* gene : olfactoryGenes) {
            auto it = tmpl->geneRanges.find(gene);
            if (it == tmpl->geneRanges.end()) {
                std::cout << "  " << std::setw(28) << gene << ": NOT SET (FAIL)\n";
                allPassed = false;
            } else {
                std::cout << "  " << std::setw(28) << gene 
                          << ": [" << std::fixed << std::setprecision(2) 
                          << it->second.first << ", " << it->second.second << "] ✓\n";
            }
        }
        std::cout << "\n";
    }
    
    std::cout << "========================================\n";
    if (allPassed) {
        std::cout << "✅ All olfactory genes are properly configured!\n";
    } else {
        std::cout << "❌ Some olfactory genes are missing!\n";
    }
    std::cout << "========================================\n\n";
}

int main() {
    testOlfactoryGenes();
    return 0;
}
