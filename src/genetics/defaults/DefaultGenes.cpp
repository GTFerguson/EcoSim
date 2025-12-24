/**
 * @file DefaultGenes.cpp
 * @brief Implementation of default gene definitions for backward compatibility
 *
 * DEPRECATED: This file forwards all calls to UniversalGenes.
 *
 * New code should use UniversalGenes directly:
 * - UniversalGenes::registerDefaults(registry)
 * - UniversalGenes::createRandomGenome(registry)
 * - UniversalGenes::createCreatureGenome(registry)
 *
 * See include/genetics/defaults/DefaultGenes.hpp for full migration guide.
 */

#include "genetics/defaults/DefaultGenes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneticTypes.hpp"

namespace EcoSim {
namespace Genetics {

// DRY refactoring: Forward to UniversalGenes instead of duplicating registration logic
// This reduces 106 lines of code to a simple delegation
void DefaultGenes::registerDefaults(GeneRegistry& registry) {
    // DEPRECATED: Forward to UniversalGenes which includes all DefaultGenes
    // functionality plus 55+ additional genes for coevolution support
    UniversalGenes::registerDefaults(registry);
}

// DRY refactoring: Forward to UniversalGenes
Genome DefaultGenes::createRandomGenome(const GeneRegistry& registry) {
    // DEPRECATED: Forward to UniversalGenes
    return UniversalGenes::createRandomGenome(registry);
}

// DRY refactoring: Forward to UniversalGenes::createCreatureGenome
// which provides the same functionality with additional genes
Genome DefaultGenes::createDefaultGenome(const GeneRegistry& registry) {
    // DEPRECATED: Forward to UniversalGenes::createCreatureGenome
    // which creates a creature-optimized genome with proper default values
    return UniversalGenes::createCreatureGenome(registry);
}

} // namespace Genetics
} // namespace EcoSim
