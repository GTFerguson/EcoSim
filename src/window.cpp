#include "../include/window.hpp"

using namespace std;

const string Window::titleHeader = "Deorys: Terminal Rebirth";
const vector<string> Window::titleOptions = {
  "Load World",
  "New World",
  "Quit",
};

//================================================================================
//  Constructor
//================================================================================
Window::Window () {
	if ((win = initscr()) == NULL) {
		throw std::runtime_error("Error initialising ncurses.");
	}

	keypad    (win, TRUE);
	raw       ();
	noecho    ();
	curs_set  (0);
	//	Enable Colours
	start_color     ();
  initColors      ();
  initColorPairs  ();

	getmaxyx (stdscr, row, col);
}

//================================================================================
//  Color Setup
//================================================================================
/**
 *  This method initialises all the colors used by the simulation.
 */
void Window::initColors () {
	//	Overwrite ncurses default colours
  //	Means they should now be the same across all terminals
  init_color (COLOR_BLACK,        0,    0,    0);
  init_color (COLOR_RED,          255,  0,    0);
  init_color (COLOR_GREEN,        305,  604,  23);
	init_color (COLOR_YELLOW,	      1000, 1000, 400);
  init_color (COLOR_BLUE,         204,  396,  643);
  init_color (COLOR_MAGENTA,      459,  314,  482);
  init_color (COLOR_CYAN,         23,   596,  604); 
	init_color (COLOR_WHITE,	      1000, 1000, 1000);

  //  Custom colours
	init_color (COLOR_PALE_YELLOW,	1000, 1000, 500);
	init_color (COLOR_LIGHT_GREEN,  0, 		700, 	0);
	init_color (COLOR_DARK_GREEN,   0, 		300, 	0);
	init_color (COLOR_LIGHT_CYAN,   600, 	710, 	700);
	init_color (COLOR_DARK_BLUE,    100,  300,  600);
	init_color (COLOR_GREY,         550, 	550, 	550);
	init_color (COLOR_DARK_GREY, 	  500, 	500, 	500);
	init_color (COLOR_OFF_WHITE, 	  700, 	700, 	700);
}

/**
 *  Initialises all the color pairs used by the simulation.
 */
void Window::initColorPairs () {
	//	Colour pairs for the environment
	init_pair (DEFAULT_PAIR,    COLOR_WHITE, 	      COLOR_BLACK);
	init_pair (D_WATER_PAIR, 	  COLOR_CYAN, 	      COLOR_DARK_BLUE);
	init_pair (WATER_PAIR, 	    COLOR_CYAN, 	      COLOR_BLUE);
	init_pair (S_WATER_PAIR, 	  COLOR_LIGHT_CYAN,   COLOR_CYAN);
	init_pair (S_WATER_2_PAIR, 	COLOR_LIGHT_CYAN,   COLOR_LIGHT_CYAN);
	init_pair (SAND_PAIR, 	    COLOR_YELLOW,       COLOR_YELLOW);
	init_pair (D_SAND_PAIR, 	  COLOR_PALE_YELLOW,  COLOR_PALE_YELLOW);
	init_pair (PLAINS_PAIR,	    COLOR_GREEN,        COLOR_GREEN);
	init_pair (SAVANNA_PAIR,	  COLOR_LIGHT_GREEN,  COLOR_LIGHT_GREEN);
	init_pair (GRASS_PAIR, 	    COLOR_YELLOW, 	    COLOR_LIGHT_GREEN);
	init_pair (L_GRASS_PAIR, 	  COLOR_DARK_GREEN,   COLOR_LIGHT_GREEN);
	init_pair (FOREST_PAIR, 	  COLOR_DARK_GREEN,   COLOR_LIGHT_GREEN);
	init_pair (TREES_PAIR, 	    COLOR_BLACK, 	      COLOR_LIGHT_GREEN);
	init_pair (MOUNTAIN_PAIR, 	COLOR_DARK_GREY,    COLOR_DARK_GREY);
	init_pair (MOUNTAIN_2_PAIR, COLOR_GREY,         COLOR_GREY);
	init_pair (MOUNTAIN_3_PAIR, COLOR_OFF_WHITE,    COLOR_OFF_WHITE);
	init_pair (SNOW_PAIR,	      COLOR_WHITE,        COLOR_WHITE);
	init_pair (PEAKS_PAIR, 	    COLOR_OFF_WHITE, 	  COLOR_WHITE);

  // Colour pairs used for displaying a creatures current profile
  init_pair (HUNGRY_PAIR,     COLOR_BLACK,    COLOR_YELLOW);
  init_pair (THIRSTY_PAIR,    COLOR_BLACK,    COLOR_CYAN);
  init_pair (SLEEP_PAIR,      COLOR_WHITE,    COLOR_BLACK);
  init_pair (BREED_PAIR,      COLOR_WHITE,    COLOR_MAGENTA);
  init_pair (MIGRATE_PAIR,    COLOR_BLACK,    COLOR_WHITE);

  init_pair (APPLE_PAIR,      16, 16);
  init_pair (BANANA_PAIR,     3,  3 );
}

//================================================================================
//  Menu Screens
//================================================================================
/**
 *  Prints to the window the main title screen of the simulation.
 *
 *  @return The menu option selected. Returns -1 if still in menu and -2 to quit 
 *          the simulation.
 */
int Window::titleMenu () {
  unsigned startx = getMidCol() - titleHeader.size() / 2;
  unsigned starty = getMidRow() - (titleOptions.size()+1) / 2;
  unsigned selected = 0;
  size_t optionSize = titleOptions.size();

  while (true) {
    clear ();
    printCenter (titleHeader, starty);
    for (size_t i = 0; i < optionSize; i++) {
      unsigned int cury = starty + 2 + i;

      if (selected == i) {
        mvprintw (cury, startx+3, ">>");
        attron   (A_BLINK);
      }
      printString (titleOptions.at(i), startx+6, cury);
      if (selected == i) attroff (A_BLINK);
    }
    refresh ();

    int ch = getch ();
    switch (ch) {
      //  Menu Navigation
      case KEY_UP:      if (selected > 0)             selected--; break;
      case KEY_DOWN:    if (selected < optionSize-1)  selected++; break;
      case KEY_N_ENTER: return selected;                          break;
      case KEY_ESCAPE:  return -2;                                break;
    }
  }
}

//================================================================================
//  Getters
//================================================================================
int Window::getRow    () {  return row;      }
int Window::getMidRow () {  return row / 2;  }
int Window::getCol    () {	return col;      }
int Window::getMidCol () {  return col / 2;  }

//================================================================================
//	Screen Print Helpers
//================================================================================
//	Print string to center of the current window
void Window::printCenter (const std::string &str, int y) {
	mvprintw (y, getMidCol() - str.length() / 2, str.c_str());
}

void Window::printCenter (const std::vector<std::string> &v, int y) {
	//	Set initial Y position
	int startY = y - (v.size() / 2);
	//	For each element in vector
	for (unsigned int i = 0; i < v.size(); i++) {
		printCenter (v[i], startY + i);
	}
}

void Window::printString (const std::string &str, const unsigned int &x, const unsigned int &y) {
	mvprintw (y, x, str.c_str());
}
