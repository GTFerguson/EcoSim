#include "genetics/behaviors/HuntingBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/Genome.hpp"
#include <cmath>
#include <sstream>
#include <functional>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

HuntingBehavior::HuntingBehavior(CombatInteraction& combat, PerceptionSystem& perception)
    : combat_(combat)
    , perception_(perception)
{
}

std::string HuntingBehavior::getId() const {
    return "hunting";
}

bool HuntingBehavior::isApplicable(const Organism& organism,
                                    const BehaviorContext& ctx) const {
    if (!canHunt(organism)) {
        return false;
    }
    
    if (!canChase(organism)) {
        return false;
    }
    
    if (isSatiated(organism, ctx)) {
        return false;
    }
    
    unsigned int organismId = getOrganismId(organism);
    if (isOnCooldown(organismId, ctx.currentTick)) {
        return false;
    }
    
    return true;
}

float HuntingBehavior::getPriority(const Organism& organism) const {
    float hunger = getHungerLevel(organism);
    float threshold = getHungerThreshold(organism);
    
    float priority = BASE_PRIORITY;
    
    if (threshold > 0.0f) {
        float hungerRatio = hunger / threshold;
        float hungerUrgency = 1.0f - hungerRatio;
        hungerUrgency = std::max(0.0f, std::min(1.0f, hungerUrgency));
        
        priority += hungerUrgency * MAX_PRIORITY_BOOST;
    }
    
    return priority;
}

BehaviorResult HuntingBehavior::execute(Organism& organism,
                                         BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = true;
    result.completed = false;
    result.energyCost = HUNT_COST;
    
    unsigned int organismId = getOrganismId(organism);
    
    if (!ctx.world) {
        result.debugInfo = "No world access - hunt attempt recorded but no prey search";
        recordHunt(organismId, ctx.currentTick);
        return result;
    }
    
    Organism* prey = findPrey(organism, ctx);
    
    if (!prey) {
        result.debugInfo = "No prey found in range";
        recordHunt(organismId, ctx.currentTick);
        return result;
    }
    
    if (attemptEscape(organism, *prey)) {
        result.completed = false;
        
        float escapeChance = calculateEscapeChance(organism, *prey);
        std::ostringstream ss;
        ss << "Prey escaped (chance: " << (escapeChance * 100.0f) << "%)";
        result.debugInfo = ss.str();
        
        recordHunt(organismId, ctx.currentTick);
        return result;
    }
    
    const Phenotype& hunterPhenotype = organism.getPhenotype();
    const Phenotype& preyPhenotype = prey->getPhenotype();
    
    CombatAction action = CombatInteraction::selectBestAction(hunterPhenotype, preyPhenotype);
    AttackResult attackResult = CombatInteraction::resolveAttack(hunterPhenotype, preyPhenotype, action);
    
    result.completed = attackResult.hit;
    
    std::ostringstream ss;
    ss << "Hunt attack: " << attackResult.describe();
    if (attackResult.hit) {
        ss << " (damage: " << attackResult.finalDamage << ")";
    }
    result.debugInfo = ss.str();
    
    recordHunt(organismId, ctx.currentTick);
    
    cleanupStaleEntries(ctx.currentTick);
    
    return result;
}

float HuntingBehavior::getEnergyCost(const Organism& organism) const {
    return HUNT_COST;
}

bool HuntingBehavior::canHunt(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    float huntInstinct = getTraitSafe(phenotype, UniversalGenes::HUNT_INSTINCT, 0.0f);
    
    return huntInstinct > HUNT_INSTINCT_THRESHOLD;
}

bool HuntingBehavior::canChase(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    float locomotion = getTraitSafe(phenotype, UniversalGenes::LOCOMOTION, 0.0f);
    
    return locomotion > LOCOMOTION_THRESHOLD;
}

bool HuntingBehavior::isSatiated(const Organism& organism, const BehaviorContext& ctx) const {
    // Check organism state from context (primary source of truth)
    if (ctx.organismState) {
        // energy_level ranges 0.0 (starving) to 1.0 (full)
        // If energy_level >= SATIATION_THRESHOLD (0.8), organism is too full to hunt
        return ctx.organismState->energy_level >= SATIATION_THRESHOLD;
    }
    
    // Fallback: assume not satiated if no state available
    // This allows behaviors to work even without state being set
    return false;
}

bool HuntingBehavior::isOnCooldown(unsigned int organismId, unsigned int currentTick) const {
    auto it = lastHuntTick_.find(organismId);
    if (it == lastHuntTick_.end()) {
        return false;
    }
    
    unsigned int ticksSinceLastHunt = currentTick - it->second;
    return ticksSinceLastHunt < HUNT_COOLDOWN;
}

float HuntingBehavior::calculateEscapeChance(const Organism& predator,
                                              const Organism& prey) const {
    const Phenotype& preyPhenotype = prey.getPhenotype();
    const Phenotype& predatorPhenotype = predator.getPhenotype();
    
    float preyFlee = getTraitSafe(preyPhenotype, UniversalGenes::FLEE_THRESHOLD, 0.0f);
    float predatorPursue = getTraitSafe(predatorPhenotype, UniversalGenes::PURSUE_THRESHOLD, 0.0f);
    
    float denominator = preyFlee + predatorPursue + BASE_ESCAPE_DENOMINATOR;
    if (denominator <= 0.0f) {
        return 0.5f;
    }
    
    float escapeChance = preyFlee / denominator;
    return std::max(0.0f, std::min(1.0f, escapeChance));
}

bool HuntingBehavior::attemptEscape(const Organism& predator,
                                     const Organism& prey) const {
    float escapeChance = calculateEscapeChance(predator, prey);
    
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng_);
    
    return roll < escapeChance;
}

Organism* HuntingBehavior::findPrey(const Organism& hunter,
                                            const BehaviorContext& ctx) const {
    return nullptr;
}

void HuntingBehavior::recordHunt(unsigned int organismId, unsigned int tick) {
    lastHuntTick_[organismId] = tick;
}

void HuntingBehavior::cleanupStaleEntries(unsigned int currentTick) {
    static constexpr unsigned int CLEANUP_THRESHOLD = HUNT_COOLDOWN * 10;
    
    auto it = lastHuntTick_.begin();
    while (it != lastHuntTick_.end()) {
        if (currentTick > it->second && (currentTick - it->second) > CLEANUP_THRESHOLD) {
            it = lastHuntTick_.erase(it);
        } else {
            ++it;
        }
    }
}

float HuntingBehavior::getHungerLevel(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    float maxHunger = getTraitSafe(phenotype, UniversalGenes::MAX_SIZE, 0.0f);
    
    return maxHunger * 0.3f;
}

float HuntingBehavior::getHungerThreshold(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    if (phenotype.hasTrait(UniversalGenes::HUNGER_THRESHOLD)) {
        return phenotype.getTrait(UniversalGenes::HUNGER_THRESHOLD);
    }
    
    return DEFAULT_HUNGER_THRESHOLD;
}

unsigned int HuntingBehavior::getOrganismId(const Organism& organism) const {
    const Genome& genome = organism.getGenome();
    return static_cast<unsigned int>(std::hash<const Genome*>{}(&genome));
}

} // namespace Genetics
} // namespace EcoSim
