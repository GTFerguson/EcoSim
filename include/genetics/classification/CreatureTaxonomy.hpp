#pragma once

#include "ArchetypeIdentity.hpp"
#include "genetics/core/Genome.hpp"
#include <string>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Named constants for creature classification thresholds
 *
 * Centralizes magic numbers used in archetype classification and
 * scientific name generation for improved maintainability and tuning.
 */
namespace ClassificationThresholds {

    // ========================================================================
    // Diet Thresholds
    // ========================================================================
    
    constexpr float CARNIVORE_MIN = 0.7f;       // Minimum meat digestion for carnivore
    constexpr float CARNIVORE_HIGH = 0.8f;      // High meat digestion (apex/ambush predator)
    constexpr float HERBIVORE_MIN = 0.7f;       // Minimum plant digestion for herbivore
    constexpr float OMNIVORE_MIN = 0.4f;        // Minimum for both diets to be omnivore
    constexpr float SCAVENGER_MEAT_MIN = 0.5f;  // Minimum meat digestion for scavenger
    constexpr float LOW_DIET_MAX = 0.3f;        // Maximum for "low" diet classification
    constexpr float SPIKY_PLANT_MIN = 0.4f;     // Plant digestion for spiky defender
    constexpr float FLEET_PLANT_MIN = 0.5f;     // Plant digestion for fleet runner

    // ========================================================================
    // Size Thresholds
    // ========================================================================
    
    constexpr float SIZE_LARGE = 2.0f;          // Large creature threshold
    constexpr float SIZE_SMALL_MAX = 1.5f;      // Maximum for "small" (pack hunters)
    
    // Species naming size brackets
    constexpr float SIZE_MINIMUS_MAX = 0.7f;    // "minimus" species
    constexpr float SIZE_MINOR_MAX = 1.0f;      // "minor" species
    constexpr float SIZE_MEDIOCRIS_MAX = 1.5f;  // "mediocris" species
    constexpr float SIZE_MAJOR_MAX = 2.0f;      // "major" species
    constexpr float SIZE_GRANDIS_MAX = 2.5f;    // "grandis" species (>=2.5 is "titan")

    // ========================================================================
    // Behavior Thresholds
    // ========================================================================
    
    constexpr float AGGRESSION_HIGH = 0.7f;     // High aggression threshold
    constexpr float AGGRESSION_VERY_HIGH = 0.8f;// Very high aggression (rex behavior)
    constexpr float AGGRESSION_LOW_MAX = 0.3f;  // Maximum for "peaceful" grazer
    constexpr float PACK_COORD_MIN = 0.7f;      // Pack coordination for social behavior
    constexpr float HUNT_INSTINCT_LOW_MAX = 0.3f;// Low hunt instinct (scavenger indicator)
    constexpr float RETREAT_HIGH = 0.7f;        // High retreat threshold (fleet runner)
    constexpr float RETREAT_VERY_HIGH = 0.8f;   // Very high retreat (timidus epithet)

    // ========================================================================
    // Stealth & Toxin Thresholds
    // ========================================================================
    
    constexpr float SCENT_MASKING_MIN = 0.6f;   // Scent masking for ambush/insidia
    constexpr float TOXIN_TOLERANCE_MIN = 0.7f; // Toxin tolerance for scavenger

    // ========================================================================
    // Physical Trait Thresholds
    // ========================================================================
    
    constexpr float HIDE_MIN = 0.7f;            // Hide thickness for armored types
    constexpr float SCALES_MIN = 0.7f;          // Scale coverage for armored grazer
    constexpr float HORN_LENGTH_MIN = 0.6f;     // Horn length for tank herbivore
    constexpr float SPINES_MIN = 0.5f;          // Spine threshold for spiky defender
    constexpr float COMBINED_ARMOR_MIN = 0.5f;  // Both hide and scales for "armatus"

    // ========================================================================
    // Movement Thresholds
    // ========================================================================
    
    constexpr float LOCOMOTION_FAST = 1.5f;     // Fast locomotion (pursuit/fleet runner)
    constexpr float LOCOMOTION_SLOW_MAX = 0.8f; // Maximum for "slow" (ambush predator)

    // ========================================================================
    // Sensory Thresholds (Canopy Forager)
    // ========================================================================
    
    constexpr float SWEET_PREFERENCE_MIN = 0.6f;// Sweetness preference for fruit specialist
    constexpr float COLOR_VISION_MIN = 0.7f;    // Color vision for canopy forager

    // ========================================================================
    // Trait Scoring (Scientific Name Generation)
    // ========================================================================
    
