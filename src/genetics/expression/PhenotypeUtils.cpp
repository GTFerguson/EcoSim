#include "genetics/expression/PhenotypeUtils.hpp"

namespace EcoSim {
namespace Genetics {
namespace PhenotypeUtils {

AccumulatedEffect applyEffect(const AccumulatedEffect& accumulated,
                               EffectType effect_type,
                               float gene_value,
                               float scale_factor) {
    AccumulatedEffect result = accumulated;
    
    switch (effect_type) {
        case EffectType::Direct:
            // Direct: gene value becomes trait value
            result.value = gene_value * scale_factor;
            result.found_contribution = true;
            break;
            
        case EffectType::Additive:
            // Additive: contribute to sum
            result.value += gene_value * scale_factor;
            result.found_contribution = true;
            break;
            
        case EffectType::Multiplicative:
            // Multiplicative: multiply existing value
            if (!result.found_contribution) {
                result.value = 1.0f;
            }
            result.value *= (gene_value * scale_factor);
            result.found_contribution = true;
            break;
            
        case EffectType::Threshold:
            // Threshold: only contributes if above/below threshold
            // scale_factor is used as threshold
            if (gene_value >= scale_factor) {
                result.value += gene_value;
                result.found_contribution = true;
            }
            break;
            
        case EffectType::Conditional:
            // Conditional: context-dependent (simplified implementation)
            // Could be extended to check specific conditions
            result.value += gene_value * scale_factor;
            result.found_contribution = true;
            break;
    }
    
    return result;
}

} // namespace PhenotypeUtils
} // namespace Genetics
} // namespace EcoSim
