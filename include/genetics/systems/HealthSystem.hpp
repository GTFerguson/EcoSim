/**
 * @file HealthSystem.hpp
 * @brief Organism-agnostic health management system for damage, healing, and regeneration
 *
 * Provides unified health calculations that work for ANY organism type
 * through the Organism interface, supporting the Unified Organism vision.
 *
 * Key Design Principles:
 * - NO type-specific code (no Plant, Creature, or DietType references)
 * - All organism queries use Organism& and phenotype traits
 * - Safe defaults for missing traits
 * - Consistent with existing Plant and Creature health behavior
 *
 * @see docs/code-review/recommendations/creature-decomposition-plan.md
 */

#ifndef HEALTH_SYSTEM_HPP
#define HEALTH_SYSTEM_HPP

#include <string>

namespace EcoSim {
namespace Genetics {

// Forward declarations - organism-agnostic interfaces only
class Organism;
class Phenotype;

/**
 * @brief Wound state categories based on health percentage
 */
enum class WoundState {
    Healthy,   // > 75% health
    Injured,   // 50-75% health
    Wounded,   // 25-50% health
    Critical,  // 0-25% health
    Dead       // <= 0% health
};

/**
 * @brief Types of damage that can be applied to organisms
 */
enum class DamageType {
    Physical,    // Teeth, claws, thorns - resisted by hardiness
    Toxin,       // Poison damage - resisted by toxin_resistance
    Starvation,  // Resource depletion - no resistance
    Environment  // Temperature, exposure - resisted by hardiness
};

/**
 * @brief Result of applying damage to an organism
 */
struct DamageResult {
    float actualDamage = 0.0f;      // Damage after resistance applied
    float resistanceApplied = 0.0f; // How much resistance reduced damage (0-1)
    bool fatal = false;              // Whether this damage was lethal
};

/**
 * @brief Result of healing an organism
 */
struct HealingResult {
    float actualHealing = 0.0f;  // Amount healed (may be less than requested)
    float newHealth = 0.0f;      // Health after healing applied
};

/**
 * @brief Unified health management system for organisms
 *
 * Provides organism-agnostic methods for:
 * - Damage application with resistance calculations
 * - Healing operations
 * - Natural regeneration processing
 * - Health state queries (wound state, severity)
 * - Max health and capacity calculations
 *
 * Shared between ALL organism types (creatures, plants, future organism types).
 * Uses phenotype traits exclusively - no type casting or type-specific code.
 *
 * @note This class supports the Unified Organism vision where capabilities
 * emerge from genetics rather than hard-coded organism types.
 */
class HealthSystem {
public:
    /**
     * @brief Default constructor
     */
    HealthSystem() = default;
    
    /**
     * @brief Destructor
     */
    ~HealthSystem() = default;
    
    // =========================================================================
    // Damage Operations
    // =========================================================================
    
    /**
     * @brief Apply damage to an organism with resistance calculations
     *
     * The actual damage applied depends on the organism's resistance traits:
     * - Physical: hardiness * 0.5 resistance (max 50% reduction)
     * - Toxin: toxin_resistance trait
     * - Starvation: No resistance
     * - Environment: hardiness * 0.5 resistance
     *
     * @param organism The organism receiving damage
     * @param currentHealth Current health of the organism (will be modified)
     * @param amount Raw damage amount before resistance
     * @param type Type of damage being applied
     * @return DamageResult with actual damage dealt and resistance info
     *
     * @note currentHealth is passed by reference and modified directly
     */
    DamageResult applyDamage(const Organism& organism,
                             float& currentHealth,
                             float amount,
                             DamageType type = DamageType::Physical) const;
    
    // =========================================================================
    // Healing Operations
    // =========================================================================
    
    /**
     * @brief Heal an organism by a specific amount
     *
     * Health is capped at the organism's maximum health.
     *
     * @param organism The organism being healed
     * @param currentHealth Current health of the organism (will be modified)
     * @param amount Amount to heal
     * @return HealingResult with actual healing applied and new health
     */
    HealingResult heal(const Organism& organism,
                       float& currentHealth,
                       float amount) const;
    
    /**
     * @brief Process natural healing/regeneration for one tick
     *
     * Uses the organism's healing rate trait (regrowth_rate for plants,
     * healing_rate for creatures) to apply regeneration.
     *
     * @param organism The organism regenerating
     * @param currentHealth Current health of the organism (will be modified)
     * @param energySurplus Available energy for regeneration (required for some organisms)
     * @return HealingResult with regeneration applied
     *
     * @note Regeneration may be blocked if organism lacks energy surplus
     */
    HealingResult processNaturalHealing(const Organism& organism,
                                        float& currentHealth,
                                        float energySurplus = 1.0f) const;
    
