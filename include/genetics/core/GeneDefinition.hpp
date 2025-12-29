#pragma once

#include "GeneticTypes.hpp"
#include <string>
#include <vector>
#include <cmath>

namespace EcoSim {
namespace Genetics {

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
};

} // namespace Genetics
} // namespace EcoSim
