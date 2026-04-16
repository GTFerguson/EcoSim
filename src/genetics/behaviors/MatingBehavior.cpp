#include "genetics/behaviors/MatingBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/interfaces/ILifecycle.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "world/SpatialIndex.hpp"
#include "objects/creature/creature.hpp"
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

bool MatingBehavior::isApplicable(const Organism& organism,
                                   const BehaviorContext& ctx) const {
    if (!isReadyToMate(organism))       return false;
    if (!isMature(organism))            return false;
    if (!hasResourcesToBread(organism)) return false;
    return true;
}

float MatingBehavior::getPriority(const Organism& organism) const {
    float mateValue = getMateValue(organism);

    float priority = BASE_PRIORITY;

    // Normalise the [MATE_THRESHOLD..1.0] band onto [0..1] so a fully
    // saturated mating urge actually reaches the full priority boost.
    // Without normalisation, urgency tops out at (1 - MATE_THRESHOLD)
    // and mating loses every tie against feeding/thirst at full priority.
    const float span = std::max(0.0001f, 1.0f - MATE_THRESHOLD);
    float mateUrgency = std::max(0.0f, (mateValue - MATE_THRESHOLD) / span);
    mateUrgency = std::min(1.0f, mateUrgency);
    priority += mateUrgency * MAX_PRIORITY_BOOST;

    return std::min(priority, BASE_PRIORITY + MAX_PRIORITY_BOOST);
}

BehaviorResult MatingBehavior::execute(Organism& organism,
                                        BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = true;
    result.completed = false;
    result.energyCost = BREED_COST;

    if (!ctx.world) {
        result.debugInfo = "No world access - cannot search for mate";
        return result;
    }

    Organism* mate = findMate(organism, ctx);
    if (!mate) {
        result.debugInfo = "No compatible mate found in range";
        return result;
    }

    float fitness = checkFitness(organism, *mate);
    if (fitness <= 0.0f) {
        result.debugInfo = "Potential mate has incompatible fitness";
        return result;
    }

    float distance = calculateDistance(organism, *mate);
    if (distance > 2.5f) {
        // Step toward the mate. Without actual movement, "found mate but
        // too far" loops forever and births never happen at scale.
        float dx = mate->getWorldX() - organism.getWorldX();
        float dy = mate->getWorldY() - organism.getWorldY();
        if (distance > 0.01f) {
            float speed = getTraitSafe(organism.getPhenotype(),
                UniversalGenes::LOCOMOTION, 0.3f);
            speed = std::max(0.1f, speed * 0.5f);
            float moveAmount = std::min(speed, distance);
            float newX = organism.getWorldX() + (dx / distance) * moveAmount;
            float newY = organism.getWorldY() + (dy / distance) * moveAmount;
            newX = std::max(0.0f,
                std::min(static_cast<float>(ctx.worldCols) - 0.01f, newX));
            newY = std::max(0.0f,
                std::min(static_cast<float>(ctx.worldRows) - 0.01f, newY));
            organism.setWorldPosition(newX, newY);
        }

        std::ostringstream ss;
        ss << "Mate found at distance " << distance << ", moving toward";
        result.debugInfo = ss.str();
        return result;
    }

    // Close enough to mate. Delegate to Organism::reproduce which handles
    // the genome crossover + construction. The returned offspring is
    // stashed on the parent for the main loop to pick up.
    auto offspring = organism.reproduce(mate);
    if (offspring) {
        organism.setPendingOffspring(std::move(offspring));
        // Both parents reset to 0 mate drive after a successful breed.
        if (organism.reproduction()) organism.reproduction()->mate = 0.0f;
        if (mate->reproduction())    mate->reproduction()->mate    = 0.0f;

        result.completed = true;
        std::ostringstream ss;
        ss << "Mating successful with fitness " << fitness;
        result.debugInfo = ss.str();
    } else {
        result.debugInfo = "Mating failed - reproduce() returned nullptr";
    }

    return result;
}

