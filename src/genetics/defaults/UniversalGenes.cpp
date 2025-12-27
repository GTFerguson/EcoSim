/**
 * @file UniversalGenes.cpp
 * @brief Implementation of unified gene definitions for all organism types
 *
 * This file creates a single gene registry that can be used by any organism.
 * Expression levels determine which genes are active:
 * - Creatures: High mobility/heterotrophy, dormant autotrophy
 * - Plants: High autotrophy, dormant mobility/heterotrophy
 * - Hybrids: Can evolve any combination!
 */

#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include <random>

namespace EcoSim {
namespace Genetics {

// Static member definitions
std::unordered_map<std::string, GeneCategory> UniversalGenes::s_geneCategories;
bool UniversalGenes::s_categoriesInitialized = false;

void UniversalGenes::initializeCategories() {
    if (s_categoriesInitialized) return;
    
    // Universal genes
    s_geneCategories[LIFESPAN] = GeneCategory::Universal;
    s_geneCategories[MAX_SIZE] = GeneCategory::Universal;
    s_geneCategories[METABOLISM_RATE] = GeneCategory::Universal;
    s_geneCategories[COLOR_HUE] = GeneCategory::Universal;
    s_geneCategories[HARDINESS] = GeneCategory::Universal;
    s_geneCategories[TEMP_TOLERANCE_LOW] = GeneCategory::Universal;
    s_geneCategories[TEMP_TOLERANCE_HIGH] = GeneCategory::Universal;
    
    // Mobility genes
    s_geneCategories[LOCOMOTION] = GeneCategory::Mobility;
    s_geneCategories[SIGHT_RANGE] = GeneCategory::Mobility;
    s_geneCategories[NAVIGATION_ABILITY] = GeneCategory::Mobility;
    s_geneCategories[FLEE_THRESHOLD] = GeneCategory::Mobility;
    s_geneCategories[PURSUE_THRESHOLD] = GeneCategory::Mobility;
    
    // Autotrophy genes
    s_geneCategories[PHOTOSYNTHESIS] = GeneCategory::Autotrophy;
    s_geneCategories[ROOT_DEPTH] = GeneCategory::Autotrophy;
    s_geneCategories[WATER_STORAGE] = GeneCategory::Autotrophy;
    s_geneCategories[LIGHT_REQUIREMENT] = GeneCategory::Autotrophy;
    s_geneCategories[WATER_REQUIREMENT] = GeneCategory::Autotrophy;
    
    // Original Heterotrophy genes
    s_geneCategories[HUNT_INSTINCT] = GeneCategory::Heterotrophy;
    s_geneCategories[HUNGER_THRESHOLD] = GeneCategory::Heterotrophy;
    s_geneCategories[THIRST_THRESHOLD] = GeneCategory::Heterotrophy;
    s_geneCategories[DIET_TYPE] = GeneCategory::Heterotrophy;  // Deprecated
    s_geneCategories[DIGESTIVE_EFFICIENCY] = GeneCategory::Heterotrophy;
    s_geneCategories[NUTRIENT_VALUE] = GeneCategory::Heterotrophy;
    
    // New Heterotrophy genes for coevolution
    s_geneCategories[PLANT_DIGESTION_EFFICIENCY] = GeneCategory::Heterotrophy;
    s_geneCategories[MEAT_DIGESTION_EFFICIENCY] = GeneCategory::Heterotrophy;
    s_geneCategories[CELLULOSE_BREAKDOWN] = GeneCategory::Heterotrophy;
    s_geneCategories[TOXIN_TOLERANCE] = GeneCategory::Heterotrophy;
    s_geneCategories[TOXIN_METABOLISM] = GeneCategory::Heterotrophy;
    s_geneCategories[SCENT_DETECTION] = GeneCategory::Heterotrophy;
    s_geneCategories[COLOR_VISION] = GeneCategory::Heterotrophy;
    
    // New Morphology genes
    s_geneCategories[GUT_LENGTH] = GeneCategory::Morphology;
    s_geneCategories[TOOTH_SHARPNESS] = GeneCategory::Morphology;
    s_geneCategories[TOOTH_GRINDING] = GeneCategory::Morphology;
    s_geneCategories[STOMACH_ACIDITY] = GeneCategory::Morphology;
    s_geneCategories[JAW_STRENGTH] = GeneCategory::Morphology;
    s_geneCategories[JAW_SPEED] = GeneCategory::Morphology;
    s_geneCategories[HIDE_THICKNESS] = GeneCategory::Morphology;
    s_geneCategories[FUR_DENSITY] = GeneCategory::Morphology;
    s_geneCategories[MUCUS_PROTECTION] = GeneCategory::Morphology;
    
    // New Behavior genes
    s_geneCategories[SWEETNESS_PREFERENCE] = GeneCategory::Behavior;
    s_geneCategories[CACHING_INSTINCT] = GeneCategory::Behavior;
    s_geneCategories[SPATIAL_MEMORY] = GeneCategory::Behavior;
    s_geneCategories[GROOMING_FREQUENCY] = GeneCategory::Behavior;
    s_geneCategories[PAIN_SENSITIVITY] = GeneCategory::Behavior;
    
    // Health/Healing genes
    s_geneCategories[REGENERATION_RATE] = GeneCategory::Morphology;
    s_geneCategories[WOUND_TOLERANCE] = GeneCategory::Behavior;
    s_geneCategories[BLEEDING_RESISTANCE] = GeneCategory::Morphology;
    
    // Combat Weapon Shape genes
    s_geneCategories[TEETH_SHARPNESS] = GeneCategory::Morphology;
    s_geneCategories[TEETH_SERRATION] = GeneCategory::Morphology;
    s_geneCategories[TEETH_SIZE] = GeneCategory::Morphology;
    s_geneCategories[CLAW_LENGTH] = GeneCategory::Morphology;
    s_geneCategories[CLAW_CURVATURE] = GeneCategory::Morphology;
    s_geneCategories[CLAW_SHARPNESS] = GeneCategory::Morphology;
    s_geneCategories[HORN_LENGTH] = GeneCategory::Morphology;
    s_geneCategories[HORN_POINTINESS] = GeneCategory::Morphology;
    s_geneCategories[HORN_SPREAD] = GeneCategory::Morphology;
    s_geneCategories[TAIL_LENGTH] = GeneCategory::Morphology;
    s_geneCategories[TAIL_MASS] = GeneCategory::Morphology;
    s_geneCategories[TAIL_SPINES] = GeneCategory::Morphology;
    s_geneCategories[BODY_SPINES] = GeneCategory::Morphology;
    
    // Combat Defense genes
    s_geneCategories[SCALE_COVERAGE] = GeneCategory::Morphology;
    s_geneCategories[FAT_LAYER_THICKNESS] = GeneCategory::Morphology;
    
    // Combat Behavior genes
    s_geneCategories[COMBAT_AGGRESSION] = GeneCategory::Behavior;
    s_geneCategories[RETREAT_THRESHOLD] = GeneCategory::Behavior;
    s_geneCategories[TERRITORIAL_AGGRESSION] = GeneCategory::Behavior;
    s_geneCategories[PACK_COORDINATION] = GeneCategory::Behavior;
    
    // Olfactory System genes
    s_geneCategories[SCENT_PRODUCTION] = GeneCategory::Behavior;
    s_geneCategories[SCENT_SIGNATURE_VARIANCE] = GeneCategory::Reproduction;
    s_geneCategories[OLFACTORY_ACUITY] = GeneCategory::Behavior;
    s_geneCategories[SCENT_MASKING] = GeneCategory::Behavior;
    
    // Seed interaction genes (metabolism related)
    s_geneCategories[GUT_TRANSIT_TIME] = GeneCategory::Heterotrophy;
    s_geneCategories[SEED_DESTRUCTION_RATE] = GeneCategory::Heterotrophy;
    
    // Plant defense genes
    s_geneCategories[TOXIN_PRODUCTION] = GeneCategory::PlantDefense;
    s_geneCategories[THORN_DENSITY] = GeneCategory::PlantDefense;
    s_geneCategories[REGROWTH_RATE] = GeneCategory::PlantDefense;
    s_geneCategories[FRUIT_PRODUCTION_RATE] = GeneCategory::PlantDefense;
    s_geneCategories[SEED_COAT_DURABILITY] = GeneCategory::PlantDefense;
    s_geneCategories[FRUIT_APPEAL] = GeneCategory::PlantDefense;
    
    // Reproduction genes
    s_geneCategories[OFFSPRING_COUNT] = GeneCategory::Reproduction;
    s_geneCategories[MATE_THRESHOLD] = GeneCategory::Reproduction;
    s_geneCategories[SPREAD_DISTANCE] = GeneCategory::Reproduction;
    s_geneCategories[FATIGUE_THRESHOLD] = GeneCategory::Reproduction;
    s_geneCategories[COMFORT_INCREASE] = GeneCategory::Reproduction;
    s_geneCategories[COMFORT_DECREASE] = GeneCategory::Reproduction;
    
    // Seed propagation genes
    s_geneCategories[SEED_MASS] = GeneCategory::Reproduction;
    s_geneCategories[SEED_AERODYNAMICS] = GeneCategory::Reproduction;
    s_geneCategories[SEED_HOOK_STRENGTH] = GeneCategory::Reproduction;
    s_geneCategories[EXPLOSIVE_POD_FORCE] = GeneCategory::Reproduction;
    s_geneCategories[RUNNER_PRODUCTION] = GeneCategory::Reproduction;
    
    s_categoriesInitialized = true;
}

GeneCategory UniversalGenes::getCategory(const std::string& gene_id) {
    initializeCategories();
    auto it = s_geneCategories.find(gene_id);
    if (it != s_geneCategories.end()) {
        return it->second;
    }
    return GeneCategory::Universal; // Default fallback
}

void UniversalGenes::registerDefaults(GeneRegistry& registry) {
    // Early exit if defaults already registered - makes this function idempotent
    if (registry.areDefaultsRegistered()) {
        return;
    }
    
    initializeCategories();
    
    registerUniversalGenes(registry);
    registerMobilityGenes(registry);
    registerAutotrophyGenes(registry);
    registerHeterotrophyGenes(registry);
    registerReproductionGenes(registry);
    
    // Additional gene categories
    registerCoevolutionHeterotrophyGenes(registry);
    registerMorphologyGenes(registry);
    registerBehaviorGenes(registry);
    registerSeedInteractionGenes(registry);
    registerPlantDefenseGenes(registry);
    registerSeedPropagationGenes(registry);
    registerOlfactoryGenes(registry);
    registerHealthHealingGenes(registry);
    
    // Combat system genes (Phase 1c)
    registerWeaponShapeGenes(registry);
    registerCombatDefenseGenes(registry);
    registerCombatBehaviorGenes(registry);
    
    // Mark that defaults have been registered
    registry.markDefaultsRegistered();
}

void UniversalGenes::registerUniversalGenes(GeneRegistry& registry) {
    // ============================================================================
    // UNIVERSAL GENES - All organisms have these
    // Energy Cost Range: 0.01-0.05 (passive structural traits)
    // ============================================================================
    
    // Lifespan - affects how long organism lives
    // [100, 1000000], creep 1000
    // Cost: 0.02 - longer-lived cells need more repair mechanisms
    GeneDefinition lifespan(LIFESPAN, ChromosomeType::Lifespan,
        GeneLimits(100.0f, 1000000.0f, 1000.0f), DominanceType::Incomplete);
    lifespan.addEffect(EffectBinding("lifespan", "max_age", EffectType::Direct, 1.0f));
    lifespan.setMaintenanceCost(0.02f);
    lifespan.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(lifespan));
    
