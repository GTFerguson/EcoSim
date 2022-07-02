/**
 *	Title		  : EcoSim
 *	Author		: Gary Ferguson
 *	Date		  : November 18th 2019 
 *	Purpose		: This program aims to simulate the evolution of an ecosystem
 *			        through the application of Agent-Based Co-Evolutionary Algorithms
 *			        for Multi-Objective Optimization. The aim being that the ecosystem
 *			        generated would be stable and maintain biodiversity.
 */

#include "../include/statistics/statistics.hpp"
#include "../include/statistics/genomeStats.hpp"
#include "../include/objects/creature/creature.hpp"
#include "../include/objects/creature/navigator.hpp"
#include "../include/objects/food.hpp"
#include "../include/objects/spawner.hpp"
#include "../include/world/world.hpp"
#include "../include/window.hpp"
#include "../include/fileHandling.hpp"
#include "../include/calendar.hpp"

#include <stdlib.h>
#include <random>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

//================================================================================
//  General simulation constants
//================================================================================
const static unsigned TICK_RATE               = 1;
const static unsigned INITIAL_POPULATION      = 200;
const static float    STARTING_RESOURCE_MIN   = 4.0f;
const static float    STARTING_RESOURCE_MAX   = 10.0f;

//================================================================================
//  World generation defualt values
//================================================================================
const static unsigned MAP_ROWS                = 500;
const static unsigned MAP_COLS                = 500;
const static double   WORLD_DEFAULT_SCALE     = 0.0035;
const static double   WORLD_DEFAULT_FREQUENCY = 4;
const static double   WORLD_DEFAULT_EXPONENT  = 0.8;
const static unsigned WORLD_DEFAULT_TERRACES  = 64;

//================================================================================
//  Food Type constants
//================================================================================
const static float    APPLE_CALS              = 1.5f;
const static unsigned APPLE_LIFESPAN          = 1000;
const static unsigned APPLE_RATE              = 1500;
const static unsigned APPLE_MIN_RANGE         = 2;
const static unsigned APPLE_MAX_RANGE         = 4;
const static unsigned APPLE_MIN_ALTITUDE      = 180;
const static unsigned APPLE_MAX_ALTITUDE      = 195;
const static unsigned BANANA_CALS             = 2.0f;
const static unsigned BANANA_LIFESPAN         = 500;
const static unsigned BANANA_RATE             = 1000;
const static unsigned BANANA_MIN_RANGE        = 2;
const static unsigned BANANA_MAX_RANGE        = 4;
const static unsigned BANANA_MIN_ALTITUDE     = 160;
const static unsigned BANANA_MAX_ALTITUDE     = 170;
const static float    CORPSE_CALS             = 1.0f;
const static unsigned CORPSE_LIFESPAN         = 2000;

//================================================================================
//  UI Constants
//================================================================================
const static unsigned MAP_HORI_BORDER         = 2;
const static unsigned MAP_VERT_BORDER         = 4;

const static vector<string> SAVE_FILES = {
  "SAVE_01",
  "SAVE_02",
  "SAVE_03"
};

//================================================================================
//  Random Number Generator
//================================================================================
static random_device rd; 
static mt19937 gen(rd());

//================================================================================
//  Structs
//================================================================================
struct Settings { bool alive, hudIsOn, isPaused; };


//================================================================================
//  Random Number Generation 
//================================================================================
/**
 *	Generates a random double between 0 and 10 to be used as a seed
 *	for procedural generation.
 *	
 *	@return	A double value to be used as a seed.	
 */
double randSeed () {
  uniform_real_distribution<> dis(0, 10);
	return dis(gen);
}

//================================================================================
//  Window Handling
//================================================================================
/**
 *  This method takes a vector of creature objects and prints them to the screen 
 *  complete with a colour representation of their current profile.
 *
 *  @param c        The vector of creature objects.
 *	@param xOrigin  Current x origin of the world map.
 *	@param yOrigin  Current y origin of the world map.
 *	@param xScreen  Starting x coordinate for drawing to the screen.
 *	@param yScreen  Starting y coordinate for drawing to the screen.
 *	@param xRange   The range along the x-axis.
 *	@param yRange   The range along the y-axis.
 */
