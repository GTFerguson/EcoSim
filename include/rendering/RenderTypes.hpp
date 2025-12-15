/**
 * @file RenderTypes.hpp
 * @brief Type definitions and data structures for the rendering system
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file contains all the platform-independent types used by the rendering
 * abstraction layer, including terrain types, entity types, viewport definitions,
 * and HUD data structures.
 */

#ifndef ECOSIM_RENDERTYPES_HPP
#define ECOSIM_RENDERTYPES_HPP

#include <string>

/**
 * @brief Terrain type enumeration for tile classification
 * 
 * These types replace raw ncurses color pair integers in Tile objects.
 * Each terrain type has associated visual properties that are defined
 * per-renderer backend.
 * 
 * Mapping from colorPairs.hpp:
 *   D_WATER_PAIR (2)      -> DEEP_WATER
 *   WATER_PAIR (3)        -> WATER
 *   S_WATER_PAIR (4)      -> SHALLOW_WATER
 *   S_WATER_2_PAIR (5)    -> SHALLOW_WATER_2
 *   SAND_PAIR (6)         -> SAND
 *   D_SAND_PAIR (7)       -> DESERT_SAND
 *   PLAINS_PAIR (8)       -> PLAINS
 *   SAVANNA_PAIR (9)      -> SAVANNA
 *   GRASS_PAIR (10)       -> SHORT_GRASS
 *   L_GRASS_PAIR (11)     -> LONG_GRASS
 *   FOREST_PAIR (12)      -> FOREST
 *   TREES_PAIR (13)       -> TREES
 *   MOUNTAIN_PAIR (14)    -> MOUNTAIN
 *   MOUNTAIN_2_PAIR (15)  -> MOUNTAIN_2
 *   MOUNTAIN_3_PAIR (16)  -> MOUNTAIN_3
 *   SNOW_PAIR (17)        -> SNOW
 *   PEAKS_PAIR (18)       -> PEAKS
 */
enum class TerrainType {
    DEEP_WATER = 0,     ///< Deep ocean water (impassable)
    WATER,              ///< Regular water
    SHALLOW_WATER,      ///< Shallow water (passable)
    SHALLOW_WATER_2,    ///< Very shallow water
    SAND,               ///< Beach/sand
    DESERT_SAND,        ///< Desert sand
    PLAINS,             ///< Open plains
    SAVANNA,            ///< Savanna grassland
    SHORT_GRASS,        ///< Short grass
    LONG_GRASS,         ///< Long/tall grass
    FOREST,             ///< Forest
    TREES,              ///< Dense trees
    MOUNTAIN,           ///< Low mountain
    MOUNTAIN_2,         ///< Medium mountain
    MOUNTAIN_3,         ///< High mountain
    SNOW,               ///< Snow-covered ground
    PEAKS,              ///< Mountain peaks (impassable)
    
    COUNT               ///< Number of terrain types (for array sizing)
};

/**
 * @brief Entity type enumeration for game objects
 * 
 * Used for creatures, food, spawners, and other game objects.
 */
enum class EntityType {
    CREATURE = 0,       ///< Generic creature
    FOOD_APPLE,         ///< Apple food item
    FOOD_BANANA,        ///< Banana food item
    FOOD_CORPSE,        ///< Corpse food item
    SPAWNER,            ///< Food spawner (tree/plant)
    
    COUNT               ///< Number of entity types
};

/**
 * @brief Behavior profile enumeration for creatures
 * 
 * Maps to the Profile enum in creature.hpp:
 *   hungry   -> HUNGRY
 *   thirsty  -> THIRSTY
 *   sleep    -> SLEEPING
 *   breed    -> BREEDING
 *   migrate  -> MIGRATING
 */
enum class BehaviorProfile {
    HUNGRY = 0,         ///< Creature is seeking food
    THIRSTY,            ///< Creature is seeking water
    SLEEPING,           ///< Creature is resting
    BREEDING,           ///< Creature is seeking mate
    MIGRATING,          ///< Creature is migrating
    DEFAULT,            ///< Default/unknown state
    
    COUNT               ///< Number of behavior profiles
};

/**
 * @brief RGB color representation
 * 
 * Platform-independent color structure that can be converted
 * to backend-specific color formats.
 */
struct Color {
    unsigned char r;    ///< Red component (0-255)
    unsigned char g;    ///< Green component (0-255)
    unsigned char b;    ///< Blue component (0-255)
    unsigned char a;    ///< Alpha component (0-255), 255 = opaque
    
