#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include <sstream>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

// ============================================================================
// Main interaction method
// ============================================================================

FeedingResult FeedingInteraction::attemptToEatPlant(
    const Phenotype& creaturePhenotype,
    const Plant& plant,
    float creatureHunger
) const {
    FeedingResult result;
    
    // ========================================================================
    // STEP 1: Detection - Can creature find/identify the plant as food?
    // ========================================================================
    
    float colorVision = getTraitSafe(creaturePhenotype, UniversalGenes::COLOR_VISION, 0.0f);
    float scentDetection = getTraitSafe(creaturePhenotype, UniversalGenes::SCENT_DETECTION, 0.0f);
    float sweetnessPreference = getTraitSafe(creaturePhenotype, UniversalGenes::SWEETNESS_PREFERENCE, 0.0f);
    float plantDigestion = getTraitSafe(creaturePhenotype, UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 0.0f);
    
    // Plant genes
    float fruitAppeal = plant.getFruitAppeal();
    
    // Detection threshold: needs color vision OR scent OR herbivore recognition
    // Both colorVision and scentDetection are 0-1 normalized values
    float detectionScore = (colorVision * fruitAppeal) + (scentDetection * 0.5f);
    
    // Herbivores can recognize plants as food through their digestive specialization
    // This represents evolved instinctive recognition of edible vegetation
    // A grazer evolved to eat grass doesn't need grass to look like colorful fruit!
    detectionScore += plantDigestion * 0.3f;  // Up to +0.3 for pure herbivores
    
    if (detectionScore < DETECTION_THRESHOLD) {
        result.success = false;
        result.description = "Cannot detect plant as food source";
        return result;
    }
    
    // Attraction: will creature bother eating this?
    float attraction = fruitAppeal * sweetnessPreference;
    
    // Herbivores are attracted to plants as forage, not just sweet fruit
    // This represents evolved appetite for vegetation as food source
    attraction += plantDigestion * 0.2f;  // Up to +0.2 for pure herbivores
    
    // Desperation: very hungry creatures will eat less appealing food
    float desperationBonus = creatureHunger * 0.3f;
    
    if (attraction + desperationBonus < ATTRACTION_THRESHOLD) {
        result.success = false;
        result.description = "Not appealing enough to eat";
        return result;
    }
    
    // ========================================================================
    // STEP 2: Physical Access - Can creature overcome plant defenses?
    // ========================================================================
    
    // Creature defense genes
    float hideThickness = getTraitSafe(creaturePhenotype, UniversalGenes::HIDE_THICKNESS, 0.0f);
    float painSensitivity = getTraitSafe(creaturePhenotype, UniversalGenes::PAIN_SENSITIVITY, 0.0f);
    float toxinTolerance = getTraitSafe(creaturePhenotype, UniversalGenes::TOXIN_TOLERANCE, 0.0f);
    float toxinMetabolism = getTraitSafe(creaturePhenotype, UniversalGenes::TOXIN_METABOLISM, 0.0f);
    
    // Plant defense genes
    float thornDensity = plant.getThornDamage();
    float toxinProduction = plant.getToxicity();
    
    // Thorn damage calculation
    float thornDamage = calculateThornDamage(creaturePhenotype, plant);
    
    // Pain threshold check (unless desperate)
    float painThreshold = (1.0f - painSensitivity);
    float desperationPainBonus = creatureHunger * 0.3f;
    
    if (thornDamage > (painThreshold + desperationPainBonus) * 15.0f && creatureHunger < 0.8f) {
        result.success = false;
        result.description = "Too painful (thorns/spines)";
        return result;
    }
    
    // Toxin damage calculation
    float toxinDamage = calculateToxinDamage(creaturePhenotype, plant);
    
    // Total damage taken
    result.damageReceived = thornDamage + toxinDamage;
    
    // Risk assessment: If damage exceeds benefit, creature may avoid
    float expectedNutrition = plant.getNutrientValue();
    float riskRatio = calculateRiskBenefitRatio(expectedNutrition, result.damageReceived);
    
    // Desperate creatures accept higher risk
    float riskThreshold = RISK_THRESHOLD + (creatureHunger * 0.3f);
    
    if (riskRatio > riskThreshold && creatureHunger < 0.7f) {
        result.success = false;
        result.description = "Risk (damage) exceeds benefit";
        return result;
    }
    
    // ========================================================================
    // STEP 3: Digestion - Can creature extract nutrition?
    // ========================================================================
    
    float digestionEfficiency = getDigestionEfficiency(creaturePhenotype, plant);
    
    // Base nutrition from plant
    float baseNutrition = expectedNutrition;
    
    // Calculate nutrition with efficiency multiplier
    result.nutritionGained = calculateNutritionExtracted(creaturePhenotype, plant);
    
    // Must gain minimum nutrition to be worth it
    if (result.nutritionGained < MIN_NUTRITION_THRESHOLD) {
        result.success = false;
        result.description = "Cannot digest this plant effectively";
        return result;
    }
    
    // ========================================================================
    // STEP 4: Seed Fate - What happens to seeds?
    // ========================================================================
    
    // Creature seed handling
    float seedDestruction = getTraitSafe(creaturePhenotype, UniversalGenes::SEED_DESTRUCTION_RATE, 0.0f);
    float gutTransit = getTraitSafe(creaturePhenotype, UniversalGenes::GUT_TRANSIT_TIME, 0.0f);
    
    // Plant seed properties
    float seedCoatDurability = plant.getSeedCoatDurability();
    int seedCount = plant.getSeedCount();
    
    // Calculate seed survival
    result.seedsConsumed = (seedCount > 0);
    
    if (result.seedsConsumed) {
        float survivalRate = calculateSeedSurvivalRate(creaturePhenotype, plant);
        
        // Transit time affects seed viability
        float transitMultiplier = 1.0f;
        if (gutTransit < OPTIMAL_TRANSIT_LOW) {
            transitMultiplier = 0.9f;  // Too fast, seeds not scarified
        } else if (gutTransit > OPTIMAL_TRANSIT_HIGH) {
            transitMultiplier = 0.7f;  // Too slow, acid damage
        } else {
            transitMultiplier = 1.2f;  // Optimal range, scarification helps
        }
        
        result.seedsToDisperse = static_cast<float>(seedCount) * survivalRate * transitMultiplier;
        result.seedsDestroyed = (survivalRate < 0.5f);
        
        // Destroyed seeds add nutrition
        float destroyedSeeds = static_cast<float>(seedCount) * (1.0f - survivalRate);
        float seedNutrition = destroyedSeeds * SEED_ENERGY_VALUE;
        result.nutritionGained += seedNutrition;
    }
    
    // Calculate damage to plant
    result.plantDamage = 0.3f + (digestionEfficiency * 0.5f);  // 30-80% damage
    
    // ========================================================================
    // SUCCESS!
    // ========================================================================
    
    result.success = true;
    result.description = buildDescription(result, "plant");
    
    return result;
}

