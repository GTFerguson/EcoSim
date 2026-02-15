#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/organisms/Organism.hpp"
#include "objects/creature/creature.hpp"
#include "world/SpatialIndex.hpp"

namespace EcoSim {
namespace Genetics {

std::vector<Organism*> BehaviorContext::queryNearbyOrganisms(float x, float y, float radius) const {
    std::vector<Organism*> result;
    if (!creatureIndex) {
        return result;
    }

    // SpatialIndex returns Creature* which inherits from Organism
    auto creatures = creatureIndex->queryRadius(x, y, radius);
    result.reserve(creatures.size());
    for (auto* c : creatures) {
        result.push_back(static_cast<EcoSim::Genetics::Organism*>(c));
    }

    return result;
}

} // namespace Genetics
} // namespace EcoSim
