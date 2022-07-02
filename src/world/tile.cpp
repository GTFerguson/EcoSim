/**
 *	Title		: EcoSim - Tiles
 *	Author	: Gary Ferguson
 *	Date		: Oct 04th, 2019
 *	Purpose	: Creates an object to handle each
 *				    tile within the world.
 */

#include "../../include/world/tile.hpp"

using namespace std;

//	Constructors
Tile::Tile () {
	_objLimit	  = 100;
	_character	= '?';
	_colPair	  = 4;
	_passable	  = false;
}

Tile::Tile (const unsigned &objLimit, 
						const char &character, 
						const unsigned &colPair, 
            const bool &passable, 
						const bool &isSource) {
	_objLimit	 = objLimit;	
	_character = character;
	_colPair	 = colPair;
	_passable  = passable;
  _isSource  = isSource;
}

Tile::Tile (const unsigned &objLimit, 
						const char &character, 
						const unsigned &colPair, 
            const bool &passable, 
						const bool &isSource, 
						const unsigned elevation) {
	_objLimit	  = objLimit;	
	_character  = character;
	_colPair	  = colPair;
	_passable 	= passable;
  _isSource   = isSource;
	_elevation	= elevation;
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
void Tile::addFood (Food obj) {
	if (_foodVec.size() < _objLimit)
		_foodVec.push_back(obj);
}

void Tile::removeFood (string objName) {
	for (int i = 0; i < _foodVec.size(); i++) {
		//	If current obj name is equal to name searched for remove
		if (objName.compare(_foodVec[i].getName()) == 0) {
			_foodVec.erase (_foodVec.begin() + i);
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
      it++;
    } else {
			_foodVec.erase (it);
    }
  }
}

void Tile::addSpawner (Spawner obj) {
	if (_spawners.size() < _objLimit) {
		_spawners.push_back (obj);
	}
}

//  TODO change to id instead of string comps
void Tile::removeSpawner (string objName) {
  vector<Spawner>::iterator it = _spawners.begin();
  while (it != _spawners.end()) {
		//	If current obj name is equal to name searched for remove
		if (objName.compare(it->getName()) == 0) {
			_spawners.erase (it);
			return;
		}
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
