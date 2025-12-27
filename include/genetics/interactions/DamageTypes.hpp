#pragma once
#include <array>
#include <string>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Three combat damage types with rock-paper-scissors effectiveness
 *
 * Each damage type has strengths and weaknesses against different defense types:
 * - Piercing: Countered by ThickHide, strong vs Scales
 * - Slashing: Countered by Scales, strong vs FatLayer
 * - Blunt: Countered by FatLayer, strong vs ThickHide
 *
 * @note Named CombatDamageType to distinguish from HealthSystem::DamageType
 *       which covers broader damage categories (Physical, Toxin, etc.)
 */
enum class CombatDamageType {
    Piercing,  ///< Puncture damage (teeth, claws sharp)
    Slashing,  ///< Cutting damage (claws, horns sweep)
    Blunt      ///< Impact damage (tail, body mass)
};

/**
 * @brief Defense types corresponding to damage types
 */
enum class DefenseType {
    ThickHide,   ///< Reduces Piercing damage (HIDE_THICKNESS gene)
    Scales,      ///< Reduces Slashing damage (SCALE_COVERAGE gene)
    FatLayer     ///< Reduces Blunt damage (FAT_LAYER_THICKNESS gene)
};

/**
 * @brief Damage effectiveness table (rock-paper-scissors)
 * 
 * Index: [attackType][defenseType]
 * Values: 0.5 = resisted, 1.0 = neutral, 1.5 = strong
 * 
 * Biological rationale:
 * - Pierce beats Scales: Sharp points find gaps between scale edges
 * - Slash beats Fat: Cutting strokes slice through soft tissue easily
 * - Blunt beats Hide: Impact force transfers through tough leather
 * - Hide resists Pierce: Too thick and dense to puncture
 * - Scales resist Slash: Hard edges deflect and blunt cutting attacks
 * - Fat resists Blunt: Soft tissue absorbs and disperses impact energy
 */
constexpr float EFFECTIVENESS_TABLE[3][3] = {
    // vs ThickHide, vs Scales, vs FatLayer
    {0.5f, 1.5f, 1.0f},  // Piercing
    {1.0f, 0.5f, 1.5f},  // Slashing
    {1.5f, 1.0f, 0.5f}   // Blunt
};

/**
 * @brief Get type effectiveness multiplier for an attack against a defense
 * @param attack The damage type being dealt
 * @param defense The defense type being applied
 * @return Effectiveness multiplier (0.5 = resisted, 1.0 = neutral, 1.5 = strong)
 */
inline float getTypeEffectiveness(CombatDamageType attack, DefenseType defense) {
    return EFFECTIVENESS_TABLE[static_cast<int>(attack)][static_cast<int>(defense)];
}

/**
 * @brief Weapon types with base stats
 * 
 * Each creature has access to multiple attack methods based on morphology.
 * Weapon balance is achieved through tick-based cooldowns.
 */
enum class WeaponType {
    Teeth,   ///< 10 dmg, 3 tick cooldown, Pierce
    Claws,   ///< 6 dmg, 2 tick cooldown, Slash
    Horns,   ///< 12 dmg, 4 tick cooldown, Pierce/Blunt
    Tail,    ///< 5 dmg, 2 tick cooldown, Blunt
    Body     ///< 8 dmg, 3 tick cooldown, Blunt (always available)
};

/**
 * @brief Base statistics for a weapon type
 */
struct WeaponStats {
    float baseDamage;          ///< Base damage before modifiers
    int baseCooldown;          ///< Ticks between attacks
    CombatDamageType primaryType;    ///< Primary damage type dealt
};

/**
 * @brief Base stats per weapon type
 * 
 * Balance table (DPS balanced around 2.5-3.3):
 * - Teeth: 10 dmg / 3 ticks = 3.33 DPS (high damage, medium speed)
 * - Claws: 6 dmg / 2 ticks = 3.00 DPS (lower damage, fast)
 * - Horns: 12 dmg / 4 ticks = 3.00 DPS (high damage, slow)
 * - Tail: 5 dmg / 2 ticks = 2.50 DPS (low damage, fast, defensive)
 * - Body: 8 dmg / 3 ticks = 2.67 DPS (always available)
 */
constexpr WeaponStats WEAPON_BASE_STATS[] = {
    {10.0f, 3, CombatDamageType::Piercing},   // Teeth
    {6.0f, 2, CombatDamageType::Slashing},    // Claws
    {12.0f, 4, CombatDamageType::Piercing},   // Horns (can also be Blunt based on genes)
    {5.0f, 2, CombatDamageType::Blunt},       // Tail
    {8.0f, 3, CombatDamageType::Blunt}        // Body
};

/**
 * @brief Get base stats for a weapon type
 * @param weapon The weapon type to query
 * @return Reference to the weapon's base stats
 */
inline const WeaponStats& getWeaponStats(WeaponType weapon) {
    return WEAPON_BASE_STATS[static_cast<int>(weapon)];
}

/**
 * @brief Get string representation of a combat damage type
 * @param type The combat damage type
 * @return Human-readable string
 */
inline const char* combatDamageTypeToString(CombatDamageType type) {
    switch (type) {
        case CombatDamageType::Piercing: return "Piercing";
        case CombatDamageType::Slashing: return "Slashing";
        case CombatDamageType::Blunt: return "Blunt";
        default: return "Unknown";
    }
}

/**
 * @brief Get string representation of a defense type
 * @param type The defense type
 * @return Human-readable string
 */
inline const char* defenseTypeToString(DefenseType type) {
    switch (type) {
        case DefenseType::ThickHide: return "ThickHide";
        case DefenseType::Scales: return "Scales";
        case DefenseType::FatLayer: return "FatLayer";
        default: return "Unknown";
    }
}

/**
 * @brief Get string representation of a weapon type
 * @param type The weapon type
 * @return Human-readable string
 */
inline const char* weaponTypeToString(WeaponType type) {
    switch (type) {
        case WeaponType::Teeth: return "Teeth";
        case WeaponType::Claws: return "Claws";
        case WeaponType::Horns: return "Horns";
        case WeaponType::Tail: return "Tail";
        case WeaponType::Body: return "Body";
        default: return "Unknown";
    }
}

} // namespace Genetics
} // namespace EcoSim
