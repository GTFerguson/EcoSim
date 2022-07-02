/**
 * Title    : EcoSim - Creature
 * Author   : Gary Ferguson
 * Created  : Oct 18th, 2019
 * Purpose  : This replicates the behaviour of a basic creature or agent
 *            that is capable of decision making and has it's own genetics.
*/

#include "../../../include/objects/creature/creature.hpp"

using namespace std;

//================================================================================
//  Constants
//================================================================================
const float Creature::DIAG_ADJUST       = 1.4f;
const float Creature::RESOURCE_LIMIT    = 10.0f;
const float Creature::BREED_COST        = 3.0f;
const float Creature::IDEAL_SIMILARITY  = 0.8f;
const float Creature::PENALTY_EXPONENT  = 1.5f;
const float Creature::INIT_FATIGUE      = 0.0f;
const float Creature::PREY_CALORIES     = 3.0f;
//  Constants for managing creature death
const float Creature::STARVATION_POINT  = -0.1f;
const float Creature::DEHYDRATION_POINT = -0.1f;
const float Creature::DISCOMFORT_POINT  = -1.0f;
//  What fraction of resources is shared
const unsigned Creature::RESOURCE_SHARED = 4;

//================================================================================
//  Constructor
//================================================================================
/**
 *  Constructor for our Creature object. This version is used when breeding
 *  a new child or for manually creating a specific creature.
 *
 *  @param x          Creature's x position.
 *  @param y          Creature's y position.
 *  @param hungry     How hungry the creature currently is.
 *  @param thirsty    How thirsty the creature currently is.
 *  @param genome     This is a collection of the creatures genetic makeup.
 */
Creature::Creature (const int &x,
                    const int &y,
                    const float &hunger, 
                    const float &thirst,
                    const Genome &genome) {
  //  State Variables
  _x = x; _y = y;
  _age = 0;

  // Will Variables 
  _hunger     = hunger;
  _thirst     = thirst;
  _fatigue    = INIT_FATIGUE;
  _mate       = 0.0f;

  // Genetic Variable
  _genome     = genome; 
 
  _speed      = 1;
  _direction  = Direction::none;

  _character  = generateChar ();
  _name       = generateName ();
}

/**
 *  Constructor for our Creature object.
 *
 *  @param x      Creature's x pos.
 *  @param y      Creature's y pos.
 *  @param genome This is a collection of the creatures genetic makeup.
 */
Creature::Creature (const int &x, const int &y, const Genome &genome) {
  //  State Variables
  _x = x; _y = y;
  _age = 0;

  //  Will Variables
  _hunger   = 1.0f;
  _thirst   = 1.0f;
  _fatigue  = 0.0f;
  _mate     = 0.0f;
 
  // Genetic Variable
  _genome  = genome;

  _speed      = 1;
  _direction  = Direction::none;

  _character  = generateChar ();
  _name       = generateName ();
}

/**
 *  Constructor for our Creature object. This version is used for loading 
 *  a creature from a save file.
 *
 *	@param name			  The name of the object.
 *	@param desc			  A description of the object.
 *	@param passable		Whether a creature can walk	through it.
 *	@param character	The ASCII representation.
 *	@param colour		  The colour of the character.
 *  @param x          Creature's x position.
 *  @param y          Creature's y position.
 *  @param hungry     How hungry the creature currently is.
 *  @param thirsty    How thirsty the creature currently is.
 *  @param mate       How much the creature wants to seek a mate.
 *  @param metabolism The rate at which the creature burns through food and water.
 *  @param speed      How fast the creature traverses the environment.
 *  @param genome     This is a collection of the creatures genetic makeup.
 */
Creature::Creature (const string &name, 
                    const string &desc, 
                    const bool &passable,
                    const char &character,      
                    const unsigned &colour,
                    const int &x,
                    const int &y, 
                    const unsigned &age,
                    const string &profile,      
                    const string &direction,
                    const float &hunger,        
                    const float &thirst, 
                    const float &fatigue,
                    const float &mate,          
                    const float &metabolism, 
                    const unsigned &speed,  
                    const Genome &genome)
                    : GameObject (name, desc, passable, character, colour) {
  //  State Variables
  _x = x; _y = y;
  _age        = age;
  _profile    = stringToProfile   (profile);
  _direction  = stringToDirection (direction);
  _metabolism = metabolism;
  _speed      = speed;
  // Will Variables
  _hunger     = hunger;
  _thirst     = thirst;
  _fatigue    = fatigue;
  _mate       = mate;

  // Genetic Variable
  _genome     = genome; 
}

