#ifndef CREATURE_SCENT_HPP
#define CREATURE_SCENT_HPP

/**
 * @file CreatureScent.hpp
 * @brief Scent system functionality extracted from Creature class
 *
 * This module handles scent signature generation, scent deposits, and scent detection.
 * Part of Phase 1b creature decomposition to improve maintainability.
 */

#include <array>
#include <optional>

// Forward declarations
class Creature;
enum class Direction;

namespace EcoSim {
class ScentLayer;
struct ScentDeposit;
}

namespace CreatureScent {

//============================================================================
//  Constants
//============================================================================

/**
 * @brief Base range for scent detection (tiles).
 *        Combined with acuity multiplier for total range.
 */
constexpr float SCENT_DETECTION_BASE_RANGE = 100.0f;

/**
 * @brief Multiplier applied to olfactory acuity for detection range.
 *        Total range = BASE_RANGE + (acuity * ACUITY_MULT)
 */
constexpr float SCENT_DETECTION_ACUITY_MULT = 100.0f;

/**
 * @brief Default olfactory acuity for creatures without the gene.
 */
constexpr float DEFAULT_OLFACTORY_ACUITY = 0.5f;

/**
 * @brief Default scent production rate for creatures without the gene.
 */
constexpr float DEFAULT_SCENT_PRODUCTION = 0.5f;

//============================================================================
//  Scent Signature Generation
//============================================================================

/**
 * @brief Compute a creature's unique genetic scent signature.
 *
 * Creates an 8-float fingerprint based on genome characteristics.
 * Related creatures will have similar signatures, enabling kin detection.
 *
 * Signature components:
 * - [0] Diet type (normalized)
 * - [1] Lifespan (normalized)
 * - [2] Sight range (normalized)
 * - [3] Metabolism (normalized)
 * - [4] Social behavior (flee/pursue ratio)
 * - [5] Flocking tendency
 * - [6] Scent signature variance (from gene or derived)
 * - [7] Scent production (from gene or derived)
 *
 * @param creature The creature to compute signature for
 * @return 8-float array representing the creature's genetic fingerprint
 */
std::array<float, 8> computeScentSignature(const Creature& creature);

/**
 * @brief Calculate genetic similarity between two scent signatures.
 *
 * Uses cosine similarity for comparing 8-dimensional signature vectors.
 * A value of 1.0 indicates identical signatures, 0.0 indicates orthogonal.
 *
 * @param sig1 First signature
 * @param sig2 Second signature
 * @return Similarity value [0.0, 1.0] where 1.0 is identical
 */
float calculateSignatureSimilarity(
    const std::array<float, 8>& sig1,
    const std::array<float, 8>& sig2);

//============================================================================
//  Scent Deposits
//============================================================================

/**
 * @brief Deposit breeding pheromone when in breeding state.
 *
 * Creates a MATE_SEEKING scent deposit based on creature's olfactory genes.
 * The deposit includes the creature's scent signature for mate recognition.
 *
 * Factors affecting deposit:
 * - SCENT_PRODUCTION gene (intensity)
 * - SCENT_MASKING gene (reduces effective production)
 * - Profile must be breed (otherwise no deposit)
 *
 * @param creature The creature depositing scent
 * @param layer The world's scent layer to deposit into
 * @param currentTick Current simulation tick for timestamp
 */
void depositBreedingScent(Creature& creature, EcoSim::ScentLayer& layer, unsigned int currentTick);

//============================================================================
//  Scent Detection
//============================================================================

/**
 * @brief Check if creature has meaningful scent detection capability.
 *
 * Used to determine if scent-based fallback navigation is available.
 * Checks the SCENT_DETECTION trait against a minimum threshold (0.1).
 *
 * @param creature The creature to check
 * @return True if scent_detection trait exceeds threshold
 */
bool hasScentDetection(const Creature& creature);

/**
 * @brief Detect the direction to a potential mate using scent trails.
 *
 * Queries the scent layer for MATE_SEEKING scents within detection range.
 * Uses OLFACTORY_ACUITY gene to determine detection range.
 * Only searches when creature is in breeding mode.
 *
 * @param creature The detecting creature
 * @param scentLayer The world's scent layer to query
 * @return Direction toward strongest mate scent, or std::nullopt if none found
 */
std::optional<Direction> detectMateDirection(const Creature& creature, const EcoSim::ScentLayer& scentLayer);

/**
 * @brief Find the coordinates of the strongest mate scent in range.
 *
 * Returns target coordinates suitable for A* pathfinding navigation.
 * Mate compatibility is determined by genetic similarity in checkFitness()
 * when creatures actually meet.
 *
 * @param creature The detecting creature
 * @param scentLayer The world's scent layer to query
 * @param outX Output parameter for scent X coordinate
 * @param outY Output parameter for scent Y coordinate
 * @return True if a valid scent was found, false otherwise
 */
bool findMateScent(const Creature& creature, const EcoSim::ScentLayer& scentLayer, int& outX, int& outY);

/**
 * @brief Find the coordinates of food scent (plant scent) in range.
 *
 * Used as a fallback when visual plant detection fails.
 * Queries for FOOD_TRAIL scent type deposited by plants.
 *
 * @param creature The detecting creature
 * @param scentLayer The world's scent layer to query
 * @param outX Output parameter for scent X coordinate
 * @param outY Output parameter for scent Y coordinate
 * @return True if a valid food scent was found, false otherwise
 */
bool findFoodScent(const Creature& creature, const EcoSim::ScentLayer& scentLayer, int& outX, int& outY);

/**
 * @brief Calculate scent detection range for a creature.
 *
 * Range formula: BASE_RANGE + (olfactory_acuity * ACUITY_MULT)
 * Uses DEFAULT_OLFACTORY_ACUITY if phenotype not available.
 *
 * @param creature The creature to calculate range for
 * @return Detection range in tiles
 */
int getDetectionRange(const Creature& creature);

} // namespace CreatureScent

#endif // CREATURE_SCENT_HPP
