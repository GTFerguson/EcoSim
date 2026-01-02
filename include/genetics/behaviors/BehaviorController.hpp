#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include <vector>
#include <memory>
#include <string>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Orchestrates behavior selection and execution for organisms
 * 
 * Manages a collection of behaviors, selecting and executing the highest
 * priority applicable behavior each tick. Behaviors are evaluated in
 * priority order (highest first), with stable sorting to maintain
 * insertion order for equal priorities.
 */
class BehaviorController {
public:
    BehaviorController() = default;
    ~BehaviorController() = default;
    
    // Non-copyable due to unique_ptr members
    BehaviorController(const BehaviorController&) = delete;
    BehaviorController& operator=(const BehaviorController&) = delete;
    
    // Movable
    BehaviorController(BehaviorController&&) = default;
    BehaviorController& operator=(BehaviorController&&) = default;
    
    /**
     * @brief Add a behavior to the controller
     * @param behavior Unique pointer to the behavior to add
     */
    void addBehavior(std::unique_ptr<IBehavior> behavior);
    
    /**
     * @brief Remove a behavior by its ID
     * @param behaviorId ID of the behavior to remove
     */
    void removeBehavior(const std::string& behaviorId);
    
    /**
     * @brief Check if a behavior with the given ID exists
     * @param behaviorId ID to check for
     * @return true if the behavior is present
     */
    bool hasBehavior(const std::string& behaviorId) const;
    
    /**
     * @brief Remove all behaviors from the controller
     */
    void clearBehaviors();
    
    /**
     * @brief Execute the highest priority applicable behavior
     * 
     * Evaluates all behaviors for applicability, sorts by priority
     * (highest first), and executes the top one.
     * 
     * @param organism The organism to execute behaviors for
     * @param ctx The current behavior context
     * @return Result of behavior execution, or {executed=false} if none applicable
     */
    BehaviorResult update(Organism& organism, BehaviorContext& ctx);
    
    /**
     * @brief Get the ID of the currently executing behavior
     * @return Behavior ID, or empty string if none
     */
    const std::string& getCurrentBehaviorId() const;
    
    /**
     * @brief Get the number of registered behaviors
     * @return Behavior count
     */
    std::size_t getBehaviorCount() const;
    
    /**
     * @brief Get all registered behavior IDs
     * @return Vector of behavior IDs
     */
    std::vector<std::string> getBehaviorIds() const;
    
    /**
     * @brief Get a human-readable status string
     * @return Status description including current behavior
     */
    std::string getStatusString() const;

private:
    std::vector<std::unique_ptr<IBehavior>> behaviors_;
    std::string currentBehaviorId_;
    
    /**
     * @brief Get behaviors that are applicable in current context
     * @param organism The organism to check against
     * @param ctx Current behavior context
     * @return Vector of raw pointers to applicable behaviors
     */
    std::vector<IBehavior*> getApplicableBehaviors(
        const Organism& organism,
        const BehaviorContext& ctx) const;
};

} // namespace Genetics
} // namespace EcoSim
