#pragma once

#include "genetics/core/Gene.hpp"
#include <unordered_map>
#include <string>
#include <optional>
#include <functional>
#include <vector>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Registry for GeneDefinition objects
 * 
 * This class stores gene blueprints and is passed via dependency injection
 * rather than using a singleton pattern (per DIP principle).
 * 
 * The registry provides:
 * - Storage for GeneDefinition objects
 * - Lookup by gene ID
 * - Lookup by chromosome type
 * - Thread-safe read access (definitions are immutable once registered)
 */
class GeneRegistry {
public:
    GeneRegistry() = default;
    
    // Disable copy to prevent accidental duplication of the registry
    GeneRegistry(const GeneRegistry&) = delete;
    GeneRegistry& operator=(const GeneRegistry&) = delete;
    
    // Allow move semantics
    GeneRegistry(GeneRegistry&&) = default;
    GeneRegistry& operator=(GeneRegistry&&) = default;
    
    /**
     * @brief Register a gene definition
     * @param definition The gene definition to register
     * @throws std::runtime_error if gene ID already exists
     */
    void registerGene(const GeneDefinition& definition);
    
    /**
     * @brief Register a gene definition (move version)
     * @param definition The gene definition to register
     * @throws std::runtime_error if gene ID already exists
     */
    void registerGene(GeneDefinition&& definition);
    
    /**
     * @brief Try to register a gene definition (non-throwing)
     *
     * If the gene already exists, this method logs a warning and returns false
     * instead of throwing an exception. Use this when duplicate registration
     * should be handled gracefully (e.g., multiple components registering defaults).
     *
     * @param definition The gene definition to register
     * @return true if registration succeeded, false if gene already exists
     */
    bool tryRegisterGene(const GeneDefinition& definition);
    
    /**
     * @brief Try to register a gene definition (move version, non-throwing)
     * @param definition The gene definition to register
     * @return true if registration succeeded, false if gene already exists
     */
    bool tryRegisterGene(GeneDefinition&& definition);
    
    /**
     * @brief Check if a gene definition exists
     * @param gene_id The gene ID to check
     * @return true if the gene definition exists
     */
    bool hasGene(const std::string& gene_id) const;
    
    /**
     * @brief Get a gene definition by ID
     * @param gene_id The gene ID to look up
     * @return Const reference to the gene definition
     * @throws std::out_of_range if gene ID not found
     */
    const GeneDefinition& getDefinition(const std::string& gene_id) const;
    
    /**
     * @brief Try to get a gene definition (non-throwing)
     * @param gene_id The gene ID to look up
     * @return Optional reference to the definition, or nullopt if not found
     */
    std::optional<std::reference_wrapper<const GeneDefinition>> 
        tryGetDefinition(const std::string& gene_id) const;
    
    /**
     * @brief Get all registered definitions
     * @return Const reference to the internal map
     */
    const std::unordered_map<std::string, GeneDefinition>& getAllDefinitions() const;
    
    /**
     * @brief Get all definitions for a specific chromosome
     * @param type The chromosome type to filter by
     * @return Vector of references to matching definitions
     */
    std::vector<std::reference_wrapper<const GeneDefinition>> 
        getDefinitionsForChromosome(ChromosomeType type) const;
    
    /**
     * @brief Get all gene IDs
     * @return Vector of all registered gene IDs
     */
    std::vector<std::string> getAllGeneIds() const;
    
    /**
     * @brief Clear all definitions
     */
    void clear();
    
    /**
     * @brief Get the number of registered definitions
     * @return Number of definitions
     */
    size_t size() const;
    
    /**
     * @brief Check if the registry is empty
     * @return true if no definitions are registered
     */
    bool empty() const;
    
    /**
     * @brief Mark that default genes have been registered
     *
     * Called by UniversalGenes::registerDefaults() after successfully registering
     * all default genes. This allows the method to be idempotent - subsequent
     * calls will return early without attempting to re-register genes.
     */
    void markDefaultsRegistered();
    
    /**
     * @brief Check if default genes have been registered
     * @return true if markDefaultsRegistered() has been called
     */
    bool areDefaultsRegistered() const;
    
private:
    std::unordered_map<std::string, GeneDefinition> definitions_;
    bool defaultsRegistered_ = false;
};

} // namespace Genetics
} // namespace EcoSim
