#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/Gene.hpp"
#include <cmath>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructors
// ============================================================================

Phenotype::Phenotype() 
    : genome_(nullptr)
    , registry_(nullptr)
    , cache_()
    , environment_()
    , organism_state_()
    , computed_traits_() 
{
}

Phenotype::Phenotype(const Genome* genome)
    : genome_(genome)
    , registry_(nullptr)
    , cache_()
    , environment_()
    , organism_state_()
    , computed_traits_()
{
}

Phenotype::Phenotype(const Genome* genome, const GeneRegistry* registry)
    : genome_(genome)
    , registry_(registry)
    , cache_()
    , environment_()
    , organism_state_()
    , computed_traits_()
{
}

// ============================================================================
// Configuration
// ============================================================================

void Phenotype::setGenome(const Genome* genome) {
    if (genome_ != genome) {
        genome_ = genome;
        invalidateCache();
    }
}

void Phenotype::setRegistry(const GeneRegistry* registry) {
    if (registry_ != registry) {
        registry_ = registry;
        invalidateCache();
    }
}

void Phenotype::updateContext(const EnvironmentState& env, const OrganismState& org) {
    // Check if environment changed significantly (triggers cache invalidation)
    cache_.checkInvalidation(env, org);
    
    environment_ = env;
    organism_state_ = org;
}

// ============================================================================
// Trait Access
// ============================================================================

float Phenotype::getTrait(const std::string& trait_id) const {
    if (!isValid()) {
        return 0.0f;
    }
    
    // Use cache with lambda for computation
    return cache_.getOrCompute(trait_id, [this, &trait_id]() {
        return computeTrait(trait_id);
    });
}

float Phenotype::computeTrait(const std::string& trait_id) const {
    if (!isValid()) {
        return 0.0f;
    }
    
    // Early exit if trait doesn't exist (either as direct gene or via effect bindings)
    if (!hasTrait(trait_id)) {
        return 0.0f;
    }
    
    // Delegate to raw computation (handles both direct genes and effect bindings)
    float raw_value = computeTraitRaw(trait_id);
    
    // Apply all modulations
    float modulated = applyAgeModulation(raw_value, organism_state_.age_normalized);
    modulated = applyEnvironmentModulation(modulated, trait_id, environment_);
    modulated = applyOrganismStateModulation(modulated, trait_id, organism_state_);
    
    // Store in computed traits map
    computed_traits_[trait_id] = modulated;
    
    return modulated;
}

float Phenotype::computeTraitRaw(const std::string& trait_id) const {
    if (!isValid()) {
        return 0.0f;
    }
    
    // Check if this is a direct gene lookup
    if (genome_->hasGene(trait_id) && registry_->hasGene(trait_id)) {
        const GeneDefinition& definition = registry_->getDefinition(trait_id);
        
        // Express the gene - NO modulations applied
        return expressGene(trait_id, definition);
    }
    
    // Trait not found - check if it's derived from effect bindings
    // Search all genes for effects that target this trait
    PhenotypeUtils::AccumulatedEffect accumulated;
    
    for (const auto& [gene_id, definition] : registry_->getAllDefinitions()) {
        for (const auto& effect : definition.getEffects()) {
            if (effect.target_trait == trait_id) {
                if (!genome_->hasGene(gene_id)) {
                    continue;
                }
                
                float gene_value = expressGene(gene_id, definition);
                accumulated = PhenotypeUtils::applyEffect(
                    accumulated, effect.effect_type, gene_value, effect.scale_factor);
            }
        }
    }
    
    // Return raw accumulated value - NO modulations applied
    if (accumulated.found_contribution) {
        return accumulated.value;
    }
    
    return 0.0f;
}

