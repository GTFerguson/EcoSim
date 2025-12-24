#pragma once

namespace EcoSim {
namespace Genetics {

// Interface for entities that can reproduce
template<typename T>
class IReproducible {
public:
    virtual ~IReproducible() = default;
    
    virtual bool canReproduce() const = 0;
    virtual float getReproductiveUrge() const = 0;
    virtual T reproduce(const T& partner) = 0;
};

} // namespace Genetics
} // namespace EcoSim