float MatingBehavior::getEnergyCost(const Organism& organism) const {
    return BREED_COST;
}

void MatingBehavior::setOffspringCallback(OffspringCallback callback) {
    offspringCallback_ = std::move(callback);
}

bool MatingBehavior::isReadyToMate(const Organism& organism) const {
    float mateValue = getMateValue(organism);
    return mateValue >= MATE_THRESHOLD;
}

float MatingBehavior::getMateValue(const Organism& organism) const {
    return organism.getPhenotype().getOrganismState().reproductive_urge;
}

float MatingBehavior::checkFitness(const Organism& seeker,
                                    const Organism& candidate) const {
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

Organism* MatingBehavior::findMate(const Organism& seeker,
                                           const BehaviorContext& ctx) const {
    if (!ctx.creatureIndex) return nullptr;

    const float sightRange = getTraitSafe(seeker.getPhenotype(),
                                          UniversalGenes::SIGHT_RANGE, 0.0f);
    if (sightRange <= 0.0f) return nullptr;

    const float cx = seeker.getWorldX();
    const float cy = seeker.getWorldY();

    // Query nearby creatures via the spatial index. Returns Creature*
    // which we up-cast to Organism* through the public inheritance.
    auto candidates = ctx.creatureIndex->queryRadius(cx, cy, sightRange);
    for (::Creature* c : candidates) {
        if (!c) continue;
        Organism* candidate = static_cast<Organism*>(c);
        if (candidate == &seeker) continue;
        if (!candidate->isAlive()) continue;
        if (!candidate->canReproduce()) continue;
        if (!seeker.isCompatibleWith(*candidate)) continue;
        return candidate;
    }
    return nullptr;
}

bool MatingBehavior::isMature(const Organism& organism) const {
    // Use the organism's size-based maturity flag (currentSize >= 0.5 * maxSize).
    // This aligns with canReproduce()'s maturity check and with Organism::grow's
    // progression criterion. The prior age-based check was too strict for
    // the sim's tick-to-lifespan ratio.
    return organism.isMature();
}

bool MatingBehavior::hasResourcesToBread(const Organism& organism) const {
    // Lower bound check — breeding uses BREED_COST (~3), so the creature
    // should have at least that much hunger headroom. MIN_HUNGER_TO_BREED
    // is 5.0 on a 0-10 scale, so 0.5 of energy_level. Relaxed to 0.3 since
    // the feeding loop tends to keep creatures around 0.2-0.4 rather than
    // fully topped up.
    return organism.getPhenotype().getOrganismState().energy_level >= 0.3f;
}

float MatingBehavior::calculateGeneticSimilarity(const Organism& org1,
                                                  const Organism& org2) const {
    const Genome& genome1 = org1.getGenome();
    const Genome& genome2 = org2.getGenome();
    
    return genome1.compare(genome2);
}

float MatingBehavior::calculateDistance(const Organism& org1,
                                         const Organism& org2) const {
    float dx = org1.getWorldX() - org2.getWorldX();
    float dy = org1.getWorldY() - org2.getWorldY();
    return std::sqrt(dx * dx + dy * dy);
}

std::unique_ptr<Genome> MatingBehavior::createOffspringGenome(const Organism& parent1,
                                                               const Organism& parent2) const {
    const Genome& genome1 = parent1.getGenome();
    const Genome& genome2 = parent2.getGenome();
    
    Genome crossed = Genome::crossover(genome1, genome2, 0.5f);
    
    crossed.mutate(0.05f, registry_.getAllDefinitions());
    
    return std::make_unique<Genome>(std::move(crossed));
}

unsigned int MatingBehavior::getOrganismId(const Organism& organism) const {
    const Genome& genome = organism.getGenome();
    return static_cast<unsigned int>(std::hash<const Genome*>{}(&genome));
}

} // namespace Genetics
} // namespace EcoSim
