#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

/**
 *	Title	  : EcoSim - GameObject
 *	Author	: Gary Ferguson
 *	Created : Sep 25th, 2019
 *	Purpose : Allows the creation of generic game objects.
 */

#include <string>
#include <sstream>

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
		GameObject (const std::string &name, const std::string &desc, const bool &passable, 
					      const char &character,   const unsigned int &colour);
		GameObject (const bool &passable, const char &character, const unsigned int &colour);
		GameObject (const bool &passable, const char &character);
		GameObject (const char &character);

    //============================================================================
		//	Getters
    //============================================================================
		std::string	  getName 	  () const; 
		std::string   getDesc 	  () const; 
		bool 			    getPassable	() const; 
		char			    getChar		  () const; 
		unsigned int	getColour	  () const; 

    //============================================================================
    //  To String
    //============================================================================
    std::string toString () const;
};

#endif
