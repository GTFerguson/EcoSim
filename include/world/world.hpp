#ifndef WORLD_H
#define WORLD_H

/**
 *	Title	  : Ecosim - World
 *	Author	: Gary Ferguson
 *	Date	  : November 18th, 2019
 *	Purpose	: Storage and manipulation of world data.
 */

#include "../objects/creature/creature.hpp"
#include "../objects/gameObject.hpp"
#include "../colorPairs.hpp"
#include "../rendering/RenderTypes.hpp"
#include "tile.hpp"
#include "SimplexNoise.hpp"
#include "ScentLayer.hpp"
#include "Corpse.hpp"
#include "SpatialIndex.hpp"

// Genetics system integration (Phase 2.4)
#include "../genetics/core/GeneRegistry.hpp"
#include "../genetics/organisms/Plant.hpp"
#include "../genetics/organisms/PlantFactory.hpp"
#include "../genetics/defaults/UniversalGenes.hpp"
#include "../genetics/expression/EnvironmentState.hpp"
#include "../genetics/interactions/SeedDispersal.hpp"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <random>
#include <memory>

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

struct OctaveGen {
  unsigned  quantity     = 4;
  double    minWeight    = 0.1;
  double    maxWeight    = 0.5;
  double    freqInterval = 1.0;

  double weightInterval () const {
    return (quantity > 1) ? (maxWeight - minWeight) / (quantity - 1) : 0.0;
  }
};

struct TileGen {
  double      elevation;
  Tile        prefab;
  TerrainType terrainType;  // NEW: Semantic terrain type for this tile
};

class World {
	private:
		std::vector<std::vector<Tile>> _grid;
    MapGen                _mapGen;
    OctaveGen             _octaveGen;
    std::vector<TileGen>  _tileGen;
    
    // RNG for tree placement - initialized once and reused
    std::mt19937          _rng;
    
    // Genetics system integration (Phase 2.4)
    std::shared_ptr<EcoSim::Genetics::GeneRegistry> _plantRegistry;
    std::unique_ptr<EcoSim::Genetics::PlantFactory> _plantFactory;
    EcoSim::Genetics::EnvironmentState _currentEnvironment;
    EcoSim::Genetics::SeedDispersal _seedDispersal;
    
    // Scent-based sensory system (Phase 1: Sensory System)
    EcoSim::ScentLayer _scentLayer;
    unsigned int _currentTick;
    
    // Spatial indexing for O(1) creature neighbor queries (Phase 3)
    std::unique_ptr<EcoSim::SpatialIndex> _creatureIndex;
    
    // Corpse management
    std::vector<std::unique_ptr<world::Corpse>> _corpses;

 public:
    //============================================================================
		//	Constructor
    //============================================================================
    World (const MapGen &mapGen, const OctaveGen &octaveGen);

    //============================================================================
		//	Getters
    //============================================================================
		std::vector<std::vector<Tile>> &getGrid ();
		double    getSeed				  () const;
		double 	  getScale			  () const;
		double 	  getFreq				  () const;
		double 	  getExponent			() const;
		unsigned  getTerraces 		() const;
		double	  getTerrainLevel (const unsigned int &level) const;
		unsigned  getRows				  () const;
		unsigned  getCols				  () const;
    MapGen    getMapGen       () const;
    OctaveGen getOctaveGen    () const;

    //============================================================================
		//	Setters
    //============================================================================
		void setSeed			    (const double &seed);
		void setScale			    (const double &scale);
		void setFreq			    (const double &freq);
		void setExponent		  (const double &exponent);
		void setTerraces		  (const unsigned int &terraces);
		void setRows				  (const unsigned int &rows);
		void setCols				  (const unsigned int &cols);
		void setTerrainLevel	(const unsigned int &level, const unsigned int &newValue);
		void set2Dgrid        ();
    void setMapGen        (const MapGen &mg);
    void setOctaveGen     (const OctaveGen &og);

    //============================================================================
		//	World Generation
    //============================================================================
    void  addOctaves    (double &noise, const double &nx, const double &ny);
		Tile  assignTerrain (const double &height);
		void  simplexGen		();

    //============================================================================
    //  Genetics-Based Plants (Phase 2.4)
    //============================================================================
    
    /**
     * @brief Initialize the genetics plant factory and registry
     * Call this once before adding genetics-based plants.
     */
    void initializeGeneticsPlants();
    
    /**
     * @brief Check if genetics plant system is initialized
     * @return true if plant factory is ready
     */
    bool hasGeneticsPlants() const;
    
    /**
     * @brief Add genetics-based plants throughout the world
     * @param lowElev Minimum elevation for plant placement
     * @param highElev Maximum elevation for plant placement
     * @param rate Percentage chance (1-100) of plant placement per tile
     * @param speciesName Species template name (e.g., "berry_bush", "oak_tree")
     */
    void addGeneticsPlants(unsigned int lowElev, unsigned int highElev,
                           unsigned int rate, const std::string& speciesName);
    
