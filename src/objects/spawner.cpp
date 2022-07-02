/**
 *	Title		: EcoSim - Spawner
 *	Author	: Gary Ferguson
 *	Date		: Oct 24th, 2019
 *	Purpose	: Used to create a GameObject that
 * 				    will spawn other GameObjects as 
 *				    specified.
 */

#include "../../include/objects/spawner.hpp"

using namespace std;

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
 */
Spawner::Spawner 	(const string &name, 			      const string &desc, 
			 		         const bool &passable, 			    const char &character, 
		 	 	 	         const unsigned int &colour, 	  const unsigned int &rate, 
					         const unsigned int &minRadius, const unsigned int &maxRadius,
					         const Food &foodObj)
                  : GameObject (name, desc, passable, character, colour) {
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
 */
Spawner::Spawner 	(const string &name, const string &desc, const bool &passable, 			    
                   const char &character,         const unsigned int &colour, 	  
                   const unsigned int &rate,      const unsigned int &timer, 
					         const unsigned int &minRadius, const unsigned int &maxRadius,
					         const Food &foodObj)
                  : GameObject (name, desc, passable, character, colour) {
	_rate		   = rate;
	_minRadius = minRadius;
	_maxRadius = maxRadius;
	_timer 		 = timer;
	_foodObj	 = foodObj;
}

//================================================================================
//	Getters
//================================================================================
unsigned int Spawner::getRate			 () const { return _rate;		   };
unsigned int Spawner::getTimer		 () const { return _timer;		 };
unsigned int Spawner::getMinRadius () const { return _minRadius; };
unsigned int Spawner::getMaxRadius () const { return _maxRadius; };
Food         Spawner::getObject	   () const { return _foodObj;	 };

//================================================================================
//	Setters
//================================================================================
void Spawner::setTimer (const unsigned int &time) { _timer = time; }

//================================================================================
//	Spawning
//================================================================================
/**
 *	Checks whether a new object can be spawned.
 *
 *	@return Returns true if object can be spawned.
 */
bool Spawner::canSpawn () {
	if (_timer > _rate) {
		_timer = 0;
		return true;
	}
	_timer++;
	return false;
}

/**
 *	Generates a single coordinate within the set radius.
 *
 *	@param init		The current position of the spawner.
 *	@param limit	The highest value that could be generated.
 *	@return			The generated coordinate.
 */
int Spawner::genCoordinate (const int &init, const int &limit) {
	int coor;
	do {
		coor = (rand()% (_maxRadius+1-_minRadius)) + _minRadius;
		//	Negative or Positive  
		if (rand()%2 < 1)	//	rand 0-1
			coor = -coor;
		coor += init;
	} while (coor <= 0 || coor >= limit);
	return coor;

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
 *	@return		  The generated coordinates.
 */
vector<int> Spawner::genCoordinates (const int &x,             const int &y,
									                   const unsigned int &cols, const unsigned int &rows) {
  vector<int> coords (2);
  coords[0] = genCoordinate (x, cols);
  coords[1] = genCoordinate (y, rows);
  return coords;
}

//================================================================================
//  To String
//================================================================================
string Spawner::toString () const {
  string output = this->GameObject::toString () + ","; 
         output += to_string (_timer)           + ",";
         output += to_string (_rate)            + ",";
         output += to_string (_minRadius)       + ",";
         output += to_string (_maxRadius)       + ",";
         output += _foodObj.toString ();

  return output;
}
