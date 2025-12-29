#pragma once

#include "Gene.hpp"
#include "GeneticTypes.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <random>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Chromosome class - Collection of linked genes
 * 
 * Represents a chromosome containing multiple genes that are
 * inherited together (with some recombination during crossover).
 */
class Chromosome {
public:
    /**
     * @brief Construct an empty chromosome of the given type
     * @param type The chromosome type (e.g., Morphology, Metabolism)
     */
    explicit Chromosome(ChromosomeType type);
    
    /**
     * @brief Get the chromosome type
     * @return The ChromosomeType enum value
     */
    ChromosomeType getType() const { return type_; }
    
    /**
     * @brief Add a gene to this chromosome
     * @param gene The gene to add
     * @throws std::runtime_error if gene with same ID already exists
     */
    void addGene(const Gene& gene);
    
    /**
     * @brief Check if chromosome contains a gene with given ID
     * @param gene_id The gene identifier to check
     * @return true if gene exists
     */
    bool hasGene(const std::string& gene_id) const;
    
    /**
     * @brief Get a gene by ID (O(1) lookup)
     * @param gene_id The gene identifier
     * @return Optional containing the gene if found
     */
    std::optional<std::reference_wrapper<const Gene>> getGene(const std::string& gene_id) const;
    
    /**
     * @brief Get a mutable reference to a gene by ID
     * @param gene_id The gene identifier
     * @return Optional containing mutable gene reference if found
     */
    std::optional<std::reference_wrapper<Gene>> getGeneMutable(const std::string& gene_id);
    
    /**
     * @brief Get the number of genes in this chromosome
     * @return Gene count
     */
    size_t size() const { return genes_.size(); }
    
    /**
     * @brief Get all genes (for iteration)
     * @return Reference to genes vector
     */
    const std::vector<Gene>& getGenes() const { return genes_; }
    
    /**
     * @brief Mutate all genes on this chromosome
     * @param mutation_rate Probability of mutation per gene
     * @param definitions Map of gene IDs to their definitions (for limits)
     */
    void mutate(float mutation_rate, 
                const std::unordered_map<std::string, GeneDefinition>& definitions);
    
    /**
     * @brief Create offspring chromosome from two parent chromosomes
     * 
     * Implements linked inheritance with potential recombination.
     * @param parent1 First parent chromosome
     * @param parent2 Second parent chromosome
     * @param recombination_rate Probability of crossover between adjacent genes
     * @return New chromosome with genes from both parents
     */
    static Chromosome crossover(const Chromosome& parent1,
                                const Chromosome& parent2,
                                float recombination_rate);
    
    // ========================================================================
    // Serialization
    // ========================================================================
    
    /**
     * @brief Serialize chromosome to JSON
     * @return JSON object with chromosome data
     */
    nlohmann::json toJson() const;
    
    /**
     * @brief Create chromosome from JSON data
     * @param j JSON object with chromosome data
     * @return Constructed Chromosome object
     * @throws std::runtime_error if JSON is invalid or missing required fields
     */
    static Chromosome fromJson(const nlohmann::json& j);

private:
    ChromosomeType type_;
    std::vector<Gene> genes_;
    std::unordered_map<std::string, size_t> gene_index_;  // O(1) lookup
    
    // Get thread-local random engine
    static std::mt19937& getRandomEngine();
    
    /**
     * @brief Convert ChromosomeType to string for serialization
     */
    static std::string typeToString(ChromosomeType type);
    
    /**
     * @brief Convert string to ChromosomeType for deserialization
     */
    static ChromosomeType stringToType(const std::string& str);
};

} // namespace Genetics
} // namespace EcoSim
