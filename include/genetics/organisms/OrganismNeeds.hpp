#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * @brief Shared needs state for all organisms
 *
 * Both plants and creatures have needs — they satisfy them through
 * different behaviors. Moving needs out of Creature's private members
 * into Organism enables behaviors (operating on Organism&) to read
 * and write the state they need to drive decisions.
 */
struct OrganismNeeds {
    float energy = 10.0f;          ///< Creature: food level. Plant: photosynthetic reserves.
    float hydration = 10.0f;       ///< Both need water.
    float fatigue = 0.0f;          ///< Creature: activity cost. Plant: stays 0.
    float reproductiveUrge = 0.0f; ///< Creature: mating drive. Plant: seed readiness.
    float maxEnergy = 10.0f;
    float maxHydration = 10.0f;

    /// Energy as fraction of max (0.0 = starving, 1.0 = full)
    float energyRatio() const {
        return maxEnergy > 0.0f ? energy / maxEnergy : 0.0f;
    }

    /// Hydration as fraction of max (0.0 = parched, 1.0 = full)
    float hydrationRatio() const {
        return maxHydration > 0.0f ? hydration / maxHydration : 0.0f;
    }
};

} // namespace Genetics
} // namespace EcoSim