    /** @brief Default constructor - creates black color */
    Color() : r(0), g(0), b(0), a(255) {}
    
    /** @brief Construct color from RGB values */
    Color(unsigned char red, unsigned char green, unsigned char blue, 
          unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
    
    // Predefined colors
    static Color Black()   { return Color(0, 0, 0); }
    static Color White()   { return Color(255, 255, 255); }
    static Color Red()     { return Color(255, 0, 0); }
    static Color Green()   { return Color(0, 255, 0); }
    static Color Blue()    { return Color(0, 0, 255); }
    static Color Yellow()  { return Color(255, 255, 0); }
    static Color Cyan()    { return Color(0, 255, 255); }
    static Color Magenta() { return Color(255, 0, 255); }
};

/**
 * @brief Viewport configuration for rendering a portion of the world
 * 
 * Defines the mapping between world coordinates and screen coordinates.
 */
struct Viewport {
    int originX;            ///< World X coordinate of top-left corner
    int originY;            ///< World Y coordinate of top-left corner
    unsigned int width;     ///< Viewport width in tiles
    unsigned int height;    ///< Viewport height in tiles
    unsigned int screenX;   ///< Screen X position to start rendering (pixels or chars)
    unsigned int screenY;   ///< Screen Y position to start rendering (pixels or chars)
    
    /** @brief Default constructor */
    Viewport() 
        : originX(0), originY(0), width(0), height(0), screenX(0), screenY(0) {}
    
    /** @brief Construct viewport with all parameters */
    Viewport(int ox, int oy, unsigned int w, unsigned int h, 
             unsigned int sx, unsigned int sy)
        : originX(ox), originY(oy), width(w), height(h), screenX(sx), screenY(sy) {}
};

/**
 * @brief HUD (Heads-Up Display) data structure
 * 
 * Contains all the statistics and information displayed in the HUD.
 * Derived from main.cpp printHUD() function (lines 278-317).
 */
struct HUDData {
    // Population statistics
    unsigned int population;    ///< Current creature count
    unsigned int births;        ///< Total births
    unsigned int foodEaten;     ///< Total food items consumed
    
    // Death statistics
    struct Deaths {
        unsigned int oldAge;        ///< Deaths from old age
        unsigned int starved;       ///< Deaths from starvation
        unsigned int dehydrated;    ///< Deaths from dehydration
        unsigned int discomfort;    ///< Deaths from discomfort
        unsigned int predator;      ///< Deaths from predators
        
        Deaths() : oldAge(0), starved(0), dehydrated(0), discomfort(0), predator(0) {}
    } deaths;
    
    // Time display
    std::string timeString;     ///< Current in-game time (short format)
    std::string dateString;     ///< Current in-game date (long format)
    
    // World information
    unsigned int worldWidth;    ///< World width in tiles
    unsigned int worldHeight;   ///< World height in tiles
    int viewportX;              ///< Current viewport X origin
    int viewportY;              ///< Current viewport Y origin
    
    // Simulation state
    unsigned int tickRate;      ///< Current simulation tick rate
    bool paused;                ///< Whether simulation is paused
    
    /** @brief Default constructor */
    HUDData() 
        : population(0), births(0), foodEaten(0), deaths()
        , timeString(""), dateString("")
        , worldWidth(0), worldHeight(0), viewportX(0), viewportY(0)
        , tickRate(1), paused(false) {}
};

/**
 * @brief Menu option structure for menu rendering
 */
struct MenuOption {
    std::string text;           ///< Display text for the option
    bool enabled;               ///< Whether option is selectable
    char hotkey;                ///< Keyboard shortcut (0 for none)
    
    /** @brief Construct menu option */
    MenuOption(const std::string& txt = "", bool en = true, char key = 0)
        : text(txt), enabled(en), hotkey(key) {}
};

/**
 * @brief Renderer capabilities flags
 * 
 * Used to query what features a renderer supports.
 */
struct RendererCapabilities {
    bool supportsColor;         ///< Can render colors
    bool supportsSprites;       ///< Can render sprite textures
    bool supportsMouse;         ///< Can handle mouse input
    bool supportsResizing;      ///< Can handle window resizing
    bool supportsUnicode;       ///< Can render Unicode characters
    
    /** @brief Default constructor - basic ASCII terminal capabilities */
    RendererCapabilities()
        : supportsColor(true), supportsSprites(false), supportsMouse(false)
        , supportsResizing(true), supportsUnicode(false) {}
};

#endif // ECOSIM_RENDERTYPES_HPP