#ifndef CREATURE_COMBAT_HPP
#define CREATURE_COMBAT_HPP

/**
 * @file CreatureCombat.hpp
 * @brief Combat system functionality extracted from Creature class
 *
 * This module handles combat hunting, prey finding, and combat state management.
 * Part of Phase 1a creature decomposition to improve maintainability.
 */

#include <vector>

class Creature;
class World;
class Tile;
struct GeneralStats;

namespace CreatureCombat {

//============================================================================
//  Combat Hunting
//============================================================================

/**
 * @brief Search for and pursue prey (carnivore behavior).
 *
 * Searches within creature's sight range for non-predator prey.
 * If prey is adjacent, engages in combat using CombatInteraction.
 * Otherwise navigates toward closest prey using A* pathfinding.
 *
 * @param creature The hunting creature
 * @param map World tile grid
 * @param rows Number of grid rows
 * @param cols Number of grid columns
 * @param creatures Vector of all creatures in world
 * @param preyAte Counter incremented when prey is killed
 * @return true if action was taken (attacking, chasing, or hunting)
 */
bool findPrey(Creature& creature,
              std::vector<std::vector<Tile>>& map,
              const int& rows,
              const int& cols,
              std::vector<Creature>& creatures,
              unsigned& preyAte);

//============================================================================
//  Combat State Management
//============================================================================

/**
 * @brief Check if creature is currently in combat.
 * @param creature The creature to check
 * @return true if in combat
 */
bool isInCombat(const Creature& creature);

/**
 * @brief Check if creature is currently fleeing.
 * @param creature The creature to check
 * @return true if fleeing
 */
bool isFleeing(const Creature& creature);

/**
 * @brief Get the ID of creature's current combat target.
 * @param creature The creature to check
 * @return Target ID, or -1 if no target
 */
int getTargetId(const Creature& creature);

/**
 * @brief Get remaining combat cooldown ticks.
 * @param creature The creature to check
 * @return Cooldown ticks remaining
 */
int getCombatCooldown(const Creature& creature);

/**
 * @brief Enter combat state with a specific target.
 *
 * Sets inCombat flag and target ID. Use when initiating an attack.
 *
 * @param creature The creature entering combat
 * @param targetId ID of the creature being attacked
 */
void enterCombat(Creature& creature, int targetId);

/**
 * @brief Exit combat state.
 *
 * Clears inCombat flag and target ID. Use after combat ends.
 *
 * @param creature The creature exiting combat
 */
void exitCombat(Creature& creature);

/**
 * @brief Start fleeing from a threat.
 *
 * Sets fleeing flag. Fleeing creatures prioritize escape over other behaviors.
 *
 * @param creature The creature starting to flee
 */
void startFleeing(Creature& creature);

/**
 * @brief Stop fleeing.
 *
 * Clears fleeing flag. Use when creature has escaped or threat is gone.
 *
 * @param creature The creature stopping fleeing
 */
void stopFleeing(Creature& creature);

/**
 * @brief Decrement combat cooldown if active.
 *
 * Should be called each tick for creatures in or recently exiting combat.
 *
 * @param creature The creature to update
 */
void updateCombatCooldown(Creature& creature);

//============================================================================
//  Health System (Combat-Related)
//============================================================================

/**
 * @brief Get current health value.
 * @param creature The creature to check
 * @return Current health
 */
float getHealth(const Creature& creature);

/**
 * @brief Get maximum health based on genetics.
 * @param creature The creature to check
 * @return Maximum health value
 */
float getMaxHealth(const Creature& creature);

/**
 * @brief Get health as percentage (0.0 to 1.0).
 * @param creature The creature to check
 * @return Health percentage
 */
float getHealthPercent(const Creature& creature);

/**
 * @brief Apply damage to creature.
 *
 * Reduces health, flooring at 0. Zero/negative amounts are no-ops.
 *
 * @param creature The creature taking damage
 * @param amount Damage amount (positive)
 */
void takeDamage(Creature& creature, float amount);

/**
 * @brief Heal creature.
 *
 * Increases health, capping at max health. Zero/negative amounts are no-ops.
 *
 * @param creature The creature being healed
 * @param amount Heal amount (positive)
 */
void heal(Creature& creature, float amount);

/**
 * @brief Get healing rate per tick.
 * @param creature The creature to check
 * @return Healing rate (health per tick)
 */
float getHealingRate(const Creature& creature);

} // namespace CreatureCombat

#endif // CREATURE_COMBAT_HPP
