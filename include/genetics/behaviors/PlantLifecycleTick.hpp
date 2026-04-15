#pragma once

#include "genetics/behaviors/IPassiveTick.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * Monolithic passive tick that wraps Plant's current procedural
 * update() logic: phenotype context refresh, environmental-stress
 * growth, fruit timer, age increment, death checks.
 *
 * Exists as a migration stepping stone — the real split into
 * PhotosynthesisTick / GrowthTick / FruitMaturationTick / etc. happens
 * later, after Creature and Plant are unified and gene-driven passive
 * tick attachment is wired in. For now this preserves the exact
 * semantics of Plant::update() while moving it behind the IPassiveTick
 * interface so the unified OrganismManager (Phase 3.6) has one dispatch
 * shape for all organisms.
 */
class PlantLifecycleTick : public IPassiveTick {
public:
    void tick(Organism& organism, const EnvironmentState& env) override;
};

} // namespace Genetics
} // namespace EcoSim
