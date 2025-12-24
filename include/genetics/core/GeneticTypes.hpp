#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace EcoSim {
namespace Genetics {

// Chromosome types for organization
enum class ChromosomeType {
    Morphology,      // Size, color, appearance
    Sensory,         // Sight, hearing, smell
    Metabolism,      // Diet, hunger rates, energy
    Locomotion,      // Speed, movement patterns
    Behavior,        // Aggression, sociality, flee/pursue
    Reproduction,    // Mating thresholds, offspring
    Environmental,   // Temperature tolerance, habitat
    Lifespan         // Longevity, aging
};

// Gene value types
using GeneValue = std::variant<float, int, bool, std::string>;

// Dominance types for expression
enum class DominanceType {
    Complete,       // One allele fully masks the other
    Incomplete,     // Blended expression
    Codominant,     // Both alleles expressed
    Overdominant    // Heterozygote advantage
};

// Effect types for how genes influence traits
enum class EffectType {
    Direct,         // Gene value directly becomes trait value
    Additive,       // Contributes to sum
    Multiplicative, // Multiplier effect
    Threshold,      // Only active above/below threshold
    Conditional     // Context-dependent
};

// Allele representation
struct Allele {
    GeneValue value;
    float expression_strength = 1.0f;  // 0.0 to 1.0
    
    Allele() = default;
    explicit Allele(GeneValue val, float strength = 1.0f) 
        : value(val), expression_strength(strength) {}
};

// Gene limits (preserving existing pattern from old Genome)
struct GeneLimits {
    float min_value;
    float max_value;
    float creep_amount;  // Small random variation during inheritance
    
    GeneLimits(float min_val, float max_val, float creep) 
        : min_value(min_val), max_value(max_val), creep_amount(creep) {}
};

// Effect binding - how a gene affects a trait
struct EffectBinding {
    std::string target_domain;   // e.g., "locomotion", "metabolism"
    std::string target_trait;    // e.g., "movement_speed", "energy_consumption"
    EffectType effect_type;
    float scale_factor;          // How much this gene contributes
    
    EffectBinding(const std::string& domain, const std::string& trait, 
                  EffectType type, float scale)
        : target_domain(domain), target_trait(trait), 
          effect_type(type), scale_factor(scale) {}
};

// Chromosome identifier
constexpr int NUM_CHROMOSOMES = 8;

// Convert chromosome type to string
const char* chromosomeTypeToString(ChromosomeType type);

// Convert string to chromosome type
std::optional<ChromosomeType> stringToChromosomeType(const std::string& str);

} // namespace Genetics
} // namespace EcoSim
