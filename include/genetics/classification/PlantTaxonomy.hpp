#pragma once

#include "ArchetypeIdentity.hpp"
#include "genetics/core/Genome.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * @brief Stateless classification utility for plants
 *
 * Mirrors CreatureTaxonomy but for plant archetypes. Classifies a genome
 * into one of the flat ArchetypeIdentity plant flyweights (Oak Tree,
 * Grass, Thorn Bush, Berry Bush, GenericPlant) based on raw gene values.
 *
 * Shares the same ArchetypeIdentity catalogue as creatures — the unified
 * organism direction means one pointer type covers every organism, with
 * gene-derived classification selecting which archetype it maps to.
 *
 * Classification is based on genetic POTENTIAL (raw values), not current
 * phenotype expression, so a plant's archetype is stable across its life.
 */
class PlantTaxonomy {
public:
    /**
     * @brief Get shared archetype identity flyweight for a plant genome
     *
     * Classification priority (first match wins):
     *  1. Oak Tree      — large and long-lived (max_size ≥ 5.0 AND lifespan ≥ 10000)
     *  2. Grass         — short-lived or highly vegetative (lifespan < 3500 OR runner_production ≥ 0.5)
     *  3. Thorn Bush    — heavily defended (thorn_density ≥ 0.5)
     *  4. Berry Bush    — fruit specialist (fruit_appeal ≥ 0.5 OR fruit_production_rate ≥ 0.5)
     *  5. Generic Plant — fallback
     *
     * @param genome The plant's genome
     * @return Non-owning pointer to shared ArchetypeIdentity (never null)
     */
    static const ArchetypeIdentity* classifyArchetype(const Genome& genome);

private:
    PlantTaxonomy() = delete;

    static float getRawGeneValue(const Genome& genome,
                                 const std::string& geneId,
                                 float fallback = 0.5f);
};

} // namespace Genetics
} // namespace EcoSim
