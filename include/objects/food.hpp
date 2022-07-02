#ifndef FOOD_H
#define FOOD_H

/**
 *	Title		: EcoSim - food
 *	Author  : Gary Ferguson
 *	Date		: Sep 25th, 2019
 *	Purpose	: Allows the use of a variety of food objects
 *	          of varying caloric content and appearance.
 */

#include "gameObject.hpp"

#include <sstream>

class Food: public GameObject {
	private:
    unsigned int  _id, _lifespan, _decay;
		float         _calories;
	public:
    //============================================================================
		//	Constructor
    //============================================================================
    Food ();
		Food (const unsigned &id,   const std::string &name,  const std::string &desc,
          const bool &passable, const char &character,    const unsigned &colour, 
          const float &calories, const unsigned &lifespan);
		Food (const unsigned &id,   const std::string &name,  const std::string &desc,
          const bool &passable, const char &character,    const unsigned &colour, 
          const float &calories, const unsigned &lifespan, const unsigned &decay);

    //============================================================================
    //  Increment
    //============================================================================
    void incrementDecay ();

    //============================================================================
		//	Getters
    //============================================================================
    unsigned  getID       () const;
		float     getCalories () const;
		unsigned  getDecay    () const;
		unsigned  getLifespan () const;
  
    //============================================================================
		//  To String	
    //============================================================================
    std::string toString () const;
};

#endif
