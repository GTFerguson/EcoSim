/**
 * @file NCursesColorMapper.cpp
 * @brief Implementation of NCursesColorMapper helper class
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file implements the ncurses color initialization and mapping
 * methods for terrain types and behavior profiles.
 */

#include "../../../../include/rendering/backends/ncurses/NCursesColorMapper.hpp"
#include "../../../../include/objects/creature/creature.hpp"

#include <ncurses.h>

//==============================================================================
// Color Initialization
//==============================================================================

void NCursesColorMapper::initializeColors() {
    // Overwrite ncurses default colours
    // Means they should now be the same across all terminals
    init_color(COLOR_BLACK,   0,    0,    0);
    init_color(COLOR_RED,     255,  0,    0);
    init_color(COLOR_GREEN,   305,  604,  23);
    init_color(COLOR_YELLOW,  1000, 1000, 400);
    init_color(COLOR_BLUE,    204,  396,  643);
    init_color(COLOR_MAGENTA, 459,  314,  482);
    init_color(COLOR_CYAN,    23,   596,  604);
    init_color(COLOR_WHITE,   1000, 1000, 1000);
    
    // Custom colours
    init_color(COLOR_PALE_YELLOW, 1000, 1000, 500);
    init_color(COLOR_LIGHT_GREEN, 0,    700,  0);
    init_color(COLOR_DARK_GREEN,  0,    300,  0);
    init_color(COLOR_LIGHT_CYAN,  600,  710,  700);
    init_color(COLOR_DARK_BLUE,   100,  300,  600);
    init_color(COLOR_GREY,        550,  550,  550);
    init_color(COLOR_DARK_GREY,   500,  500,  500);
    init_color(COLOR_OFF_WHITE,   700,  700,  700);
}

void NCursesColorMapper::initializeColorPairs() {
    // Colour pairs for the environment/terrain
    init_pair(DEFAULT_PAIR,     COLOR_WHITE,       COLOR_BLACK);
    init_pair(D_WATER_PAIR,     COLOR_CYAN,        COLOR_DARK_BLUE);
    init_pair(WATER_PAIR,       COLOR_CYAN,        COLOR_BLUE);
    init_pair(S_WATER_PAIR,     COLOR_LIGHT_CYAN,  COLOR_CYAN);
    init_pair(S_WATER_2_PAIR,   COLOR_LIGHT_CYAN,  COLOR_LIGHT_CYAN);
    init_pair(SAND_PAIR,        COLOR_YELLOW,      COLOR_YELLOW);
    init_pair(D_SAND_PAIR,      COLOR_PALE_YELLOW, COLOR_PALE_YELLOW);
    init_pair(PLAINS_PAIR,      COLOR_GREEN,       COLOR_GREEN);
    init_pair(SAVANNA_PAIR,     COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN);
    init_pair(GRASS_PAIR,       COLOR_YELLOW,      COLOR_LIGHT_GREEN);
    init_pair(L_GRASS_PAIR,     COLOR_DARK_GREEN,  COLOR_LIGHT_GREEN);
    init_pair(FOREST_PAIR,      COLOR_DARK_GREEN,  COLOR_LIGHT_GREEN);
    init_pair(TREES_PAIR,       COLOR_BLACK,       COLOR_LIGHT_GREEN);
    init_pair(MOUNTAIN_PAIR,    COLOR_DARK_GREY,   COLOR_DARK_GREY);
    init_pair(MOUNTAIN_2_PAIR,  COLOR_GREY,        COLOR_GREY);
    init_pair(MOUNTAIN_3_PAIR,  COLOR_OFF_WHITE,   COLOR_OFF_WHITE);
    init_pair(SNOW_PAIR,        COLOR_WHITE,       COLOR_WHITE);
    init_pair(PEAKS_PAIR,       COLOR_OFF_WHITE,   COLOR_WHITE);
    
    // Colour pairs used for displaying a creature's current profile
    init_pair(HUNGRY_PAIR,      COLOR_BLACK,       COLOR_YELLOW);
    init_pair(THIRSTY_PAIR,     COLOR_BLACK,       COLOR_CYAN);
    init_pair(SLEEP_PAIR,       COLOR_WHITE,       COLOR_BLACK);
    init_pair(BREED_PAIR,       COLOR_WHITE,       COLOR_MAGENTA);
    init_pair(MIGRATE_PAIR,     COLOR_BLACK,       COLOR_WHITE);
    
    // Food color pairs
    init_pair(APPLE_PAIR,       16, 16);
    init_pair(BANANA_PAIR,      3,  3);
}

bool NCursesColorMapper::initializeAll() {
    // Check if terminal supports colors
    if (!has_colors()) {
        return false;
    }
    
    // Initialize color subsystem
    if (start_color() != OK) {
        return false;
    }
    
    // Initialize custom colors and color pairs
    initializeColors();
    initializeColorPairs();
    
    return true;
}

//==============================================================================
// Terrain Color Mapping
//==============================================================================

