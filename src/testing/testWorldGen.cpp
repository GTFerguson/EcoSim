/**
 *	Title		  : ToT - World Gen Tester
 *	Author		: Gary Ferguson
 *	Date		  : May 18, 2017 20:22:46 (BST)
 *	Purpose		: Test different terrain gen approaches
 */

#include <stdlib.h>
#include <random>
#include <time.h>
#include <iostream>
#include <sstream>
#include "../include/world.hpp"
#include "../include/tile.hpp"
#include "../include/window.hpp"
#include "../include/objects/spawner.hpp"

using namespace std;

#define TRN_SELECTORS 11
#define ROWS  	500	
#define COLS  	500
#define LAYERS	255

/*	Origin coordinates for
	  drawing world map.	*/
int xOrigin 	= 0;
int yOrigin 	= 0;

double randSeed () {
    random_device rd;  //Will be used to obtain a seed for the random number engine
    mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    uniform_real_distribution<> dis(0, 10);

	return dis(gen);
}

void print2Dgrid 	(Window &win, vector<vector<Tile>> &grid, 
					 unsigned int &mapHeight, unsigned int &mapWidth) {
	int startx = win.getMidCol() - mapWidth/2;
	int starty = win.getMidRow() - mapHeight/2;

	for (int x = xOrigin; x < xOrigin+mapWidth && x < COLS; x++) {
		for (int y = yOrigin; y < yOrigin+mapHeight && y < ROWS; y++) {
			attron 	(COLOR_PAIR (grid[x][y].getColPair()));
			mvaddch (starty+y-yOrigin, startx+x-xOrigin, grid[x][y].getChar());
			attroff (COLOR_PAIR (grid[x][y].getColPair()));
		}
	}

	attron 	(COLOR_PAIR (2));
	//	Draw border corners
	mvaddch (starty, startx, '+');
	mvaddch (starty, startx+mapWidth, '+');
	mvaddch (starty+mapHeight, startx, '+');
	mvaddch (starty+mapHeight, startx+mapWidth, '+');
	//	Vertical border
	for (int y = 1; y < mapHeight; y++) {
		mvaddch (starty+y, startx, '|');
		mvaddch (starty+y, startx+mapWidth, '|');
	}
	//	Horizontal border
	for (int x = 1; x < mapWidth; x++) {
		mvaddch (starty, startx+x, '-');
		mvaddch (starty+mapHeight, startx+x, '-');
	}
	attroff (COLOR_PAIR (2));
}

void incTrn (World &w, const unsigned int &level) {
	w.setTerrainLevel (level, w.getTerrainLevel(level) + 1);
}

void decTrn (World &w, const unsigned int &level) {
	w.setTerrainLevel (level, w.getTerrainLevel(level) - 1);
}

string mapCreatorInput (Window &win, 
                        World &w, 
                        unsigned &mapHeight, 
					              unsigned &mapWidth, 
                        unsigned &trnSelector, 
					              bool &alive) {
	int c = getch();
	int inc = 5;

	switch (c) {
		//	Movement
		case KEY_UP: case 107:
			if (yOrigin > 0)
				yOrigin -= inc;
			else
				yOrigin = 0;
			break;
		case KEY_DOWN: case 106:
			if (yOrigin < ROWS - mapHeight)
				yOrigin += inc;
			else
				yOrigin = ROWS - mapHeight;
			break;
		case KEY_LEFT: case 104:
			if (xOrigin > 0)
				xOrigin -= inc;
			else
				xOrigin = 0;
			break;
		case KEY_RIGHT: case 108:
			if (xOrigin < COLS - mapWidth)
				xOrigin += inc;
			else
				xOrigin = COLS - mapWidth;
			break;

		case 27: case 10:	//	Escape and Enter - quit
			alive = false;
			break;

		//	Scale Handling
		case KEY_NPAGE: case 95:
			if (w.getScale() < 1)
				w.setScale (w.getScale() + 0.0001);
			else
				w.setScale (1);
			break;
		case KEY_PPAGE: case 43:
			if (w.getScale() > 0.0001)
				w.setScale (w.getScale() - 0.0001);
			else
				w.setScale (0.0001);
			break;

		//	Seed Handling
		case 110:	// n - new seed
			w.setSeed (randSeed());
			break;
		case 100:	// d - decrease seed
			w.setSeed (w.getSeed() - 0.005);
			break;
		case 102:	// f - increase seed
			w.setSeed (w.getSeed() + 0.005);
			break;

		//	Frequency Handling
		case 61:	// = - Increase freq
			w.setFreq (w.getFreq() + 0.01);
			break;
		case 45:	// - - Decrease freq
			w.setFreq (w.getFreq() - 0.01);
			break;

		//	Exponent Handling
		case 118:	// v - Increase Exponent
			w.setExponent (w.getExponent() + 0.01);
			break;
		case 99:	// c - Decrease Exponent
			w.setExponent (w.getExponent() - 0.01);
			break;

		//	Terrace Handling
		case 115:	// s - Decrease terraces
			w.setTerraces (w.getTerraces() + 1);
			break;
		case 97:	// a -  Decrease terraces
			if (w.getTerraces() > 1)
				w.setTerraces (w.getTerraces() - 1);
			else
				w.setTerraces (1);
			break;

		//	Terrain Level Handling
		// Keys 1 - 9
		case 49: case 50: case 51: case 52: case 53: 
		case 54: case 55: case 56: case 57:
			trnSelector = c - 49;
			break;

		//	Switch currently selected terrain level 
		case 119:
			incTrn (w, trnSelector);
			break;
		case 113:
			decTrn (w, trnSelector);
			break;

		default:
			break;
	}
	
	ostringstream oss;
	oss << "Key Code :: " << c;

	return oss.str();
}

