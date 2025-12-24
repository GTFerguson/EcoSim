#pragma once

namespace EcoSim {
namespace Genetics {

// Forward declarations
class Genome;
class Phenotype;

// Interface for entities with genetics
class IGeneticOrganism {
public:
    virtual ~IGeneticOrganism() = default;
    
    virtual const Genome& getGenome() const = 0;
    virtual Genome& getGenomeMutable() = 0;
    virtual const Phenotype& getPhenotype() const = 0;
    virtual void updatePhenotype() = 0;  // Recalculate expressed traits
};

} // namespace Genetics
} // namespace EcoSim
