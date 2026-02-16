#include "genetics/behaviors/SeedDispersalBehavior.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/interactions/SeedDispersal.hpp"

namespace EcoSim {
namespace Genetics {

SeedDispersalBehavior::SeedDispersalBehavior(SeedDispersal& seedDispersal)
    : seedDispersal_(seedDispersal) {}

std::string SeedDispersalBehavior::getId() const {
    return "seed_dispersal";
}

bool SeedDispersalBehavior::isApplicable(const Organism& organism,
                                          const BehaviorContext& /* ctx */) const {
    const auto* plant = dynamic_cast<const Plant*>(&organism);
    if (!plant) return false;
    return plant->canProduceFruit() || plant->canSpreadVegetatively();
}

float SeedDispersalBehavior::getPriority(const Organism& organism) const {
    const auto* plant = dynamic_cast<const Plant*>(&organism);
    if (!plant) return 0.0f;
    // Higher reproductive urge = higher priority
    return BASE_PRIORITY + plant->getNeeds().reproductiveUrge * 10.0f;
}

BehaviorResult SeedDispersalBehavior::execute(Organism& organism,
                                               BehaviorContext& /* ctx */) {
    BehaviorResult result;
    auto* plant = dynamic_cast<Plant*>(&organism);
    if (!plant) return result;

    // Calculate dispersal chance based on plant traits
    float dispersalChance;
    if (plant->canSpreadVegetatively()) {
        float sizeRatio = plant->getCurrentSize() / plant->getMaxSize();
        dispersalChance = plant->getRunnerProduction() * 0.15f * sizeRatio;
    } else {
        dispersalChance = plant->getFruitProductionRate() * 0.1f;
    }

    // Signal that dispersal was evaluated — PlantManager reads the result
    result.executed = true;
    result.energyCost = DISPERSAL_ENERGY_COST;

    // Probabilistic dispersal (using simple modular check since we
    // don't have RNG in the behavior — PlantManager applies full
    // probability and offspring creation)
    if (dispersalChance > 0.0f) {
        result.completed = true;
        result.debugInfo = "dispersal_ready";
    }

    plant->resetFruitTimer();

    return result;
}

float SeedDispersalBehavior::getEnergyCost(const Organism& /* organism */) const {
    return DISPERSAL_ENERGY_COST;
}

} // namespace Genetics
} // namespace EcoSim