void printCreatures (const vector<Creature> &creatures,
                     const int &xOrigin,
                     const int &yOrigin,
                     const unsigned &xScreen, 
                     const unsigned &yScreen,
                     const unsigned &xRange, 
                     const unsigned &yRange) {
  int cX, cY;
  for (const Creature & creature : creatures) {
    cX = creature.getX();
    cY = creature.getY();

    //  Check creature is within the shown area of the map
    if (cX >= xOrigin && cX < xRange &&
        cY >= yOrigin && cY < yRange) {
      //  Visualise the creatures current profile
      int cColor;
      switch (creature.getProfile()) {
        case Profile::hungry:   cColor = HUNGRY_PAIR;   break;
        case Profile::thirsty:  cColor = THIRSTY_PAIR;  break;
        case Profile::sleep:    cColor = SLEEP_PAIR;    break;
        case Profile::breed:    cColor = BREED_PAIR;    break;
        case Profile::migrate:  cColor = MIGRATE_PAIR;  break;
      }

      attron  (COLOR_PAIR(cColor));
      mvaddch (yScreen + cY, xScreen + cX, creature.getChar());
      attroff (COLOR_PAIR(cColor));
    }
  }
}

/**
 *	Displays a 2D representation of the game world.
 *
 *	@param grid 		  A reference to the world map.
 *  @param c          The vector of creature objects.
 *	@param xOrigin    Current x origin of the world map.
 *	@param yOrigin    Current y origin of the world map.
 *	@param startx		  Starting x coordinate for drawing to the screen.
 *	@param starty		  Starting y coordinate for drawing to the screen.
 *	@param mapWidth		The width of the frame to be drawn
 *	@param mapHeight	The height of the frame to be drawn.
 */
  //  TODO make this method easier to read
void printWorld	(vector<vector<Tile>> &grid,
                 const vector<Creature> &c,
                 const int &xOrigin,
                 const int &yOrigin,
					       unsigned startx,
                 unsigned starty,
					       const unsigned &mapWidth,
                 const unsigned &mapHeight) {
  //  Center the map within the given space by getting difference 
  if (mapWidth  > MAP_COLS) startx += (mapWidth   - MAP_COLS) / 2;
  if (mapHeight > MAP_ROWS) starty += (mapHeight  - MAP_ROWS) / 2;

  //  Work out print boundaries before loops for efficiency
  unsigned xRange = mapWidth  + xOrigin;
  unsigned yRange = mapHeight + yOrigin;
  if (xRange > MAP_COLS) xRange = MAP_COLS;
  if (yRange > MAP_ROWS) yRange = MAP_ROWS;
  //  Where to draw in the terminal
  const unsigned xScreen = startx - xOrigin;
  const unsigned yScreen = starty - yOrigin;

  for (unsigned y = yOrigin; y < yRange; y++) {
    for (unsigned x = xOrigin; x < xRange; x++) {
			//	Terrain
      Tile *curTile = &grid.at(x).at(y);
			attron 	(COLOR_PAIR (curTile->getColPair()));
			mvaddch (yScreen + y, xScreen + x, curTile->getChar());
			attroff (COLOR_PAIR (curTile->getColPair()));

			//	Spawners
			const vector<Spawner> *spawners = &curTile->getSpawners();
			attron (COLOR_PAIR (TREES_PAIR));
			if (!spawners->empty())
				mvaddch (yScreen + y, xScreen + x, spawners->begin()->getChar());
			attroff	(COLOR_PAIR (TREES_PAIR));

			//	Food
			const vector<Food> *food = &curTile->getFoodVec();
			if (!food->empty())
        mvaddch (yScreen + y, xScreen + x, food->begin()->getChar());
		}
	}
  //  Creature displayal is handled differently so must be done separately
  printCreatures (c, xOrigin, yOrigin, xScreen, yScreen, xRange, yRange);
}

/**
 *  This prints to the screen all the specifications for generating a world map.
 *
 *  @param w  The world object.
 */
void printWorldDetails (const World &w) {
  mvprintw (1,  2, "Seed      : %f", w.getSeed());
  mvprintw (2,  2, "Scale     : %f", w.getScale());
  mvprintw (3,  2, "Freq      : %f", w.getFreq());
  mvprintw (4,  2, "Exponent  : %f", w.getExponent());
  mvprintw (5,  2, "Terraces  : %d", w.getTerraces());

  mvprintw (7,  2, "OCTAVES");
  mvprintw (8,  2, "Quantity        : %d", w.getOctaveGen().quantity);
  mvprintw (9,  2, "Min Weight      : %f", w.getOctaveGen().minWeight);
  mvprintw (10, 2, "Max Weight      : %f", w.getOctaveGen().maxWeight);
  mvprintw (11, 2, "Freq. Interval  : %f", w.getOctaveGen().freqInterval);
}

