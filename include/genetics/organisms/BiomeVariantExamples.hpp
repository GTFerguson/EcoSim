#pragma once

/**
 * @file BiomeVariantExamples.hpp
 * @brief Proof-of-concept biome-specific organism variants
 *
 * Provides factory functions that create creatures and plants adapted to
 * specific biomes by starting with base archetypes and overriding thermal
 * genes for extreme environments.
 *
 * Each major biome (Tundra, Desert, Tropical) is represented by:
 * - One plant variant
 * - One herbivore variant
 * - One carnivore variant
 *
 * Thermal adaptations include:
 * - Temperature tolerance ranges
 * - Fur density (insulation)
 * - Fat layer thickness (thermal mass)
 * - Metabolism rate (heat generation)
 * - Water storage (drought tolerance)
 *
 * @see plans/biome-specific-organism-prefabs.md
 * @see docs/technical/genetics/creature-plant-coevolution.md
 */

#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include <memory>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Biome type enumeration for variant creation
 */
enum class BiomeType {
    Temperate,  ///< Default balanced climate (-5°C to +35°C)
    Tundra,     ///< Arctic/cold climate (-35°C to +10°C)
    Desert,     ///< Hot arid climate (+5°C to +50°C)
    Tropical    ///< Hot humid climate (+15°C to +45°C)
};

/**
 * @brief Factory for creating biome-specific organism variants
 *
 * This class wraps CreatureFactory and PlantFactory to provide convenient
 * methods for creating organisms adapted to specific biomes. Each variant
 * starts with a base archetype and applies biome-specific thermal gene
 * modifications.
 *
 * Usage:
 * @code
 *   auto registry = std::make_shared<GeneRegistry>();
 *   UniversalGenes::registerDefaults(*registry);
 *   
 *   BiomeVariantFactory factory(registry);
 *   
 *   // Create a tundra-adapted wolf at position (10, 20)
 *   Creature wolf = factory.createArcticWolf(10, 20);
 *   
 *   // Create a desert-adapted cactus at position (30, 40)
 *   Plant cactus = factory.createDesertCactus(30, 40);
 * @endcode
 */
class BiomeVariantFactory {
public:
    /**
     * @brief Construct factory with gene registry
     * @param registry Shared pointer to gene registry
     */
    explicit BiomeVariantFactory(std::shared_ptr<GeneRegistry> registry);

    // ========================================================================
    // Tundra Variants (-35°C to +10°C)
    // ========================================================================

    /**
     * @brief Create Arctic Wolf (Tundra Carnivore)
     * @param x X position in world
     * @param y Y position in world
     * @return Creature adapted to tundra climate
     *
     * Base: Pack Hunter archetype
     * Adaptations:
     * - Very high fur density (0.90) for insulation
     * - High fat layer (0.85) for energy reserves and warmth
     * - Elevated metabolism (1.6) for heat generation
     * - Cold tolerance: -35°C to +15°C
     */
    Creature createArcticWolf(int x, int y);

    /**
     * @brief Create Woolly Mammoth (Tundra Herbivore)
     * @param x X position in world
     * @param y Y position in world
     * @return Creature adapted to tundra climate
     *
     * Base: Tank Herbivore archetype
     * Adaptations:
     * - Maximum fur density (0.95) - thick woolly coat
     * - Very high fat layer (0.90) for extreme cold
     * - Elevated metabolism (1.4) for constant heat
     * - Cold tolerance: -40°C to +10°C
     */
    Creature createWoollyMammoth(int x, int y);

    /**
     * @brief Create Tundra Moss (Tundra Plant)
     * @param x X position in world
     * @param y Y position in world
     * @return Plant adapted to tundra climate
     *
     * Base: Grass template
     * Adaptations:
     * - Extreme cold tolerance: -40°C to +10°C
     * - Low water storage (0.30) - frozen soil limits uptake
     * - High hardiness for frost resistance
     */
    Plant createTundraMoss(int x, int y);

    // ========================================================================
    // Desert Variants (+5°C to +50°C)
    // ========================================================================

    /**
     * @brief Create Desert Fennec (Desert Carnivore)
     * @param x X position in world
     * @param y Y position in world
     * @return Creature adapted to desert climate
     *
     * Base: Small Omnivore/Ambush Predator archetype
     * Adaptations:
     * - Minimal fur (0.10) for heat dissipation
     * - Low fat layer (0.15) - reduces heat retention
     * - Reduced metabolism (0.70) for energy conservation
     * - Heat tolerance: +5°C to +50°C
     */
    Creature createDesertFennec(int x, int y);

