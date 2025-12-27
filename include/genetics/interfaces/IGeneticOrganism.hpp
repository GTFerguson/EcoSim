#pragma once

namespace EcoSim {
namespace Genetics {

// Forward declarations
class Genome;
class Phenotype;

/**
 * @brief Interface for entities with genetics
 *
 * Abstract interface for any organism with a genome and phenotype.
 * Uses reference returns for compatibility with existing code.
 *
 * Implementers: Creature, Plant (for future polymorphic behavior handling)
 */
class IGeneticOrganism {
public:
    virtual ~IGeneticOrganism() = default;
    
    /// Get the organism's genome
    virtual const Genome& getGenome() const = 0;
    
    /// Get mutable genome for modification
    virtual Genome& getGenomeMutable() = 0;
    
    /// Get the organism's phenotype
    virtual const Phenotype& getPhenotype() const = 0;
    
    /// Recalculate expressed traits from genome
    virtual void updatePhenotype() = 0;
    
    /// Get the organism's world position
    virtual int getX() const = 0;
    virtual int getY() const = 0;
    
    /// Get unique identifier for tracking
    virtual int getId() const = 0;
};

} // namespace Genetics
} // namespace EcoSim