/**
 *  Displays some UI elements to give feedback on the simulation.
 *
 *  @parma rows     The number or rows in the window.
 *  @parma cols     The number or columns in the window.
 *  @param Calender An object that tracks the in-game date and time.
 *	@param gs       General data stored on the simuation.
 *  @param c        A vector of all creatures.
 */
void printHUD (const unsigned &rows,
               const unsigned &cols,
               const Calendar &calendar, 
               const GeneralStats &gs,
               const vector<Creature> &c) {
  //	Extra interface stuff 
  mvprintw (1,  2, "Population : %d", gs.population);
  mvprintw (2,  2, "Births :     %d", gs.births);
  mvprintw (3,  2, "Food Ate :   %d", gs.foodAte);

  mvprintw (5,  2, "Deaths");
  mvprintw (6,  2, "Old Age :    %d", gs.deaths.oldAge);
  mvprintw (7,  2, "Starved :    %d", gs.deaths.starved);
  mvprintw (8,  2, "Dehydrated : %d", gs.deaths.dehydrated);
  mvprintw (9,  2, "Discomfort : %d", gs.deaths.discomfort);
  mvprintw (10, 2, "Predator :   %d", gs.deaths.predator);

  mvprintw (rows-1, 2, calendar.shortTime().c_str());
  mvprintw (rows-1, 8, calendar.longDate().c_str());

  //  Feedback on two creature objects so we can monitor them at runtime
  if (c.size() > 0) {
    mvprintw (1, 20, "Age:     %d", c.begin()->getAge     ());
    mvprintw (2, 20, "Hunger:  %f", c.begin()->getHunger  ());
    mvprintw (3, 20, "Thirst:  %f", c.begin()->getThirst  ());
    mvprintw (4, 20, "Fatigue: %f", c.begin()->getFatigue ());
    mvprintw (5, 20, "Mate:    %f", c.begin()->getMate    ());
    mvprintw (6, 20, "Profile: ");
    addstr (c.begin()->profileToString().c_str());
    if (c.size() > 1) {
      mvprintw (1, 40, "Age:     %d", c.at(1).getAge     ());
      mvprintw (2, 40, "Hunger:  %f", c.at(1).getHunger  ());
      mvprintw (3, 40, "Thirst:  %f", c.at(1).getThirst  ());
      mvprintw (4, 40, "Fatigue: %f", c.at(1).getFatigue ());
      mvprintw (5, 40, "Mate:    %f", c.at(1).getMate    ());
      mvprintw (6, 40, "Profile: ");
      addstr (c.at(1).profileToString().c_str());
    }
  }
}

//================================================================================
//  Simulation Advancement
//================================================================================
/**
 *	This controls the behaviour of each individual creature.
 *
 *	@param w        A reference to the world map.
 *	@param gs       General data stored on the simuation.
 *	@param c	      A vector containing all of the creatures.
 *	@param cIndex	  Current creature acting.
 *	@return		      If food was successfully found.
 */
void takeTurn (World &w, GeneralStats &gs, vector<Creature> &c, 
               const unsigned int &cIndex) {
  Creature *activeC = &c.at(cIndex);

  short dc = activeC->deathCheck();
  if (dc != 0) {
    //  Record death statistic
    switch (dc) {
      case 1: gs.deaths.oldAge++;     break;
      case 2: gs.deaths.starved++;    break;
      case 3: gs.deaths.dehydrated++; break;
      case 4: gs.deaths.discomfort++; break;
    }

    //  Create and add corpse
    float calories = CORPSE_CALS + activeC->getHunger();
    if (calories > 0.0f) {
      Food corpse (2, "Corpse", "Dead animal", true, 'c', 1, calories, CORPSE_LIFESPAN);
      w.addFood (activeC->getX(), activeC->getY(), corpse);
    }

    c.erase(c.begin() + cIndex);

  //  If not dead update and take action
  } else {
    activeC->update ();

    switch(activeC->getProfile ()) {
      case Profile::migrate:  activeC->migrateProfile  (w, c, cIndex);      break;
      case Profile::hungry:   activeC->hungryProfile   (w, c, cIndex, gs);  break;
      case Profile::thirsty:  activeC->thirstyProfile  (w, c, cIndex);      break;
      case Profile::breed:    activeC->breedProfile    (w, c, cIndex, gs);  break;
      case Profile::sleep:    break;
    }
  }
}

/**
 *  Advances the simulation a singular turn
 *
 *  @param w The world object.
 *  @param c A vector of creature objects.
 */
