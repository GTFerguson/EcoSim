/**
 * @file NCursesColorMapper.hpp
 * @brief Helper class for ncurses color management
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file provides static methods for initializing ncurses colors
 * and mapping terrain types and behavior profiles to ncurses color pairs.
 */

#ifndef ECOSIM_NCURSES_COLOR_MAPPER_HPP
#define ECOSIM_NCURSES_COLOR_MAPPER_HPP

#include "../../../rendering/RenderTypes.hpp"

// Forward declarations to avoid ncurses include in header
// Motivation enum from creature.hpp (replaces legacy Profile enum)
enum class Motivation;

/**
 * @brief Helper class for ncurses color management
 * 
 * This class provides static utility methods for managing ncurses colors,
 * including initialization of custom colors and color pairs, and mapping
 * between platform-independent types (TerrainType, BehaviorProfile) and
 * ncurses color pair indices.
 */
class NCursesColorMapper {
public:
    //==========================================================================
    // Color Initialization
    //==========================================================================
    
    /**
     * @brief Initialize all custom colors used by the simulation
     * 
     * This initializes custom ncurses colors beyond the 8 standard colors.
     * Must be called after start_color() but before using any color pairs.
     */
    static void initializeColors();
    
    /**
     * @brief Initialize all color pairs used by the simulation
     * 
     * This creates all color pairs for terrain, creatures, food, etc.
     * Must be called after initializeColors().
     */
    static void initializeColorPairs();
    
    /**
     * @brief Perform complete color system initialization
     * 
     * Convenience method that calls both initializeColors() and
     * initializeColorPairs() in the correct order.
     * 
     * @return true if color initialization successful
     */
    static bool initializeAll();

    //==========================================================================
    // Terrain Color Mapping
    //==========================================================================
    
    /**
     * @brief Convert TerrainType to ncurses color pair index
     * 
     * @param terrain The terrain type to convert
     * @return ncurses color pair index
     */
    static int terrainToColorPair(TerrainType terrain);
    
    /**
     * @brief Convert elevation value to ncurses color pair index
     * 
     * This uses the existing elevation-based system from world.cpp.
     * 
     * @param elevation The elevation value (0-255)
     * @return ncurses color pair index
     */
    static int elevationToColorPair(unsigned int elevation);

    //==========================================================================
    // Entity Color Mapping
    //==========================================================================
    
    /**
     * @brief Convert BehaviorProfile to ncurses color pair index
     * 
     * @param profile The behavior profile (from RenderTypes)
     * @return ncurses color pair index
     */
    static int profileToColorPair(BehaviorProfile profile);
    
    /**
     * @brief Convert Motivation enum to ncurses color pair index
     *
     * This method handles the Motivation enum from creature.hpp.
     *
     * @param motivation The Motivation enum value
     * @return ncurses color pair index
     */
    static int motivationToColorPair(Motivation motivation);
    
    /**
     * @brief Convert EntityType to ncurses color pair index
     * 
     * @param entity The entity type
     * @return ncurses color pair index
     */
    static int entityToColorPair(EntityType entity);

    //==========================================================================
    // Color Pair Constants
    //==========================================================================
    
    // Custom colors (indices 8-15)
    static constexpr int COLOR_PALE_YELLOW = 8;
    static constexpr int COLOR_LIGHT_GREEN = 9;
    static constexpr int COLOR_DARK_GREEN  = 10;
    static constexpr int COLOR_LIGHT_CYAN  = 11;
    static constexpr int COLOR_DARK_BLUE   = 12;
    static constexpr int COLOR_GREY        = 13;
    static constexpr int COLOR_DARK_GREY   = 14;
    static constexpr int COLOR_OFF_WHITE   = 15;
    
    // World tile color pairs
    static constexpr int DEFAULT_PAIR      = 1;
    static constexpr int D_WATER_PAIR      = 2;
    static constexpr int WATER_PAIR        = 3;
    static constexpr int S_WATER_PAIR      = 4;
    static constexpr int S_WATER_2_PAIR    = 5;
    static constexpr int SAND_PAIR         = 6;
    static constexpr int D_SAND_PAIR       = 7;
    static constexpr int PLAINS_PAIR       = 8;
    static constexpr int SAVANNA_PAIR      = 9;
    static constexpr int GRASS_PAIR        = 10;
    static constexpr int L_GRASS_PAIR      = 11;
    static constexpr int FOREST_PAIR       = 12;
    static constexpr int TREES_PAIR        = 13;
    static constexpr int MOUNTAIN_PAIR     = 14;
    static constexpr int MOUNTAIN_2_PAIR   = 15;
    static constexpr int MOUNTAIN_3_PAIR   = 16;
    static constexpr int SNOW_PAIR         = 17;
    static constexpr int PEAKS_PAIR        = 18;
    
    // Behavior profile color pairs
    static constexpr int HUNGRY_PAIR       = 19;
    static constexpr int THIRSTY_PAIR      = 20;
    static constexpr int SLEEP_PAIR        = 21;
    static constexpr int BREED_PAIR        = 22;
    static constexpr int MIGRATE_PAIR      = 23;
    
    // Food color pairs
    static constexpr int APPLE_PAIR        = 24;
    static constexpr int BANANA_PAIR       = 25;
    
    // Spawner/misc color pair
    static constexpr int SPAWNER_PAIR      = 13;  // Same as TREES_PAIR

private:
    // Private constructor - all methods are static
    NCursesColorMapper() = default;
    
    // Prevent copying
    NCursesColorMapper(const NCursesColorMapper&) = delete;
    NCursesColorMapper& operator=(const NCursesColorMapper&) = delete;
};

#endif // ECOSIM_NCURSES_COLOR_MAPPER_HPP