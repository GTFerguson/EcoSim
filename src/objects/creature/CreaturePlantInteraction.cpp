/**
 * @file CreaturePlantInteraction.cpp
 * @brief Plant interaction functionality extracted from Creature class
 *
 * This module handles plant feeding, burr attachment/detachment, and zoochory
 * (seed dispersal via gut passage).
 */

#include "objects/creature/CreaturePlantInteraction.hpp"
#include "objects/creature/creature.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <cmath>
#include <algorithm>

namespace CreaturePlantInteraction {

//============================================================================
//  Plant Feeding
//============================================================================

EcoSim::Genetics::FeedingResult eatPlant(
    Creature& creature,
    EcoSim::Genetics::Plant& plant,
    EcoSim::Genetics::FeedingInteraction& feedingInteraction) {
    
    using namespace EcoSim::Genetics;
    
    const auto& phenotype = creature.getPhenotype();
    
    // Calculate hunger level (0-1, higher = more desperate)
    float hungerLevel = 1.0f - std::max(0.0f, std::min(1.0f,
        creature.getHunger() / Creature::RESOURCE_LIMIT));
    
    // Attempt the feeding interaction
    FeedingResult result = feedingInteraction.attemptToEatPlant(
        phenotype, plant, hungerLevel
    );
    
    if (result.success) {
        // Apply nutrition gained
        float newHunger = creature.getHunger() + result.nutritionGained;
        if (newHunger > Creature::RESOURCE_LIMIT) {
            newHunger = Creature::RESOURCE_LIMIT;
        }
        creature.setHunger(newHunger);
        
        // Apply damage from plant defenses (damage costs energy to heal)
        float totalDamage = result.damageReceived;
        creature.setHunger(creature.getHunger() - totalDamage * DAMAGE_HUNGER_COST);
        
        // Apply damage to the plant
        plant.takeDamage(result.plantDamage);
        
        // Note: Seed consumption is handled by the caller (Creature::eatPlant)
        // which has access to _gutSeeds and can call consumeSeeds()
    }
    
    return result;
}

bool canEatPlant(
    const Creature& creature,
    const EcoSim::Genetics::Plant& plant,
    EcoSim::Genetics::FeedingInteraction& feedingInteraction) {
    
    const auto& phenotype = creature.getPhenotype();
    
    // For plants on the same tile as creature, use distance 0
    // Plant.getX()/getY() may not be set correctly for tile-stored plants
    // The calling code in findGeneticsPlants already verifies the plant is reachable
    float distance = 0.0f;
    if (plant.getX() >= 0 && plant.getY() >= 0) {
        // Only calculate distance if plant has valid coordinates
        distance = creature.calculateDistance(plant.getX(), plant.getY());
    }
    
    // Detection check - with distance 0, should always pass
    bool canDetect = feedingInteraction.canDetectPlant(phenotype, plant, distance);
    if (!canDetect) {
        return false;
    }
    
    return feedingInteraction.canOvercomeDefenses(phenotype, plant);
}

float getPlantDetectionRange(const Creature& creature) {
    using namespace EcoSim::Genetics;
    
    const auto& phenotype = creature.getPhenotype();
    
    // Get sense enhancement traits
    float colorVision = phenotype.hasTrait(UniversalGenes::COLOR_VISION) ?
                        phenotype.getTrait(UniversalGenes::COLOR_VISION) : 0.5f;
    float scentDetection = phenotype.hasTrait(UniversalGenes::SCENT_DETECTION) ?
                           phenotype.getTrait(UniversalGenes::SCENT_DETECTION) : 0.5f;
    
    // Base range from sight, enhanced by color vision and scent
    float baseRange = static_cast<float>(creature.getSightRange());
    float enhancement = 1.0f + (colorVision * COLOR_VISION_RANGE_BONUS) + 
                        (scentDetection * SCENT_DETECTION_RANGE_BONUS);
    
    return baseRange * enhancement;
}

//============================================================================
//  Burr Management (External Seed Dispersal)
//============================================================================

void attachBurr(
    Creature& creature,
    const EcoSim::Genetics::Plant& plant,
    EcoSim::Genetics::SeedDispersal& seedDispersal,
    std::vector<std::tuple<int, int, int, int>>& attachedBurrs) {
    
    const auto& phenotype = creature.getPhenotype();
    
    // Check if burr will attach based on plant hook strength and creature fur
    if (seedDispersal.willBurrAttach(plant, phenotype)) {
        // Store burr info: (dispersal strategy as int, originX, originY, ticksAttached)
        attachedBurrs.push_back(std::make_tuple(
            static_cast<int>(plant.getPrimaryDispersalStrategy()),
            plant.getX(),
            plant.getY(),
            0  // Just attached
        ));
    }
}

std::vector<EcoSim::Genetics::DispersalEvent> detachBurrs(
    const Creature& creature,
    EcoSim::Genetics::SeedDispersal& seedDispersal,
    std::vector<std::tuple<int, int, int, int>>& attachedBurrs) {
    
    using namespace EcoSim::Genetics;
    
    std::vector<DispersalEvent> events;
    const auto& phenotype = creature.getPhenotype();
    
    auto it = attachedBurrs.begin();
    while (it != attachedBurrs.end()) {
        int ticksAttached = std::get<3>(*it);
        
        // Check if burr detaches
        if (seedDispersal.willBurrDetach(phenotype, ticksAttached)) {
            // Create dispersal event at current creature location
            DispersalEvent event;
            event.originX = std::get<1>(*it);
            event.originY = std::get<2>(*it);
            event.targetX = creature.tileX();
            event.targetY = creature.tileY();
            event.method = static_cast<DispersalStrategy>(std::get<0>(*it));
            event.disperserInfo = "creature_burr_detach";
            event.seedViability = BURR_SEED_VIABILITY;
            
            events.push_back(event);
            it = attachedBurrs.erase(it);
        } else {
            // Increment ticks attached
            std::get<3>(*it) = ticksAttached + 1;
            ++it;
        }
    }
    
    return events;
}

bool hasBurrs(const std::vector<std::tuple<int, int, int, int>>& attachedBurrs) {
    return !attachedBurrs.empty();
}

std::vector<EcoSim::Genetics::DispersalEvent> getPendingBurrDispersal(
    const Creature& creature,
    const std::vector<std::tuple<int, int, int, int>>& attachedBurrs) {
    
    using namespace EcoSim::Genetics;
    
    std::vector<DispersalEvent> events;
    
    for (const auto& burr : attachedBurrs) {
        DispersalEvent event;
        event.originX = std::get<1>(burr);
        event.originY = std::get<2>(burr);
        event.targetX = creature.tileX();  // Current creature position
        event.targetY = creature.tileY();
        event.method = static_cast<DispersalStrategy>(std::get<0>(burr));
        event.disperserInfo = "creature_burr_pending";
        event.seedViability = BURR_SEED_VIABILITY;
        
        events.push_back(event);
    }
    
    return events;
}

//============================================================================
//  Zoochory (Gut Seed Dispersal)
//============================================================================

void consumeSeeds(
    const Creature& creature,
    const EcoSim::Genetics::Plant& plant,
    int count,
    float viability,
    std::vector<std::tuple<int, float, int>>& gutSeeds) {
    
    using namespace EcoSim::Genetics;
    
    const auto& phenotype = creature.getPhenotype();
    
    // Calculate gut transit time from phenotype
    float gutTransit = phenotype.hasTrait(UniversalGenes::GUT_TRANSIT_TIME) ?
                       phenotype.getTrait(UniversalGenes::GUT_TRANSIT_TIME) : 
                       DEFAULT_GUT_TRANSIT_HOURS;
    
    // Convert hours to ticks
    int transitTicks = static_cast<int>(gutTransit * TICKS_PER_HOUR);
    
    // Encode origin position as single int for storage
    int encodedOrigin = plant.getX() * 10000 + plant.getY();
    
    // Add each seed to gut
    for (int i = 0; i < count; ++i) {
        gutSeeds.push_back(std::make_tuple(encodedOrigin, viability, transitTicks));
    }
}

std::vector<EcoSim::Genetics::DispersalEvent> processGutSeeds(
    const Creature& creature,
    int ticksElapsed,
    std::vector<std::tuple<int, float, int>>& gutSeeds) {
    
    using namespace EcoSim::Genetics;
    
    std::vector<DispersalEvent> events;
    const auto& phenotype = creature.getPhenotype();
    
    auto it = gutSeeds.begin();
    while (it != gutSeeds.end()) {
        int ticksRemaining = std::get<2>(*it) - ticksElapsed;
        
        if (ticksRemaining <= 0) {
            // Seed is ready to be dispersed
            int encodedOrigin = std::get<0>(*it);
            float viability = std::get<1>(*it);
            
            // Decode origin position
            int originX = encodedOrigin / 10000;
            int originY = encodedOrigin % 10000;
            
            // Create dispersal event at current creature location
            DispersalEvent event;
            event.originX = originX;
            event.originY = originY;
            event.targetX = creature.tileX();
            event.targetY = creature.tileY();
            event.method = DispersalStrategy::ANIMAL_FRUIT;
            event.disperserInfo = "creature_gut_passage";
            
            // Viability affected by gut passage
            // Optimal transit time (4-12 hours) can improve viability through scarification
            float transitHours = phenotype.hasTrait(UniversalGenes::GUT_TRANSIT_TIME) ?
                                 phenotype.getTrait(UniversalGenes::GUT_TRANSIT_TIME) : 
                                 DEFAULT_GUT_TRANSIT_HOURS;
            
            if (transitHours >= 4.0f && transitHours <= 12.0f) {
                viability = std::min(1.0f, viability * GUT_SEED_SCARIFICATION_BONUS);
            } else if (transitHours > 12.0f) {
                viability *= GUT_SEED_ACID_DAMAGE;
            }
            
            event.seedViability = viability;
            events.push_back(event);
            
            it = gutSeeds.erase(it);
        } else {
            // Update remaining time
            std::get<2>(*it) = ticksRemaining;
            ++it;
        }
    }
    
    return events;
}

bool hasGutSeeds(const std::vector<std::tuple<int, float, int>>& gutSeeds) {
    return !gutSeeds.empty();
}

} // namespace CreaturePlantInteraction