//================================================================================
//  Setters
//================================================================================
void Creature::setAge     (unsigned age) { _age    = age;    }
void Creature::setHunger  (float hunger) { _hunger = hunger; }
void Creature::setThirst  (float thirst) { _thirst = thirst; }
void Creature::setFatigue (float thirst) { _thirst = thirst; }
void Creature::setMate    (float mate)   { _mate   = mate;   }
void Creature::setXY      (int x, int y) { _x = x; _y = y;   }
void Creature::setX       (int x)        { _x = x;           }
void Creature::setY       (int y)        { _y = y;           }

//================================================================================
//  Getters
//================================================================================
unsigned  Creature::getAge        () const { return _age;                   }
float     Creature::getHunger     () const { return _hunger;                }
float     Creature::getThirst     () const { return _thirst;                }
float     Creature::getFatigue    () const { return _fatigue;               }
float     Creature::getMate       () const { return _mate;                  }
float     Creature::getMetabolism () const { return _metabolism;            }
unsigned  Creature::getSpeed      () const { return _speed;                 }
int       Creature::getX          () const { return _x;                     }
int       Creature::getY          () const { return _y;                     }
Direction Creature::getDirection  () const { return _direction;             }
Profile   Creature::getProfile    () const { return _profile;               }
//  Genome Getters
Genome    Creature::getGenome     () const { return _genome;                }
unsigned  Creature::getLifespan   () const { return _genome.getLifespan();  }
unsigned  Creature::getSightRange () const { return _genome.getSight();     }
float     Creature::getTHunger    () const { return _genome.getTHunger();   }
float     Creature::getTThirst    () const { return _genome.getTThirst();   }
float     Creature::getTFatigue   () const { return _genome.getTFatigue();  }
float     Creature::getTMate      () const { return _genome.getTMate();     }
float     Creature::getComfInc    () const { return _genome.getComfInc();   }
float     Creature::getComfDec    () const { return _genome.getComfDec();   }
Diet      Creature::getDiet       () const { return _genome.getDiet();      }
bool      Creature::ifFlocks      () const { return _genome.ifFlocks();     }
unsigned  Creature::getFlee       () const { return _genome.getFlee();      }
unsigned  Creature::getPursue     () const { return _genome.getPursue();    }

//================================================================================
//  Behaviours
//================================================================================
void Creature::migrateProfile (World &world,
                               vector<Creature> &creatures, 
                               const unsigned index) {
  if (ifFlocks()) {
    if (flock (world, creatures))
      return;
  }

  Navigator::wander (*this, world.getGrid(), world.getRows(), world.getCols());
}

void Creature::hungryProfile (World &world,
                              vector<Creature> &creatures,
                              const unsigned index,
                              GeneralStats &gs) {
  bool actionTaken = false;
  if (getDiet() == Diet::predator) {
    actionTaken = findPrey 
      (world.getGrid(), world.getRows(), world.getCols(), creatures, gs.deaths.predator);
  } else {
    actionTaken = findFood
      (world.getGrid(), world.getRows(), world.getCols(), gs.foodAte);
  }

  if (!actionTaken) migrateProfile (world, creatures, index);
}

void Creature::thirstyProfile (World &world,
                               vector<Creature> &creatures,
                               const unsigned index) {
  if (!findWater(world.getGrid(), world.getRows(), world.getCols()))
    migrateProfile (world, creatures, index);
}

void Creature::breedProfile (World &world,
                             vector<Creature> &creatures,
                             const unsigned index, 
                             GeneralStats &gs) {
  if (!findMate (world.getGrid(), world.getRows(), 
        world.getCols(), creatures, index, gs.births)) {
    migrateProfile (world, creatures, index);
  }
}

/**
 *  Uses a flocking algorithm to gather creatures together  
 *
 *  @param world      The world object.
 *  @param creatures  A vector of all creatures.
 *  @return           True if an actiom was taken.
 */