void advanceSimulation (World &w, vector<Creature> &c, GeneralStats &gs) {
  //  Push simulation forward
  w.updateAllObjects ();
  for (size_t i = 0; i < c.size(); i++)
    takeTurn (w, gs, c, i);

  gs.population = c.size ();
}

/**
 * This method creates an initial population of creatures and
 * adds them to the world.
 *
 * @param w       A reference to the world map.
 * @param c	      A vector containing all of the creatures.
 * @param amount  The amount of creatures to be added.
 */
void populateWorld (World &w, vector<Creature> &c, unsigned amount) {
  uniform_int_distribution<int> colDis (0, MAP_COLS-1);
  uniform_int_distribution<int> rowDis (0, MAP_ROWS-1);
  uniform_real_distribution<float> dis (STARTING_RESOURCE_MIN, STARTING_RESOURCE_MAX);

	for (unsigned i = 0; i < amount; i++) {
		int x, y;
		do {
			x = colDis (gen);
			y = rowDis (gen);
		} while (w.getGrid().at(x).at(y).isPassable() == false);

    //  Randomise needs
    float hunger  = dis(gen);
    float thirst  = dis(gen);
    //  Randomise genetics
    Genome g;
    g.randomise ();
    g.setDiet (Diet::apple);
    Creature newC (x, y, hunger, thirst, g);

		c.push_back (newC);
	}
}

//================================================================================
//  Input Handling
//================================================================================
/**
 *	This handles user input from a keyboard.
 *
 *	@param win			  The window object.	
 *	@param w			    The world object.
 *	@param xOrigin    The left most point displayed.
 *	@param yOrigin    The top most point displayed.
 *	@param settings   Simulation settings.
 *	@param mapHeight	Height of the world map.
 *	@param mapWidth		Width of the world map.
 */
void takeInput 	(Window &win,
                 World &w,
                 vector<Creature> &c,
                 Calendar &calendar,
                 Statistics &stats,
                 FileHandling &file,
                 int &xOrigin,
                 int &yOrigin,
                 Settings &settings,
                 const unsigned &mapHeight,
                 const unsigned &mapWidth) {
	int ch  = getch();
	int inc = 5;

	switch (ch) {
		//	Movement on Map
    //  Up : Up key or K
		case KEY_UP: case KEY_K: 
      yOrigin -= inc;
			if (yOrigin < 0)
				yOrigin = 0;
			break;
    //  Down : Down key or j
		case KEY_DOWN: case KEY_J:
      {
        yOrigin += inc;
        int relativePos = MAP_ROWS - (int)mapHeight;
        if (yOrigin > relativePos)
          yOrigin = relativePos;
        break;
      }
    //  Left : Left key or h
		case KEY_LEFT: case KEY_H:  
      xOrigin -= inc;
			if (xOrigin < 0)
				xOrigin = 0;
			break;
    //  Right : Right key or l
		case KEY_RIGHT: case KEY_L: 
      {
        xOrigin += inc;
        int relativePos = MAP_COLS - (int)mapWidth;
        if (xOrigin > relativePos)
          xOrigin = relativePos;
        break;
      }

    //  Additional Controls
    //  HUD switch : f
    case KEY_N_F:
      settings.hudIsOn = !settings.hudIsOn;
      break;

    //  Pause : Spacebar
    case KEY_SPACE:
      settings.isPaused = !settings.isPaused;
      break;

    //  adds 100 creatures : a
    case KEY_A:
      populateWorld (w, c, 100);
      break;

    // Saves current creature genomes to file : s
    case KEY_S:
      {
        string filepath = "last_save.csv";
        file.saveGenomes (filepath, c);
        file.saveState   (w, c, calendar, stats);
        break;
      }
 
    //	Quit : Escape and Enter
		case KEY_ESCAPE:
			settings.alive = false;
			break;
	}
}

void incTrn (World &w, const unsigned &level) {
	w.setTerrainLevel (level, w.getTerrainLevel(level) + 1);
}

void decTrn (World &w, const unsigned int &level) {
	w.setTerrainLevel (level, w.getTerrainLevel(level) - 1);
}

