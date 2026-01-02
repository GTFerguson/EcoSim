#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include <string>

namespace EcoSim {
namespace Genetics {

class Phenotype;

/**
 * @brief Behavior for organism rest/sleep to recover from fatigue
 * 
 * Implements the IBehavior interface to handle rest behavior.
 * This behavior is organism-agnostic - it works with any organism that
 * has fatigue tracking and can benefit from rest.
 * 
 * Applicability:
 * - Organism's fatigue level is above the fatigue threshold
 * - Organism has fatigue tracking (via phenotype traits)
 * 
 * Priority:
 * - Base priority: NORMAL (50)
 * - Increases with fatigue up to HIGH (75)
 * 
 * Execution:
 * - Reduces fatigue by recovery rate each tick
 * - Minimal energy cost (sleeping saves energy)
 * - Completes when fatigue drops below threshold
 * 
 * @see IBehavior
 */
class RestBehavior : public IBehavior {
public:
    /**
     * @brief Default constructor
     */
    RestBehavior() = default;
    
    /**
     * @brief Virtual destructor
     */
    ~RestBehavior() override = default;
    
    /**
     * @brief Get behavior identifier
     * @return "rest"
     */
    std::string getId() const override;
    
    /**
     * @brief Check if this behavior applies to the organism
     * 
     * Returns true when:
     * - Organism's fatigue is above the fatigue threshold
     * - Organism has fatigue tracking (phenotype traits present)
     * 
     * @param organism The organism to check
     * @param ctx Current behavior context
     * @return true if organism is tired and should rest
     */
    bool isApplicable(const Organism& organism,
                      const BehaviorContext& ctx) const override;
    
    /**
     * @brief Get behavior priority based on fatigue level
     * 
     * Priority scales with fatigue:
     * - Base: NORMAL (50)
     * - Adds fatigue ratio * 25 when tired
     * - Maximum: HIGH (75)
     * 
     * @param organism The organism to evaluate
     * @return Priority value (50-75)
     */
    float getPriority(const Organism& organism) const override;
    
    /**
     * @brief Execute rest behavior
     * 
     * Process:
     * 1. Reduce fatigue by recovery rate
     * 2. Apply minimal energy cost (resting conserves energy)
     * 3. Complete when fatigue drops below threshold
     * 
     * @param organism The organism executing behavior (mutable)
     * @param ctx Behavior context
     * @return Result with execution status and energy cost
     */
    BehaviorResult execute(Organism& organism,
                          BehaviorContext& ctx) override;
    
    /**
     * @brief Get estimated energy cost for resting
     * @param organism The organism that would execute
     * @return Energy units consumed (low - resting saves energy)
     */
    float getEnergyCost(const Organism& organism) const override;

private:
    /**
     * @brief Get current fatigue level from organism
     * @param organism The organism to query
     * @return Fatigue level (0 = rested, higher = more tired)
     */
    float getFatigueLevel(const Organism& organism) const;
    
    /**
     * @brief Get fatigue threshold from phenotype
     * 
     * Organisms need rest when fatigue exceeds this threshold.
     * 
     * @param organism The organism to query
     * @return Threshold value (typically around 3.0)
     */
    float getFatigueThreshold(const Organism& organism) const;
    
    /**
     * @brief Get fatigue recovery rate from phenotype
     * 
     * How much fatigue is reduced per tick while resting.
     * Higher stamina = faster recovery.
     * 
     * @param organism The organism to query
     * @return Recovery rate per tick
     */
    float getRecoveryRate(const Organism& organism) const;
    
    /**
     * @brief Check if organism is tired (fatigue above threshold)
     * @param organism The organism to check
     * @return true if fatigue > threshold
     */
    bool isTired(const Organism& organism) const;
    
    /// Default fatigue threshold (need rest above this)
    static constexpr float DEFAULT_FATIGUE_THRESHOLD = 3.0f;
    
    /// Default fatigue recovery rate per tick
    static constexpr float DEFAULT_RECOVERY_RATE = 0.01f;
    
    /// Base energy cost for resting (very low - conserves energy)
    static constexpr float REST_ENERGY_COST = 0.005f;
    
    /// Priority range
    static constexpr float BASE_PRIORITY = 50.0f;
    static constexpr float MAX_PRIORITY_BOOST = 25.0f;
};

} // namespace Genetics
} // namespace EcoSim
