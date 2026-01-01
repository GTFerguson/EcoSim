#pragma once

namespace genetics {
    class Genome;
    class Phenotype;
}

/**
 * Pure genetics interface - provides access to genome and phenotype.
 * Separated from position/lifecycle concerns for interface segregation.
 */
class IGenetic {
public:
    virtual ~IGenetic() = default;
    
    // Genome access
    virtual const genetics::Genome& getGenome() const = 0;
    virtual genetics::Genome& getGenomeMutable() = 0;
    
    // Phenotype access
    virtual const genetics::Phenotype& getPhenotype() const = 0;
    virtual void updatePhenotype() = 0;
};
