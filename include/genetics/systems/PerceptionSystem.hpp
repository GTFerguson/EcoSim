/**
 * @file PerceptionSystem.hpp
 * @brief Organism-agnostic perception system for scent emission and detection
 *
 * Provides unified perception calculations that work for ANY organism type
 * through the IGeneticOrganism interface, supporting the Unified Organism vision.
 *
 * Key Design Principles:
 * - NO type-specific code (no Plant, Creature, or DietType references)
 * - All organism queries use IGeneticOrganism& and phenotype traits
 * - Integrates with existing ScentLayer and its 8-element signature array
 * - Position passed as parameters (IGeneticOrganism doesn't expose position)
 *
 * @see docs/code-review/recommendations/creature-decomposition-plan.md (lines 714-778)
 * @see plans/color-vision-detection-analysis.md (lines 67-125)
 */

#ifndef PERCEPTION_SYSTEM_HPP
#define PERCEPTION_SYSTEM_HPP

#include <array>
#include <optional>
#include <utility>
#include <cmath>
#include <algorithm>
#include <string>

// Forward declarations - organism-agnostic interfaces only
namespace EcoSim {
    class ScentLayer;
    enum class ScentType;
    struct ScentDeposit;

namespace Genetics {
    class IGeneticOrganism;
    class Phenotype;
} // namespace Genetics
} // namespace EcoSim

namespace EcoSim {
namespace Genetics {

/**
 * @brief Unified perception system for organisms
 *
 * Provides organism-agnostic methods for:
 * - Scent signature generation from phenotype traits
 * - Scent emission into ScentLayer
 * - Food and mate detection via scent
 * - Visual and scent range calculations
 *
 * Shared between ALL organism types (creatures, plants, future organism types).
 * Uses phenotype traits exclusively - no type casting or type-specific code.
 *
 * @note This class supports the Unified Organism vision where capabilities
 * emerge from genetics rather than hard-coded organism types.
 */
class PerceptionSystem {
public:
    /**
     * @brief Default constructor
     */
    PerceptionSystem() = default;
    
    /**
     * @brief Destructor
     */
    ~PerceptionSystem() = default;
    
    // =========================================================================
    // Scent Signature Generation
    // =========================================================================
    
    /**
     * @brief Build 8-element scent signature from any organism's phenotype
     *
     * Signature indices (consistent for ALL organisms):
     * - [0] nutrition     - getTrait("nutrient_value") / 100
     * - [1] fruit_appeal  - getTrait("fruit_appeal")
     * - [2] toxicity      - getTrait("toxicity")
     * - [3] hardiness     - getTrait("hardiness")
     * - [4] scent_production - getTrait("scent_production")
     * - [5] color_hue     - getTrait("color_hue")
     * - [6] size_gene     - getTrait("size_gene")
     * - [7] reserved      - 0.0f
     *
     * @param organism Any organism implementing IGeneticOrganism
     * @return 8-element scent signature array compatible with ScentDeposit
     *
     * @note Missing traits default to 0.0f, allowing this to work for
     *       organisms that don't have all traits (e.g., plants without size_gene)
     */
    std::array<float, 8> buildScentSignature(const IGeneticOrganism& organism) const;
    
    // =========================================================================
    // Scent Emission
    // =========================================================================
    
    /**
     * @brief Deposit scent at specified position into ScentLayer
     *
     * Works for ANY organism type - the scent signature is built from
     * the organism's phenotype traits, not its concrete type.
     *
     * @param organism The organism emitting scent (provides phenotype for signature)
     * @param posX X position to deposit at
     * @param posY Y position to deposit at
     * @param sourceId Unique ID of the organism (use negative for plants by convention)
     * @param scentLayer The scent layer to deposit into
     * @param type ScentType (MATE_SEEKING, FOOD_TRAIL, etc.)
     * @param currentTick Current simulation tick for decay calculation
     *
     * @note If organism's scent_production trait is below threshold (0.05),
     *       no scent is deposited. This allows some organisms to be "scentless".
     */
    void depositScent(
        const IGeneticOrganism& organism,
        int posX, int posY,
        int sourceId,
        ScentLayer& scentLayer,
        ScentType type,
        unsigned int currentTick) const;
    
    // =========================================================================
    // Scent Detection
    // =========================================================================
    
    /**
     * @brief Find direction to strongest food scent the seeker would eat
     *
     * Searches the ScentLayer for FOOD_TRAIL scents within the seeker's
     * scent detection range. Uses phenotype-based diet checking to filter
     * scents to those the seeker can actually eat.
     *
     * @param seeker The organism looking for food
     * @param seekerX Current X position
     * @param seekerY Current Y position
     * @param scentLayer The scent layer to search
     * @return Optional (targetX, targetY) if food scent found, nullopt otherwise
     *
     * @note The seeker's scent_detection trait determines the search radius:
     *       radius = scent_detection * 100 (so 0.5 = 50 tiles)
     */
    std::optional<std::pair<int, int>> detectFoodDirection(
        const IGeneticOrganism& seeker,
        int seekerX, int seekerY,
        const ScentLayer& scentLayer) const;
    
