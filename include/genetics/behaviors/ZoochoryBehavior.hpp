#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include <vector>
#include <tuple>
#include <unordered_map>

namespace EcoSim {
namespace Genetics {

class SeedDispersal;
struct DispersalEvent;

/**
 * @brief Handles animal-mediated seed dispersal (zoochory)
 * 
 * Zoochory is the biological term for seed dispersal by animals.
 * This behavior manages both:
 * - Endozoochory: Seeds consumed with fruit, pass through gut
 * - Epizoochory: Burrs/hooks that attach to animal fur
 * 
 * Centralizes seed state management (not per-creature storage).
 * Works with SeedDispersal interaction class for calculations.
 * 
 * @note Passive processing behavior (IDLE priority).
 * Runs when no higher priority behaviors are active.
 */
class ZoochoryBehavior : public IBehavior {
public:
    /**
     * @brief Construct with reference to dispersal calculator
     * @param dispersal Reference to SeedDispersal for calculations
     */
    explicit ZoochoryBehavior(SeedDispersal& dispersal);
    
    ~ZoochoryBehavior() override = default;
    
    /**
     * @brief Returns "zoochory"
     */
    std::string getId() const override;
    
    /**
     * @brief Always returns true (passive processing)
     * @param organism The organism to check
     * @param ctx Current behavior context
     * @return true - always applicable
     */
    bool isApplicable(const Organism& organism,
                      const BehaviorContext& ctx) const override;
    
    /**
     * @brief Returns IDLE (0) priority
     * @param organism The organism to evaluate
     * @return Priority value (IDLE)
     */
    float getPriority(const Organism& organism) const override;
    
    /**
     * @brief Process gut seeds and burr detachment
     * 
     * Performs:
     * - Decrements gut seed timers
     * - Creates dispersal events for ready seeds
     * - Checks burr detachment probability
     * - Returns events in result debugInfo
     * 
     * @param organism The organism executing this behavior
     * @param ctx Behavior context with world access
     * @return Result with dispersal events info
     */
    BehaviorResult execute(Organism& organism,
                          BehaviorContext& ctx) override;
    
    /**
     * @brief Returns 0 (passive, no energy cost)
     * @param organism The organism that would execute
     * @return 0.0f
     */
    float getEnergyCost(const Organism& organism) const override;
    
    /**
     * @brief Attach a burr from a plant to an organism (epizoochory)
     * 
     * Called by FeedingBehavior when encountering thorny plants.
     * Stores burr info for later detachment processing.
     * 
     * @param organismId Unique ID of the organism
     * @param plantX Origin plant X position
     * @param plantY Origin plant Y position
     * @param strategy Dispersal strategy (encoded as int)
     */
    void attachBurr(unsigned int organismId, int plantX, int plantY, int strategy);
    
    /**
     * @brief Process all seeds for an organism and return dispersal events
     * 
     * Main tick processing - handles both gut seeds and burr detachment.
     * 
     * @param organismId Unique ID of the organism
     * @param currentX Current organism X position
     * @param currentY Current organism Y position
     * @param ticksElapsed Number of ticks since last processing
     * @return Vector of dispersal events from seeds ready to disperse
     */
    std::vector<DispersalEvent> processOrganismSeeds(unsigned int organismId,
                                                      int currentX, int currentY,
                                                      int ticksElapsed);
    
    /**
     * @brief Check if an organism has burrs attached
     * @param organismId Unique ID of the organism
     * @return true if one or more burrs are attached
     */
    bool hasBurrs(unsigned int organismId) const;
    
    /**
     * @brief Add seeds to gut for dispersal via fruit consumption (endozoochory)
     * 
     * Called by FeedingBehavior when eating fruiting plants.
     * Seeds will be dispersed after gut transit time.
     * 
     * @param organismId Unique ID of the organism
     * @param plantX Origin plant X position
     * @param plantY Origin plant Y position
     * @param count Number of seeds consumed
     * @param viability Initial seed viability (0-1)
     */
    void consumeSeeds(unsigned int organismId, int plantX, int plantY,
                      int count, float viability);
    
    /**
     * @brief Clear all seed data for an organism (e.g., on death)
     * @param organismId Unique ID of the organism
     */
    void clearOrganismData(unsigned int organismId);

private:
    SeedDispersal& dispersal_;
    
    /**
     * Burr storage (epizoochory): organismId -> vector<(strategy, originX, originY, ticksAttached)>
     * Strategy is stored as int (from DispersalStrategy enum)
     */
    std::unordered_map<unsigned int, std::vector<std::tuple<int, int, int, int>>> attachedBurrs_;
    
    /**
     * Gut seeds (endozoochory): organismId -> vector<(encodedOrigin, viability, ticksRemaining)>
     * encodedOrigin = originX * 10000 + originY
     */
    std::unordered_map<unsigned int, std::vector<std::tuple<int, float, int>>> gutSeeds_;
    
    /// Probability of burr detaching per tick (base rate)
    static constexpr float BURR_DETACH_CHANCE = 0.05f;
    
    /// Default gut transit time in ticks
    static constexpr int GUT_TRANSIT_TICKS = 500;
    
    /**
     * @brief Process gut seeds for an organism (endozoochory)
     * @return Vector of dispersal events for seeds that completed transit
     */
    std::vector<DispersalEvent> processGutSeeds(unsigned int organismId,
                                                 int currentX, int currentY);
    
    /**
     * @brief Process burr detachment for an organism (epizoochory)
     * @return Vector of dispersal events for detached burrs
     */
    std::vector<DispersalEvent> processBurrDetachment(unsigned int organismId,
                                                       int currentX, int currentY);
    
    /**
     * @brief Get a unique ID for an organism based on its genome
     * @param organism The organism to get ID for
     * @return Hash-based unique identifier
     */
    unsigned int getOrganismId(const Organism& organism) const;
};

} // namespace Genetics
} // namespace EcoSim
