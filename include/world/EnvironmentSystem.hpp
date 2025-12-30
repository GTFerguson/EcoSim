#pragma once

#include "world/SeasonManager.hpp"
#include "world/WorldGrid.hpp"

namespace EcoSim {

/**
 * Central environmental query system.
 * 
 * Provides per-tile environmental calculations. Initially returns static
 * default values for backward compatibility with the global EnvironmentState,
 * but the interface supports future per-tile calculations based on terrain,
 * elevation, time of day, and seasons.
 * 
 * Design note: This class takes references to SeasonManager and WorldGrid
 * rather than owning them, allowing the World class to coordinate ownership
 * and lifecycle of all components.
 */
class EnvironmentSystem {
public:
    /**
     * Aggregate of all environmental values at a tile
     */
    struct TileEnvironment {
        float temperature;      // Celsius
        float humidity;         // 0.0 to 1.0
        float lightLevel;       // 0.0 to 1.0
        float windSpeed;        // m/s
        float windDirection;    // degrees, 0 = North
    };
    
    /**
     * @brief Construct environment system with required dependencies
     * @param seasonManager Reference to season manager for seasonal calculations
     * @param grid Reference to world grid for terrain-based calculations
     */
    EnvironmentSystem(const SeasonManager& seasonManager, const WorldGrid& grid);
    
    //==========================================================================
    // Per-tile Environmental Queries
    //==========================================================================
    
    /**
     * @brief Get temperature at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Temperature in Celsius
     * 
     * Currently returns DEFAULT_TEMPERATURE for backward compatibility.
     * Future implementation will calculate based on terrain, elevation,
     * time of day, and season.
     */
    float getTemperature(int x, int y) const;
    
    /**
     * @brief Get humidity at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Humidity from 0.0 to 1.0
     */
    float getHumidity(int x, int y) const;
    
    /**
     * @brief Get light level at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Light level from 0.0 to 1.0
     */
    float getLightLevel(int x, int y) const;
    
    /**
     * @brief Get wind speed at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Wind speed in m/s
     */
    float getWindSpeed(int x, int y) const;
    
    /**
     * @brief Get wind direction at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Wind direction in degrees (0 = North, 90 = East)
     */
    float getWindDirection(int x, int y) const;
    
    /**
     * @brief Get all environmental values at a tile in one call
     * @param x X coordinate
     * @param y Y coordinate
     * @return TileEnvironment struct with all values
     * 
     * More efficient than calling individual methods when multiple
     * values are needed, as it avoids redundant coordinate validation.
     */
    TileEnvironment getEnvironmentAt(int x, int y) const;
    
    //==========================================================================
    // Default Values (matches current EnvironmentState for backward compat)
    //==========================================================================
    
    static constexpr float DEFAULT_TEMPERATURE = 20.0f;
    static constexpr float DEFAULT_HUMIDITY = 0.5f;
    static constexpr float DEFAULT_LIGHT_LEVEL = 1.0f;
    static constexpr float DEFAULT_WIND_SPEED = 0.0f;
    static constexpr float DEFAULT_WIND_DIRECTION = 0.0f;
    
private:
    const SeasonManager& _seasonManager;
    const WorldGrid& _grid;
};

} // namespace EcoSim
