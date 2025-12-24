#pragma once

namespace EcoSim {
namespace Genetics {

// Holds only organism state data (SRP - single concern)
struct OrganismState {
    float age_normalized = 0.0f;    // 0.0 = birth, 1.0 = max lifespan
    float energy_level = 1.0f;      // 0.0 to 1.0
    float health = 1.0f;            // 0.0 to 1.0
    bool is_pregnant = false;
    bool is_sleeping = false;
    
    OrganismState() = default;
    
    // Check if state differs significantly from another
    bool significantlyDifferent(const OrganismState& other, float threshold = 0.1f) const;
};

} // namespace Genetics
} // namespace EcoSim
