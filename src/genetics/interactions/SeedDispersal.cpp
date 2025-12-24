#include "genetics/interactions/SeedDispersal.hpp"
#include "logging/Logger.hpp"
#include <chrono>
#include <cmath>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructors
// ============================================================================

SeedDispersal::SeedDispersal() {
    // Initialize RNG with time-based seed
    auto seed = static_cast<unsigned int>(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    rng_.seed(seed);
}

SeedDispersal::SeedDispersal(unsigned int randomSeed) {
    rng_.seed(randomSeed);
}

// ============================================================================
// Strategy-specific dispersal methods
// ============================================================================

DispersalEvent SeedDispersal::disperseByGravity(const Plant& plant) const {
    DispersalEvent event;
    event.originX = plant.getX();
    event.originY = plant.getY();
    event.method = DispersalStrategy::GRAVITY;
    event.disperserInfo = "gravity";
    
    // Seed mass affects fall distance slightly
    float seedMass = plant.getSeedMass();
    
    // Heavier seeds fall closer, lighter seeds may roll/bounce a bit
    float maxDistance = GRAVITY_MAX_DISTANCE - (seedMass * 1.5f);
    maxDistance = std::max(GRAVITY_BASE_DISTANCE, maxDistance);
    
    // Generate random offset
    auto offset = generateRandomOffset(maxDistance);
    event.targetX = event.originX + offset.first;
    event.targetY = event.originY + offset.second;
    
    // High viability for gravity dispersal (minimal stress)
    event.seedViability = 0.9f + (plant.getSeedCoatDurability() * 0.1f);
    
    return event;
}

DispersalEvent SeedDispersal::disperseByWind(
    const Plant& plant,
    float windSpeed,
    float windDirection
) const {
    DispersalEvent event;
    event.originX = plant.getX();
    event.originY = plant.getY();
    event.method = DispersalStrategy::WIND;
    event.disperserInfo = "wind";
    
    // Get seed properties
    float seedMass = plant.getSeedMass();
    float seedAerodynamics = plant.getSeedAerodynamics();
    
    // Calculate wind dispersal distance
    float distance = calculateWindDistance(seedMass, seedAerodynamics, windSpeed);
    
    // Add some randomness to direction (+-30 degrees)
    std::uniform_real_distribution<float> dirVariation(-30.0f, 30.0f);
    float actualDirection = windDirection + dirVariation(rng_);
    
    // Generate directional offset
    auto offset = generateDirectionalOffset(distance, actualDirection);
    event.targetX = event.originX + offset.first;
    event.targetY = event.originY + offset.second;
    
    // Viability slightly reduced by wind stress
    event.seedViability = 0.8f + (plant.getSeedCoatDurability() * 0.15f);
    
    // Longer flights reduce viability slightly
    float distancePenalty = std::min(0.2f, distance / 100.0f);
    event.seedViability -= distancePenalty;
    event.seedViability = std::max(0.5f, event.seedViability);
    
    return event;
}

DispersalEvent SeedDispersal::disperseByAnimalFruit(
    const Plant& plant,
    const Phenotype& creature,
    int creatureX,
    int creatureY
) const {
    DispersalEvent event;
    event.originX = plant.getX();
    event.originY = plant.getY();
    event.method = DispersalStrategy::ANIMAL_FRUIT;
    event.disperserInfo = "creature_gut";
    
    // Get creature properties
    float gutTransit = getTraitSafe(creature, UniversalGenes::GUT_TRANSIT_TIME, 6.0f);
    float locomotion = getTraitSafe(creature, UniversalGenes::LOCOMOTION, 1.0f);
    float seedDestruction = getTraitSafe(creature, UniversalGenes::SEED_DESTRUCTION_RATE, 0.5f);
    
    // Calculate dispersal distance based on transit time and movement speed
    float transitTicks = gutTransit * 10.0f;  // Convert hours to ticks
    float dispersalDistance = locomotion * transitTicks;
    
    // Generate random offset from creature's current position
    auto offset = generateRandomOffset(dispersalDistance * 0.3f);  // Some randomness in final position
    
    // Target is near creature's position (after gut passage, they defecate somewhere)
    event.targetX = creatureX + offset.first;
    event.targetY = creatureY + offset.second;
    
    // Viability depends on seed coat vs gut destruction
    float seedCoat = plant.getSeedCoatDurability();
    float survivalBase = seedCoat * (1.0f - seedDestruction);
    
    // Scarification can help viability in optimal transit range
    float scarificationBonus = 0.0f;
    if (gutTransit >= 4.0f && gutTransit <= 12.0f) {
        scarificationBonus = 0.15f;  // Optimal transit time for scarification
    } else if (gutTransit > 12.0f) {
        scarificationBonus = -0.1f;  // Too long, acid damage
    }
    
    event.seedViability = std::max(0.0f, std::min(1.0f, survivalBase + scarificationBonus));
    
    return event;
}

DispersalEvent SeedDispersal::disperseByAnimalBurr(
    const Plant& plant,
    const Phenotype& creature,
    int creatureX,
    int creatureY
) const {
    DispersalEvent event;
    event.originX = plant.getX();
    event.originY = plant.getY();
    event.method = DispersalStrategy::ANIMAL_BURR;
    event.disperserInfo = "creature_fur";
    
    // Get creature properties
    float groomingFrequency = getTraitSafe(creature, UniversalGenes::GROOMING_FREQUENCY, 0.5f);
    float locomotion = getTraitSafe(creature, UniversalGenes::LOCOMOTION, 1.0f);
    float hookStrength = plant.getSeedHookStrength();
    
    // Calculate expected carry distance
    float carryTime = (1.0f - groomingFrequency) * hookStrength * 50.0f;  // Ticks carried
    float distance = locomotion * carryTime;
    
    // Generate offset based on expected distance
    auto offset = generateRandomOffset(distance * 0.5f);
    
    // Target is near creature's current position (detachment point)
    event.targetX = creatureX + offset.first;
    event.targetY = creatureY + offset.second;
    
    // High viability for burrs (no gut passage damage)
    event.seedViability = 0.85f + (plant.getSeedCoatDurability() * 0.1f);
    
    return event;
}

DispersalEvent SeedDispersal::disperseByExplosive(const Plant& plant) const {
    DispersalEvent event;
    event.originX = plant.getX();
    event.originY = plant.getY();
    event.method = DispersalStrategy::EXPLOSIVE;
    event.disperserInfo = "explosive_pod";
    
    // Get plant's explosive force
    const Phenotype& phenotype = plant.getPhenotype();
    float explosiveForce = 0.5f;
    if (phenotype.hasTrait(UniversalGenes::EXPLOSIVE_POD_FORCE)) {
        explosiveForce = phenotype.getTrait(UniversalGenes::EXPLOSIVE_POD_FORCE);
    }
    
    // Calculate launch distance
    float distance = EXPLOSIVE_BASE_DISTANCE + (explosiveForce * EXPLOSIVE_FORCE_FACTOR);
    
    // Random direction (360 degrees)
    std::uniform_real_distribution<float> dirDist(0.0f, 360.0f);
    float direction = dirDist(rng_);
    
    // Generate directional offset
    auto offset = generateDirectionalOffset(distance, direction);
    event.targetX = event.originX + offset.first;
    event.targetY = event.originY + offset.second;
    
    // Moderate viability - explosive force can damage some seeds
    event.seedViability = 0.7f + (plant.getSeedCoatDurability() * 0.2f);
    
    return event;
}

DispersalEvent SeedDispersal::disperseByVegetative(const Plant& plant) const {
    DispersalEvent event;
    event.originX = plant.getX();
    event.originY = plant.getY();
    event.method = DispersalStrategy::VEGETATIVE;
    event.disperserInfo = "runner";
    
    // Get runner production rate
    const Phenotype& phenotype = plant.getPhenotype();
    float runnerProduction = 0.5f;
    if (phenotype.hasTrait(UniversalGenes::RUNNER_PRODUCTION)) {
        runnerProduction = phenotype.getTrait(UniversalGenes::RUNNER_PRODUCTION);
    }
    
    // Calculate runner length
    float distance = VEGETATIVE_BASE_DISTANCE + 
                     (runnerProduction * (VEGETATIVE_MAX_DISTANCE - VEGETATIVE_BASE_DISTANCE));
    
    // Random direction
    std::uniform_real_distribution<float> dirDist(0.0f, 360.0f);
    float direction = dirDist(rng_);
    
    auto offset = generateDirectionalOffset(distance, direction);
    event.targetX = event.originX + offset.first;
    event.targetY = event.originY + offset.second;
    
    // Very high viability for vegetative (clone, not seed)
    event.seedViability = 0.95f;
    
    return event;
}

// ============================================================================
// Automatic dispersal
// ============================================================================

DispersalEvent SeedDispersal::disperse(
    const Plant& plant,
    const EnvironmentState* env
) const {
    DispersalStrategy strategy = plant.getPrimaryDispersalStrategy();
    DispersalEvent event;
    
    switch (strategy) {
        case DispersalStrategy::GRAVITY:
            event = disperseByGravity(plant);
            break;
            
        case DispersalStrategy::WIND: {
            float windSpeed = DEFAULT_WIND_SPEED;
            float windDirection = DEFAULT_WIND_DIRECTION;
            if (env) {
                // Extract wind data from environment if available
                // For now, use defaults or simple weather-based adjustment
                windSpeed = DEFAULT_WIND_SPEED * (1.0f + env->humidity * 0.3f);
            }
            event = disperseByWind(plant, windSpeed, windDirection);
            break;
        }
            
        case DispersalStrategy::ANIMAL_FRUIT:
            // Without a specific creature, simulate generic fruit dispersal
            // Seeds fall near parent but with higher viability potential
            {
                event = disperseByGravity(plant);
                event.method = DispersalStrategy::ANIMAL_FRUIT;
                event.disperserInfo = "uneaten_fruit";
            }
            break;
            
        case DispersalStrategy::ANIMAL_BURR:
            // Without a creature, burrs stay at plant
            {
                event.originX = plant.getX();
                event.originY = plant.getY();
                event.targetX = plant.getX();
                event.targetY = plant.getY();
                event.method = DispersalStrategy::ANIMAL_BURR;
                event.disperserInfo = "waiting_for_animal";
                event.seedViability = 0.9f;
            }
            break;
            
        case DispersalStrategy::EXPLOSIVE:
            event = disperseByExplosive(plant);
            break;
            
        case DispersalStrategy::VEGETATIVE:
            event = disperseByVegetative(plant);
            break;
            
        default:
            event = disperseByGravity(plant);
            break;
    }
    
    // Log the seed dispersal event
    logging::Logger::getInstance().seedDispersal(
        plant.getId(),
        event.disperserInfo,
        event.targetX,
        event.targetY,
        event.seedViability > 0.5f  // viable if >50% viability
    );
    
    return event;
}

// ============================================================================
// Burr attachment/detachment mechanics
// ============================================================================

bool SeedDispersal::willBurrAttach(
    const Plant& plant,
    const Phenotype& creature
) const {
    float hookStrength = plant.getSeedHookStrength();
    float furDensity = getTraitSafe(creature, UniversalGenes::FUR_DENSITY, 0.5f);
    float locomotion = getTraitSafe(creature, UniversalGenes::LOCOMOTION, 1.0f);
    
    // Base attachment probability
    float baseProb = BURR_ATTACH_BASE_PROB * hookStrength * furDensity;
    
    // Fast movement reduces attachment chance
    float speedReduction = 1.0f - (locomotion * 0.2f);
    speedReduction = std::max(0.5f, speedReduction);
    
    float attachProbability = baseProb * speedReduction;
    
    // Roll for attachment
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng_) < attachProbability;
}