/**
 *  Returns a string of the terrain levels for the users view.
 *
 *  @param w            The world object.
 *  @param trnSelector  The terrain selector.
 */
vector<string> getUI (World &w, const unsigned int trnSelector) {
	ostringstream oss;
	oss << "Seed :: " 	<< w.getSeed() 		<< "    ";
	oss << "Scale :: " 	<< w.getScale() 	<< "    ";
	oss << "Freq :: " 	<< w.getFreq() 		<< "    ";
	oss << "Expo :: " 	<< w.getExponent() 	<< "    ";
	oss << "Terr :: " 	<< w.getTerraces() 	<< "    ";

	vector<string> ui;
	ui.push_back (oss.str());
	oss.str (string());

	for (int i = 0; i < TRN_SELECTORS; i++)
		oss << " ::" << w.getTerrainLevel(i) << ":: ";

	ui.push_back (oss.str());

	return ui;
}

int main () {
	//	Colours
	init_color (7,	1000, 1000, 1000);	//	White is white
	init_color (3,	1000, 1000, 400);	  //	Make yellow nice
	init_color (12,	1000, 1000, 500);	  //	Lighter Yellow	
	init_color (8, 	500,  500,  500);		//	Mountains
	init_color (11, 0,    300,  0);			//	Mountains 3
	init_color (13, 550,  550,  550);		//	Mountains 2
	init_color (9, 	700,  700,  700);		//	Peaks
	init_color (10, 0,    700,  0);			//	Savanna
	init_color (14, 260,  340,  600);		//	Deep Water
	init_color (15, 600,  710,  700);		//	Deep Water
	//	Colour pairs
	init_pair (2, 	0,  7);		//	Boundaries	
	init_pair (16, 	6,  14);	//	Deep Water	
	init_pair (3, 	6,  4);		//	Water	
	init_pair (17, 	15, 15);	//	Shallow Water
	init_pair (6, 	15, 6);		//	Shallow Water 2
	init_pair (4, 	8,  8);		//	Mountains	
	init_pair (15, 	13, 13);	//	Mountains 2	
	init_pair (11, 	9,  9);		//	Mountains 3	
	init_pair (5, 	9,  7);		//	Peaks
	init_pair (8, 	3,  3);		//	Sand	
	init_pair (14, 	12, 12);	//	Desert Sand	
	init_pair (10,	7,  7);		//	Snow
	init_pair (1,	  2,  2);		//	Plains 
	init_pair (12,	10, 10);	//	Savanna
	init_pair (9, 	3,  10);	//	Grass	
	init_pair (13, 	11, 10);	//	Long Grass	
	init_pair (7, 	11, 10);	//	Forests	

  //  World generation
	double 			  seed		  = randSeed ();
	double 			  scale		  = 0.007;
	double 			  freq		  = 4.4; 
	double			  exponent	= 0.6;
	unsigned int 	terraces	= 64;

	World w (seed, scale, freq, exponent, terraces, ROWS, COLS, LAYERS);
	string curKeyCode;
	unsigned int trnSelector 	= 1;

  //  Window setup
	Window win;
	unsigned int mapHeight 	= win.getRow() - 5;
	unsigned int mapWidth 	= win.getCol() - 5;

	bool alive = true;

	while (alive) {
		//	Print World to Window
		print2Dgrid (win, w.getGrid()[0], mapHeight, mapWidth);
		win.printCenter (getUI(w, trnSelector), 1);

		win.printStr (curKeyCode, 0, 0);
		string newKeyCode = takeInput (win, w, mapHeight, mapWidth, trnSelector, alive);
		if (!newKeyCode.empty()) {
			curKeyCode = newKeyCode;
		}
		refresh (); clear ();
	}
	win.kill ();
	return 0;
}
