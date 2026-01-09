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
{
}

void EnvironmentSystem::setClimateMap(const std::vector<std::vector<TileClimate>>* climateMap) {
    _climateMap = climateMap;
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
        
        // Get time of day from season manager (normalized 0.0-1.0)
        float timeOfDay = _seasonManager.getDayProgress();
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
        env.time_of_day = _seasonManager.getDayProgress();
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
    
    // Calculate light based on time of day
    float timeOfDay = _seasonManager.getDayProgress();
    constexpr float PI = 3.14159265358979f;
    
    // Base light follows sinusoidal day/night cycle
    // timeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    float baseLightLevel = 0.5f + 0.5f * std::sin((timeOfDay - 0.25f) * 2.0f * PI);
    baseLightLevel = std::max(0.0f, std::min(1.0f, baseLightLevel));
    
    // Reduce light in dense vegetation (canopy effect)
    if (_climateMap) {
        float canopyReduction = getClimateAt(x, y).getVegetationDensity() * 0.3f;
        baseLightLevel *= (1.0f - canopyReduction);
    }
    
    return baseLightLevel;
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
