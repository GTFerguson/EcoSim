/**
 * @file PlantGenes.cpp
 * @brief Implementation of plant-specific gene definitions
 * 
 * This file creates gene definitions for plants, allowing the genetics system
 * to support multiple organism types (plants, creatures, etc.) using the same
 * core infrastructure with different gene sets.
 */

#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/core/GeneticTypes.hpp"

namespace EcoSim {
namespace Genetics {

void PlantGenes::registerDefaults(GeneRegistry& registry) {
    // ============================================================================
    // Metabolism Chromosome - Photosynthesis and resource needs
    // ============================================================================
    
    // Growth rate - how fast the plant grows per tick
    // [0.1, 2.0], creep 0.1 - allows for slow to fast growing plants
    GeneDefinition growthRate(GROWTH_RATE, ChromosomeType::Metabolism,
        GeneLimits(0.1f, 2.0f, 0.1f), DominanceType::Incomplete);
    growthRate.addEffect(EffectBinding("metabolism", "growth_rate", EffectType::Direct, 1.0f));
    growthRate.setModulationPolicy(TraitModulationPolicy::HEALTH_ONLY);
    registry.registerGene(std::move(growthRate));
    
    // Water need - how much water the plant requires
    // [0.0, 1.0], creep 0.05 - 0 = drought resistant, 1 = water-loving
    GeneDefinition waterNeed(WATER_NEED, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    waterNeed.addEffect(EffectBinding("metabolism", "water_requirement", EffectType::Direct, 1.0f));
    waterNeed.setModulationPolicy(TraitModulationPolicy::HEALTH_ONLY);
    registry.registerGene(std::move(waterNeed));
    
    // Light need - how much light the plant requires
    // [0.0, 1.0], creep 0.05 - 0 = shade tolerant, 1 = full sun required
    GeneDefinition lightNeed(LIGHT_NEED, ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    lightNeed.addEffect(EffectBinding("metabolism", "light_requirement", EffectType::Direct, 1.0f));
    lightNeed.setModulationPolicy(TraitModulationPolicy::HEALTH_ONLY);
    registry.registerGene(std::move(lightNeed));
    
    // Nutrient value - how nutritious the plant is when eaten
    // [1.0, 100.0], creep 5.0 - energy provided to herbivores
    GeneDefinition nutrientValue(NUTRIENT_VALUE, ChromosomeType::Metabolism,
        GeneLimits(1.0f, 100.0f, 5.0f), DominanceType::Incomplete);
    nutrientValue.addEffect(EffectBinding("metabolism", "nutrient_value", EffectType::Direct, 1.0f));
    nutrientValue.setModulationPolicy(TraitModulationPolicy::ENERGY_GATED);
    registry.registerGene(std::move(nutrientValue));
    
    // ============================================================================
    // Morphology Chromosome - Physical structure and appearance
    // ============================================================================
    
    // Maximum size - how large the plant can grow
    // [1.0, 10.0], creep 0.5 - abstract size units
    GeneDefinition maxSize(MAX_SIZE, ChromosomeType::Morphology,
        GeneLimits(1.0f, 10.0f, 0.5f), DominanceType::Incomplete);
    maxSize.addEffect(EffectBinding("morphology", "max_size", EffectType::Direct, 1.0f));
    maxSize.setModulationPolicy(TraitModulationPolicy::NEVER);
    registry.registerGene(std::move(maxSize));
    
    // Hardiness - resistance to damage from herbivores and environment
    // [0.0, 1.0], creep 0.05 - affects survival chance under stress
    GeneDefinition hardiness(HARDINESS, ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.05f), DominanceType::Incomplete);
    hardiness.addEffect(EffectBinding("morphology", "damage_resistance", EffectType::Direct, 1.0f));
    hardiness.setModulationPolicy(TraitModulationPolicy::NEVER);
    registry.registerGene(std::move(hardiness));
    
    // Color hue - HSV hue for visual appearance
    // [0.0, 360.0], creep 10.0 - color variation for visualization
    GeneDefinition colorHue(COLOR_HUE, ChromosomeType::Morphology,
        GeneLimits(0.0f, 360.0f, 10.0f), DominanceType::Incomplete);
    colorHue.addEffect(EffectBinding("morphology", "color_hue", EffectType::Direct, 1.0f));
    registry.registerGene(std::move(colorHue));
    
    // ============================================================================
    // Reproduction Chromosome - Seed production and dispersal
    // ============================================================================
    
    // Seed production - number of seeds per growth cycle
    // [0.0, 10.0], creep 0.5 - reproductive output
    GeneDefinition seedProduction(SEED_PRODUCTION, ChromosomeType::Reproduction,
        GeneLimits(0.0f, 10.0f, 0.5f), DominanceType::Incomplete);
    seedProduction.addEffect(EffectBinding("reproduction", "seed_count", EffectType::Direct, 1.0f));
    registry.registerGene(std::move(seedProduction));
    
    // Spread distance - how far seeds can disperse from parent
    // [1.0, 20.0], creep 1.0 - dispersal range in tiles
    GeneDefinition spreadDistance(SPREAD_DISTANCE, ChromosomeType::Reproduction,
        GeneLimits(1.0f, 20.0f, 1.0f), DominanceType::Incomplete);
    spreadDistance.addEffect(EffectBinding("reproduction", "dispersal_range", EffectType::Direct, 1.0f));
    registry.registerGene(std::move(spreadDistance));
    
    // ============================================================================
    // Environmental Chromosome - Climate tolerance
    // ============================================================================
    
    // Temperature tolerance low - minimum survival temperature
    // [-20.0, 30.0], creep 2.0 - Celsius, allows arctic to tropical plants
    GeneDefinition tempLow(TEMP_TOLERANCE_LOW, ChromosomeType::Environmental,
        GeneLimits(-20.0f, 30.0f, 2.0f), DominanceType::Incomplete);
    tempLow.addEffect(EffectBinding("environmental", "temp_tolerance_low", EffectType::Direct, 1.0f));
    registry.registerGene(std::move(tempLow));
    
    // Temperature tolerance high - maximum survival temperature
    // [10.0, 50.0], creep 2.0 - Celsius, allows cold to desert plants
    GeneDefinition tempHigh(TEMP_TOLERANCE_HIGH, ChromosomeType::Environmental,
        GeneLimits(10.0f, 50.0f, 2.0f), DominanceType::Incomplete);
    tempHigh.addEffect(EffectBinding("environmental", "temp_tolerance_high", EffectType::Direct, 1.0f));
    registry.registerGene(std::move(tempHigh));
    
    // ============================================================================
    // Lifespan Chromosome - Longevity
    // ============================================================================
    
    // Lifespan - how long the plant lives before dying naturally
    // [100, 10000], creep 500 - annual to perennial plants
    GeneDefinition lifespan(LIFESPAN, ChromosomeType::Lifespan,
        GeneLimits(100.0f, 10000.0f, 500.0f), DominanceType::Incomplete);
    lifespan.addEffect(EffectBinding("lifespan", "max_age", EffectType::Direct, 1.0f));
    registry.registerGene(std::move(lifespan));
}

Genome PlantGenes::createRandomGenome(const GeneRegistry& registry) {
    Genome genome;
    
    // Get all definitions and create random genes for each
    const auto& definitions = registry.getAllDefinitions();
    
    for (const auto& [id, definition] : definitions) {
        // Only add plant genes (those starting with "plant_")
        if (id.find("plant_") == 0) {
            GeneValue randomValue = definition.createRandomValue();
            Gene gene(id, randomValue);
            genome.addGene(gene, definition.getChromosome());
        }
    }
    
    return genome;
}

Genome PlantGenes::createDefaultGenome(const GeneRegistry& registry) {
    Genome genome;
    
    // Create genes with balanced default values
    
    // Metabolism chromosome
    if (registry.hasGene(GROWTH_RATE)) {
        Gene gene(GROWTH_RATE, GeneValue(DEFAULT_GROWTH_RATE));
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    if (registry.hasGene(WATER_NEED)) {
        Gene gene(WATER_NEED, GeneValue(DEFAULT_WATER_NEED));
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    if (registry.hasGene(LIGHT_NEED)) {
        Gene gene(LIGHT_NEED, GeneValue(DEFAULT_LIGHT_NEED));
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    if (registry.hasGene(NUTRIENT_VALUE)) {
        Gene gene(NUTRIENT_VALUE, GeneValue(DEFAULT_NUTRIENT_VALUE));
        genome.addGene(gene, ChromosomeType::Metabolism);
    }
    
    // Morphology chromosome
    if (registry.hasGene(MAX_SIZE)) {
        Gene gene(MAX_SIZE, GeneValue(DEFAULT_MAX_SIZE));
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    if (registry.hasGene(HARDINESS)) {
        Gene gene(HARDINESS, GeneValue(DEFAULT_HARDINESS));
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    if (registry.hasGene(COLOR_HUE)) {
        Gene gene(COLOR_HUE, GeneValue(DEFAULT_COLOR_HUE));
        genome.addGene(gene, ChromosomeType::Morphology);
    }
    
    // Reproduction chromosome
    if (registry.hasGene(SEED_PRODUCTION)) {
        Gene gene(SEED_PRODUCTION, GeneValue(DEFAULT_SEED_PRODUCTION));
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    if (registry.hasGene(SPREAD_DISTANCE)) {
        Gene gene(SPREAD_DISTANCE, GeneValue(DEFAULT_SPREAD_DISTANCE));
        genome.addGene(gene, ChromosomeType::Reproduction);
    }
    
    // Environmental chromosome
    if (registry.hasGene(TEMP_TOLERANCE_LOW)) {
        Gene gene(TEMP_TOLERANCE_LOW, GeneValue(DEFAULT_TEMP_LOW));
        genome.addGene(gene, ChromosomeType::Environmental);
    }
    
    if (registry.hasGene(TEMP_TOLERANCE_HIGH)) {
        Gene gene(TEMP_TOLERANCE_HIGH, GeneValue(DEFAULT_TEMP_HIGH));
        genome.addGene(gene, ChromosomeType::Environmental);
    }
    
    // Lifespan chromosome
    if (registry.hasGene(LIFESPAN)) {
        Gene gene(LIFESPAN, GeneValue(DEFAULT_LIFESPAN));
        genome.addGene(gene, ChromosomeType::Lifespan);
    }
    
    return genome;
}

} // namespace Genetics
} // namespace EcoSim
