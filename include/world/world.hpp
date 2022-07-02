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
#include "../objects/spawner.hpp"
#include "../colorPairs.hpp"
#include "tile.hpp"
#include "SimplexNoise.hpp"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <random>

struct MapGen {
  double 	  seed, scale, freq, exponent;
  unsigned  terraces, rows, cols;
  bool      isIsland;
};

struct OctaveGen {
  unsigned  quantity;
  double    minWeight, maxWeight, freqInterval;

  double weightInterval () const {
    return (maxWeight - minWeight) / (quantity - 1);
  }
};

struct TileGen {
  double  elevation;
  Tile    prefab;
};

class World {
	private:
		std::vector<std::vector<Tile>> _grid;
    MapGen                _mapGen;
    OctaveGen             _octaveGen;
    std::vector<TileGen>  _tileGen;

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
		//	Game Object Handling
    //============================================================================
		void addFood          (const int &x, const int &y, const Food    &obj);
		void addSpawner	      (const int &x, const int &y, const Spawner &obj);
		void addTrees 	      (const unsigned int &lowElev, const unsigned int &highElev,
                           const unsigned int &rate,    Spawner tree);
		void removeFood       (const int &x, const int &y, const std::string &objName);
		void removeSpawner		(const int &x, const int &y, const std::string &objName);
 
    //============================================================================
		//	Game Object Handling
    //============================================================================
		void updateAllObjects	();
		void updateSpawners   (std::vector<Spawner> &spawners,
                           const int &curX, const int &curY);

    //============================================================================
		//	To String
    //============================================================================
    std::string toString () const;
};

#endif
