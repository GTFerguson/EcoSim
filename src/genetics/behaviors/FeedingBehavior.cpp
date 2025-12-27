#include "genetics/behaviors/FeedingBehavior.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <cmath>
#include <vector>
#include <sstream>

namespace EcoSim {
namespace Genetics {

FeedingBehavior::FeedingBehavior(FeedingInteraction& feeding, PerceptionSystem& perception)
    : feeding_(feeding)
    , perception_(perception)
{
}

std::string FeedingBehavior::getId() const {
    return "feeding";
}

bool FeedingBehavior::isApplicable(const IGeneticOrganism& organism,
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

float FeedingBehavior::getPriority(const IGeneticOrganism& organism) const {
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

BehaviorResult FeedingBehavior::execute(IGeneticOrganism& organism,
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
    // Since IGeneticOrganism doesn't expose position, we'll use perception system
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
        // Apply damage to the plant
        targetPlant->takeDamage(feedingResult.plantDamage);
        
        result.executed = true;
        result.completed = true;
        result.energyCost = BASE_ENERGY_COST;
        
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

float FeedingBehavior::getEnergyCost(const IGeneticOrganism& organism) const {
    // Base cost modified by organism's metabolism
    const Phenotype& phenotype = organism.getPhenotype();
    float metabolism = getTraitSafe(phenotype, UniversalGenes::METABOLISM_RATE, 0.5f);
    
    return BASE_ENERGY_COST * (0.5f + metabolism);
}

bool FeedingBehavior::canEatPlants(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    float plantDigestion = getTraitSafe(phenotype,
                                         UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 
                                         0.0f);
    
    return plantDigestion > PLANT_DIGESTION_THRESHOLD;
}

Plant* FeedingBehavior::findNearestEdiblePlant(const IGeneticOrganism& organism,
                                                const BehaviorContext& ctx) const {
    if (!ctx.world) {
        return nullptr;
    }
    
    const Phenotype& phenotype = organism.getPhenotype();
    float detectionRange = getDetectionRange(organism);
    
    Plant* nearestPlant = nullptr;
    float nearestDistance = detectionRange + 1.0f;
    
    // Search the world for plants
    // Since we don't have organism position through IGeneticOrganism,
    // we'll search all tiles within world bounds
    // In a complete implementation, organism position would be provided
    
    // Get world dimensions from context
    int rows = ctx.worldRows;
    int cols = ctx.worldCols;
    
    // For now, we need to iterate through tiles to find plants
    // This is a simplified implementation - in practice, we'd use
    // spatial indexing or have organism position available
    
    // Access world's grid to search for plants
    // Note: This requires World to provide plant access
    // For now, return nullptr and document the limitation
    
    // The actual implementation would use IWorldQuery or similar
    // to get plants in radius:
    // auto plants = ctx.worldQuery->getPlantsInRadius(orgX, orgY, detectionRange);
    
    // Placeholder: The behavior is structured correctly but needs
    // proper world querying support
    
    return nullptr;
}

float FeedingBehavior::getHungerLevel(const IGeneticOrganism& organism) const {
    // Try to get hunger from phenotype traits
    // In a full implementation, OrganismState would track current hunger
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Use max_hunger trait as reference
    float maxHunger = getTraitSafe(phenotype, UniversalGenes::MAX_SIZE, 10.0f);
    
    // Without access to actual organism state, return a default
    // that indicates the organism is hungry (triggering applicability)
    // In practice, this would come from organism state
    return 0.3f;  // Default to 30% full (hungry)
}

float FeedingBehavior::getHungerThreshold(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Try to get from phenotype
    if (phenotype.hasTrait(UniversalGenes::HUNGER_THRESHOLD)) {
        // Hunger threshold is typically 0-10 scale, normalize to 0-1
        float threshold = phenotype.getTrait(UniversalGenes::HUNGER_THRESHOLD);
        return threshold / 10.0f;
    }
    
    return DEFAULT_HUNGER_THRESHOLD;
}

float FeedingBehavior::getDetectionRange(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Base sight range
    float sightRange = getTraitSafe(phenotype, UniversalGenes::SIGHT_RANGE, 50.0f);
    
    // Bonuses from color vision and scent detection
    float colorVision = getTraitSafe(phenotype, UniversalGenes::COLOR_VISION, 0.3f);
    float scentDetection = getTraitSafe(phenotype, UniversalGenes::SCENT_DETECTION, 0.5f);
    
    // Visual and scent bonuses (same formula as FeedingInteraction)
    float visualBonus = colorVision * 0.3f * 100.0f;  // Up to 30 tiles for colorful plants
    float scentBonus = scentDetection * 0.5f * 100.0f;  // Up to 50 tiles via scent
    
    return sightRange + std::max(visualBonus, scentBonus);
}

float FeedingBehavior::getTraitSafe(const Phenotype& phenotype,
                                     const std::string& traitName,
                                     float defaultValue) const {
    if (phenotype.hasTrait(traitName)) {
        return phenotype.getTrait(traitName);
    }
    return defaultValue;
}

float FeedingBehavior::calculateDistance(int x1, int y1, int x2, int y2) {
    float dx = static_cast<float>(x2 - x1);
    float dy = static_cast<float>(y2 - y1);
    return std::sqrt(dx * dx + dy * dy);
}

} // namespace Genetics
} // namespace EcoSim
