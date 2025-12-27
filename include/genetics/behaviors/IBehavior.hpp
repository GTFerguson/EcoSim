#pragma once

#include <string>
#include <memory>

namespace EcoSim {
namespace Genetics {

// Forward declarations
class IGeneticOrganism;
struct BehaviorContext;

/**
 * @brief Priority levels for behavior execution
 * 
 * Higher priority behaviors execute first when applicable.
 * Survival-critical behaviors (fleeing, feeding when starving) have highest priority.
 */
enum class BehaviorPriority : int {
    CRITICAL = 100,    ///< Fleeing from danger, dying
    HIGH = 75,         ///< Feeding when hungry, drinking when thirsty
    NORMAL = 50,       ///< Hunting, mating when ready
    LOW = 25,          ///< Wandering, social behaviors
    IDLE = 0           ///< Default/fallback behaviors
};

/**
 * @brief Result of behavior execution
 */
struct BehaviorResult {
    bool executed = false;           ///< Did the behavior run?
    bool completed = false;          ///< Is the behavior goal achieved?
    float energyCost = 0.0f;         ///< Energy consumed
    std::string debugInfo;           ///< Debug/logging information
};

/**
 * @brief Base interface for all organism behaviors
 * 
 * Behaviors are modular components that can be:
 * - Added/removed dynamically based on gene expression
 * - Shared between organism types (creature/plant)
 * - Prioritized for execution
 * - Tested in isolation
 * 
 * @note This interface supports the Unified Organism vision where
 * capabilities emerge from genetics rather than hard-coded types.
 */
class IBehavior {
public:
    virtual ~IBehavior() = default;
    
    /**
     * @brief Unique identifier for this behavior type
     * @return String identifier (e.g., "hunting", "feeding", "mating")
     */
    virtual std::string getId() const = 0;
    
    /**
     * @brief Check if this behavior is applicable to the given organism
     * 
     * Evaluates the organism's current state and capabilities to determine
     * if this behavior should be considered for execution.
     * 
     * @param organism The organism to check
     * @param ctx Current behavior context (world state, time, etc.)
     * @return true if the behavior can meaningfully execute
     * 
     * @example HuntingBehavior returns true if:
     *   - organism has hunt_instinct > threshold
     *   - organism has locomotion_speed > threshold  
     *   - organism is hungry
     *   - prey is detected nearby
     */
    virtual bool isApplicable(const IGeneticOrganism& organism,
                              const BehaviorContext& ctx) const = 0;
    
    /**
     * @brief Get execution priority for this behavior
     * 
     * Higher priority behaviors are evaluated first. Priority may vary
     * based on organism state (e.g., feeding priority increases with hunger).
     * 
     * @param organism The organism to evaluate
     * @return Priority value (higher = more urgent)
     */
    virtual float getPriority(const IGeneticOrganism& organism) const = 0;
    
    /**
     * @brief Execute the behavior for one tick
     * 
     * Performs the behavior's action on the organism. May modify organism
     * state, world state (through context), or both.
     * 
     * @param organism The organism executing this behavior (mutable)
     * @param ctx Behavior context with world access
     * @return Result indicating what happened
     */
    virtual BehaviorResult execute(IGeneticOrganism& organism,
                                   BehaviorContext& ctx) = 0;
    
    /**
     * @brief Get estimated energy cost for this behavior
     * 
     * Used by BehaviorController for energy budget planning.
     * 
     * @param organism The organism that would execute
     * @return Estimated energy units consumed
     */
    virtual float getEnergyCost(const IGeneticOrganism& organism) const = 0;
};

/**
 * @brief Factory for creating behavior instances with dependency injection
 * 
 * Enables creation of behaviors with proper dependencies without
 * coupling to concrete implementations.
 */
class IBehaviorFactory {
public:
    virtual ~IBehaviorFactory() = default;
    
    /**
     * @brief Create a behavior instance by ID
     * @param behaviorId Identifier of the behavior to create
     * @return Unique pointer to the created behavior, or nullptr if unknown
     */
    virtual std::unique_ptr<IBehavior> create(const std::string& behaviorId) = 0;
};

} // namespace Genetics
} // namespace EcoSim
