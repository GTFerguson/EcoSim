#include "genetics/expression/EnvironmentState.hpp"
#include <cmath>

namespace EcoSim {
namespace Genetics {

bool EnvironmentState::significantlyDifferent(const EnvironmentState& other, float threshold) const {
    return std::abs(temperature - other.temperature) > threshold * 10.0f ||  // 10 degree threshold
           std::abs(humidity - other.humidity) > threshold ||
           terrain_type != other.terrain_type ||
           season != other.season;
}

} // namespace Genetics
} // namespace EcoSim
