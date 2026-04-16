/**
 * @file BiomeVariantExamples.cpp
 * @brief Implementation of biome-specific organism variants
 *
 * Creates proof-of-concept organisms adapted to extreme biomes by
 * starting with base archetypes and overriding thermal genes.
 */

#include "genetics/organisms/BiomeVariantExamples.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructors
// ============================================================================

BiomeVariantFactory::BiomeVariantFactory(std::shared_ptr<GeneRegistry> registry)
    : registry_(std::move(registry))
    , ownedCreatureFactory_(std::make_unique<CreatureFactory>(registry_))
    , ownedPlantFactory_(std::make_unique<PlantFactory>(registry_))
    , creatureFactory_(ownedCreatureFactory_.get())
    , plantFactory_(ownedPlantFactory_.get())
{
    // Register default templates so we can create base archetypes
    creatureFactory_->registerDefaultTemplates();
    plantFactory_->registerDefaultTemplates();
}

BiomeVariantFactory::BiomeVariantFactory(std::shared_ptr<GeneRegistry> registry,
                                         CreatureFactory* creatureFactory,
                                         PlantFactory* plantFactory)
    : registry_(std::move(registry))
    , ownedCreatureFactory_(nullptr)
    , ownedPlantFactory_(nullptr)
    , creatureFactory_(creatureFactory)
    , plantFactory_(plantFactory)
{
    // Use existing factories - no template registration needed as they should already be set up
}

// ============================================================================
// Private Helpers
// ============================================================================

void BiomeVariantFactory::applyThermalOverrides(Organism& creature, float furDensity,
                                                 float fatLayer, float metabolism,
                                                 float tempMin, float tempMax) {
    Genome& genome = creature.getGenomeMutable();

    // Override thermal genes with biome-specific values
    if (genome.hasGene(UniversalGenes::FUR_DENSITY)) {
        genome.getGeneMutable(UniversalGenes::FUR_DENSITY).setAlleleValues(furDensity);
    }
    if (genome.hasGene(UniversalGenes::FAT_LAYER_THICKNESS)) {
        genome.getGeneMutable(UniversalGenes::FAT_LAYER_THICKNESS).setAlleleValues(fatLayer);
    }
    if (genome.hasGene(UniversalGenes::METABOLISM_RATE)) {
        genome.getGeneMutable(UniversalGenes::METABOLISM_RATE).setAlleleValues(metabolism);
    }
    if (genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW)) {
        genome.getGeneMutable(UniversalGenes::TEMP_TOLERANCE_LOW).setAlleleValues(tempMin);
    }
    if (genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH)) {
        genome.getGeneMutable(UniversalGenes::TEMP_TOLERANCE_HIGH).setAlleleValues(tempMax);
    }
}

void BiomeVariantFactory::applyThermalOverrides(Plant& plant, float waterStorage,
                                                 float tempMin, float tempMax) {
    Genome& genome = plant.getGenomeMutable();

    // Override thermal genes with biome-specific values
    if (genome.hasGene(UniversalGenes::WATER_STORAGE)) {
        genome.getGeneMutable(UniversalGenes::WATER_STORAGE).setAlleleValues(waterStorage);
    }
    if (genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW)) {
        genome.getGeneMutable(UniversalGenes::TEMP_TOLERANCE_LOW).setAlleleValues(tempMin);
    }
    if (genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH)) {
        genome.getGeneMutable(UniversalGenes::TEMP_TOLERANCE_HIGH).setAlleleValues(tempMax);
    }
}

// ============================================================================
// Tundra Variants
// ============================================================================

OrganismPtr BiomeVariantFactory::createArcticWolf(int x, int y) {
    OrganismPtr wolf = creatureFactory_->createPackHunter(x, y);

    applyThermalOverrides(*wolf,
        0.90f,
        0.85f,
        1.6f,
        -35.0f,
        15.0f
    );

    wolf->reclassifyBiomeAdaptation();

    return wolf;
}