    // Max Size - maximum organism size
    // [0.2, 20.0], creep 0.2 - lowered from 0.5 to help prevent baby cannibalism exploit
    // Cost: 0.05 - larger bodies need proportionally more maintenance (superlinear)
    GeneDefinition maxSize(MAX_SIZE, ChromosomeType::Morphology,
        GeneLimits(0.2f, 20.0f, 0.2f), DominanceType::Incomplete);
    maxSize.addEffect(EffectBinding("morphology", "max_size", EffectType::Direct, 1.0f));
    maxSize.setMaintenanceCost(0.05f);
    maxSize.setCostScaling(1.5f);  // Superlinear - size is expensive
    registry.tryRegisterGene(std::move(maxSize));
    
    // Metabolism Rate - base metabolic rate
    // [0.1, 3.0], creep 0.1
    // Cost: 0.03 - higher metabolism requires more enzyme production
    GeneDefinition metabolismRate(METABOLISM_RATE, ChromosomeType::Metabolism,
        GeneLimits(0.1f, 3.0f, 0.1f), DominanceType::Incomplete);
    metabolismRate.addEffect(EffectBinding("metabolism", "base_rate", EffectType::Direct, 1.0f));
    metabolismRate.setMaintenanceCost(0.03f);
    metabolismRate.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(metabolismRate));
    
    // Color Hue - HSV hue for visual appearance
    // [0.0, 360.0], creep 10.0
    // Cost: 0.01 - pigment production minimal cost
    GeneDefinition colorHue(COLOR_HUE, ChromosomeType::Morphology,
        GeneLimits(0.0f, 360.0f, 10.0f), DominanceType::Incomplete);
    colorHue.addEffect(EffectBinding("morphology", "color_hue", EffectType::Direct, 1.0f));
    colorHue.setMaintenanceCost(0.01f);
    colorHue.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(colorHue));
    
    // Hardiness - resistance to damage and environmental stress
    // [0.0, 1.0], creep 0.05
    // Cost: 0.04 - stress resistance proteins are energetically costly
    GeneDefinition hardiness(HARDINESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    hardiness.addEffect(EffectBinding("morphology", "damage_resistance", EffectType::Direct, 1.0f));
    hardiness.setMaintenanceCost(0.04f);
    hardiness.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(hardiness));
    
    // Temperature Tolerance Low - minimum survival temperature
    // [-30.0, 30.0], creep 5.0
    // Cost: 0.03 - cold adaptation requires antifreeze proteins
    GeneDefinition tempLow(TEMP_TOLERANCE_LOW, ChromosomeType::Environmental,
        GeneLimits(-30.0f, 30.0f, 5.0f), DominanceType::Incomplete);
    tempLow.addEffect(EffectBinding("environmental", "temp_tolerance_low", EffectType::Direct, 1.0f));
    tempLow.setMaintenanceCost(0.03f);
    tempLow.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(tempLow));
    
    // Temperature Tolerance High - maximum survival temperature
    // [10.0, 60.0], creep 5.0
    // Cost: 0.03 - heat shock proteins are expensive
    GeneDefinition tempHigh(TEMP_TOLERANCE_HIGH, ChromosomeType::Environmental,
        GeneLimits(10.0f, 60.0f, 5.0f), DominanceType::Incomplete);
    tempHigh.addEffect(EffectBinding("environmental", "temp_tolerance_high", EffectType::Direct, 1.0f));
    tempHigh.setMaintenanceCost(0.03f);
    tempHigh.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(tempHigh));
}

void UniversalGenes::registerMobilityGenes(GeneRegistry& registry) {
    // ============================================================================
    // MOBILITY GENES - Active in creatures, dormant in plants
    // Energy Cost Range: 0.15-0.25 (muscle maintenance, high cost)
    // ============================================================================
    
    // Locomotion - movement speed capability
    // [0.0, 2.0], creep 0.1
    // Cost: 0.20 - fast movement = bigger muscles = high maintenance
    GeneDefinition locomotion(LOCOMOTION, ChromosomeType::Locomotion,
        GeneLimits(0.0f, 2.0f, 0.1f), DominanceType::Incomplete);
    locomotion.addEffect(EffectBinding("locomotion", "speed_multiplier", EffectType::Direct, 1.0f));
    locomotion.setMaintenanceCost(0.20f);
    locomotion.setCostScaling(1.3f);  // Superlinear - speed is expensive
    registry.tryRegisterGene(std::move(locomotion));
    
    // Sight Range - vision distance
    // [0.0, 200.0], creep 5.0
    // Cost: 0.10 - long-range vision = large retinas + brain processing
    GeneDefinition sightRange(SIGHT_RANGE, ChromosomeType::Sensory,
        GeneLimits(0.0f, 200.0f, 5.0f), DominanceType::Incomplete);
    sightRange.addEffect(EffectBinding("sensory", "vision_range", EffectType::Direct, 1.0f));
    sightRange.setMaintenanceCost(0.10f);
    sightRange.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(sightRange));
    
    // Navigation Ability - pathfinding capability
    // [0.0, 1.0], creep 0.05
    // Cost: 0.08 - pathfinding requires spatial memory (brain tissue)
    GeneDefinition navAbility(NAVIGATION_ABILITY, ChromosomeType::Sensory,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    navAbility.addEffect(EffectBinding("sensory", "navigation_skill", EffectType::Direct, 1.0f));
    navAbility.setMaintenanceCost(0.08f);
    navAbility.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(navAbility));
    
    // Flee Threshold - distance at which fleeing is triggered
    // [0.0, 30.0], creep 2.0
    // Cost: 0.03 - fast reflexes require alert nervous system
    GeneDefinition fleeThreshold(FLEE_THRESHOLD, ChromosomeType::Behavior,
        GeneLimits(0.0f, 30.0f, 2.0f), DominanceType::Incomplete);
    fleeThreshold.addEffect(EffectBinding("behavior", "flee_trigger_distance", EffectType::Direct, 1.0f));
    fleeThreshold.setMaintenanceCost(0.03f);
    fleeThreshold.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(fleeThreshold));
    
    // Pursue Threshold - distance at which pursuing is triggered
    // [0.0, 30.0], creep 2.0
    // Cost: 0.03 - hunting behavior needs sustained attention
    GeneDefinition pursueThreshold(PURSUE_THRESHOLD, ChromosomeType::Behavior,
        GeneLimits(0.0f, 30.0f, 2.0f), DominanceType::Incomplete);
    pursueThreshold.addEffect(EffectBinding("behavior", "pursue_trigger_distance", EffectType::Direct, 1.0f));
    pursueThreshold.setMaintenanceCost(0.03f);
    pursueThreshold.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(pursueThreshold));
}

void UniversalGenes::registerAutotrophyGenes(GeneRegistry& registry) {
    // ============================================================================
    // AUTOTROPHY GENES - Active in plants, dormant in creatures
    // Energy Cost Range: 0.02-0.05 (infrastructure investment)
    // ============================================================================
    
    // Photosynthesis - energy from light efficiency
    // [0.0, 1.0], creep 0.05
    // Cost: 0.04 - chloroplast maintenance
    GeneDefinition photosynthesis(PHOTOSYNTHESIS, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    photosynthesis.addEffect(EffectBinding("metabolism", "photosynthesis_efficiency", EffectType::Direct, 1.0f));
    photosynthesis.setMaintenanceCost(0.04f);
    photosynthesis.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(photosynthesis));
    
    // Root Depth - underground resource access
    // [0.0, 10.0], creep 0.5
    // Cost: 0.05 - deep roots = more structural tissue
    GeneDefinition rootDepth(ROOT_DEPTH, ChromosomeType::Morphology,
        GeneLimits(0.0f, 10.0f, 0.5f), DominanceType::Incomplete);
    rootDepth.addEffect(EffectBinding("morphology", "root_depth", EffectType::Direct, 1.0f));
    rootDepth.setMaintenanceCost(0.05f);
    rootDepth.setCostScaling(1.3f);  // Deep roots are expensive
    registry.tryRegisterGene(std::move(rootDepth));
    
    // Water Storage - internal water capacity
    // [0.0, 1.0], creep 0.05
    // Cost: 0.03 - water storage organs (succulence)
    GeneDefinition waterStorage(WATER_STORAGE, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    waterStorage.addEffect(EffectBinding("metabolism", "water_storage_capacity", EffectType::Direct, 1.0f));
    waterStorage.setMaintenanceCost(0.03f);
    waterStorage.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(waterStorage));
    
    // Light Requirement - how much light is needed
    // [0.0, 1.0], creep 0.05
    // Cost: 0.02 - light sensing mechanisms
    GeneDefinition lightReq(LIGHT_REQUIREMENT, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    lightReq.addEffect(EffectBinding("metabolism", "light_requirement", EffectType::Direct, 1.0f));
    lightReq.setMaintenanceCost(0.02f);
    lightReq.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(lightReq));
    
    // Water Requirement - how much water is needed
    // [0.0, 1.0], creep 0.05
    // Cost: 0.02 - water regulation systems
    GeneDefinition waterReq(WATER_REQUIREMENT, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    waterReq.addEffect(EffectBinding("metabolism", "water_requirement", EffectType::Direct, 1.0f));
    waterReq.setMaintenanceCost(0.02f);
    waterReq.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(waterReq));
}

void UniversalGenes::registerHeterotrophyGenes(GeneRegistry& registry) {
    // ============================================================================
    // HETEROTROPHY GENES - Original set, active in creatures, dormant in plants
    // Energy Cost Range: 0.05-0.15 (digestive systems, moderate cost)
    // ============================================================================
    
    // Hunt Instinct - predation drive
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - predatory behavior requires vigilance
    GeneDefinition huntInstinct(HUNT_INSTINCT, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    huntInstinct.addEffect(EffectBinding("behavior", "hunting_drive", EffectType::Direct, 1.0f));
    huntInstinct.setMaintenanceCost(0.05f);
    huntInstinct.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(huntInstinct));
    
    // Hunger Threshold - when organism seeks food
    // [0.0, 10.0], creep 0.05
    // Cost: 0.02 - hunger sensing is low cost
    GeneDefinition hungerThreshold(HUNGER_THRESHOLD, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 10.0f, 0.05f), DominanceType::Incomplete);
    hungerThreshold.addEffect(EffectBinding("metabolism", "hunger_threshold", EffectType::Direct, 1.0f));
    hungerThreshold.setMaintenanceCost(0.02f);
    hungerThreshold.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(hungerThreshold));
    
    // Thirst Threshold - when organism seeks water
    // [0.0, 10.0], creep 0.05
    // Cost: 0.02 - thirst sensing is low cost
    GeneDefinition thirstThreshold(THIRST_THRESHOLD, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 10.0f, 0.05f), DominanceType::Incomplete);
    thirstThreshold.addEffect(EffectBinding("metabolism", "thirst_threshold", EffectType::Direct, 1.0f));
    thirstThreshold.setMaintenanceCost(0.02f);
    thirstThreshold.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(thirstThreshold));
    
    // Diet Type - DEPRECATED: what the organism eats (discrete enum)
    // [0, 3], creep 0 (discrete values)
    // Note: This gene is deprecated. Use emergent diet calculation instead.
    // Cost: 0.01 - minimal cost for deprecated gene
    GeneDefinition dietType(DIET_TYPE, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 3.0f, 0.0f), DominanceType::Complete);
    dietType.addEffect(EffectBinding("metabolism", "diet_type", EffectType::Direct, 1.0f));
    dietType.setMaintenanceCost(0.01f);
    dietType.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(dietType));
    
    // Digestive Efficiency - how well food is converted to energy
    // [0.1, 1.0], creep 0.05
    // Cost: 0.08 - general digestive system maintenance
    GeneDefinition digestiveEff(DIGESTIVE_EFFICIENCY, ChromosomeType::Metabolism,
        GeneLimits(0.1f, 1.0f, 0.05f), DominanceType::Incomplete);
    digestiveEff.addEffect(EffectBinding("metabolism", "digestive_efficiency", EffectType::Direct, 1.0f));
    digestiveEff.setMaintenanceCost(0.08f);
    digestiveEff.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(digestiveEff));
    
    // Nutrient Value - how nutritious when eaten
    // [1.0, 100.0], creep 5.0
    // Cost: 0.02 - passive trait (how much you're worth to predators)
    GeneDefinition nutrientValue(NUTRIENT_VALUE, ChromosomeType::Metabolism,
        GeneLimits(1.0f, 100.0f, 5.0f), DominanceType::Incomplete);
    nutrientValue.addEffect(EffectBinding("metabolism", "nutrient_value", EffectType::Direct, 1.0f));
    nutrientValue.setMaintenanceCost(0.02f);
    nutrientValue.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(nutrientValue));
}

