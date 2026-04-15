#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * Tuning constants for creature-style organisms. Historically lived as
 * static const members on the Creature class; extracted here so they
 * are reachable without including creature.hpp and so the Creature
 * class itself can be deleted.
 *
 * Values are inline constexpr, available at compile time.
 */
namespace Constants {

// ---------- Breeding / resource economy ----------
inline constexpr float RESOURCE_LIMIT   = 10.0f;
inline constexpr float BREED_COST       =  3.0f;
inline constexpr float DIAG_ADJUST      =  1.4f;    // diagonal-movement cost multiplier
inline constexpr float IDEAL_SIMILARITY =  0.8f;    // mate selection sweet spot
inline constexpr float PENALTY_EXPONENT =  1.5f;
inline constexpr float INIT_FATIGUE     =  0.0f;
inline constexpr float PREY_CALORIES    =  3.0f;

// Death thresholds
inline constexpr float STARVATION_POINT  = -0.1f;
inline constexpr float DEHYDRATION_POINT = -0.1f;
inline constexpr float DISCOMFORT_POINT  = -3.0f;

// What fraction of resources is shared in cooperative exchanges
inline constexpr unsigned RESOURCE_SHARED = 4;

// ---------- Scent detection ----------
inline constexpr float SCENT_DETECTION_BASE_RANGE  = 100.0f;
inline constexpr float SCENT_DETECTION_ACUITY_MULT = 100.0f;
inline constexpr float DEFAULT_OLFACTORY_ACUITY    =   0.5f;
inline constexpr float DEFAULT_SCENT_PRODUCTION    =   0.5f;

// ---------- Seed dispersal (zoochory) ----------
inline constexpr float BURR_SEED_VIABILITY          = 0.85f;
inline constexpr float GUT_SEED_SCARIFICATION_BONUS = 1.15f;
inline constexpr float GUT_SEED_ACID_DAMAGE         = 0.9f;
inline constexpr float DEFAULT_GUT_TRANSIT_HOURS    = 6.0f;
inline constexpr float TICKS_PER_HOUR               = 10.0f;

// ---------- Feeding interaction ----------
inline constexpr float FEEDING_MATE_BOOST        = 2.0f;
inline constexpr float DAMAGE_HUNGER_COST        = 0.5f;
inline constexpr float SEEKING_FOOD_MATE_PENALTY = 0.5f;

// ---------- Sense enhancement ----------
inline constexpr float COLOR_VISION_RANGE_BONUS    = 0.3f;
inline constexpr float SCENT_DETECTION_RANGE_BONUS = 0.5f;

// ---------- Movement ----------
inline constexpr float BASE_MOVEMENT_SPEED = 0.5f;
inline constexpr float MIN_MOVEMENT_SPEED  = 0.1f;
inline constexpr float DEFAULT_LEG_LENGTH  = 0.5f;
inline constexpr float DEFAULT_BODY_MASS   = 1.0f;

} // namespace Constants

} // namespace Genetics
} // namespace EcoSim