bool Creature::flock (World &world, vector<Creature> &creatures) {
	Creature *closestC = NULL;
  float closestDistance = getSightRange();

	//	Find closest creature
  for (Creature & creature : creatures) {
    //  Exclude itself from the check
		if (&creature != this) {
      float distance = calculateDistance(creature.getX(), creature.getY());

			if (distance < closestDistance) {
				closestC = &creature;
				closestDistance = distance;
			}
		}
	}

	if (closestC != NULL) {
    int cX = closestC->getX();
    int cY = closestC->getY();
    unsigned mapWidth   = world.getCols();
    unsigned mapHeight  = world.getRows();

    float distance = calculateDistance(cX, cY);

    unsigned flee   = getFlee();
    unsigned pursue = getPursue();

		if (distance < flee) {			
      Navigator::moveAway 
        (*this, world.getGrid(), mapHeight,  mapWidth, cX, cY);	
			return true;

    //  Creature is too far, pursue
		} else if (flee < pursue && distance > pursue) {	
			Direction cDir = closestC->getDirection();

      //  Predict where target is moving
      switch (cDir) {
        case Direction::N:  cY -= distance; break;
        case Direction::S:  cY += distance; break;
        case Direction::E:  cX += distance; break;
        case Direction::W:  cX -= distance; break;
        case Direction::NE: cX += distance/2; cY -= distance/2; break;
        case Direction::NW: cX -= distance/2; cY -= distance/2; break;
        case Direction::SE: cX += distance/2; cY += distance/2; break;
        case Direction::SW: cX -= distance;   cY += distance/2; break;
        case Direction::none: default:                          break;
      }

      //  Clamp values
      if      (cX > mapWidth - 1)   cX = mapWidth - 1;
      else if (cX < 0)              cX = 0;
      if      (cY > mapHeight - 1)  cY = mapHeight - 1;
      else if (cY < 0)              cY = 0;

      return Navigator::astarSearch
        (*this, world.getGrid(), mapHeight, mapWidth, cX, cY);
		}
	}
  return false;
}


/**
 *  This method updates the creatures variables relevant to one turn within the
 *  simulation.
 */
void Creature::update () {
  decideBehaviour();

  //  Sleeping slows down metabolism
  float change = _metabolism;
  if (_profile == Profile::sleep) {
    _fatigue  -= _metabolism;
    change    /= 2;
  } else {
    _fatigue  += _metabolism;
  }

  _hunger -= change;
  _thirst -= change;
  _age++;
}

/**
 *  This method checks the relevant values of a creature to determine
 *  whether it is dead or not. A creature can die of old age, starvation,
 *  dehydration, and of discomfort.
 *
 *  @param c The creature being checked.
 *  @return  Returns 0 if the creature is not dead.
 *            1 - Old age
 *            2 - Starvation
 *            3 - Dehydration
 *            4 - Discomfort
 */
short Creature::deathCheck () const {
  //  First check creatures age against limit remove if dead
  if      (_age     > getLifespan())      return 1;
  else if (_hunger  < STARVATION_POINT)   return 2;
  else if (_thirst  < DEHYDRATION_POINT)  return 3;
  else if (_mate    < DISCOMFORT_POINT)   return 4;

  return 0;
}

/**
 *  This method allows a creature to share a resource with another.
 *  It is given variable to divide it's current amount by to work out
 *  what is to be given.
 *  
 *  @param amount The lower the value the higher the amount
 *  @return       The amount of a resource given.
 */
float Creature::shareResource (const int& amount, float& resource) {
  float shared;

  if (resource > 0.0f) {
    shared = resource / RESOURCE_SHARED;
    resource -= shared;
  } else {
    shared = 0.0f;
  }

  return shared;
}

float Creature::shareFood (const int& amount) { 
  return shareResource (amount, _hunger); 
}

float Creature::shareWater (const int& amount) { 
  return shareResource (amount, _thirst); 
}

/**
 *  Based on the creatures needs and priorities decides their behaviour by setting
 *  the creature to the relevant profile.
 */
