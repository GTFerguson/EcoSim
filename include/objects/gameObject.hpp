#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

/**
 *	Title	  : EcoSim - GameObject
 *	Author	: Gary Ferguson
 *	Created : Sep 25th, 2019
 *	Purpose : Allows the creation of generic game objects.
 */

#include <string>

/**
 * @class GameObject
 * @brief Base class for all objects in the EcoSim simulation.
 *
 * Provides fundamental properties for any object that can exist
 * in the simulation world, including visual representation (character
 * and color), physical properties (passability), and metadata
 * (name and description).
 */
class GameObject {
	protected:
		std::string		_name;
		std::string		_desc;
		bool			    _passable;
		char			    _character;
		unsigned int	_colour;

	public:
	   //============================================================================
	 //	Constructor
	   //============================================================================
	 GameObject ();
	 GameObject (const std::string &name, const std::string &desc, bool passable,
	   	      char character, unsigned int colour);
	 GameObject (bool passable, char character, unsigned int colour);
	 GameObject (bool passable, char character);
	 GameObject (char character);

	   //============================================================================
	 //	Destructor
	   //============================================================================
	 virtual ~GameObject() = default;

	   //============================================================================
	 //	Getters
	   //============================================================================
	 /**
	  * @brief Gets the name of the object.
	  * @return The object's name as a string.
	  */
	 std::string	  getName 	  () const;
	 
	 /**
	  * @brief Gets the description of the object.
	  * @return The object's description as a string.
	  */
	 std::string   getDesc 	  () const;
	 
	 /**
	  * @brief Checks if the object is passable.
	  * @return true if creatures can walk through this object, false otherwise.
	  */
	 bool 			    getPassable	() const;
	 
	 /**
	  * @brief Gets the ASCII character representation.
	  * @return The character used to render this object.
	  */
	 char			    getChar		  () const;
	 
	 /**
	  * @brief Gets the color code for rendering.
	  * @return The color code as an unsigned integer.
	  */
	 unsigned int	getColour	  () const;

	   //============================================================================
	   //  To String
	   //============================================================================
	   /**
	    * @brief Converts the object to a string representation.
	    * @return A comma-separated string containing object properties.
	    */
	   virtual std::string toString () const;
};

#endif
