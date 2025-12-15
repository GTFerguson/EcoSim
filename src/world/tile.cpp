/**
 *	Title		: EcoSim - Tiles
 *	Author	: Gary Ferguson
 *	Date		: Oct 04th, 2019
 *	Purpose	: Creates an object to handle each
 *				    tile within the world.
 */

#include "../../include/world/tile.hpp"

#include <iostream>

using namespace std;

//	Constructors
Tile::Tile () {
	_objLimit	    = 100;
	_character	  = '?';
	_colPair	    = 4;
	_terrainType  = TerrainType::PLAINS;
	_passable	    = false;
}

Tile::Tile (const unsigned &objLimit,
						const char &character,
						const unsigned &colPair,
            const bool &passable,
						const bool &isSource,
						TerrainType terrainType) {
	_objLimit	    = objLimit;
	_character    = character;
	_colPair	    = colPair;
	_terrainType  = terrainType;
	_passable     = passable;
  _isSource     = isSource;
}

Tile::Tile (const unsigned &objLimit,
						const char &character,
						const unsigned &colPair,
            const bool &passable,
						const bool &isSource,
						const unsigned elevation,
						TerrainType terrainType) {
	_objLimit	    = objLimit;
	_character    = character;
	_colPair	    = colPair;
	_terrainType  = terrainType;
	_passable 	  = passable;
  _isSource     = isSource;
	_elevation	  = elevation;
}

//================================================================================
//	Getters
//================================================================================
vector<Food>&           Tile::getFoodVec    ()       { return _foodVec;  }
vector<Spawner>&        Tile::getSpawners	  ()       { return _spawners; }
const vector<Food>      Tile::getFoodVec    () const { return _foodVec;  }
const vector<Spawner>   Tile::getSpawners	  () const { return _spawners; }
unsigned int            Tile::getElevation  () const { return _elevation;	}
char                    Tile::getChar		    () const { return _character;	}
int			                Tile::getColPair    () const { return _colPair;   }
TerrainType             Tile::getTerrainType() const { return _terrainType; }
bool		                Tile::isPassable    () const { return _passable;	}
bool                    Tile::isSource      () const { return _isSource;  }

//================================================================================
//	Setters
//================================================================================
void Tile::setElevation (unsigned int elevation) {
	_elevation = elevation;
}

//================================================================================
//	Object Handling
//================================================================================
bool Tile::addFood (const Food& obj) {
	if (_foodVec.size() >= _objLimit) {
		std::cerr << "[Tile] Warning: Cannot add food '" << obj.getName()
		          << "' - tile object limit (" << _objLimit << ") reached" << std::endl;
		return false;
	}
	_foodVec.push_back(obj);
	return true;
}

void Tile::removeFood (const string& objName) {
	for (size_t i = 0; i < _foodVec.size(); i++) {
		//	If current obj name is equal to name searched for remove
		if (objName.compare(_foodVec[i].getName()) == 0) {
			_foodVec.erase (_foodVec.begin() + static_cast<ptrdiff_t>(i));
			return;
		}
	}
}

/**
 *  Updates all decay values of food on the tile.
 */
void Tile::updateFood () {
  vector<Food>::iterator it = _foodVec.begin();
  while (it != _foodVec.end()) {
    if (it->getDecay() < it->getLifespan()) {
      it->incrementDecay ();
      ++it;
    } else {
      it = _foodVec.erase (it);
    }
  }
}

bool Tile::addSpawner (const Spawner& obj) {
	if (_spawners.size() >= _objLimit) {
		std::cerr << "[Tile] Warning: Cannot add spawner '" << obj.getName()
		          << "' - tile object limit (" << _objLimit << ") reached" << std::endl;
		return false;
	}
	_spawners.push_back (obj);
	return true;
}

// NOTE: Currently uses string comparison for object identification.
// Consider refactoring to use numeric IDs for better performance.
void Tile::removeSpawner (const string& objName) {
  vector<Spawner>::iterator it = _spawners.begin();
  while (it != _spawners.end()) {
    //	If current obj name is equal to name searched for remove
    if (objName.compare(it->getName()) == 0) {
      _spawners.erase (it);
      return;
    }
    ++it;
  }
}

//================================================================================
//  To String
//================================================================================
string Tile::contentToString () const {
  ostringstream ss;

  vector<Food>::const_iterator food = _foodVec.begin();
  while (food != _foodVec.end()) {
    ss << food->toString();
    if (++food != _foodVec.end()) ss << endl;
  }

  vector<Spawner>::const_iterator spawner = _spawners.begin();
  while (spawner != _spawners.end()) {
    ss << spawner->toString();
    if (++spawner != _spawners.end()) ss << endl;
  }

  return ss.str ();
}

string Tile::toString () const {
  ostringstream ss;
  ss  << _objLimit << "," << _elevation
      << endl << contentToString();

  return ss.str ();
}