bool Phenotype::hasTrait(const std::string& trait_id) const {
    if (!isValid()) {
        return false;
    }
    
    // Check direct gene
    if (genome_->hasGene(trait_id) && registry_->hasGene(trait_id)) {
        return true;
    }
    
    // Check if any gene has an effect binding to this trait
    for (const auto& [gene_id, definition] : registry_->getAllDefinitions()) {
        for (const auto& effect : definition.getEffects()) {
            if (effect.target_trait == trait_id && genome_->hasGene(gene_id)) {
                return true;
            }
        }
    }
    
    return false;
}

void Phenotype::invalidateCache() {
    cache_.invalidateAll();
    computed_traits_.clear();
}

const std::unordered_map<std::string, float>& Phenotype::getAllTraits() const {
    if (!isValid()) {
        return computed_traits_;
    }
    
    computeAllTraits();
    return computed_traits_;
}

float Phenotype::getCacheHitRate() const {
    return cache_.getCacheHitRate();
}

bool Phenotype::isValid() const {
    return genome_ != nullptr && registry_ != nullptr;
}

// ============================================================================
// Expression Logic
// ============================================================================

float Phenotype::expressGene(const std::string& gene_id, 
                             const GeneDefinition& definition) const {
    if (!genome_->hasGene(gene_id)) {
        return 0.0f;
    }
    
    const Gene& gene = genome_->getGene(gene_id);
    
    // Get the numeric value based on dominance type
    return gene.getNumericValue(definition.getDominance());
}

float Phenotype::applyAgeModulation(float value, float age_normalized) const {
    // Clamp age to valid range
    float age = std::clamp(age_normalized, 0.0f, 1.0f);
    
    float modulation_factor;
    
    // Four life stages for realistic development and anti-exploit balance:
    // - Infant (0-0.05): 40% → 60% expression - newborns are underdeveloped
    // - Juvenile (0.05-0.15): 60% → 100% expression - rapid growth phase
    // - Adult (0.15-0.8): 100% expression - prime of life
    // - Elderly (0.8-1.0): 100% → 80% expression - gradual decline
    //
    // The infant stage at 40% expression helps prevent baby cannibalism exploit
    // by ensuring newborn corpses yield significantly less nutrition.
    
    if (age < 0.05f) {
        // Infant phase: 40% expression at birth, linearly increasing to 60% at age 0.05
        modulation_factor = 0.4f + (age / 0.05f) * 0.2f;
    }
    else if (age < 0.15f) {
        // Juvenile phase: 60% expression, linearly increasing to 100% at age 0.15
        modulation_factor = 0.6f + ((age - 0.05f) / 0.1f) * 0.4f;
    }
    else if (age < 0.8f) {
        // Adult phase: 100% expression
        modulation_factor = 1.0f;
    }
    else {
        // Elderly phase: Linear decline from 100% at age=0.8 to 80% at age=1.0
        float elderly_progress = (age - 0.8f) / 0.2f;
        modulation_factor = 1.0f - (elderly_progress * 0.2f);
    }
    
    return value * modulation_factor;
}

float Phenotype::applyEnvironmentModulation(float value, const std::string& trait_id,
                                            const EnvironmentState& env) const {
    float modulated_value = value;
    
    // Temperature effects on metabolism-related traits
    // Optimal temperature around 20-25°C, reduced expression at extremes
    if (trait_id.find("metabolism") != std::string::npos ||
        trait_id.find("energy") != std::string::npos ||
        trait_id.find("hunger") != std::string::npos) {
        
        // Optimal temperature range: 15-30°C
        float temp = env.temperature;
        float temp_factor = 1.0f;
        
        if (temp < 15.0f) {
            // Cold: reduced metabolism (linear drop to 0.5 at -10°C)
            temp_factor = std::max(0.5f, 1.0f - (15.0f - temp) * 0.02f);
        }
        else if (temp > 30.0f) {
            // Hot: increased metabolism (linear increase to 1.5 at 45°C)
            temp_factor = std::min(1.5f, 1.0f + (temp - 30.0f) * 0.033f);
        }
        
        modulated_value *= temp_factor;
    }
    
    // Light/time of day effects on sensory traits
    if (trait_id.find("vision") != std::string::npos ||
        trait_id.find("sight") != std::string::npos) {
        
        // Vision reduced at night (time_of_day: 0.0 = midnight, 0.5 = noon)
        // Simple model: vision is 50% at midnight, 100% at noon
        float light_factor = 0.5f + 0.5f * std::sin(env.time_of_day * 3.14159f);
        modulated_value *= light_factor;
    }
    
    // Humidity effects on locomotion
    if (trait_id.find("speed") != std::string::npos ||
        trait_id.find("movement") != std::string::npos ||
        trait_id.find("locomotion") != std::string::npos) {
        
        // Optimal humidity around 0.4-0.6, slight reduction at extremes
        float humidity = env.humidity;
        float humidity_factor = 1.0f;
        
        if (humidity < 0.2f || humidity > 0.8f) {
            humidity_factor = 0.9f;
        }
        
        modulated_value *= humidity_factor;
    }
    
    return modulated_value;
}

