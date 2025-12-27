#pragma once

#include <vector>

namespace EcoSim {

// Forward declarations for external types
class Tile;
class Creature;
class ScentLayer;

namespace Genetics {

// Forward declaration for Plant (in Genetics namespace)
class Plant;

/**
 * @brief Read-only interface for world queries
 * 
 * Provides organisms access to world state without mutation capability.
 * This interface enables behaviors to query the environment while
 * maintaining clear separation of concerns.
 * 
 * @note Implementations should handle out-of-bounds coordinates gracefully.
 */
class IWorldQuery {
public:
    virtual ~IWorldQuery() = default;
    
    /**
     * @brief Get the tile at the specified coordinates
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return Reference to the tile at (x, y)
     */
    virtual const Tile& getTile(int x, int y) const = 0;
    
    /**
     * @brief Get temperature at the specified coordinates
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return Temperature value at (x, y)
     */
    virtual float getTemperature(int x, int y) const = 0;
    
    /**
     * @brief Check if the tile at coordinates contains water
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return true if the tile is a water tile
     */
    virtual bool isWater(int x, int y) const = 0;
    
    /**
     * @brief Find all creatures within a radius of a point
     * @param x X coordinate of center
     * @param y Y coordinate of center
     * @param radius Search radius in tiles
     * @return Vector of pointers to creatures in range
     */
    virtual std::vector<const Creature*> getCreaturesInRadius(
        int x, int y, float radius) const = 0;
    
    /**
     * @brief Find all plants within a radius of a point
     * @param x X coordinate of center
     * @param y Y coordinate of center
     * @param radius Search radius in tiles
     * @return Vector of pointers to plants in range
     */
    virtual std::vector<const Plant*> getPlantsInRadius(
        int x, int y, float radius) const = 0;
    
    /**
     * @brief Get the scent layer for olfactory queries
     * @return Reference to the scent layer
     */
    virtual const ScentLayer& getScentLayer() const = 0;
    
    /**
     * @brief Get the current simulation tick
     * @return Current tick count
     */
    virtual unsigned int getCurrentTick() const = 0;
    
    /**
     * @brief Get the number of rows (height) in the world
     * @return World height in tiles
     */
    virtual int getRows() const = 0;
    
    /**
     * @brief Get the number of columns (width) in the world
     * @return World width in tiles
     */
    virtual int getCols() const = 0;
};

} // namespace Genetics
} // namespace EcoSim
