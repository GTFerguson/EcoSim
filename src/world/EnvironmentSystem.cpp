#include "world/EnvironmentSystem.hpp"

namespace EcoSim {

EnvironmentSystem::EnvironmentSystem(const SeasonManager& seasonManager, const WorldGrid& grid)
    : _seasonManager(seasonManager)
    , _grid(grid)
{
}

float EnvironmentSystem::getTemperature(int x, int y) const {
    // Validate coordinates are in bounds
    if (!_grid.inBounds(x, y)) {
        return DEFAULT_TEMPERATURE;
    }
    
    // Currently returns static default for backward compatibility
    // Future: calculate based on terrain, elevation, time of day, season
    return DEFAULT_TEMPERATURE;
}

float EnvironmentSystem::getHumidity(int x, int y) const {
    if (!_grid.inBounds(x, y)) {
        return DEFAULT_HUMIDITY;
    }
    
    return DEFAULT_HUMIDITY;
}

float EnvironmentSystem::getLightLevel(int x, int y) const {
    if (!_grid.inBounds(x, y)) {
        return DEFAULT_LIGHT_LEVEL;
    }
    
    return DEFAULT_LIGHT_LEVEL;
}

float EnvironmentSystem::getWindSpeed(int x, int y) const {
    if (!_grid.inBounds(x, y)) {
        return DEFAULT_WIND_SPEED;
    }
    
    return DEFAULT_WIND_SPEED;
}

float EnvironmentSystem::getWindDirection(int x, int y) const {
    if (!_grid.inBounds(x, y)) {
        return DEFAULT_WIND_DIRECTION;
    }
    
    return DEFAULT_WIND_DIRECTION;
}

EnvironmentSystem::TileEnvironment EnvironmentSystem::getEnvironmentAt(int x, int y) const {
    // Return all defaults if out of bounds
    if (!_grid.inBounds(x, y)) {
        return TileEnvironment{
            DEFAULT_TEMPERATURE,
            DEFAULT_HUMIDITY,
            DEFAULT_LIGHT_LEVEL,
            DEFAULT_WIND_SPEED,
            DEFAULT_WIND_DIRECTION
        };
    }
    
    // Currently returns all static defaults for backward compatibility
    return TileEnvironment{
        DEFAULT_TEMPERATURE,
        DEFAULT_HUMIDITY,
        DEFAULT_LIGHT_LEVEL,
        DEFAULT_WIND_SPEED,
        DEFAULT_WIND_DIRECTION
    };
}

} // namespace EcoSim
