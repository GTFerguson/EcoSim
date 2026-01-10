#ifndef ECOSIM_CLIMATE_WORLD_GENERATOR_HPP
#define ECOSIM_CLIMATE_WORLD_GENERATOR_HPP

/**
 * @file ClimateWorldGenerator.hpp
 * @brief Climate-based world generation with Whittaker biome system
 * 
 * Implements a multi-pass generation pipeline:
 * 1. Continental noise → Land/Ocean mask
 * 2. Elevation detail → Height map with mountain ridges
 * 3. Temperature → Based on latitude and elevation
 * 4. Moisture → Based on distance to water, wind, rain shadow
 * 5. Biome lookup → Whittaker diagram (temperature × moisture)
 * 6. Rivers/Lakes → Downhill flow simulation
 */

#include "WorldGrid.hpp"
#include "SimplexNoise.hpp"
#include "tile.hpp"

#include <vector>
#include <string>
#include <cmath>
#include <random>
#include <queue>
#include <set>
#include <functional>

namespace EcoSim {

/**
 * @brief Climate-based biome types following Whittaker classification
 */
enum class Biome {
    // Aquatic biomes
    OCEAN_DEEP,
    OCEAN_SHALLOW,
    OCEAN_COAST,
    FRESHWATER,
    
    // Cold biomes
    ICE_SHEET,
    TUNDRA,
    TAIGA,
    
    // Temperate biomes
    BOREAL_FOREST,
    TEMPERATE_RAINFOREST,
    TEMPERATE_FOREST,
    TEMPERATE_GRASSLAND,
    
    // Warm biomes
    TROPICAL_RAINFOREST,
    TROPICAL_SEASONAL_FOREST,
    SAVANNA,
    
    // Dry biomes
    DESERT_HOT,
    DESERT_COLD,
    STEPPE,
    SHRUBLAND,
    
    // Elevation-dependent biomes
    ALPINE_MEADOW,
    ALPINE_TUNDRA,
    MOUNTAIN_BARE,
    GLACIER,
    
    COUNT  // Number of biomes
};

/**
 * @brief Terrain features overlaid on biomes
 */
enum class TerrainFeature {
    NONE,
    FLAT,
    ROLLING,
    HILLS,
    MOUNTAINS,
    CLIFF,
    VALLEY,
    CANYON,
    
    // Water features
    RIVER,
    LAKE,
    MARSH,
    BEACH,
    REEF
};

/**
 * @brief Maximum number of biomes that can blend into a single tile
 */
constexpr int MAX_BIOME_BLEND = 4;

/**
 * @brief A single biome contribution to a blend
 */
struct BiomeWeight {
    Biome biome = Biome::TEMPERATE_GRASSLAND;
    float weight = 0.0f;  // 0.0 to 1.0, all weights in a blend sum to 1.0
};

/**
 * @brief Blended biome data for smooth transitions between biomes (ecotones)
 *
 * Each tile can have up to 4 biomes contributing to its properties.
 * The primary biome has the highest weight, secondary biomes create
 * gradual transitions at biome boundaries.
 */
struct BiomeBlend {
    BiomeWeight contributions[MAX_BIOME_BLEND];
    int count = 1;  // Number of contributing biomes (1-4)
    
    BiomeBlend() {
        contributions[0] = {Biome::TEMPERATE_GRASSLAND, 1.0f};
    }
    
    explicit BiomeBlend(Biome primary) {
        contributions[0] = {primary, 1.0f};
        count = 1;
    }
    
    // Get primary (dominant) biome
    Biome primary() const { return contributions[0].biome; }
    float primaryWeight() const { return contributions[0].weight; }
    
    // Add a secondary biome contribution
    void addContribution(Biome biome, float weight);
    
    // Normalize weights to sum to 1.0
    void normalize();
    
    // Get blended property values
    float getBlendedVegetationDensity() const;
    float getBlendedMovementCost() const;
    float getBlendedEvaporationRate() const;
    float getBlendedTemperatureModifier() const;
    bool canSupportPlants() const;
    bool canSupportTrees() const;
};

/**
 * @brief Complete climate data for a single tile
 */
struct TileClimate {
    float elevation = 0.0f;      // 0.0 - 1.0 normalized
    float temperature = 15.0f;   // Celsius (-40 to +50)
    float moisture = 0.5f;       // 0.0 - 1.0 (precipitation index)
    float waterLevel = 0.0f;     // For rivers/lakes
    
    BiomeBlend biomeBlend;       // Weighted blend of biomes for smooth transitions
    TerrainFeature feature = TerrainFeature::NONE;
    
    // Convenience accessor for primary biome
    Biome biome() const { return biomeBlend.primary(); }
    
    bool isLand() const { return elevation >= 0.4f; }
    bool hasWater() const {
        return !isLand() || feature == TerrainFeature::RIVER ||
               feature == TerrainFeature::LAKE || feature == TerrainFeature::MARSH;
    }
    float getAnnualRainfall() const { return moisture * 4000.0f; } // mm/year
    
