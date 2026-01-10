#ifndef ECOSIM_WORLD_GENERATOR_HPP
#define ECOSIM_WORLD_GENERATOR_HPP

/**
 * @file WorldGenerator.hpp
 * @brief Terrain generation using Simplex noise
 *
 * @deprecated This generator uses simple elevation-based terrain types.
 *             For new code, use ClimateWorldGenerator which provides
 *             realistic climate-based biomes with temperature, moisture,
 *             rivers, and proper ecological zones.
 *
 * Extracted from World class as part of Phase 1.2 decomposition.
 * Handles procedural terrain generation using octaves of Simplex noise
 * with configurable terrain types and elevation mappings.
 */

#include "WorldGrid.hpp"
#include "tile.hpp"
#include "SimplexNoise.hpp"

#include <vector>
#include <random>
#include <string>

namespace EcoSim {

/**
 * @brief Configuration for map generation parameters
 */
struct MapGen {
    double    seed      = 0.0;
    double    scale     = 0.01;
    double    freq      = 1.0;
    double    exponent  = 1.0;
    unsigned  terraces  = 20;
    unsigned  rows      = 500;
    unsigned  cols      = 500;
    bool      isIsland  = false;
};

/**
 * @brief Configuration for octave noise generation
 */
struct OctaveGen {
    unsigned  quantity     = 4;
    double    minWeight    = 0.1;
    double    maxWeight    = 0.5;
    double    freqInterval = 1.0;

    double weightInterval() const {
        return (quantity > 1) ? (maxWeight - minWeight) / (quantity - 1) : 0.0;
    }
};

/**
 * @brief Rule for mapping elevation to terrain type
 */
struct TileGenRule {
    double      maxElevation;
    Tile        prefab;
    TerrainType terrainType;
};

/**
 * @brief Procedural terrain generator using Simplex noise
 * 
 * WorldGenerator extracts terrain generation logic from the World class,
 * providing a focused component for map creation. It uses multiple octaves
 * of Simplex noise combined with elevation-based terrain assignment.
 * 
 * Usage:
 * @code
 * WorldGenerator generator;
 * generator.setMapGen(mapConfig);
 * generator.setOctaveGen(octaveConfig);
 * generator.generate(grid);
 * @endcode
 */
class WorldGenerator {
public:
    /**
     * @brief Construct generator with default configuration
     */
    WorldGenerator();
    
    /**
     * @brief Construct generator with specific configuration
     * @param mapGen Map generation parameters
     * @param octaveGen Octave generation parameters
     */
    WorldGenerator(const MapGen& mapGen, const OctaveGen& octaveGen);
    
    //=========================================================================
    // Configuration
    //=========================================================================
    
    /**
     * @brief Set map generation parameters
     * @param mg Map generation configuration
     */
    void setMapGen(const MapGen& mg);
    
    /**
     * @brief Get current map generation parameters
     * @return Current MapGen configuration
     */
    MapGen getMapGen() const;
    
    /**
     * @brief Set octave generation parameters
     * @param og Octave generation configuration
     */
    void setOctaveGen(const OctaveGen& og);
    
    /**
     * @brief Get current octave generation parameters
     * @return Current OctaveGen configuration
     */
    OctaveGen getOctaveGen() const;
    
    /**
     * @brief Set a specific terrain level's elevation threshold
     * @param level Index of the terrain level
     * @param newValue New elevation threshold
     */
    void setTerrainLevel(unsigned int level, double newValue);
    
    /**
     * @brief Get a specific terrain level's elevation threshold
     * @param level Index of the terrain level
     * @return Elevation threshold for that level
     */
    double getTerrainLevel(unsigned int level) const;
    
    /**
     * @brief Get the number of terrain rules
     * @return Number of terrain generation rules
     */
    size_t getTerrainRuleCount() const;
    
    //=========================================================================
    // Generation
    //=========================================================================
    
    /**
     * @brief Generate terrain on a grid
     * 
     * Fills the provided WorldGrid with terrain based on Simplex noise.
     * Grid must be properly sized before calling this method.
     * 
     * @param grid WorldGrid to populate with terrain
     */
    void generate(WorldGrid& grid);
    
    /**
     * @brief Generate terrain with a specific seed
     * @param grid WorldGrid to populate with terrain
     * @param seed Random seed for terrain generation
     */
    void generate(WorldGrid& grid, double seed);
    
    /**
     * @brief Get the default terrain rules
     * 
     * Returns a copy of the terrain rules vector for inspection.
     * Useful for serialization and debugging.
     * 
     * @return Vector of terrain generation rules
     */
    const std::vector<TileGenRule>& getTerrainRules() const;
    
    //=========================================================================
    // Serialization Support
    //=========================================================================
    
    /**
     * @brief Serialize generator configuration to string
     * @return Serialized configuration
     */
    std::string serializeConfig() const;

private:
    MapGen _mapGen;
    OctaveGen _octaveGen;
    std::vector<TileGenRule> _tileGen;
    
    /**
     * @brief Initialize default terrain rules
     * 
     * Sets up the default elevation-to-terrain mappings including
     * water, sand, grass, forest, mountain, and snow terrain types.
     */
    void initializeDefaultTerrainRules();
    
    /**
     * @brief Add octaves to a base noise value
     * @param noise Base noise value (modified in place)
     * @param nx X coordinate in noise space
     * @param ny Y coordinate in noise space
     */
    void addOctaves(double& noise, double nx, double ny) const;
    
    /**
     * @brief Assign terrain tile based on elevation
     * @param height Elevation value (0-255)
     * @return Appropriate tile for that elevation
     */
    Tile assignTerrain(double height) const;
};

} // namespace EcoSim

#endif // ECOSIM_WORLD_GENERATOR_HPP
