#pragma once
#include "DamageTypes.hpp"
#include "CombatAction.hpp"
#include "CombatConfig.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Combat state tracking for a creature
 * 
 * Tracks whether a creature is in combat and manages weapon cooldowns.
 */
struct CombatState {
    bool inCombat = false;              ///< Currently engaged in combat
    int ticksSinceCombatStart = 0;      ///< Duration of current combat
    std::vector<CombatAction> availableActions;  ///< Actions this creature can perform
    
    // Individual weapon cooldowns
    int teethCooldown = 0;
    int clawsCooldown = 0;
    int hornsCooldown = 0;
    int tailCooldown = 0;
    int bodyCooldown = 0;
    
    /**
     * @brief Tick all cooldowns by one
     */
    void tickAllCooldowns() {
        if (teethCooldown > 0) --teethCooldown;
        if (clawsCooldown > 0) --clawsCooldown;
        if (hornsCooldown > 0) --hornsCooldown;
        if (tailCooldown > 0) --tailCooldown;
        if (bodyCooldown > 0) --bodyCooldown;
        ++ticksSinceCombatStart;
    }
    
    /**
     * @brief Check if a weapon is ready to use
     * @param weapon The weapon type to check
     * @return True if cooldown is complete
     */
    bool isWeaponReady(WeaponType weapon) const {
        switch (weapon) {
            case WeaponType::Teeth: return teethCooldown <= 0;
            case WeaponType::Claws: return clawsCooldown <= 0;
            case WeaponType::Horns: return hornsCooldown <= 0;
            case WeaponType::Tail: return tailCooldown <= 0;
            case WeaponType::Body: return bodyCooldown <= 0;
            default: return false;
        }
    }
    
    /**
     * @brief Start cooldown for a weapon
     * @param weapon The weapon that was used
     */
    void startCooldown(WeaponType weapon) {
        int cooldown = getWeaponStats(weapon).baseCooldown;
        switch (weapon) {
            case WeaponType::Teeth: teethCooldown = cooldown; break;
            case WeaponType::Claws: clawsCooldown = cooldown; break;
            case WeaponType::Horns: hornsCooldown = cooldown; break;
            case WeaponType::Tail: tailCooldown = cooldown; break;
            case WeaponType::Body: bodyCooldown = cooldown; break;
        }
    }
    
    /**
     * @brief Reset combat state
     */
    void reset() {
        inCombat = false;
        ticksSinceCombatStart = 0;
        teethCooldown = 0;
        clawsCooldown = 0;
        hornsCooldown = 0;
        tailCooldown = 0;
        bodyCooldown = 0;
        availableActions.clear();
    }
};

/**
 * @brief Result of combat resolution
 * 
 * Contains the outcome of a combat encounter including damage dealt,
 * retreats, and deaths.
 */
struct CombatResult {
    bool combatOccurred = false;        ///< Did combat actually happen?
    AttackResult attackerResult;        ///< Result of attacker's attack
    AttackResult defenderResult;        ///< Result of defender's counter-attack (if any)
    bool attackerRetreated = false;     ///< Did attacker flee?
    bool defenderRetreated = false;     ///< Did defender flee?
    bool attackerDied = false;          ///< Is attacker dead?
    bool defenderDied = false;          ///< Is defender dead?
    
    /**
     * @brief Build a description of the combat outcome
     * @return Human-readable combat summary
     */
    std::string describe() const {
        if (!combatOccurred) {
            return "No combat occurred";
        }
        
        std::string desc;
        desc += "Attacker: " + attackerResult.describe();
        
        if (defenderResult.hit) {
            desc += " | Defender: " + defenderResult.describe();
        }
        
        if (attackerDied) desc += " [ATTACKER DIED]";
        if (defenderDied) desc += " [DEFENDER DIED]";
        if (attackerRetreated) desc += " [ATTACKER FLED]";
        if (defenderRetreated) desc += " [DEFENDER FLED]";
        
        return desc;
    }
};

/**
 * @brief Handles combat interactions between creatures
 * 
 * This class encapsulates all the logic for creature-creature combat interactions,
 * implementing the damage type system with shape-gene-based weapons.
 * 
 * The combat process involves:
 * 1. Weapon damage calculation from shape genes
 * 2. Defense value lookup from defense genes
 * 3. Type effectiveness calculation
 * 4. Final damage application with specialization bonus
 * 
 * Usage:
 * @code
 *   CombatInteraction combat;
 *   
 *   // Check if creature wants to fight
 *   if (CombatInteraction::shouldInitiateCombat(attackerPhenotype, targetPhenotype)) {
 *       // Calculate best attack
 *       CombatAction action = CombatInteraction::selectBestAction(
 *           attackerPhenotype, targetPhenotype);
 *       
 *       // Resolve the attack
 *       AttackResult result = CombatInteraction::resolveAttack(
 *           attackerPhenotype, targetPhenotype, action);
 *       
 *       // Apply damage
 *       target.takeDamage(result.finalDamage);
 *   }
 * @endcode
 * 
 * @see DamageDistribution
 * @see CombatAction
 * @see AttackResult
 */
