#pragma once

#include "genetics/systems/HealthSystem.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * Runtime state for organisms that participate in combat — both
 * aggressors and defenders. Present when the organism has attack or
 * defense gene expression. Purely passive organisms (e.g. most plants)
 * may still track wound state via HealthSystem but carry no combat
 * target.
 *
 * Reuses the canonical WoundState from HealthSystem rather than
 * redeclaring, to keep a single source of truth.
 */
struct CombatComponent {
    WoundState woundState = WoundState::Healthy;
    bool inCombat = false;
    bool isFleeing = false;
    int targetId = -1;                   // Current combat/pursuit target
    int combatCooldown = 0;              // Ticks until next attack allowed
    unsigned lastDamageTick = 0;         // For wound recovery timing
};

} // namespace Genetics
} // namespace EcoSim
