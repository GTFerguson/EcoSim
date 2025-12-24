#include "genetics/expression/EnergyBudget.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <algorithm>

namespace EcoSim {
namespace Genetics {

float EnergyBudget::calculateMaintenanceCost(const Genome& genome, const GeneRegistry& registry) const {
    float totalCost = 0.0f;
    
    // Iterate through all genes in the genome using getAllGenes()
    auto allGenes = genome.getAllGenes();
    
    for (const auto& geneRef : allGenes) {
        const Gene& gene = geneRef.get();
        const std::string& geneId = gene.getId();
        
        // Skip if gene definition doesn't exist in registry
        if (!registry.hasGene(geneId)) {
            continue;
        }
        
        const GeneDefinition& definition = registry.getDefinition(geneId);
        
        // Get expression level from allele (average of both alleles' expression strength)
        float expressionLevel = (gene.getAllele1().expression_strength +
                                 gene.getAllele2().expression_strength) / 2.0f;
        
        // Calculate this gene's maintenance cost
        float geneCost = definition.calculateMaintenanceCost(expressionLevel);
        totalCost += geneCost;
    }
    
    return totalCost;
}

float EnergyBudget::calculateSpecialistBonus(const Phenotype& phenotype) const {
    // Get digestion efficiency traits
    float plantDigestion = phenotype.getTrait(UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    float meatDigestion = phenotype.getTrait(UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    
    // Specialist bonus: up to 30% more extraction from preferred food
    // The more specialized (greater difference between plant and meat), the bigger the bonus
    float specialization = std::abs(plantDigestion - meatDigestion);
    float bonus = 1.0f + (specialization * 0.3f);  // 0% to 30% bonus
    
    return bonus;
}

float EnergyBudget::calculateMetabolicOverhead(const Phenotype& phenotype) const {
    int systemCount = 0;
    
    // Count active digestive systems (threshold: 0.3)
    // Each active system adds metabolic overhead for maintaining multiple enzyme pathways
    
    if (phenotype.getTrait(UniversalGenes::PLANT_DIGESTION_EFFICIENCY) > 0.3f) {
        systemCount++;
    }
    if (phenotype.getTrait(UniversalGenes::MEAT_DIGESTION_EFFICIENCY) > 0.3f) {
        systemCount++;
    }
    if (phenotype.getTrait(UniversalGenes::CELLULOSE_BREAKDOWN) > 0.3f) {
        systemCount++;
    }
    if (phenotype.getTrait(UniversalGenes::TOXIN_METABOLISM) > 0.3f) {
        systemCount++;
    }
    
    // 8% overhead per system, compounding
    // e.g., 1 system = 1.08x, 2 systems = 1.1664x, 3 systems = 1.2597x, 4 systems = 1.3605x
    float overhead = std::pow(1.08f, static_cast<float>(systemCount));
    
    return overhead;
}

EnergyState EnergyBudget::updateEnergy(const EnergyState& current, float income, float activity) const {
    EnergyState newState = current;
    
    // Calculate total expenditure
    newState.activityCost = activity;
    newState.totalExpenditure = newState.baseMetabolism + newState.maintenanceCost + activity;
    
    // Update energy level
    // Energy(t+1) = Energy(t) + Income - TotalExpenditure
    float netChange = income - newState.totalExpenditure;
    newState.currentEnergy = current.currentEnergy + netChange;
    
    // Clamp to valid range
    newState.currentEnergy = std::clamp(newState.currentEnergy, 0.0f, newState.maxEnergy);
    
    return newState;
}

bool EnergyBudget::isStarving(const EnergyState& state) const {
    // Organism is starving if energy ratio is at or below threshold
    return state.getEnergyRatio() <= starvationThreshold_;
}

bool EnergyBudget::canReproduce(const EnergyState& state, float reproductionCost) const {
    // Organism needs:
    // 1. Enough energy to pay reproduction cost
    // 2. Plus enough reserve to stay above 20% of maxEnergy (avoid immediate starvation)
    // 3. Plus 2x maintenance as additional reserve
    float minimumReserve = state.maxEnergy * 0.2f;  // Must keep at least 20% energy
    float maintenanceReserve = state.maintenanceCost * 2.0f;
    
    // After reproduction, energy remaining must be >= max(minimumReserve, maintenanceReserve)
    float requiredReserve = std::max(minimumReserve, maintenanceReserve);
    float requiredEnergy = reproductionCost + requiredReserve;
    
    return state.currentEnergy >= requiredEnergy;
}

} // namespace Genetics
} // namespace EcoSim
