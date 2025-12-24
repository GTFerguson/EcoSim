#include "genetics/core/Genome.hpp"
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

Genome::Genome()
    : chromosomes_{{
        Chromosome(ChromosomeType::Morphology),
        Chromosome(ChromosomeType::Sensory),
        Chromosome(ChromosomeType::Metabolism),
        Chromosome(ChromosomeType::Locomotion),
        Chromosome(ChromosomeType::Behavior),
        Chromosome(ChromosomeType::Reproduction),
        Chromosome(ChromosomeType::Environmental),
        Chromosome(ChromosomeType::Lifespan)
    }} {
}

size_t Genome::chromosomeIndex(ChromosomeType type) {
    switch (type) {
        case ChromosomeType::Morphology:    return 0;
        case ChromosomeType::Sensory:       return 1;
        case ChromosomeType::Metabolism:    return 2;
        case ChromosomeType::Locomotion:    return 3;
        case ChromosomeType::Behavior:      return 4;
        case ChromosomeType::Reproduction:  return 5;
        case ChromosomeType::Environmental: return 6;
        case ChromosomeType::Lifespan:      return 7;
        default:
            throw std::invalid_argument("Invalid chromosome type");
    }
}

const Chromosome& Genome::getChromosome(ChromosomeType type) const {
    return chromosomes_[chromosomeIndex(type)];
}

Chromosome& Genome::getChromosomeMutable(ChromosomeType type) {
    cache_valid_ = false;  // Chromosome may be modified
    return chromosomes_[chromosomeIndex(type)];
}

void Genome::rebuildCache() const {
    gene_cache_.clear();
    
    for (size_t i = 0; i < NUM_CHROMOSOMES; ++i) {
        const auto& chromosome = chromosomes_[i];
        ChromosomeType type = chromosome.getType();
        const auto& genes = chromosome.getGenes();
        
        for (size_t j = 0; j < genes.size(); ++j) {
            gene_cache_[genes[j].getId()] = std::make_pair(type, j);
        }
    }
    
    cache_valid_ = true;
}

bool Genome::hasGene(const std::string& gene_id) const {
    if (!cache_valid_) {
        rebuildCache();
    }
    return gene_cache_.find(gene_id) != gene_cache_.end();
}

const Gene& Genome::getGene(const std::string& gene_id) const {
    auto opt = tryGetGene(gene_id);
    if (!opt.has_value()) {
        throw std::out_of_range("Gene '" + gene_id + "' not found in genome");
    }
    return opt->get();
}

Gene& Genome::getGeneMutable(const std::string& gene_id) {
    if (!cache_valid_) {
        rebuildCache();
    }
    
    auto it = gene_cache_.find(gene_id);
    if (it == gene_cache_.end()) {
        throw std::out_of_range("Gene '" + gene_id + "' not found in genome");
    }
    
    ChromosomeType chr_type = it->second.first;
    auto& chromosome = chromosomes_[chromosomeIndex(chr_type)];
    auto gene_opt = chromosome.getGeneMutable(gene_id);
    
    if (!gene_opt.has_value()) {
        throw std::out_of_range("Gene '" + gene_id + "' cache inconsistency");
    }
    
    return gene_opt->get();
}

std::optional<std::reference_wrapper<const Gene>> 
Genome::tryGetGene(const std::string& gene_id) const {
    if (!cache_valid_) {
        rebuildCache();
    }
    
    auto it = gene_cache_.find(gene_id);
    if (it == gene_cache_.end()) {
        return std::nullopt;
    }
    
    ChromosomeType chr_type = it->second.first;
    const auto& chromosome = chromosomes_[chromosomeIndex(chr_type)];
    return chromosome.getGene(gene_id);
}

void Genome::addGene(const Gene& gene, ChromosomeType chromosome) {
    cache_valid_ = false;  // Invalidate cache before modification
    chromosomes_[chromosomeIndex(chromosome)].addGene(gene);
}

std::vector<std::reference_wrapper<const Gene>> Genome::getAllGenes() const {
    std::vector<std::reference_wrapper<const Gene>> all_genes;
    
    for (const auto& chromosome : chromosomes_) {
        for (const auto& gene : chromosome.getGenes()) {
            all_genes.push_back(std::cref(gene));
        }
    }
    
    return all_genes;
}

size_t Genome::getTotalGeneCount() const {
    size_t count = 0;
    for (const auto& chromosome : chromosomes_) {
        count += chromosome.size();
    }
    return count;
}

Genome Genome::crossover(const Genome& parent1, const Genome& parent2,
                          float recombination_rate) {
    Genome offspring;
    
    // Cross over each chromosome pair
    for (size_t i = 0; i < NUM_CHROMOSOMES; ++i) {
        offspring.chromosomes_[i] = Chromosome::crossover(
            parent1.chromosomes_[i],
            parent2.chromosomes_[i],
            recombination_rate
        );
    }
    
    // Offspring cache is invalid until rebuilt
    offspring.cache_valid_ = false;
    
    return offspring;
}

void Genome::mutate(float mutation_rate,
                    const std::unordered_map<std::string, GeneDefinition>& definitions) {
    for (auto& chromosome : chromosomes_) {
        chromosome.mutate(mutation_rate, definitions);
    }
    // Note: Mutation doesn't add/remove genes, so cache remains valid
}

float Genome::compare(const Genome& other) const {
    // Collect all unique gene IDs from both genomes
    std::unordered_map<std::string, bool> all_gene_ids;
    
    for (const auto& chromosome : chromosomes_) {
        for (const auto& gene : chromosome.getGenes()) {
            all_gene_ids[gene.getId()] = true;
        }
    }
    
    for (const auto& chromosome : other.chromosomes_) {
        for (const auto& gene : chromosome.getGenes()) {
            all_gene_ids[gene.getId()] = true;
        }
    }
    
    if (all_gene_ids.empty()) {
        return 1.0f;  // Two empty genomes are identical
    }
    
    float total_similarity = 0.0f;
    size_t compared_count = 0;
    
    for (const auto& [gene_id, _] : all_gene_ids) {
        auto gene1_opt = tryGetGene(gene_id);
        auto gene2_opt = other.tryGetGene(gene_id);
        
        if (gene1_opt.has_value() && gene2_opt.has_value()) {
            // Both genomes have this gene - compare numeric values
            // Use Incomplete dominance for comparison (blended expression)
            float val1 = gene1_opt->get().getNumericValue(DominanceType::Incomplete);
            float val2 = gene2_opt->get().getNumericValue(DominanceType::Incomplete);
            
            // Calculate similarity: 1.0 - normalized_difference
            // For equal values, similarity = 1.0
            // For max difference, we cap at 0.0
            float max_val = std::max(std::abs(val1), std::abs(val2));
            if (max_val > 0.0f) {
                float diff = std::abs(val1 - val2);
                float normalized_diff = diff / (2.0f * max_val);  // Normalize to [0, 1]
                total_similarity += (1.0f - std::min(normalized_diff, 1.0f));
            } else {
                // Both values are 0, so they're identical
                total_similarity += 1.0f;
            }
            compared_count++;
        } else {
            // Gene exists in only one genome - contributes 0 similarity
            total_similarity += 0.0f;
            compared_count++;
        }
    }
    
    if (compared_count == 0) {
        return 1.0f;  // Edge case: no genes to compare
    }
    
    return total_similarity / static_cast<float>(compared_count);
}

} // namespace Genetics
} // namespace EcoSim
