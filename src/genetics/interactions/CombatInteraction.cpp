#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/interactions/CombatConfig.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include <sstream>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;
using namespace CombatConfig;

// ============================================================================
// Weapon Damage Calculation
// ============================================================================

DamageDistribution CombatInteraction::calculateWeaponDamage(
    const Phenotype& phenotype,
    WeaponType weapon
) {
    switch (weapon) {
        case WeaponType::Teeth:
            return calculateTeethDamage(phenotype);
        case WeaponType::Claws:
            return calculateClawsDamage(phenotype);
        case WeaponType::Horns:
            return calculateHornsDamage(phenotype);
        case WeaponType::Tail:
            return calculateTailDamage(phenotype);
        case WeaponType::Body:
        default:
            return calculateBodyDamage(phenotype);
    }
}

DefenseProfile CombatInteraction::getDefenseProfile(const Phenotype& phenotype) {
    DefenseProfile profile;
    profile.thickHide = getTraitSafe(phenotype, UniversalGenes::HIDE_THICKNESS, 0.0f);
    profile.scales = getTraitSafe(phenotype, UniversalGenes::SCALE_COVERAGE, 0.0f);
    profile.fatLayer = getTraitSafe(phenotype, UniversalGenes::FAT_LAYER_THICKNESS, 0.0f);
    return profile;
}

float CombatInteraction::getDefenseValue(const Phenotype& phenotype, DefenseType type) {
    switch (type) {
        case DefenseType::ThickHide:
            return getTraitSafe(phenotype, UniversalGenes::HIDE_THICKNESS, 0.0f);
        case DefenseType::Scales:
            return getTraitSafe(phenotype, UniversalGenes::SCALE_COVERAGE, 0.0f);
        case DefenseType::FatLayer:
            return getTraitSafe(phenotype, UniversalGenes::FAT_LAYER_THICKNESS, 0.0f);
        default:
            return 0.0f;
    }
}

// ============================================================================
// Damage Application
// ============================================================================

float CombatInteraction::applyDamageWithDefense(
    float rawDamage,
    CombatDamageType attackType,
    const Phenotype& defenderPhenotype
) {
    // Get the defense type that counters this attack type
    DefenseType counterDefense = getCounteringDefense(attackType);
    
    // Get the defender's strongest defense
    DefenseProfile defProfile = getDefenseProfile(defenderPhenotype);
    float defenseValue = defProfile.getDefenseForType(counterDefense);
    
    // Get type effectiveness
    float effectiveness = getTypeEffectiveness(attackType, counterDefense);
    
    // Apply defense reduction (up to 50% at max defense)
    float damageAfterDefense = applyDefense(rawDamage, defenseValue);
    
    // Apply type effectiveness
    return damageAfterDefense * effectiveness;
}

float CombatInteraction::applyDefense(float rawDamage, float defenseValue) {
    // Defense provides up to 50% reduction at max value
    float reduction = defenseValue * MAX_DEFENSE_REDUCTION;
    return rawDamage * (1.0f - reduction);
}

// ============================================================================
// Combat Behavior
// ============================================================================

bool CombatInteraction::shouldInitiateCombat(
    const Phenotype& attackerPhenotype,
    const Phenotype& targetPhenotype,
    float attackerHunger
) {
    float aggression = getTraitSafe(attackerPhenotype, UniversalGenes::COMBAT_AGGRESSION, 0.0f);
    float huntInstinct = getTraitSafe(attackerPhenotype, UniversalGenes::HUNT_INSTINCT, 0.0f);
    
    // More aggressive creatures attack more readily
    // Also consider hunger level - desperate creatures fight more
    float hungerMotive = 1.0f - attackerHunger;  // Low hunger (high value) = high motive
    
    // Formula: aggression × huntInstinct × (base + hungerMotive × scale)
    float combatScore = aggression * huntInstinct *
        (HUNGER_MOTIVATION_BASE + hungerMotive * HUNGER_MOTIVATION_SCALE);
    
    return combatScore > COMBAT_INITIATION_THRESHOLD;
}