int NCursesColorMapper::terrainToColorPair(TerrainType terrain) {
    switch (terrain) {
        case TerrainType::DEEP_WATER:      return D_WATER_PAIR;
        case TerrainType::WATER:           return WATER_PAIR;
        case TerrainType::SHALLOW_WATER:   return S_WATER_PAIR;
        case TerrainType::SHALLOW_WATER_2: return S_WATER_2_PAIR;
        case TerrainType::SAND:            return SAND_PAIR;
        case TerrainType::DESERT_SAND:     return D_SAND_PAIR;
        case TerrainType::PLAINS:          return PLAINS_PAIR;
        case TerrainType::SAVANNA:         return SAVANNA_PAIR;
        case TerrainType::SHORT_GRASS:     return GRASS_PAIR;
        case TerrainType::LONG_GRASS:      return L_GRASS_PAIR;
        case TerrainType::FOREST:          return FOREST_PAIR;
        case TerrainType::TREES:           return TREES_PAIR;
        case TerrainType::MOUNTAIN:        return MOUNTAIN_PAIR;
        case TerrainType::MOUNTAIN_2:      return MOUNTAIN_2_PAIR;
        case TerrainType::MOUNTAIN_3:      return MOUNTAIN_3_PAIR;
        case TerrainType::SNOW:            return SNOW_PAIR;
        case TerrainType::PEAKS:           return PEAKS_PAIR;
        default:                           return DEFAULT_PAIR;
    }
}

int NCursesColorMapper::elevationToColorPair(unsigned int elevation) {
    // Based on terrain levels from world.cpp
    // These elevation thresholds match the TileGen configuration
    if (elevation < 90)  return D_WATER_PAIR;      // Deep Water
    if (elevation < 110) return WATER_PAIR;        // Water
    if (elevation < 120) return S_WATER_PAIR;      // Shallow Water
    if (elevation < 130) return S_WATER_2_PAIR;    // Shallow Water 2
    if (elevation < 135) return SAND_PAIR;         // Sand
    if (elevation < 138) return D_SAND_PAIR;       // Desert Sand
    if (elevation < 155) return PLAINS_PAIR;       // Plains
    if (elevation < 160) return SAVANNA_PAIR;      // Savanna
    if (elevation < 165) return GRASS_PAIR;        // Short Grass
    if (elevation < 170) return GRASS_PAIR;        // Medium Grass (same color)
    if (elevation < 180) return L_GRASS_PAIR;      // Long Grass
    if (elevation < 200) return FOREST_PAIR;       // Forests
    if (elevation < 205) return MOUNTAIN_PAIR;     // Mountains
    if (elevation < 210) return MOUNTAIN_2_PAIR;   // Mountains 2
    if (elevation < 220) return MOUNTAIN_3_PAIR;   // Mountains 3
    if (elevation < 235) return SNOW_PAIR;         // Snow
    
    return PEAKS_PAIR;  // Peaks (255 and above)
}

//==============================================================================
// Entity Color Mapping
//==============================================================================

int NCursesColorMapper::profileToColorPair(BehaviorProfile profile) {
    switch (profile) {
        case BehaviorProfile::HUNGRY:     return HUNGRY_PAIR;
        case BehaviorProfile::THIRSTY:    return THIRSTY_PAIR;
        case BehaviorProfile::SLEEPING:   return SLEEP_PAIR;
        case BehaviorProfile::BREEDING:   return BREED_PAIR;
        case BehaviorProfile::MIGRATING:  return MIGRATE_PAIR;
        case BehaviorProfile::DEFAULT:    return DEFAULT_PAIR;
        default:                          return DEFAULT_PAIR;
    }
}

int NCursesColorMapper::motivationToColorPair(Motivation motivation) {
    // Map from the Motivation enum in creature.hpp
    switch (motivation) {
        case Motivation::Hungry:   return HUNGRY_PAIR;
        case Motivation::Thirsty:  return THIRSTY_PAIR;
        case Motivation::Tired:    return SLEEP_PAIR;
        case Motivation::Amorous:  return BREED_PAIR;
        case Motivation::Content:  return MIGRATE_PAIR;
        default:                   return DEFAULT_PAIR;
    }
}

int NCursesColorMapper::entityToColorPair(EntityType entity) {
    switch (entity) {
        case EntityType::CREATURE:         return DEFAULT_PAIR;
        case EntityType::FOOD_APPLE:       return APPLE_PAIR;
        case EntityType::FOOD_BANANA:      return BANANA_PAIR;
        case EntityType::FOOD_CORPSE:      return DEFAULT_PAIR;
        case EntityType::SPAWNER:          return SPAWNER_PAIR;
        // Phase 2.4: Genetics-based plant entity types
        case EntityType::PLANT_BERRY_BUSH: return FOREST_PAIR;   // Green for berry bushes
        case EntityType::PLANT_OAK_TREE:   return TREES_PAIR;    // Dark green for trees
        case EntityType::PLANT_GRASS:      return GRASS_PAIR;    // Yellow-green for grass
        case EntityType::PLANT_THORN_BUSH: return L_GRASS_PAIR;  // Dark green for thorns
        case EntityType::PLANT_GENERIC:    return FOREST_PAIR;   // Default green
        default:                           return DEFAULT_PAIR;
    }
}