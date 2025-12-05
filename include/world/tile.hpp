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

#include <string>
#include <sstream>
#include <vector>

class Tile {
  private:
      //========================================================================== 
      //  Containers
      //========================================================================== 
      unsigned int          _objLimit;
      std::vector<Food>     _foodVec;
      std::vector<Spawner>  _spawners;
      //========================================================================== 
      //  Tile Information
      //========================================================================== 
      char          _character;
      int           _colPair;
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
            const bool &passable,         const bool &_isSource);
      // Paremeterized Constructor w/ Elevation
      Tile (const unsigned int &objLimit, const char &character, 
            const unsigned int &colPair,  
            const bool &passable,         const bool &_isSource, 
            const unsigned int elevation);
      
      //==========================================================================      
      // Getters
      //==========================================================================      
      std::vector<Food>&          getFoodVec    ();
      std::vector<Spawner>&       getSpawners   ();
      const std::vector<Food>     getFoodVec    () const;
      const std::vector<Spawner>  getSpawners   () const;
      unsigned int                getElevation  () const;
      char                        getChar       () const;
      int                         getColPair    () const;
      bool                        isPassable    () const;
      bool                        isSource      () const;

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
 
      //==========================================================================
      //  To String
      //==========================================================================
      /** @brief Returns a string representation of tile contents (food and spawners) */
      std::string contentToString () const;
      /** @brief Returns a string representation of the entire tile state */
      std::string toString        () const;
};

#endif  // ECOSIM_WORLD_TILE_HPP
