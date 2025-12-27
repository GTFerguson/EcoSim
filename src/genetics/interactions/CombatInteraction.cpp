#include "genetics/interactions/CombatInteraction.hpp"
#include <sstream>

namespace EcoSim {
namespace Genetics {

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
    profile.thickHide = getTraitSafe(phenotype, UniversalGenes::HIDE_THICKNESS, 0.5f);
    profile.scales = getTraitSafe(phenotype, UniversalGenes::SCALE_COVERAGE, 0.3f);
    profile.fatLayer = getTraitSafe(phenotype, UniversalGenes::FAT_LAYER_THICKNESS, 0.4f);
    return profile;
}

float CombatInteraction::getDefenseValue(const Phenotype& phenotype, DefenseType type) {
    switch (type) {
        case DefenseType::ThickHide:
            return getTraitSafe(phenotype, UniversalGenes::HIDE_THICKNESS, 0.5f);
        case DefenseType::Scales:
            return getTraitSafe(phenotype, UniversalGenes::SCALE_COVERAGE, 0.3f);
        case DefenseType::FatLayer:
            return getTraitSafe(phenotype, UniversalGenes::FAT_LAYER_THICKNESS, 0.4f);
        default:
            return 0.0f;
    }
}

// ============================================================================
// Damage Application
// ============================================================================

float CombatInteraction::applyDamageWithDefense(
    float rawDamage,
    DamageType attackType,
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
    float aggression = getTraitSafe(attackerPhenotype, UniversalGenes::COMBAT_AGGRESSION, 0.5f);
    float huntInstinct = getTraitSafe(attackerPhenotype, UniversalGenes::HUNT_INSTINCT, 0.5f);
    
    // More aggressive creatures attack more readily
    // Also consider hunger level - desperate creatures fight more
    float hungerMotive = 1.0f - attackerHunger;  // Low hunger (high value) = high motive
    
    // Formula: aggression × huntInstinct × (0.5 + hungerMotive × 0.5)
    float combatScore = aggression * huntInstinct * (0.5f + hungerMotive * 0.5f);
    
    return combatScore > COMBAT_INITIATION_THRESHOLD;
}

bool CombatInteraction::shouldRetreat(const Phenotype& phenotype, float healthPercent) {
    float retreatThreshold = getTraitSafe(phenotype, UniversalGenes::RETREAT_THRESHOLD, 0.3f);
    
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
    // Perfect generalist (33% each) gets ~17% bonus (0.33 * 0.5)
    return maxRatio * 0.5f;
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
    float minDefense = 2.0f;  // Start higher than max possible
    
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
    DamageType targetType;
    switch (weakestDefense) {
        case DefenseType::ThickHide: targetType = DamageType::Blunt; break;  // Blunt beats Hide
        case DefenseType::Scales: targetType = DamageType::Piercing; break;  // Pierce beats Scales
        case DefenseType::FatLayer: targetType = DamageType::Slashing; break; // Slash beats Fat
        default: targetType = DamageType::Blunt;
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
    float teethSize = getTraitSafe(phenotype, UniversalGenes::TEETH_SIZE, 0.5f);
    if (teethSize >= WEAPON_USABILITY_THRESHOLD) {
        weapons.push_back(WeaponType::Teeth);
    }
    
    // Check if claws are developed
    float clawLength = getTraitSafe(phenotype, UniversalGenes::CLAW_LENGTH, 0.5f);
    if (clawLength >= WEAPON_USABILITY_THRESHOLD) {
        weapons.push_back(WeaponType::Claws);
    }
    
    // Check if horns are developed
    float hornLength = getTraitSafe(phenotype, UniversalGenes::HORN_LENGTH, 0.5f);
    if (hornLength >= WEAPON_USABILITY_THRESHOLD) {
        weapons.push_back(WeaponType::Horns);
    }
    
    // Check if tail is developed
    float tailLength = getTraitSafe(phenotype, UniversalGenes::TAIL_LENGTH, 0.5f);
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
    const CombatAction& action
) {
    AttackResult result;
    result.hit = true;  // For now, all attacks hit (could add evasion later)
    result.weaponUsed = action.weapon;
    
    // Get base damage from weapon stats
    const WeaponStats& baseStats = getWeaponStats(action.weapon);
    
    // Calculate damage distribution from shape genes
    DamageDistribution damage = action.damage;
    
    // Calculate specialization bonus
    float specBonus = calculateSpecializationBonus(damage);
    float specMultiplier = 1.0f + specBonus;
    
    // Get the dominant damage type
    result.primaryType = damage.getDominantType();
    
    // Calculate raw damage (total damage × base weapon damage × specialization)
    result.rawDamage = damage.total() * baseStats.baseDamage * specMultiplier;
    
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
    if (result.primaryType == DamageType::Piercing && 
        damage.piercing > BLEEDING_DAMAGE_THRESHOLD) {
        // Bleeding resistance check
        float bleedResist = getTraitSafe(defenderPhenotype, 
            UniversalGenes::BLEEDING_RESISTANCE, 0.5f);
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
    float newDefenderHealth = defenderHealth - (result.attackerResult.finalDamage / 100.0f);
    if (newDefenderHealth <= 0.0f) {
        result.defenderDied = true;
        return result;
    }
    
    // Defender counter-attacks (if combat-oriented)
    float defenderAggression = getTraitSafe(defenderPhenotype, 
        UniversalGenes::COMBAT_AGGRESSION, 0.5f);
    
    if (defenderAggression > 0.3f) {  // Only counter if aggressive enough
        CombatAction defenderAction = selectBestAction(defenderPhenotype, attackerPhenotype);
        result.defenderResult = resolveAttack(defenderPhenotype, attackerPhenotype, defenderAction);
        
        // Check if attacker would die
        float newAttackerHealth = attackerHealth - (result.defenderResult.finalDamage / 100.0f);
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
    
    float sharpness = getTraitSafe(phenotype, UniversalGenes::TEETH_SHARPNESS, 0.7f);
    float serration = getTraitSafe(phenotype, UniversalGenes::TEETH_SERRATION, 0.3f);
    float size = getTraitSafe(phenotype, UniversalGenes::TEETH_SIZE, 0.5f);
    
    // Formulas from design doc section 4.5:
    // pierce = TEETH_SHARPNESS × TEETH_SIZE
    // slash  = TEETH_SERRATION × TEETH_SIZE × 0.5
    // blunt  = (1.0 - TEETH_SHARPNESS) × TEETH_SIZE
    damage.piercing = sharpness * size;
    damage.slashing = serration * size * 0.5f;
    damage.blunt = (1.0f - sharpness) * size;
    
    return damage;
}

DamageDistribution CombatInteraction::calculateClawsDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float curvature = getTraitSafe(phenotype, UniversalGenes::CLAW_CURVATURE, 0.4f);
    float length = getTraitSafe(phenotype, UniversalGenes::CLAW_LENGTH, 0.5f);
    float sharpness = getTraitSafe(phenotype, UniversalGenes::CLAW_SHARPNESS, 0.6f);
    
    // Formulas from design doc section 4.5:
    // pierce = CLAW_CURVATURE × CLAW_LENGTH × CLAW_SHARPNESS
    // slash  = (1.0 - CLAW_CURVATURE) × CLAW_LENGTH × CLAW_SHARPNESS
    // blunt  = CLAW_LENGTH × (1.0 - CLAW_SHARPNESS) × 0.3
    damage.piercing = curvature * length * sharpness;
    damage.slashing = (1.0f - curvature) * length * sharpness;
    damage.blunt = length * (1.0f - sharpness) * 0.3f;
    
    return damage;
}

DamageDistribution CombatInteraction::calculateHornsDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float pointiness = getTraitSafe(phenotype, UniversalGenes::HORN_POINTINESS, 0.5f);
    float length = getTraitSafe(phenotype, UniversalGenes::HORN_LENGTH, 0.5f);
    float spread = getTraitSafe(phenotype, UniversalGenes::HORN_SPREAD, 0.3f);
    
    // Formulas from design doc section 4.5:
    // pierce = HORN_POINTINESS × HORN_LENGTH
    // slash  = HORN_SPREAD × HORN_LENGTH × 0.3 (sweep attacks)
    // blunt  = (1.0 - HORN_POINTINESS) × HORN_LENGTH
    damage.piercing = pointiness * length;
    damage.slashing = spread * length * 0.3f;
    damage.blunt = (1.0f - pointiness) * length;
    
    return damage;
}

DamageDistribution CombatInteraction::calculateTailDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float spines = getTraitSafe(phenotype, UniversalGenes::TAIL_SPINES, 0.1f);
    float length = getTraitSafe(phenotype, UniversalGenes::TAIL_LENGTH, 0.5f);
    float mass = getTraitSafe(phenotype, UniversalGenes::TAIL_MASS, 0.4f);
    
    // Formulas from design doc section 4.5:
    // pierce = TAIL_SPINES × TAIL_LENGTH
    // slash  = (1.0 - TAIL_MASS) × TAIL_LENGTH × 0.5 (whip)
    // blunt  = TAIL_MASS × TAIL_LENGTH
    damage.piercing = spines * length;
    damage.slashing = (1.0f - mass) * length * 0.5f;
    damage.blunt = mass * length;
    
    return damage;
}

DamageDistribution CombatInteraction::calculateBodyDamage(const Phenotype& phenotype) {
    DamageDistribution damage;
    
    float spines = getTraitSafe(phenotype, UniversalGenes::BODY_SPINES, 0.0f);
    float maxSize = getTraitSafe(phenotype, UniversalGenes::MAX_SIZE, 1.0f);
    
    // Formulas from design doc section 4.5:
    // pierce = BODY_SPINES × MAX_SIZE (counter-damage)
    // slash  = 0.0
    // blunt  = MAX_SIZE (body slam uses size)
    damage.piercing = spines * maxSize;
    damage.slashing = 0.0f;
    damage.blunt = maxSize;
    
    return damage;
}

// ============================================================================
// Helper Methods
// ============================================================================

float CombatInteraction::getTraitSafe(
    const Phenotype& phenotype,
    const char* traitName,
    float defaultValue
) {
    if (phenotype.hasTrait(traitName)) {
        return phenotype.getTrait(traitName);
    }
    return defaultValue;
}

DefenseType CombatInteraction::getCounteringDefense(DamageType attackType) {
    switch (attackType) {
        case DamageType::Piercing: return DefenseType::ThickHide;
        case DamageType::Slashing: return DefenseType::Scales;
        case DamageType::Blunt: return DefenseType::FatLayer;
        default: return DefenseType::ThickHide;
    }
}

} // namespace Genetics
} // namespace EcoSim
