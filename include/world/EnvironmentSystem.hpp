#pragma once

#include "world/SeasonManager.hpp"
#include "world/WorldGrid.hpp"
#include "genetics/expression/EnvironmentState.hpp"

#include <vector>

namespace EcoSim {

// Forward declarations
struct TileClimate;
enum class Biome;

/**
 * Central environmental query system with climate map integration.
 * 
 * Provides per-tile environmental calculations based on climate data from
 * the ClimateWorldGenerator. When a climate map is connected via setClimateMap(),
 * queries return actual climate-based values. Without a climate map, queries
 * fall back to default values for backward compatibility.
 * 
 * Biome Blending: Environmental values like vegetation density and movement cost
 * are automatically blended according to the tile's BiomeBlend. Temperature and
 * moisture come directly from climate simulation (not blended).
 * 
 * Design note: This class holds a non-owning pointer to the climate map.
 * The climate map is owned by ClimateWorldGenerator and must outlive this system.
 */
class EnvironmentSystem {
public:
    /**
     * Aggregate of all environmental values at a tile (legacy format)
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
    // Climate Map Connection
    //==========================================================================
    
    /**
     * @brief Connect to climate data from world generator
     * @param climateMap Pointer to the climate map (non-owning)
     * 
     * Must be called after world generation to enable per-tile queries.
     * Passing nullptr disables climate-based queries (falls back to defaults).
     */
    void setClimateMap(const std::vector<std::vector<TileClimate>>* climateMap);
    
    /**
     * @brief Check if climate data is available
     * @return true if climate map is connected
     */
    bool hasClimateData() const { return _climateMap != nullptr; }
    
    //==========================================================================
    // Complete Environment Query (for organisms)
    //==========================================================================
    
    /**
     * @brief Get complete environment state for genetics/organism system
     * @param x X coordinate
     * @param y Y coordinate
     * @return Genetics::EnvironmentState for use by organisms
     * 
     * Returns climate-based values when available, defaults otherwise.
     * This is the primary method for organisms to query their environment.
     * Biome-derived properties (vegetation density, movement cost) use
     * weighted averages from the tile's biome blend.
     */
    Genetics::EnvironmentState getEnvironmentStateAt(int x, int y) const;
    
    //==========================================================================
    // Raw Climate Data Access
    //==========================================================================
    
    /**
     * @brief Get raw climate data for a tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Reference to TileClimate, or default climate if unavailable
     * 
     * Use this for direct access to biome blend data (e.g., for rendering).
     */
    const TileClimate& getClimateAt(int x, int y) const;
    
    //==========================================================================
    // Individual Property Queries (Enhanced with climate data)
    //==========================================================================
    
    /**
     * @brief Get temperature at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Temperature in Celsius
     */
    float getTemperature(int x, int y) const;
    
    /**
     * @brief Get moisture at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Moisture from 0.0 to 1.0
     */
    float getMoisture(int x, int y) const;
    
    /**
     * @brief Get elevation at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Elevation from 0.0 to 1.0 (normalized)
     */
    float getElevation(int x, int y) const;
    
    /**
     * @brief Get primary biome at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Biome enum value as int (dominant biome from blend)
     */
    int getBiome(int x, int y) const;
    
    /**
     * @brief Get humidity at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Humidity from 0.0 to 1.0
     * 
     * Legacy alias for getMoisture().
     */
    float getHumidity(int x, int y) const;
    
    /**
     * @brief Get light level at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Light level from 0.0 to 1.0
     * 
     * Calculated based on time of day and vegetation density.
     */
    float getLightLevel(int x, int y) const;
    
    /**
     * @brief Get wind speed at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Wind speed in m/s
     * 
     * Wind system not yet implemented - returns default.
     */
    float getWindSpeed(int x, int y) const;
    
    /**
     * @brief Get wind direction at a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @return Wind direction in degrees (0 = North, 90 = East)
     * 
     * Wind system not yet implemented - returns default.
     */
    float getWindDirection(int x, int y) const;
    
    /**
     * @brief Get all environmental values at a tile in one call (legacy format)
     * @param x X coordinate
     * @param y Y coordinate
     * @return TileEnvironment struct with all values
     * 
     * For backward compatibility. Prefer getEnvironmentStateAt() for new code.
     */
    TileEnvironment getEnvironmentAt(int x, int y) const;
    
    //==========================================================================
    // Default Values (matches current EnvironmentState for backward compat)
    //==========================================================================
    
    static constexpr float DEFAULT_TEMPERATURE = 20.0f;
    static constexpr float DEFAULT_MOISTURE = 0.5f;
    static constexpr float DEFAULT_ELEVATION = 0.5f;
    static constexpr float DEFAULT_HUMIDITY = 0.5f;
    static constexpr float DEFAULT_LIGHT_LEVEL = 1.0f;
    static constexpr float DEFAULT_WIND_SPEED = 0.0f;
    static constexpr float DEFAULT_WIND_DIRECTION = 0.0f;
    
private:
    const SeasonManager& _seasonManager;
    const WorldGrid& _grid;
    const std::vector<std::vector<TileClimate>>* _climateMap = nullptr;
    
    // Default climate for out-of-bounds or missing data
    static TileClimate _defaultClimate;
    
    // Helper to validate coordinates
    bool isValidPosition(int x, int y) const;
};

} // namespace EcoSim
