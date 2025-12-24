#pragma once

namespace EcoSim {
namespace Genetics {

// Interface for entities with position
class IPositionable {
public:
    virtual ~IPositionable() = default;
    
    virtual int getX() const = 0;
    virtual int getY() const = 0;
    virtual void setPosition(int x, int y) = 0;
};

} // namespace Genetics
} // namespace EcoSim
