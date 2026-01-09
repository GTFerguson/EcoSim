#ifndef ECOSIM_WORLD_WORLD_HPP
#define ECOSIM_WORLD_WORLD_HPP

/**
 * @file world.hpp
 * @brief Main World class - coordinates all world subsystems
 * @author Gary Ferguson
 * 
 * The World class serves as the central coordinator for the simulation environment,
 * providing access to specialized subsystems that manage different aspects:
 * 
 * - WorldGrid: Tile storage and access
 * - WorldGenerator: Terrain generation using simplex noise
 * - ScentLayer: Scent-based communication system
 * - SpatialIndex: O(1) creature neighbor queries
 * - CorpseManager: Corpse lifecycle management
 * - SeasonManager: Time and season tracking
 * - EnvironmentSystem: Environmental queries
 * - PlantManager: Plant lifecycle management
 * 
 * Access subsystems via their accessor methods (e.g., grid(), plants(), corpses()).
 */

#include "WorldGrid.hpp"
#include "WorldGenerator.hpp"
#include "ClimateWorldGenerator.hpp"
#include "ScentLayer.hpp"
#include "SpatialIndex.hpp"
#include "CorpseManager.hpp"
#include "SeasonManager.hpp"
#include "EnvironmentSystem.hpp"
#include "PlantManager.hpp"
#include "tile.hpp"
#include "Corpse.hpp"

#include "../objects/creature/creature.hpp"
#include "../rendering/RenderTypes.hpp"

#include <vector>
#include <memory>
#include <string>

// Use struct definitions from WorldGenerator.hpp
using EcoSim::MapGen;
using EcoSim::OctaveGen;

/**
 * @class World
 * @brief Central coordinator for all world subsystems
 * 
 * The World class owns and coordinates all world-related subsystems. Rather than
 * implementing all functionality directly, it delegates to specialized components
 * and provides a unified interface for the rest of the application.
 */
class World {
public:
    //============================================================================
    // Construction
    //============================================================================
    
    /**
     * @brief Construct a new World
     * @param mapGen Map generation configuration
     * @param octaveGen Octave generation configuration
     */
    World(const MapGen& mapGen, const OctaveGen& octaveGen);

    //============================================================================
    // Core Subsystem Accessors
    //============================================================================
    
    /**
     * @brief Get the WorldGrid for tile access
     * @return Reference to the WorldGrid
     */
    EcoSim::WorldGrid& grid();
    const EcoSim::WorldGrid& grid() const;
    
    /**
     * @brief Get the scent layer for scent-based communication
     * @return Reference to the ScentLayer
     */
    EcoSim::ScentLayer& scentLayer();
    const EcoSim::ScentLayer& scentLayer() const;
    
    /**
     * @brief Get the CorpseManager
     * @return Reference to the CorpseManager
     */
    EcoSim::CorpseManager& corpses();
    const EcoSim::CorpseManager& corpses() const;
    
    /**
     * @brief Get the SeasonManager
     * @return Reference to the SeasonManager
     */
    EcoSim::SeasonManager& seasons();
    const EcoSim::SeasonManager& seasons() const;
    
    /**
     * @brief Get the EnvironmentSystem
     * @return Reference to the EnvironmentSystem
     */
    EcoSim::EnvironmentSystem& environment();
    const EcoSim::EnvironmentSystem& environment() const;
    
    /**
     * @brief Get the PlantManager
     * @return Reference to the PlantManager
     */
    EcoSim::PlantManager& plants();
    const EcoSim::PlantManager& plants() const;
    
    //============================================================================
    // Spatial Indexing
    //============================================================================
    
    /**
     * @brief Initialize the creature spatial index
     * Call after world dimensions are set, before adding creatures.
     */
    void initializeCreatureIndex();
    
    /**
     * @brief Get the creature spatial index
     * @return Pointer to spatial index, or nullptr if not initialized
     */
    EcoSim::SpatialIndex* getCreatureIndex();
    const EcoSim::SpatialIndex* getCreatureIndex() const;
    
    /**
     * @brief Rebuild the spatial index from a creature vector
     * Call after loading saves or major population changes.
     * @param creatures Vector of all creatures
     */
    void rebuildCreatureIndex(std::vector<Creature>& creatures);
    
    //============================================================================
    // Terrain Generation Configuration
    //============================================================================
    
