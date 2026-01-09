#include "genetics/expression/EnvironmentState.hpp"
#include "world/ClimateWorldGenerator.hpp"
#include <cmath>

namespace EcoSim {
namespace Genetics {

EnvironmentState EnvironmentState::fromTileClimate(
    const TileClimate& climate,
    float timeOfDay,
    int seasonValue) {
    
    EnvironmentState env;
    
    //==========================================================================
    // Core Climate Values (direct from climate simulation, NOT blended)
    //==========================================================================
    
    env.temperature = climate.temperature;
    env.moisture = climate.moisture;
    env.elevation = climate.elevation;
    env.time_of_day = timeOfDay;
    env.season = seasonValue;
    
    //==========================================================================
    // Light Level Calculation
    //==========================================================================
    
    // Base light follows sinusoidal day/night cycle
    // timeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    // sin((t - 0.25) * 2π) gives: -1 at midnight, +1 at noon
    constexpr float PI = 3.14159265358979f;
    float baseLightLevel = 0.5f + 0.5f * std::sin((timeOfDay - 0.25f) * 2.0f * PI);
    baseLightLevel = std::max(0.0f, std::min(1.0f, baseLightLevel));
    
    // Adjust for canopy cover using BLENDED vegetation density
    // Dense vegetation reduces light reaching the ground
    float canopyReduction = climate.getVegetationDensity() * 0.3f;
    env.lightLevel = baseLightLevel * (1.0f - canopyReduction);
    
    // Further adjustments for elevation (higher = more UV, but similar visible light)
    // Optional: could add seasonal variation here
    
    //==========================================================================
    // Biome-Derived Properties (BLENDED values from BiomeBlend)
    //==========================================================================
    
    // Primary biome: the dominant biome (highest weight in blend)
    // Stored as int to avoid circular include dependency
    env.primaryBiome = static_cast<int>(climate.biome());
    
    // Vegetation density: weighted average across all biomes in blend
    // Already blended by TileClimate::getVegetationDensity()
    env.vegetationDensity = climate.getVegetationDensity();
    
    // Movement cost: weighted average across all biomes in blend
    // Already blended by TileClimate::getMovementCost()
    env.movementCostModifier = climate.getMovementCost();
    
    //==========================================================================
    // Legacy Field Compatibility
    //==========================================================================
    
    // humidity is an alias for moisture
    env.humidity = env.moisture;
    
    // terrain_type uses primary biome cast to int
    env.terrain_type = env.primaryBiome;
    
    return env;
}

bool EnvironmentState::significantlyDifferent(const EnvironmentState& other, float threshold) const {
    // Check primary climate values
    // Temperature has larger absolute range, so scale threshold
    if (std::abs(temperature - other.temperature) > threshold * 10.0f) return true;
    if (std::abs(moisture - other.moisture) > threshold) return true;
    if (std::abs(elevation - other.elevation) > threshold) return true;
    if (std::abs(lightLevel - other.lightLevel) > threshold) return true;
    if (std::abs(vegetationDensity - other.vegetationDensity) > threshold) return true;
    
    // Check biome change (always significant)
    if (primaryBiome != other.primaryBiome) return true;
    
    // Check season change (always significant)
    if (season != other.season) return true;
    
    return false;
}

} // namespace Genetics
} // namespace EcoSim
