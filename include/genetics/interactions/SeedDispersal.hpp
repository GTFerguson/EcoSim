#pragma once

#include "genetics/expression/Phenotype.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <string>
#include <cmath>
#include <algorithm>
#include <random>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Represents a seed dispersal event from a plant
 * 
 * Contains information about where a seed originates, where it lands,
 * and the dispersal method used.
 */
struct DispersalEvent {
    int originX = 0;                              ///< Parent plant X location
    int originY = 0;                              ///< Parent plant Y location
    int targetX = 0;                              ///< Seed landing X location
    int targetY = 0;                              ///< Seed landing Y location
    float seedViability = 1.0f;                   ///< Probability of germination (0-1)
    DispersalStrategy method = DispersalStrategy::GRAVITY;  ///< How the seed was dispersed
    std::string disperserInfo;                    ///< "wind", "creature_id:123", etc.
    
    /**
     * @brief Calculate distance from origin to target
     * @return Euclidean distance in tiles
     */
    float getDistance() const {
        float dx = static_cast<float>(targetX - originX);
        float dy = static_cast<float>(targetY - originY);
        return std::sqrt(dx * dx + dy * dy);
    }
};

/**
 * @brief Handles seed dispersal mechanics for plants
 * 
 * This class implements the six dispersal strategies described in the design:
 * - GRAVITY: Heavy seeds fall near parent
 * - WIND: Light seeds with wings/parachutes travel on wind
 * - ANIMAL_FRUIT: Attractive fruit eaten, seeds pass through gut
 * - ANIMAL_BURR: Hooks attach to animal fur for transport
 * - EXPLOSIVE: Pod tension launches seeds ballistically
 * - VEGETATIVE: Runners/stolons create clonal spread
 * 
 * The dispersal strategy emerges from continuous physical properties of the seeds
 * rather than categorical gene values, creating realistic trade-offs and
 * evolutionary pathways.
 * 
 * Usage:
 * @code
 *   SeedDispersal dispersal;
 *   
 *   // Automatic dispersal based on plant's emergent strategy
 *   DispersalEvent event = dispersal.disperse(plant, &environment);
 *   
 *   // Or specific dispersal by animal
 *   DispersalEvent fruitEvent = dispersal.disperseByAnimalFruit(
 *       plant, creaturePhenotype, creatureX, creatureY
 *   );
 * @endcode
 * 
 * @see DispersalEvent
 * @see DispersalStrategy
 * @see Plant
 */
class SeedDispersal {
public:
    /**
     * @brief Default constructor
     */
    SeedDispersal();
    
    /**
     * @brief Constructor with random seed
     * @param randomSeed Seed for random number generator
     */
    explicit SeedDispersal(unsigned int randomSeed);
    
    /**
     * @brief Destructor
     */
    ~SeedDispersal() = default;
    
    // ========================================================================
    // Strategy-specific dispersal methods
    // ========================================================================
    
    /**
     * @brief Disperse seeds by gravity (fall near parent)
     * @param plant The parent plant
     * @return DispersalEvent with nearby landing location
     * 
     * Heavy seeds simply fall, landing within 1-3 tiles of parent.
     * Low risk, low reward strategy.
     */
    DispersalEvent disperseByGravity(const Plant& plant) const;
    
    /**
     * @brief Disperse seeds by wind
     * @param plant The parent plant
     * @param windSpeed Current wind speed (0-20 m/s typical)
     * @param windDirection Wind direction in degrees (0 = North)
     * @return DispersalEvent with wind-carried landing location
     * 
     * Distance depends on seed_aerodynamics, seed_mass, and wind conditions.
     * Light seeds with high aerodynamics travel farthest.
     */
    DispersalEvent disperseByWind(
        const Plant& plant,
        float windSpeed,
        float windDirection
    ) const;
    
    /**
     * @brief Disperse seeds through animal fruit consumption
     * @param plant The parent plant
     * @param creature The dispersing creature's phenotype
     * @param creatureX Current X position of creature
     * @param creatureY Current Y position of creature
     * @return DispersalEvent with gut-passage adjusted location
     * 
     * Seeds travel with creature based on gut transit time and locomotion.
     * Viability affected by seed_coat_durability vs seed_destruction_rate.
     */
    DispersalEvent disperseByAnimalFruit(
        const Plant& plant,
        const Phenotype& creature,
        int creatureX,
        int creatureY
    ) const;
    
    /**
     * @brief Disperse seeds by burr attachment
     * @param plant The parent plant (with hooks)
     * @param creature The creature carrying the burr
     * @param creatureX Creature's current X position (detachment point)
     * @param creatureY Creature's current Y position (detachment point)
     * @return DispersalEvent with burr landing location
     * 
     * Seeds attach to fur and detach after movement.
     * Distance depends on creature locomotion and grooming frequency.
     */
    DispersalEvent disperseByAnimalBurr(
        const Plant& plant,
        const Phenotype& creature,
        int creatureX,
        int creatureY
    ) const;
    
    /**
     * @brief Disperse seeds by explosive pod
     * @param plant The parent plant
     * @return DispersalEvent with ballistic trajectory landing
     * 
     * Pods build tension and explode, launching seeds 5-15 tiles.
     * Distance depends on explosive_pod_force gene.
     */
    DispersalEvent disperseByExplosive(const Plant& plant) const;
    
