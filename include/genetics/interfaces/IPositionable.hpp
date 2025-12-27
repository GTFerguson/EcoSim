#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * @brief Interface for entities with position in the world.
 *
 * Provides both tile-based (integer) and world (float) coordinate access.
 * The float position system allows fractional movement each tick,
 * enabling visible speed differences between creatures.
 */
class IPositionable {
public:
    virtual ~IPositionable() = default;
    
    /**
     * @brief Get tile X coordinate (for collision detection, NCurses rendering).
     * @return Integer tile X, equivalent to floor(getWorldX())
     */
    virtual int getX() const = 0;
    
    /**
     * @brief Get tile Y coordinate (for collision detection, NCurses rendering).
     * @return Integer tile Y, equivalent to floor(getWorldY())
     */
    virtual int getY() const = 0;
    
    /**
     * @brief Get world X coordinate (float position).
     * @return Actual X position in world space
     */
    virtual float getWorldX() const = 0;
    
    /**
     * @brief Get world Y coordinate (float position).
     * @return Actual Y position in world space
     */
    virtual float getWorldY() const = 0;
    
    /**
     * @brief Set world position using float coordinates.
     * @param x World X coordinate
     * @param y World Y coordinate
     */
    virtual void setWorldPosition(float x, float y) = 0;
};

} // namespace Genetics
} // namespace EcoSim