bool CombatInteraction::shouldRetreat(const Phenotype& phenotype, float healthPercent) {
    float retreatThreshold = getTraitSafe(phenotype, UniversalGenes::RETREAT_THRESHOLD, 0.0f);
    
    // Creature retreats when health drops below its retreat threshold
    return healthPercent < retreatThreshold;
}

// ============================================================================
// Specialization System
// ============================================================================

float CombatInteraction::calculateSpecializationBonus(const DamageDistribution& damage) {
    float total = damage.total();
    if (total <= 0.0f) return 0.0f;
    
    // Find the maximum ratio (how specialized the creature is)
    float maxRatio = std::max({
        damage.piercing / total,
        damage.slashing / total,
        damage.blunt / total
    });
    
    // 0-50% bonus based on specialization
    // Pure specialist (100% in one type) gets 50% bonus
    // Perfect generalist (33% each) gets ~17% bonus
    return maxRatio * SPECIALIZATION_BONUS_MAX;
}

// ============================================================================
// Action Selection
// ============================================================================

CombatAction CombatInteraction::selectBestAction(
    const Phenotype& attackerPhenotype,
    const Phenotype& defenderPhenotype
) {
    // Get defender's defense profile to find weakness
    DefenseProfile defProfile = getDefenseProfile(defenderPhenotype);
    
    // Find the weakest defense
    DefenseType weakestDefense;
    float minDefense = DEFENSE_COMPARISON_INITIAL;  // Start higher than max possible
    
    if (defProfile.thickHide < minDefense) {
        minDefense = defProfile.thickHide;
        weakestDefense = DefenseType::ThickHide;
    }
    if (defProfile.scales < minDefense) {
        minDefense = defProfile.scales;
        weakestDefense = DefenseType::Scales;
    }
    if (defProfile.fatLayer < minDefense) {
        minDefense = defProfile.fatLayer;
        weakestDefense = DefenseType::FatLayer;
    }
    
    // Map weakest defense to best attack type
    CombatDamageType targetType;
    switch (weakestDefense) {
        case DefenseType::ThickHide: targetType = CombatDamageType::Blunt; break;  // Blunt beats Hide
        case DefenseType::Scales: targetType = CombatDamageType::Piercing; break;  // Pierce beats Scales
        case DefenseType::FatLayer: targetType = CombatDamageType::Slashing; break; // Slash beats Fat
        default: targetType = CombatDamageType::Blunt;
    }
    
    // Get available weapons
    std::vector<WeaponType> weapons = getAvailableWeapons(attackerPhenotype);
    
    // Find weapon that deals most damage of the target type
    WeaponType bestWeapon = WeaponType::Body;  // Fallback
    float bestDamage = 0.0f;
    
    for (WeaponType weapon : weapons) {
        DamageDistribution dist = calculateWeaponDamage(attackerPhenotype, weapon);
        float damage = dist.getDamageForType(targetType);
        
        if (damage > bestDamage) {
            bestDamage = damage;
            bestWeapon = weapon;
        }
    }
    
    // Create the action
    CombatAction action;
    action.weapon = bestWeapon;
    action.damage = calculateWeaponDamage(attackerPhenotype, bestWeapon);
    action.cooldownRemaining = 0;
    action.staminaCost = DEFAULT_STAMINA_COST;
    
    return action;
}

