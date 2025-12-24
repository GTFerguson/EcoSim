#ifndef TILE_H
#define TILE_H

/**
 *  Title   : Ecosim- Tiles
 *  Author  : Gary Ferguson
 *  Date    : Oct 04, 2019
 *  Purpose : Creates an object to handle each tile within
 *            the world. Includes both aesthetic and practical
 *            information on the tile and it's contents.
 */

#include "../objects/gameObject.hpp"
#include "../objects/spawner.hpp"
#include "../rendering/RenderTypes.hpp"

// Genetics system integration (Phase 2.4)
#include "../genetics/organisms/Plant.hpp"
#include "../genetics/expression/EnvironmentState.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <memory>

class Tile {
  private:
      //==========================================================================
      //  Containers
      //==========================================================================
      unsigned int          _objLimit;
      std::vector<Food>     _foodVec;
      std::vector<Spawner>  _spawners;
      
      // Genetics-based plants (Phase 2.4)
      // Using shared_ptr instead of unique_ptr to allow Tile to be copyable
      // (needed for std::vector<std::vector<Tile>> in World)
      std::vector<std::shared_ptr<EcoSim::Genetics::Plant>> _plants;
      
      //==========================================================================
      //  Tile Information
      //==========================================================================
      char          _character;
      int           _colPair;         // DEPRECATED: Keep for backward compatibility
      TerrainType   _terrainType;     // NEW: Semantic terrain type
      bool          _passable;
      bool          _isSource;
      unsigned int  _elevation;
  public:
      //==========================================================================
      // Default Constructor
      //==========================================================================
      Tile ();
      // Paremeterized Constructor w/ Default Elevation
      Tile (const unsigned int &objLimit, const char &character,
            const unsigned int &colPair,
            const bool &passable,         const bool &_isSource,
            TerrainType terrainType = TerrainType::PLAINS);
      // Paremeterized Constructor w/ Elevation
      Tile (const unsigned int &objLimit, const char &character,
            const unsigned int &colPair,
            const bool &passable,         const bool &_isSource,
            const unsigned int elevation,
            TerrainType terrainType = TerrainType::PLAINS);
      
      //==========================================================================
      // Getters
      //==========================================================================
      std::vector<Food>&          getFoodVec    ();
      std::vector<Spawner>&       getSpawners   ();
      const std::vector<Food>     getFoodVec    () const;
      const std::vector<Spawner>  getSpawners   () const;
      unsigned int                getElevation  () const;
      char                        getChar       () const;
      int                         getColPair    () const;  // DEPRECATED: Use getTerrainType()
      TerrainType                 getTerrainType() const;  // NEW: Returns semantic terrain type
      bool                        isPassable    () const;
      bool                        isSource      () const;
      
      // Genetics-based plant getters (Phase 2.4)
      std::vector<std::shared_ptr<EcoSim::Genetics::Plant>>& getPlants();
      const std::vector<std::shared_ptr<EcoSim::Genetics::Plant>>& getPlants() const;

      //==========================================================================      
      // Setters
      //==========================================================================      
      void setElevation (unsigned int elevation);

      //==========================================================================
      // Container Handling
      //==========================================================================
      bool addFood        (const Food& obj);
      void removeFood     (const std::string& objName);
      void updateFood     ();
      bool addSpawner     (const Spawner& obj);
      void removeSpawner  (const std::string& objName);
      
      // Genetics-based plant handling (Phase 2.4)
      /**
       * @brief Add a genetics-based plant to this tile
       * @param plant The plant to add (ownership shared)
       * @return true if added successfully, false if object limit reached
       */
      bool addPlant(std::shared_ptr<EcoSim::Genetics::Plant> plant);
      
      /**
       * @brief Remove a plant at the given index
       * @param index Index of plant to remove
       */
      void removePlant(size_t index);
      
      /**
       * @brief Update all genetics-based plants on this tile
       * @param env Current environment state (light, water, temperature, etc.)
       */
      void updatePlants(const EcoSim::Genetics::EnvironmentState& env);
      
      /**
       * @brief Remove dead plants from the tile
       * @return Number of plants removed
       */
      size_t removeDeadPlants();
 
      //==========================================================================
      //  To String
      //==========================================================================
      /** @brief Returns a string representation of tile contents (food and spawners) */
      std::string contentToString () const;
      /** @brief Returns a string representation of the entire tile state */
      std::string toString        () const;
};

#endif  // ECOSIM_WORLD_TILE_HPP