    /** @brief Get the random seed for terrain generation */
    double    getSeed() const;
    /** @brief Get the scale factor for terrain generation */
    double    getScale() const;
    /** @brief Get the base frequency for noise generation */
    double    getFreq() const;
    /** @brief Get the exponent for terrain height curve */
    double    getExponent() const;
    /** @brief Get the number of terraces for height quantization */
    unsigned  getTerraces() const;
    /** @brief Get the elevation threshold for a terrain level */
    double    getTerrainLevel(const unsigned int& level) const;
    /** @brief Get number of rows in the grid */
    unsigned  getRows() const;
    /** @brief Get number of columns in the grid */
    unsigned  getCols() const;
    /** @brief Get the full map generation configuration */
    MapGen    getMapGen() const;
    /** @brief Get the octave generation configuration */
    OctaveGen getOctaveGen() const;
    
    /** @brief Set the random seed for terrain generation */
    void setSeed(const double& seed);
    /** @brief Set the scale factor for terrain generation */
    void setScale(const double& scale);
    /** @brief Set the base frequency for noise generation */
    void setFreq(const double& freq);
    /** @brief Set the exponent for terrain height curve */
    void setExponent(const double& exponent);
    /** @brief Set the number of terraces for height quantization */
    void setTerraces(const unsigned int& terraces);
    /** @brief Set the number of rows (requires regeneration) */
    void setRows(const unsigned int& rows);
    /** @brief Set the number of columns (requires regeneration) */
    void setCols(const unsigned int& cols);
    /** @brief Set the elevation threshold for a terrain level */
    void setTerrainLevel(const unsigned int& level, const unsigned int& newValue);
    /** @brief Set the full map generation configuration */
    void setMapGen(const MapGen& mg);
    /** @brief Set the octave generation configuration */
    void setOctaveGen(const OctaveGen& og);
    
    /**
     * @brief Regenerate terrain using current configuration
     * Call after changing generation parameters.
     */
    void simplexGen();

    //============================================================================
    // Simulation Update
    //============================================================================
    
    /**
     * @brief Update all world objects for one tick
     * Updates plants and other time-dependent systems.
     */
    void updateAllObjects();
    
    /**
     * @brief Update the scent layer (decay old scents)
     * Call each tick during the main update loop.
     */
    void updateScentLayer();
    
    /**
     * @brief Get the current simulation tick
     * @return Current tick count
     */
    unsigned int getCurrentTick() const;
    
    //============================================================================
    // Corpse Convenience Methods
    //============================================================================
    // These delegate to CorpseManager for backward compatibility.
    // Prefer using corpses() accessor for new code.
    
    void addCorpse(float x, float y, float size, const std::string& speciesName, float bodyCondition = 0.5f);
    void tickCorpses();
    const std::vector<std::unique_ptr<world::Corpse>>& getCorpses() const;
    world::Corpse* findNearestCorpse(float x, float y, float maxRange);
    void removeCorpse(world::Corpse* corpse);
    
    //============================================================================
    // Serialization
    //============================================================================
    
    /** @brief Serialize the world state to a string for saving */
    std::string toString() const;
    
    //============================================================================
    // Legacy Interface
    //============================================================================
    // These methods are kept for backward compatibility with existing code.
    // Prefer using grid() accessor for new code.
    
    /** @brief Get raw 2D grid (legacy - prefer grid() accessor) */
    std::vector<std::vector<Tile>>& getGrid();
    
    /** @brief Get scent layer (legacy - prefer scentLayer() accessor) */
    EcoSim::ScentLayer& getScentLayer();
    const EcoSim::ScentLayer& getScentLayer() const;

private:
    //============================================================================
    // Core Components
    //============================================================================
    EcoSim::WorldGrid _grid;
    std::unique_ptr<EcoSim::WorldGenerator> _generator;
    std::unique_ptr<EcoSim::ClimateWorldGenerator> _climateGenerator;
    EcoSim::ScentLayer _scentLayer;
    
    //============================================================================
    // Subsystem Managers
    //============================================================================
    std::unique_ptr<EcoSim::SpatialIndex> _creatureIndex;
    std::unique_ptr<EcoSim::CorpseManager> _corpseManager;
    std::unique_ptr<EcoSim::SeasonManager> _seasonManager;
    std::unique_ptr<EcoSim::EnvironmentSystem> _environmentSystem;
    std::unique_ptr<EcoSim::PlantManager> _plantManager;
    
    //============================================================================
    // State
    //============================================================================
    unsigned int _currentTick;
    
    //============================================================================
    // Private Methods
    //============================================================================
    
    /** @brief Initialize 2D grid dimensions */
    void set2Dgrid();
};

#endif  // ECOSIM_WORLD_WORLD_HPP