std::vector<WeaponType> CombatInteraction::getAvailableWeapons(const Phenotype& phenotype) {
    std::vector<WeaponType> weapons;
    
    // Body is always available
    weapons.push_back(WeaponType::Body);
    
    // Check if teeth are developed enough
    float teethSize = getTraitSafe(phenotype, UniversalGenes::TEETH_SIZE, 0.0f);
    if (teethSize >= WEAPON_USABILITY_THRESHOLD) {
        weapons.push_back(WeaponType::Teeth);
    }
    
    // Check if claws are developed
    float clawLength = getTraitSafe(phenotype, UniversalGenes::CLAW_LENGTH, 0.0f);
    if (clawLength >= WEAPON_USABILITY_THRESHOLD) {
        weapons.push_back(WeaponType::Claws);
    }
    
    // Check if horns are developed
    float hornLength = getTraitSafe(phenotype, UniversalGenes::HORN_LENGTH, 0.0f);
    if (hornLength >= WEAPON_USABILITY_THRESHOLD) {
        weapons.push_back(WeaponType::Horns);
    }
    
    // Check if tail is developed
    float tailLength = getTraitSafe(phenotype, UniversalGenes::TAIL_LENGTH, 0.0f);
    if (tailLength >= WEAPON_USABILITY_THRESHOLD) {
        weapons.push_back(WeaponType::Tail);
    }
    
    return weapons;
}

// ============================================================================
// Attack Resolution
// ============================================================================

AttackResult CombatInteraction::resolveAttack(
    const Phenotype& attackerPhenotype,
    const Phenotype& defenderPhenotype,
    const CombatAction& action,
    float attackerSizeRatio
) {
    AttackResult result;
    result.hit = true;  // For now, all attacks hit (could add evasion later)
    result.weaponUsed = action.weapon;
    
    // Get base damage from weapon stats
    const WeaponStats& baseStats = getWeaponStats(action.weapon);
    
    // Calculate damage distribution from shape genes (now normalized to sum to 1.0)
    DamageDistribution damage = action.damage;
    
    // Calculate specialization bonus
    float specBonus = calculateSpecializationBonus(damage);
    float specMultiplier = 1.0f + specBonus;
    
    // Get size gene based on weapon type for magnitude multiplier
    float sizeFactor = getSizeFactorForWeapon(attackerPhenotype, action.weapon);
    
    // Apply growth-based size scaling: juveniles deal less damage
    // Formula: 0.5 + 0.5 * sizeRatio gives range [0.5, 1.0]
    // - 10% grown juvenile deals 55% damage (0.5 + 0.5 * 0.1)
    // - 100% grown adult deals 100% damage (0.5 + 0.5 * 1.0)
    float growthMultiplier = 0.5f + 0.5f * std::clamp(attackerSizeRatio, 0.0f, 1.0f);
    
    // Get the dominant damage type
    result.primaryType = damage.getDominantType();
    
    // Calculate raw damage (total damage × base weapon damage × size × specialization × growth)
    // Note: damage.total() should now be ~1.0 (normalized distribution)
    result.rawDamage = damage.total() * baseStats.baseDamage * sizeFactor * specMultiplier * growthMultiplier;
    
    // Get type effectiveness against defender's defense
    DefenseType counterDefense = getCounteringDefense(result.primaryType);
    result.effectivenessMultiplier = getTypeEffectiveness(result.primaryType, counterDefense);
    
    // Apply defense and effectiveness
    result.finalDamage = applyDamageWithDefense(
        result.rawDamage,
        result.primaryType,
        defenderPhenotype
    );
    
    // Check for bleeding (pierce damage can cause bleeding)
    if (result.primaryType == CombatDamageType::Piercing &&
        damage.piercing > BLEEDING_DAMAGE_THRESHOLD) {
        // Bleeding resistance check
        float bleedResist = getTraitSafe(defenderPhenotype,
            UniversalGenes::BLEEDING_RESISTANCE, 0.0f);
        result.causedBleeding = (damage.piercing * (1.0f - bleedResist)) > BLEEDING_DAMAGE_THRESHOLD;
    }
    
    return result;
}