void Creature::decideBehaviour () {
  //  TODO implementing profile elasticity and more nuanced decision making
  // This prevents creatures getting stuck switching between resource profiles
  bool seekingResource = 
    (_profile == Profile::hungry  && _hunger < getTHunger()) ||
    (_profile == Profile::thirsty && _thirst < getTThirst());
  bool isAsleep = _profile == Profile::sleep && _fatigue > 0.0f;

  if (seekingResource) {
    _mate   -= getComfDec(); 
  } else if (!isAsleep) {
    // Calculate the priority of each behaviour
    vector<float> priorities;
    priorities.push_back (getTThirst()  - _thirst);
    priorities.push_back (getTHunger()  - _hunger);
    priorities.push_back (_mate         - getTMate());
    priorities.push_back (_fatigue      - getTFatigue());

    vector<float>::iterator highestPriority = 
      max_element  (priorities.begin(), priorities.end());
    unsigned pIndex = distance(priorities.begin(), highestPriority);

    if (*highestPriority  > 0.0f) {
      _profile = (Profile) pIndex;
      switch (_profile) {
        case Profile::thirsty: case Profile::hungry:
          _mate -= getComfDec(); 
          break;
      }

    //  All needs have been met
    } else {
      _mate     += getComfInc();
      _profile   = Profile::migrate;
    }
  }
}

bool Creature::foodCheck (const vector<vector<Tile>> &map,
                          const unsigned &rows,
                          const unsigned &cols,
                          const int &x,
                          const int &y) {
  if (Navigator::boundaryCheck (x, y, rows, cols)) {
    const vector<Food> &foodVec = map.at(x).at(y).getFoodVec();
    for (const Food & food : foodVec) {
      if (food.getID() == (unsigned)getDiet()) {
        if (Navigator::astarSearch (*this, map, rows, cols, x, y)) {
          return true; 
        }
      }
    }
  }
  return false;
}

bool Creature::waterCheck (const vector<vector<Tile>> &map,
                           const unsigned &rows,
                           const unsigned &cols,
                           const int &x,
                           const int &y) {
  if (Navigator::boundaryCheck (x, y, rows, cols)) {
    if (map.at(x).at(y).isSource()) {
      if (Navigator::astarSearch (*this, map, rows, cols, x, y))
        return true; 
    }
  }
  return false;
}

/**
 *  This method searches for the sight range of the creature for suitable food to
 *  consume. If the creature is already on suitable food it is eaten.
 *
 *  @param map  A reference to the world map.
 *  @param rows Number of rows on the map.
 *  @param cols Number of columns on the map.
 *  @return     Returns true turn has been spent.
 */
bool Creature::findFood (vector<vector<Tile>> &map,
                         const int &rows,         
                         const int &cols, 
                         unsigned &foodCounter) {
  vector<Food> *foodOnTile = &map.at(_x).at(_y).getFoodVec();
  vector<Food>::iterator it = foodOnTile->begin();

  //  Check if creature is standing on correct food type
  while (it != foodOnTile->end()) {
    if (it->getID() == (unsigned)getDiet()) {
      _hunger += it->getCalories();
      foodOnTile->erase(it);
      return true;
    } else {
      it++;
    }
  }

  unsigned maxRadius = getSightRange ();
  //  While the search radius does not exceed the available space
  for (int radius = 1; radius < maxRadius; radius++) {
    //  Top and Bottom Lines
    for (int xMod = -radius; xMod <= radius; xMod++) {
      int curX = _x + xMod;
      int curY = _y + radius;
      if (foodCheck(map, rows, cols, curX, curY)) return true;
      curY = _y - radius;
      if (foodCheck(map, rows, cols, curX, curY)) return true;
    }

    //  Right and Left Line
    for (int yMod = -radius + 1; yMod <= radius - 1; yMod++) {
      int curX = _x + radius;
      int curY = _y + yMod;
      if (foodCheck(map, rows, cols, curX, curY)) return true;
      curX = _x - radius;
      if (foodCheck(map, rows, cols, curX, curY)) return true;
    }
  }

  return false;
}

/**
 *  Simulates a basic line of sight system used for 
 *  searching the environment for water.
 *
 *  @param map  A reference to the world map.
 *  @param rows Number of rows on the map.
 *  @param cols Number of columns on the map.
 *  @return     Returns true turn has been spent.
 */
