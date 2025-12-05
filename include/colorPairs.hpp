#ifndef COLOR_PAIRS_H
#define COLOR_PAIRS_H

/**
 *  Title   : Color Pairs
 *	Author  : Gary Ferguson
 *	Date		: Jun 15, 2020 08:01:55 (BST)
 *	Purpose	: This header file holds type-safe color and color pair constants
 *	          for use with ncurses.
 */

//================================================================================
//  Additional Colors
//================================================================================
constexpr int COLOR_PALE_YELLOW = 8;
constexpr int COLOR_LIGHT_GREEN = 9;
constexpr int COLOR_DARK_GREEN  = 10;
constexpr int COLOR_LIGHT_CYAN  = 11;
constexpr int COLOR_DARK_BLUE   = 12;
constexpr int COLOR_GREY        = 13;
constexpr int COLOR_DARK_GREY   = 14;
constexpr int COLOR_OFF_WHITE   = 15;

//================================================================================
//  World Tile Color Pairs
//================================================================================
constexpr int DEFAULT_PAIR      = 1;
constexpr int D_WATER_PAIR      = 2;
constexpr int WATER_PAIR        = 3;
constexpr int S_WATER_PAIR      = 4;
constexpr int S_WATER_2_PAIR    = 5;
constexpr int SAND_PAIR         = 6;
constexpr int D_SAND_PAIR       = 7;
constexpr int PLAINS_PAIR       = 8;
constexpr int SAVANNA_PAIR      = 9;
constexpr int GRASS_PAIR        = 10;
constexpr int L_GRASS_PAIR      = 11;
constexpr int FOREST_PAIR       = 12;
constexpr int TREES_PAIR        = 13;
constexpr int MOUNTAIN_PAIR     = 14;
constexpr int MOUNTAIN_2_PAIR   = 15;
constexpr int MOUNTAIN_3_PAIR   = 16;
constexpr int SNOW_PAIR         = 17;
constexpr int PEAKS_PAIR        = 18;

//================================================================================
//  Behaviour Profile Color Pairs
//================================================================================
constexpr int HUNGRY_PAIR       = 19;
constexpr int THIRSTY_PAIR      = 20;
constexpr int SLEEP_PAIR        = 21;
constexpr int BREED_PAIR        = 22;
constexpr int MIGRATE_PAIR      = 23;

//================================================================================
//  Food Color Pairs
//================================================================================
constexpr int APPLE_PAIR        = 24;
constexpr int BANANA_PAIR       = 25;


#endif
