#pragma once

namespace EcoSim {

// Forward declarations to avoid circular includes
struct TileClimate;
enum class Biome;

namespace Genetics {

/**
 * @brief Complete environmental state for organisms
 * 
 * This struct holds all environmental data that affects organism behavior
 * and phenotype expression. It can be populated from TileClimate data
 * using the fromTileClimate() factory method, which automatically handles
 * biome blending to produce averaged environmental values.
 * 
 * Design note: Organisms perceive averaged values from biome blends.
 * Rendering systems can query TileClimate directly for full blend data.
 */
struct EnvironmentState {
    //==========================================================================
    // Core Climate Values (from climate simulation)
    //==========================================================================
    
    float temperature = 20.0f;        // Celsius (-40 to +50)
    float moisture = 0.5f;            // 0.0 to 1.0 (precipitation index)
    float elevation = 0.5f;           // 0.0 to 1.0 normalized
    float lightLevel = 1.0f;          // 0.0 to 1.0 (affected by time of day and canopy)
    float time_of_day = 0.5f;         // 0.0 to 1.0 (0 = midnight, 0.5 = noon)
    
    //==========================================================================
    // Biome-Derived Properties (blended from BiomeBlend)
    //==========================================================================
    
    int primaryBiome = 10;            // Biome enum cast to int (10 = TEMPERATE_GRASSLAND)
    float vegetationDensity = 0.5f;   // 0.0 to 1.0 (weighted average)
    float movementCostModifier = 1.0f; // Movement cost multiplier (weighted average)
    
    //==========================================================================
    // Legacy Fields (backward compatibility)
    //==========================================================================
    
    float humidity = 0.5f;            // DEPRECATED: Alias for moisture
    int terrain_type = 0;             // Legacy terrain enum value
    int season = 0;                   // Season enum value (0=Spring, 1=Summer, etc.)
    
    //==========================================================================
    // Constructors
    //==========================================================================
    
    EnvironmentState() = default;
    
    /**
     * @brief Full constructor for explicit initialization
     */
    EnvironmentState(float temp, float moist, float elev, float light, float timeOfDay,
                     int biome, float vegDensity, float moveCost, int seasonVal)
        : temperature(temp)
        , moisture(moist)
        , elevation(elev)
        , lightLevel(light)
        , time_of_day(timeOfDay)
        , primaryBiome(biome)
        , vegetationDensity(vegDensity)
        , movementCostModifier(moveCost)
        , humidity(moist)  // Alias
        , terrain_type(biome)
        , season(seasonVal)
    {}
    
    //==========================================================================
    // Factory Methods
    //==========================================================================
    
    /**
     * @brief Create EnvironmentState from TileClimate data
     * 
     * This factory method properly handles biome blending by using TileClimate's
     * blended accessors (getVegetationDensity(), getMovementCost()). Temperature
     * and moisture come directly from climate simulation.
     * 
     * @param climate The tile's climate data
     * @param timeOfDay Current time of day (0.0-1.0)
     * @param seasonValue Current season enum value
     * @return EnvironmentState populated with climate data
     */
    static EnvironmentState fromTileClimate(
        const TileClimate& climate,
        float timeOfDay,
        int seasonValue);
    
    //==========================================================================
    // Comparison
    //==========================================================================
    
    /**
     * @brief Check if environment differs significantly from another
     * @param other Environment to compare against
     * @param threshold Significance threshold (default 0.1)
     * @return true if significantly different
     */
    bool significantlyDifferent(const EnvironmentState& other, float threshold = 0.1f) const;
};

} // namespace Genetics
} // namespace EcoSim
