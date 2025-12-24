#include "genetics/expression/OrganismState.hpp"
#include <cmath>

namespace EcoSim {
namespace Genetics {

bool OrganismState::significantlyDifferent(const OrganismState& other, float threshold) const {
    return std::abs(age_normalized - other.age_normalized) > threshold ||
           std::abs(energy_level - other.energy_level) > threshold ||
           std::abs(health - other.health) > threshold ||
           is_pregnant != other.is_pregnant;
}

} // namespace Genetics
} // namespace EcoSim