CombatResult CombatInteraction::resolveCombatTick(
    const Phenotype& attackerPhenotype,
    const Phenotype& defenderPhenotype,
    float attackerHealth,
    float defenderHealth
) {
    CombatResult result;
    
    // Check if attacker wants to retreat
    if (shouldRetreat(attackerPhenotype, attackerHealth)) {
        result.combatOccurred = false;
        result.attackerRetreated = true;
        return result;
    }
    
    // Check if defender wants to retreat
    if (shouldRetreat(defenderPhenotype, defenderHealth)) {
        result.combatOccurred = false;
        result.defenderRetreated = true;
        return result;
    }
    
    result.combatOccurred = true;
    
    // Attacker attacks
    CombatAction attackerAction = selectBestAction(attackerPhenotype, defenderPhenotype);
    result.attackerResult = resolveAttack(attackerPhenotype, defenderPhenotype, attackerAction);
    
    // Check if defender would die
    float newDefenderHealth = defenderHealth - (result.attackerResult.finalDamage / HEALTH_CONVERSION_DIVISOR);
    if (newDefenderHealth <= 0.0f) {
        result.defenderDied = true;
        return result;
    }
    
    // Defender counter-attacks (if combat-oriented)
    float defenderAggression = getTraitSafe(defenderPhenotype,
        UniversalGenes::COMBAT_AGGRESSION, 0.0f);
    
    if (defenderAggression > COUNTER_ATTACK_AGGRESSION_THRESHOLD) {
        CombatAction defenderAction = selectBestAction(defenderPhenotype, attackerPhenotype);
        result.defenderResult = resolveAttack(defenderPhenotype, attackerPhenotype, defenderAction);
        
        // Check if attacker would die
        float newAttackerHealth = attackerHealth - (result.defenderResult.finalDamage / HEALTH_CONVERSION_DIVISOR);
        if (newAttackerHealth <= 0.0f) {
            result.attackerDied = true;
        }
    }
    
    return result;
}

// ============================================================================
// Shape Gene to Damage Type Formulas
// ============================================================================

DamageDistribution CombatInteraction::calculateTeethDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float sharpness = getTraitSafe(phenotype, UniversalGenes::TEETH_SHARPNESS, 0.0f);
    float serration = getTraitSafe(phenotype, UniversalGenes::TEETH_SERRATION, 0.0f);
    // Note: TEETH_SIZE is now applied as magnitude multiplier in resolveAttack()
    
    // Calculate raw weights for each damage type
    float pierceWeight = sharpness;
    float slashWeight = serration * TEETH_SERRATION_SLASH_WEIGHT;
    float bluntWeight = 1.0f - sharpness;
    
    // Normalize so distribution sums to 1.0
    float totalWeight = pierceWeight + slashWeight + bluntWeight;
    if (totalWeight > 0.0f) {
        damage.piercing = pierceWeight / totalWeight;
        damage.slashing = slashWeight / totalWeight;
        damage.blunt = bluntWeight / totalWeight;
    }
    
    return damage;
}

DamageDistribution CombatInteraction::calculateClawsDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float curvature = getTraitSafe(phenotype, UniversalGenes::CLAW_CURVATURE, 0.0f);
    float sharpness = getTraitSafe(phenotype, UniversalGenes::CLAW_SHARPNESS, 0.0f);
    // Note: CLAW_LENGTH is now applied as magnitude multiplier in resolveAttack()
    
    // Calculate raw weights for each damage type
    float pierceWeight = curvature * sharpness;
    float slashWeight = (1.0f - curvature) * sharpness;
    float bluntWeight = (1.0f - sharpness) * CLAWS_BLUNT_WEIGHT;
    
    // Normalize so distribution sums to 1.0
    float totalWeight = pierceWeight + slashWeight + bluntWeight;
    if (totalWeight > 0.0f) {
        damage.piercing = pierceWeight / totalWeight;
        damage.slashing = slashWeight / totalWeight;
        damage.blunt = bluntWeight / totalWeight;
    }
    
    return damage;
}