class CombatInteraction {
public:
    /**
     * @brief Default constructor
     */
    CombatInteraction() = default;
    
    /**
     * @brief Destructor
     */
    ~CombatInteraction() = default;
    
    // ========================================================================
    // Weapon Damage Calculation
    // ========================================================================
    
    /**
     * @brief Calculate damage distribution for a weapon based on shape genes
     * @param phenotype The creature's expressed traits
     * @param weapon The weapon type to calculate damage for
     * @return DamageDistribution with pierce/slash/blunt values
     * 
     * Damage formulas from shape genes:
     * - Teeth: pierce=sharpness×size, slash=serration×size×0.5, blunt=(1-sharpness)×size
     * - Claws: pierce=curvature×length×sharpness, slash=(1-curvature)×length×sharpness
     * - Horns: pierce=pointiness×length, blunt=(1-pointiness)×length
     * - Tail: pierce=spines×length, blunt=mass×length
     * - Body: pierce=spines×maxSize, blunt=maxSize
     */
    static DamageDistribution calculateWeaponDamage(
        const Phenotype& phenotype,
        WeaponType weapon
    );
    
    /**
     * @brief Get defense profile from creature's genes
     * @param phenotype The creature's expressed traits
     * @return DefenseProfile with all defense values
     */
    static DefenseProfile getDefenseProfile(const Phenotype& phenotype);
    
    /**
     * @brief Get defense value for a specific defense type
     * @param phenotype The creature's expressed traits
     * @param type The defense type to query
     * @return Defense value (0.0 to 1.0)
     */
    static float getDefenseValue(const Phenotype& phenotype, DefenseType type);
    
    // ========================================================================
    // Damage Application
    // ========================================================================
    
    /**
     * @brief Apply damage with type effectiveness and defense
     * @param rawDamage Base damage before modifiers
     * @param attackType The combat damage type being dealt
     * @param defenderPhenotype The defender's expressed traits
     * @return Final damage after all reductions
     *
     * Formula:
     * 1. Get type effectiveness (0.5, 1.0, or 1.5)
     * 2. Get defense value for countering type
     * 3. finalDamage = rawDamage × effectiveness × (1 - defense × 0.5)
     */
    static float applyDamageWithDefense(
        float rawDamage,
        CombatDamageType attackType,
        const Phenotype& defenderPhenotype
    );
    
    /**
     * @brief Apply defense reduction to raw damage
     * @param rawDamage Damage before defense
     * @param defenseValue Defense gene value (0.0 to 1.0)
     * @return Damage after defense reduction (up to 50% reduction at max defense)
     */
    static float applyDefense(float rawDamage, float defenseValue);
    
    // ========================================================================
    // Combat Behavior
    // ========================================================================
    
    /**
     * @brief Check if creature should initiate combat
     * @param attackerPhenotype The attacker's expressed traits
     * @param targetPhenotype The potential target's expressed traits
     * @param attackerHunger Attacker's hunger level (0-1)
     * @return True if attacker should attack
     * 
     * Decision based on:
     * - COMBAT_AGGRESSION gene
     * - HUNT_INSTINCT gene
     * - Hunger level (desperate creatures fight more)
     */
    static bool shouldInitiateCombat(
        const Phenotype& attackerPhenotype,
        const Phenotype& targetPhenotype,
        float attackerHunger = 0.5f
    );
    
    /**
     * @brief Check if creature should retreat from combat
     * @param phenotype The creature's expressed traits
     * @param healthPercent Current health as fraction of max (0-1)
     * @return True if creature should flee
     */
    static bool shouldRetreat(const Phenotype& phenotype, float healthPercent);
    
    // ========================================================================
    // Specialization System
    // ========================================================================
    
    /**
     * @brief Calculate specialization bonus for focused damage investment
     * @param damage The damage distribution to evaluate
     * @return Bonus multiplier (0.0 to 0.5, added to 1.0 for final mult)
     * 
     * Specialists (focused investment) get up to 50% bonus damage.
     * Generalists (even split) get no bonus but have flexibility.
     */
    static float calculateSpecializationBonus(const DamageDistribution& damage);
    
