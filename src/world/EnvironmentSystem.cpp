#include "world/EnvironmentSystem.hpp"
#include "world/ClimateWorldGenerator.hpp"
#include <cmath>

namespace EcoSim {

// Static default climate instance
TileClimate EnvironmentSystem::_defaultClimate = TileClimate{};

EnvironmentSystem::EnvironmentSystem(const SeasonManager& seasonManager, const WorldGrid& grid)
    : _seasonManager(seasonManager)
    , _grid(grid)
    , _climateMap(nullptr)
    , _cachedDayProgress(0.5f)
    , _cachedBaseLightLevel(0.5f)
    , _lastCachedTickId(-1)
{
}

void EnvironmentSystem::setClimateMap(const std::vector<std::vector<TileClimate>>* climateMap) {
    _climateMap = climateMap;
}

//==============================================================================
// Per-Tick Cache Management
//==============================================================================

void EnvironmentSystem::updateTickCache(int tickId) {
    if (tickId == _lastCachedTickId) {
        return;  // Already cached for this tick
    }
    
    _cachedDayProgress = _seasonManager.getDayProgress();
    
    // Pre-compute base light level (expensive sin calculation done once per tick)
    constexpr float PI = 3.14159265358979f;
    // timeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    _cachedBaseLightLevel = 0.5f + 0.5f * std::sin((_cachedDayProgress - 0.25f) * 2.0f * PI);
    _cachedBaseLightLevel = std::max(0.0f, std::min(1.0f, _cachedBaseLightLevel));
    
    _lastCachedTickId = tickId;
}

bool EnvironmentSystem::isValidPosition(int x, int y) const {
    return _grid.inBounds(x, y);
}

//==============================================================================
// Raw Climate Access
//==============================================================================

const TileClimate& EnvironmentSystem::getClimateAt(int x, int y) const {
    if (!isValidPosition(x, y) || !_climateMap) {
        return _defaultClimate;
    }
    
    // Climate map uses same indexing as grid [x][y]
    return (*_climateMap)[static_cast<size_t>(x)][static_cast<size_t>(y)];
}

//==============================================================================
// Complete Environment Query (for organisms)
//==============================================================================

Genetics::EnvironmentState EnvironmentSystem::getEnvironmentStateAt(int x, int y) const {
    if (!isValidPosition(x, y)) {
        // Return default environment for out-of-bounds
        return Genetics::EnvironmentState{};
    }
    
    if (_climateMap) {
        // Use climate-based environment with biome blending
        const TileClimate& climate = getClimateAt(x, y);
        
        // Use cached day progress if available, fall back to live query
        float timeOfDay = _cachedDayProgress;
        int season = static_cast<int>(_seasonManager.getCurrentSeason());
        
        return Genetics::EnvironmentState::fromTileClimate(climate, timeOfDay, season);
    } else {
        // Fall back to defaults for backward compatibility
        Genetics::EnvironmentState env;
        env.temperature = DEFAULT_TEMPERATURE;
        env.moisture = DEFAULT_MOISTURE;
        env.humidity = DEFAULT_HUMIDITY;
        env.elevation = DEFAULT_ELEVATION;
        env.lightLevel = DEFAULT_LIGHT_LEVEL;
        // Use cached day progress if available
        env.time_of_day = _cachedDayProgress;
        env.season = static_cast<int>(_seasonManager.getCurrentSeason());
        env.primaryBiome = static_cast<int>(Biome::TEMPERATE_GRASSLAND);
        env.vegetationDensity = 0.5f;
        env.movementCostModifier = 1.0f;
        env.terrain_type = static_cast<int>(Biome::TEMPERATE_GRASSLAND);
        return env;
    }
}

//==============================================================================
// Individual Property Queries
//==============================================================================

float EnvironmentSystem::getTemperature(int x, int y) const {
    if (_climateMap && isValidPosition(x, y)) {
        return getClimateAt(x, y).temperature;
    }
    return DEFAULT_TEMPERATURE;
}

float EnvironmentSystem::getMoisture(int x, int y) const {
    if (_climateMap && isValidPosition(x, y)) {
        return getClimateAt(x, y).moisture;
    }
    return DEFAULT_MOISTURE;
}

float EnvironmentSystem::getElevation(int x, int y) const {
    if (_climateMap && isValidPosition(x, y)) {
        return getClimateAt(x, y).elevation;
    }
    return DEFAULT_ELEVATION;
}

int EnvironmentSystem::getBiome(int x, int y) const {
    if (_climateMap && isValidPosition(x, y)) {
        return static_cast<int>(getClimateAt(x, y).biome());
    }
    return static_cast<int>(Biome::TEMPERATE_GRASSLAND);
}

float EnvironmentSystem::getHumidity(int x, int y) const {
    // Legacy alias for getMoisture
    return getMoisture(x, y);
}

float EnvironmentSystem::getLightLevel(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return DEFAULT_LIGHT_LEVEL;
    }
    
    // Use pre-computed base light level from updateTickCache()
    // This avoids expensive sin() calculation per query
    float lightLevel = _cachedBaseLightLevel;
    
    // Reduce light in dense vegetation (canopy effect) - per-tile modifier
    if (_climateMap) {
        float canopyReduction = getClimateAt(x, y).getVegetationDensity() * 0.3f;
        lightLevel *= (1.0f - canopyReduction);
    }
    
    return lightLevel;
}

float EnvironmentSystem::getWindSpeed(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return DEFAULT_WIND_SPEED;
    }
    
    // Wind system not yet implemented - return default
    return DEFAULT_WIND_SPEED;
}

float EnvironmentSystem::getWindDirection(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return DEFAULT_WIND_DIRECTION;
    }
    
    // Wind system not yet implemented - return default
    return DEFAULT_WIND_DIRECTION;
}

//==============================================================================
// Legacy TileEnvironment Query
//==============================================================================

EnvironmentSystem::TileEnvironment EnvironmentSystem::getEnvironmentAt(int x, int y) const {
    // Return all defaults if out of bounds
    if (!isValidPosition(x, y)) {
        return TileEnvironment{
            DEFAULT_TEMPERATURE,
            DEFAULT_HUMIDITY,
            DEFAULT_LIGHT_LEVEL,
            DEFAULT_WIND_SPEED,
            DEFAULT_WIND_DIRECTION
        };
    }
    
    return TileEnvironment{
        getTemperature(x, y),
        getHumidity(x, y),
        getLightLevel(x, y),
        getWindSpeed(x, y),
        getWindDirection(x, y)
    };
}

} // namespace EcoSim
