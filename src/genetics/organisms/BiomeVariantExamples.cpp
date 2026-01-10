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

void BiomeVariantFactory::applyThermalOverrides(Creature& creature, float furDensity,
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

Creature BiomeVariantFactory::createArcticWolf(int x, int y) {
    // Start with Pack Hunter archetype (coordinated group hunters)
    Creature wolf = creatureFactory_->createPackHunter(x, y);

    // Apply arctic thermal adaptations
    // Very high insulation for -40°C survival
    applyThermalOverrides(wolf,
        0.90f,   // furDensity - thick double coat
        0.85f,   // fatLayer - substantial reserves
        1.6f,    // metabolism - elevated for heat generation
        -35.0f,  // tempMin - arctic winter
        15.0f    // tempMax - struggles in warmth
    );

    return wolf;
}

Creature BiomeVariantFactory::createWoollyMammoth(int x, int y) {
    // Start with Tank Herbivore archetype (large, armored)
    Creature mammoth = creatureFactory_->createTankHerbivore(x, y);

    // Apply extreme cold adaptations
    // Maximum insulation for ice age survival
    applyThermalOverrides(mammoth,
        0.95f,   // furDensity - maximum woolly coat
        0.90f,   // fatLayer - extreme cold reserves
        1.4f,    // metabolism - constant heat generation
        -40.0f,  // tempMin - extreme cold tolerance
        10.0f    // tempMax - overheats easily
    );

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

Creature BiomeVariantFactory::createDesertFennec(int x, int y) {
    // Start with Ambush Predator (patient, efficient hunter)
    Creature fennec = creatureFactory_->createAmbushPredator(x, y);

    // Apply desert thermal adaptations
    // Minimal insulation, heat dissipation focus
    applyThermalOverrides(fennec,
        0.10f,   // furDensity - minimal, just sun protection
        0.15f,   // fatLayer - lean body dissipates heat
        0.70f,   // metabolism - reduced for water conservation
        5.0f,    // tempMin - struggles in cold
        50.0f    // tempMax - extreme heat tolerance
    );

    return fennec;
}

Creature BiomeVariantFactory::createDesertCamel(int x, int y) {
    // Start with Tank Herbivore archetype
    Creature camel = creatureFactory_->createTankHerbivore(x, y);

    // Apply desert adaptations
    // Specialized for water conservation and heat tolerance
    applyThermalOverrides(camel,
        0.20f,   // furDensity - light coat reflects sun
        0.80f,   // fatLayer - stored in hump for energy, not insulation
        0.75f,   // metabolism - reduced for water conservation
        0.0f,    // tempMin - tolerates cold desert nights
        55.0f    // tempMax - extreme heat tolerance
    );

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

Creature BiomeVariantFactory::createTropicalJaguar(int x, int y) {
    // Start with Ambush Predator archetype (patient, powerful)
    Creature jaguar = creatureFactory_->createAmbushPredator(x, y);

    // Apply tropical thermal adaptations
    // Minimal insulation, adapted to constant warmth
    applyThermalOverrides(jaguar,
        0.15f,   // furDensity - short coat for heat dissipation
        0.20f,   // fatLayer - minimal, no cold stress
        0.85f,   // metabolism - slightly reduced
        15.0f,   // tempMin - struggles in cold
        45.0f    // tempMax - high heat tolerance
    );

    return jaguar;
}

Creature BiomeVariantFactory::createJungleElephant(int x, int y) {
    // Start with Tank Herbivore archetype
    Creature elephant = creatureFactory_->createTankHerbivore(x, y);

    // Apply tropical adaptations
    // Large body, minimal insulation, uses ears for cooling
    applyThermalOverrides(elephant,
        0.10f,   // furDensity - nearly hairless
        0.25f,   // fatLayer - minimal for tropical climate
        0.95f,   // metabolism - near standard
        10.0f,   // tempMin - some cold tolerance
        45.0f    // tempMax - good heat tolerance
    );

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