    /**
     * @brief Disperse by vegetative growth (runners/stolons)
     * @param plant The parent plant
     * @return DispersalEvent representing clonal spread
     * 
     * Creates a clone at the end of a runner.
     * High viability but short distance (2-5 tiles).
     */
    DispersalEvent disperseByVegetative(const Plant& plant) const;
    
    // ========================================================================
    // Automatic dispersal
    // ========================================================================
    
    /**
     * @brief Automatically disperse based on plant's emergent strategy
     * @param plant The parent plant
     * @param env Current environment state (optional, for wind data)
     * @return DispersalEvent using plant's primary strategy
     * 
     * Determines strategy from Plant::getPrimaryDispersalStrategy()
     * and dispatches to appropriate method.
     */
    DispersalEvent disperse(
        const Plant& plant,
        const EnvironmentState* env = nullptr
    ) const;
    
    // ========================================================================
    // Burr attachment/detachment mechanics
    // ========================================================================
    
    /**
     * @brief Check if a burr will attach to creature when passing plant
     * @param plant The plant with potential burrs
     * @param creature The passing creature's phenotype
     * @return true if burr attaches
     * 
     * Probability depends on:
     * - Plant's seed_hook_strength
     * - Creature's fur_density
     * - Creature's locomotion speed (fast = less attachment)
     */
    bool willBurrAttach(const Plant& plant, const Phenotype& creature) const;
    
    /**
     * @brief Check if an attached burr will detach this tick
     * @param creature The creature carrying the burr
     * @param ticksAttached How many ticks burr has been attached
     * @return true if burr detaches this tick
     * 
     * Probability increases with:
     * - Time attached (natural falloff)
     * - Creature's grooming_frequency
     * - Distance traveled
     */
    bool willBurrDetach(const Phenotype& creature, int ticksAttached) const;
    
    /**
     * @brief Calculate expected distance a burr will travel before detaching
     * @param plant The plant (for hook strength)
     * @param creature The creature carrying it
     * @return Expected distance in tiles
     */
    float calculateExpectedBurrDistance(
        const Plant& plant,
        const Phenotype& creature
    ) const;

private:
    // ========================================================================
    // Internal helper methods
    // ========================================================================
    
    /**
     * @brief Safe trait getter with default value
     */
    float getTraitSafe(
        const Phenotype& phenotype,
        const char* traitName,
        float defaultValue = 0.0f
    ) const;
    
    /**
     * @brief Generate random offset from origin
     * @param maxDistance Maximum distance from origin
     * @return Pair of (deltaX, deltaY)
     */
    std::pair<int, int> generateRandomOffset(float maxDistance) const;
    
    /**
     * @brief Generate offset in specific direction
     * @param distance Distance to travel
     * @param directionDegrees Direction in degrees (0 = North)
     * @return Pair of (deltaX, deltaY)
     */
    std::pair<int, int> generateDirectionalOffset(
        float distance,
        float directionDegrees
    ) const;
    
    /**
     * @brief Calculate wind dispersal distance
     * @param seedMass Mass of seed
     * @param seedAerodynamics Aerodynamic rating
     * @param windSpeed Wind speed
     * @return Distance in tiles
     */
    float calculateWindDistance(
        float seedMass,
        float seedAerodynamics,
        float windSpeed
    ) const;
    
    // ========================================================================
    // Random number generation
    // ========================================================================
    
    mutable std::mt19937 rng_;
    
    // ========================================================================
    // Constants
    // ========================================================================
    
    /// Base gravity dispersal distance
    static constexpr float GRAVITY_BASE_DISTANCE = 1.5f;
    
    /// Maximum gravity dispersal distance
    static constexpr float GRAVITY_MAX_DISTANCE = 3.0f;
    
    /// Wind distance multiplier
    static constexpr float WIND_DISTANCE_FACTOR = 5.0f;
    
    /// Maximum wind dispersal distance
    static constexpr float WIND_MAX_DISTANCE = 50.0f;
    
    /// Explosive pod base distance
    static constexpr float EXPLOSIVE_BASE_DISTANCE = 5.0f;
    
    /// Explosive pod force multiplier
    static constexpr float EXPLOSIVE_FORCE_FACTOR = 10.0f;
    
    /// Vegetative runner base length
    static constexpr float VEGETATIVE_BASE_DISTANCE = 2.0f;
    
    /// Vegetative runner max length
    static constexpr float VEGETATIVE_MAX_DISTANCE = 5.0f;
    
    /// Base burr attachment probability
    static constexpr float BURR_ATTACH_BASE_PROB = 0.5f;
    
    /// Natural burr detachment rate per tick
    static constexpr float BURR_NATURAL_DETACH_RATE = 0.01f;
    
    /// Grooming detachment rate multiplier
    static constexpr float GROOMING_DETACH_FACTOR = 0.05f;
    
    /// Default wind speed when environment not provided
    static constexpr float DEFAULT_WIND_SPEED = 5.0f;
    
    /// Default wind direction when environment not provided
    static constexpr float DEFAULT_WIND_DIRECTION = 0.0f;
};

} // namespace Genetics
} // namespace EcoSim
