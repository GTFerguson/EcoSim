#pragma once

#include <memory>

namespace EcoSim {
namespace Genetics {

class Organism;  // Forward declaration

enum class ReproductionMode {
    SEXUAL,    // Two parents, genome recombination
    ASEXUAL    // Single parent, cloning with mutation
};

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
    virtual bool isCompatibleWith(const Organism& other) const = 0;
    
    // Reproduction - returns base Organism type
    virtual std::unique_ptr<Organism> reproduce(
        const Organism* partner = nullptr  // nullptr for asexual
    ) = 0;
};

} // namespace Genetics
} // namespace EcoSim