    /**
     * @brief Create Desert Camel (Desert Herbivore)
     * @param x X position in world
     * @param y Y position in world
     * @return Creature adapted to desert climate
     *
     * Base: Tank Herbivore archetype
     * Adaptations:
     * - Low fur (0.20) - light coat
     * - High fat layer (0.80) - energy reserves for food scarcity
     * - Reduced metabolism (0.75) for water conservation
     * - Heat tolerance: +0°C to +55°C
     */
    Creature createDesertCamel(int x, int y);

    /**
     * @brief Create Desert Cactus (Desert Plant)
     * @param x X position in world
     * @param y Y position in world
     * @return Plant adapted to desert climate
     *
     * Base: Thorn Bush template
     * Adaptations:
     * - Extreme heat tolerance: +5°C to +55°C
     * - Maximum water storage (0.95) for drought survival
     * - High thorn density for herbivore defense
     */
    Plant createDesertCactus(int x, int y);

    // ========================================================================
    // Tropical Variants (+15°C to +45°C)
    // ========================================================================

    /**
     * @brief Create Tropical Jaguar (Tropical Carnivore)
     * @param x X position in world
     * @param y Y position in world
     * @return Creature adapted to tropical climate
     *
     * Base: Ambush Predator archetype
     * Adaptations:
     * - Minimal fur (0.15) for heat management
     * - Low fat layer (0.20) - no cold stress
     * - Slightly reduced metabolism (0.85)
     * - Heat tolerance: +15°C to +45°C
     */
    Creature createTropicalJaguar(int x, int y);

    /**
     * @brief Create Jungle Elephant (Tropical Herbivore)
     * @param x X position in world
     * @param y Y position in world
     * @return Creature adapted to tropical climate
     *
     * Base: Tank Herbivore archetype
     * Adaptations:
     * - Minimal fur (0.10) - no insulation needed
     * - Low fat layer (0.25) - warm year-round
     * - Standard metabolism (0.95)
     * - Heat tolerance: +10°C to +45°C
     */
    Creature createJungleElephant(int x, int y);

    /**
     * @brief Create Rainforest Vine (Tropical Plant)
     * @param x X position in world
     * @param y Y position in world
     * @return Plant adapted to tropical climate
     *
     * Base: Berry Bush template
     * Adaptations:
     * - Warm temperature tolerance: +15°C to +40°C
     * - Moderate water storage (0.40) - abundant rainfall
     * - High growth rate for canopy competition
     */
    Plant createRainforestVine(int x, int y);

    // ========================================================================
    // Generic Biome Adaptation
    // ========================================================================

    /**
     * @brief Apply biome-specific thermal adaptations to a creature genome
     * @param genome Genome to modify
     * @param biome Target biome type
     *
     * Modifies thermal genes based on biome requirements:
     * - Tundra: High insulation, elevated metabolism
     * - Desert: Low insulation, reduced metabolism
     * - Tropical: Minimal insulation, standard metabolism
     */
    static void applyBiomeAdaptation(Genome& genome, BiomeType biome);

    /**
     * @brief Get temperature range for a biome
     * @param biome Target biome type
     * @return Pair of (min_temp, max_temp) in Celsius
     */
    static std::pair<float, float> getBiomeTemperatureRange(BiomeType biome);

private:
    std::shared_ptr<GeneRegistry> registry_;
    CreatureFactory creatureFactory_;
    PlantFactory plantFactory_;

    /**
     * @brief Apply thermal gene overrides to creature genome
     * @param creature Creature to modify
     * @param furDensity Fur/insulation value [0.0, 1.0]
     * @param fatLayer Fat layer thickness [0.0, 1.0]
     * @param metabolism Metabolism rate multiplier
     * @param tempMin Minimum survivable temperature
     * @param tempMax Maximum survivable temperature
     */
    void applyThermalOverrides(Creature& creature, float furDensity, float fatLayer,
                               float metabolism, float tempMin, float tempMax);

    /**
     * @brief Apply thermal gene overrides to plant genome
     * @param plant Plant to modify
     * @param waterStorage Water storage capacity [0.0, 1.0]
     * @param tempMin Minimum survivable temperature
     * @param tempMax Maximum survivable temperature
     */
    void applyThermalOverrides(Plant& plant, float waterStorage,
                               float tempMin, float tempMax);
};

} // namespace Genetics
} // namespace EcoSim
