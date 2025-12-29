#include "genetics/core/Chromosome.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

Chromosome::Chromosome(ChromosomeType type)
    : type_(type) {
}

void Chromosome::addGene(const Gene& gene) {
    const std::string& id = gene.getId();
    
    // Check if gene already exists
    if (gene_index_.find(id) != gene_index_.end()) {
        throw std::runtime_error("Gene with ID '" + id + "' already exists in chromosome");
    }
    
    // Add gene and update index
    size_t index = genes_.size();
    genes_.push_back(gene);
    gene_index_[id] = index;
}

bool Chromosome::hasGene(const std::string& gene_id) const {
    return gene_index_.find(gene_id) != gene_index_.end();
}

std::optional<std::reference_wrapper<const Gene>> 
Chromosome::getGene(const std::string& gene_id) const {
    auto it = gene_index_.find(gene_id);
    if (it == gene_index_.end()) {
        return std::nullopt;
    }
    return std::cref(genes_[it->second]);
}

std::optional<std::reference_wrapper<Gene>> 
Chromosome::getGeneMutable(const std::string& gene_id) {
    auto it = gene_index_.find(gene_id);
    if (it == gene_index_.end()) {
        return std::nullopt;
    }
    return std::ref(genes_[it->second]);
}

void Chromosome::mutate(float mutation_rate,
                        const std::unordered_map<std::string, GeneDefinition>& definitions) {
    for (auto& gene : genes_) {
        auto def_it = definitions.find(gene.getId());
        if (def_it != definitions.end()) {
            gene.mutate(mutation_rate, def_it->second.getLimits());
        }
    }
}

Chromosome Chromosome::crossover(const Chromosome& parent1,
                                  const Chromosome& parent2,
                                  float recombination_rate) {
    // Validate chromosome types match
    if (parent1.type_ != parent2.type_) {
        throw std::invalid_argument("Cannot crossover chromosomes of different types");
    }
    
    Chromosome offspring(parent1.type_);
    auto& rng = getRandomEngine();
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Track which parent we're currently copying from (for linked inheritance)
    bool from_parent1 = (dist(rng) < 0.5f);  // Start with random parent
    
    // Process genes in order (maintaining linkage)
    const auto& genes1 = parent1.genes_;
    const auto& genes2 = parent2.genes_;
    
    // Build a combined list of gene IDs from both parents
    std::vector<std::string> all_gene_ids;
    for (const auto& gene : genes1) {
        all_gene_ids.push_back(gene.getId());
    }
    for (const auto& gene : genes2) {
        if (!parent1.hasGene(gene.getId())) {
            all_gene_ids.push_back(gene.getId());
        }
    }
    
    for (const auto& gene_id : all_gene_ids) {
        // Check for recombination event (switch which parent we copy from)
        if (dist(rng) < recombination_rate) {
            from_parent1 = !from_parent1;
        }
        
        // Get gene from appropriate parent
        const Chromosome& source = from_parent1 ? parent1 : parent2;
        const Chromosome& other = from_parent1 ? parent2 : parent1;
        
        auto gene_opt = source.getGene(gene_id);
        if (!gene_opt.has_value()) {
            // Gene not in preferred parent, try the other
            gene_opt = other.getGene(gene_id);
        }
        
        if (gene_opt.has_value()) {
            // If both parents have this gene, do allele-level crossover
            auto other_gene_opt = other.getGene(gene_id);
            if (other_gene_opt.has_value()) {
                Gene crossed = Gene::crossover(gene_opt->get(), other_gene_opt->get());
                offspring.addGene(crossed);
            } else {
                // Only one parent has this gene
                offspring.addGene(gene_opt->get());
            }
        }
    }
    
    return offspring;
}

std::mt19937& Chromosome::getRandomEngine() {
    thread_local static std::mt19937 engine(std::random_device{}());
    return engine;
}

// ============================================================================
// Chromosome Serialization
// ============================================================================

nlohmann::json Chromosome::toJson() const {
    nlohmann::json j;
    j["type"] = typeToString(type_);
    
    nlohmann::json genes_array = nlohmann::json::array();
    for (const auto& gene : genes_) {
        genes_array.push_back(gene.toJson());
    }
    j["genes"] = genes_array;
    
    return j;
}

Chromosome Chromosome::fromJson(const nlohmann::json& j) {
    // Validate required fields
    if (!j.contains("type")) {
        throw std::runtime_error("Chromosome::fromJson: missing required field 'type'");
    }
    if (!j.contains("genes")) {
        throw std::runtime_error("Chromosome::fromJson: missing required field 'genes'");
    }
    
    std::string type_str = j.at("type").get<std::string>();
    ChromosomeType type = stringToType(type_str);
    
    Chromosome chromosome(type);
    
    for (const auto& gene_json : j.at("genes")) {
        Gene gene = Gene::fromJson(gene_json);
        chromosome.addGene(gene);
    }
    
    return chromosome;
}

std::string Chromosome::typeToString(ChromosomeType type) {
    switch (type) {
        case ChromosomeType::Morphology:    return "Morphology";
        case ChromosomeType::Sensory:       return "Sensory";
        case ChromosomeType::Metabolism:    return "Metabolism";
        case ChromosomeType::Locomotion:    return "Locomotion";
        case ChromosomeType::Behavior:      return "Behavior";
        case ChromosomeType::Reproduction:  return "Reproduction";
        case ChromosomeType::Environmental: return "Environmental";
        case ChromosomeType::Lifespan:      return "Lifespan";
        default:
            throw std::runtime_error("Chromosome::typeToString: unknown chromosome type");
    }
}

ChromosomeType Chromosome::stringToType(const std::string& str) {
    if (str == "Morphology")    return ChromosomeType::Morphology;
    if (str == "Sensory")       return ChromosomeType::Sensory;
    if (str == "Metabolism")    return ChromosomeType::Metabolism;
    if (str == "Locomotion")    return ChromosomeType::Locomotion;
    if (str == "Behavior")      return ChromosomeType::Behavior;
    if (str == "Reproduction")  return ChromosomeType::Reproduction;
    if (str == "Environmental") return ChromosomeType::Environmental;
    if (str == "Lifespan")      return ChromosomeType::Lifespan;
    
    throw std::runtime_error("Chromosome::stringToType: unknown chromosome type '" + str + "'");
}

} // namespace Genetics
} // namespace EcoSim
