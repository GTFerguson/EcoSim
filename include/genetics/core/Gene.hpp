#pragma once

#include "GeneticTypes.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <random>
#include <cmath>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Gene class - Holds two Allele values (diploid organism)
 * 
 * Represents a single gene with two alleles, supporting various
 * dominance patterns for expression.
 */
class Gene {
public:
    /**
     * @brief Construct a Gene with two alleles (heterozygous or homozygous)
     * @param gene_id Unique identifier for this gene
     * @param allele1 First allele
     * @param allele2 Second allele
     */
    Gene(const std::string& gene_id, const Allele& allele1, const Allele& allele2);
    
    /**
     * @brief Construct a homozygous Gene with a single value
     * @param gene_id Unique identifier for this gene
     * @param value The value for both alleles
     */
    Gene(const std::string& gene_id, const GeneValue& value);
    
    /**
     * @brief Get the gene identifier
     * @return The gene ID string
     */
    const std::string& getId() const { return id_; }
    
    /**
     * @brief Get the first allele
     * @return Reference to allele 1
     */
    const Allele& getAllele1() const { return allele1_; }
    
    /**
     * @brief Get the second allele
     * @return Reference to allele 2
     */
    const Allele& getAllele2() const { return allele2_; }
    
    /**
     * @brief Check if this gene is heterozygous (alleles differ)
     * @return true if alleles have different values
     */
    bool isHeterozygous() const;
    
    /**
     * @brief Get the expressed value based on dominance type
     * @param dominance The dominance pattern to apply
     * @return The expressed GeneValue
     */
    GeneValue getExpressedValue(DominanceType dominance) const;
    
    /**
     * @brief Get numeric value for continuous genes
     * @param dominance The dominance pattern to apply
     * @return Float value based on expression
     */
    float getNumericValue(DominanceType dominance) const;
    
    /**
     * @brief Apply random mutation to the gene
     * @param mutation_rate Probability of mutation (0.0 to 1.0)
     * @param limits Gene limits for constraining mutations
     */
    void mutate(float mutation_rate, const GeneLimits& limits);
    
    /**
     * @brief Create offspring gene from two parent genes
     * @param parent1 First parent's gene
     * @param parent2 Second parent's gene
     * @return New Gene with one allele from each parent
     */
    static Gene crossover(const Gene& parent1, const Gene& parent2);
    
    /**
     * @brief Set both alleles to the same value (homozygous)
     * @param value The value to set
     *
     * Used by PlantFactory to apply species template values.
     */
    void setAlleleValues(float value);
    
    /**
     * @brief Set allele values individually (heterozygous)
     * @param value1 Value for allele 1
     * @param value2 Value for allele 2
     */
    void setAlleleValues(float value1, float value2);
    
    // ========================================================================
    // Serialization
    // ========================================================================
    
    /**
     * @brief Serialize gene to JSON
     * @return JSON object with gene data
     */
    nlohmann::json toJson() const;
    
    /**
     * @brief Create gene from JSON data
     * @param j JSON object with gene data
     * @return Constructed Gene object
     * @throws std::runtime_error if JSON is invalid or missing required fields
     */
    static Gene fromJson(const nlohmann::json& j);

private:
    std::string id_;
    Allele allele1_;
    Allele allele2_;
    
    // Helper to extract numeric value from a GeneValue
    static float toNumeric(const GeneValue& value);
    
    // Get thread-local random engine
    static std::mt19937& getRandomEngine();
};


/**
 * @brief GeneDefinition class - Blueprint for gene types
 *
 * Defines the properties and constraints for a type of gene,
 * including its limits, dominance pattern, and effects on traits.
 */
class GeneDefinition {
public:
    /**
     * @brief Construct a gene definition
     * @param id Unique identifier for this gene type
     * @param chromosome Which chromosome this gene belongs to
     * @param limits The value constraints for this gene
     * @param dominance The dominance pattern for expression
     */
    GeneDefinition(const std::string& id,
                   ChromosomeType chromosome,
                   const GeneLimits& limits,
                   DominanceType dominance = DominanceType::Incomplete);
    
    /**
     * @brief Get the gene definition ID
     */
    const std::string& getId() const { return id_; }
    
    /**
     * @brief Get the chromosome type
     */
    ChromosomeType getChromosome() const { return chromosome_; }
    
    /**
     * @brief Get the gene limits
     */
    const GeneLimits& getLimits() const { return limits_; }
    
    /**
     * @brief Get the dominance type
     */
    DominanceType getDominance() const { return dominance_; }
    
    /**
     * @brief Get all effect bindings
     */
    const std::vector<EffectBinding>& getEffects() const { return effects_; }
    
    /**
     * @brief Add an effect binding for pleiotropy
     * @param effect The effect to add
     */
    void addEffect(const EffectBinding& effect);
    
    /**
     * @brief Create a random value within limits
     * @return Random GeneValue (as float)
     */
    GeneValue createRandomValue() const;
    
    /**
     * @brief Clamp a value to be within limits
     * @param value The value to clamp
     * @return Clamped value
     */
    float clampValue(float value) const;
    
    /**
     * @brief Apply small random variation (creep) to a value
     * @param value The base value
     * @return Value with random variation applied, clamped to limits
     */
    float applyCreep(float value) const;
    
    // ========================================================================
    // Energy Budget System
    // ========================================================================
    
    /**
     * @brief Get the maintenance cost per tick at expression 1.0
     * @return Base energy cost per tick
     */
    float getMaintenanceCost() const { return maintenanceCost_; }
    
    /**
     * @brief Set the maintenance cost for this gene
     * @param cost Energy cost per tick at expression 1.0
     */
    void setMaintenanceCost(float cost) { maintenanceCost_ = cost; }
    
    /**
     * @brief Get the cost scaling factor
     * @return How cost scales with expression (1.0=linear, >1=exponential)
     */
    float getCostScaling() const { return costScaling_; }
    
    /**
     * @brief Set the cost scaling factor
     * @param scaling How cost scales with expression (1.0=linear, >1=exponential)
     */
    void setCostScaling(float scaling) { costScaling_ = scaling; }
    
    /**
     * @brief Calculate actual maintenance cost for a given expression level
     * @param expressionLevel The current expression level (0.0 to 1.0)
     * @return Actual energy cost per tick
     *
     * Formula: maintenanceCost * pow(expressionLevel, costScaling)
     */
    float calculateMaintenanceCost(float expressionLevel) const;
    
    // ========================================================================
    // Trait Modulation Policy
    // ========================================================================
    
    /**
     * @brief Get the modulation policy for this gene
     * @return How organism state affects trait expression
     */
    TraitModulationPolicy getModulationPolicy() const { return modulationPolicy_; }
    
    /**
     * @brief Set the modulation policy for this gene
     * @param policy How organism state should affect trait expression
     */
    void setModulationPolicy(TraitModulationPolicy policy) { modulationPolicy_ = policy; }

private:
    std::string id_;
    ChromosomeType chromosome_;
    GeneLimits limits_;
    DominanceType dominance_;
    std::vector<EffectBinding> effects_;
    
    // Energy budget fields
    float maintenanceCost_ = 0.0f;  // Energy cost per tick at expression 1.0
    float costScaling_ = 1.5f;       // How cost scales with expression (1.0=linear, >1=exponential)
    
    // Trait modulation policy (default: immutable physical structure)
    TraitModulationPolicy modulationPolicy_ = TraitModulationPolicy::NEVER;
    
    // Get thread-local random engine
    static std::mt19937& getRandomEngine();
};

} // namespace Genetics
} // namespace EcoSim
