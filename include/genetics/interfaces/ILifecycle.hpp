#pragma once

namespace EcoSim {
namespace Genetics {

// Interface for entities with lifecycle
class ILifecycle {
public:
    virtual ~ILifecycle() = default;
    
    virtual unsigned int getAge() const = 0;
    virtual unsigned int getMaxLifespan() const = 0;
    virtual float getAgeNormalized() const = 0;  // 0.0 to 1.0
    virtual bool isAlive() const = 0;
    virtual void age(unsigned int ticks = 1) = 0;
};

} // namespace Genetics
} // namespace EcoSim