    // =========================================================================
    // Query Operations
    // =========================================================================
    
    /**
     * @brief Check if an organism should be considered dead
     *
     * @param currentHealth Current health value
     * @return true if health <= 0
     */
    bool checkDeathCondition(float currentHealth) const;
    
    /**
     * @brief Get the wound state based on health percentage
     *
     * Thresholds:
     * - Healthy: > 75%
     * - Injured: 50-75%
     * - Wounded: 25-50%
     * - Critical: 0-25%
     * - Dead: <= 0%
     *
     * @param organism The organism to query (for max health)
     * @param currentHealth Current health value
     * @return WoundState category
     */
    WoundState getWoundState(const Organism& organism,
                             float currentHealth) const;
    
    /**
     * @brief Get wound severity as a value from 0.0 to 1.0
     *
     * @param organism The organism to query (for max health)
     * @param currentHealth Current health value
     * @return 0.0 = fully healthy, 1.0 = dead
     */
    float getWoundSeverity(const Organism& organism,
                           float currentHealth) const;
    
    // =========================================================================
    // Capacity Operations
    // =========================================================================
    
    /**
     * @brief Get maximum health for an organism
     *
     * Calculation varies by organism traits:
     * - If "max_size" trait exists: max_size * 10.0
     * - Otherwise: 10.0 (default)
     *
     * @param organism The organism to query
     * @return Maximum health value
     */
    float getMaxHealth(const Organism& organism) const;
    
    /**
     * @brief Get health as percentage (0.0 to 1.0)
     *
     * @param organism The organism to query (for max health)
     * @param currentHealth Current health value
     * @return Health percentage (clamped to 0.0-1.0)
     */
    float getHealthPercent(const Organism& organism,
                           float currentHealth) const;
    
    // =========================================================================
    // Regeneration Queries
    // =========================================================================
    
    /**
     * @brief Check if organism can currently regenerate
     *
     * Requirements:
     * - Must be alive (health > 0)
     * - Must have regeneration ability (regrowth_rate or healing_rate > 0.1)
     * - Must have energy surplus (if applicable)
     *
     * @param organism The organism to query
     * @param currentHealth Current health value
     * @param energySurplus Available energy (1.0 = sufficient by default)
     * @return true if regeneration is possible
     */
    bool canRegenerate(const Organism& organism,
                       float currentHealth,
                       float energySurplus = 1.0f) const;
    
    /**
     * @brief Get the natural healing rate per tick
     *
     * Uses:
     * - "regrowth_rate" trait * 0.01 (for plants)
     * - "healing_rate" trait (for creatures)
     * - Fallback: 0.005 per tick
     *
     * @param organism The organism to query
     * @return Healing amount per tick
     */
    float getHealingRate(const Organism& organism) const;

private:
    // =========================================================================
    // Constants
    // =========================================================================
    
    /// Default max health when no traits specify it
    static constexpr float DEFAULT_MAX_HEALTH = 10.0f;
    
    /// Multiplier for max_size trait to get max health
    static constexpr float MAX_SIZE_HEALTH_MULTIPLIER = 10.0f;
    
    /// Maximum damage reduction from hardiness (50%)
    static constexpr float MAX_HARDINESS_RESISTANCE = 0.5f;
    
    /// Minimum regrowth/healing rate to enable regeneration
    static constexpr float MIN_REGENERATION_THRESHOLD = 0.1f;
    
    /// Default healing rate when no trait specified
    static constexpr float DEFAULT_HEALING_RATE = 0.005f;
    
    /// Multiplier for regrowth_rate trait to get healing amount
    static constexpr float REGROWTH_RATE_MULTIPLIER = 0.01f;
    
    // =========================================================================
    // Wound State Thresholds
    // =========================================================================
    
    static constexpr float HEALTHY_THRESHOLD = 0.75f;
    static constexpr float INJURED_THRESHOLD = 0.50f;
    static constexpr float WOUNDED_THRESHOLD = 0.25f;
    
    // =========================================================================
    // Helper Methods
    // =========================================================================
    
    /**
     * @brief Calculate resistance value for a given damage type
     *
     * @param organism The organism to query
     * @param type The damage type
     * @return Resistance multiplier (0.0 = no resistance, 1.0 = immune)
     */
    float calculateResistance(const Organism& organism,
                              DamageType type) const;
    
    /**
     * @brief Clamp value to range [0, 1]
     */
    static float clamp01(float value) {
        return (value < 0.0f) ? 0.0f : ((value > 1.0f) ? 1.0f : value);
    }
    
    /**
     * @brief Clamp value to minimum of 0
     */
    static float clampMin0(float value) {
        return (value < 0.0f) ? 0.0f : value;
    }
};

} // namespace Genetics
} // namespace EcoSim

#endif // HEALTH_SYSTEM_HPP