float Phenotype::applyOrganismStateModulation(float value, const std::string& trait_id,
                                               const OrganismState& org) const {
    // Look up the gene's modulation policy
    TraitModulationPolicy policy = TraitModulationPolicy::NEVER;
    
    if (registry_ && registry_->hasGene(trait_id)) {
        const GeneDefinition& def = registry_->getDefinition(trait_id);
        policy = def.getModulationPolicy();
    }
    
    // Apply modulation based on policy
    switch (policy) {
        case TraitModulationPolicy::NEVER:
            // Physical structure traits - never modulate
            // Hide thickness, teeth, claws don't change with fatigue
            return value;
            
        case TraitModulationPolicy::HEALTH_ONLY:
            // Metabolic efficiency traits - only affected by health
            // Poor health reduces digestive/metabolic efficiency
            if (org.health < 0.5f) {
                float health_factor = 0.7f + (org.health / 0.5f) * 0.3f;
                return value * health_factor;
            }
            return value;
            
        case TraitModulationPolicy::ENERGY_GATED:
            // Production traits - phenotype returns raw capacity
            // Consumer checks energy surplus before using
            return value;
            
        case TraitModulationPolicy::CONSUMER_APPLIED:
            // Performance traits - phenotype returns raw value
            // Consumer applies context-specific modulation at use-time
            return value;
    }
    
    return value;
}

void Phenotype::computeAllTraits() const {
    if (!isValid()) {
        return;
    }
    
    // First, compute all direct gene traits
    for (const auto& [gene_id, definition] : registry_->getAllDefinitions()) {
        if (genome_->hasGene(gene_id)) {
            computeTrait(gene_id);
        }
    }
    
    // Then, find all unique trait targets from effect bindings
    std::unordered_map<std::string, bool> processed_traits;
    
    for (const auto& [gene_id, definition] : registry_->getAllDefinitions()) {
        for (const auto& effect : definition.getEffects()) {
            const std::string& trait = effect.target_trait;
            if (processed_traits.find(trait) == processed_traits.end()) {
                if (computed_traits_.find(trait) == computed_traits_.end()) {
                    computeTrait(trait);
                }
                processed_traits[trait] = true;
            }
        }
    }
}

// ============================================================================
// Emergent Diet Type Calculation (Phase 2.1)
// ============================================================================

