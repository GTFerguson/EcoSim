#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"
#include <memory>
#include <string>

namespace EcoSim {
namespace Genetics {

class FeedingInteraction;
class PerceptionSystem;
class Plant;
class Phenotype;

/**
 * @brief Behavior for herbivore feeding on plants
 * 
 * Implements the IBehavior interface to handle plant-eating behavior.
 * This behavior is organism-agnostic - it works with any organism that
 * has the plant_digestion trait and can access plants in the world.
 * 
 * Applicability:
 * - Organism has plant_digestion trait > 0.1 (can digest plants)
 * - Organism's energy level is below threshold (hungry)
 * 
 * Priority:
 * - Base priority: NORMAL (50)
 * - Increases with hunger up to HIGH (75)
 * 
 * Execution:
 * - Finds nearest edible plant within detection range
 * - If adjacent, attempts to eat using FeedingInteraction
 * - If not adjacent, signals movement is needed
 * 
 * @see IBehavior
 * @see FeedingInteraction
 * @see PerceptionSystem
 */
class FeedingBehavior : public IBehavior {
public:
    /**
     * @brief Construct FeedingBehavior with required dependencies
     * @param feeding Reference to FeedingInteraction calculator
     * @param perception Reference to PerceptionSystem for detection
     */
    FeedingBehavior(FeedingInteraction& feeding, PerceptionSystem& perception);
    
    /**
     * @brief Virtual destructor
     */
    ~FeedingBehavior() override = default;
    
    /**
     * @brief Get behavior identifier
     * @return "feeding"
     */
    std::string getId() const override;
    
    /**
     * @brief Check if this behavior applies to the organism
     * 
     * Returns true when:
     * - Organism has plant_digestion trait > PLANT_DIGESTION_THRESHOLD
     * - Organism's hunger is below the hunger threshold (is hungry)
     * 
     * @param organism The organism to check
     * @param ctx Current behavior context
     * @return true if organism can and should eat
     */
    bool isApplicable(const IGeneticOrganism& organism,
                      const BehaviorContext& ctx) const override;
    
    /**
     * @brief Get behavior priority based on hunger level
     * 
     * Priority scales with hunger:
     * - Base: NORMAL (50)
     * - Adds (1 - hungerRatio) * 25 when hungry
     * - Maximum: HIGH (75)
     * 
     * @param organism The organism to evaluate
     * @return Priority value (50-75)
     */
    float getPriority(const IGeneticOrganism& organism) const override;
    
    /**
     * @brief Execute feeding behavior
     * 
     * Process:
     * 1. Find nearest edible plant within detection range
     * 2. If adjacent (distance <= 1), eat using FeedingInteraction
     * 3. If not adjacent, return result indicating movement needed
     * 4. Deduct energy cost
     * 
     * @param organism The organism executing behavior (mutable)
     * @param ctx Behavior context with world access
     * @return Result with execution status and energy cost
     */
    BehaviorResult execute(IGeneticOrganism& organism,
                          BehaviorContext& ctx) override;
    
    /**
     * @brief Get estimated energy cost for feeding
     * @param organism The organism that would execute
     * @return Energy units consumed (based on metabolism)
     */
    float getEnergyCost(const IGeneticOrganism& organism) const override;

private:
    FeedingInteraction& feeding_;
    PerceptionSystem& perception_;
    
    /**
     * @brief Check if organism can eat plants based on phenotype
     * @param organism The organism to check
     * @return true if plant_digestion > PLANT_DIGESTION_THRESHOLD
     */
    bool canEatPlants(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Find the nearest edible plant within range
     * @param organism The organism searching for food
     * @param ctx Behavior context with world access
     * @return Pointer to nearest edible plant, or nullptr if none found
     */
    Plant* findNearestEdiblePlant(const IGeneticOrganism& organism,
                                   const BehaviorContext& ctx) const;
    
    /**
     * @brief Get current hunger level (0=starving, 1=full)
     * 
     * Derives hunger from phenotype/organism state if available.
     * Falls back to default value if state not accessible.
     * 
     * @param organism The organism to query
     * @return Hunger ratio (0-1)
     */
    float getHungerLevel(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Get hunger threshold from phenotype
     * 
     * Organisms seek food when hunger drops below this threshold.
     * 
     * @param organism The organism to query
     * @return Threshold value (typically around 0.5)
     */
    float getHungerThreshold(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Get detection range for finding plants
     * @param organism The organism to query
     * @return Detection range in tiles
     */
    float getDetectionRange(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Safely get a trait value from phenotype
     * @param phenotype The phenotype to query
     * @param traitName Name of the trait
     * @param defaultValue Value if trait not present
     * @return Trait value or default
     */
    float getTraitSafe(const Phenotype& phenotype, 
                       const std::string& traitName,
                       float defaultValue) const;
    
    /**
     * @brief Calculate distance between two positions
     * @param x1 First x coordinate
     * @param y1 First y coordinate
     * @param x2 Second x coordinate
     * @param y2 Second y coordinate
     * @return Euclidean distance
     */
    static float calculateDistance(int x1, int y1, int x2, int y2);
    
    /// Minimum plant_digestion trait to eat plants
    static constexpr float PLANT_DIGESTION_THRESHOLD = 0.1f;
    
    /// Default hunger threshold (seek food below this)
    static constexpr float DEFAULT_HUNGER_THRESHOLD = 0.5f;
    
    /// Base energy cost for feeding behavior
    static constexpr float BASE_ENERGY_COST = 0.01f;
    
    /// Adjacent distance threshold for eating
    static constexpr float ADJACENT_DISTANCE = 1.5f;
    
    /// Priority range
    static constexpr float BASE_PRIORITY = 50.0f;
    static constexpr float MAX_PRIORITY_BOOST = 25.0f;
};

} // namespace Genetics
} // namespace EcoSim
