/**
 *	Title		: EcoSim - Food
 *	Author	: Gary Ferguson
 *	Date		: Sep 25th, 2019
 *	Purpose	: Allows the use of a variety of food objects
 *	          of varying caloric content and appearance.
 */

#include "../../include/objects/food.hpp"

#include <cassert>
#include <sstream>

using namespace std;

//================================================================================
//  Constructors
//================================================================================
/**
 *  This is the default constructor that creates a generic food item.
 *  Sets default values for ID, calories, lifespan, and decay.
 */
Food::Food () : GameObject () {
  _id       = 0;
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
Food::Food 	(unsigned id, const std::string &name, const std::string &desc,
			       bool passable, char character, unsigned int colour,
             float calories, unsigned int lifespan)
			      : GameObject (name, desc, passable, character, colour) {
  assert(calories >= 0.0f && "Calories cannot be negative");
  assert(lifespan > 0 && "Lifespan must be positive");
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
Food::Food 	(unsigned id, const std::string &name, const std::string &desc,
			       bool passable, char character, unsigned int colour,
             float calories, unsigned int lifespan, unsigned int decay)
			      : GameObject (name, desc, passable, character, colour) {
  assert(calories >= 0.0f && "Calories cannot be negative");
  assert(lifespan > 0 && "Lifespan must be positive");
  assert(decay <= lifespan && "Decay cannot exceed lifespan");
  _id       = id;
	_calories = calories;
  _lifespan = lifespan;
  _decay    = decay;
}


//================================================================================
//  Decay Handling
//================================================================================
void Food::incrementDecay () {
  if (_decay < _lifespan) {
    ++_decay;
  }
}

bool Food::isDecayed () const {
  return _decay >= _lifespan;
}

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