DietType Phenotype::calculateDietType() const {
    if (!isValid()) {
        return DietType::OMNIVORE;  // Default fallback
    }
    
    // Use RAW trait values (no age/health/energy modulation) for stable classification
    // This ensures diet type doesn't change when creature is young, injured, or tired
    
    // Get effective digestion efficiencies (include morphology contributions via effect bindings)
    // These automatically include contributions from:
    // - PLANT_DIGESTION_EFFICIENCY gene (base)
    // - GUT_LENGTH → plant_digestion_efficiency (+0.35)
    // - TOOTH_GRINDING → plant_digestion_efficiency (+0.15)
    // - MEAT_DIGESTION_EFFICIENCY gene (base)
    // - STOMACH_ACIDITY → meat_digestion_efficiency (+0.35)
    // - TOOTH_SHARPNESS → meat_digestion_efficiency (+0.15)
    float plantEfficiency = computeTraitRaw("plant_digestion_efficiency");
    float meatEfficiency = computeTraitRaw("meat_digestion_efficiency");
    
    // Other traits for specialization checks
    float celluloseBreakdown = computeTraitRaw(UniversalGenes::CELLULOSE_BREAKDOWN);
    float colorVision = computeTraitRaw(UniversalGenes::COLOR_VISION);
    float toxinTolerance = computeTraitRaw(UniversalGenes::TOXIN_TOLERANCE);
    float combatAggression = computeTraitRaw(UniversalGenes::COMBAT_AGGRESSION);
    
    // Calculate total digestive capacity and ratios
    float totalDigestion = plantEfficiency + meatEfficiency;
    
    // Avoid division by zero - if no digestion capability, default to omnivore
    if (totalDigestion < 0.01f) {
        return DietType::OMNIVORE;
    }
    
    float meatRatio = meatEfficiency / totalDigestion;
    float plantRatio = plantEfficiency / totalDigestion;
    
    // Classification logic using 70% ratio threshold
    // Priority order: Necrovore > Carnivore > Frugivore > Herbivore > Omnivore
    
    // NECROVORE: High toxin tolerance (for eating rotting corpses), moderate meat digestion,
    // typically lower aggression (scavengers avoid confrontation)
    // Check this first as it's a specialized carnivore variant
    if (toxinTolerance > 0.7f && meatEfficiency > 0.4f && combatAggression < 0.4f) {
        return DietType::NECROVORE;
    }
    
    // CARNIVORE: Meat-dominant digestion (>70% of total capacity is meat)
    if (meatRatio >= 0.7f) {
        return DietType::CARNIVORE;
    }
    
    // HERBIVORE: Plant-dominant digestion (>70% of total capacity is plant)
    // Also requires some cellulose breakdown capability for true herbivore status
    if (plantRatio >= 0.7f && celluloseBreakdown > 0.3f) {
        return DietType::HERBIVORE;
    }
    
    // FRUGIVORE: Plant-dominant but low cellulose (fruit specialist)
    // High color vision for fruit detection, but can't process tough plant matter
    if (plantRatio >= 0.6f && colorVision > 0.5f && celluloseBreakdown < 0.3f) {
        return DietType::FRUGIVORE;
    }
    
    // OMNIVORE: Default - balanced capabilities (neither ratio exceeds 70%)
    return DietType::OMNIVORE;
}

const char* Phenotype::dietTypeToString(DietType type) {
    switch (type) {
        case DietType::HERBIVORE: return "Herbivore";
        case DietType::FRUGIVORE: return "Frugivore";
        case DietType::OMNIVORE:  return "Omnivore";
        case DietType::CARNIVORE: return "Carnivore";
        case DietType::NECROVORE: return "Necrovore";
        default: return "Unknown";
    }
}

// ============================================================================
// Energy Budget Methods (Phase 2.2)
// ============================================================================

float Phenotype::getTotalMaintenanceCost() const {
    if (!isValid()) {
        return 0.0f;
    }
    
    return energyBudget_.calculateMaintenanceCost(*genome_, *registry_);
}

float Phenotype::getSpecialistBonus() const {
    if (!isValid()) {
        return 1.0f;  // No bonus by default
    }
    
    return energyBudget_.calculateSpecialistBonus(*this);
}

float Phenotype::getMetabolicOverhead() const {
    if (!isValid()) {
        return 1.0f;  // No overhead by default
    }
    
    return energyBudget_.calculateMetabolicOverhead(*this);
}

const EnergyBudget& Phenotype::getEnergyBudget() const {
    return energyBudget_;
}

} // namespace Genetics
} // namespace EcoSim