void mapCreatorInput (Window &win, 
                      World &w, 
                      int &xOrigin, 
                      int &yOrigin,
                      unsigned &mapHeight, 
					            unsigned &mapWidth, 
                      unsigned &trnSelector, 
					            bool &alive) {
	int c = getch();
	int inc = 5;

	switch (c) {
    //  Up : Up key of k
		case KEY_UP: case KEY_K: 
      yOrigin -= inc;
			if (yOrigin < 0)
				yOrigin = 0;
			break;
    //  Down : Down key or j
		case KEY_DOWN: case KEY_J:
      {
        yOrigin += inc;
        int relativePos = MAP_ROWS - (int)mapHeight;
        if (yOrigin > relativePos)
          yOrigin = relativePos;
        break;
      }
    //  Left : Left key or h
		case KEY_LEFT: case KEY_H:  
      xOrigin -= inc;
			if (xOrigin < 0)
				xOrigin = 0;
			break;
    //  Right : Right key or l
		case KEY_RIGHT: case KEY_L: 
      {
        xOrigin += inc;
        int relativePos = MAP_COLS - (int)mapWidth;
        if (xOrigin > relativePos)
          xOrigin = relativePos;
        break;
      }

		case KEY_N_ENTER:	//	Escape and Enter - quit
			alive = false;
			break;

		//	Scale Handling
		case KEY_NPAGE: case KEY_PLUS:
			if (w.getScale() < 1)
				w.setScale (w.getScale() + 0.0001);
			else
				w.setScale (1);

      w.simplexGen ();
			break;
		case KEY_PPAGE: case KEY_MINUS:
			if (w.getScale() > 0.0001)
				w.setScale (w.getScale() - 0.0001);
			else
				w.setScale (0.0001);

      w.simplexGen ();
			break;

		//	Seed Handling
		case KEY_N:	// n - new seed
			w.setSeed (randSeed());
      w.simplexGen ();
			break;
		case KEY_D:	// d - decrease seed
			w.setSeed (w.getSeed() - 0.005);
      w.simplexGen ();
			break;
		case KEY_N_F:	// f - increase seed
			w.setSeed (w.getSeed() + 0.005);
      w.simplexGen ();
			break;

		//	Frequency Handling
		case KEY_EQUAL:	// = - Increase freq
			w.setFreq (w.getFreq() + 0.01);
      w.simplexGen ();
			break;
		case KEY_DASH:	// - - Decrease freq
			w.setFreq (w.getFreq() - 0.01);
      w.simplexGen ();
			break;

		//	Exponent Handling
		case KEY_V:	// v - Increase Exponent
			w.setExponent (w.getExponent() + 0.01);
      w.simplexGen ();
			break;
		case KEY_C:	// c - Decrease Exponent
			w.setExponent (w.getExponent() - 0.01);
      w.simplexGen ();
			break;

		//	Terrace Handling
		case KEY_S:	// s - Increase terraces
			w.setTerraces (w.getTerraces() + 1);
      w.simplexGen ();
			break;
		case KEY_A:	// a -  Decrease terraces
			if (w.getTerraces() > 1)
				w.setTerraces (w.getTerraces() - 1);
			else
				w.setTerraces (1);

      w.simplexGen ();
			break;

		//	Terrain Level Handling
		// Keys 1 - 9
    case KEY_ONE:   case KEY_TWO:   case KEY_THREE:
    case KEY_FOUR:  case KEY_FIVE:  case KEY_SIX: 
    case KEY_SEVEN: case KEY_EIGHT: case KEY_NINE:
			trnSelector = c - KEY_ONE;
			break;

		//	Switch currently selected terrain level 
		case 119:
			incTrn (w, trnSelector);
      w.simplexGen ();
			break;
		case 113:
			decTrn (w, trnSelector);
      w.simplexGen ();
			break;
	}
}

//================================================================================
//  Main Method
//================================================================================
/**
 *	This is the main game loop where all the action happens.
 *
 *	@return 0 is returned if program was successfully ran,
 *			    1 is returned if the program was terminated with an error.
 */