OrganismPtr BiomeVariantFactory::createWoollyMammoth(int x, int y) {
    OrganismPtr mammoth = creatureFactory_->createTankHerbivore(x, y);

    applyThermalOverrides(*mammoth,
        0.95f,
        0.90f,
        1.4f,
        -40.0f,
        10.0f
    );

    mammoth->reclassifyBiomeAdaptation();

    return mammoth;
}

Plant BiomeVariantFactory::createTundraMoss(int x, int y) {
    // Start with Grass template (fast-growing ground cover)
    Plant moss = plantFactory_->createFromTemplate("grass", x, y);

    // Apply tundra adaptations
    // Cold-hardy with antifreeze compounds
    applyThermalOverrides(moss,
        0.30f,   // waterStorage - frozen soil limits uptake
        -40.0f,  // tempMin - survives permafrost
        10.0f    // tempMax - wilts in warmth
    );

    // Increase hardiness for frost resistance
    Genome& genome = moss.getGenomeMutable();
    if (genome.hasGene(UniversalGenes::HARDINESS)) {
        genome.getGeneMutable(UniversalGenes::HARDINESS).setAlleleValues(0.85f);
    }

    return moss;
}

// ============================================================================
// Desert Variants
// ============================================================================

OrganismPtr BiomeVariantFactory::createDesertFennec(int x, int y) {
    OrganismPtr fennec = creatureFactory_->createAmbushPredator(x, y);

    applyThermalOverrides(*fennec,
        0.10f,
        0.15f,
        0.70f,
        5.0f,
        50.0f
    );

    fennec->reclassifyBiomeAdaptation();

    return fennec;
}

OrganismPtr BiomeVariantFactory::createDesertCamel(int x, int y) {
    OrganismPtr camel = creatureFactory_->createTankHerbivore(x, y);

    applyThermalOverrides(*camel,
        0.20f,
        0.80f,
        0.75f,
        0.0f,
        55.0f
    );

    camel->reclassifyBiomeAdaptation();

    return camel;
}

Plant BiomeVariantFactory::createDesertCactus(int x, int y) {
    // Start with Thorn Bush template (defensive, drought-tolerant)
    Plant cactus = plantFactory_->createFromTemplate("thorn_bush", x, y);

    // Apply desert adaptations
    // Maximum water storage, extreme heat tolerance
    applyThermalOverrides(cactus,
        0.95f,   // waterStorage - succulent water reserves
        5.0f,    // tempMin - frost-sensitive
        55.0f    // tempMax - extreme heat tolerance
    );

    // Maximize defensive thorns
    Genome& genome = cactus.getGenomeMutable();
    if (genome.hasGene(UniversalGenes::THORN_DENSITY)) {
        genome.getGeneMutable(UniversalGenes::THORN_DENSITY).setAlleleValues(0.95f);
    }

    return cactus;
}

// ============================================================================
// Tropical Variants
// ============================================================================

OrganismPtr BiomeVariantFactory::createTropicalJaguar(int x, int y) {
    OrganismPtr jaguar = creatureFactory_->createAmbushPredator(x, y);

    applyThermalOverrides(*jaguar,
        0.15f,
        0.20f,
        0.85f,
        15.0f,
        45.0f
    );

    jaguar->reclassifyBiomeAdaptation();

    return jaguar;
}

OrganismPtr BiomeVariantFactory::createJungleElephant(int x, int y) {
    OrganismPtr elephant = creatureFactory_->createTankHerbivore(x, y);

    applyThermalOverrides(*elephant,
        0.10f,
        0.25f,
        0.95f,
        10.0f,
        45.0f
    );

    elephant->reclassifyBiomeAdaptation();

    return elephant;
}