bool Creature::findWater (const vector<vector<Tile>> &map,
                          const int &rows, const int &cols) {
  //  If on water source drink from it
  if (map.at(_x).at(_y).isSource()) {
    _thirst = RESOURCE_LIMIT;
    return true;
  }

  unsigned maxRadius = getSightRange ();
  //  While the search radius does not exceed the available space
  for (int radius = 1; radius < maxRadius; radius++) {
    //  Top and Bottom Lines
    for (int xMod = -radius; xMod <= radius; xMod++) {
      int curX = _x + xMod;
      int curY = _y + radius;
      if (waterCheck(map, rows, cols, curX, curY)) return true;
      curY = _y - radius;
      if (waterCheck(map, rows, cols, curX, curY)) return true;
    }

    //  Right and Left Line
    for (int yMod = -radius + 1; yMod <= radius - 1; yMod++) {
      int curX = _x + radius;
      int curY = _y + yMod;
      if (waterCheck(map, rows, cols, curX, curY)) return true;
      curX = _x - radius;
      if (waterCheck(map, rows, cols, curX, curY)) return true;
    }
  }

  return false;
}

/**
 *  Has the current agent search for a suitable mate, if found a new offspring 
 *  will be created with the mate.
 *
 *  @param map          A reference to the world map.
 *  @param rows         Number of rows on the map.
 *  @param cols         Number of columns on the map.
 *  @param c            A vector of all creatures.
 *  @param index        The creature currently moving.
 *  @param birthCounter Aids in keeping count of creatures born.
 *  @param return       True if action taken.
 */
bool Creature::findMate (vector<vector<Tile>> &map,
                         const int &rows,
                         const int &cols,
                         vector<Creature> &creatures,
                         const unsigned &index,
                         unsigned &birthCounter) {
  float bestDesirability = -1.0f;
  Creature *bestMate = NULL;

  //  Attempt to find the most desirable mate
  for (Creature & creature : creatures) {
    bool isPotentialMate = &creature != this
        && creature.getProfile() == Profile::breed
        && creature.getDiet() == getDiet();

    if (isPotentialMate) {
      //  Calculate the distance between the creatures
      unsigned diffX = abs(_x - creature.getX());
      unsigned diffY = abs(_y - creature.getY());
      unsigned sightRange = getSightRange();

      if (diffX < sightRange && diffY < sightRange) {
        float desirability = checkFitness (creature);
        if (desirability > bestDesirability) {
          bestMate = &creature;
          bestDesirability = desirability;
        }
      }
    }
  }

  //  If mate is found seek it
  if (bestMate != NULL) {
    int mateX = bestMate->getX();
    int mateY = bestMate->getY();
    unsigned diffX = abs(_x - bestMate->getX());
    unsigned diffY = abs(_y - bestMate->getY());

    if (diffX <= 1 && diffY <= 1) {
      creatures.push_back (breedCreature(*bestMate));
      birthCounter++;
      return true;

    //  Move towards ideal mate
    } else {
      return Navigator::astarSearch (*this, map, rows, cols, mateX, mateY);
    }
  }

  return false;
}


/**
 *  Used by carnivorous creatures to seek out prey.
 *
 *  @param map          A reference to the world map.
 *  @param rows         Number of rows on the map.
 *  @param cols         Number of columns on the map.
 *  @param c            A vector of all creatures.
 *  @param return       True if action taken.
 */
bool Creature::findPrey (vector<vector<Tile>> &map,
                         const int &rows,
                         const int &cols,
                         vector<Creature> &creatures,
                         unsigned &preyAte) {
  float closestDistance = getSightRange();
  vector<Creature>::iterator creature     = creatures.begin();
  vector<Creature>::iterator closestPrey  = creature;

  while (creature != creatures.end()) {
    //  Exclude predators
    if (creature->getDiet() != Diet::predator) {
      //  Calculate the distance between the creatures
      float distance = calculateDistance(creature->getX(), creature->getY());

      if (distance < closestDistance) {
        closestPrey = creature;
        closestDistance = distance;
      }
    }
    creature++;
  }

  //  If prey is found seek it
  if (closestDistance < getSightRange()) {
    int preyX = closestPrey->getX();
    int preyY = closestPrey->getY();
    unsigned xDist = abs(_x - preyX);
    unsigned yDist = abs(_y - preyY);

    // If on prey convert it to food and eat
    if (xDist <= 1 && yDist <= 1) {
      float calories = PREY_CALORIES + closestPrey->getHunger();
      _hunger += calories;

      creatures.erase (closestPrey);

      if (_hunger > RESOURCE_LIMIT) _hunger = RESOURCE_LIMIT;
      preyAte++;
      return true;

    //  Move towards ideal mate
    } else {
      return Navigator::astarSearch (*this, map, rows, cols, preyX, preyY);
    }
  }

  return false;
}


