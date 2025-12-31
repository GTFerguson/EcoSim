#pragma once

#include "genetics/core/GeneticTypes.hpp"
#include <string>

namespace EcoSim {
namespace Genetics {

// Forward declaration
class Phenotype;

/**
 * @brief Utility functions for phenotype effect calculations
 *
 * Extracted to reduce code duplication in Phenotype.cpp.
 * These utilities handle the common effect accumulation logic
 * used by both computeTrait() and computeTraitRaw().
 */
namespace PhenotypeUtils {

/**
 * @brief Structure to track accumulated effect values
 * 
 * Used during trait computation to accumulate contributions
 * from multiple genes with different effect types.
 */
struct AccumulatedEffect {
    float value = 0.0f;          ///< Current accumulated value
    bool found_contribution = false;  ///< Whether any gene contributed
};

/**
 * @brief Apply a single effect to the accumulated value
 * 
 * Handles all effect types (Direct, Additive, Multiplicative, Threshold, Conditional)
 * and updates the accumulated state accordingly.
 * 
 * @param accumulated Current accumulated state
 * @param effect_type Type of effect being applied
 * @param gene_value The expressed gene value contributing to this effect
 * @param scale_factor The effect's scale factor
 * @return Updated accumulated effect state
 * 
 * Effect type behaviors:
 * - Direct: gene value becomes trait value (overwrites)
 * - Additive: adds to accumulated sum
 * - Multiplicative: multiplies existing value (starts at 1.0 if first)
 * - Threshold: only contributes if gene_value >= scale_factor
 * - Conditional: context-dependent (simplified as additive)
 */
AccumulatedEffect applyEffect(const AccumulatedEffect& accumulated,
                               EffectType effect_type,
                               float gene_value,
                               float scale_factor);

/**
 * @brief Safely get a trait value from a phenotype with a fallback default
 *
 * @param phenotype The phenotype to query
 * @param traitName The name of the trait to retrieve
 * @param defaultValue Value to return if trait doesn't exist
 * @return The trait value if present, otherwise defaultValue
 */
float getTraitSafe(const Phenotype& phenotype,
                   const std::string& traitName,
                   float defaultValue);

} // namespace PhenotypeUtils

} // namespace Genetics
} // namespace EcoSim
