#include "genetics/behaviors/MatingBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"
#include "genetics/interfaces/IPositionable.hpp"
#include "genetics/interfaces/ILifecycle.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include <cmath>
#include <sstream>
#include <functional>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

MatingBehavior::MatingBehavior(PerceptionSystem& perception, const GeneRegistry& registry)
    : perception_(perception)
    , registry_(registry)
{
}

std::string MatingBehavior::getId() const {
    return "mating";
}

bool MatingBehavior::isApplicable(const IGeneticOrganism& organism,
                                   const BehaviorContext& ctx) const {
    if (!isReadyToMate(organism)) {
        return false;
    }
    
    if (!isMature(organism)) {
        return false;
    }
    
    if (!hasResourcesToBread(organism)) {
        return false;
    }
    
    return true;
}

float MatingBehavior::getPriority(const IGeneticOrganism& organism) const {
    float mateValue = getMateValue(organism);
    
    float priority = BASE_PRIORITY;
    
    float mateUrgency = std::max(0.0f, mateValue - MATE_THRESHOLD);
    priority += mateUrgency * MAX_PRIORITY_BOOST;
    
    return std::min(priority, BASE_PRIORITY + MAX_PRIORITY_BOOST);
}

BehaviorResult MatingBehavior::execute(IGeneticOrganism& organism,
                                        BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = true;
    result.completed = false;
    result.energyCost = BREED_COST;
    
    if (!ctx.world) {
        result.debugInfo = "No world access - cannot search for mate";
        return result;
    }
    
    IGeneticOrganism* mate = findMate(organism, ctx);
    
    if (!mate) {
        result.debugInfo = "No compatible mate found in range";
        result.completed = false;
        return result;
    }
    
    float fitness = checkFitness(organism, *mate);
    
    if (fitness <= 0.0f) {
        result.debugInfo = "Potential mate has incompatible fitness";
        result.completed = false;
        return result;
    }
    
    float distance = calculateDistance(organism, *mate);
    if (distance > 1.5f) {
        std::ostringstream ss;
        ss << "Mate found at distance " << distance << ", moving toward";
        result.debugInfo = ss.str();
        result.completed = false;
        return result;
    }
    
    auto offspringGenome = createOffspringGenome(organism, *mate);
    
    if (offspringCallback_ && offspringGenome) {
        result.completed = true;
        
        std::ostringstream ss;
        ss << "Mating successful with fitness " << fitness;
        result.debugInfo = ss.str();
    } else {
        result.debugInfo = "Mating failed - no offspring callback or genome creation failed";
        result.completed = false;
    }
    
    return result;
}

float MatingBehavior::getEnergyCost(const IGeneticOrganism& organism) const {
    return BREED_COST;
}

void MatingBehavior::setOffspringCallback(OffspringCallback callback) {
    offspringCallback_ = std::move(callback);
}

bool MatingBehavior::isReadyToMate(const IGeneticOrganism& organism) const {
    float mateValue = getMateValue(organism);
    return mateValue >= MATE_THRESHOLD;
}

float MatingBehavior::getMateValue(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    if (phenotype.hasTrait(UniversalGenes::MATE_THRESHOLD)) {
        float threshold = phenotype.getTrait(UniversalGenes::MATE_THRESHOLD);
        return threshold;
    }
    
    return 0.0f;
}

float MatingBehavior::checkFitness(const IGeneticOrganism& seeker,
                                    const IGeneticOrganism& candidate) const {
    const Phenotype& seekerPhenotype = seeker.getPhenotype();
    float sightRange = getTraitSafe(seekerPhenotype, UniversalGenes::SIGHT_RANGE, 0.0f);
    
    float distance = calculateDistance(seeker, candidate);
    float proximity = 1.0f - std::min(1.0f, distance / sightRange);
    
    float similarity = calculateGeneticSimilarity(seeker, candidate);
    
    if (similarity > IDEAL_SIMILARITY) {
        float excess = similarity - IDEAL_SIMILARITY;
        similarity -= std::pow(excess, SIMILARITY_PENALTY_EXPONENT);
    }
    
    return (proximity / 2.0f) + similarity;
}

IGeneticOrganism* MatingBehavior::findMate(const IGeneticOrganism& seeker,
                                           const BehaviorContext& ctx) const {
    return nullptr;
}

bool MatingBehavior::isMature(const IGeneticOrganism& organism) const {
    const ILifecycle* lifecycle = dynamic_cast<const ILifecycle*>(&organism);
    if (!lifecycle) {
        return true;  // Assume mature if no lifecycle info available
    }
    
    float ageNormalized = lifecycle->getAgeNormalized();
    return ageNormalized >= MATURITY_AGE_RATIO;
}

bool MatingBehavior::hasResourcesToBread(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    float hunger = getTraitSafe(phenotype, UniversalGenes::HUNGER_THRESHOLD, 0.0f);
    
    return hunger >= MIN_HUNGER_TO_BREED;
}

float MatingBehavior::calculateGeneticSimilarity(const IGeneticOrganism& org1,
                                                  const IGeneticOrganism& org2) const {
    const Genome& genome1 = org1.getGenome();
    const Genome& genome2 = org2.getGenome();
    
    return genome1.compare(genome2);
}

float MatingBehavior::calculateDistance(const IGeneticOrganism& org1,
                                         const IGeneticOrganism& org2) const {
    const auto* pos1 = dynamic_cast<const IPositionable*>(&org1);
    const auto* pos2 = dynamic_cast<const IPositionable*>(&org2);
    
    if (pos1 && pos2) {
        float dx = static_cast<float>(pos1->getX() - pos2->getX());
        float dy = static_cast<float>(pos1->getY() - pos2->getY());
        return std::sqrt(dx * dx + dy * dy);
    }
    
    return 0.0f;
}

std::unique_ptr<Genome> MatingBehavior::createOffspringGenome(const IGeneticOrganism& parent1,
                                                               const IGeneticOrganism& parent2) const {
    const Genome& genome1 = parent1.getGenome();
    const Genome& genome2 = parent2.getGenome();
    
    Genome crossed = Genome::crossover(genome1, genome2, 0.5f);
    
    crossed.mutate(0.05f, registry_.getAllDefinitions());
    
    return std::make_unique<Genome>(std::move(crossed));
}

unsigned int MatingBehavior::getOrganismId(const IGeneticOrganism& organism) const {
    const Genome& genome = organism.getGenome();
    return static_cast<unsigned int>(std::hash<const Genome*>{}(&genome));
}

} // namespace Genetics
} // namespace EcoSim
