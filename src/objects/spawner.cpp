/**
 *	Title		: EcoSim - Spawner
 *	Author	: Gary Ferguson
 *	Date		: Oct 24th, 2019
 *	Purpose	: Used to create a GameObject that
 * 				    will spawn other GameObjects as 
 *				    specified.
 */

#include "../../include/objects/spawner.hpp"

#include <random>
#include <algorithm>
#include <cassert>
#include <sstream>

using namespace std;

// Thread-safe random number generator
static std::mt19937& getRandomGenerator() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

/**
 *	This is our constructor for Spawner objects.
 *
 *	@param name			  The name of the spawner.
 *	@param desc			  A description of the spawner.
 *	@param passable		Whether a creature can walk through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 *	@param rate			  Rate at which objects are spawned.
 *	@param minRadius	The minimum radius objects can spawn.
 *	@param maxRadius	The maximum radius objects can spawn.
 *	@param foodObj    The Food object the spawner creates.
 *	@param entityType The semantic entity type for renderer-agnostic rendering.
 */
Spawner::Spawner 	(const string &name, 			      const string &desc,
			 		         bool passable, 			          char character,
		 	 	 	         unsigned int colour, 	        unsigned int rate,
				         unsigned int minRadius,        unsigned int maxRadius,
				         const Food &foodObj,           EntityType entityType)
                  : GameObject (name, desc, passable, character, colour, entityType) {
  assert(minRadius <= maxRadius && "minRadius must be <= maxRadius");
  assert(rate > 0 && "Spawn rate must be positive");
	_rate		   = rate;
	_minRadius = minRadius;
	_maxRadius = maxRadius;
	_timer 		 = 0;
	_foodObj	 = foodObj;
}

/**
 *	This is our constructor for Spawner objects.
 *
 *	@param name			  The name of the spawner.
 *	@param desc			  A description of the spawner.
 *	@param passable		Whether a creature can walk through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 *	@param rate			  Rate at which objects are spawned.
 *	@param timer      The timer object for spawning.
 *	@param minRadius	The minimum radius objects can spawn.
 *	@param maxRadius	The maximum radius objects can spawn.
 *	@param foodObj    The Food object the spawner creates.
 *	@param entityType The semantic entity type for renderer-agnostic rendering.
 */
Spawner::Spawner 	(const string &name, const string &desc, bool passable,
                   char character,              unsigned int colour,
                   unsigned int rate,           unsigned int timer,
				         unsigned int minRadius,        unsigned int maxRadius,
				         const Food &foodObj,           EntityType entityType)
                  : GameObject (name, desc, passable, character, colour, entityType) {
  assert(minRadius <= maxRadius && "minRadius must be <= maxRadius");
  assert(rate > 0 && "Spawn rate must be positive");
	_rate		   = rate;
	_minRadius = minRadius;
	_maxRadius = maxRadius;
	_timer 		 = timer;
	_foodObj	 = foodObj;
}

//================================================================================
//	Getters
//================================================================================
unsigned int Spawner::getRate			 () const { return _rate;		   }
unsigned int Spawner::getTimer		 () const { return _timer;		 }
unsigned int Spawner::getMinRadius () const { return _minRadius; }
unsigned int Spawner::getMaxRadius () const { return _maxRadius; }
const Food&  Spawner::getObject	   () const { return _foodObj;	 }

//================================================================================
//	Setters
//================================================================================
void Spawner::setTimer (unsigned int time) { _timer = time; }

//================================================================================
//	Spawning
//================================================================================
/**
 *	Checks whether a new object can be spawned (query only, no side effects).
 *
 *	@return Returns true if object can be spawned.
 */
bool Spawner::canSpawn () const {
	return _timer > _rate;
}

/**
 *	Increments timer and checks if spawn should occur.
 *	Resets timer when spawn condition is met.
 *
 *	@return Returns true if object should be spawned.
 */
bool Spawner::tickAndCheckSpawn () {
	_timer++;
	if (_timer > _rate) {
		_timer = 0;
		return true;
	}
	return false;
}

/**
 *	Generates a single coordinate within the set radius.
 *
 *	@param init		The current position of the spawner.
 *	@param limit	The highest value that could be generated.
 *	@return			The generated coordinate.
 */
int Spawner::genCoordinate (int init, int limit) {
	constexpr int MAX_ATTEMPTS = 100;
	std::uniform_int_distribution<int> radiusDist(_minRadius, _maxRadius);
	std::uniform_int_distribution<int> signDist(0, 1);
	
	int coor;
	for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
		coor = radiusDist(getRandomGenerator());
		//	Negative or Positive
		if (signDist(getRandomGenerator()) == 0)
			coor = -coor;
		coor += init;
		
		if (coor > 0 && coor < limit)
			return coor;
	}
	// Fallback: clamp to valid range if no valid coordinate found
	return std::max(1, std::min(init, limit - 1));
}

/**
 *	This should be performed every game turn to check
 *	whether the spawner has reached a point where it can
 *	produce a new object.
 *
 *	@param x	  The current x pos of the spawner.
 *	@param y	  The current y pos of the spawner.
 *	@param cols	The number of columns on the map.
 *	@param rows	The number of rows on the map.
 *	@return		  The generated coordinates as a pair (x, y).
 */
std::pair<int, int> Spawner::genCoordinates (int x, int y,
									                           unsigned int cols, unsigned int rows) {
  return {genCoordinate(x, cols), genCoordinate(y, rows)};
}

//================================================================================
//  To String
//================================================================================
string Spawner::toString () const {
  ostringstream ss;
  ss << GameObject::toString() << ","
     << _timer << "," << _rate << ","
     << _minRadius << "," << _maxRadius << ","
     << _foodObj.toString();
  return ss.str();
}
