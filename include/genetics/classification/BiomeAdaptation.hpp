#pragma once

/**
 * @file BiomeAdaptation.hpp
 * @brief Flyweight class for creature biome adaptation identity
 * 
 * Implements a systematic naming convention: {Biome Prefix} + {Archetype Suffix}
 * Examples: "Arctic Pack", "Jungle Tyrant", "Sand Lurker"
 * 
 * This class is compositional with ArchetypeIdentity - creatures have both
 * an archetype (behavioral role) and a biome adaptation (environmental specialization).
 * 
 * Uses Meyer's Singleton pattern for thread-safe static initialization.
 * Maintains atomic population counters for O(1) census queries.
 */

#include <string>
#include <atomic>

// Forward declare to avoid circular dependency
namespace EcoSim {
namespace Genetics {
class ArchetypeIdentity;
}
}

namespace EcoSim {
namespace Genetics {

/**
 * @brief Creature biome adaptation categories for systematic naming
 * 
 * These map to world biomes but represent creature adaptations rather than
 * terrain types. A "Tundra" adaptation means the creature has cold tolerance,
 * not that it's currently standing in tundra.
 */
enum class BiomeAdaptationType {
    Temperate,   // Baseline - no prefix (original archetype names)
    Tundra,      // Cold-adapted - prefix: "Arctic" or "Frost"
    Taiga,       // Cold forest - prefix: "Boreal" or "Snow"
    Tropical,    // Hot/humid - prefix: "Jungle"
    Desert       // Hot/arid - prefix: "Dune" or "Sand"
};

/**
 * @brief Shared flyweight for creature biome adaptation identity
 * 
 * Complements ArchetypeIdentity by adding environmental specialization.
 * Combined identity = Biome Prefix + Archetype Suffix
 * 
 * Examples:
 *   Temperate + Pack Hunter = "Pack Hunter" (baseline)
 *   Tundra + Pack Hunter = "Arctic Pack"
 *   Desert + Ambush Predator = "Sand Lurker"
 * 
 * Thread Safety:
 * - All static access methods are thread-safe
 * - Population counters use atomic operations
 * 
 * Lifetime:
 * - Flyweight objects live for the duration of the program
 * - Returned pointers are NEVER null and NEVER become invalid
 * 
 * @note Callers should NOT delete the returned pointers.
 */
class BiomeAdaptation {
public:
    // ========== Predefined Biome Adaptations (Thread-Safe) ==========
    
    /**
     * @brief Temperate biome adaptation (baseline, no prefix)
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Standard environment - uses original archetype names.
     * Scientific name modifier: (none)
     */
    static const BiomeAdaptation* Temperate();
    
    /**
     * @brief Tundra biome adaptation (Arctic/Frost prefix)
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Cold-adapted with high insulation. Creatures with high fur density
     * and low temperature tolerance.
     * Scientific name modifier: "borealis"
     */
    static const BiomeAdaptation* Tundra();
    
    /**
     * @brief Taiga biome adaptation (Boreal/Snow prefix)
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Cold forest adapted. Between temperate and tundra.
     * Scientific name modifier: "borealis"
     */
    static const BiomeAdaptation* Taiga();
    
    /**
     * @brief Tropical biome adaptation (Jungle prefix)
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Hot/humid adapted with low insulation. Creatures with low fur
     * density and high heat tolerance.
     * Scientific name modifier: "tropicus"
     */
    static const BiomeAdaptation* Tropical();
    
    /**
     * @brief Desert biome adaptation (Dune/Sand prefix)
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Hot/arid adapted with water conservation. Creatures with high
     * heat tolerance and water storage.
     * Scientific name modifier: "deserti"
     */
    static const BiomeAdaptation* Desert();
    
    // ========== Identity Properties (Immutable) ==========
    
    /**
     * @brief Get unique identifier for this biome adaptation
     * @return ID like "temperate", "tundra", "desert", etc.
     */
    const std::string& getId() const { return id_; }
    