DamageDistribution CombatInteraction::calculateHornsDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float pointiness = getTraitSafe(phenotype, UniversalGenes::HORN_POINTINESS, 0.0f);
    float spread = getTraitSafe(phenotype, UniversalGenes::HORN_SPREAD, 0.0f);
    // Note: HORN_LENGTH is now applied as magnitude multiplier in resolveAttack()
    
    // Calculate raw weights for each damage type
    float pierceWeight = pointiness;
    float slashWeight = spread * HORNS_SPREAD_SLASH_WEIGHT;
    float bluntWeight = 1.0f - pointiness;
    
    // Normalize so distribution sums to 1.0
    float totalWeight = pierceWeight + slashWeight + bluntWeight;
    if (totalWeight > 0.0f) {
        damage.piercing = pierceWeight / totalWeight;
        damage.slashing = slashWeight / totalWeight;
        damage.blunt = bluntWeight / totalWeight;
    }
    
    return damage;
}

DamageDistribution CombatInteraction::calculateTailDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float spines = getTraitSafe(phenotype, UniversalGenes::TAIL_SPINES, 0.0f);
    float mass = getTraitSafe(phenotype, UniversalGenes::TAIL_MASS, 0.0f);
    // Note: TAIL_LENGTH is now applied as magnitude multiplier in resolveAttack()
    
    // Calculate raw weights for each damage type
    float pierceWeight = spines;
    float slashWeight = (1.0f - mass) * TAIL_SLASH_WEIGHT;
    float bluntWeight = mass;
    
    // Normalize so distribution sums to 1.0
    float totalWeight = pierceWeight + slashWeight + bluntWeight;
    if (totalWeight > 0.0f) {
        damage.piercing = pierceWeight / totalWeight;
        damage.slashing = slashWeight / totalWeight;
        damage.blunt = bluntWeight / totalWeight;
    }
    
    return damage;
}

DamageDistribution CombatInteraction::calculateBodyDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float spines = getTraitSafe(phenotype, UniversalGenes::BODY_SPINES, 0.0f);
    // Note: MAX_SIZE is now applied as magnitude multiplier in resolveAttack()
    
    // Calculate raw weights for each damage type
    // Body is special: primarily blunt with optional spine piercing
    float pierceWeight = spines;
    float slashWeight = 0.0f;
    float bluntWeight = 1.0f;
    
    // Normalize so distribution sums to 1.0
    float totalWeight = pierceWeight + slashWeight + bluntWeight;
    if (totalWeight > 0.0f) {
        damage.piercing = pierceWeight / totalWeight;
        damage.slashing = slashWeight / totalWeight;
        damage.blunt = bluntWeight / totalWeight;
    }
    
    return damage;
}

// ============================================================================
// Helper Methods
// ============================================================================

DefenseType CombatInteraction::getCounteringDefense(CombatDamageType attackType) {
    switch (attackType) {
        case CombatDamageType::Piercing: return DefenseType::ThickHide;
        case CombatDamageType::Slashing: return DefenseType::Scales;
        case CombatDamageType::Blunt: return DefenseType::FatLayer;
        default: return DefenseType::ThickHide;
    }
}

float CombatInteraction::getSizeFactorForWeapon(const Phenotype& phenotype, WeaponType weapon) {
    switch (weapon) {
        case WeaponType::Teeth:
            return getTraitSafe(phenotype, UniversalGenes::TEETH_SIZE, 0.0f);
        case WeaponType::Claws:
            return getTraitSafe(phenotype, UniversalGenes::CLAW_LENGTH, 0.0f);
        case WeaponType::Horns:
            return getTraitSafe(phenotype, UniversalGenes::HORN_LENGTH, 0.0f);
        case WeaponType::Tail:
            return getTraitSafe(phenotype, UniversalGenes::TAIL_LENGTH, 0.0f);
        case WeaponType::Body:
        default:
            return getTraitSafe(phenotype, UniversalGenes::MAX_SIZE, 0.0f);
    }
}

} // namespace Genetics
} // namespace EcoSim
