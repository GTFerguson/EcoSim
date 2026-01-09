#pragma once

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/PhenotypeCache.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/expression/EnergyBudget.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <unordered_map>
#include <string>

namespace EcoSim {
namespace Genetics {

// Forward declarations
class Genome;
class GeneDefinition;

/**
 * @brief The expressed traits of an organism based on its genome
 * 
 * Phenotype bridges genotype (Genome) to actual traits.
 * It handles:
 * - Expression based on dominance (complete, incomplete, codominant, overdominant)
 * - Age-dependent expression modulation
 * - Environmental effects on gene expression
 * - Caching for performance
 * 
 * The Phenotype uses dependency injection for both the Genome and GeneRegistry,
 * following the Dependency Inversion Principle (DIP).
 * 
 * Usage:
 * @code
 *   GeneRegistry registry;
 *   // ... register gene definitions ...
 *   
 *   Genome genome;
 *   // ... populate genome ...
 *   
 *   Phenotype phenotype(&genome, &registry);
 *   phenotype.updateContext(environment, organism_state);
 *   float speed = phenotype.getTrait("movement_speed");
 * @endcode
 */
class Phenotype {
public:
    /**
     * @brief Default constructor - creates empty phenotype
     */
    Phenotype();
    
    /**
     * @brief Construct phenotype from genome only
     * @param genome Pointer to the genome (not owned, must outlive Phenotype)
     */
    explicit Phenotype(const Genome* genome);
    
    /**
     * @brief Construct phenotype with both genome and registry
     * @param genome Pointer to the genome (not owned, must outlive Phenotype)
     * @param registry Pointer to the gene registry (not owned, must outlive Phenotype)
     */
    Phenotype(const Genome* genome, const GeneRegistry* registry);
    
    /**
     * @brief Set the genome this phenotype expresses
     * @param genome Pointer to the genome
     */
    void setGenome(const Genome* genome);
    
    /**
     * @brief Set the gene registry for looking up definitions
     * @param registry Pointer to the gene registry
     */
    void setRegistry(const GeneRegistry* registry);
    
    /**
     * @brief Update context for expression calculations
     * @param env Current environment state
     * @param org Current organism state
     * 
     * This should be called when environment or organism state changes.
     * The phenotype will automatically invalidate affected cached values.
     */
    void updateContext(const EnvironmentState& env, const OrganismState& org);
    
    /**
     * @brief Get expressed trait value (uses cache)
     * @param trait_id The ID of the trait to retrieve
     * @return The computed trait value, or 0.0f if trait doesn't exist
     * 
     * Returns cached value if available, otherwise computes and caches.
     */
    float getTrait(const std::string& trait_id) const;
    
    /**
     * @brief Get trait with explicit computation (ignores cache)
     * @param trait_id The ID of the trait to compute
     * @return The computed trait value
     */
    float computeTrait(const std::string& trait_id) const;
    
    /**
     * @brief Compute trait value WITHOUT age/health/energy modulation
     * @param trait_id The ID of the trait to compute
     * @return The raw genetic expression value (unaffected by organism state)
     *
     * Use this for identity-based classification (diet type, scientific name)
     * where the trait should remain stable regardless of the organism's
     * current condition. This gives the "true genetic potential" value.
     */
    float computeTraitRaw(const std::string& trait_id) const;
    
    /**
     * @brief Check if a trait can be computed
     * @param trait_id The trait ID to check
     * @return true if the trait exists and can be computed
     */
    bool hasTrait(const std::string& trait_id) const;
    
    /**
     * @brief Invalidate all cached values
     * 
     * Call when genome or context changes significantly.
     */
    void invalidateCache();
    
    /**
     * @brief Get all computed traits
     * @return Map of trait IDs to their computed values
     * 
     * Note: This triggers computation for all registered genes.
     */
    const std::unordered_map<std::string, float>& getAllTraits() const;
    
    /**
     * @brief Get cache hit rate for diagnostics
     * @return Ratio of cache hits to total lookups (0.0 to 1.0)
     */
    float getCacheHitRate() const;
    
    /**
     * @brief Check if phenotype is valid (has genome and registry)
     * @return true if both genome and registry are set
     */
    bool isValid() const;
    
    /**
     * @brief Get the current health percentage from organism state
     * @return Health value (0.0 to 1.0) where 1.0 is full health
     *
     * This is provided for consumers that need to apply health-based
     * modulation at use-time (CONSUMER_APPLIED traits).
     */
    float getHealth() const { return organism_state_.health; }
    
