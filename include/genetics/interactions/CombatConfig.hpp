#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * @brief Centralized combat balance configuration
 *
 * All combat-related multipliers and thresholds are defined here
 * for easy tuning and maintenance.
 */
namespace CombatConfig {

// ============================================================================
// Combat Initiation
// ============================================================================

/// Minimum gene value to consider weapon "usable"
constexpr float WEAPON_USABILITY_THRESHOLD = 0.1f;

/// Threshold for combat initiation (aggression × hunt_instinct × hunger_mod)
constexpr float COMBAT_INITIATION_THRESHOLD = 0.3f;

/// Minimum aggression required for defender to counter-attack
constexpr float COUNTER_ATTACK_AGGRESSION_THRESHOLD = 0.3f;

// ============================================================================
// Hunger Motivation Formula
// ============================================================================

/// Base multiplier for hunger-motivated combat
constexpr float HUNGER_MOTIVATION_BASE = 0.5f;

/// Additional multiplier scaled by hunger level
constexpr float HUNGER_MOTIVATION_SCALE = 0.5f;

// ============================================================================
// Defense System
// ============================================================================

/// Maximum defense reduction (50% at max defense value)
constexpr float MAX_DEFENSE_REDUCTION = 0.5f;

/// Starting value for defense comparison (higher than max possible)
constexpr float DEFENSE_COMPARISON_INITIAL = 2.0f;

// ============================================================================
// Specialization Bonus
// ============================================================================

/// Maximum bonus for damage specialization (50% bonus at pure specialist)
constexpr float SPECIALIZATION_BONUS_MAX = 0.5f;

// ============================================================================
// Health Conversion
// ============================================================================

/// Divisor for converting damage to health percentage
constexpr float HEALTH_CONVERSION_DIVISOR = 100.0f;

// ============================================================================
// Bleeding System
// ============================================================================

/// Pierce damage threshold that may cause bleeding
constexpr float BLEEDING_DAMAGE_THRESHOLD = 5.0f;

// ============================================================================
// Stamina Costs
// ============================================================================

/// Default stamina cost per attack
constexpr float DEFAULT_STAMINA_COST = 0.1f;

// ============================================================================
// Damage Calculation Multipliers
// ============================================================================

/// Teeth: Slash damage weight from serration
constexpr float TEETH_SERRATION_SLASH_WEIGHT = 0.5f;

/// Claws: Blunt damage weight from lack of sharpness
constexpr float CLAWS_BLUNT_WEIGHT = 0.3f;

/// Horns: Slash damage weight from spread
constexpr float HORNS_SPREAD_SLASH_WEIGHT = 0.3f;

/// Tail: Slash damage weight from non-mass
constexpr float TAIL_SLASH_WEIGHT = 0.5f;

} // namespace CombatConfig

} // namespace Genetics
} // namespace EcoSim