    // ========================================================================
    // Action Selection
    // ========================================================================
    
    /**
     * @brief Select the best attack for the current situation
     * @param attackerPhenotype The attacker's expressed traits
     * @param defenderPhenotype The defender's expressed traits
     * @return The optimal CombatAction to use
     */
    static CombatAction selectBestAction(
        const Phenotype& attackerPhenotype,
        const Phenotype& defenderPhenotype
    );
    
    /**
     * @brief Get all available weapons for a creature
     * @param phenotype The creature's expressed traits
     * @return Vector of available weapon types
     */
    static std::vector<WeaponType> getAvailableWeapons(const Phenotype& phenotype);
    
    // ========================================================================
    // Attack Resolution
    // ========================================================================
    
    /**
     * @brief Resolve a single attack
     * @param attackerPhenotype The attacker's expressed traits
     * @param defenderPhenotype The defender's expressed traits
     * @param action The attack action to resolve
     * @return AttackResult with damage details
     */
    static AttackResult resolveAttack(
        const Phenotype& attackerPhenotype,
        const Phenotype& defenderPhenotype,
        const CombatAction& action
    );
    
    /**
     * @brief Resolve a full combat tick between two creatures
     * @param attackerPhenotype The attacker's expressed traits
     * @param defenderPhenotype The defender's expressed traits
     * @param attackerHealth Attacker's current health percent
     * @param defenderHealth Defender's current health percent
     * @return CombatResult with full encounter details
     */
    static CombatResult resolveCombatTick(
        const Phenotype& attackerPhenotype,
        const Phenotype& defenderPhenotype,
        float attackerHealth,
        float defenderHealth
    );
    
private:
    // ========================================================================
    // Shape Gene to Damage Type Formulas
    // ========================================================================
    
    /**
     * @brief Calculate teeth damage distribution
     * pierce = TEETH_SHARPNESS × TEETH_SIZE
     * slash  = TEETH_SERRATION × TEETH_SIZE × 0.5
     * blunt  = (1.0 - TEETH_SHARPNESS) × TEETH_SIZE
     */
    static DamageDistribution calculateTeethDamage(const Phenotype& phenotype);
    
    /**
     * @brief Calculate claws damage distribution
     * pierce = CLAW_CURVATURE × CLAW_LENGTH × CLAW_SHARPNESS
     * slash  = (1.0 - CLAW_CURVATURE) × CLAW_LENGTH × CLAW_SHARPNESS
     * blunt  = CLAW_LENGTH × (1.0 - CLAW_SHARPNESS) × 0.3
     */
    static DamageDistribution calculateClawsDamage(const Phenotype& phenotype);
    
    /**
     * @brief Calculate horns damage distribution
     * pierce = HORN_POINTINESS × HORN_LENGTH
     * slash  = HORN_SPREAD × HORN_LENGTH × 0.3
     * blunt  = (1.0 - HORN_POINTINESS) × HORN_LENGTH
     */
    static DamageDistribution calculateHornsDamage(const Phenotype& phenotype);
    
    /**
     * @brief Calculate tail damage distribution
     * pierce = TAIL_SPINES × TAIL_LENGTH
     * slash  = (1.0 - TAIL_MASS) × TAIL_LENGTH × 0.5
     * blunt  = TAIL_MASS × TAIL_LENGTH
     */
    static DamageDistribution calculateTailDamage(const Phenotype& phenotype);
    
    /**
     * @brief Calculate body damage distribution
     * pierce = BODY_SPINES × MAX_SIZE
     * slash  = 0.0
     * blunt  = MAX_SIZE
     */
    static DamageDistribution calculateBodyDamage(const Phenotype& phenotype);
    
public:
    // ========================================================================
    // Public Helper Methods (for combat logging)
    // ========================================================================
    
    /**
     * @brief Get defense type that counters a combat damage type
     */
    static DefenseType getCounteringDefense(CombatDamageType attackType);
    
private:
    // ========================================================================
    // Private Helper Methods
    // ========================================================================
    
    /**
     * @brief Get the size/magnitude factor for a weapon type
     * @param phenotype The creature's expressed traits
     * @param weapon The weapon type
     * @return Size factor value from the appropriate gene (0.0 to 1.0+)
     *
     * Maps weapons to their size genes:
     * - Teeth → TEETH_SIZE
     * - Claws → CLAW_LENGTH
     * - Horns → HORN_LENGTH
     * - Tail → TAIL_LENGTH
     * - Body → MAX_SIZE
     */
    static float getSizeFactorForWeapon(const Phenotype& phenotype, WeaponType weapon);
    
};

} // namespace Genetics
} // namespace EcoSim