    /**
     * @brief Add a single genetics plant at specific location
     * @param x X coordinate
     * @param y Y coordinate
     * @param speciesName Species template name
     * @return true if plant added successfully
     */
    bool addGeneticsPlant(int x, int y, const std::string& speciesName);
    
    /**
     * @brief Get the current environment state
     * @return Reference to current environment state
     */
    EcoSim::Genetics::EnvironmentState& getEnvironment();
    
    /**
     * @brief Update environment state (call each tick or when environment changes)
     * @param temperature Current temperature
     * @param lightLevel Light level (0.0 to 1.0)
     * @param waterAvailability Water availability (0.0 to 1.0)
     */
    void updateEnvironment(float temperature, float lightLevel, float waterAvailability);
    
    /**
     * @brief Get the plant factory (for direct plant creation)
     * @return Pointer to plant factory, or nullptr if not initialized
     */
    EcoSim::Genetics::PlantFactory* getPlantFactory();
    
    /**
     * @brief Get the plant registry (for gene lookups)
     * @return Shared pointer to gene registry
     */
    std::shared_ptr<EcoSim::Genetics::GeneRegistry> getPlantRegistry();
 
    //============================================================================
    //	Update Objects
    //============================================================================
    void updateAllObjects();
    
    /**
     * @brief Update all genetics-based plants in the world
     * Handles growth, fruit production, death, and cleanup
     */
    void updateGeneticsPlants();
    
    //============================================================================
    //  Scent Layer (Phase 1: Sensory System)
    //============================================================================
    
    /**
     * @brief Get the scent layer for querying or depositing scents
     * @return Reference to the scent layer
     */
    EcoSim::ScentLayer& getScentLayer();
    
    /**
     * @brief Get the scent layer (const version)
     * @return Const reference to the scent layer
     */
    const EcoSim::ScentLayer& getScentLayer() const;
    
    /**
     * @brief Update the scent layer (decay old scents, etc.)
     * Call this each tick during the main update loop.
     */
    void updateScentLayer();
    
    /**
     * @brief Get the current simulation tick
     * @return Current tick count
     */
    unsigned int getCurrentTick() const;
    
    //============================================================================
    //  Spatial Indexing (Phase 3: O(1) Neighbor Queries)
    //============================================================================
    
    /**
     * @brief Initialize the creature spatial index
     * Call this after world dimensions are set, before adding creatures.
     */
    void initializeCreatureIndex();
    
    /**
     * @brief Get the creature spatial index
     * @return Pointer to spatial index, or nullptr if not initialized
     */
    EcoSim::SpatialIndex* getCreatureIndex();
    
    /**
     * @brief Get the creature spatial index (const version)
     * @return Const pointer to spatial index, or nullptr if not initialized
     */
    const EcoSim::SpatialIndex* getCreatureIndex() const;
    
    /**
     * @brief Rebuild the spatial index from a creature vector
     * Call this after loading saves or major population changes.
     * @param creatures Vector of all creatures
     */
    void rebuildCreatureIndex(std::vector<Creature>& creatures);
    
    //============================================================================
    //  Corpse Management
    //============================================================================
    
    /**
     * @brief Add a corpse at a location when a creature dies
     * @param x X coordinate
     * @param y Y coordinate
     * @param size Size of the creature (affects nutrition and decay time)
     * @param speciesName Species name for the corpse
     * @param bodyCondition Body condition (0.0-1.0) based on creature's energy when it died
     */
    void addCorpse(float x, float y, float size, const std::string& speciesName, float bodyCondition = 0.5f);
    
    /**
     * @brief Update all corpses (advance decay, remove fully decayed)
     */
    void tickCorpses();
    
    /**
     * @brief Get all corpses in the world
     * @return Const reference to corpse vector
     */
    const std::vector<std::unique_ptr<world::Corpse>>& getCorpses() const;
    
    /**
     * @brief Find the nearest corpse within range
     * @param x X coordinate to search from
     * @param y Y coordinate to search from
     * @param maxRange Maximum search range
     * @return Pointer to nearest corpse, or nullptr if none found
     */
    world::Corpse* findNearestCorpse(float x, float y, float maxRange);
    
    /**
     * @brief Remove a specific corpse from the world
     * @param corpse Pointer to the corpse to remove
     */
    void removeCorpse(world::Corpse* corpse);
    
    /// Maximum number of corpses for performance
    static constexpr size_t MAX_CORPSES = 100;

    //============================================================================
  //	To String
    //============================================================================
    /** @brief Serializes the world state to a string for saving */
    std::string toString () const;
};

#endif  // ECOSIM_WORLD_WORLD_HPP
