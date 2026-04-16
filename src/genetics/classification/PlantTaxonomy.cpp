/**
 * @file PlantTaxonomy.cpp
 * @brief Plant archetype classifier — selects ArchetypeIdentity flyweight
 *        from raw genetic values.
 */

#include "genetics/classification/PlantTaxonomy.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"

namespace EcoSim {
namespace Genetics {

float PlantTaxonomy::getRawGeneValue(const Genome& genome,
                                     const std::string& geneId,
                                     float fallback) {
    if (genome.hasGene(geneId)) {
        const Gene& gene = genome.getGene(geneId);
        return gene.getNumericValue(DominanceType::Incomplete);
    }
    return fallback;
}

const ArchetypeIdentity* PlantTaxonomy::classifyArchetype(const Genome& genome) {
    const float maxSize       = getRawGeneValue(genome, PlantGenes::MAX_SIZE,    1.0f);
    const float lifespan      = getRawGeneValue(genome, UniversalGenes::LIFESPAN, 5000.0f);
    const float runner        = getRawGeneValue(genome, UniversalGenes::RUNNER_PRODUCTION, 0.0f);
    const float thorns        = getRawGeneValue(genome, UniversalGenes::THORN_DENSITY, 0.0f);
    const float fruitAppeal   = getRawGeneValue(genome, UniversalGenes::FRUIT_APPEAL, 0.0f);
    const float fruitRate     = getRawGeneValue(genome, UniversalGenes::FRUIT_PRODUCTION_RATE, 0.0f);

    if (maxSize >= 5.0f && lifespan >= 10000.0f) {
        return ArchetypeIdentity::OakTree();
    }

    if (lifespan < 3500.0f || runner >= 0.5f) {
        return ArchetypeIdentity::Grass();
    }

    if (thorns >= 0.5f) {
        return ArchetypeIdentity::ThornBush();
    }

    if (fruitAppeal >= 0.5f || fruitRate >= 0.5f) {
        return ArchetypeIdentity::BerryBush();
    }

    return ArchetypeIdentity::GenericPlant();
}

} // namespace Genetics
} // namespace EcoSim
