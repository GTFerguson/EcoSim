/**
 * @file HealthSystem.cpp
 * @brief Implementation of organism-agnostic health management system
 *
 * All methods use Organism& interface and phenotype traits.
 * NO type-specific code (Plant, Creature, DietType, etc.)
 *
 * @see include/genetics/systems/HealthSystem.hpp
 */

#include "genetics/systems/HealthSystem.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <cmath>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

// =============================================================================
// Resistance Calculation
// =============================================================================

float HealthSystem::calculateResistance(
    const Organism& organism,
    DamageType type) const
{
    const Phenotype& phenotype = organism.getPhenotype();
    
    switch (type) {
        case DamageType::Physical: {
            // Physical damage resisted by hardiness (max 50% reduction)
            float hardiness = getTraitSafe(phenotype, UniversalGenes::HARDINESS, 0.0f);
            return clamp01(hardiness * MAX_HARDINESS_RESISTANCE);
        }
        
        case DamageType::Toxin: {
            // Toxin damage resisted by toxin_tolerance trait
            float toxinResistance = getTraitSafe(phenotype, UniversalGenes::TOXIN_TOLERANCE, 0.0f);
            return clamp01(toxinResistance);
        }
        
        case DamageType::Starvation: {
            // Starvation has no resistance
            return 0.0f;
        }
        
        case DamageType::Environment: {
            // Environmental damage resisted by hardiness (max 50% reduction)
            float hardiness = getTraitSafe(phenotype, UniversalGenes::HARDINESS, 0.0f);
            return clamp01(hardiness * MAX_HARDINESS_RESISTANCE);
        }
        
        default:
            return 0.0f;
    }
}

// =============================================================================
// Damage Operations
// =============================================================================

DamageResult HealthSystem::applyDamage(
    const Organism& organism,
    float& currentHealth,
    float amount,
    DamageType type) const
{
    DamageResult result;
    
    // Early exit for non-positive damage
    if (amount <= 0.0f) {
        return result;
    }
    
    // Calculate resistance for this damage type
    result.resistanceApplied = calculateResistance(organism, type);
    
    // Apply resistance to reduce damage
    result.actualDamage = amount * (1.0f - result.resistanceApplied);
    
    // Apply damage to health
    currentHealth -= result.actualDamage;
    currentHealth = clampMin0(currentHealth);
    
    // Check if fatal
    result.fatal = (currentHealth <= 0.0f);
    
    return result;
}

// =============================================================================
// Healing Operations
// =============================================================================

HealingResult HealthSystem::heal(
    const Organism& organism,
    float& currentHealth,
    float amount) const
{
    HealingResult result;
    
    // Early exit for non-positive healing
    if (amount <= 0.0f) {
        result.newHealth = currentHealth;
        return result;
    }
    
    // Get max health for capping
    float maxHealth = getMaxHealth(organism);
    
    // Calculate actual healing (can't exceed max)
    float potentialHealth = currentHealth + amount;
    result.newHealth = std::min(potentialHealth, maxHealth);
    result.actualHealing = result.newHealth - currentHealth;
    
    // Apply healing
    currentHealth = result.newHealth;
    
    return result;
}

HealingResult HealthSystem::processNaturalHealing(
    const Organism& organism,
    float& currentHealth,
    float energySurplus) const
{
    HealingResult result;
    result.newHealth = currentHealth;
    
    // Check if regeneration is possible
    if (!canRegenerate(organism, currentHealth, energySurplus)) {
        return result;
    }
    
    // Get healing rate and apply
    float healingRate = getHealingRate(organism);
    
    return heal(organism, currentHealth, healingRate);
}

// =============================================================================
// Query Operations
// =============================================================================

bool HealthSystem::checkDeathCondition(float currentHealth) const
{
    return currentHealth <= 0.0f;
}

WoundState HealthSystem::getWoundState(
    const Organism& organism,
    float currentHealth) const
{
    float healthPercent = getHealthPercent(organism, currentHealth);
    
    if (healthPercent <= 0.0f) {
        return WoundState::Dead;
    } else if (healthPercent > HEALTHY_THRESHOLD) {
        return WoundState::Healthy;
    } else if (healthPercent > INJURED_THRESHOLD) {
        return WoundState::Injured;
    } else if (healthPercent > WOUNDED_THRESHOLD) {
        return WoundState::Wounded;
    } else {
        return WoundState::Critical;
    }
}

float HealthSystem::getWoundSeverity(
    const Organism& organism,
    float currentHealth) const
{
    float healthPercent = getHealthPercent(organism, currentHealth);
    return clamp01(1.0f - healthPercent);
}

// =============================================================================
// Capacity Operations
// =============================================================================

float HealthSystem::getMaxHealth(const Organism& organism) const
{
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Check for max_size trait (used by creatures)
    if (phenotype.hasTrait(UniversalGenes::MAX_SIZE)) {
        float maxSize = phenotype.getTrait(UniversalGenes::MAX_SIZE);
        return maxSize * MAX_SIZE_HEALTH_MULTIPLIER;
    }
    
    // Default max health for organisms without size trait (like plants)
    return DEFAULT_MAX_HEALTH;
}

float HealthSystem::getHealthPercent(
    const Organism& organism,
    float currentHealth) const
{
    float maxHealth = getMaxHealth(organism);
    
    if (maxHealth <= 0.0f) {
        return 0.0f;
    }
    
    return clamp01(currentHealth / maxHealth);
}

// =============================================================================
// Regeneration Operations
// =============================================================================

bool HealthSystem::canRegenerate(
    const Organism& organism,
    float currentHealth,
    float energySurplus) const
{
    // Can't regenerate if dead
    if (currentHealth <= 0.0f) {
        return false;
    }
    
    // Already at full health?
    float maxHealth = getMaxHealth(organism);
    if (currentHealth >= maxHealth) {
        return false;
    }
    
    // Need energy surplus for regeneration
    if (energySurplus <= 0.0f) {
        return false;
    }
    
    // Need minimum regeneration ability
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Check regrowth_rate (plants) or regeneration_rate (creatures)
    float regrowthRate = getTraitSafe(phenotype, UniversalGenes::REGROWTH_RATE, 0.0f);
    float regenRate = getTraitSafe(phenotype, UniversalGenes::REGENERATION_RATE, 0.0f);
    
    // Either trait above threshold enables regeneration
    return (regrowthRate > MIN_REGENERATION_THRESHOLD) ||
           (regenRate > MIN_REGENERATION_THRESHOLD);
}

float HealthSystem::getHealingRate(const Organism& organism) const
{
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Try regrowth_rate first (plants use this)
    if (phenotype.hasTrait(UniversalGenes::REGROWTH_RATE)) {
        float regrowth = phenotype.getTrait(UniversalGenes::REGROWTH_RATE);
        return regrowth * REGROWTH_RATE_MULTIPLIER;
    }
    
    // Try regeneration_rate (creatures use this)
    if (phenotype.hasTrait(UniversalGenes::REGENERATION_RATE)) {
        // Regeneration rate is already in usable form
        // Scale by 0.001 to match creature healing behavior
        float regen = phenotype.getTrait(UniversalGenes::REGENERATION_RATE);
        return regen * 0.001f;
    }
    
    // Default healing rate
    return DEFAULT_HEALING_RATE;
}

} // namespace Genetics
} // namespace EcoSim
