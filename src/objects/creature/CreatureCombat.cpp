/**
 * @file CreatureCombat.cpp
 * @brief Combat system functionality extracted from Creature class
 *
 * This module handles combat hunting, prey finding, and combat state management.
 * Part of Phase 1a creature decomposition to improve maintainability.
 */

#include "objects/creature/CreatureCombat.hpp"
#include "objects/creature/creature.hpp"
#include "objects/creature/navigator.hpp"
#include "world/tile.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "logging/Logger.hpp"

#include <cmath>
#include <algorithm>

namespace CreatureCombat {

//============================================================================
//  Combat Hunting
//============================================================================

bool findPrey(Creature& creature,
              std::vector<std::vector<Tile>>& map,
              const int& rows,
              const int& cols,
              std::vector<Creature>& creatures,
              unsigned& preyAte) {
    using namespace EcoSim::Genetics;
    
    // Constants from Creature class
    constexpr float PREY_CALORIES = 3.0f;
    constexpr float RESOURCE_LIMIT = 10.0f;
    
    float closestDistance = creature.getSightRange();
    std::vector<Creature>::iterator closestPrey = creatures.begin();
    bool foundPrey = false;
    
    for (auto it = creatures.begin(); it != creatures.end(); ++it) {
        // Exclude predators (carnivores and necrovores) - they're not prey
        DietType creatureDiet = it->getDietType();
        if (creatureDiet != DietType::CARNIVORE && creatureDiet != DietType::NECROVORE) {
            // Calculate the distance between the creatures
            float distance = creature.calculateDistance(it->getX(), it->getY());
            
            if (distance < closestDistance) {
                closestPrey = it;
                closestDistance = distance;
                foundPrey = true;
            }
        }
    }
    
    // If prey is found seek it
    if (foundPrey && closestDistance < creature.getSightRange()) {
        int preyX = closestPrey->getX();
        int preyY = closestPrey->getY();
        
        // Use tile positions for grid-based distance
        unsigned xDist = abs(creature.tileX() - preyX);
        unsigned yDist = abs(creature.tileY() - preyY);
        
        // If adjacent to prey, engage in combat
        if (xDist <= 1 && yDist <= 1) {
            // Set combat state
            enterCombat(creature, closestPrey->getId());
            
            // Check cooldown before attacking
            if (getCombatCooldown(creature) <= 0) {
                creature.setAction(Action::Attacking);
                
                // Use CombatInteraction to resolve attack
                const Phenotype& attackerPhenotype = creature.getPhenotype();
                const Phenotype& preyPhenotype = closestPrey->getPhenotype();
                
                // Select best attack and resolve it
                CombatAction action = CombatInteraction::selectBestAction(
                    attackerPhenotype, preyPhenotype);
                
                // Pass attacker's size ratio for growth-based damage scaling
                AttackResult result = CombatInteraction::resolveAttack(
                    attackerPhenotype, preyPhenotype, action, creature.getSizeRatio());
                
                // Capture health BEFORE applying damage for accurate logging
                float defenderHealthBefore = getHealth(*closestPrey);
                
                // Apply damage to prey
                takeDamage(*closestPrey, result.finalDamage);
                
                // Log combat attack using detailed combat event
                logging::CombatLogEvent logEvent;
                logEvent.attackerId = creature.getId();
                logEvent.defenderId = closestPrey->getId();
                logEvent.attackerName = creature.generateName();
                logEvent.defenderName = closestPrey->generateName();
                logEvent.weapon = action.weapon;
                logEvent.primaryDamageType = result.primaryType;
                logEvent.rawDamage = result.rawDamage;
                logEvent.finalDamage = result.finalDamage;
                logEvent.effectivenessMultiplier = result.effectivenessMultiplier;
                logEvent.defenseUsed = CombatInteraction::getCounteringDefense(result.primaryType);
                logEvent.defenseValue = CombatInteraction::getDefenseProfile(preyPhenotype)
                                        .getDefenseForType(logEvent.defenseUsed);
                logEvent.attackerHealthBefore = getHealth(creature);
                logEvent.attackerHealthAfter = getHealth(creature);
                logEvent.attackerMaxHealth = getMaxHealth(creature);
                logEvent.defenderHealthBefore = defenderHealthBefore;
                logEvent.defenderHealthAfter = getHealth(*closestPrey);
                logEvent.defenderMaxHealth = getMaxHealth(*closestPrey);
                logEvent.hit = result.hit;
                logEvent.causedBleeding = result.causedBleeding;
                logEvent.defenderDied = (getHealth(*closestPrey) <= 0.0f);
                
                logging::Logger::getInstance().combatEvent(logEvent);
                
                // Set cooldown based on weapon type
                creature.setCombatCooldown(getWeaponStats(action.weapon).baseCooldown);
                
                // Check if prey died
                if (getHealth(*closestPrey) <= 0.0f) {
                    // Prey killed - gain calories
                    float calories = PREY_CALORIES + closestPrey->getHunger();
                    float newHunger = creature.getHunger() + calories;
                    if (newHunger > RESOURCE_LIMIT) newHunger = RESOURCE_LIMIT;
                    creature.setHunger(newHunger);
                    
                    // Log the creature death event
                    logging::Logger::getInstance().creatureDied(
                        closestPrey->getId(),
                        closestPrey->generateName(),
                        "combat",
                        closestPrey->getHunger(),
                        static_cast<int>(closestPrey->getAge())
                    );
                    
                    // Log the feeding event (carnivore eating prey)
                    logging::Logger::getInstance().feeding(
                        creature.getId(),
                        closestPrey->getId(),
                        true,
                        calories,
                        0.0f
                    );
                    
                    preyAte++;
                    
                    // Clear combat state
                    exitCombat(creature);
                }
                // Else: prey survives, combat continues next tick
            } else {
                // On cooldown - decrement and wait
                updateCombatCooldown(creature);
                creature.setAction(Action::Attacking);  // Still in combat stance
            }
            
            return true;
            
        // Move towards prey
        } else {
            creature.setAction(Action::Chasing);
            return Navigator::astarSearch(creature, map, rows, cols, preyX, preyY);
        }
    }
    
    return false;
}

//============================================================================
//  Combat State Management
//============================================================================

bool isInCombat(const Creature& creature) {
    return creature.isInCombat();
}

bool isFleeing(const Creature& creature) {
    return creature.isFleeing();
}

int getTargetId(const Creature& creature) {
    return creature.getTargetId();
}

int getCombatCooldown(const Creature& creature) {
    return creature.getCombatCooldown();
}

void enterCombat(Creature& creature, int targetId) {
    creature.setInCombat(true);
    creature.setTargetId(targetId);
}

void exitCombat(Creature& creature) {
    creature.setInCombat(false);
    creature.setTargetId(-1);
}

void startFleeing(Creature& creature) {
    creature.setFleeing(true);
}

void stopFleeing(Creature& creature) {
    creature.setFleeing(false);
}

void updateCombatCooldown(Creature& creature) {
    int cooldown = creature.getCombatCooldown();
    if (cooldown > 0) {
        creature.setCombatCooldown(cooldown - 1);
    }
}

//============================================================================
//  Health System (Combat-Related)
//============================================================================

float getHealth(const Creature& creature) {
    return creature.getHealth();
}

float getMaxHealth(const Creature& creature) {
    return creature.getMaxHealth();
}

float getHealthPercent(const Creature& creature) {
    return creature.getHealthPercent();
}

void takeDamage(Creature& creature, float amount) {
    creature.takeDamage(amount);
}

void heal(Creature& creature, float amount) {
    creature.heal(amount);
}

float getHealingRate(const Creature& creature) {
    return creature.getHealingRate();
}

} // namespace CreatureCombat
