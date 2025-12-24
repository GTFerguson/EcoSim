#include "genetics/core/GeneRegistry.hpp"
#include <stdexcept>
#include <optional>
#include <iostream>

namespace EcoSim {
namespace Genetics {

void GeneRegistry::registerGene(const GeneDefinition& definition) {
    const std::string& id = definition.getId();
    
    if (definitions_.find(id) != definitions_.end()) {
        throw std::runtime_error("Gene definition already exists: " + id);
    }
    
    definitions_.emplace(id, definition);
}

void GeneRegistry::registerGene(GeneDefinition&& definition) {
    const std::string id = definition.getId(); // Copy ID before move
    
    if (definitions_.find(id) != definitions_.end()) {
        throw std::runtime_error("Gene definition already exists: " + id);
    }
    
    definitions_.emplace(id, std::move(definition));
}

bool GeneRegistry::tryRegisterGene(const GeneDefinition& definition) {
    const std::string& id = definition.getId();
    
    if (definitions_.find(id) != definitions_.end()) {
        // Gene already exists - log warning and return false instead of throwing
        std::cerr << "[GeneRegistry] Warning: Skipping duplicate gene registration: "
                  << id << std::endl;
        return false;
    }
    
    definitions_.emplace(id, definition);
    return true;
}

bool GeneRegistry::tryRegisterGene(GeneDefinition&& definition) {
    const std::string id = definition.getId(); // Copy ID before move
    
    if (definitions_.find(id) != definitions_.end()) {
        // Gene already exists - log warning and return false instead of throwing
        std::cerr << "[GeneRegistry] Warning: Skipping duplicate gene registration: "
                  << id << std::endl;
        return false;
    }
    
    definitions_.emplace(id, std::move(definition));
    return true;
}

bool GeneRegistry::hasGene(const std::string& gene_id) const {
    return definitions_.find(gene_id) != definitions_.end();
}

const GeneDefinition& GeneRegistry::getDefinition(const std::string& gene_id) const {
    auto it = definitions_.find(gene_id);
    if (it == definitions_.end()) {
        throw std::out_of_range("Gene definition not found: " + gene_id);
    }
    return it->second;
}

std::optional<std::reference_wrapper<const GeneDefinition>> 
GeneRegistry::tryGetDefinition(const std::string& gene_id) const {
    auto it = definitions_.find(gene_id);
    if (it == definitions_.end()) {
        return std::nullopt;
    }
    return std::cref(it->second);
}

const std::unordered_map<std::string, GeneDefinition>& 
GeneRegistry::getAllDefinitions() const {
    return definitions_;
}

std::vector<std::reference_wrapper<const GeneDefinition>> 
GeneRegistry::getDefinitionsForChromosome(ChromosomeType type) const {
    std::vector<std::reference_wrapper<const GeneDefinition>> result;
    
    for (const auto& pair : definitions_) {
        if (pair.second.getChromosome() == type) {
            result.push_back(std::cref(pair.second));
        }
    }
    
    return result;
}

std::vector<std::string> GeneRegistry::getAllGeneIds() const {
    std::vector<std::string> ids;
    ids.reserve(definitions_.size());
    
    for (const auto& pair : definitions_) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void GeneRegistry::clear() {
    definitions_.clear();
}

size_t GeneRegistry::size() const {
    return definitions_.size();
}

bool GeneRegistry::empty() const {
    return definitions_.empty();
}

} // namespace Genetics
} // namespace EcoSim