//  TODO Change to degree system
/**
 *  Changes the direction the creature is facing to be used for line of sight
 *  as well as allowing other creatures to better predict where they are going
 *  as to allow them to more effectively move towards them.
 *
 *  @param xChange  Movement made along x-axis
 *  @param yChange  Movement made along y-axis
 */
void Creature::changeDirection (const int &xChange, const int &yChange) {
  if (xChange == 1) {
    if      (yChange ==  1) _direction = Direction::SE;
    else if (yChange == -1) _direction = Direction::NE;
    else                    _direction = Direction::E;
  } else if (xChange == -1) {
    if      (yChange ==  1) _direction = Direction::SW;
    else if (yChange == -1) _direction = Direction::NW;
    else                    _direction = Direction::W;
  } else {
    if      (yChange ==  1) _direction = Direction::S;
    else if (yChange == -1) _direction = Direction::N;
    else                    _direction = Direction::none;
  }
}

/**
 *  Calculates the euclidean distance between two points.
 *
 *  @param goalX  The x-coordinate of the destination.
 *  @param goalY  The y-coordinate of the destination. 
 */
float Creature::calculateDistance (const int &goalX, const int &goalY) const {
  float xDist = _x - goalX;
  float yDist = _y - goalY;
  return sqrt(pow(xDist, 2) + pow(yDist, 2));
}

void Creature::movementCost (const float &distance) {
  //  First get non-diagonal movement by getting absolute difference
  _hunger -= _metabolism * distance;
}

//================================================================================
//  Breeding
//================================================================================
/**
 *  The fitness is calculated as a float value between 0 and 1.5. The higher the 
 *  value returned the better the fitness, and therefore the more likely it is to
 *  breed. The proximity is given half the weight of the similarity.
 *
 *  desirability = proximity / 2 + similarity
 *
 *  @param c2 The creature who's fitness is being calculated.
 *  @return   The desirability of this pairing. 
 */
float Creature::checkFitness (const Creature &c2) const {
  float distance   = calculateDistance(c2.getX(), c2.getY());
  float proximity  = 1.0f - distance / getSightRange ();
  float similarity = _genome.compare (c2.getGenome());

  //  Penalise if too similar
  if (similarity > IDEAL_SIMILARITY) {
    similarity -= pow (similarity - IDEAL_SIMILARITY, PENALTY_EXPONENT);
  }

  return proximity / 2 + similarity;
}

/**
 *  This method takes two creatures and splices their DNA together to
 *  create offspring based on them.
 *
 *  @param c1   First parent of the new creature.
 *  @param c2   Second parent of the new creature.
 *  @return     Offspring created from the parents combined DNA.
 */
Creature Creature::breedCreature (Creature &mate) {
  Genome newGenome = _genome.spliceGenome(mate.getGenome());

  //  Charge the cost to breed to parents
  _hunger -= Creature::BREED_COST; _thirst -= Creature::BREED_COST;
  mate.setHunger (mate.getHunger() - Creature::BREED_COST);
  mate.setThirst (mate.getThirst() - Creature::BREED_COST);

  //  Give the offspring a quarter of each parents resources
  float hunger = shareFood(RESOURCE_SHARED)  + mate.shareFood(RESOURCE_SHARED);
  float thirst = shareWater(RESOURCE_SHARED) + mate.shareWater(RESOURCE_SHARED);
  //  Erroneous value check to be safe
  if (hunger > RESOURCE_LIMIT) hunger = RESOURCE_LIMIT;
  if (thirst > RESOURCE_LIMIT) thirst = RESOURCE_LIMIT;

  //  Reset the parents mating levels
  _mate = 0.0f; mate.setMate (0.0f);

	return Creature (_x, _y, hunger, thirst, newGenome);
}

//================================================================================
//  Variable Generators
//================================================================================
/**
 *  Generates a character representation of a creature based on it's diet.
 *
 *  @param diet The diet used to generate the character.
 *  @return     The character used to represent the creature.
 */
char Creature::generateChar () {
  switch (getDiet()) {
    case Diet::banana:    return 'Q'; break;
    case Diet::apple:     return '0'; break;
    case Diet::scavenger: return 'm'; break;
    case Diet::predator:  return 'M'; break;
    default:              return '?'; break;
  }
}

