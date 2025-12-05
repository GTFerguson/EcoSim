#ifndef WINDOW_H
#define WINDOW_H

//  Included for color pair definitions
#include "colorPairs.hpp"

#include <stdlib.h>
#include <ncurses.h>
#include <string>
#include <vector>

//  Keycodes
#define KEY_N_ENTER 10
#define KEY_ESCAPE  27
#define KEY_SPACE   32
#define KEY_PLUS    43  // ASCII '+' is 43
#define KEY_MINUS   45  // ASCII '-' is 45
#define KEY_DASH    45  // Same as minus
#define KEY_ONE     49
#define KEY_TWO     50
#define KEY_THREE   51
#define KEY_FOUR    52
#define KEY_FIVE    53
#define KEY_SIX     54
#define KEY_SEVEN   55
#define KEY_EIGHT   56
#define KEY_NINE    57
#define KEY_EQUAL   61
#define KEY_UNDERSCORE 95  // ASCII '_' is 95
#define KEY_A       97
#define KEY_C       99
#define KEY_D       100
#define KEY_N_F     102
#define KEY_H       104
#define KEY_J       106
#define KEY_K       107
#define KEY_L       108
#define KEY_S       115
#define KEY_N       110
#define KEY_V       118

class Window {
	private:
    const static std::string titleHeader;
    const static std::vector<std::string> titleOptions;

		WINDOW* win;
		int row, col;

	public:
    //============================================================================
		//  Constructor
    //============================================================================
		Window ();

    //============================================================================
    //  Menu
    //============================================================================
    int titleMenu ();

    //============================================================================
    //  Color Setup
    //============================================================================
    void initColors     ();
    void initColorPairs ();

    //============================================================================
 //	Getters
    //============================================================================
 int   getRow    ();
 int   getMidRow ();
 int   getCol		();
 int   getMidCol	();

    //============================================================================
		//	Print Methods
    //============================================================================
		void printCenter  (const std::string &str, int y);
		void printCenter	(const std::vector<std::string> &v, int y);
		void printString 	(const std::string &str,
                       const unsigned int &x, 
							         const unsigned int &y);
};

#endif