void UniversalGenes::registerCoevolutionHeterotrophyGenes(GeneRegistry& registry) {
    // ============================================================================
    // HETEROTROPHY GENES - New for coevolution (7 genes)
    // These genes enable emergent diet types through specialization
    // Energy Cost Range: 0.08-0.15 (enzyme systems, active metabolic)
    // ============================================================================
    
    // Plant Digestion Efficiency - extract calories from plants
    // [0.0, 1.0], creep 0.05
    // PLEIOTROPY: High plant digestion inhibits meat digestion
    // Cost: 0.10 - plant enzyme system maintenance
    GeneDefinition plantDigest(PLANT_DIGESTION_EFFICIENCY, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    plantDigest.addEffect(EffectBinding("metabolism", "plant_digestion_efficiency", EffectType::Direct, 1.0f));
    // Pleiotropy: reduces meat digestion (50% inverse effect)
    plantDigest.addEffect(EffectBinding("metabolism", "meat_digestion_efficiency", EffectType::Additive, -0.5f));
    plantDigest.setMaintenanceCost(0.10f);
    plantDigest.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(plantDigest));
    
    // Meat Digestion Efficiency - extract calories from meat
    // [0.0, 1.0], creep 0.05
    // PLEIOTROPY: High meat digestion inhibits plant digestion
    // Cost: 0.10 - meat enzyme system maintenance
    GeneDefinition meatDigest(MEAT_DIGESTION_EFFICIENCY, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    meatDigest.addEffect(EffectBinding("metabolism", "meat_digestion_efficiency", EffectType::Direct, 1.0f));
    // Pleiotropy: reduces plant digestion (50% inverse effect)
    meatDigest.addEffect(EffectBinding("metabolism", "plant_digestion_efficiency", EffectType::Additive, -0.5f));
    meatDigest.setMaintenanceCost(0.10f);
    meatDigest.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(meatDigest));
    
    // Cellulose Breakdown - digest tough plant fibers
    // [0.0, 1.0], creep 0.05
    // PLEIOTROPY: Cellulose breakdown inhibits stomach acidity (needs neutral pH)
    // Cost: 0.15 - cellulose digestion requires gut symbionts (expensive!)
    GeneDefinition cellulose(CELLULOSE_BREAKDOWN, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    cellulose.addEffect(EffectBinding("metabolism", "cellulose_breakdown", EffectType::Direct, 1.0f));
    // Pleiotropy: reduces stomach acidity (50% inverse effect)
    cellulose.addEffect(EffectBinding("morphology", "stomach_acidity", EffectType::Additive, -0.5f));
    cellulose.setMaintenanceCost(0.15f);
    cellulose.setCostScaling(1.5f);  // Very expensive to maintain gut flora
    registry.tryRegisterGene(std::move(cellulose));
    
    // Toxin Tolerance - resist plant toxins
    // [0.0, 1.0], creep 0.05
    // Cost: 0.08 - stress response proteins
    GeneDefinition toxinTol(TOXIN_TOLERANCE, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    toxinTol.addEffect(EffectBinding("metabolism", "toxin_tolerance", EffectType::Direct, 1.0f));
    toxinTol.setMaintenanceCost(0.08f);
    toxinTol.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(toxinTol));
    
    // Toxin Metabolism - detoxify plant compounds
    // [0.0, 1.0], creep 0.05
    // Cost: 0.12 - detoxification enzymes (P450s) are very costly
    GeneDefinition toxinMet(TOXIN_METABOLISM, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    toxinMet.addEffect(EffectBinding("metabolism", "toxin_metabolism", EffectType::Direct, 1.0f));
    toxinMet.setMaintenanceCost(0.12f);
    toxinMet.setCostScaling(1.3f);  // Expensive detox enzymes
    registry.tryRegisterGene(std::move(toxinMet));
    
    // Scent Detection - smell food from distance
    // [0.0, 1.0], creep 0.05
    // Cost: 0.08 - olfactory neural maintenance
    GeneDefinition scent(SCENT_DETECTION, ChromosomeType::Sensory,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    scent.addEffect(EffectBinding("sensory", "scent_detection", EffectType::Direct, 1.0f));
    scent.setMaintenanceCost(0.08f);
    scent.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(scent));
    
    // Color Vision - detect ripe fruit
    // [0.0, 1.0], creep 0.05
    // Cost: 0.10 - color vision requires specialized photoreceptors
    GeneDefinition colorVis(COLOR_VISION, ChromosomeType::Sensory,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    colorVis.addEffect(EffectBinding("sensory", "color_vision", EffectType::Direct, 1.0f));
    colorVis.setMaintenanceCost(0.10f);
    colorVis.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(colorVis));
}

void UniversalGenes::registerMorphologyGenes(GeneRegistry& registry) {
    // ============================================================================
    // MORPHOLOGY GENES - New anatomical trade-off genes (9 genes)
    // Energy Cost Range: 0.01-0.05 (passive structural traits)
    // ============================================================================
    
    // Gut Length - relative gut length
    // [0.3, 1.0], creep 0.05
    // PLEIOTROPY: Long gut reduces movement speed (more mass to carry)
    // Cost: 0.04 - structural maintenance
    GeneDefinition gutLen(GUT_LENGTH, ChromosomeType::Morphology,
        GeneLimits(0.3f, 1.0f, 0.05f), DominanceType::Incomplete);
    gutLen.addEffect(EffectBinding("morphology", "gut_length", EffectType::Direct, 1.0f));
    // Pleiotropy: reduces locomotion speed (30% inverse effect)
    gutLen.addEffect(EffectBinding("locomotion", "speed_multiplier", EffectType::Additive, -0.3f));
    // Diet contribution: longer gut helps plant digestion (+35% contribution)
    gutLen.addEffect(EffectBinding("metabolism", "plant_digestion_efficiency", EffectType::Additive, 0.35f));
    gutLen.setMaintenanceCost(0.04f);
    gutLen.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(gutLen));
    
    // Tooth Sharpness - sharp teeth for meat
    // [0.0, 1.0], creep 0.05
    // PLEIOTROPY: Sharp teeth inhibit grinding ability
    // Cost: 0.02 - enamel maintenance
    GeneDefinition toothSharp(TOOTH_SHARPNESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    toothSharp.addEffect(EffectBinding("morphology", "tooth_sharpness", EffectType::Direct, 1.0f));
    // Pleiotropy: reduces tooth grinding (50% inverse effect)
    toothSharp.addEffect(EffectBinding("morphology", "tooth_grinding", EffectType::Additive, -0.5f));
    // Diet contribution: sharp teeth help meat processing (+15% contribution)
    toothSharp.addEffect(EffectBinding("metabolism", "meat_digestion_efficiency", EffectType::Additive, 0.15f));
    toothSharp.setMaintenanceCost(0.02f);
    toothSharp.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(toothSharp));
    
    // Tooth Grinding - flat teeth for plants
    // [0.0, 1.0], creep 0.05
    // PLEIOTROPY: Grinding teeth inhibit sharpness
    // Cost: 0.02 - enamel maintenance
    GeneDefinition toothGrind(TOOTH_GRINDING, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    toothGrind.addEffect(EffectBinding("morphology", "tooth_grinding", EffectType::Direct, 1.0f));
    // Pleiotropy: reduces tooth sharpness (50% inverse effect)
    toothGrind.addEffect(EffectBinding("morphology", "tooth_sharpness", EffectType::Additive, -0.5f));
    // Diet contribution: grinding teeth help plant processing (+15% contribution)
    toothGrind.addEffect(EffectBinding("metabolism", "plant_digestion_efficiency", EffectType::Additive, 0.15f));
    toothGrind.setMaintenanceCost(0.02f);
    toothGrind.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(toothGrind));
    
    // Stomach Acidity - acidic for meat digestion
    // [0.0, 1.0], creep 0.05
    // PLEIOTROPY: High acidity inhibits cellulose breakdown
    // Cost: 0.05 - acid gland maintenance
    GeneDefinition stomachAcid(STOMACH_ACIDITY, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    stomachAcid.addEffect(EffectBinding("morphology", "stomach_acidity", EffectType::Direct, 1.0f));
    // Pleiotropy: reduces cellulose breakdown (50% inverse effect)
    stomachAcid.addEffect(EffectBinding("metabolism", "cellulose_breakdown", EffectType::Additive, -0.5f));
    // Diet contribution: acidic stomach helps meat digestion (+35% contribution)
    stomachAcid.addEffect(EffectBinding("metabolism", "meat_digestion_efficiency", EffectType::Additive, 0.35f));
    stomachAcid.setMaintenanceCost(0.05f);
    stomachAcid.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(stomachAcid));
    
    // Jaw Strength - crack hard nuts/shells
    // [0.0, 1.0], creep 0.05
    // Cost: 0.03 - muscle mass maintenance
    GeneDefinition jawStr(JAW_STRENGTH, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    jawStr.addEffect(EffectBinding("morphology", "jaw_strength", EffectType::Direct, 1.0f));
    jawStr.setMaintenanceCost(0.03f);
    jawStr.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(jawStr));
    
    // Jaw Speed - fast snapping
    // [0.0, 1.0], creep 0.05
    // Cost: 0.03 - fast-twitch muscle maintenance
    GeneDefinition jawSpd(JAW_SPEED, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    jawSpd.addEffect(EffectBinding("morphology", "jaw_speed", EffectType::Direct, 1.0f));
    jawSpd.setMaintenanceCost(0.03f);
    jawSpd.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(jawSpd));
    
    // Hide Thickness - thorn resistance
    // [0.0, 1.0], creep 0.05
    // Cost: 0.03 - skin/hide maintenance
    GeneDefinition hide(HIDE_THICKNESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    hide.addEffect(EffectBinding("morphology", "hide_thickness", EffectType::Direct, 1.0f));
    hide.setMaintenanceCost(0.03f);
    hide.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(hide));
    
    // Fur Density - burr attachment
    // [0.0, 1.0], creep 0.05
    // Cost: 0.02 - fur growth/maintenance
    GeneDefinition fur(FUR_DENSITY, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    fur.addEffect(EffectBinding("morphology", "fur_density", EffectType::Direct, 1.0f));
    fur.setMaintenanceCost(0.02f);
    fur.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(fur));
    
    // Mucus Protection - gut lining protection
    // [0.0, 1.0], creep 0.05
    // Cost: 0.03 - mucus secretion
    GeneDefinition mucus(MUCUS_PROTECTION, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    mucus.addEffect(EffectBinding("morphology", "mucus_protection", EffectType::Direct, 1.0f));
    mucus.setMaintenanceCost(0.03f);
    mucus.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(mucus));
}

void UniversalGenes::registerBehaviorGenes(GeneRegistry& registry) {
    // ============================================================================
    // BEHAVIOR GENES - New behavioral traits (5 genes)
    // Energy Cost Range: 0.02-0.05 (neural/behavioral patterns)
    // ============================================================================
    
    // Sweetness Preference - attraction to fruit
    // [0.0, 1.0], creep 0.05
    // Cost: 0.02 - taste receptor maintenance
    GeneDefinition sweetness(SWEETNESS_PREFERENCE, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    sweetness.addEffect(EffectBinding("behavior", "sweetness_preference", EffectType::Direct, 1.0f));
    sweetness.setMaintenanceCost(0.02f);
    sweetness.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(sweetness));
    
    // Caching Instinct - squirrel-like hoarding
    // [0.0, 1.0], creep 0.05
    // Cost: 0.03 - behavioral pattern maintenance
    GeneDefinition caching(CACHING_INSTINCT, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    caching.addEffect(EffectBinding("behavior", "caching_instinct", EffectType::Direct, 1.0f));
    caching.setMaintenanceCost(0.03f);
    caching.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(caching));
    
    // Spatial Memory - remember locations
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - memory requires brain tissue (expensive)
    GeneDefinition spatial(SPATIAL_MEMORY, ChromosomeType::Sensory,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    spatial.addEffect(EffectBinding("sensory", "spatial_memory", EffectType::Direct, 1.0f));
    spatial.setMaintenanceCost(0.05f);
    spatial.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(spatial));
    
    // Grooming Frequency - remove burrs
    // [0.0, 1.0], creep 0.05
    // Cost: 0.02 - behavioral pattern
    GeneDefinition grooming(GROOMING_FREQUENCY, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    grooming.addEffect(EffectBinding("behavior", "grooming_frequency", EffectType::Direct, 1.0f));
    grooming.setMaintenanceCost(0.02f);
    grooming.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(grooming));
    
    // Pain Sensitivity - tolerance level
    // [0.0, 1.0], creep 0.05
    // Cost: 0.02 - nociceptor maintenance
    GeneDefinition pain(PAIN_SENSITIVITY, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    pain.addEffect(EffectBinding("behavior", "pain_sensitivity", EffectType::Direct, 1.0f));
    pain.setMaintenanceCost(0.02f);
    pain.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(pain));
}

void UniversalGenes::registerSeedInteractionGenes(GeneRegistry& registry) {
    // ============================================================================
    // SEED INTERACTION GENES (2 genes)
    // Energy Cost Range: 0.02-0.03 (digestive timing/processing)
    // ============================================================================
    
    // Gut Transit Time - hours for seed passage
    // [0.5, 4.0], creep 0.2
    // Cost: 0.03 - gut motility regulation
    GeneDefinition transit(GUT_TRANSIT_TIME, ChromosomeType::Metabolism,
        GeneLimits(0.5f, 4.0f, 0.2f), DominanceType::Incomplete);
    transit.addEffect(EffectBinding("metabolism", "gut_transit_time", EffectType::Direct, 1.0f));
    transit.setMaintenanceCost(0.03f);
    transit.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(transit));
    
    // Seed Destruction Rate - grind vs pass seeds
    // [0.0, 1.0], creep 0.05
    // Cost: 0.02 - grinding mechanism
    GeneDefinition seedDestroy(SEED_DESTRUCTION_RATE, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    seedDestroy.addEffect(EffectBinding("metabolism", "seed_destruction_rate", EffectType::Direct, 1.0f));
    seedDestroy.setMaintenanceCost(0.02f);
    seedDestroy.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(seedDestroy));
}

