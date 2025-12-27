#pragma once
#include "DamageTypes.hpp"
#include <string>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Damage distribution across types (derived from shape genes)
 * 
 * Each weapon's damage is split across three types based on the creature's
 * shape genes. Specialization in one type provides bonus damage, while
 * generalists have more flexibility but less raw power.
 */
struct DamageDistribution {
    float piercing = 0.0f;  ///< Puncture damage component
    float slashing = 0.0f;  ///< Cutting damage component
    float blunt = 0.0f;     ///< Impact damage component
    
    /**
     * @brief Get total damage across all types
     * @return Sum of all damage components
     */
    float total() const { return piercing + slashing + blunt; }
    
    /**
     * @brief Get specialization score (0-1, higher = more focused)
     * 
     * Measures how concentrated damage is in one type.
     * Returns the ratio of the highest component to total damage.
     * 
     * @return Specialization score from 0.0 (even split) to 1.0 (pure specialist)
     */
    float getSpecialization() const {
        float t = total();
        if (t <= 0.0f) return 0.0f;
        
        float maxComponent = std::max({piercing, slashing, blunt});
        return maxComponent / t;
    }
    
    /**
     * @brief Get the dominant damage type
     * @return DamageType with highest damage value
     */
    DamageType getDominantType() const {
        if (piercing >= slashing && piercing >= blunt) {
            return DamageType::Piercing;
        }
        if (slashing >= blunt) {
            return DamageType::Slashing;
        }
        return DamageType::Blunt;
    }
    
    /**
     * @brief Get damage value for a specific type
     * @param type The damage type to query
     * @return Damage value for that type
     */
    float getDamageForType(DamageType type) const {
        switch (type) {
            case DamageType::Piercing: return piercing;
            case DamageType::Slashing: return slashing;
            case DamageType::Blunt: return blunt;
            default: return 0.0f;
        }
    }
};

/**
 * @brief A single combat action (attack)
 * 
 * Represents one weapon attack with its current state (cooldown)
 * and calculated damage distribution based on shape genes.
 */
struct CombatAction {
    WeaponType weapon;              ///< Type of weapon being used
    DamageDistribution damage;      ///< Damage split across types
    int cooldownRemaining;          ///< Ticks until weapon ready (0 = ready)
    float staminaCost;              ///< Energy cost to use this attack
    
    /**
     * @brief Check if the action is ready to use
     * @return True if cooldown is complete
     */
    bool isReady() const { return cooldownRemaining <= 0; }
    
    /**
     * @brief Advance cooldown by one tick
     */
    void tickCooldown() { 
        if (cooldownRemaining > 0) {
            --cooldownRemaining;
        }
    }
    
    /**
     * @brief Reset cooldown to base value for weapon type
     */
    void startCooldown() {
        cooldownRemaining = getWeaponStats(weapon).baseCooldown;
    }
    
    /**
     * @brief Get the weapon's name for display
     * @return Human-readable weapon name
     */
    const char* getName() const {
        return weaponTypeToString(weapon);
    }
};

/**
 * @brief Result of a single attack
 * 
 * Contains detailed information about what happened when an attack landed,
 * including damage before and after defense, type effectiveness, and effects.
 */
struct AttackResult {
    bool hit = true;                              ///< Did the attack connect?
    float rawDamage = 0.0f;                       ///< Damage before defense
    float finalDamage = 0.0f;                     ///< Damage after defense reduction
    DamageType primaryType = DamageType::Blunt;  ///< Main damage type dealt
    bool causedBleeding = false;                  ///< Did attack cause bleeding?
    float effectivenessMultiplier = 1.0f;        ///< Type effectiveness applied
    WeaponType weaponUsed = WeaponType::Body;    ///< Which weapon was used
    
    /**
     * @brief Build a description string for the attack
     * @return Human-readable attack description
     */
    std::string describe() const {
        if (!hit) {
            return "Attack missed";
        }
        
        std::string desc = weaponTypeToString(weaponUsed);
        desc += " attack dealt ";
        desc += std::to_string(static_cast<int>(finalDamage));
        desc += " ";
        desc += damageTypeToString(primaryType);
        desc += " damage";
        
        if (effectivenessMultiplier > 1.0f) {
            desc += " (super effective!)";
        } else if (effectivenessMultiplier < 1.0f) {
            desc += " (not very effective)";
        }
        
        if (causedBleeding) {
            desc += " [BLEEDING]";
        }
        
        return desc;
    }
};

/**
 * @brief Defense profile for a creature
 * 
 * Captures a creature's defensive capabilities across all defense types.
 */
struct DefenseProfile {
    float thickHide = 0.0f;     ///< HIDE_THICKNESS gene value (reduces pierce)
    float scales = 0.0f;        ///< SCALE_COVERAGE gene value (reduces slash)
    float fatLayer = 0.0f;      ///< FAT_LAYER_THICKNESS gene value (reduces blunt)
    
    /**
     * @brief Get defense value for a specific type
     * @param type The defense type to query
     * @return Defense value (0.0 to 1.0)
     */
    float getDefenseForType(DefenseType type) const {
        switch (type) {
            case DefenseType::ThickHide: return thickHide;
            case DefenseType::Scales: return scales;
            case DefenseType::FatLayer: return fatLayer;
            default: return 0.0f;
        }
    }
    
    /**
     * @brief Get the strongest defense type
     * @return DefenseType with highest value
     */
    DefenseType getStrongestDefense() const {
        if (thickHide >= scales && thickHide >= fatLayer) {
            return DefenseType::ThickHide;
        }
        if (scales >= fatLayer) {
            return DefenseType::Scales;
        }
        return DefenseType::FatLayer;
    }
    
    /**
     * @brief Calculate total defense investment
     * @return Sum of all defense values
     */
    float totalInvestment() const {
        return thickHide + scales + fatLayer;
    }
};

} // namespace Genetics
} // namespace EcoSim
