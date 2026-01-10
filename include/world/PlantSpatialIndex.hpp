#ifndef ECOSIM_WORLD_PLANT_SPATIAL_INDEX_HPP
#define ECOSIM_WORLD_PLANT_SPATIAL_INDEX_HPP

#include <vector>
#include <unordered_map>
#include <memory>

namespace EcoSim {
namespace Genetics {
class Plant;
}

/**
 * @brief Grid-based spatial index for fast plant neighbor queries.
 * 
 * Uses a uniform grid where each cell contains pointers to plants
 * within that spatial region. Provides O(1) average-case neighbor queries
 * instead of O(r²) tile iteration through all tiles in sight range.
 * 
 * Unlike the creature SpatialIndex, plants don't move, so this index
 * only needs rebuilding when plants are added or removed.
 */
class PlantSpatialIndex {
public:
    static constexpr int DEFAULT_CELL_SIZE = 32;
    
    /**
     * @brief Construct spatial index for given world dimensions.
     * @param worldWidth Width of world in tiles
     * @param worldHeight Height of world in tiles
     * @param cellSize Size of each cell in tiles (default 32)
     */
    PlantSpatialIndex(int worldWidth, int worldHeight, int cellSize = DEFAULT_CELL_SIZE);
    
    //==========================================================================
    // Core Operations
    //==========================================================================
    
    /**
     * @brief Add a plant to the index.
     * @param plant Pointer to plant (non-owning)
     * @param x X position of plant
     * @param y Y position of plant
     */
    void insert(Genetics::Plant* plant, int x, int y);
    
    /**
     * @brief Remove a plant from the index.
     * @param plant Pointer to plant to remove
     * @param x X position of plant
     * @param y Y position of plant
     */
    void remove(Genetics::Plant* plant, int x, int y);
    
    /**
     * @brief Clear all plants from the index.
     */
    void clear();
    
    //==========================================================================
    // Query Operations
    //==========================================================================
    
    /**
     * @brief Find all plants within radius of a position.
     * @param x Center X position
     * @param y Center Y position
     * @param radius Search radius in tiles
     * @return Vector of plant pointers within radius
     */
    std::vector<Genetics::Plant*> queryRadius(float x, float y, float radius) const;
    
    /**
     * @brief Find all plants in a specific grid cell.
     * @param cellX Cell X coordinate (not tile coordinate)
     * @param cellY Cell Y coordinate (not tile coordinate)
     * @return Vector of plant pointers in cell
     */
    std::vector<Genetics::Plant*> queryCell(int cellX, int cellY) const;
    
    //==========================================================================
    // Utility
    //==========================================================================
    
    /**
     * @brief Get cell coordinates for a world position.
     * @param x World X position
     * @param y World Y position
     * @return Pair of (cellX, cellY)
     */
    std::pair<int, int> getCellCoords(float x, float y) const;
    
    /**
     * @brief Get total number of indexed plants.
     */
    size_t size() const;
    
    /**
     * @brief Check if index is empty.
     */
    bool empty() const;
    
    /**
     * @brief Get cell size.
     */
    int getCellSize() const { return cellSize_; }
    
private:
    struct CellKey {
        int x, y;
        bool operator==(const CellKey& other) const {
            return x == other.x && y == other.y;
        }
    };
    
    struct CellKeyHash {
        size_t operator()(const CellKey& k) const {
            // Cantor pairing function for hash
            return std::hash<int>()((k.x + k.y) * (k.x + k.y + 1) / 2 + k.y);
        }
    };
    
    int worldWidth_;
    int worldHeight_;
    int cellSize_;
    int cellsX_;  // Number of cells in X dimension
    int cellsY_;  // Number of cells in Y dimension
    size_t plantCount_;  // Total number of indexed plants
    
    std::unordered_map<CellKey, std::vector<Genetics::Plant*>, CellKeyHash> grid_;
    
    // Helper to clamp cell coordinates to valid range
    CellKey clampCell(int x, int y) const;
};

} // namespace EcoSim

#endif // ECOSIM_WORLD_PLANT_SPATIAL_INDEX_HPP
