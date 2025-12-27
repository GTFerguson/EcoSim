#pragma once

#include "ArchetypeIdentity.hpp"
#include "genetics/core/Genome.hpp"
#include <string>

namespace EcoSim {
namespace Genetics {

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
