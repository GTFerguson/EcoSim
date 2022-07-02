/**
 *	Title		: EcoSim - Food
 *	Author	: Gary Ferguson
 *	Date		: Sep 25th, 2019
 *	Purpose	: Allows the use of a variety of food objects
 *	          of varying caloric content and appearance.
 */

#include "../../include/objects/food.hpp"

using namespace std;

//================================================================================
//  Constructors
//================================================================================
/**
 *  This is the default constructor that creates the default apple.
 */
Food::Food () : GameObject () {
  _calories = 0.5f;
  _lifespan = 100;
  _decay    = 0;
}

/**
 *	This is our constructor for Food objects.
 *
 *  @param id         This is how we quickly identify the food.
 *	@param name			  The name of the food.
 *	@param desc			  A description of the food.
 *	@param passable		Whether a creature can walk through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 *	@param calories		This is the energy that can be obtained from the food.
 *	@param lifespan   How long the food takes to decay.
 */
Food::Food 	(const unsigned &id, const std::string &name, const std::string &desc, 
			       const bool &passable, const char &character, const unsigned &colour, 
             const float &calories, const unsigned &lifespan) 
			      : GameObject (name, desc, passable, character, colour) {
  _id       = id;
	_calories = calories;
  _lifespan = lifespan;
  _decay    = 0;
}

/**
 *	This is our constructor for Food objects.
 *
 *	@param name			  The name of the food.
 *	@param desc			  A description of the food.
 *	@param passable		Whether a creature can walk through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 *	@param calories		This is the energy that can be obtained from the food.
 *	@param lifespan   How long the food takes to decay.
 *	@param decay      A timer for when the food will despawn.
 */
Food::Food 	(const unsigned &id, const std::string &name, const std::string &desc, 
			       const bool &passable, const char &character, const unsigned &colour, 
             const float &calories, const unsigned &lifespan, const unsigned &decay) 
			      : GameObject (name, desc, passable, character, colour) {
  _id       = id;
	_calories = calories;
  _lifespan = lifespan;
  _decay    = decay;
}


//================================================================================
//  Increment
//================================================================================
void Food::incrementDecay () { _decay++; };

//================================================================================
//  Getters
//================================================================================
unsigned  Food::getID       () const { return _id;        }
float     Food::getCalories () const { return _calories;  }
unsigned  Food::getDecay    () const { return _decay;     }
unsigned  Food::getLifespan () const { return _lifespan;  }

//================================================================================
//  To String
//================================================================================
string Food::toString () const {
  ostringstream ss;
  ss  << this->GameObject::toString()   << "," 
      << _id        << "," << _calories << ","
      << _lifespan  << "," << _decay;

  return ss.str();
}