/**
 *  This method generates a species name for a specimen based on their genetics.
 *  Not only does this provide some interesting flavour text to the simulation,
 *  it also gives something that is a lot quicker to examine to see if certain
 *  subspecies have become dominant.
 *
 *  @return The creatures species name.
 */
string Creature::generateName () {
  string name = "";

  switch (getDiet()) {
    case Diet::banana:    name += "Musa";   break;
    case Diet::apple:     name += "Malu";   break;
    case Diet::scavenger: name += "Putre";  break;
    case Diet::predator:  name += "Caro";   break;
  };

  if (ifFlocks()) {
    unsigned  flee      = getFlee ();
    int       diffFlock = flee - getPursue();

    //  Fleeing behaviour
    if (diffFlock > 0) {
      if (flee < 10)  name += "tim";  
      else            name += "gax";

    //  Flocking behaviour
    } else {
      if      (flee < 10) name += "milia";
      else if (flee < 20) name += "micus";
      else                name += "verec";
    }

  //  Independant behaviour
  } else {
    name += "mita";
  }

  name += " ";

  unsigned lifespan = getLifespan ();
  if      (lifespan < 250000) name += "Brevi";
  else if (lifespan < 500000) name += "Aevi";
  else if (lifespan < 750000) name += "Diu";
  else                        name += "Perti";

  unsigned sight = getSightRange ();
  if      (sight < 60)  name += "caecus";
  else if (sight < 120) name += "visus";
  else                  name += "sensus";

  return name;
}

//================================================================================
//  To String
//================================================================================
Profile Creature::stringToProfile (const string &str) {
  Profile p = Profile::migrate;

  if      (str.compare("hungry")  == 0) p = Profile::hungry;
  else if (str.compare("thirsty") == 0) p = Profile::thirsty;
  else if (str.compare("sleep")   == 0) p = Profile::sleep;
  else if (str.compare("breed")   == 0) p = Profile::breed;

  return p;
}

Direction Creature::stringToDirection (const string &str) {
  Direction d = Direction::none;

  if      (str.compare("SE") == 0) d = Direction::SE;
  else if (str.compare("NE") == 0) d = Direction::NE;
  else if (str.compare("E")  == 0) d = Direction::E;
  else if (str.compare("SW") == 0) d = Direction::SW;
  else if (str.compare("NW") == 0) d = Direction::NW;
  else if (str.compare("W")  == 0) d = Direction::W;
  else if (str.compare("S")  == 0) d = Direction::S;
  else if (str.compare("N")  == 0) d = Direction::N;

  return d;
}

/**
 *  This method converts the profile to a human readable string.
 *
 *  @return A human readable string representation of the profile.
 */
string Creature::profileToString () const {
  switch(_profile) {
    case Profile::hungry:   return "hungry";  break;
    case Profile::thirsty:  return "thirsty"; break;
    case Profile::sleep:    return "sleep";   break;
    case Profile::breed:    return "breed";   break;
    case Profile::migrate:  return "migrate"; break;
    default:                return "error";   break;
  }
}

/**
 *  This method converts the direction to a human readable string.
 *
 *  @return A human readable string representation of the profile.
 */  
string Creature::directionToString () const {
  switch (_direction) {
    case Direction::SE:   return "SE";    break;
    case Direction::NE:   return "NE";    break;
    case Direction::E:    return "E";     break;
    case Direction::SW:   return "SW";    break;
    case Direction::NW:   return "NW";    break;
    case Direction::W:    return "W";     break;
    case Direction::S:    return "S";     break;
    case Direction::N:    return "N";     break;
    case Direction::none: return "none";  break;
    default:              return "error"; break;
  }
}

/**
 *  This converts the creature objects member variables to a string that can
 *  be read by a user or saved to a file.
 *
 *  @return A string representation of the creature.
 */
string Creature::toString () const {
  ostringstream ss;
  ss  << this->GameObject::toString ()  << ","
      << _x << "," << _y << "," << _age << ","
      << directionToString ()           << ","
      << profileToString ()             << ","
      << _hunger      << ","
      << _thirst      << ","
      << _fatigue     << ","
      << _mate        << "," 
      << _metabolism  << ","
      << _speed       << ","
      << _genome.toString ();

  return ss.str();
}
