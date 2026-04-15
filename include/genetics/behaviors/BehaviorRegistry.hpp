#pragma once

#include "genetics/core/GeneRegistry.hpp"

namespace EcoSim {
namespace Genetics {

class BehaviorController;
class Phenotype;
class PerceptionSystem;
class CombatInteraction;
class FeedingInteraction;
class SeedDispersal;

/**
 * Services that behaviors need at construction. Bundled into one struct
 * so the registry doesn't take six separate parameters and so future
 * new behavior dependencies can be added without breaking the call site.
 *
 * Callers own these — the registry holds them by reference only for the
 * duration of the attach call.
 */
struct BehaviorServices {
    PerceptionSystem*  perception      = nullptr;
    CombatInteraction* combat          = nullptr;
    FeedingInteraction* feeding        = nullptr;
    SeedDispersal*     seedDispersal   = nullptr;
    GeneRegistry*      geneRegistry    = nullptr;
};

/**
 * Attaches behaviors to a BehaviorController based on gene expression.
 *
 * The hardcoded list in Creature::initializeBehaviorController() is
 * replaced by gene-threshold predicates per behavior: an organism gets
 * FeedingBehavior only if its phenotype expresses plant digestion,
 * HuntingBehavior only if it expresses hunt instinct, and so on. This
 * is the Phase 2 "capabilities determine active behaviors" vision from
 * the unified-organism-genome plan.
 *
 * For now the predicates are liberal — any positive gene value
 * attaches the behavior — because the existing Creature preset uses
 * default genes that all read > 0. As gene presets diverge this will
 * produce genuinely different behavior sets per organism.
 *
 * @param controller  controller to attach behaviors to (assumed empty)
 * @param phenotype   organism's phenotype for gene value queries
 * @param services    shared services the behaviors need
 */
class BehaviorRegistry {
public:
    static void attachBehaviorsFor(BehaviorController& controller,
                                   const Phenotype& phenotype,
                                   const BehaviorServices& services);
};

} // namespace Genetics
} // namespace EcoSim
