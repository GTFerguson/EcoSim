/**
 * @file SDL2ColorMapper.hpp
 * @brief Helper class for SDL2 color management
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file provides static methods for mapping terrain types, behavior
 * profiles, and entities to SDL2 colors.
 */

#ifndef ECOSIM_SDL2_COLOR_MAPPER_HPP
#define ECOSIM_SDL2_COLOR_MAPPER_HPP

#include "../../../rendering/RenderTypes.hpp"
#include <SDL.h>

// Forward declarations
class Creature;
enum class Profile;

/**
 * @brief Helper class for SDL2 color management
 * 
 * This class provides static utility methods for mapping platform-independent
 * types (TerrainType, BehaviorProfile, EntityType) to SDL_Color values for
 * rendering with SDL2.
 */
class SDL2ColorMapper {
public:
    //==========================================================================
    // Terrain Color Mapping
    //==========================================================================
    
    /**
     * @brief Convert TerrainType to SDL_Color
     * 
     * @param terrain The terrain type to convert
     * @return SDL_Color for the terrain
     */
    static SDL_Color terrainToColor(TerrainType terrain);
    
    /**
     * @brief Convert elevation value to SDL_Color
     * 
     * Uses elevation-based terrain classification.
     * 
     * @param elevation The elevation value (0-255)
     * @return SDL_Color for the elevation
     */
    static SDL_Color elevationToColor(unsigned int elevation);

    //==========================================================================
    // Entity Color Mapping
    //==========================================================================
    
    /**
     * @brief Convert BehaviorProfile to SDL_Color
     * 
     * @param profile The behavior profile (from RenderTypes)
     * @return SDL_Color for the profile
     */
    static SDL_Color profileToColor(BehaviorProfile profile);
    
    /**
     * @brief Convert creature's Profile enum to SDL_Color
     * 
     * @param creature The creature to get color for
     * @return SDL_Color based on creature's current profile
     */
    static SDL_Color creatureProfileToColor(const Creature& creature);
    
    /**
     * @brief Convert legacy Profile enum to SDL_Color
     * 
     * @param profile The legacy Profile enum value
     * @return SDL_Color for the profile
     */
    static SDL_Color legacyProfileToColor(Profile profile);
    
    /**
     * @brief Convert EntityType to SDL_Color
     * 
     * @param entity The entity type
     * @return SDL_Color for the entity
     */
    static SDL_Color entityToColor(EntityType entity);

    //==========================================================================
    // Color Utility Methods
    //==========================================================================
    
    /**
     * @brief Create SDL_Color from RGB values
     * 
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @param a Alpha component (0-255), default 255 (opaque)
     * @return SDL_Color
     */
    static SDL_Color makeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    
    /**
     * @brief Blend two colors
     * 
     * @param c1 First color
     * @param c2 Second color
     * @param t Blend factor (0.0 = c1, 1.0 = c2)
     * @return Blended SDL_Color
     */
    static SDL_Color blendColors(SDL_Color c1, SDL_Color c2, float t);
    
    /**
     * @brief Lighten a color
     * 
     * @param color Original color
     * @param factor Lightening factor (0.0 = no change, 1.0 = white)
     * @return Lightened SDL_Color
     */
    static SDL_Color lightenColor(SDL_Color color, float factor);
    
    /**
     * @brief Darken a color
     * 
     * @param color Original color
     * @param factor Darkening factor (0.0 = no change, 1.0 = black)
     * @return Darkened SDL_Color
     */
    static SDL_Color darkenColor(SDL_Color color, float factor);

    //==========================================================================
    // Predefined Colors
    //==========================================================================
    
    // Water colors
    static SDL_Color deepWaterColor()     { return {0, 30, 100, 255}; }
    static SDL_Color waterColor()         { return {52, 101, 164, 255}; }
    static SDL_Color shallowWaterColor()  { return {85, 150, 200, 255}; }
    static SDL_Color shallowWater2Color() { return {120, 180, 210, 255}; }
    
    // Sand colors
    static SDL_Color sandColor()          { return {255, 255, 150, 255}; }
    static SDL_Color desertSandColor()    { return {255, 255, 190, 255}; }
    
    // Grass colors
    static SDL_Color plainsColor()        { return {78, 154, 6, 255}; }
    static SDL_Color savannaColor()       { return {100, 180, 30, 255}; }
    static SDL_Color grassColor()         { return {50, 150, 50, 255}; }
    static SDL_Color tallGrassColor()     { return {34, 120, 34, 255}; }
    
    // Forest colors
    static SDL_Color forestColor()        { return {34, 139, 34, 255}; }
    static SDL_Color denseForestColor()   { return {0, 100, 0, 255}; }
    
    // Mountain colors
    static SDL_Color mountainColor()      { return {128, 128, 128, 255}; }
    static SDL_Color highMountainColor()  { return {80, 80, 80, 255}; }
    static SDL_Color snowColor()          { return {255, 250, 250, 255}; }
    
    // Other terrain colors
    static SDL_Color wetlandColor()       { return {100, 130, 100, 255}; }
    static SDL_Color swampColor()         { return {60, 90, 60, 255}; }
    static SDL_Color tundraColor()        { return {200, 220, 200, 255}; }
    
    // Entity colors
    static SDL_Color foodColor()          { return {255, 200, 50, 255}; }
    static SDL_Color spawnerColor()       { return {200, 100, 255, 255}; }
    
    // Creature profile colors
    static SDL_Color hungryColor()        { return {255, 255, 0, 255}; }     // Yellow
    static SDL_Color thirstyColor()       { return {0, 255, 255, 255}; }     // Cyan
    static SDL_Color sleepyColor()        { return {50, 50, 50, 255}; }      // Dark gray
    static SDL_Color breedingColor()      { return {255, 0, 255, 255}; }     // Magenta
    static SDL_Color migratingColor()     { return {255, 255, 255, 255}; }   // White
    static SDL_Color wanderingColor()     { return {100, 200, 100, 255}; }   // Light green
    
    // UI colors
    static SDL_Color hudBackgroundColor() { return {40, 40, 40, 255}; }
    static SDL_Color textColor()          { return {255, 255, 255, 255}; }
    static SDL_Color selectedColor()      { return {100, 100, 200, 255}; }
    static SDL_Color highlightColor()     { return {255, 200, 100, 255}; }

private:
    // Prevent instantiation
    SDL2ColorMapper() = delete;
    ~SDL2ColorMapper() = delete;
    SDL2ColorMapper(const SDL2ColorMapper&) = delete;
    SDL2ColorMapper& operator=(const SDL2ColorMapper&) = delete;
};

#endif // ECOSIM_SDL2_COLOR_MAPPER_HPP
    