void UniversalGenes::registerPlantDefenseGenes(GeneRegistry& registry) {
    // ============================================================================
    // PLANT DEFENSE GENES (6 genes)
    // Energy Cost Range: 0.10-0.20 (chemical production, high cost)
    // ============================================================================
    
    // Toxin Production - chemical defenses
    // [0.0, 1.0], creep 0.05
    // Cost: 0.18 - alkaloids/glycosides are metabolically expensive
    GeneDefinition toxinProd(TOXIN_PRODUCTION, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    toxinProd.addEffect(EffectBinding("metabolism", "toxin_production", EffectType::Direct, 1.0f));
    toxinProd.setMaintenanceCost(0.18f);
    toxinProd.setCostScaling(1.4f);  // High toxin levels very expensive
    registry.tryRegisterGene(std::move(toxinProd));
    
    // Thorn Density - physical defenses
    // [0.0, 1.0], creep 0.05
    // Cost: 0.12 - physical defenses use biomass
    GeneDefinition thorns(THORN_DENSITY, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    thorns.addEffect(EffectBinding("morphology", "thorn_density", EffectType::Direct, 1.0f));
    thorns.setMaintenanceCost(0.12f);
    thorns.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(thorns));
    
    // Regrowth Rate - recovery after grazing
    // [0.0, 1.0], creep 0.05
    // Cost: 0.10 - fast regrowth requires nutrient reserves
    GeneDefinition regrowth(REGROWTH_RATE, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    regrowth.addEffect(EffectBinding("metabolism", "regrowth_rate", EffectType::Direct, 1.0f));
    regrowth.setMaintenanceCost(0.10f);
    regrowth.setCostScaling(1.3f);
    registry.tryRegisterGene(std::move(regrowth));
    
    // Fruit Production Rate - food object spawning
    // [0.0, 1.0], creep 0.05
    // Cost: 0.08 - sugar investment in dispersal
    GeneDefinition fruitProd(FRUIT_PRODUCTION_RATE, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    fruitProd.addEffect(EffectBinding("reproduction", "fruit_production_rate", EffectType::Direct, 1.0f));
    fruitProd.setMaintenanceCost(0.08f);
    fruitProd.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(fruitProd));
    
    // Seed Coat Durability - survives digestion
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - tougher seeds = more resources per seed
    GeneDefinition seedCoat(SEED_COAT_DURABILITY, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    seedCoat.addEffect(EffectBinding("reproduction", "seed_coat_durability", EffectType::Direct, 1.0f));
    seedCoat.setMaintenanceCost(0.05f);
    seedCoat.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(seedCoat));
    
    // Fruit Appeal - attracts dispersers
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - attractive coloring/scent
    GeneDefinition fruitAppeal(FRUIT_APPEAL, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    fruitAppeal.addEffect(EffectBinding("reproduction", "fruit_appeal", EffectType::Direct, 1.0f));
    fruitAppeal.setMaintenanceCost(0.06f);
    fruitAppeal.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(fruitAppeal));
}

void UniversalGenes::registerReproductionGenes(GeneRegistry& registry) {
    // ============================================================================
    // REPRODUCTION GENES - All organisms, different expression
    // Energy Cost Range: 0.02-0.06 (reproductive system maintenance)
    // ============================================================================
    
    // Offspring Count - number of offspring per reproduction
    // [1, 20], creep 1
    // Cost: 0.05 - more offspring capacity = more reproductive tissue
    GeneDefinition offspringCount(OFFSPRING_COUNT, ChromosomeType::Reproduction,
        GeneLimits(1.0f, 20.0f, 1.0f), DominanceType::Incomplete);
    offspringCount.addEffect(EffectBinding("reproduction", "offspring_count", EffectType::Direct, 1.0f));
    offspringCount.setMaintenanceCost(0.05f);
    offspringCount.setCostScaling(1.2f);  // High offspring count is expensive
    registry.tryRegisterGene(std::move(offspringCount));
    
    // Mate Threshold - when organism seeks mate
    // [0.0, 10.0], creep 0.05
    // Cost: 0.02 - mate seeking behavior
    GeneDefinition mateThreshold(MATE_THRESHOLD, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 10.0f, 0.05f), DominanceType::Incomplete);
    mateThreshold.addEffect(EffectBinding("reproduction", "mate_threshold", EffectType::Direct, 1.0f));
    mateThreshold.setMaintenanceCost(0.02f);
    mateThreshold.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(mateThreshold));
    
    // Spread Distance - how far offspring/seeds spread
    // [0.0, 30.0], creep 2.0
    // Cost: 0.03 - dispersal mechanisms
    GeneDefinition spreadDist(SPREAD_DISTANCE, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 30.0f, 2.0f), DominanceType::Incomplete);
    spreadDist.addEffect(EffectBinding("reproduction", "dispersal_range", EffectType::Direct, 1.0f));
    spreadDist.setMaintenanceCost(0.03f);
    spreadDist.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(spreadDist));
    
    // Fatigue Threshold - when organism rests
    // [0.0, 10.0], creep 0.05
    // Cost: 0.02 - fatigue sensing/management
    GeneDefinition fatigueThreshold(FATIGUE_THRESHOLD, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 10.0f, 0.05f), DominanceType::Incomplete);
    fatigueThreshold.addEffect(EffectBinding("reproduction", "fatigue_threshold", EffectType::Direct, 1.0f));
    fatigueThreshold.setMaintenanceCost(0.02f);
    fatigueThreshold.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(fatigueThreshold));
    
    // Comfort Increase - how quickly comfort rises
    // [0.001, 0.02], creep 0.002
    // Cost: 0.02 - recovery mechanisms
    GeneDefinition comfortInc(COMFORT_INCREASE, ChromosomeType::Reproduction,
        GeneLimits(0.001f, 0.02f, 0.002f), DominanceType::Incomplete);
    comfortInc.addEffect(EffectBinding("reproduction", "comfort_increase_rate", EffectType::Direct, 1.0f));
    comfortInc.setMaintenanceCost(0.02f);
    comfortInc.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(comfortInc));
    
    // Comfort Decrease - how quickly comfort falls
    // [0.001, 0.02], creep 0.002
    // Cost: 0.01 - passive trait (low cost)
    GeneDefinition comfortDec(COMFORT_DECREASE, ChromosomeType::Reproduction,
        GeneLimits(0.001f, 0.02f, 0.002f), DominanceType::Incomplete);
    comfortDec.addEffect(EffectBinding("reproduction", "comfort_decrease_rate", EffectType::Direct, 1.0f));
    comfortDec.setMaintenanceCost(0.01f);
    comfortDec.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(comfortDec));
}

void UniversalGenes::registerSeedPropagationGenes(GeneRegistry& registry) {
    // ============================================================================
    // SEED PROPAGATION GENES (5 genes)
    // Physical properties that determine emergent dispersal strategy
    // Energy Cost Range: 0.03-0.08 (seed production structures)
    // ============================================================================
    
    // Seed Mass - affects dispersal potential
    // [0.01, 1.0], creep 0.05
    // Low mass enables wind dispersal, high mass for gravity/animal
    // PLEIOTROPY: Higher mass seeds have better germination success
    // Cost: 0.04 - nutrient storage in seeds
    GeneDefinition seedMass(SEED_MASS, ChromosomeType::Reproduction,
        GeneLimits(0.01f, 1.0f, 0.05f), DominanceType::Incomplete);
    seedMass.addEffect(EffectBinding("reproduction", "seed_mass", EffectType::Direct, 1.0f));
    // Pleiotropy: heavier seeds = better germination (30% positive effect)
    seedMass.addEffect(EffectBinding("reproduction", "germination_success", EffectType::Additive, 0.3f));
    seedMass.setMaintenanceCost(0.04f);
    seedMass.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(seedMass));
    
    // Seed Aerodynamics - surface area to mass ratio
    // [0.0, 1.0], creep 0.05
    // High values indicate wings/parachutes for wind dispersal
    // Cost: 0.03 - specialized appendage development
    GeneDefinition seedAero(SEED_AERODYNAMICS, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    seedAero.addEffect(EffectBinding("reproduction", "seed_aerodynamics", EffectType::Direct, 1.0f));
    seedAero.setMaintenanceCost(0.03f);
    seedAero.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(seedAero));
    
    // Seed Hook Strength - mechanical hook/barb development
    // [0.0, 1.0], creep 0.05
    // High values enable attachment to animal fur (burr dispersal)
    // Cost: 0.04 - specialized structural proteins
    GeneDefinition seedHook(SEED_HOOK_STRENGTH, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    seedHook.addEffect(EffectBinding("reproduction", "seed_hook_strength", EffectType::Direct, 1.0f));
    seedHook.setMaintenanceCost(0.04f);
    seedHook.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(seedHook));
    
    // Explosive Pod Force - ballistic dispersal mechanism
    // [0.0, 1.0], creep 0.05 (normalized from 0-15N physical range)
    // High values enable explosive seed launching
    // Cost: 0.08 - tension structures are energetically expensive
    GeneDefinition explosivePod(EXPLOSIVE_POD_FORCE, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    explosivePod.addEffect(EffectBinding("reproduction", "explosive_pod_force", EffectType::Direct, 1.0f));
    explosivePod.setMaintenanceCost(0.08f);
    explosivePod.setCostScaling(1.3f);  // Expensive mechanism
    registry.tryRegisterGene(std::move(explosivePod));
    
    // Runner Production - vegetative spread rate
    // [0.0, 2.0], creep 0.1
    // High values indicate clonal spreading strategy (stolons/runners)
    // PLEIOTROPY: High runner production reduces fruit production
    // Cost: 0.06 - continuous vegetative growth
    GeneDefinition runnerProd(RUNNER_PRODUCTION, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 2.0f, 0.1f), DominanceType::Incomplete);
    runnerProd.addEffect(EffectBinding("reproduction", "runner_production", EffectType::Direct, 1.0f));
    // Pleiotropy: runners compete with fruit for resources (20% inverse effect)
    runnerProd.addEffect(EffectBinding("reproduction", "fruit_production_rate", EffectType::Additive, -0.2f));
    runnerProd.setMaintenanceCost(0.06f);
    runnerProd.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(runnerProd));
}