// ============================================================================
// Detection phase
// ============================================================================

bool FeedingInteraction::canDetectPlant(
    const Phenotype& creature,
    const Plant& plant,
    float distance
) const {
    float detectionRange = getDetectionRange(creature, plant);
    return distance <= detectionRange;
}

float FeedingInteraction::getDetectionRange(
    const Phenotype& creature,
    const Plant& plant
) const {
    float colorVision = getTraitSafe(creature, UniversalGenes::COLOR_VISION, 0.0f);
    float scentDetection = getTraitSafe(creature, UniversalGenes::SCENT_DETECTION, 0.0f);
    float sightRange = getTraitSafe(creature, UniversalGenes::SIGHT_RANGE, 0.0f);
    
    float fruitAppeal = plant.getFruitAppeal();
    
    // CORRECT FORMULA:
    // Color vision specifically helps spot COLORFUL plants (high fruit appeal)
    // - High CV + high fruit appeal = big range bonus (fruit is easy to see)
    // - High CV + low fruit appeal = minimal bonus (green grass blends in)
    // Scent works independently of color
    
    float visualBonus = colorVision * fruitAppeal * 100.0f;  // 0.9 × 0.8 × 100 = 72 tiles
    float scentBonus = scentDetection * 100.0f;              // 0.9 × 100 = 90 tiles
    float effectiveRange = sightRange + std::max(visualBonus, scentBonus);
    
    return effectiveRange;
}

// ============================================================================
// Access phase
// ============================================================================

float FeedingInteraction::calculateThornDamage(
    const Phenotype& creature,
    const Plant& plant
) const {
    float hideThickness = getTraitSafe(creature, UniversalGenes::HIDE_THICKNESS, 0.0f);
    float thornDensity = plant.getThornDamage();
    
    // Base thorn damage from plant
    float baseDamage = thornDensity * THORN_DAMAGE_BASE;
    
    // Hide reduces damage
    float hideProtection = hideThickness * HIDE_PROTECTION_FACTOR;
    
    return std::max(0.0f, baseDamage - hideProtection);
}

float FeedingInteraction::calculateToxinDamage(
    const Phenotype& creature,
    const Plant& plant
) const {
    float toxinTolerance = getTraitSafe(creature, UniversalGenes::TOXIN_TOLERANCE, 0.0f);
    float toxinMetabolism = getTraitSafe(creature, UniversalGenes::TOXIN_METABOLISM, 0.0f);
    float mucusProtection = getTraitSafe(creature, UniversalGenes::MUCUS_PROTECTION, 0.0f);
    
    float toxinProduction = plant.getToxicity();
    
    // Base toxin damage
    float baseDamage = toxinProduction * TOXIN_DAMAGE_BASE;
    
    // Combined defense factors
    // Formula: toxin_damage = toxin_production * (1 - toxin_tolerance) * (1 - toxin_metabolism)
    float toleranceReduction = 1.0f - toxinTolerance;
    float metabolismReduction = 1.0f - toxinMetabolism;
    float mucusReduction = 1.0f - (mucusProtection * 0.5f);  // Mucus provides partial protection
    
    return baseDamage * toleranceReduction * metabolismReduction * mucusReduction;
}