bool SeedDispersal::willBurrDetach(
    const Phenotype& creature,
    int ticksAttached
) const {
    float groomingFrequency = getTraitSafe(creature, UniversalGenes::GROOMING_FREQUENCY, 0.5f);
    
    // Base detachment rate increases with time
    float naturalDetach = BURR_NATURAL_DETACH_RATE * static_cast<float>(ticksAttached);
    
    // Grooming increases detachment
    float groomingDetach = groomingFrequency * GROOMING_DETACH_FACTOR;
    
    float detachProbability = naturalDetach + groomingDetach;
    detachProbability = std::min(1.0f, detachProbability);
    
    // Roll for detachment
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng_) < detachProbability;
}

float SeedDispersal::calculateExpectedBurrDistance(
    const Plant& plant,
    const Phenotype& creature
) const {
    float hookStrength = plant.getSeedHookStrength();
    float groomingFrequency = getTraitSafe(creature, UniversalGenes::GROOMING_FREQUENCY, 0.5f);
    float locomotion = getTraitSafe(creature, UniversalGenes::LOCOMOTION, 1.0f);
    
    // Expected time attached (ticks)
    float expectedAttachTime = hookStrength * (1.0f - groomingFrequency) * 50.0f;
    
    // Distance = speed * time
    return locomotion * expectedAttachTime;
}