int main () {
  //  TODO this rand is rarely used now, can remove once it's no longer needed
  //  Initialises the pseudo-random number generator with the current time
	srand ((unsigned)time(0));

	Window win;

	//	World Generation
	double seed	= randSeed ();

  MapGen mg { seed, 
    WORLD_DEFAULT_SCALE,
    WORLD_DEFAULT_FREQUENCY, 
    WORLD_DEFAULT_EXPONENT,
    WORLD_DEFAULT_TERRACES,
    MAP_ROWS,
    MAP_COLS,
    false
  };

  OctaveGen og {
    2, 0.25, 0.5, 2
  };

  World w (mg, og);

	vector<Creature> creatures;
  Calendar calendar;
  Statistics stats;
  FileHandling file (SAVE_FILES.at(1));
  Settings settings { true, true, false };

	halfdelay (TICK_RATE);
  int menuOption = win.titleMenu ();
  erase ();

  //  Origin coordinates for drawing world map.
  int xOrigin	= 0, yOrigin = 0;
  unsigned starty     = win.getMidRow ();
  unsigned mapHeight  = win.getRow() - MAP_HORI_BORDER;
  unsigned mapWidth   = win.getCol() - MAP_VERT_BORDER;

  switch (menuOption) {
    case 0: //  Load World 
      win.printCenter ("LOADING WORLD", starty);
      refresh ();
      if (file.loadState (w, creatures, calendar, stats)) {
        break;
      } else {
        win.printCenter ("FAILED TO LOAD", starty-1);
        win.printCenter ("NEW WORLD WILL BE CREATED", starty+1);
        while (getch() == ERR);
        erase ();
      }
  
    case 1: //  New World
      {
        file.saveStatsHeader ();
        win.printCenter ("CREATING NEW WORLD", starty);
        refresh ();

        //  Edit world to liking
        bool inWorldEdit = true;
        unsigned trnSelector;
        while (inWorldEdit) {
          mapHeight   = win.getRow()    - MAP_HORI_BORDER;
          mapWidth    = win.getCol()    - MAP_VERT_BORDER;
          int startx  = win.getMidCol() - mapWidth/2;
          int starty  = win.getMidRow() - mapHeight/2;

          mapCreatorInput (win, w, xOrigin, yOrigin, mapHeight, 
                           mapWidth, trnSelector, inWorldEdit);
          erase ();
          printWorld (w.getGrid(), creatures, xOrigin, yOrigin, 
                      startx, starty, mapWidth, mapHeight);
          printWorldDetails (w);
          refresh ();
        }

        //  Add Creatures
        populateWorld (w, creatures, INITIAL_POPULATION); 
        //  Food prefabs
        Food    banana  (0, "Banana",  "A curved yellow fruit", true, ')', 1,
                         BANANA_CALS, BANANA_LIFESPAN);
        Food    apple   (1, "Apple", "A delicious red apple", true, '*', 1,
                         APPLE_CALS, APPLE_LIFESPAN);

        //  Spawner prefabs
        Spawner bananaPlant ("Banana Plant", "A tall plant that makes bananas",
                             true, 'T', 13, BANANA_RATE, BANANA_MIN_RANGE, 
                             BANANA_MAX_RANGE, banana);
        Spawner appleTree   ("Apple Tree",	"A big tree that makes apples",
                             true, '^', 13, APPLE_RATE, APPLE_MIN_RANGE, 
                             APPLE_MAX_RANGE, apple);

        w.addTrees (APPLE_MIN_ALTITUDE,   APPLE_MAX_ALTITUDE,   2, appleTree);
        w.addTrees (BANANA_MIN_ALTITUDE,  BANANA_MAX_ALTITUDE,  2, bananaPlant);
        break;
      }

    case 2: case -2:  // Quit
      settings.alive = false;
      break;
  }

	while (settings.alive) {
    mapHeight   = win.getRow()    - MAP_HORI_BORDER;
    mapWidth    = win.getCol()    - MAP_VERT_BORDER;
		int startx  = win.getMidCol() - mapWidth/2;
		int starty  = win.getMidRow() - mapHeight/2;

    GeneralStats gs = { calendar, 0, 0, 0, 0 };
    advanceSimulation (w, creatures, gs);

    do {
      erase ();
      //  Display output to terminal
      printWorld (w.getGrid(), creatures, xOrigin, yOrigin, 
          startx, starty, mapWidth, mapHeight);

      if (settings.hudIsOn)
        printHUD (win.getRow(), win.getCol(), calendar, gs, creatures);

      takeInput (win, w, creatures, calendar, stats, file, 
                 xOrigin, yOrigin, settings, mapHeight, mapWidth);

      refresh (); 
    } while (settings.isPaused && settings.alive);

    /*/
    //Saved here for testing new key codes
    char ch = getch();
    mvprintw (0, 2,   "Char : %d", ch);
    */

    //  Statistics 
    stats.addRecord (gs);
    if (calendar.getMinute() == 0) {
      if (calendar.getHour() == 0) {
        stats.accumulate ();
        string filepath = calendar.shortDate() + ".csv";
        file.saveGenomes (filepath, creatures);
        file.appendStats (stats.toString());
        stats.clearRecords ();
      } else {
        stats.accumulateByHour ();
      }
    }

    calendar++;
	}

  clear ();
  endwin ();
	return 0;
}
