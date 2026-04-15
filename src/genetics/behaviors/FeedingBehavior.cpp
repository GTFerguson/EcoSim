#include "genetics/behaviors/FeedingBehavior.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "world/world.hpp"
#include "world/tile.hpp"
#include <cmath>
#include <vector>
#include <sstream>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

FeedingBehavior::FeedingBehavior(FeedingInteraction& feeding, PerceptionSystem& perception)
    : feeding_(feeding)
    , perception_(perception)
{
}

std::string FeedingBehavior::getId() const {
    return "feeding";
}

bool FeedingBehavior::isApplicable(const Organism& organism,
                                    const BehaviorContext& ctx) const {
    // Check if organism can eat plants
    if (!canEatPlants(organism)) {
        return false;
    }
    
    // Check if organism is hungry (hunger below threshold)
    float hunger = getHungerLevel(organism);
    float threshold = getHungerThreshold(organism);
    
    // Hungry when current hunger is below threshold
    return hunger < threshold;
}

float FeedingBehavior::getPriority(const Organism& organism) const {
    float hunger = getHungerLevel(organism);
    float threshold = getHungerThreshold(organism);
    
    // Base priority is NORMAL (50)
    float priority = BASE_PRIORITY;
    
    // If hungry, increase priority based on how hungry
    if (hunger < threshold) {
        // Calculate hunger urgency (0 = at threshold, 1 = starving)
        float urgency = (threshold - hunger) / threshold;
        urgency = std::max(0.0f, std::min(1.0f, urgency));
        
        // Add up to MAX_PRIORITY_BOOST (25) based on urgency
        priority += urgency * MAX_PRIORITY_BOOST;
    }
    
    return priority;
}

BehaviorResult FeedingBehavior::execute(Organism& organism,
                                         BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = false;
    result.completed = false;
    result.energyCost = 0.0f;
    
    // Verify world access is available
    if (!ctx.world) {
        result.debugInfo = "No world access in context";
        return result;
    }
    
    // Get phenotype for feeding calculations
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Calculate detection range
    float detectionRange = getDetectionRange(organism);
    
    // Find nearest edible plant
    Plant* targetPlant = findNearestEdiblePlant(organism, ctx);
    
    if (!targetPlant) {
        result.executed = true;
        result.completed = false;
        result.energyCost = BASE_ENERGY_COST;
        result.debugInfo = "No edible plants in range";
        return result;
    }
    
    // Get target position
    int targetX = targetPlant->getX();
    int targetY = targetPlant->getY();
    
    // We need organism position to calculate distance
    // Since Organism doesn't expose position, we'll use perception system
    // to check if the plant is within eating range based on context
    
    // Check if adjacent to target (within eating distance)
    // For now, assume we can eat if we found the plant
    // In a complete implementation, we'd check actual distance
    
    // Get hunger level for feeding calculation
    float hungerLevel = 1.0f - getHungerLevel(organism);  // Convert to desperation (0=full, 1=starving)
    
    // Attempt to eat the plant
    FeedingResult feedingResult = feeding_.attemptToEatPlant(
        phenotype, *targetPlant, hungerLevel
    );
    
    if (feedingResult.success) {
        targetPlant->takeDamage(feedingResult.plantDamage);

        // @todo Caller should apply nutrition gain (feedingResult.nutritionGained)
        // to organism state after execute() returns. Behaviors should not mutate
        // organism needs directly.

        result.executed = true;
        result.completed = true;
        // Report nutrition as negative energy cost so the caller can apply the net effect
        result.energyCost = BASE_ENERGY_COST - feedingResult.nutritionGained;

        std::ostringstream ss;
        ss << "Fed on plant, gained " << feedingResult.nutritionGained
           << " nutrition, took " << feedingResult.damageReceived << " damage";
        result.debugInfo = ss.str();
    } else {
        result.executed = true;
        result.completed = false;
        result.energyCost = BASE_ENERGY_COST;
        result.debugInfo = "Feeding failed: " + feedingResult.description;
    }
    
    return result;
}

float FeedingBehavior::getEnergyCost(const Organism& organism) const {
    // Base cost modified by organism's metabolism
    const Phenotype& phenotype = organism.getPhenotype();
    float metabolism = getTraitSafe(phenotype, UniversalGenes::METABOLISM_RATE, 0.0f);
    
    return BASE_ENERGY_COST * (0.5f + metabolism);
}

bool FeedingBehavior::canEatPlants(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    float plantDigestion = getTraitSafe(phenotype,
                                         UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 
                                         0.0f);
    
    return plantDigestion > PLANT_DIGESTION_THRESHOLD;
}

Plant* FeedingBehavior::findNearestEdiblePlant(const Organism& organism,
                                                const BehaviorContext& ctx) const {
    // @todo Add spatial query through BehaviorContext::creatureIndex
    // Previously used IPositionable dynamic_cast and direct grid iteration.
    // Needs reimplementation using ctx.creatureIndex or a plant-specific
    // spatial query once the spatial index supports plant lookups.
    (void)organism;
    (void)ctx;
    return nullptr;
}

float FeedingBehavior::getHungerLevel(const Organism& organism) const {
    return organism.getPhenotype().getOrganismState().energy_level;
}

float FeedingBehavior::getHungerThreshold(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Try to get from phenotype
    if (phenotype.hasTrait(UniversalGenes::HUNGER_THRESHOLD)) {
        // Hunger threshold is typically 0-10 scale, normalize to 0-1
        float threshold = phenotype.getTrait(UniversalGenes::HUNGER_THRESHOLD);
        return threshold / 10.0f;
    }
    
    return DEFAULT_HUNGER_THRESHOLD;
}

float FeedingBehavior::getDetectionRange(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Base sight range
    float sightRange = getTraitSafe(phenotype, UniversalGenes::SIGHT_RANGE, 0.0f);
    
    // Bonuses from color vision and scent detection
    float colorVision = getTraitSafe(phenotype, UniversalGenes::COLOR_VISION, 0.0f);
    float scentDetection = getTraitSafe(phenotype, UniversalGenes::SCENT_DETECTION, 0.0f);
    
    // Visual and scent bonuses (same formula as FeedingInteraction)
    float visualBonus = colorVision * 0.3f * 100.0f;  // Up to 30 tiles for colorful plants
    float scentBonus = scentDetection * 0.5f * 100.0f;  // Up to 50 tiles via scent
    
    return sightRange + std::max(visualBonus, scentBonus);
}

float FeedingBehavior::calculateDistance(int x1, int y1, int x2, int y2) {
    float dx = static_cast<float>(x2 - x1);
    float dy = static_cast<float>(y2 - y1);
    return std::sqrt(dx * dx + dy * dy);
}

} // namespace Genetics
} // namespace EcoSim
