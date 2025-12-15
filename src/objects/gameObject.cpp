/**
 *	Title   : Ecosim - gameObjects
 *	Author	: Gary Ferguson
 *	Date		: Sep 25, 2019
 *	Purpose	: Allows the creation of generic game objects.
 */

#include "../../include/objects/gameObject.hpp"

#include <sstream>

using namespace std;

//================================================================================
//  Constructors
//================================================================================
GameObject::GameObject () {
  _name       = "";
  _desc       = "";
  _passable   = true;
  _character  = ' ';
  _colour     = 1;
  _entityType = EntityType::CREATURE;
}

/**
 *	This is our constructor for general GameObjects.
 *
 *	@param name			  The name of the object.
 *	@param desc			  A description of the object.
 *	@param passable		Whether a creature can walk	through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 *	@param entityType The semantic entity type for renderer-agnostic rendering.
 */
GameObject::GameObject (const string &name,
                        const string &desc,
                        bool passable,
						            char character,
                        unsigned int colour,
                        EntityType entityType) {
	_name		    = name;
	_desc		    = desc;
	_passable	  = passable;
	_character	= character;
	_colour		  = colour;
	_entityType = entityType;
}

/**
 *	This is our constructor for general GameObjects with
 *	name and description set to blank.
 *
 *	@param passable		Whether a creature can walk through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 *	@param entityType The semantic entity type for renderer-agnostic rendering.
 */
GameObject::GameObject (bool passable,
                        char character,
                        unsigned int colour,
                        EntityType entityType) {
	_name		    = "";
	_desc		    = "";
	_passable	  = passable;
	_character  = character;
	_colour		  = colour;
	_entityType = entityType;
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
GameObject::GameObject (bool passable, char character) {
	_name		    = "";
	_desc		    = "";
	_passable	  = passable;
	_character  = character;
	_colour		  = 1;
	_entityType = EntityType::CREATURE;
}

/**
 *	This is our constructor for general GameObjects with
 *	name, description set to blank, as well as colour to
 *	a default value of 1.
 *
 *	@param character	The ASCII representation.
 */
GameObject::GameObject (char character) {
	_name		    = "";
	_desc		    = "";
	_passable	  = true;
	_character  = character;
	_colour		  = 1;
	_entityType = EntityType::CREATURE;
}

//================================================================================
//  Getters
//================================================================================
string 		GameObject::getName 		() const { return _name;		  }
string 		GameObject::getDesc 		() const { return _desc;		  }
bool 			GameObject::getPassable	() const { return _passable;	}
char			GameObject::getChar			() const { return _character;	}
unsigned int GameObject::getColour() const { return _colour; }
EntityType  GameObject::getEntityType() const { return _entityType; }

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