bool FeedingInteraction::canOvercomeDefenses(
    const Phenotype& creature,
    const Plant& plant
) const {
    float thornDamage = calculateThornDamage(creature, plant);
    float toxinDamage = calculateToxinDamage(creature, plant);
    float totalDamage = thornDamage + toxinDamage;
    
    float expectedNutrition = plant.getNutrientValue();
    
    // GENETICS-MIGRATION: Lowered threshold to make feeding more accessible
    // Original: damage < nutrition * 0.5 (too strict)
    // Now: damage < nutrition * 2.0 (allow moderate damage for nutrition)
    // This enables creatures to actually eat plants and survive
    return totalDamage < (expectedNutrition * 2.0f);
}

// ============================================================================
// Digestion phase
// ============================================================================

float FeedingInteraction::calculateNutritionExtracted(
    const Phenotype& creature,
    const Plant& plant
) const {
    float baseNutrition = plant.getNutrientValue();
    float efficiency = getDigestionEfficiency(creature, plant);
    
    // Apply specialist bonus
    float specialistBonus = creature.getSpecialistBonus();
    
    // Apply metabolic overhead penalty
    float overhead = creature.getMetabolicOverhead();
    
    // Final nutrition = base * efficiency * specialist_bonus / overhead
    return baseNutrition * efficiency * specialistBonus / overhead;
}

float FeedingInteraction::getDigestionEfficiency(
    const Phenotype& creature,
    const Plant& plant
) const {
    float plantDigestion = getTraitSafe(creature, UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 0.0f);
    float celluloseBreakdown = getTraitSafe(creature, UniversalGenes::CELLULOSE_BREAKDOWN, 0.0f);
    
    // Base efficiency from plant digestion gene
    float baseEfficiency = plantDigestion;
    
    // Cellulose breakdown adds extra efficiency for tough plants
    float hardiness = plant.getHardiness();
    float celluloseBonus = celluloseBreakdown * hardiness * 0.3f;
    
    // Combined efficiency (capped at 1.0 without specialist bonus)
    return std::min(1.0f, baseEfficiency + celluloseBonus);
}

// ============================================================================
// Seed dispersal
// ============================================================================

float FeedingInteraction::calculateSeedSurvivalRate(
    const Phenotype& creature,
    const Plant& plant
) const {
    float seedDestruction = getTraitSafe(creature, UniversalGenes::SEED_DESTRUCTION_RATE, 0.0f);
    float seedCoatDurability = plant.getSeedCoatDurability();
    
    // Seeds survive passage if coat is durable AND creature doesn't destroy them
    // Formula: survival = seed_coat_durability - seed_destruction_rate
    float survivalRate = seedCoatDurability * (1.0f - seedDestruction);
    
    return std::max(0.0f, std::min(1.0f, survivalRate));
}

float FeedingInteraction::calculateDispersalDistance(
    const Phenotype& creature
) const {
    float locomotion = getTraitSafe(creature, UniversalGenes::LOCOMOTION, 0.0f);
    float gutTransit = getTraitSafe(creature, UniversalGenes::GUT_TRANSIT_TIME, 0.0f);
    
    // Distance = speed * transit time (in game ticks)
    // Convert transit time from hours to approximate game ticks
    float transitTicks = gutTransit * 10.0f;  // Assume 10 ticks per hour
    
    return locomotion * transitTicks;
}

bool FeedingInteraction::willCacheSeeds(
    const Phenotype& creature
) const {
    float cachingInstinct = getTraitSafe(creature, UniversalGenes::CACHING_INSTINCT, 0.0f);
    return cachingInstinct > CACHING_THRESHOLD;
}

// ============================================================================
// Internal helper methods
// ============================================================================

float FeedingInteraction::calculateRiskBenefitRatio(
    float expectedNutrition,
    float expectedDamage
) const {
    if (expectedNutrition <= 0.0f) {
        return 1.0f;  // Maximum risk if no benefit
    }
    return expectedDamage / expectedNutrition;
}

std::string FeedingInteraction::buildDescription(
    const FeedingResult& result,
    const std::string& plantInfo
) const {
    std::ostringstream ss;
    
    ss << "Ate " << plantInfo;
    ss << ", gained " << static_cast<int>(result.nutritionGained) << " nutrition";
    
    if (result.damageReceived > 0.0f) {
        ss << ", took " << static_cast<int>(result.damageReceived) << " damage";
    }
    
    if (result.seedsConsumed) {
        if (result.seedsToDisperse > 0) {
            ss << ", will disperse " << static_cast<int>(result.seedsToDisperse) << " seeds";
        }
        if (result.seedsDestroyed) {
            ss << " (many seeds destroyed)";
        }
    }
    
    return ss.str();
}

} // namespace Genetics
} // namespace EcoSim