Plant BiomeVariantFactory::createRainforestVine(int x, int y) {
    // Start with Berry Bush template (fast-growing, fruit-producing)
    Plant vine = plantFactory_->createFromTemplate("berry_bush", x, y);

    // Apply tropical adaptations
    // Warm-adapted, moderate water storage (abundant rainfall)
    applyThermalOverrides(vine,
        0.40f,   // waterStorage - moderate (rain is plentiful)
        15.0f,   // tempMin - frost-sensitive
        40.0f    // tempMax - adapted to warm climate
    );

    // Increase growth rate for canopy competition
    Genome& genome = vine.getGenomeMutable();
    if (genome.hasGene(PlantGenes::GROWTH_RATE)) {
        genome.getGeneMutable(PlantGenes::GROWTH_RATE).setAlleleValues(0.75f);
    }

    return vine;
}

// ============================================================================
// Static Utilities
// ============================================================================

void BiomeVariantFactory::applyBiomeAdaptation(Genome& genome, BiomeType biome) {
    auto [tempMin, tempMax] = getBiomeTemperatureRange(biome);

    // Set temperature tolerance
    if (genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW)) {
        genome.getGeneMutable(UniversalGenes::TEMP_TOLERANCE_LOW).setAlleleValues(tempMin);
    }
    if (genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH)) {
        genome.getGeneMutable(UniversalGenes::TEMP_TOLERANCE_HIGH).setAlleleValues(tempMax);
    }

    // Apply biome-specific morphology
    switch (biome) {
        case BiomeType::Tundra:
            if (genome.hasGene(UniversalGenes::FUR_DENSITY)) {
                genome.getGeneMutable(UniversalGenes::FUR_DENSITY).setAlleleValues(0.85f);
            }
            if (genome.hasGene(UniversalGenes::FAT_LAYER_THICKNESS)) {
                genome.getGeneMutable(UniversalGenes::FAT_LAYER_THICKNESS).setAlleleValues(0.80f);
            }
            if (genome.hasGene(UniversalGenes::METABOLISM_RATE)) {
                genome.getGeneMutable(UniversalGenes::METABOLISM_RATE).setAlleleValues(1.5f);
            }
            break;

        case BiomeType::Desert:
            if (genome.hasGene(UniversalGenes::FUR_DENSITY)) {
                genome.getGeneMutable(UniversalGenes::FUR_DENSITY).setAlleleValues(0.15f);
            }
            if (genome.hasGene(UniversalGenes::FAT_LAYER_THICKNESS)) {
                genome.getGeneMutable(UniversalGenes::FAT_LAYER_THICKNESS).setAlleleValues(0.20f);
            }
            if (genome.hasGene(UniversalGenes::METABOLISM_RATE)) {
                genome.getGeneMutable(UniversalGenes::METABOLISM_RATE).setAlleleValues(0.75f);
            }
            if (genome.hasGene(UniversalGenes::WATER_STORAGE)) {
                genome.getGeneMutable(UniversalGenes::WATER_STORAGE).setAlleleValues(0.90f);
            }
            break;

        case BiomeType::Tropical:
            if (genome.hasGene(UniversalGenes::FUR_DENSITY)) {
                genome.getGeneMutable(UniversalGenes::FUR_DENSITY).setAlleleValues(0.15f);
            }
            if (genome.hasGene(UniversalGenes::FAT_LAYER_THICKNESS)) {
                genome.getGeneMutable(UniversalGenes::FAT_LAYER_THICKNESS).setAlleleValues(0.25f);
            }
            if (genome.hasGene(UniversalGenes::METABOLISM_RATE)) {
                genome.getGeneMutable(UniversalGenes::METABOLISM_RATE).setAlleleValues(0.90f);
            }
            break;

        case BiomeType::Temperate:
        default:
            // Temperate is the baseline - no modifications needed
            break;
    }
}

std::pair<float, float> BiomeVariantFactory::getBiomeTemperatureRange(BiomeType biome) {
    switch (biome) {
        case BiomeType::Tundra:
            return {-35.0f, 10.0f};
        case BiomeType::Desert:
            return {5.0f, 50.0f};
        case BiomeType::Tropical:
            return {15.0f, 45.0f};
        case BiomeType::Temperate:
        default:
            return {-5.0f, 35.0f};
    }
}

} // namespace Genetics
} // namespace EcoSim
