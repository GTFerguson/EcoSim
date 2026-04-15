#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * Runtime state for organisms that participate in combat — both
 * aggressors and defenders. Present when the organism has attack or
 * defense gene expression. Purely passive organisms (e.g. most plants)
 * may still have WoundState for damage tracking but no combat target.
 *
 * WoundState values match the existing Creature enum to ease migration.
 */
enum class WoundState : int {
    Healthy  = 0,
    Injured  = 1,
    Wounded  = 2,
    Critical = 3,
    Dead     = 4,
};

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
