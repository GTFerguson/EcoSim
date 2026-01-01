#ifndef CREATURE_SERIALIZATION_HPP
#define CREATURE_SERIALIZATION_HPP

/**
 * @file CreatureSerialization.hpp
 * @brief Serialization functionality extracted from Creature class
 *
 * This module handles string and JSON serialization/deserialization for creatures.
 * Part of Phase 1d creature decomposition to improve maintainability.
 */

#include <string>
#include <nlohmann/json.hpp>

class Creature;

// Forward declare enums
enum class Profile;
enum class Direction;
enum class WoundState;
enum class Motivation;
enum class Action;

namespace CreatureSerialization {

//============================================================================
//  String Serialization
//============================================================================

/**
 * @brief Convert creature state to string representation.
 *
 * Creates a comma-separated string containing creature position, age,
 * direction, profile, and resource levels. Used for legacy save format.
 *
 * @param creature The creature to serialize
 * @return String representation of creature state
 */
std::string toString(const Creature& creature);

/**
 * @brief Convert profile enum to string.
 * @param profile The profile value
 * @return Human-readable profile string
 */
std::string profileToString(Profile profile);

/**
 * @brief Convert direction enum to string.
 * @param direction The direction value
 * @return Direction string (e.g., "N", "SE", "none")
 */
std::string directionToString(Direction direction);

/**
 * @brief Parse profile from string.
 * @param str The string to parse
 * @return Profile enum value (defaults to migrate on unknown)
 */
Profile stringToProfile(const std::string& str);

/**
 * @brief Parse direction from string.
 * @param str The string to parse
 * @return Direction enum value (defaults to none on unknown)
 */
Direction stringToDirection(const std::string& str);

//============================================================================
//  Enum Conversion Helpers
//============================================================================

/**
 * @brief Convert wound state enum to string.
 * @param state The wound state value
 * @return Uppercase string (e.g., "HEALTHY", "CRITICAL")
 */
std::string woundStateToString(WoundState state);

/**
 * @brief Parse wound state from string.
 * @param str The string to parse
 * @return WoundState enum value (defaults to Healthy on unknown)
 */
WoundState stringToWoundState(const std::string& str);

/**
 * @brief Convert motivation enum to string.
 * @param motivation The motivation value
 * @return Uppercase string (e.g., "HUNGRY", "CONTENT")
 */
std::string motivationToString(Motivation motivation);

/**
 * @brief Parse motivation from string.
 * @param str The string to parse
 * @return Motivation enum value (defaults to Content on unknown)
 */
Motivation stringToMotivation(const std::string& str);

/**
 * @brief Convert action enum to string.
 * @param action The action value
 * @return Uppercase string (e.g., "IDLE", "HUNTING")
 */
std::string actionToString(Action action);

/**
 * @brief Parse action from string.
 * @param str The string to parse
 * @return Action enum value (defaults to Idle on unknown)
 */
Action stringToAction(const std::string& str);

//============================================================================
//  JSON Serialization
//============================================================================

/**
 * @brief Serialize creature state to JSON.
 *
 * Creates a comprehensive JSON object containing:
 * - Identity (id, archetype, scientific name)
 * - State (hunger, thirst, fatigue, mate, age)
 * - Position (worldX, worldY, tileX, tileY)
 * - Health (current, max, wound state)
 * - Combat (target, flags, cooldown)
 * - Behavior (motivation, action)
 * - Genome (full genetic data)
 *
 * @param creature The creature to serialize
 * @return JSON object containing creature state
 */
nlohmann::json toJson(const Creature& creature);

/**
 * @brief Deserialize creature from JSON.
 *
 * Reconstructs a creature from saved JSON data:
 * - Loads genome and regenerates phenotype
 * - Restores state, position, health
 * - Reclassifies archetype from genome
 * - Validates and clamps position to world bounds
 *
 * @param j JSON object containing creature data
 * @param mapWidth World width for position validation
 * @param mapHeight World height for position validation
 * @return Reconstructed Creature object
 * @throws std::runtime_error if required fields are missing
 */
Creature fromJson(const nlohmann::json& j, int mapWidth, int mapHeight);

} // namespace CreatureSerialization

#endif // CREATURE_SERIALIZATION_HPP
