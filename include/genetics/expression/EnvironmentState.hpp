#pragma once

namespace EcoSim {
namespace Genetics {

// Holds only environment data (SRP - single concern)
struct EnvironmentState {
    float temperature = 20.0f;      // Celsius
    float humidity = 0.5f;          // 0.0 to 1.0
    float time_of_day = 0.5f;       // 0.0 to 1.0 (0 = midnight, 0.5 = noon)
    int terrain_type = 0;           // Terrain enum value
    int season = 0;                 // Season enum value
    
    EnvironmentState() = default;
    
    // Check if environment differs significantly from another
    bool significantlyDifferent(const EnvironmentState& other, float threshold = 0.1f) const;
};

} // namespace Genetics
} // namespace EcoSim