    // Blended property accessors
    float getVegetationDensity() const { return biomeBlend.getBlendedVegetationDensity(); }
    float getMovementCost() const { return biomeBlend.getBlendedMovementCost(); }
    float getEvaporationRate() const { return biomeBlend.getBlendedEvaporationRate(); }
};

/**
 * @brief Properties associated with each biome type
 */
struct BiomeProperties {
    const char* name;
    float temperatureModifier;   // Local temp adjustment
    float evaporationRate;       // Water loss rate
    float vegetationDensity;     // 0.0 - 1.0
    float movementCost;          // Movement speed multiplier
    bool supportsPlants;
    bool supportsTrees;
    TerrainType terrainType;     // For rendering compatibility
    char displayChar;
};

/**
 * @brief Configuration for climate-based world generation
 */
struct ClimateGeneratorConfig {
    // Dimensions
    unsigned int width = 500;
    unsigned int height = 500;
    
    // Terrain shape
    float seaLevel = 0.30f;      // Lowered to allow ~50-60% land
    bool isIsland = true;
    float islandFalloff = 0.35f;  // Controls edge-to-center transition
    
    // Climate parameters
    float equatorPosition = 0.5f;     // Where equator is (0-1 in Y)
    float temperatureRange = 70.0f;   // Total temperature variation
    float baseTemperature = 15.0f;    // Temperature at equator, sea level
    float lapseRate = 6.5f;           // °C per 1000m elevation
    float maxElevationMeters = 8000.0f;
    
    // Moisture parameters
    float moistureScale = 1.0f;
    float coastalMoistureDecay = 50.0f;    // Distance for moisture decay
    int rainShadowDistance = 100;           // How far to look upwind
    float rainShadowStrength = 0.7f;        // How much mountains block moisture
    
    // Water features
    bool generateRivers = true;
    int maxRivers = 20;
    float riverSourceElevation = 0.55f;   // Lowered to find more sources
    float riverSourceMoisture = 0.4f;     // Lowered to find more sources
    float riverSpawnChance = 0.02f;       // Increased spawn chance
    bool generateLakes = true;
    
    // Noise parameters
    float continentFrequency = 0.002f;
    int continentOctaves = 3;
    float elevationFrequency = 0.01f;
    int elevationOctaves = 4;
    float ridgeFrequency = 0.01f;
    float temperatureNoiseScale = 0.01f;
    float moistureNoiseScale = 0.008f;
    
    // Mountain range parameters
    int numPlateRidges = 4;              // Number of tectonic plate boundaries
    float ridgeStrength = 0.5f;          // How much ridges boost elevation (0-1)
    float ridgeWidth = 80.0f;            // Width of mountain range influence (pixels)
    float foothillsWidth = 120.0f;       // Width of foothills zone beyond ridges
    int ridgeOctaves = 4;                // Octaves for ridged multifractal noise
    float ridgeLacunarity = 2.2f;        // Frequency multiplier between octaves
    float ridgeGain = 0.5f;              // Amplitude multiplier between octaves
    float mountainClusterFreq = 0.015f;  // Frequency for clustering noise
    
    // Inland sea removal
    bool removeInlandSeas = true;        // Fill inland water bodies that don't connect to ocean
    int minInlandSeaSize = 500;          // Minimum size to keep as lake (smaller seas are filled)
    float inlandSeaFillElevation = 0.35f; // Elevation to fill inland seas to (just above sea level)
    
    // Random seed
    unsigned int seed = 0;
};

/**
 * @brief Climate-based procedural world generator
 * 
 * Generates realistic terrain using multiple noise layers and climate
 * simulation to determine biomes. Creates rivers and lakes based on
 * terrain topology.
 */
class ClimateWorldGenerator {
public:
    ClimateWorldGenerator();
    explicit ClimateWorldGenerator(const ClimateGeneratorConfig& config);
    
    //=========================================================================
    // Configuration
    //=========================================================================
    
    void setConfig(const ClimateGeneratorConfig& config);
    const ClimateGeneratorConfig& getConfig() const { return _config; }
    ClimateGeneratorConfig& getConfig() { return _config; }
    
    //=========================================================================
    // Generation
    //=========================================================================
    
    /**
     * @brief Generate a complete world with climate-based biomes
     * @param grid WorldGrid to populate (will be resized)
     */
    void generate(WorldGrid& grid);
    
    /**
     * @brief Generate with a specific seed
     * @param grid WorldGrid to populate
     * @param seed Random seed for reproducible generation
     */
    void generate(WorldGrid& grid, unsigned int seed);
    
    //=========================================================================
    // Data Access (for visualization/debugging)
    //=========================================================================
    
    /**
     * @brief Get climate data for a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Climate data for that tile
     */
    const TileClimate& getClimate(unsigned int x, unsigned int y) const;
    
    /**
     * @brief Get the full climate map
     * @return 2D vector of climate data
     */
    const std::vector<std::vector<TileClimate>>& getClimateMap() const { return _climateMap; }
    
    /**
     * @brief Get raw elevation map (for debugging)
     */
    const std::vector<std::vector<float>>& getElevationMap() const { return _elevationMap; }
    