    /**
     * @brief Calculate emergent diet type from digestion genes
     * @return The emergent DietType based on digestive capabilities
     *
     * This replaces the deprecated categorical DIET_TYPE gene with an emergent
     * classification based on actual gene values:
     * - CARNIVORE: High meat digestion (>0.7), low plant digestion (<0.3)
     * - HERBIVORE: High plant digestion (>0.7), high cellulose breakdown (>0.5)
     * - FRUGIVORE: High plant digestion (>0.5), high color vision (>0.6), low cellulose (<0.3)
     * - OMNIVORE: Balanced or intermediate capabilities (default)
     *
     * This creates smooth transitions between diet types through evolution.
     */
    DietType calculateDietType() const;
    
    /**
     * @brief Get diet type as a string for debugging/display
     * @return String representation of the diet type
     */
    static const char* dietTypeToString(DietType type);
    
    // ========== Energy Budget Methods (Phase 2.2) ==========
    
    /**
     * @brief Get total maintenance cost for all expressed genes
     * @return Total energy cost per tick to maintain gene expression
     *
     * Iterates through all genes in the genome and sums their maintenance
     * costs, weighted by expression level and cost scaling.
     * Formula per gene: maintenanceCost * pow(expressionLevel, costScaling)
     */
    float getTotalMaintenanceCost() const;
    
    /**
     * @brief Calculate specialist efficiency bonus for dietary specialization
     * @return Multiplier for food extraction efficiency (1.0 to 1.3)
     *
     * Organisms that specialize in one food type (high plant OR high meat
     * digestion, not both) get up to 30% bonus extraction efficiency.
     * This rewards specialization over generalist strategies.
     */
    float getSpecialistBonus() const;
    
    /**
     * @brief Calculate metabolic overhead for maintaining multiple enzyme systems
     * @return Multiplier for base metabolic cost (1.0 to ~1.36)
     *
     * Organisms maintaining multiple digestive enzyme systems pay a compounding
     * overhead of 8% per active system (threshold > 0.3 expression):
     * - Plant digestion system
     * - Meat digestion system
     * - Cellulose breakdown system
     * - Toxin metabolism system
     *
     * This creates a cost for being a generalist.
     */
    float getMetabolicOverhead() const;
    
    /**
     * @brief Get the EnergyBudget calculator for more advanced calculations
     * @return Reference to internal EnergyBudget instance
     */
    const EnergyBudget& getEnergyBudget() const;
    
    /**
     * @brief Get the current environment state
     * @return Reference to the stored environment state
     *
     * This provides access to the environment state that was passed to
     * updateContext(), allowing external systems (like stress calculation)
     * to access environmental parameters.
     */
    const EnvironmentState& getEnvironment() const { return environment_; }

private:
    const Genome* genome_ = nullptr;
    const GeneRegistry* registry_ = nullptr;
    mutable PhenotypeCache cache_;
    EnvironmentState environment_;
    OrganismState organism_state_;
    mutable std::unordered_map<std::string, float> computed_traits_;
    EnergyBudget energyBudget_;  // Phase 2.2: Energy budget calculator
    
    /**
     * @brief Express a single gene based on its definition
     * @param gene_id The gene ID to express
     * @param definition The gene's definition containing dominance info
     * @return The raw expressed value before modulation
     */
    float expressGene(const std::string& gene_id, 
                      const GeneDefinition& definition) const;
    
    /**
     * @brief Apply age-based modulation to a trait value
     * @param value The base trait value
     * @param age_normalized Normalized age (0.0 = birth, 1.0 = max lifespan)
     * @return Modulated value
     * 
     * Age modulation curve:
     * - 0.0-0.1 (juvenile): 60% expression
     * - 0.1-0.8 (adult): 100% expression  
     * - 0.8-1.0 (elderly): Linear decline to 80%
     */
    float applyAgeModulation(float value, float age_normalized) const;
    
    /**
     * @brief Apply environment-based modulation to a trait value
     * @param value The base trait value
     * @param trait_id The trait being modulated (for trait-specific effects)
     * @param env Current environment state
     * @return Modulated value
     */
    float applyEnvironmentModulation(float value, const std::string& trait_id,
                                     const EnvironmentState& env) const;
    
    /**
     * @brief Apply health and energy modulation based on trait's policy
     * @param value The base trait value
     * @param trait_id The trait being modulated (to look up policy)
     * @param org Current organism state
     * @return Modulated value based on the gene's TraitModulationPolicy:
     *         - NEVER: Returns unmodified value (physical structure)
     *         - HEALTH_ONLY: Applies only health-based modulation (metabolic efficiency)
     *         - ENERGY_GATED: Returns unmodified (consumer checks energy)
     *         - CONSUMER_APPLIED: Returns unmodified (consumer applies context-specific modulation)
     */
    float applyOrganismStateModulation(float value, const std::string& trait_id,
                                       const OrganismState& org) const;
    
    /**
     * @brief Compute all traits from the genome
     * 
     * Internal method that iterates over all genes and computes their expression.
     */
    void computeAllTraits() const;
};

} // namespace Genetics
} // namespace EcoSim