    /**
     * @brief Get primary prefix for naming (e.g., "Arctic", "Jungle", "")
     * @return Prefix string, empty for Temperate
     */
    const std::string& getPrefix() const { return prefix_; }
    
    /**
     * @brief Get alternate prefix for naming variety (e.g., "Frost", "Sand", "")
     * @return Alternate prefix string, empty if none
     */
    const std::string& getAltPrefix() const { return altPrefix_; }
    
    /**
     * @brief Get biome adaptation type enum
     * @return BiomeAdaptationType enum value
     */
    BiomeAdaptationType getBiomeType() const { return biomeType_; }
    
    /**
     * @brief Get Latin modifier for scientific naming
     * @return Latin suffix like "borealis", "tropicus", "deserti", or ""
     */
    const std::string& getLatinModifier() const { return latinModifier_; }
    
    /**
     * @brief Check if this is the baseline (Temperate) adaptation
     * @return true if Temperate, false otherwise
     */
    bool isBaseline() const { return biomeType_ == BiomeAdaptationType::Temperate; }
    
    // ========== Archetype Suffix Mapping ==========
    
    /**
     * @brief Get the systematic suffix for an archetype
     * @param archetype Pointer to archetype identity
     * @return Suffix like "Tyrant", "Pack", "Lurker", "Titan", etc.
     * 
     * Mapping:
     *   apex_predator -> "Tyrant"
     *   pack_hunter -> "Pack"
     *   ambush_predator -> "Lurker"
     *   pursuit_hunter -> "Chaser"
     *   tank_herbivore -> "Titan"
     *   armored_grazer -> "Shell"
     *   fleet_runner -> "Runner"
     *   spiky_defender -> "Spike"
     *   carrion_stalker -> "Scav"
     *   omnivore_generalist -> "Forager"
     *   canopy_forager -> "Canopy"
     */
    static std::string getArchetypeSuffix(const ArchetypeIdentity* archetype);
    
    /**
     * @brief Generate the full label combining biome and archetype
     * @param archetype Pointer to archetype identity
     * @return Full label like "Arctic Pack", "Jungle Tyrant", or "Pack Hunter"
     * 
     * For Temperate biome, returns the original archetype label.
     * For other biomes, combines prefix + suffix.
     */
    std::string getFullLabel(const ArchetypeIdentity* archetype) const;
    
    // ========== Population Tracking (Thread-Safe) ==========
    
    /**
     * @brief Increment population count for this biome adaptation
     * @note Called by Creature constructor
     */
    void incrementPopulation() const { ++population_; }
    
    /**
     * @brief Decrement population count for this biome adaptation
     * @note Called by Creature destructor
     */
    void decrementPopulation() const { --population_; }
    
    /**
     * @brief Get current population count
     * @return Number of living creatures with this biome adaptation
     */
    int getPopulation() const { return population_.load(); }
    
    // ========== Non-copyable (Flyweight Pattern) ==========
    BiomeAdaptation(const BiomeAdaptation&) = delete;
    BiomeAdaptation& operator=(const BiomeAdaptation&) = delete;

private:
    /**
     * @brief Private constructor - only static factory methods can create
     * @param id Unique identifier (e.g., "tundra")
     * @param prefix Primary naming prefix (e.g., "Arctic")
     * @param altPrefix Alternate naming prefix (e.g., "Frost")
     * @param latinModifier Scientific name modifier (e.g., "borealis")
     * @param biomeType The biome adaptation type enum
     */
    BiomeAdaptation(std::string id, std::string prefix, std::string altPrefix,
                    std::string latinModifier, BiomeAdaptationType biomeType);
    
    std::string id_;
    std::string prefix_;
    std::string altPrefix_;
    std::string latinModifier_;
    BiomeAdaptationType biomeType_;
    mutable std::atomic<int> population_{0};
};

} // namespace Genetics
} // namespace EcoSim
