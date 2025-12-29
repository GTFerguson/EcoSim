#pragma once

#include "GeneticTypes.hpp"
// Backward compatibility: GeneDefinition was moved to its own file
#include "GeneDefinition.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
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
};

} // namespace Genetics
} // namespace EcoSim
