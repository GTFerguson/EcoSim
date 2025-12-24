#pragma once

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/Chromosome.hpp"
#include <array>
#include <unordered_map>
#include <optional>
#include <functional>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Complete genetic makeup of an organism
 * 
 * The Genome class manages all 8 chromosomes and provides:
 * - Gene lookup across all chromosomes
 * - Reproduction (crossover between two genomes)
 * - Mutation
 * - Comparison for similarity
 */
class Genome {
public:
    Genome();
    
    // Access chromosomes by type
    const Chromosome& getChromosome(ChromosomeType type) const;
    Chromosome& getChromosomeMutable(ChromosomeType type);
    
    // Gene access across all chromosomes
    bool hasGene(const std::string& gene_id) const;
    const Gene& getGene(const std::string& gene_id) const;
    Gene& getGeneMutable(const std::string& gene_id);
    std::optional<std::reference_wrapper<const Gene>> tryGetGene(const std::string& gene_id) const;
    
    // Add gene to appropriate chromosome
    void addGene(const Gene& gene, ChromosomeType chromosome);
    
    // Get all genes (flattened view)
    std::vector<std::reference_wrapper<const Gene>> getAllGenes() const;
    size_t getTotalGeneCount() const;
    
    // Reproduction - create offspring from two parent genomes
    static Genome crossover(const Genome& parent1, const Genome& parent2,
                            float recombination_rate = 0.5f);
    
    // Apply mutation to all chromosomes
    void mutate(float mutation_rate,
                const std::unordered_map<std::string, GeneDefinition>& definitions);
    
    // Calculate genetic similarity (0.0 = completely different, 1.0 = identical)
    float compare(const Genome& other) const;
    
    // Iteration support
    using iterator = std::array<Chromosome, NUM_CHROMOSOMES>::iterator;
    using const_iterator = std::array<Chromosome, NUM_CHROMOSOMES>::const_iterator;
    
    iterator begin() { return chromosomes_.begin(); }
    iterator end() { return chromosomes_.end(); }
    const_iterator begin() const { return chromosomes_.begin(); }
    const_iterator end() const { return chromosomes_.end(); }
    const_iterator cbegin() const { return chromosomes_.cbegin(); }
    const_iterator cend() const { return chromosomes_.cend(); }

private:
    std::array<Chromosome, NUM_CHROMOSOMES> chromosomes_;
    
    // Cache for O(1) gene lookup across chromosomes
    mutable std::unordered_map<std::string, std::pair<ChromosomeType, size_t>> gene_cache_;
    mutable bool cache_valid_ = false;
    
    void rebuildCache() const;
    static size_t chromosomeIndex(ChromosomeType type);
};

} // namespace Genetics
} // namespace EcoSim
