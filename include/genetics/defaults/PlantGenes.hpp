#pragma once

#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/Genome.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * @brief Default gene definitions for plants
 * 
 * Plants use a subset of chromosomes focused on:
 * - Morphology (size, structure, visual appearance)
 * - Metabolism (photosynthesis, water needs, light requirements)
 * - Reproduction (seed production, spreading distance)
 * - Environmental (temperature tolerance, hardiness)
 * - Lifespan (how long the plant lives)
 * 
 * Unlike creatures, plants don't have Sensory, Locomotion, or Behavior chromosomes
 * since they don't move or exhibit complex behaviors.
 */
class PlantGenes {
public:
    /**
     * @brief Populate a registry with plant-specific gene definitions
     * @param registry The registry to populate with plant gene definitions
     * 
     * This creates 12 plant genes across 5 chromosomes.
     */
    static void registerDefaults(GeneRegistry& registry);
    
    /**
     * @brief Create a new plant Genome with random gene values
     * @param registry The registry containing plant gene definitions
     * @return A new Genome with randomized plant gene values within limits
     */
    static Genome createRandomGenome(const GeneRegistry& registry);
    
    /**
     * @brief Create a plant Genome with balanced default values
     * @param registry The registry containing plant gene definitions
     * @return A new Genome with default plant values
     */
    static Genome createDefaultGenome(const GeneRegistry& registry);
    
    // ========================================================================
    // Plant Gene IDs
    // ========================================================================
    
    // Metabolism Chromosome - Photosynthesis and resource needs
    /// How fast the plant grows per tick (affects size increase rate)
    static constexpr const char* GROWTH_RATE = "plant_growth_rate";
    
    /// Water requirements (0.0 = drought resistant, 1.0 = water loving)
    static constexpr const char* WATER_NEED = "plant_water_need";
    
    /// Light requirements (0.0 = shade tolerant, 1.0 = full sun required)
    static constexpr const char* LIGHT_NEED = "plant_light_need";
    
    /// Nutritional value when eaten by herbivores
    static constexpr const char* NUTRIENT_VALUE = "plant_nutrient_value";
    
    // Morphology Chromosome - Physical structure and appearance
    /// Maximum size the plant can reach (in abstract units)
    static constexpr const char* MAX_SIZE = "plant_max_size";
    
    /// Resistance to damage from herbivores and environment (0.0 to 1.0)
    static constexpr const char* HARDINESS = "plant_hardiness";
    
    /// HSV hue for visual appearance (0-360 degrees)
    static constexpr const char* COLOR_HUE = "plant_color_hue";
    
    // Reproduction Chromosome - Seed production and dispersal
    /// Number of seeds produced per growth cycle
    static constexpr const char* SEED_PRODUCTION = "plant_seed_production";
    
    /// How far seeds can spread from parent plant (in tiles)
    static constexpr const char* SPREAD_DISTANCE = "plant_spread_distance";
    
    // Environmental Chromosome - Climate tolerance
    /// @deprecated Use UniversalGenes::TEMP_TOLERANCE_LOW instead
    /// Minimum temperature the plant can survive (Celsius)
    /// Note: PlantFactory now uses UniversalGenes versions for consistency
    [[deprecated("Use UniversalGenes::TEMP_TOLERANCE_LOW instead")]]
    static constexpr const char* TEMP_TOLERANCE_LOW = "plant_temp_tolerance_low";
    
    /// @deprecated Use UniversalGenes::TEMP_TOLERANCE_HIGH instead
    /// Maximum temperature the plant can survive (Celsius)
    /// Note: PlantFactory now uses UniversalGenes versions for consistency
    [[deprecated("Use UniversalGenes::TEMP_TOLERANCE_HIGH instead")]]
    static constexpr const char* TEMP_TOLERANCE_HIGH = "plant_temp_tolerance_high";
    
    // Lifespan Chromosome - Longevity
    /// Maximum age in ticks before the plant dies naturally
    static constexpr const char* LIFESPAN = "plant_lifespan";

private:
    // Default values for balanced plant genome
    static constexpr float DEFAULT_GROWTH_RATE = 0.5f;
    static constexpr float DEFAULT_MAX_SIZE = 5.0f;
    static constexpr float DEFAULT_WATER_NEED = 0.5f;
    static constexpr float DEFAULT_LIGHT_NEED = 0.5f;
    static constexpr float DEFAULT_NUTRIENT_VALUE = 25.0f;
    static constexpr float DEFAULT_SEED_PRODUCTION = 3.0f;
    static constexpr float DEFAULT_SPREAD_DISTANCE = 5.0f;
    static constexpr float DEFAULT_TEMP_LOW = 5.0f;    // Celsius
    static constexpr float DEFAULT_TEMP_HIGH = 35.0f;  // Celsius
    static constexpr float DEFAULT_LIFESPAN = 5000.0f; // Ticks
    static constexpr float DEFAULT_HARDINESS = 0.5f;
    static constexpr float DEFAULT_COLOR_HUE = 120.0f; // Green
};

} // namespace Genetics
} // namespace EcoSim
