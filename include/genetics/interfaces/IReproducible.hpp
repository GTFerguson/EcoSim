#pragma once

#include <memory>

namespace EcoSim {
namespace Genetics {

class IGeneticOrganism;  // Forward declaration

enum class ReproductionMode {
    SEXUAL,    // Two parents, genome recombination
    ASEXUAL    // Single parent, cloning with mutation
};

// @todo Simplify when Creature/Plant classes are unified into single Organism class.
// The reproduce() return type and dynamic_cast requirements will be eliminated
// once all organisms share the same type.
class IReproducible {
public:
    virtual ~IReproducible() = default;
    
    // Common capabilities
    virtual bool canReproduce() const = 0;
    virtual float getReproductiveUrge() const = 0;
    virtual float getReproductionEnergyCost() const = 0;
    
    // Mode determination (gene-driven)
    virtual ReproductionMode getReproductionMode() const = 0;
    
    // Compatibility check for sexual reproduction
    virtual bool isCompatibleWith(const IGeneticOrganism& other) const = 0;
    
    // Reproduction - returns base type (requires dynamic_cast at call site)
    // @todo Return concrete Organism type once unified
    virtual std::unique_ptr<IGeneticOrganism> reproduce(
        const IGeneticOrganism* partner = nullptr  // nullptr for asexual
    ) = 0;
};

} // namespace Genetics
} // namespace EcoSim
