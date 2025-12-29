#include "genetics/core/Gene.hpp"
#include "genetics/core/RandomEngine.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <variant>
#include <stdexcept>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Gene Implementation
// ============================================================================

Gene::Gene(const std::string& gene_id, const Allele& allele1, const Allele& allele2)
    : id_(gene_id), allele1_(allele1), allele2_(allele2) {
}

Gene::Gene(const std::string& gene_id, const GeneValue& value)
    : id_(gene_id), allele1_(Allele(value)), allele2_(Allele(value)) {
}

bool Gene::isHeterozygous() const {
    return allele1_.value != allele2_.value;
}

GeneValue Gene::getExpressedValue(DominanceType dominance) const {
    switch (dominance) {
        case DominanceType::Complete: {
            if (allele1_.expression_strength >= allele2_.expression_strength) {
                return allele1_.value;
            }
            return allele2_.value;
        }
        
        case DominanceType::Incomplete: {
            float val1 = toNumeric(allele1_.value);
            float val2 = toNumeric(allele2_.value);
            return GeneValue((val1 + val2) / 2.0f);
        }
        
        case DominanceType::Codominant: {
            float val1 = toNumeric(allele1_.value);
            float val2 = toNumeric(allele2_.value);
            return GeneValue((val1 + val2) / 2.0f);
        }
        
        case DominanceType::Overdominant: {
            float val1 = toNumeric(allele1_.value);
            float val2 = toNumeric(allele2_.value);
            float avg = (val1 + val2) / 2.0f;
            
            if (isHeterozygous()) {
                static constexpr float HETEROZYGOTE_ADVANTAGE = 1.1f;
                avg *= HETEROZYGOTE_ADVANTAGE;
            }
            return GeneValue(avg);
        }
    }
    
    return allele1_.value;
}

float Gene::getNumericValue(DominanceType dominance) const {
    GeneValue expressed = getExpressedValue(dominance);
    return toNumeric(expressed);
}

void Gene::mutate(float mutation_rate, const GeneLimits& limits) {
    auto& rng = getThreadLocalRNG();
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    std::uniform_real_distribution<float> creep(-limits.creep_amount, limits.creep_amount);
    
    if (chance(rng) < mutation_rate) {
        float val = toNumeric(allele1_.value);
        val += creep(rng);
        val = std::clamp(val, limits.min_value, limits.max_value);
        allele1_.value = GeneValue(val);
    }
    
    if (chance(rng) < mutation_rate) {
        float val = toNumeric(allele2_.value);
        val += creep(rng);
        val = std::clamp(val, limits.min_value, limits.max_value);
        allele2_.value = GeneValue(val);
    }
}

Gene Gene::crossover(const Gene& parent1, const Gene& parent2) {
    if (parent1.id_ != parent2.id_) {
        throw std::invalid_argument("Cannot crossover genes with different IDs");
    }
    
    auto& rng = getThreadLocalRNG();
    std::uniform_int_distribution<int> coin(0, 1);
    
    const Allele& from_parent1 = (coin(rng) == 0) ? parent1.allele1_ : parent1.allele2_;
    const Allele& from_parent2 = (coin(rng) == 0) ? parent2.allele1_ : parent2.allele2_;
    
    return Gene(parent1.id_, from_parent1, from_parent2);
}

void Gene::setAlleleValues(float value) {
    allele1_.value = GeneValue(value);
    allele2_.value = GeneValue(value);
}

void Gene::setAlleleValues(float value1, float value2) {
    allele1_.value = GeneValue(value1);
    allele2_.value = GeneValue(value2);
}

// ============================================================================
// Gene Serialization
// ============================================================================

nlohmann::json Gene::toJson() const {
    return {
        {"id", id_},
        {"allele1", {{"value", toNumeric(allele1_.value)}}},
        {"allele2", {{"value", toNumeric(allele2_.value)}}}
    };
}

Gene Gene::fromJson(const nlohmann::json& j) {
    if (!j.contains("id")) {
        throw std::runtime_error("Gene::fromJson: missing required field 'id'");
    }
    if (!j.contains("allele1") || !j["allele1"].contains("value")) {
        throw std::runtime_error("Gene::fromJson: missing required field 'allele1.value'");
    }
    if (!j.contains("allele2") || !j["allele2"].contains("value")) {
        throw std::runtime_error("Gene::fromJson: missing required field 'allele2.value'");
    }
    
    std::string id = j.at("id").get<std::string>();
    float val1 = j.at("allele1").at("value").get<float>();
    float val2 = j.at("allele2").at("value").get<float>();
    
    Allele a1{GeneValue{val1}};
    Allele a2{GeneValue{val2}};
    
    return Gene(id, a1, a2);
}

float Gene::toNumeric(const GeneValue& value) {
    return std::visit([](auto&& arg) -> float {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, float>) {
            return arg;
        } else if constexpr (std::is_same_v<T, int>) {
            return static_cast<float>(arg);
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? 1.0f : 0.0f;
        } else if constexpr (std::is_same_v<T, std::string>) {
            try {
                return std::stof(arg);
            } catch (...) {
                return 0.0f;
            }
        }
        return 0.0f;
    }, value);
}

} // namespace Genetics
} // namespace EcoSim
