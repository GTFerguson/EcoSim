/**
 * @file CreatureSerialization.cpp
 * @brief Serialization functionality extracted from Creature class
 *
 * This module handles string and JSON serialization/deserialization for creatures.
 * Part of Phase 1d creature decomposition to improve maintainability.
 */

#include "objects/creature/CreatureSerialization.hpp"
#include "objects/creature/creature.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"

#include <sstream>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

namespace CreatureSerialization {

//============================================================================
//  String Serialization
//============================================================================

std::string toString(const Creature& creature) {
    std::ostringstream ss;
    ss  << creature.GameObject::toString()  << ","
        << creature.tileX() << "," << creature.tileY() << "," << creature.getAge() << ","
        << directionToString(creature.getDirection()) << ","
        << profileToString(creature.getProfile()) << ","
        << creature.getHunger()     << ","
        << creature.getThirst()     << ","
        << creature.getFatigue()    << ","
        << creature.getMate()       << ","
        << creature.getMetabolism() << ","
        << creature.getSpeed();

    return ss.str();
}

std::string profileToString(Profile profile) {
    switch(profile) {
        case Profile::hungry:   return "hungry";
        case Profile::thirsty:  return "thirsty";
        case Profile::sleep:    return "sleep";
        case Profile::breed:    return "breed";
        case Profile::migrate:  return "migrate";
        default:                return "error";
    }
}

std::string directionToString(Direction direction) {
    switch (direction) {
        case Direction::SE:   return "SE";
        case Direction::NE:   return "NE";
        case Direction::E:    return "E";
        case Direction::SW:   return "SW";
        case Direction::NW:   return "NW";
        case Direction::W:    return "W";
        case Direction::S:    return "S";
        case Direction::N:    return "N";
        case Direction::none: return "none";
        default:              return "error";
    }
}

Profile stringToProfile(const std::string& str) {
    static const std::unordered_map<std::string, Profile> profileMap = {
        {"hungry",  Profile::hungry},
        {"thirsty", Profile::thirsty},
        {"sleep",   Profile::sleep},
        {"breed",   Profile::breed},
        {"migrate", Profile::migrate}
    };

    auto it = profileMap.find(str);
    return (it != profileMap.end()) ? it->second : Profile::migrate;
}

Direction stringToDirection(const std::string& str) {
    static const std::unordered_map<std::string, Direction> directionMap = {
        {"SE",   Direction::SE},
        {"NE",   Direction::NE},
        {"E",    Direction::E},
        {"SW",   Direction::SW},
        {"NW",   Direction::NW},
        {"W",    Direction::W},
        {"S",    Direction::S},
        {"N",    Direction::N},
        {"none", Direction::none}
    };

    auto it = directionMap.find(str);
    return (it != directionMap.end()) ? it->second : Direction::none;
}

//============================================================================
//  Enum Conversion Helpers
//============================================================================

std::string woundStateToString(WoundState state) {
    switch (state) {
        case WoundState::Healthy:  return "HEALTHY";
        case WoundState::Injured:  return "INJURED";
        case WoundState::Wounded:  return "WOUNDED";
        case WoundState::Critical: return "CRITICAL";
        case WoundState::Dead:     return "DEAD";
        default:                   return "HEALTHY";
    }
}

WoundState stringToWoundState(const std::string& str) {
    static const std::unordered_map<std::string, WoundState> map = {
        {"HEALTHY",  WoundState::Healthy},
        {"INJURED",  WoundState::Injured},
        {"WOUNDED",  WoundState::Wounded},
        {"CRITICAL", WoundState::Critical},
        {"DEAD",     WoundState::Dead}
    };
    auto it = map.find(str);
    return (it != map.end()) ? it->second : WoundState::Healthy;
}

std::string motivationToString(Motivation motivation) {
    switch (motivation) {
        case Motivation::Hungry:   return "HUNGRY";
        case Motivation::Thirsty:  return "THIRSTY";
        case Motivation::Amorous:  return "AMOROUS";
        case Motivation::Tired:    return "TIRED";
        case Motivation::Content:  return "CONTENT";
        default:                   return "CONTENT";
    }
}

Motivation stringToMotivation(const std::string& str) {
    static const std::unordered_map<std::string, Motivation> map = {
        {"HUNGRY",   Motivation::Hungry},
        {"THIRSTY",  Motivation::Thirsty},
        {"AMOROUS",  Motivation::Amorous},
        {"TIRED",    Motivation::Tired},
        {"CONTENT",  Motivation::Content}
    };
    auto it = map.find(str);
    return (it != map.end()) ? it->second : Motivation::Content;
}

std::string actionToString(Action action) {
    switch (action) {
        case Action::Idle:       return "IDLE";
        case Action::Wandering:  return "WANDERING";
        case Action::Searching:  return "SEARCHING";
        case Action::Navigating: return "NAVIGATING";
        case Action::Eating:     return "EATING";
        case Action::Grazing:    return "GRAZING";
        case Action::Hunting:    return "HUNTING";
        case Action::Chasing:    return "CHASING";
        case Action::Attacking:  return "ATTACKING";
        case Action::Fleeing:    return "FLEEING";
        case Action::Drinking:   return "DRINKING";
        case Action::Courting:   return "COURTING";
        case Action::Mating:     return "MATING";
        case Action::Resting:    return "RESTING";
        default:                 return "IDLE";
    }
}

Action stringToAction(const std::string& str) {
    static const std::unordered_map<std::string, Action> map = {
        {"IDLE",       Action::Idle},
        {"WANDERING",  Action::Wandering},
        {"SEARCHING",  Action::Searching},
        {"NAVIGATING", Action::Navigating},
        {"EATING",     Action::Eating},
        {"GRAZING",    Action::Grazing},
        {"HUNTING",    Action::Hunting},
        {"CHASING",    Action::Chasing},
        {"ATTACKING",  Action::Attacking},
        {"FLEEING",    Action::Fleeing},
        {"DRINKING",   Action::Drinking},
        {"COURTING",   Action::Courting},
        {"MATING",     Action::Mating},
        {"RESTING",    Action::Resting}
    };
    auto it = map.find(str);
    return (it != map.end()) ? it->second : Action::Idle;
}

//============================================================================
//  JSON Serialization
//============================================================================

nlohmann::json toJson(const Creature& creature) {
    nlohmann::json j;
    
    // Identity
    j["id"] = creature.getId();
    j["creatureId"] = creature.getCreatureId();  // Creature-specific sequential ID
    j["archetypeLabel"] = creature.getArchetypeLabel();
    j["scientificName"] = creature.getScientificName();
    
    // State (0-10 scales)
    j["state"] = {
        {"hunger", creature.getHunger()},
        {"thirst", creature.getThirst()},
        {"fatigue", creature.getFatigue()},
        {"mate", creature.getMate()},
        {"age", creature.getAge()},
        {"lifespan", creature.getLifespan()}
    };
    
    // Position
    j["position"] = {
        {"worldX", creature.getWorldX()},
        {"worldY", creature.getWorldY()},
        {"tileX", creature.tileX()},
        {"tileY", creature.tileY()}
    };
    
    // Health system
    j["health"] = {
        {"current", creature.getHealth()},
        {"max", creature.getMaxHealth()},
        {"woundSeverity", creature.getWoundSeverity()},
        {"woundState", woundStateToString(creature.getWoundState())}
    };
    
    // Growth state
    j["growth"] = {
        {"currentSize", creature.getCurrentSize()},
        {"maxSize", creature.getMaxSize()},
        {"mature", creature.isMature()}
    };
    
    // Combat state
    j["combat"] = {
        {"targetId", creature.getTargetId()},
        {"inCombat", creature.isInCombat()},
        {"isFleeing", creature.isFleeing()},
        {"cooldown", creature.getCombatCooldown()}
    };
    
    // Behavior
    j["behavior"] = {
        {"motivation", motivationToString(creature.getMotivation())},
        {"action", actionToString(creature.getAction())}
    };
    
    // Genome (uses Genome::toJson())
    j["genome"] = creature.getGenome().toJson();
    
    return j;
}

Creature fromJson(const nlohmann::json& j, int mapWidth, int mapHeight) {
    // Validate required fields
    if (!j.contains("genome")) {
        throw std::runtime_error("Creature::fromJson: missing required field 'genome'");
    }
    
    // Load genome first
    auto genome = std::make_unique<EcoSim::Genetics::Genome>(
        EcoSim::Genetics::Genome::fromJson(j.at("genome"))
    );
    
    // Get position with bounds checking
    float worldX = 0.0f, worldY = 0.0f;
    if (j.contains("position")) {
        const auto& pos = j.at("position");
        worldX = pos.value("worldX", 0.0f);
        worldY = pos.value("worldY", 0.0f);
        
        // Clamp to world bounds
        worldX = std::max(0.0f, std::min(worldX, static_cast<float>(mapWidth - 1)));
        worldY = std::max(0.0f, std::min(worldY, static_cast<float>(mapHeight - 1)));
    }
    
    // Get initial state values - use RESOURCE_LIMIT from Creature class
    float hunger = Creature::RESOURCE_LIMIT / 2.0f;  // Default to half
    float thirst = Creature::RESOURCE_LIMIT / 2.0f;
    if (j.contains("state")) {
        const auto& state = j.at("state");
        hunger = state.value("hunger", hunger);
        thirst = state.value("thirst", thirst);
    }
    
    // Create creature with genome and position
    Creature creature(
        static_cast<int>(worldX),
        static_cast<int>(worldY),
        hunger,
        thirst,
        std::move(genome)
    );
    
    // Restore precise world position
    creature.setWorldPosition(worldX, worldY);
    
    // Restore state
    if (j.contains("state")) {
        const auto& state = j.at("state");
        creature.setFatigue(state.value("fatigue", 0.0f));
        creature.setMate(state.value("mate", 0.0f));
        creature.setAge(state.value("age", 0u));
    }
    
    // Restore health - need direct member access, use takeDamage/heal approach
    if (j.contains("health")) {
        const auto& health = j.at("health");
        float targetHealth = health.value("current", creature.getMaxHealth());
        // Heal or damage to reach target health
        float delta = targetHealth - creature.getHealth();
        if (delta > 0) {
            creature.heal(delta);
        } else if (delta < 0) {
            creature.takeDamage(-delta);
        }
    }
    
    // Restore combat state
    if (j.contains("combat")) {
        const auto& combat = j.at("combat");
        creature.setTargetId(combat.value("targetId", -1));
        creature.setInCombat(combat.value("inCombat", false));
        creature.setFleeing(combat.value("isFleeing", false));
        creature.setCombatCooldown(combat.value("cooldown", 0));
    }
    
    // Restore behavior state
    if (j.contains("behavior")) {
        const auto& behavior = j.at("behavior");
        if (behavior.contains("motivation")) {
            creature.setMotivation(stringToMotivation(behavior.at("motivation").get<std::string>()));
        }
        if (behavior.contains("action")) {
            creature.setAction(stringToAction(behavior.at("action").get<std::string>()));
        }
    }
    
    // Restore growth state (with backward compatibility defaults)
    if (j.contains("growth")) {
        const auto& growth = j.at("growth");
        // Must set maxSize first so setCurrentSize can compute mature_ correctly
        creature.setMaxSize(growth.value("maxSize", creature.getMaxSize()));
        creature.setCurrentSize(growth.value("currentSize", 0.1f));
        // Override mature if explicitly set (backward compatibility)
        if (growth.contains("mature")) {
            creature.setMature(growth.at("mature").get<bool>());
        }
    }
    
    // Restore the saved creature ID (overwrite the auto-generated one)
    if (j.contains("creatureId")) {
        creature.setCreatureId(j.at("creatureId").get<int>());
    }
    
    // Archetype is reclassified from genome in constructor
    // Phenotype is regenerated from genome in constructor
    
    return creature;
}

} // namespace CreatureSerialization
