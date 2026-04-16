/**
 * @file OrganismFactory.cpp
 * @brief Gene-driven organism construction.
 *
 * Single entry point for building Organism instances. Inspects the
 * genome's expressed capabilities and attaches only the components the
 * organism actually needs — no dormant MobilityComponent on a tree, no
 * phantom AutotrophyComponent on a predator.
 *
 * The factory is the mechanism that lets creature-ness and plant-ness
 * become emergent properties of gene expression. A creature whose
 * photosynthesis gene evolves above the autotrophy threshold will,
 * starting with its next offspring, produce organisms carrying an
 * AutotrophyComponent — no class change required.
 */

#include "genetics/organisms/OrganismFactory.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/OrganismConstants.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/classification/BiomeAdaptation.hpp"
#include "genetics/classification/CreatureTaxonomy.hpp"
#include "genetics/classification/PlantTaxonomy.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/PlantLifecycleTick.hpp"

namespace EcoSim {
namespace Genetics {

float OrganismFactory::getRawGeneValue(const Genome& genome,
                                       const std::string& geneId,
                                       float fallback) {
    if (genome.hasGene(geneId)) {
        const Gene& gene = genome.getGene(geneId);
        return gene.getNumericValue(DominanceType::Incomplete);
    }
    return fallback;
}

ComponentSignature OrganismFactory::classifyComponentSet(const Genome& genome) {
    ComponentSignature sig;

    const float locomotion     = getRawGeneValue(genome, UniversalGenes::LOCOMOTION);
    const float photosynthesis = getRawGeneValue(genome, UniversalGenes::PHOTOSYNTHESIS);
    const float meatDig        = getRawGeneValue(genome, UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    const float plantDig       = getRawGeneValue(genome, UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    const float aggression     = getRawGeneValue(genome, UniversalGenes::COMBAT_AGGRESSION);
    const float hide           = getRawGeneValue(genome, UniversalGenes::HIDE_THICKNESS);
    const float fur            = getRawGeneValue(genome, UniversalGenes::FUR_DENSITY);

    sig.mobility     = locomotion  >= AttachmentThresholds::LOCOMOTION_MIN;
    // Autotrophy: either the universal PHOTOSYNTHESIS trait is expressed,
    // or the genome carries plant-specific growth genes (plant_growth_rate
    // / plant_max_size). The latter is a bridge until plant genomes absorb
    // UniversalGenes::PHOTOSYNTHESIS — current PlantGenes::createRandomGenome
    // only seeds plant_*-prefixed genes, so we use those as a presence
    // marker for "this is a photosynthesiser".
    const bool hasPlantMarkers =
        genome.hasGene(PlantGenes::GROWTH_RATE)
     || genome.hasGene(PlantGenes::MAX_SIZE);
    sig.autotrophy   = (photosynthesis >= AttachmentThresholds::PHOTOSYNTHESIS_MIN)
                    || hasPlantMarkers;
    sig.heterotrophy = (meatDig    >= AttachmentThresholds::DIGESTION_MIN
                     || plantDig   >= AttachmentThresholds::DIGESTION_MIN);
    // Combat presence: either offensive (aggression) or defensive (hide)
    // crosses threshold. Hybrid organisms can emerge with just one.
    sig.combat       = (aggression >= AttachmentThresholds::COMBAT_MIN
                     || hide       >= AttachmentThresholds::COMBAT_MIN);
    // Thermal: fur or fat (fat_storage not wired up yet — fur is the
    // canonical thermoregulation indicator today).
    sig.thermal      = fur         >= AttachmentThresholds::THERMAL_MIN;

    return sig;
}

void OrganismFactory::attachComponents(Organism& organism,
                                       const ComponentSignature& sig) {
    const int x = organism.getX();
    const int y = organism.getY();

    // ----- Attach components per signature -----
    if (sig.mobility) {
        organism.attachMobility(std::make_unique<MobilityComponent>());
        organism.mobility()->worldX = static_cast<float>(x);
        organism.mobility()->worldY = static_cast<float>(y);
        organism.mobility()->direction = Direction::none;
    }
    if (sig.heterotrophy) {
        organism.attachHeterotrophy(std::make_unique<HeterotrophyComponent>());
        organism.heterotrophy()->hunger  = Constants::RESOURCE_LIMIT;
        organism.heterotrophy()->thirst  = Constants::RESOURCE_LIMIT;
        organism.heterotrophy()->fatigue = Constants::INIT_FATIGUE;
        if (organism.getGenome().hasGene(UniversalGenes::METABOLISM_RATE)) {
            organism.heterotrophy()->metabolism =
                organism.getPhenotype().getTrait(UniversalGenes::METABOLISM_RATE) * 0.001f;
        } else {
            organism.heterotrophy()->metabolism = 0.001f;
        }
    }
    if (sig.autotrophy) {
        organism.attachAutotrophy(std::make_unique<AutotrophyComponent>());
    }
    if (sig.combat) {
        organism.attachCombat(std::make_unique<CombatComponent>());
    }
    if (sig.thermal) {
        organism.attachThermal(std::make_unique<ThermalComponent>());
    }
    // Reproduction and identity: always attached.
    organism.attachReproduction(std::make_unique<ReproductionComponent>());
    organism.attachIdentity(std::make_unique<IdentityComponent>());

    // ----- Archetype / biome classification -----
    // Heterotroph + mobility → creature archetypes (apex predator, pack
    // hunter, etc.). Autotroph without heterotrophy → plant archetypes
    // (oak tree, grass, etc.). Everything else (hybrids) falls through
    // to the GenericPlant flyweight until Phase 4 populates the hybrid
    // catalogue (Phototroph, MobilePlant, SessileCreature).
    if (sig.heterotrophy && sig.mobility) {
        organism.identity()->archetype =
            CreatureTaxonomy::classifyArchetype(organism.getGenome());
        organism.identity()->biomeAdaptation =
            CreatureTaxonomy::classifyBiomeAdaptation(organism.getGenome());
    } else if (sig.autotrophy && !sig.heterotrophy) {
        organism.identity()->archetype =
            PlantTaxonomy::classifyArchetype(organism.getGenome());
    } else {
        organism.identity()->archetype = ArchetypeIdentity::GenericPlant();
    }

    if (organism.identity()->archetype) {
        organism.identity()->archetype->incrementPopulation();
    }
    if (organism.identity()->biomeAdaptation) {
        organism.identity()->biomeAdaptation->incrementPopulation();
    }

    // Species name generation uses diet/flock genes — only meaningful
    // for heterotrophs. Autotrophs fall through to archetype label via
    // Organism::getName().
    if (sig.heterotrophy) {
        organism.identity()->speciesName = organism.generateName();
    }

    // ----- Initial state -----
    organism.setCurrentSize(0.1f);
    organism.setHealth(organism.getMaxHealth());

    // Phenotype context (default environment — sim seeds real context
    // on first tick).
    EnvironmentState defaultEnv;
    organism.updatePhenotypeContext(defaultEnv);

    // ----- Behavior controller -----
    // Autotrophs get a BehaviorController with PlantLifecycleTick for
    // passive photosynthesis / fruit timer / environmental stress. Active
    // behaviors (Hunting, Feeding, Mating, etc.) are attached lazily on
    // first use by Creature (legacy path). Once call sites migrate to
    // Organism directly, active behaviors attach here too.
    if (sig.autotrophy) {
        auto bc = std::make_unique<BehaviorController>();
        bc->addPassiveTick(std::make_unique<PlantLifecycleTick>());
        organism.setOrganismBehaviorController(std::move(bc));
    }
}

std::unique_ptr<Organism> OrganismFactory::fromGenome(
    Genome genome, int x, int y, const GeneRegistry& registry) {

    // Classify before the genome is moved into the organism.
    const ComponentSignature sig = classifyComponentSet(genome);

    auto organism = std::make_unique<Organism>(x, y, std::move(genome), registry);
    attachComponents(*organism, sig);
    return organism;
}

} // namespace Genetics
} // namespace EcoSim
