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

/**
 * @class Food
 * @brief Represents consumable food items in the simulation.
 *
 * Food objects extend GameObject with nutritional properties (calories),
 * decay mechanics (lifespan and decay timer), and unique identification.
 * Creatures can consume food to gain energy, and food naturally decays
 * over time until it becomes inedible.
 */
class Food: public GameObject {
	private:
    unsigned int  _id, _lifespan, _decay;
		float         _calories;
	public:
    //============================================================================
		//	Constructor
    //============================================================================
    Food ();
		Food (unsigned id,   const std::string &name,  const std::string &desc,
          bool passable, char character,           unsigned int colour,
          float calories, unsigned int lifespan);
		Food (unsigned id,   const std::string &name,  const std::string &desc,
          bool passable, char character,           unsigned int colour,
          float calories, unsigned int lifespan,   unsigned int decay);

    //============================================================================
    //  Decay Handling
    //============================================================================
    /**
     * @brief Increments the decay timer by one unit.
     *
     * Will not exceed the lifespan value.
     */
    void incrementDecay ();
    
    /**
     * @brief Checks if the food has fully decayed.
     * @return true if decay >= lifespan, false otherwise.
     */
    bool isDecayed      () const;

    //============================================================================
  //	Getters
    //============================================================================
    /**
     * @brief Gets the unique identifier for this food item.
     * @return The food's ID.
     */
    unsigned  getID       () const;
    
    /**
     * @brief Gets the caloric value of the food.
     * @return The calories as a float value.
     */
  float     getCalories () const;
  
  /**
   * @brief Gets the current decay value.
   * @return The decay timer value.
   */
  unsigned  getDecay    () const;
  
  /**
   * @brief Gets the maximum lifespan of the food.
   * @return The lifespan value (decay limit).
   */
  unsigned  getLifespan () const;
  
    //============================================================================
  //  To String
    //============================================================================
    /**
     * @brief Converts the food object to a string representation.
     * @return A comma-separated string with all food properties.
     */
    std::string toString () const override;
};

#endif
