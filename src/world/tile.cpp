/**
 *	Title		: EcoSim - Tiles
 *	Author	: Gary Ferguson
 *	Date		: Oct 04th, 2019
 *	Purpose	: Creates an object to handle each
 *				    tile within the world.
 */

#include "../../include/world/tile.hpp"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace EcoSim::Genetics;

//	Constructors
Tile::Tile () {
	_objLimit	    = 100;
	_character	  = '?';
	_colPair	    = 4;
	_terrainType  = TerrainType::PLAINS;
	_passable	    = false;
	_waterDepth   = 0.0f;
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
	_waterDepth   = 0.0f;
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
	_waterDepth   = 0.0f;
}

//================================================================================
//	Getters
//================================================================================
unsigned int            Tile::getElevation  () const { return _elevation;	}
char                    Tile::getChar		    () const { return _character;	}
int			                Tile::getColPair    () const { return _colPair;   }
TerrainType             Tile::getTerrainType() const { return _terrainType; }
bool		                Tile::isPassable    () const { return _passable;	}
bool                    Tile::isSource      () const { return _isSource;  }
float                   Tile::getWaterDepth () const { return _waterDepth; }

//================================================================================
//	Setters
//================================================================================
void Tile::setElevation (unsigned int elevation) {
	_elevation = elevation;
}

void Tile::setWaterDepth(float depth) {
	_waterDepth = depth;
}

//================================================================================
//  Genetics-Based Plant Handling (Phase 2.4)
//================================================================================

std::vector<std::shared_ptr<Plant>>& Tile::getPlants() {
  return _plants;
}

const std::vector<std::shared_ptr<Plant>>& Tile::getPlants() const {
  return _plants;
}

bool Tile::addPlant(std::shared_ptr<Plant> plant) {
  // Limit 1 plant per tile to prevent population exploits
  // Multiple plants on the same tile allows infinite resource stacking
  if (!_plants.empty()) {
    // Check if there's already a living plant
    for (const auto& existingPlant : _plants) {
      if (existingPlant && existingPlant->isAlive()) {
        // Already has a living plant - reject new plant
        return false;
      }
    }
  }
  
  // Count total objects including plants
  size_t totalObjects = _plants.size();
  if (totalObjects >= _objLimit) {
    std::cerr << "[Tile] Warning: Cannot add plant - tile object limit ("
              << _objLimit << ") reached" << std::endl;
    return false;
  }
  _plants.push_back(plant);
  return true;
}

void Tile::removePlant(size_t index) {
  if (index < _plants.size()) {
    _plants.erase(_plants.begin() + static_cast<ptrdiff_t>(index));
  }
}

void Tile::updatePlants(const EnvironmentState& env) {
  for (auto& plant : _plants) {
    if (plant && plant->isAlive()) {
      plant->update(env);
    }
  }
}

size_t Tile::removeDeadPlants() {
  size_t removed = 0;
  auto it = _plants.begin();
  while (it != _plants.end()) {
    if (!(*it) || !(*it)->isAlive()) {
      it = _plants.erase(it);
      ++removed;
    } else {
      ++it;
    }
  }
  return removed;
}

//================================================================================
//  To String
//================================================================================
string Tile::contentToString () const {
  ostringstream ss;

  // Display plants
  for (const auto& plant : _plants) {
    if (plant) {
      ss << "Plant #" << plant->getId() << " (";
      ss << (plant->isAlive() ? "alive" : "dead") << ", size: ";
      ss << plant->getCurrentSize() << ")";
      ss << endl;
    }
  }

  return ss.str ();
}

string Tile::toString () const {
  ostringstream ss;
  ss  << _objLimit << "," << _elevation
      << endl << contentToString();

  return ss.str ();
}