    /**
     * @brief Find direction to mate-seeking scent
     *
     * Searches for MATE_SEEKING scents from potential mates. Excludes
     * the seeker's own scent (by sourceId check).
     *
     * @param seeker The organism looking for a mate
     * @param seekerX Current X position
     * @param seekerY Current Y position
     * @param seekerId Seeker's ID (to avoid detecting own scent)
     * @param scentLayer The scent layer to search
     * @return Optional (targetX, targetY) if mate scent found, nullopt otherwise
     */
    std::optional<std::pair<int, int>> detectMateDirection(
        const IGeneticOrganism& seeker,
        int seekerX, int seekerY,
        int seekerId,
        const ScentLayer& scentLayer) const;
    
    // =========================================================================
    // Edibility Checking
    // =========================================================================
    
    /**
     * @brief Check if a scent signature indicates edible food for this organism
     *
     * Uses PHENOTYPE traits for diet checking - no DietType enum or type casting.
     *
     * Checks:
     * - toxin_resistance vs signature[2] (toxicity) - can they tolerate it?
     * - hardiness vs signature[3] (can they chew it?)
     * - plant_digestion trait - can they digest plant matter?
     * - meat_digestion trait - can they digest meat? (for future creature scents)
     *
     * @param signature The 8-element scent signature to evaluate
     * @param eater The organism considering eating
     * @return true if the scent indicates compatible food
     *
     * @note This is a pure phenotype-based check. An organism can "eat" something
     *       if it has sufficient toxin_resistance and appropriate digestion traits.
     */
    bool isEdibleScent(
        const std::array<float, 8>& signature,
        const IGeneticOrganism& eater) const;
    
    // =========================================================================
    // Detection Range Calculations
    // =========================================================================
    
    /**
     * @brief Calculate visual detection range based on seeker's phenotype
     *
     * Formula: sightRange + (colorVision * targetColorfulness * 100)
     *
     * High color vision helps spot colorful targets (high fruit_appeal, color_hue).
     * Base sight range applies to all targets regardless of color.
     *
     * @param seeker The organism doing the looking
     * @param targetColorfulness How colorful/visible the target is (0-1)
     * @return Visual detection range in world units (tiles)
     */
    float calculateVisualRange(
        const IGeneticOrganism& seeker,
        float targetColorfulness) const;
    
    /**
     * @brief Calculate scent detection range from seeker's phenotype
     *
     * Formula: scent_detection * 100 (so trait of 0.5 = 50 tiles)
     *
     * @param seeker The organism doing the smelling
     * @return Scent detection range in world units (tiles)
     */
    float calculateScentRange(const IGeneticOrganism& seeker) const;
    
    /**
     * @brief Calculate effective detection range (max of visual and scent)
     *
     * Detection occurs if target is within EITHER visual OR scent range.
     *
     * @param seeker The organism perceiving
     * @param targetColorfulness How colorful the target is (for visual bonus)
     * @return Effective detection range in world units
     */
    float calculateEffectiveRange(
        const IGeneticOrganism& seeker,
        float targetColorfulness) const;
    
    // =========================================================================
    // Utility Methods
    // =========================================================================
    
    /**
     * @brief Calculate Euclidean distance between two points
     * @param x1 First point X
     * @param y1 First point Y
     * @param x2 Second point X
     * @param y2 Second point Y
     * @return Distance between points
     */
    static float calculateDistance(float x1, float y1, float x2, float y2);
    
    /**
     * @brief Check if a position is within detection range
     * @param seekerX Seeker X position
     * @param seekerY Seeker Y position
     * @param targetX Target X position
     * @param targetY Target Y position
     * @param range Detection range
     * @return true if target is within range
     */
    static bool isWithinRange(
        float seekerX, float seekerY,
        float targetX, float targetY,
        float range);
    
    /**
     * @brief Calculate scent signature similarity
     *
     * Used for mate compatibility, kin recognition, etc.
     *
     * @param sig1 First signature
     * @param sig2 Second signature
     * @return Similarity score (0-1, higher = more similar)
     */
    float calculateSignatureSimilarity(
        const std::array<float, 8>& sig1,
        const std::array<float, 8>& sig2) const;
    
private:
    // =========================================================================
    // Constants
    // =========================================================================
    
    /// Multiplier for color vision range bonus
    static constexpr float COLOR_VISION_RANGE_MULTIPLIER = 100.0f;
    
    /// Multiplier for scent detection range
    static constexpr float SCENT_RANGE_MULTIPLIER = 100.0f;
    
    /// Minimum scent production to emit any scent
    static constexpr float MIN_SCENT_PRODUCTION = 0.05f;
    
    /// Default scent decay rate in ticks
    static constexpr unsigned int DEFAULT_DECAY_RATE = 50;
    
    // =========================================================================
    // Helper Methods
    // =========================================================================
    
    /**
     * @brief Safely get a trait from phenotype, returning default if not present
     * @param phenotype The phenotype to query
     * @param traitName Name of the trait
     * @param defaultValue Value to return if trait doesn't exist
     * @return Trait value or default
     */
    float getTraitSafe(
        const Phenotype& phenotype,
        const std::string& traitName,
        float defaultValue = 0.0f) const;
    
    /**
     * @brief Clamp value to range [0, 1]
     */
    static float clamp01(float value) {
        return std::max(0.0f, std::min(1.0f, value));
    }
};

} // namespace Genetics
} // namespace EcoSim

#endif // PERCEPTION_SYSTEM_HPP