// ============================================================================
// Internal helper methods
// ============================================================================

float SeedDispersal::getTraitSafe(
    const Phenotype& phenotype,
    const char* traitName,
    float defaultValue
) const {
    if (phenotype.hasTrait(traitName)) {
        return phenotype.getTrait(traitName);
    }
    return defaultValue;
}

std::pair<int, int> SeedDispersal::generateRandomOffset(float maxDistance) const {
    // Random direction
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    float angle = angleDist(rng_);
    
    // Random distance (uniform within circle)
    std::uniform_real_distribution<float> distDist(0.0f, maxDistance);
    float distance = distDist(rng_);
    
    int dx = static_cast<int>(std::round(distance * std::cos(angle)));
    int dy = static_cast<int>(std::round(distance * std::sin(angle)));
    
    return {dx, dy};
}

std::pair<int, int> SeedDispersal::generateDirectionalOffset(
    float distance,
    float directionDegrees
) const {
    // Convert degrees to radians (0 = North, clockwise)
    float radians = (90.0f - directionDegrees) * 3.14159f / 180.0f;
    
    int dx = static_cast<int>(std::round(distance * std::cos(radians)));
    int dy = static_cast<int>(std::round(distance * std::sin(radians)));
    
    return {dx, dy};
}

float SeedDispersal::calculateWindDistance(
    float seedMass,
    float seedAerodynamics,
    float windSpeed
) const {
    // Light seeds with high aerodynamics travel far
    // Formula: distance = windSpeed * aerodynamics / sqrt(mass)
    
    // Avoid division by zero
    float effectiveMass = std::max(0.01f, seedMass);
    
    float distance = windSpeed * WIND_DISTANCE_FACTOR * seedAerodynamics / std::sqrt(effectiveMass);
    
    // Cap at maximum
    return std::min(WIND_MAX_DISTANCE, distance);
}

} // namespace Genetics
} // namespace EcoSim