void UniversalGenes::registerOlfactoryGenes(GeneRegistry& registry) {
    // ============================================================================
    // OLFACTORY SYSTEM GENES - Phase 1: Sensory System (4 genes)
    // These genes enable scent-based communication and mate finding
    // Energy Cost Range: 0.02-0.08 (scent glands and neural processing)
    // ============================================================================
    
    // Scent Production - rate of general scent emission
    // [0.0, 1.0], creep 0.05
    // Higher values = stronger pheromone output, more detectable
    // Cost: 0.05 - scent glands require energy to produce pheromones
    GeneDefinition scentProd(SCENT_PRODUCTION, ChromosomeType::Sensory,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    scentProd.addEffect(EffectBinding("sensory", "scent_production", EffectType::Direct, 1.0f));
    scentProd.setMaintenanceCost(0.05f);
    scentProd.setCostScaling(1.2f);  // Higher scent = more energy for glands
    registry.tryRegisterGene(std::move(scentProd));
    
    // Scent Signature Variance - how unique the genetic fingerprint is
    // [0.0, 1.0], creep 0.05
    // Higher values = more unique scent signature, easier to identify individual
    // Cost: 0.02 - passive trait related to pheromone composition
    GeneDefinition scentSigVar(SCENT_SIGNATURE_VARIANCE, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    scentSigVar.addEffect(EffectBinding("reproduction", "scent_signature_variance", EffectType::Direct, 1.0f));
    scentSigVar.setMaintenanceCost(0.02f);
    scentSigVar.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(scentSigVar));
    
    // Olfactory Acuity - ability to detect and distinguish scents
    // [0.0, 1.0], creep 0.05
    // Higher values = better scent detection range and discrimination
    // Cost: 0.08 - requires specialized olfactory neurons
    GeneDefinition olfAcuity(OLFACTORY_ACUITY, ChromosomeType::Sensory,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    olfAcuity.addEffect(EffectBinding("sensory", "olfactory_acuity", EffectType::Direct, 1.0f));
    olfAcuity.setMaintenanceCost(0.08f);
    olfAcuity.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(olfAcuity));
    
    // Scent Masking - ability to reduce own scent detectability
    // [0.0, 1.0], creep 0.05
    // Higher values = harder to detect by predators or rivals
    // Cost: 0.06 - requires active suppression of scent glands
    GeneDefinition scentMask(SCENT_MASKING, ChromosomeType::Sensory,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    scentMask.addEffect(EffectBinding("sensory", "scent_masking", EffectType::Direct, 1.0f));
    scentMask.setMaintenanceCost(0.06f);
    scentMask.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(scentMask));
}

void UniversalGenes::registerHealthHealingGenes(GeneRegistry& registry) {
    // ============================================================================
    // HEALTH/HEALING GENES (3 genes)
    // These genes control wound recovery and health management
    // Energy Cost Range: 0.05-0.10 (regenerative systems)
    // ============================================================================
    
    // Regeneration Rate - multiplier on base healing rate
    // [0.0, 2.0], creep 0.1
    // Higher values = faster wound recovery, but costs more energy during healing
    // Cost: 0.10 - regenerative tissue maintenance is expensive
    GeneDefinition regenRate(REGENERATION_RATE, ChromosomeType::Morphology,
        GeneLimits(0.0f, 2.0f, 0.1f), DominanceType::Incomplete);
    regenRate.addEffect(EffectBinding("morphology", "regeneration_rate", EffectType::Direct, 1.0f));
    regenRate.setMaintenanceCost(0.10f);
    regenRate.setCostScaling(1.3f);  // Fast healing is expensive
    registry.tryRegisterGene(std::move(regenRate));
    
    // Wound Tolerance - reduces behavioral penalty from wounds
    // [0.0, 1.0], creep 0.05
    // Higher values = less speed/behavior penalty when injured
    // Cost: 0.05 - pain management systems
    GeneDefinition woundTol(WOUND_TOLERANCE, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    woundTol.addEffect(EffectBinding("behavior", "wound_tolerance", EffectType::Direct, 1.0f));
    woundTol.setMaintenanceCost(0.05f);
    woundTol.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(woundTol));
    
    // Bleeding Resistance - reduces bleed damage over time
    // [0.0, 1.0], creep 0.05
    // Higher values = wounds stop bleeding faster
    // Cost: 0.08 - coagulation factor production
    GeneDefinition bleedRes(BLEEDING_RESISTANCE, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    bleedRes.addEffect(EffectBinding("morphology", "bleeding_resistance", EffectType::Direct, 1.0f));
    bleedRes.setMaintenanceCost(0.08f);
    bleedRes.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(bleedRes));
}

void UniversalGenes::registerWeaponShapeGenes(GeneRegistry& registry) {
    // ============================================================================
    // WEAPON SHAPE GENES (13 genes) - Phase 1c Combat
    // Physical weapon morphology affecting combat damage types
    // Energy Cost Range: 0.05-0.10 (weapons cost energy to maintain)
    // ============================================================================
    
    // ========== TEETH (3 genes) ==========
    
    // Teeth Sharpness - determines damage type (sharp→pierce, dull→blunt)
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - enamel maintenance for sharp points
    GeneDefinition teethSharp(TEETH_SHARPNESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    teethSharp.addEffect(EffectBinding("morphology", "teeth_sharpness", EffectType::Direct, 1.0f));
    teethSharp.setMaintenanceCost(0.05f);
    teethSharp.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(teethSharp));
    
    // Teeth Serration - adds slash damage component
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - serrated edges require more maintenance
    GeneDefinition teethSerr(TEETH_SERRATION, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    teethSerr.addEffect(EffectBinding("morphology", "teeth_serration", EffectType::Direct, 1.0f));
    teethSerr.setMaintenanceCost(0.06f);
    teethSerr.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(teethSerr));
    
    // Teeth Size - scales base bite damage
    // [0.0, 1.0], creep 0.05
    // Cost: 0.07 - larger teeth require more calcium/resources
    GeneDefinition teethSize(TEETH_SIZE, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    teethSize.addEffect(EffectBinding("morphology", "teeth_size", EffectType::Direct, 1.0f));
    teethSize.setMaintenanceCost(0.07f);
    teethSize.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(teethSize));
    
    // ========== CLAWS (3 genes) ==========
    
    // Claw Length - reach and damage scaling
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - keratin production for long claws
    GeneDefinition clawLen(CLAW_LENGTH, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    clawLen.addEffect(EffectBinding("morphology", "claw_length", EffectType::Direct, 1.0f));
    clawLen.setMaintenanceCost(0.06f);
    clawLen.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(clawLen));
    
    // Claw Curvature - determines damage type (curved→pierce, straight→slash)
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - structural maintenance
    GeneDefinition clawCurv(CLAW_CURVATURE, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    clawCurv.addEffect(EffectBinding("morphology", "claw_curvature", EffectType::Direct, 1.0f));
    clawCurv.setMaintenanceCost(0.05f);
    clawCurv.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(clawCurv));
    
    // Claw Sharpness - overall cutting ability
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - keeping claws sharp requires energy
    GeneDefinition clawSharp(CLAW_SHARPNESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    clawSharp.addEffect(EffectBinding("morphology", "claw_sharpness", EffectType::Direct, 1.0f));
    clawSharp.setMaintenanceCost(0.06f);
    clawSharp.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(clawSharp));
    
    // ========== HORNS (3 genes) ==========
    
    // Horn Length - reach and charge damage
    // [0.0, 1.0], creep 0.05
    // Cost: 0.08 - horns are bone/keratin intensive structures
    GeneDefinition hornLen(HORN_LENGTH, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    hornLen.addEffect(EffectBinding("morphology", "horn_length", EffectType::Direct, 1.0f));
    hornLen.setMaintenanceCost(0.08f);
    hornLen.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(hornLen));
    
    // Horn Pointiness - determines damage type (pointed→pierce, broad→blunt)
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - point maintenance
    GeneDefinition hornPoint(HORN_POINTINESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    hornPoint.addEffect(EffectBinding("morphology", "horn_pointiness", EffectType::Direct, 1.0f));
    hornPoint.setMaintenanceCost(0.05f);
    hornPoint.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(hornPoint));
    
    // Horn Spread - attack mode (narrow→gore, wide→sweep)
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - wider spread = more structural bone
    GeneDefinition hornSpread(HORN_SPREAD, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    hornSpread.addEffect(EffectBinding("morphology", "horn_spread", EffectType::Direct, 1.0f));
    hornSpread.setMaintenanceCost(0.06f);
    hornSpread.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(hornSpread));
    
    // ========== TAIL (3 genes) ==========
    
    // Tail Length - reach and whip damage
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - longer tails require more vertebrae/muscle
    GeneDefinition tailLen(TAIL_LENGTH, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    tailLen.addEffect(EffectBinding("morphology", "tail_length", EffectType::Direct, 1.0f));
    tailLen.setMaintenanceCost(0.06f);
    tailLen.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(tailLen));
    
    // Tail Mass - determines damage type (heavy→blunt club, light→slash whip)
    // [0.0, 1.0], creep 0.05
    // Cost: 0.07 - heavier tail = more muscle to carry
    GeneDefinition tailMass(TAIL_MASS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    tailMass.addEffect(EffectBinding("morphology", "tail_mass", EffectType::Direct, 1.0f));
    tailMass.setMaintenanceCost(0.07f);
    tailMass.setCostScaling(1.2f);
    registry.tryRegisterGene(std::move(tailMass));
    
    // Tail Spines - adds pierce component
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - spine production/maintenance
    GeneDefinition tailSpine(TAIL_SPINES, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    tailSpine.addEffect(EffectBinding("morphology", "tail_spines", EffectType::Direct, 1.0f));
    tailSpine.setMaintenanceCost(0.06f);
    tailSpine.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(tailSpine));
    
    // ========== BODY (1 gene) ==========
    
    // Body Spines - counter-damage pierce (porcupine-like defense)
    // [0.0, 1.0], creep 0.05
    // Cost: 0.10 - extensive spine coverage is expensive
    GeneDefinition bodySpines(BODY_SPINES, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    bodySpines.addEffect(EffectBinding("morphology", "body_spines", EffectType::Direct, 1.0f));
    bodySpines.setMaintenanceCost(0.10f);
    bodySpines.setCostScaling(1.3f);  // Very expensive at high values
    registry.tryRegisterGene(std::move(bodySpines));
}

void UniversalGenes::registerCombatDefenseGenes(GeneRegistry& registry) {
    // ============================================================================
    // COMBAT DEFENSE GENES (2 new genes) - Phase 1c Combat
    // Note: HIDE_THICKNESS already exists in registerMorphologyGenes
    // Energy Cost Range: 0.05-0.08 (passive defensive structures)
    // ============================================================================
    
    // Scale Coverage - reduces slashing damage
    // [0.0, 1.0], creep 0.05
    // Cost: 0.06 - scale production and maintenance
    GeneDefinition scaleCov(SCALE_COVERAGE, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    scaleCov.addEffect(EffectBinding("morphology", "scale_coverage", EffectType::Direct, 1.0f));
    scaleCov.setMaintenanceCost(0.06f);
    scaleCov.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(scaleCov));
    
    // Fat Layer Thickness - reduces blunt damage
    // [0.0, 1.0], creep 0.05
    // Cost: 0.08 - fat storage requires metabolic investment
    GeneDefinition fatLayer(FAT_LAYER_THICKNESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    fatLayer.addEffect(EffectBinding("morphology", "fat_layer_thickness", EffectType::Direct, 1.0f));
    fatLayer.setMaintenanceCost(0.08f);
    fatLayer.setCostScaling(1.2f);  // Thick fat layers require significant calories
    registry.tryRegisterGene(std::move(fatLayer));
}

void UniversalGenes::registerCombatBehaviorGenes(GeneRegistry& registry) {
    // ============================================================================
    // COMBAT BEHAVIOR GENES (4 genes) - Phase 1c Combat
    // Combat behavioral traits affecting aggression and retreat
    // Energy Cost Range: 0.03-0.05 (neural/behavioral patterns)
    // ============================================================================
    
    // Combat Aggression - willingness to initiate/continue combat
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - aggressive behavior requires more adrenaline/vigilance
    GeneDefinition combatAgg(COMBAT_AGGRESSION, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    combatAgg.addEffect(EffectBinding("behavior", "combat_aggression", EffectType::Direct, 1.0f));
    combatAgg.setMaintenanceCost(0.05f);
    combatAgg.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(combatAgg));
    
    // Retreat Threshold - health % at which creature flees
    // [0.0, 1.0], creep 0.05
    // Cost: 0.03 - self-preservation instinct (low cost)
    GeneDefinition retreatThr(RETREAT_THRESHOLD, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    retreatThr.addEffect(EffectBinding("behavior", "retreat_threshold", EffectType::Direct, 1.0f));
    retreatThr.setMaintenanceCost(0.03f);
    retreatThr.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(retreatThr));
    
    // Territorial Aggression - aggression toward same-species
    // [0.0, 1.0], creep 0.05
    // Cost: 0.04 - territorial marking and defense behavior
    GeneDefinition terrAgg(TERRITORIAL_AGGRESSION, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    terrAgg.addEffect(EffectBinding("behavior", "territorial_aggression", EffectType::Direct, 1.0f));
    terrAgg.setMaintenanceCost(0.04f);
    terrAgg.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(terrAgg));
    
    // Pack Coordination - future: pack hunting behavior
    // [0.0, 1.0], creep 0.05
    // Cost: 0.05 - social cognition requires brain resources
    GeneDefinition packCoord(PACK_COORDINATION, ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    packCoord.addEffect(EffectBinding("behavior", "pack_coordination", EffectType::Direct, 1.0f));
    packCoord.setMaintenanceCost(0.05f);
    packCoord.setCostScaling(1.0f);
    registry.tryRegisterGene(std::move(packCoord));
}

Genome UniversalGenes::createCreatureGenome(const GeneRegistry& registry) {
    Genome genome;
    initializeCategories();
    
    // Default values for creature-like organisms
    // High expression for mobility and heterotrophy, low for autotrophy
    
    // ========== UNIVERSAL GENES - Full expression ==========
    if (registry.hasGene(LIFESPAN)) {
        Allele a(10000.0f, 1.0f);  // value, expression
        Gene gene(LIFESPAN, a, a);
        genome.addGene(gene, ChromosomeType::Lifespan);
    }
    if (registry.hasGene(MAX_SIZE)) {
        Allele a(5.0f, 1.0f);
        Gene gene(MAX_SIZE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(METABOLISM_RATE)) {
        Allele a(1.0f, 1.0f);
        Gene gene(METABOLISM_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(COLOR_HUE)) {
        Allele a(30.0f, 1.0f);  // Orange-ish for creatures
        Gene gene(COLOR_HUE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HARDINESS)) {
        Allele a(0.5f, 1.0f);
        Gene gene(HARDINESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TEMP_TOLERANCE_LOW)) {
        Allele a(5.0f, 1.0f);
        Gene gene(TEMP_TOLERANCE_LOW, a, a);
        genome.addGene(gene, ChromosomeType::Environmental);
    }
    if (registry.hasGene(TEMP_TOLERANCE_HIGH)) {
        Allele a(35.0f, 1.0f);
        Gene gene(TEMP_TOLERANCE_HIGH, a, a);
        genome.addGene(gene, ChromosomeType::Environmental);
    }
    
    // ========== MOBILITY GENES - High expression ==========
    if (registry.hasGene(LOCOMOTION)) {
        Allele a(1.0f, 1.0f);  // Full locomotion capability
        Gene gene(LOCOMOTION, a, a);
        genome.addGene(gene, ChromosomeType::Locomotion);
    }
    if (registry.hasGene(SIGHT_RANGE)) {
        Allele a(50.0f, 1.0f);
        Gene gene(SIGHT_RANGE, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(NAVIGATION_ABILITY)) {
        Allele a(0.7f, 1.0f);
        Gene gene(NAVIGATION_ABILITY, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(FLEE_THRESHOLD)) {
        Allele a(3.0f, 1.0f);
        Gene gene(FLEE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(PURSUE_THRESHOLD)) {
        Allele a(6.0f, 1.0f);
        Gene gene(PURSUE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    
    // ========== AUTOTROPHY GENES - Dormant (low expression) ==========
    if (registry.hasGene(PHOTOSYNTHESIS)) {
        Allele a(0.0f, 0.1f);  // Dormant but can evolve
        Gene gene(PHOTOSYNTHESIS, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(ROOT_DEPTH)) {
        Allele a(0.0f, 0.1f);
        Gene gene(ROOT_DEPTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(WATER_STORAGE)) {
        Allele a(0.1f, 0.1f);
        Gene gene(WATER_STORAGE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(LIGHT_REQUIREMENT)) {
        Allele a(0.0f, 0.1f);
        Gene gene(LIGHT_REQUIREMENT, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(WATER_REQUIREMENT)) {
        Allele a(0.3f, 0.5f);  // Some water needed
        Gene gene(WATER_REQUIREMENT, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    // ========== ORIGINAL HETEROTROPHY GENES - High expression ==========
    if (registry.hasGene(HUNT_INSTINCT)) {
        Allele a(0.5f, 1.0f);
        Gene gene(HUNT_INSTINCT, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(HUNGER_THRESHOLD)) {
        Allele a(5.0f, 1.0f);
        Gene gene(HUNGER_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(THIRST_THRESHOLD)) {
        Allele a(5.0f, 1.0f);
        Gene gene(THIRST_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(DIET_TYPE)) {
        Allele a(1.0f, 1.0f);  // Omnivore (deprecated, kept for backward compatibility)
        Gene gene(DIET_TYPE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(DIGESTIVE_EFFICIENCY)) {
        Allele a(0.7f, 1.0f);
        Gene gene(DIGESTIVE_EFFICIENCY, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(NUTRIENT_VALUE)) {
        Allele a(30.0f, 1.0f);  // Moderate nutritional value
        Gene gene(NUTRIENT_VALUE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    // ========== NEW HETEROTROPHY GENES - Balanced omnivore defaults ==========
    if (registry.hasGene(PLANT_DIGESTION_EFFICIENCY)) {
        Allele a(0.5f, 1.0f);  // Balanced omnivore
        Gene gene(PLANT_DIGESTION_EFFICIENCY, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(MEAT_DIGESTION_EFFICIENCY)) {
        Allele a(0.5f, 1.0f);  // Balanced omnivore
        Gene gene(MEAT_DIGESTION_EFFICIENCY, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(CELLULOSE_BREAKDOWN)) {
        Allele a(0.3f, 1.0f);  // Some plant processing
        Gene gene(CELLULOSE_BREAKDOWN, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(TOXIN_TOLERANCE)) {
        Allele a(0.3f, 1.0f);  // Some toxin resistance
        Gene gene(TOXIN_TOLERANCE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(TOXIN_METABOLISM)) {
        Allele a(0.3f, 1.0f);
        Gene gene(TOXIN_METABOLISM, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(SCENT_DETECTION)) {
        Allele a(0.5f, 1.0f);
        Gene gene(SCENT_DETECTION, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(COLOR_VISION)) {
        Allele a(0.5f, 1.0f);
        Gene gene(COLOR_VISION, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    
    // ========== NEW MORPHOLOGY GENES - Balanced omnivore anatomy ==========
    if (registry.hasGene(GUT_LENGTH)) {
        Allele a(0.5f, 1.0f);  // Medium gut length
        Gene gene(GUT_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TOOTH_SHARPNESS)) {
        Allele a(0.5f, 1.0f);  // Mixed teeth
        Gene gene(TOOTH_SHARPNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TOOTH_GRINDING)) {
        Allele a(0.5f, 1.0f);  // Mixed teeth
        Gene gene(TOOTH_GRINDING, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(STOMACH_ACIDITY)) {
        Allele a(0.5f, 1.0f);  // Moderate acidity
        Gene gene(STOMACH_ACIDITY, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(JAW_STRENGTH)) {
        Allele a(0.5f, 1.0f);
        Gene gene(JAW_STRENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(JAW_SPEED)) {
        Allele a(0.5f, 1.0f);
        Gene gene(JAW_SPEED, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HIDE_THICKNESS)) {
        Allele a(0.3f, 1.0f);
        Gene gene(HIDE_THICKNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(FUR_DENSITY)) {
        Allele a(0.5f, 1.0f);
        Gene gene(FUR_DENSITY, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(MUCUS_PROTECTION)) {
        Allele a(0.5f, 1.0f);
        Gene gene(MUCUS_PROTECTION, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== NEW BEHAVIOR GENES ==========
    if (registry.hasGene(SWEETNESS_PREFERENCE)) {
        Allele a(0.5f, 1.0f);
        Gene gene(SWEETNESS_PREFERENCE, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(CACHING_INSTINCT)) {
        Allele a(0.3f, 1.0f);
        Gene gene(CACHING_INSTINCT, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(SPATIAL_MEMORY)) {
        Allele a(0.5f, 1.0f);
        Gene gene(SPATIAL_MEMORY, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(GROOMING_FREQUENCY)) {
        Allele a(0.5f, 1.0f);
        Gene gene(GROOMING_FREQUENCY, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(PAIN_SENSITIVITY)) {
        Allele a(0.5f, 1.0f);
        Gene gene(PAIN_SENSITIVITY, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    
    // ========== OLFACTORY SYSTEM GENES - Active in creatures (Phase 1) ==========
    if (registry.hasGene(SCENT_PRODUCTION)) {
        Allele a(0.5f, 1.0f);  // Moderate scent production
        Gene gene(SCENT_PRODUCTION, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(SCENT_SIGNATURE_VARIANCE)) {
        Allele a(0.5f, 1.0f);  // Moderate signature uniqueness
        Gene gene(SCENT_SIGNATURE_VARIANCE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(OLFACTORY_ACUITY)) {
        Allele a(0.5f, 1.0f);  // Moderate scent detection ability
        Gene gene(OLFACTORY_ACUITY, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(SCENT_MASKING)) {
        Allele a(0.3f, 1.0f);  // Low scent masking by default
        Gene gene(SCENT_MASKING, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    
    // ========== SEED INTERACTION GENES ==========
    if (registry.hasGene(GUT_TRANSIT_TIME)) {
        Allele a(1.5f, 1.0f);  // Medium transit time
        Gene gene(GUT_TRANSIT_TIME, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(SEED_DESTRUCTION_RATE)) {
        Allele a(0.5f, 1.0f);  // Some seeds survive
        Gene gene(SEED_DESTRUCTION_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    // ========== PLANT DEFENSE GENES - Dormant in creatures ==========
    if (registry.hasGene(TOXIN_PRODUCTION)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(TOXIN_PRODUCTION, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(THORN_DENSITY)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(THORN_DENSITY, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(REGROWTH_RATE)) {
        Allele a(0.1f, 0.1f);  // Low regeneration
        Gene gene(REGROWTH_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(FRUIT_PRODUCTION_RATE)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(FRUIT_PRODUCTION_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SEED_COAT_DURABILITY)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(SEED_COAT_DURABILITY, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(FRUIT_APPEAL)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(FRUIT_APPEAL, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    // ========== REPRODUCTION GENES - Full expression ==========
    if (registry.hasGene(OFFSPRING_COUNT)) {
        Allele a(2.0f, 1.0f);  // Small litters
        Gene gene(OFFSPRING_COUNT, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(MATE_THRESHOLD)) {
        Allele a(3.0f, 1.0f);
        Gene gene(MATE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SPREAD_DISTANCE)) {
        Allele a(5.0f, 0.5f);  // Moderate spread (offspring walk)
        Gene gene(SPREAD_DISTANCE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(FATIGUE_THRESHOLD)) {
        Allele a(5.0f, 1.0f);
        Gene gene(FATIGUE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(COMFORT_INCREASE)) {
        Allele a(0.004f, 1.0f);
        Gene gene(COMFORT_INCREASE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(COMFORT_DECREASE)) {
        Allele a(0.002f, 1.0f);
        Gene gene(COMFORT_DECREASE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    // ========== SEED PROPAGATION GENES - Dormant in creatures (Phase 2.3) ==========
    if (registry.hasGene(SEED_MASS)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(SEED_MASS, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SEED_AERODYNAMICS)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(SEED_AERODYNAMICS, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SEED_HOOK_STRENGTH)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(SEED_HOOK_STRENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(EXPLOSIVE_POD_FORCE)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(EXPLOSIVE_POD_FORCE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(RUNNER_PRODUCTION)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(RUNNER_PRODUCTION, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    // ========== HEALTH/HEALING GENES - Full expression ==========
    if (registry.hasGene(REGENERATION_RATE)) {
        Allele a(0.5f, 1.0f);  // Moderate regeneration
        Gene gene(REGENERATION_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(WOUND_TOLERANCE)) {
        Allele a(0.5f, 1.0f);
        Gene gene(WOUND_TOLERANCE, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(BLEEDING_RESISTANCE)) {
        Allele a(0.5f, 1.0f);
        Gene gene(BLEEDING_RESISTANCE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== COMBAT WEAPON SHAPE GENES - Phase 1c ==========
    // Teeth (3 genes) - predator-oriented defaults
    if (registry.hasGene(TEETH_SHARPNESS)) {
        Allele a(0.7f, 1.0f);  // Sharp teeth
        Gene gene(TEETH_SHARPNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TEETH_SERRATION)) {
        Allele a(0.3f, 1.0f);  // Moderate serration
        Gene gene(TEETH_SERRATION, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TEETH_SIZE)) {
        Allele a(0.5f, 1.0f);  // Medium teeth
        Gene gene(TEETH_SIZE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Claws (3 genes)
    if (registry.hasGene(CLAW_LENGTH)) {
        Allele a(0.5f, 1.0f);
        Gene gene(CLAW_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(CLAW_CURVATURE)) {
        Allele a(0.4f, 1.0f);
        Gene gene(CLAW_CURVATURE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(CLAW_SHARPNESS)) {
        Allele a(0.6f, 1.0f);
        Gene gene(CLAW_SHARPNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Horns (3 genes) - defensive-oriented defaults
    if (registry.hasGene(HORN_LENGTH)) {
        Allele a(0.5f, 1.0f);
        Gene gene(HORN_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HORN_POINTINESS)) {
        Allele a(0.5f, 1.0f);
        Gene gene(HORN_POINTINESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HORN_SPREAD)) {
        Allele a(0.3f, 1.0f);
        Gene gene(HORN_SPREAD, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Tail (3 genes)
    if (registry.hasGene(TAIL_LENGTH)) {
        Allele a(0.5f, 1.0f);
        Gene gene(TAIL_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TAIL_MASS)) {
        Allele a(0.4f, 1.0f);
        Gene gene(TAIL_MASS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TAIL_SPINES)) {
        Allele a(0.1f, 1.0f);  // Low by default
        Gene gene(TAIL_SPINES, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Body (1 gene)
    if (registry.hasGene(BODY_SPINES)) {
        Allele a(0.0f, 1.0f);  // Rare trait
        Gene gene(BODY_SPINES, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== COMBAT DEFENSE GENES - Phase 1c ==========
    if (registry.hasGene(SCALE_COVERAGE)) {
        Allele a(0.3f, 1.0f);  // Some scale coverage
        Gene gene(SCALE_COVERAGE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(FAT_LAYER_THICKNESS)) {
        Allele a(0.4f, 1.0f);  // Moderate fat layer
        Gene gene(FAT_LAYER_THICKNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== COMBAT BEHAVIOR GENES - Phase 1c ==========
    if (registry.hasGene(COMBAT_AGGRESSION)) {
        Allele a(0.5f, 1.0f);  // Balanced aggression
        Gene gene(COMBAT_AGGRESSION, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(RETREAT_THRESHOLD)) {
        Allele a(0.3f, 1.0f);  // Flee at 30% health
        Gene gene(RETREAT_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(TERRITORIAL_AGGRESSION)) {
        Allele a(0.3f, 1.0f);  // Low intra-species aggression
        Gene gene(TERRITORIAL_AGGRESSION, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(PACK_COORDINATION)) {
        Allele a(0.0f, 1.0f);  // No pack behavior (future)
        Gene gene(PACK_COORDINATION, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    
    return genome;
}

Genome UniversalGenes::createPlantGenome(const GeneRegistry& registry) {
    Genome genome;
    initializeCategories();
    
    // Default values for plant-like organisms
    // High expression for autotrophy, low for mobility and heterotrophy
    
    // ========== UNIVERSAL GENES - Full expression ==========
    if (registry.hasGene(LIFESPAN)) {
        Allele a(5000.0f, 1.0f);  // Shorter lifespan than creatures
        Gene gene(LIFESPAN, a, a);
        genome.addGene(gene, ChromosomeType::Lifespan);
    }
    if (registry.hasGene(MAX_SIZE)) {
        Allele a(5.0f, 1.0f);
        Gene gene(MAX_SIZE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(METABOLISM_RATE)) {
        Allele a(0.5f, 1.0f);  // Slower metabolism
        Gene gene(METABOLISM_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(COLOR_HUE)) {
        Allele a(120.0f, 1.0f);  // Green for plants
        Gene gene(COLOR_HUE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HARDINESS)) {
        Allele a(0.5f, 1.0f);
        Gene gene(HARDINESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TEMP_TOLERANCE_LOW)) {
        Allele a(5.0f, 1.0f);
        Gene gene(TEMP_TOLERANCE_LOW, a, a);
        genome.addGene(gene, ChromosomeType::Environmental);
    }
    if (registry.hasGene(TEMP_TOLERANCE_HIGH)) {
        Allele a(35.0f, 1.0f);
        Gene gene(TEMP_TOLERANCE_HIGH, a, a);
        genome.addGene(gene, ChromosomeType::Environmental);
    }
    
    // ========== MOBILITY GENES - Dormant (very low expression) ==========
    if (registry.hasGene(LOCOMOTION)) {
        Allele a(0.0f, 0.1f);  // Plants don't move (usually!)
        Gene gene(LOCOMOTION, a, a);
        genome.addGene(gene, ChromosomeType::Locomotion);
    }
    if (registry.hasGene(SIGHT_RANGE)) {
        Allele a(0.0f, 0.1f);
        Gene gene(SIGHT_RANGE, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(NAVIGATION_ABILITY)) {
        Allele a(0.0f, 0.1f);
        Gene gene(NAVIGATION_ABILITY, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(FLEE_THRESHOLD)) {
        Allele a(0.0f, 0.1f);
        Gene gene(FLEE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(PURSUE_THRESHOLD)) {
        Allele a(0.0f, 0.1f);
        Gene gene(PURSUE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    
    // ========== AUTOTROPHY GENES - High expression ==========
    if (registry.hasGene(PHOTOSYNTHESIS)) {
        Allele a(0.8f, 1.0f);  // High photosynthesis
        Gene gene(PHOTOSYNTHESIS, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(ROOT_DEPTH)) {
        Allele a(3.0f, 1.0f);
        Gene gene(ROOT_DEPTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(WATER_STORAGE)) {
        Allele a(0.5f, 1.0f);
        Gene gene(WATER_STORAGE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(LIGHT_REQUIREMENT)) {
        Allele a(0.5f, 1.0f);  // Moderate light needed
        Gene gene(LIGHT_REQUIREMENT, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(WATER_REQUIREMENT)) {
        Allele a(0.5f, 1.0f);  // Moderate water needed
        Gene gene(WATER_REQUIREMENT, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    // ========== ORIGINAL HETEROTROPHY GENES - Dormant (can activate for carnivorous plants!) ==========
    if (registry.hasGene(HUNT_INSTINCT)) {
        Allele a(0.0f, 0.1f);  // Dormant but can evolve
        Gene gene(HUNT_INSTINCT, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(HUNGER_THRESHOLD)) {
        Allele a(0.0f, 0.1f);
        Gene gene(HUNGER_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(THIRST_THRESHOLD)) {
        Allele a(0.0f, 0.1f);
        Gene gene(THIRST_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(DIET_TYPE)) {
        Allele a(0.0f, 0.1f);  // Not applicable for most plants
        Gene gene(DIET_TYPE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(DIGESTIVE_EFFICIENCY)) {
        Allele a(0.1f, 0.1f);
        Gene gene(DIGESTIVE_EFFICIENCY, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(NUTRIENT_VALUE)) {
        Allele a(25.0f, 1.0f);  // Plants are nutritious when eaten
        Gene gene(NUTRIENT_VALUE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    // ========== NEW HETEROTROPHY GENES - Dormant in plants ==========
    if (registry.hasGene(PLANT_DIGESTION_EFFICIENCY)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(PLANT_DIGESTION_EFFICIENCY, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(MEAT_DIGESTION_EFFICIENCY)) {
        Allele a(0.0f, 0.1f);  // Dormant (carnivorous plants could evolve this!)
        Gene gene(MEAT_DIGESTION_EFFICIENCY, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(CELLULOSE_BREAKDOWN)) {
        Allele a(0.0f, 0.1f);
        Gene gene(CELLULOSE_BREAKDOWN, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(TOXIN_TOLERANCE)) {
        Allele a(0.0f, 0.1f);
        Gene gene(TOXIN_TOLERANCE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(TOXIN_METABOLISM)) {
        Allele a(0.0f, 0.1f);
        Gene gene(TOXIN_METABOLISM, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(SCENT_DETECTION)) {
        Allele a(0.0f, 0.1f);
        Gene gene(SCENT_DETECTION, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(COLOR_VISION)) {
        Allele a(0.0f, 0.1f);
        Gene gene(COLOR_VISION, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    
    // ========== NEW MORPHOLOGY GENES - Plant defaults ==========
    if (registry.hasGene(GUT_LENGTH)) {
        Allele a(0.3f, 0.1f);  // Minimal gut
        Gene gene(GUT_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TOOTH_SHARPNESS)) {
        Allele a(0.0f, 0.1f);  // No teeth
        Gene gene(TOOTH_SHARPNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TOOTH_GRINDING)) {
        Allele a(0.0f, 0.1f);  // No teeth
        Gene gene(TOOTH_GRINDING, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(STOMACH_ACIDITY)) {
        Allele a(0.0f, 0.1f);  // No stomach
        Gene gene(STOMACH_ACIDITY, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(JAW_STRENGTH)) {
        Allele a(0.0f, 0.1f);  // No jaw
        Gene gene(JAW_STRENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(JAW_SPEED)) {
        Allele a(0.0f, 0.1f);  // No jaw
        Gene gene(JAW_SPEED, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HIDE_THICKNESS)) {
        Allele a(0.3f, 0.5f);  // Some bark/skin
        Gene gene(HIDE_THICKNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(FUR_DENSITY)) {
        Allele a(0.0f, 0.1f);  // No fur (could have trichomes)
        Gene gene(FUR_DENSITY, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(MUCUS_PROTECTION)) {
        Allele a(0.3f, 0.5f);  // Some surface protection
        Gene gene(MUCUS_PROTECTION, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== NEW BEHAVIOR GENES - Dormant in plants ==========
    if (registry.hasGene(SWEETNESS_PREFERENCE)) {
        Allele a(0.0f, 0.1f);
        Gene gene(SWEETNESS_PREFERENCE, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(CACHING_INSTINCT)) {
        Allele a(0.0f, 0.1f);
        Gene gene(CACHING_INSTINCT, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(SPATIAL_MEMORY)) {
        Allele a(0.0f, 0.1f);
        Gene gene(SPATIAL_MEMORY, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(GROOMING_FREQUENCY)) {
        Allele a(0.0f, 0.1f);
        Gene gene(GROOMING_FREQUENCY, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(PAIN_SENSITIVITY)) {
        Allele a(0.2f, 0.3f);  // Some damage sensing
        Gene gene(PAIN_SENSITIVITY, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    
    // ========== OLFACTORY SYSTEM GENES - Dormant in plants (Phase 1) ==========
    // Plants could potentially evolve scent production for attracting pollinators
    if (registry.hasGene(SCENT_PRODUCTION)) {
        Allele a(0.0f, 0.1f);  // Dormant, but could evolve for pollinator attraction
        Gene gene(SCENT_PRODUCTION, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(SCENT_SIGNATURE_VARIANCE)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(SCENT_SIGNATURE_VARIANCE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(OLFACTORY_ACUITY)) {
        Allele a(0.0f, 0.1f);  // Dormant - plants don't detect scents
        Gene gene(OLFACTORY_ACUITY, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    if (registry.hasGene(SCENT_MASKING)) {
        Allele a(0.0f, 0.1f);  // Dormant
        Gene gene(SCENT_MASKING, a, a);
        genome.addGene(gene, ChromosomeType::Sensory);
    }
    
    // ========== SEED INTERACTION GENES - Not applicable for plants ==========
    if (registry.hasGene(GUT_TRANSIT_TIME)) {
        Allele a(0.5f, 0.1f);  // Minimal
        Gene gene(GUT_TRANSIT_TIME, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(SEED_DESTRUCTION_RATE)) {
        Allele a(0.0f, 0.1f);  // Not applicable
        Gene gene(SEED_DESTRUCTION_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    // ========== PLANT DEFENSE GENES - Active! ==========
    if (registry.hasGene(TOXIN_PRODUCTION)) {
        Allele a(0.3f, 1.0f);  // Some toxin production
        Gene gene(TOXIN_PRODUCTION, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(THORN_DENSITY)) {
        Allele a(0.2f, 1.0f);  // Some thorns
        Gene gene(THORN_DENSITY, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(REGROWTH_RATE)) {
        Allele a(0.5f, 1.0f);  // Good regrowth
        Gene gene(REGROWTH_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    if (registry.hasGene(FRUIT_PRODUCTION_RATE)) {
        Allele a(0.5f, 1.0f);  // Moderate fruit production
        Gene gene(FRUIT_PRODUCTION_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SEED_COAT_DURABILITY)) {
        Allele a(0.5f, 1.0f);  // Seeds can survive digestion
        Gene gene(SEED_COAT_DURABILITY, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(FRUIT_APPEAL)) {
        Allele a(0.5f, 1.0f);  // Moderately appealing fruit
        Gene gene(FRUIT_APPEAL, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    // ========== REPRODUCTION GENES - Full expression (seed-based) ==========
    if (registry.hasGene(OFFSPRING_COUNT)) {
        Allele a(5.0f, 1.0f);  // More seeds than creature offspring
        Gene gene(OFFSPRING_COUNT, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(MATE_THRESHOLD)) {
        Allele a(0.0f, 0.1f);  // Plants don't "seek mates" (asexual or wind pollination)
        Gene gene(MATE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SPREAD_DISTANCE)) {
        Allele a(5.0f, 1.0f);  // Seed dispersal range
        Gene gene(SPREAD_DISTANCE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(FATIGUE_THRESHOLD)) {
        Allele a(0.0f, 0.1f);  // Plants don't fatigue
        Gene gene(FATIGUE_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(COMFORT_INCREASE)) {
        Allele a(0.01f, 0.5f);  // Different comfort dynamics for plants
        Gene gene(COMFORT_INCREASE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(COMFORT_DECREASE)) {
        Allele a(0.005f, 0.5f);
        Gene gene(COMFORT_DECREASE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    // ========== SEED PROPAGATION GENES - Active in plants! (Phase 2.3) ==========
    if (registry.hasGene(SEED_MASS)) {
        Allele a(0.3f, 1.0f);  // Medium seed mass for gravity dispersal
        Gene gene(SEED_MASS, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SEED_AERODYNAMICS)) {
        Allele a(0.3f, 1.0f);  // Moderate aerodynamics
        Gene gene(SEED_AERODYNAMICS, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(SEED_HOOK_STRENGTH)) {
        Allele a(0.1f, 1.0f);  // Low hook strength by default
        Gene gene(SEED_HOOK_STRENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(EXPLOSIVE_POD_FORCE)) {
        Allele a(0.0f, 1.0f);  // No explosive dispersal by default
        Gene gene(EXPLOSIVE_POD_FORCE, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    if (registry.hasGene(RUNNER_PRODUCTION)) {
        Allele a(0.2f, 1.0f);  // Low runner production by default
        Gene gene(RUNNER_PRODUCTION, a, a);
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    // ========== HEALTH/HEALING GENES - Active in plants ==========
    if (registry.hasGene(REGENERATION_RATE)) {
        Allele a(0.7f, 1.0f);  // Plants have good regeneration
        Gene gene(REGENERATION_RATE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(WOUND_TOLERANCE)) {
        Allele a(0.6f, 1.0f);  // Plants tolerate wounds well
        Gene gene(WOUND_TOLERANCE, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(BLEEDING_RESISTANCE)) {
        Allele a(0.4f, 1.0f);  // Some sap loss resistance
        Gene gene(BLEEDING_RESISTANCE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== COMBAT WEAPON SHAPE GENES - Dormant in plants (Phase 1c) ==========
    // Plants don't have physical weapons - all set to 0.0 with low expression
    // Teeth (3 genes)
    if (registry.hasGene(TEETH_SHARPNESS)) {
        Allele a(0.0f, 0.1f);  // No teeth
        Gene gene(TEETH_SHARPNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TEETH_SERRATION)) {
        Allele a(0.0f, 0.1f);  // No teeth
        Gene gene(TEETH_SERRATION, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TEETH_SIZE)) {
        Allele a(0.0f, 0.1f);  // No teeth
        Gene gene(TEETH_SIZE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Claws (3 genes)
    if (registry.hasGene(CLAW_LENGTH)) {
        Allele a(0.0f, 0.1f);  // No claws
        Gene gene(CLAW_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(CLAW_CURVATURE)) {
        Allele a(0.0f, 0.1f);  // No claws
        Gene gene(CLAW_CURVATURE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(CLAW_SHARPNESS)) {
        Allele a(0.0f, 0.1f);  // No claws
        Gene gene(CLAW_SHARPNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Horns (3 genes)
    if (registry.hasGene(HORN_LENGTH)) {
        Allele a(0.0f, 0.1f);  // No horns
        Gene gene(HORN_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HORN_POINTINESS)) {
        Allele a(0.0f, 0.1f);  // No horns
        Gene gene(HORN_POINTINESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(HORN_SPREAD)) {
        Allele a(0.0f, 0.1f);  // No horns
        Gene gene(HORN_SPREAD, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Tail (3 genes)
    if (registry.hasGene(TAIL_LENGTH)) {
        Allele a(0.0f, 0.1f);  // No tail
        Gene gene(TAIL_LENGTH, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TAIL_MASS)) {
        Allele a(0.0f, 0.1f);  // No tail
        Gene gene(TAIL_MASS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(TAIL_SPINES)) {
        Allele a(0.0f, 0.1f);  // No tail spines
        Gene gene(TAIL_SPINES, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    // Body (1 gene) - could evolve for defensive thorns
    if (registry.hasGene(BODY_SPINES)) {
        Allele a(0.0f, 0.3f);  // Dormant but could evolve (thorns!)
        Gene gene(BODY_SPINES, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== COMBAT DEFENSE GENES - Moderate in plants (Phase 1c) ==========
    // scale_coverage → bark, fat_layer → waxy coating
    if (registry.hasGene(SCALE_COVERAGE)) {
        Allele a(0.4f, 0.8f);  // Bark-like protection
        Gene gene(SCALE_COVERAGE, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    if (registry.hasGene(FAT_LAYER_THICKNESS)) {
        Allele a(0.3f, 0.8f);  // Waxy cuticle coating
        Gene gene(FAT_LAYER_THICKNESS, a, a);
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // ========== COMBAT BEHAVIOR GENES - Dormant in plants (Phase 1c) ==========
    if (registry.hasGene(COMBAT_AGGRESSION)) {
        Allele a(0.0f, 0.1f);  // Plants don't fight
        Gene gene(COMBAT_AGGRESSION, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(RETREAT_THRESHOLD)) {
        Allele a(0.0f, 0.1f);  // Plants can't flee
        Gene gene(RETREAT_THRESHOLD, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(TERRITORIAL_AGGRESSION)) {
        Allele a(0.0f, 0.1f);  // No territorial behavior
        Gene gene(TERRITORIAL_AGGRESSION, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    if (registry.hasGene(PACK_COORDINATION)) {
        Allele a(0.0f, 0.1f);  // No pack behavior
        Gene gene(PACK_COORDINATION, a, a);
        genome.addGene(gene, ChromosomeType::Behavior);
    }
    
    return genome;
}

Genome UniversalGenes::createRandomGenome(const GeneRegistry& registry) {
    Genome genome;
    
    // Thread-local random engine
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> expressionDist(0.1f, 1.0f);
    
    // Get all definitions and create random genes for each
    const auto& definitions = registry.getAllDefinitions();
    
    for (const auto& [id, definition] : definitions) {
        // Create random value within gene limits
        GeneValue randomValue = definition.createRandomValue();
        
        // Random expression strength
        float expression = expressionDist(rng);
        
        Allele a1(randomValue, expression);
        
        // Second allele might be different (heterozygous)
        GeneValue randomValue2 = definition.createRandomValue();
        float expression2 = expressionDist(rng);
        Allele a2(randomValue2, expression2);
        
        Gene gene(id, a1, a2);
        genome.addGene(gene, definition.getChromosome());
    }
    
    return genome;
}

} // namespace Genetics
} // namespace EcoSim
