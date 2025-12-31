#include "genetics/behaviors/RestBehavior.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <cmath>
#include <sstream>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

std::string RestBehavior::getId() const {
    return "rest";
}

bool RestBehavior::isApplicable(const IGeneticOrganism& organism,
                                 const BehaviorContext& ctx) const {
    return isTired(organism);
}

float RestBehavior::getPriority(const IGeneticOrganism& organism) const {
    float fatigue = getFatigueLevel(organism);
    float threshold = getFatigueThreshold(organism);
    
    float priority = BASE_PRIORITY;
    
    if (fatigue > threshold) {
        float urgency = (fatigue - threshold) / (threshold * 2.0f);
        urgency = std::max(0.0f, std::min(1.0f, urgency));
        
        priority += urgency * MAX_PRIORITY_BOOST;
    }
    
    return priority;
}

BehaviorResult RestBehavior::execute(IGeneticOrganism& organism,
                                      BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = false;
    result.completed = false;
    result.energyCost = 0.0f;
    
    float fatigue = getFatigueLevel(organism);
    float threshold = getFatigueThreshold(organism);
    float recoveryRate = getRecoveryRate(organism);
    
    result.executed = true;
    result.energyCost = REST_ENERGY_COST;
    
    float newFatigue = fatigue - recoveryRate;
    
    if (newFatigue <= threshold) {
        result.completed = true;
        std::ostringstream ss;
        ss << "Rest complete, fatigue reduced from " << fatigue 
           << " to " << newFatigue << " (threshold: " << threshold << ")";
        result.debugInfo = ss.str();
    } else {
        result.completed = false;
        std::ostringstream ss;
        ss << "Resting, fatigue: " << fatigue << " -> " << newFatigue 
           << " (threshold: " << threshold << ")";
        result.debugInfo = ss.str();
    }
    
    return result;
}

float RestBehavior::getEnergyCost(const IGeneticOrganism& organism) const {
    return REST_ENERGY_COST;
}

float RestBehavior::getFatigueLevel(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    float threshold = getTraitSafe(phenotype,
                                    UniversalGenes::FATIGUE_THRESHOLD, 
                                    DEFAULT_FATIGUE_THRESHOLD);
    
    return threshold * 1.5f;
}

float RestBehavior::getFatigueThreshold(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    if (phenotype.hasTrait(UniversalGenes::FATIGUE_THRESHOLD)) {
        return phenotype.getTrait(UniversalGenes::FATIGUE_THRESHOLD);
    }
    
    return DEFAULT_FATIGUE_THRESHOLD;
}

float RestBehavior::getRecoveryRate(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    float metabolism = getTraitSafe(phenotype,
                                     UniversalGenes::METABOLISM_RATE,
                                     0.5f);
    
    float regeneration = getTraitSafe(phenotype,
                                       UniversalGenes::REGENERATION_RATE,
                                       0.5f);
    
    return DEFAULT_RECOVERY_RATE * (1.0f + regeneration) * metabolism;
}

bool RestBehavior::isTired(const IGeneticOrganism& organism) const {
    float fatigue = getFatigueLevel(organism);
    float threshold = getFatigueThreshold(organism);
    
    return fatigue > threshold;
}

} // namespace Genetics
} // namespace EcoSim
