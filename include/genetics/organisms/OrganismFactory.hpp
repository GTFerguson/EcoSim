#pragma once

#include "genetics/core/Genome.hpp"

#include <memory>
#include <string>

namespace EcoSim {
namespace Genetics {

class Organism;
class GeneRegistry;

/**
 * @brief Summary of which components an organism will carry, derived
 *        purely from its genome's expressed capabilities.
 *
 * Used by OrganismFactory to make attachment decisions and exposed by
 * the public API so tests and diagnostics can ask "given this genome,
 * what would an organism look like?" without actually building one.
 *
 * reproduction and identity are always present — every organism can
 * reproduce (even if only asexually once in its life) and every
 * organism carries an identity slot for the archetype flyweight and
 * cached name. The others are gene-gated.
 */
struct ComponentSignature {
    bool mobility     = false;   ///< LOCOMOTION expressed → can move
    bool heterotrophy = false;   ///< digestion genes expressed → eats food
    bool autotrophy   = false;   ///< PHOTOSYNTHESIS expressed → produces own energy
    bool combat       = false;   ///< aggression / defensive traits expressed
    bool thermal      = false;   ///< active thermoregulation expressed
    bool reproduction = true;    ///< always attached
    bool identity     = true;    ///< always attached
};

/**
 * @brief Raw-gene-value thresholds that gate component attachment.
 *
 * All values read via DominanceType::Incomplete (genetic POTENTIAL, not
 * phenotype-modulated expression) so classification is stable across
 * an organism's life and not perturbed by age / health / energy.
 *
 * Current thresholds are the "meaningfully expressed" lower bound —
 * defaulting to 0.1 across the board. Below this the trait is
 * considered dormant and the corresponding component is unnecessary.
 * These are tunable; tests in test_organism_factory pin the current
 * behaviour and should be updated together with any threshold change.
 */
struct AttachmentThresholds {
    static constexpr float LOCOMOTION_MIN     = 0.1f;
    static constexpr float PHOTOSYNTHESIS_MIN = 0.1f;
    static constexpr float DIGESTION_MIN      = 0.1f;
    static constexpr float COMBAT_MIN         = 0.1f;
    static constexpr float THERMAL_MIN        = 0.1f;
};

/**
 * @brief Single entry point for building Organism instances from a genome.
 *
 * Inspects gene expression to decide which components to attach. This is
 * the mechanism by which creature-ness and plant-ness become emergent
 * properties of genes rather than class identities — a creature whose
 * photosynthesis gene is expressed gets an AutotrophyComponent; a plant
 * whose locomotion gene is expressed gets a MobilityComponent.
 *
 * Creature and Plant constructors will route through this factory
 * incrementally; eventually both classes disappear and OrganismFactory
 * is the only path to creating an organism.
 *
 * Step B.1 scope: factory exists and is independently testable, but no
 * production code calls it yet. Creature/Plant ctors still do their own
 * component attachment. The factory is verified against the same
 * observable behaviour (same component set for a creature genome, same
 * for a plant genome) before migration begins.
 */
class OrganismFactory {
public:
    /**
     * @brief Classify which components a genome's expressed capabilities
     *        would produce. Pure function of the genome.
     *
     * Useful for:
     *  - Building an organism via fromGenome (uses this internally)
     *  - Asking "would this hybrid genome produce a mobile autotroph?"
     *  - Testing gene → component attachment invariants
     */
    static ComponentSignature classifyComponentSet(const Genome& genome);

    /**
     * @brief Build an Organism with the component set dictated by its
     *        genome. Attaches components, classifies archetype, seeds
     *        initial state, wires BehaviorController with gene-gated
     *        active behaviors and passive ticks.
     *
     * Returns a fully-initialised organism ready to tick. Caller owns
     * the returned pointer.
     *
     * @param genome   Genome to build from (moved into the organism)
     * @param x, y     Tile position
     * @param registry Gene registry for phenotype expression
     */
    static std::unique_ptr<Organism> fromGenome(
        Genome genome, int x, int y, const GeneRegistry& registry);

    /**
     * @brief Attach components + classify archetype + seed initial state
     *        on an already-constructed Organism.
     *
     * Transitional entry point used by Creature and Plant constructors
     * while they still exist as distinct types. Same work as fromGenome
     * minus the Organism construction itself — applied in place to an
     * organism whose Organism base has already been initialised with a
     * genome and registry.
     *
     * Once all call sites create organisms via fromGenome directly, this
     * helper and the Creature/Plant constructors that call it go away.
     *
     * @param organism Organism with Organism base already constructed
     *                 (genome_, phenotype_, registry_ populated) but no
     *                 components attached yet.
     * @param sig      Component signature to apply (caller passes the
     *                 result of classifyComponentSet).
     */
    static void attachComponents(Organism& organism,
                                 const ComponentSignature& sig);

private:
    /// Read raw (non-modulated) gene value for classification decisions.
    static float getRawGeneValue(const Genome& genome,
                                 const std::string& geneId,
                                 float fallback = 0.0f);
};

} // namespace Genetics
} // namespace EcoSim