    /**
     * @brief Get raw temperature map (for debugging)
     */
    const std::vector<std::vector<float>>& getTemperatureMap() const { return _temperatureMap; }
    
    /**
     * @brief Get raw moisture map (for debugging)
     */
    const std::vector<std::vector<float>>& getMoistureMap() const { return _moistureMap; }
    
    //=========================================================================
    // Biome Utilities
    //=========================================================================
    
    /**
     * @brief Get properties for a specific biome
     */
    static const BiomeProperties& getBiomeProperties(Biome biome);
    
    /**
     * @brief Convert biome to legacy TerrainType for rendering
     */
    static TerrainType biomeToTerrainType(Biome biome);
    
    /**
     * @brief Get biome name as string
     */
    static const char* getBiomeName(Biome biome);
    
private:
    ClimateGeneratorConfig _config;
    std::mt19937 _rng;
    
    // Intermediate generation maps
    std::vector<std::vector<float>> _continentMap;
    std::vector<std::vector<float>> _elevationMap;
    std::vector<std::vector<float>> _temperatureMap;
    std::vector<std::vector<float>> _moistureMap;
    std::vector<std::vector<TileClimate>> _climateMap;
    
    // Distance to water cache
    std::vector<std::vector<float>> _waterDistanceMap;
    
    //=========================================================================
    // Generation Phases
    //=========================================================================
    
    void initializeMaps();
    void generateContinentMap();
    void generateElevationMap();
    void calculateTemperature();
    void calculateWaterDistance();
    void calculateMoisture();
    void determineBiomes();
    void generateRivers();
    void applyToGrid(WorldGrid& grid);
    
    //=========================================================================
    // Noise Functions
    //=========================================================================
    
    float continentNoise(int x, int y) const;
    float coastlineDetailNoise(int x, int y) const;
    float elevationNoise(int x, int y) const;
    float ridgedNoise(float x, float y) const;
    float ridgedMultifractal(float x, float y, int octaves, float lacunarity, float gain) const;
    
    //=========================================================================
    // Tectonic Ridge Generation
    //=========================================================================
    
    struct PlateRidge {
        float startX, startY;  // Starting point (normalized 0-1)
        float endX, endY;      // Ending point (normalized 0-1)
        float controlX, controlY;  // Bezier control point for curvature
        float strength;        // Ridge intensity multiplier
    };
    
    std::vector<PlateRidge> _plateRidges;
    std::vector<std::vector<float>> _ridgeDistanceMap;
    
    void generatePlateRidges();
    void calculateRidgeDistanceMap();
    float distanceToRidge(float x, float y, const PlateRidge& ridge) const;
    float bezierPoint(float t, float p0, float p1, float p2) const;
    float ridgeInfluence(int x, int y) const;
    
    //=========================================================================
    // Inland Sea Removal
    //=========================================================================
    
    void removeInlandSeas();
    void floodFillOcean(std::vector<std::vector<bool>>& oceanMask);
    
    //=========================================================================
    // Climate Calculations
    //=========================================================================
    
    float calculateTileTemperature(int x, int y) const;
    float calculateTileMoisture(int x, int y) const;
    float calculateWindMoisture(int x, int y) const;
    float calculateRainShadow(int x, int y) const;
    
    //=========================================================================
    // Biome Lookup
    //=========================================================================
    
    // Returns weighted blend of nearby biomes based on position in climate space
    BiomeBlend lookupBiomeBlend(float temperature, float moisture, float elevation) const;
    
    // Legacy single-biome lookup (returns primary biome from blend)
    Biome lookupBiome(float temperature, float moisture, float elevation) const;
    
    TerrainFeature determineTerrainFeature(unsigned int x, unsigned int y, const TileClimate& climate) const;
    float calculateLocalSlope(unsigned int x, unsigned int y) const;
    bool isCoastalTile(unsigned int x, unsigned int y) const;
    bool isLocalMinimum(unsigned int x, unsigned int y) const;
    
    //=========================================================================
    // Water Feature Generation
    //=========================================================================
    
    struct RiverSource {
        int x, y;
        float flow;
    };
    
    void findRiverSources(std::vector<RiverSource>& sources);
    void traceRiver(int x, int y, float flow);
    void floodFillLake(int x, int y, float inflow);
    void formLake(int centerX, int centerY, float inflow, float maxFlow);
    
    //=========================================================================
    // Utility Functions
    //=========================================================================
    
    bool inBounds(int x, int y) const;
    float distanceToEdge(int x, int y) const;
    float smoothstep(float edge0, float edge1, float x) const;
    int getColorPairForBiome(Biome biome, TerrainFeature feature) const;
    
    // Helper for clamping (C++11/14 compatible)
    template<typename T>
    static T clampValue(T value, T minVal, T maxVal) {
        return (value < minVal) ? minVal : ((value > maxVal) ? maxVal : value);
    }
    
    // 8-directional neighbors
    static constexpr std::pair<int,int> NEIGHBORS_8[8] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };
};

} // namespace EcoSim

#endif // ECOSIM_CLIMATE_WORLD_GENERATOR_HPP
