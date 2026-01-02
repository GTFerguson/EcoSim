#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include <unordered_map>
#include <random>
#include <string>

namespace EcoSim {
namespace Genetics {

class CombatInteraction;
class PerceptionSystem;
class Phenotype;

/**
 * @brief Hunting behavior for predatory organisms
 *
 * Implements hunting with critical bug fixes that prevent prey extinction:
 * 1. Satiation check - Don't hunt when 80%+ full
 * 2. Hunt energy cost - Deduct energy for each hunt attempt
 * 3. Hunt cooldown tracking - Minimum ticks between hunts
 * 4. Prey escape mechanics - Use flee/pursue genes for escape chance
 *
 * All methods use Organism& interface and phenotype traits.
 * NO type-specific code (no direct Creature references).
 *
 * @see docs/code-review/recommendations/creature-decomposition-plan.md Section 4
 */
class HuntingBehavior : public IBehavior {
public:
    /**
     * @brief Construct hunting behavior with required dependencies
     * @param combat Reference to combat interaction system
     * @param perception Reference to perception system for prey detection
     */
    HuntingBehavior(CombatInteraction& combat, PerceptionSystem& perception);
    ~HuntingBehavior() override = default;
    
    /**
     * @brief Get behavior identifier
     * @return "hunting"
     */
    std::string getId() const override;
    
    /**
     * @brief Check if hunting is applicable for this organism
     *
     * Returns true when:
     * - Organism has hunt_instinct trait > HUNT_INSTINCT_THRESHOLD (0.4)
     * - Organism has locomotion_speed > 0.3 (can chase)
     * - Organism is NOT satiated (hunger < 80% full)
     * - Organism is NOT on cooldown
     *
     * @param organism The organism to check
     * @param ctx Current behavior context (provides currentTick)
     * @return true if the organism should consider hunting
     */
    bool isApplicable(const Organism& organism,
                      const BehaviorContext& ctx) const override;
    
    /**
     * @brief Get hunting priority based on hunger level
     *
     * Base priority: NORMAL (50)
     * Increases with hunger level (up to 75)
     *
     * @param organism The organism to evaluate
     * @return Priority value
     */
    float getPriority(const Organism& organism) const override;
    
    /**
     * @brief Execute hunting for one tick
     *
     * Process:
     * 1. Find prey via perception system
     * 2. Calculate escape chance using flee/pursue genes
     * 3. If prey escapes: return executed=true, completed=false
     * 4. If prey doesn't escape: engage combat via CombatInteraction
     * 5. Record hunt tick for cooldown
     * 6. Deduct HUNT_COST energy
     *
     * @param organism The hunting organism (mutable for energy deduction)
     * @param ctx Behavior context with world access
     * @return Result indicating outcome
     */
    BehaviorResult execute(Organism& organism,
                          BehaviorContext& ctx) override;
    
    /**
     * @brief Get estimated energy cost for hunting
     * @param organism The organism that would hunt
     * @return Estimated energy units consumed (HUNT_COST)
     */
    float getEnergyCost(const Organism& organism) const override;

private:
    CombatInteraction& combat_;
    PerceptionSystem& perception_;
    
    mutable std::unordered_map<unsigned int, unsigned int> lastHuntTick_;
    
    mutable std::mt19937 rng_{std::random_device{}()};
    
    static constexpr float HUNT_INSTINCT_THRESHOLD = 0.4f;
    static constexpr float LOCOMOTION_THRESHOLD = 0.3f;
    static constexpr float SATIATION_THRESHOLD = 0.8f;
    static constexpr float HUNT_COST = 1.5f;
    static constexpr unsigned int HUNT_COOLDOWN = 30;
    
    static constexpr float BASE_PRIORITY = 50.0f;
    static constexpr float MAX_PRIORITY_BOOST = 25.0f;
    static constexpr float DEFAULT_HUNGER_THRESHOLD = 10.0f;
    static constexpr float BASE_ESCAPE_DENOMINATOR = 0.1f;
    
    /**
     * @brief Check if organism has hunting capability
     * @param organism The organism to check
     * @return true if hunt_instinct > threshold
     */
    bool canHunt(const Organism& organism) const;
    
    /**
     * @brief Check if organism has mobility for hunting
     * @param organism The organism to check
     * @return true if locomotion_speed > threshold
     */
    bool canChase(const Organism& organism) const;
    
    /**
     * @brief Check if organism is satiated (too full to hunt)
     *
     * Uses energy_level from OrganismState in context. If >= SATIATION_THRESHOLD (80%),
     * the organism is too full to bother hunting.
     *
     * @param organism The organism to check
     * @param ctx Context containing organism state
     * @return true if organism is 80%+ full
     */
    bool isSatiated(const Organism& organism, const BehaviorContext& ctx) const;
    
    /**
     * @brief Check if organism is on hunt cooldown
     * @param organismId Unique identifier for the organism
     * @param currentTick Current simulation tick
     * @return true if fewer than HUNT_COOLDOWN ticks since last hunt
     */
    bool isOnCooldown(unsigned int organismId, unsigned int currentTick) const;
    
    /**
     * @brief Calculate prey's escape chance
     *
     * Formula: escapeChance = prey_flee / (prey_flee + predator_pursue + 0.1)
     *
     * This ensures:
     * - High flee_threshold in prey increases escape chance
     * - High pursue_threshold in predator decreases escape chance
     * - The +0.1 prevents division by zero and ensures no guaranteed outcomes
     *
     * @param predator The hunting organism
     * @param prey The prey organism
     * @return Escape probability (0.0 to 1.0)
     */
    float calculateEscapeChance(const Organism& predator,
                                const Organism& prey) const;
    
    /**
     * @brief Roll escape chance and determine if prey escapes
     * @param predator The hunting organism
     * @param prey The prey organism
     * @return true if prey successfully escapes
     */
    bool attemptEscape(const Organism& predator,
                       const Organism& prey) const;
    
    /**
     * @brief Find potential prey in detection range
     *
     * Uses perception system to find organisms that:
     * - Are within detection range
     * - Have lower hunt_instinct (not predators themselves)
     * - Have positive meat_value or nutrient_value
     *
     * @param hunter The hunting organism
     * @param ctx Behavior context with world access
     * @return Pointer to prey organism, or nullptr if none found
     */
    Organism* findPrey(const Organism& hunter,
                       const BehaviorContext& ctx) const;
    
    /**
     * @brief Record hunt attempt for cooldown tracking
     * @param organismId Unique identifier for the organism
     * @param tick Tick when hunt occurred
     */
    void recordHunt(unsigned int organismId, unsigned int tick);
    
    /**
     * @brief Remove stale cooldown entries to prevent memory bloat
     * @param currentTick Current simulation tick
     */
    void cleanupStaleEntries(unsigned int currentTick);
    
    /**
     * @brief Get hunger level from organism phenotype
     * @param organism The organism to query
     * @return Current hunger level (0 = starving, max = full)
     */
    float getHungerLevel(const Organism& organism) const;
    
    /**
     * @brief Get hunger threshold from organism phenotype
     * @param organism The organism to query
     * @return Maximum hunger capacity
     */
    float getHungerThreshold(const Organism& organism) const;
    
    /**
     * @brief Get organism's unique ID for tracking
     *
     * Uses a hash of the organism's genome pointer as a stable identifier.
     *
     * @param organism The organism
     * @return Unique identifier
     */
    unsigned int getOrganismId(const Organism& organism) const;
};

} // namespace Genetics
} // namespace EcoSim
