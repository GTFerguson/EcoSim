#include "genetics/behaviors/BehaviorRegistry.hpp"

#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/FeedingBehavior.hpp"
#include "genetics/behaviors/HuntingBehavior.hpp"
#include "genetics/behaviors/MatingBehavior.hpp"
#include "genetics/behaviors/MovementBehavior.hpp"
#include "genetics/behaviors/RestBehavior.hpp"
#include "genetics/behaviors/ZoochoryBehavior.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

namespace {

bool expressesMobility(const Phenotype& p) {
    return getTraitSafe(p, UniversalGenes::LOCOMOTION, 0.0f) > 0.0f;
}

bool expressesRest(const Phenotype& p) {
    // Anything mobile or heterotrophic accumulates fatigue and needs rest.
    return expressesMobility(p) ||
           getTraitSafe(p, UniversalGenes::HUNGER_THRESHOLD, 0.0f) > 0.0f;
}

bool expressesHunting(const Phenotype& p) {
    // Hunters need both predatory drive and the ability to move.
    return expressesMobility(p) &&
           getTraitSafe(p, UniversalGenes::MEAT_DIGESTION_EFFICIENCY, 0.0f) > 0.1f;
}

bool expressesPlantFeeding(const Phenotype& p) {
    return expressesMobility(p) &&
           getTraitSafe(p, UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 0.0f) > 0.1f;
}

bool expressesMating(const Phenotype& p) {
    return getTraitSafe(p, UniversalGenes::MATE_THRESHOLD, 0.0f) > 0.0f;
}

bool expressesZoochory(const Phenotype& p) {
    // Seed dispersal via digestion requires both mobility and plant-eating.
    return expressesPlantFeeding(p);
}

} // namespace

void BehaviorRegistry::attachBehaviorsFor(BehaviorController& controller,
                                          const Phenotype& phenotype,
                                          const BehaviorServices& services) {
    // RestBehavior — CRITICAL priority when exhausted, no service deps.
    if (expressesRest(phenotype)) {
        controller.addBehavior(std::make_unique<RestBehavior>());
    }

    // HuntingBehavior — HIGH priority for carnivores.
    if (expressesHunting(phenotype) && services.combat && services.perception) {
        controller.addBehavior(std::make_unique<HuntingBehavior>(
            *services.combat, *services.perception));
    }

    // FeedingBehavior — NORMAL priority for plant-eaters.
    if (expressesPlantFeeding(phenotype) && services.feeding && services.perception) {
        controller.addBehavior(std::make_unique<FeedingBehavior>(
            *services.feeding, *services.perception));
    }

    // MatingBehavior — NORMAL priority when ready to mate.
    if (expressesMating(phenotype) && services.perception && services.geneRegistry) {
        controller.addBehavior(std::make_unique<MatingBehavior>(
            *services.perception, *services.geneRegistry));
    }

    // ZoochoryBehavior — LOW priority seed dispersal via digestion.
    if (expressesZoochory(phenotype) && services.seedDispersal) {
        controller.addBehavior(std::make_unique<ZoochoryBehavior>(
            *services.seedDispersal));
    }

    // MovementBehavior — IDLE priority fallback wander for any mobile organism.
    if (expressesMobility(phenotype)) {
        controller.addBehavior(std::make_unique<MovementBehavior>());
    }
}

} // namespace Genetics
} // namespace EcoSim
