#pragma once

namespace EcoSim {
namespace Genetics {

// Forward declarations
class Genome;
class Phenotype;

/**
 * @brief Pure genetics interface - provides access to genome and phenotype.
 *
 * Separated from position/lifecycle concerns for interface segregation.
 * This interface contains only genetic-related methods, unlike Organism
 * which mixes genetics with position and identity concerns.
 */
class IGenetic {
public:
    virtual ~IGenetic() = default;
    
    /// Get the organism's genome
    virtual const Genome& getGenome() const = 0;
    
    /// Get mutable genome for modification
    virtual Genome& getGenomeMutable() = 0;
    
    /// Get the organism's phenotype
    virtual const Phenotype& getPhenotype() const = 0;
    
    /// Recalculate expressed traits from genome
    virtual void updatePhenotype() = 0;
};

} // namespace Genetics
} // namespace EcoSim
