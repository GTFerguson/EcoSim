/**
 *	Title   : Ecosim - gameObjects
 *	Author	: Gary Ferguson
 *	Date		: Sep 25, 2019
 *	Purpose	: Allows the creation of generic game objects.
 */

#include "../../include/objects/gameObject.hpp"

using namespace std;

//================================================================================
//  Constructors
//================================================================================
GameObject::GameObject () {
  _name       = "";
  _desc       = "";
  _character  = ' ';
  _colour     = 1;
}

/**
 *	This is our constructor for general GameObjects.
 *
 *	@param name			  The name of the object.
 *	@param desc			  A description of the object.
 *	@param passable		Whether a creature can walk	through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 */
GameObject::GameObject (const string &name,
                        const string &desc,
                        const bool &passable, 
						            const char &character,
                        const unsigned &colour) {
	_name		    = name;
	_desc		    = desc;
	_passable	  = passable;
	_character	= character;
	_colour		  = colour;
}

/**
 *	This is our constructor for general GameObjects with 
 *	name and description set to blank.
 *
 *	@param passable		Whether a creature can walk through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 */
GameObject::GameObject (const bool &passable,
                        const char &character, 
                        const unsigned &colour) {
	_name		    = "";
	_desc		    = "";
	_passable	  = passable;
	_character  = character;
	_colour		  = colour;
}

/**
 *	This is our constructor for general GameObjects with 
 *	name, description set to blank, as well as colour to 
 *	a default value of 1.
 *
 *	@param passable		Whether a creature can walk
 *						        through it.
 *	@param character	The ASCII representation.
 */
GameObject::GameObject (const bool &passable, const char &character) {
	_name		    = "";
	_desc		    = "";
	_passable	  = passable;
	_character  = character;
	_colour		  = 1;
}

/**
 *	This is our constructor for general GameObjects with 
 *	name, description set to blank, as well as colour to 
 *	a default value of 1.
 *
 *	@param character	The ASCII representation.
 */
GameObject::GameObject (const char &character) {
	_name		    = "";
	_desc		    = "";
	_passable	  = true;
	_character  = character;
	_colour		  = 1;
}

//================================================================================
//  Getters
//================================================================================
string 		GameObject::getName 		() const { return _name;		  }
string 		GameObject::getDesc 		() const { return _desc;		  }
bool 			GameObject::getPassable	() const { return _passable;	}
char			GameObject::getChar			() const { return _character;	}
unsigned  GameObject::getColour 	() const { return _colour;		}

//================================================================================
//  To String
//================================================================================
string GameObject::toString () const {
  ostringstream ss;
  ss  << "\"" << _name      << "\"," 
      << "\"" << _desc      << "\","
      << "\"" << _character << "\","
      << _colour  << ","    << _passable;

  return ss.str();
}