    constexpr float TRAIT_THRESHOLD = 0.5f;     // Minimum score for distinctive trait
    constexpr float COMBINED_ARMOR_BONUS = 0.1f;// Bonus for combined hide+scales
    
    // Normalization factors for traits with non-standard ranges
    constexpr float REGEN_MAX = 2.0f;           // Maximum regeneration rate
    constexpr float SIGHT_RANGE_MAX = 150.0f;   // Maximum sight range
    constexpr float LOCOMOTION_MAX = 2.0f;      // Maximum locomotion value

} // namespace ClassificationThresholds

/**
 * @brief Stateless classification utility for creatures
 * 
 * Single source of truth for organism identification.
 * All methods are static - this is a stateless utility class.
 * 
 * Design for Future:
 * - Methods take Genome& (not Creature&) for unified organism taxonomy
 * - Same classification can apply to creatures and eventually plants
 * 
 * Naming Convention:
 * - classifyX() - Returns category/type based on genome analysis
 * - generateX() - Creates new string from genome analysis
 * 
 * Uses RAW genetic values (not modulated by age/energy/health)
 * so classification remains stable throughout organism's life.
 * A newborn apex predator is still an apex predator genetically!
 */
class CreatureTaxonomy {
public:
    /**
     * @brief Get shared archetype identity flyweight for a genome
     * @param genome The creature's genome
     * @return Non-owning pointer to shared ArchetypeIdentity (never null)
     * 
     * Uses RAW gene values (not age/health modulated) to determine
     * the archetype. Classification is based on genetic POTENTIAL,
     * not current phenotype expression.
     * 
     * Classification Priority (checked in order):
     * 1. Apex Predator: meatDig > 0.8, size > 2.0, aggression > 0.7
     * 2. Pack Hunter: meatDig > 0.7, packCoord > 0.7, size < 1.5
     * 3. Ambush Predator: meatDig > 0.8, scentMasking > 0.6, locomotion < 0.8
     * 4. Pursuit Hunter: meatDig > 0.7, locomotion > 1.5
     * 5. Scavenger: meatDig > 0.5, huntInstinct < 0.3, toxinTol > 0.7
     * 6. Tank Herbivore: plantDig > 0.7, hide > 0.7, size > 2.0, hornLen > 0.6
     * 7. Armored Grazer: plantDig > 0.7, scales > 0.7
     * 8. Spiky Defender: (bodySpines > 0.5 || tailSpines > 0.5), plantDig > 0.4
     * 9. Fleet Runner: plantDig > 0.5, locomotion > 1.5, retreat > 0.7
     * 10. Omnivore Generalist: default
     */
    static const ArchetypeIdentity* classifyArchetype(const Genome& genome);
    
    /**
     * @brief Generate a scientific name based on genetics
     * 
     * Format: "Genus species epithet" (e.g., "Carnopredax titan dentatus")
     * 
     * Structure:
     * - Genus: Diet prefix + behavior suffix
     *   - Diet prefix: Necro (scavenger), Carno (carnivore), Herbo (herbivore), Omni (omnivore)
     *   - Behavior suffix: rex, predax, latens, socialis, cursor, scutum, insidia, grazer, flexus
     * 
     * - Species: Size-based
     *   - minimus (<0.7), minor (<1.0), mediocris (<1.5), major (<2.0), grandis (<2.5), titan (>=2.5)
     * 
     * - Epithet: Most distinctive trait
     *   - dentatus (teeth), unguiculatus (claws), cornuatus (horns), caudatus (tail)
     *   - fortis (hide), squamatus (scales), crassus (fat), spinosus (spines)
     *   - regenerans (regen), olfactans (smell), vigilans (sight), velocis (speed)
     *   - armatus (combined armor), timidus (defensive), vulgaris (default)
     * 
     * @param genome The creature's genome
     * @return Trinomial scientific name
     */
    static std::string generateScientificName(const Genome& genome);
    
    /**
     * @brief Get raw gene value from genome (bypasses phenotype age modulation)
     * 
     * Classification must use raw genetic potential, not phenotype's modulated values.
     * This ensures a newborn apex predator is classified as an apex predator,
     * even though its phenotype may express differently due to age modulation.
     * 
     * @param genome The genome to query
     * @param geneId The gene ID to look up
     * @param fallback Value if gene not found (default: 0.5)
     * @return Raw genetic value (typically 0.0-1.0, varies by gene)
     */
    static float getRawGeneValue(const Genome& genome, const std::string& geneId, float fallback = 0.5f);
    
private:
    // Not instantiable - all static methods
    CreatureTaxonomy() = delete;
};

} // namespace Genetics
} // namespace EcoSim
