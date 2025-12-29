#include "genetics/behaviors/ZoochoryBehavior.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/RandomEngine.hpp"
#include <sstream>
#include <functional>

namespace EcoSim {
namespace Genetics {

ZoochoryBehavior::ZoochoryBehavior(SeedDispersal& dispersal)
    : dispersal_(dispersal) {
}

std::string ZoochoryBehavior::getId() const {
    return "zoochory";
}

bool ZoochoryBehavior::isApplicable(const IGeneticOrganism& organism,
                                          const BehaviorContext& ctx) const {
    return true;
}

float ZoochoryBehavior::getPriority(const IGeneticOrganism& organism) const {
    return static_cast<float>(BehaviorPriority::IDLE);
}

BehaviorResult ZoochoryBehavior::execute(IGeneticOrganism& organism,
                                               BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = true;
    result.completed = true;
    result.energyCost = 0.0f;
    
    unsigned int organismId = getOrganismId(organism);
    
    std::ostringstream debugInfo;
    debugInfo << "Zoochory passive for organism " << organismId;
    
    bool hasBurrsPending = hasBurrs(organismId);
    auto gutIt = gutSeeds_.find(organismId);
    bool hasGutSeeds = (gutIt != gutSeeds_.end() && !gutIt->second.empty());
    
    if (hasBurrsPending || hasGutSeeds) {
        debugInfo << "; pending: ";
        if (hasBurrsPending) debugInfo << "burrs(epizoochory) ";
        if (hasGutSeeds) debugInfo << "gut_seeds(endozoochory:" << gutIt->second.size() << ")";
    }
    
    result.debugInfo = debugInfo.str();
    
    return result;
}

float ZoochoryBehavior::getEnergyCost(const IGeneticOrganism& organism) const {
    return 0.0f;
}

void ZoochoryBehavior::attachBurr(unsigned int organismId,
                                        int plantX, int plantY,
                                        int strategy) {
    attachedBurrs_[organismId].push_back(
        std::make_tuple(strategy, plantX, plantY, 0));
}

std::vector<DispersalEvent> ZoochoryBehavior::processOrganismSeeds(
    unsigned int organismId,
    int currentX, int currentY,
    int ticksElapsed) {
    
    std::vector<DispersalEvent> events;
    
    auto gutEvents = processGutSeeds(organismId, currentX, currentY);
    events.insert(events.end(), gutEvents.begin(), gutEvents.end());
    
    auto burrEvents = processBurrDetachment(organismId, currentX, currentY);
    events.insert(events.end(), burrEvents.begin(), burrEvents.end());
    
    return events;
}

bool ZoochoryBehavior::hasBurrs(unsigned int organismId) const {
    auto it = attachedBurrs_.find(organismId);
    if (it == attachedBurrs_.end()) {
        return false;
    }
    return !it->second.empty();
}

void ZoochoryBehavior::consumeSeeds(unsigned int organismId,
                                          int plantX, int plantY,
                                          int count, float viability) {
    int encodedOrigin = plantX * 10000 + plantY;
    
    for (int i = 0; i < count; ++i) {
        gutSeeds_[organismId].push_back(
            std::make_tuple(encodedOrigin, viability, GUT_TRANSIT_TICKS));
    }
}

void ZoochoryBehavior::clearOrganismData(unsigned int organismId) {
    attachedBurrs_.erase(organismId);
    gutSeeds_.erase(organismId);
}

std::vector<DispersalEvent> ZoochoryBehavior::processGutSeeds(
    unsigned int organismId,
    int currentX, int currentY) {
    
    std::vector<DispersalEvent> events;
    
    auto it = gutSeeds_.find(organismId);
    if (it == gutSeeds_.end()) {
        return events;
    }
    
    auto& seeds = it->second;
    auto seedIt = seeds.begin();
    
    while (seedIt != seeds.end()) {
        int ticksRemaining = std::get<2>(*seedIt) - 1;
        
        if (ticksRemaining <= 0) {
            int encodedOrigin = std::get<0>(*seedIt);
            float viability = std::get<1>(*seedIt);
            
            int originX = encodedOrigin / 10000;
            int originY = encodedOrigin % 10000;
            
            DispersalEvent event;
            event.originX = originX;
            event.originY = originY;
            event.targetX = currentX;
            event.targetY = currentY;
            event.method = DispersalStrategy::ANIMAL_FRUIT;
            event.disperserInfo = "endozoochory";
            event.seedViability = viability;
            
            events.push_back(event);
            seedIt = seeds.erase(seedIt);
        } else {
            std::get<2>(*seedIt) = ticksRemaining;
            ++seedIt;
        }
    }
    
    return events;
}

std::vector<DispersalEvent> ZoochoryBehavior::processBurrDetachment(
    unsigned int organismId,
    int currentX, int currentY) {
    
    std::vector<DispersalEvent> events;
    
    auto it = attachedBurrs_.find(organismId);
    if (it == attachedBurrs_.end()) {
        return events;
    }
    
    auto& burrs = it->second;
    auto burrIt = burrs.begin();
    
    while (burrIt != burrs.end()) {
        int ticksAttached = std::get<3>(*burrIt);
        
        float detachChance = BURR_DETACH_CHANCE + (ticksAttached * 0.001f);
        detachChance = std::min(1.0f, detachChance);
        
        if (RandomEngine::rollProbability(detachChance)) {
            int strategy = std::get<0>(*burrIt);
            int originX = std::get<1>(*burrIt);
            int originY = std::get<2>(*burrIt);
            
            DispersalEvent event;
            event.originX = originX;
            event.originY = originY;
            event.targetX = currentX;
            event.targetY = currentY;
            event.method = static_cast<DispersalStrategy>(strategy);
            event.disperserInfo = "epizoochory";
            event.seedViability = 0.85f;
            
            events.push_back(event);
            burrIt = burrs.erase(burrIt);
        } else {
            std::get<3>(*burrIt) = ticksAttached + 1;
            ++burrIt;
        }
    }
    
    return events;
}

unsigned int ZoochoryBehavior::getOrganismId(const IGeneticOrganism& organism) const {
    const Genome& genome = organism.getGenome();
    return static_cast<unsigned int>(std::hash<const Genome*>{}(&genome));
}

} // namespace Genetics
} // namespace EcoSim
