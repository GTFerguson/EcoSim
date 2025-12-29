#ifndef ECOSIM_WORLD_SPATIAL_INDEX_HPP
#define ECOSIM_WORLD_SPATIAL_INDEX_HPP

#include <vector>
#include <unordered_map>
#include <functional>

class Creature;

namespace EcoSim {

/**
 * @brief Grid-based spatial index for fast neighbor queries.
 * 
 * Uses a uniform grid where each cell contains pointers to creatures
 * within that spatial region. Provides O(1) average-case neighbor queries
 * instead of O(n) linear scans through all creatures.
 */
class SpatialIndex {
public:
    static constexpr int DEFAULT_CELL_SIZE = 32;
    
    /**
     * @brief Construct spatial index for given world dimensions.
     * @param worldWidth Width of world in tiles
     * @param worldHeight Height of world in tiles
     * @param cellSize Size of each cell in tiles (default 32)
     */
    SpatialIndex(int worldWidth, int worldHeight, int cellSize = DEFAULT_CELL_SIZE);
    
    //==========================================================================
    // Core Operations
    //==========================================================================
    
    /**
     * @brief Add a creature to the index.
     * @param creature Pointer to creature (non-owning)
     */
    void insert(Creature* creature);
    
    /**
     * @brief Remove a creature from the index.
     * @param creature Pointer to creature to remove
     */
    void remove(Creature* creature);
    
    /**
     * @brief Update creature's position in the index.
     * 
     * More efficient than remove+insert when creature stays in same cell.
     * 
     * @param creature Pointer to creature
     * @param oldX Previous X position
     * @param oldY Previous Y position
     */
    void update(Creature* creature, float oldX, float oldY);
    
    /**
     * @brief Clear all creatures from the index.
     */
    void clear();
    
    /**
     * @brief Rebuild index from creature vector.
     * 
     * Use after loading saves or major changes.
     * 
     * @param creatures Vector of all creatures
     */
    void rebuild(std::vector<Creature>& creatures);
    
    //==========================================================================
    // Query Operations
    //==========================================================================
    
    /**
     * @brief Find all creatures within radius of a position.
     * @param x Center X position
     * @param y Center Y position
     * @param radius Search radius in tiles
     * @return Vector of creature pointers within radius
     */
    std::vector<Creature*> queryRadius(float x, float y, float radius) const;
    
    /**
     * @brief Find all creatures in a specific grid cell.
     * @param cellX Cell X coordinate (not tile coordinate)
     * @param cellY Cell Y coordinate (not tile coordinate)
     * @return Vector of creature pointers in cell
     */
    std::vector<Creature*> queryCell(int cellX, int cellY) const;
    
    /**
     * @brief Find all creatures in cell containing position and adjacent cells.
     * @param x Position X in tiles
     * @param y Position Y in tiles
     * @return Vector of creature pointers in nearby cells
     */
    std::vector<Creature*> queryNearbyCells(float x, float y) const;
    
    /**
     * @brief Query with custom filter predicate.
     * @param x Center X position
     * @param y Center Y position
     * @param radius Search radius
     * @param predicate Filter function returning true for matches
     * @return Filtered vector of creature pointers
     */
    std::vector<Creature*> queryWithFilter(
        float x, float y, float radius,
        std::function<bool(const Creature*)> predicate) const;
    
    /**
     * @brief Find single nearest creature matching predicate.
     * @param x Center X position
     * @param y Center Y position
     * @param maxRadius Maximum search radius
     * @param predicate Filter function (return true to consider)
     * @return Pointer to nearest matching creature, or nullptr
     */
    Creature* findNearest(
        float x, float y, float maxRadius,
        std::function<bool(const Creature*)> predicate) const;
    
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
     * @brief Get total number of indexed creatures.
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
    size_t creatureCount_;  // Total number of indexed creatures
    
    std::unordered_map<CellKey, std::vector<Creature*>, CellKeyHash> grid_;
    
    // Helper to clamp cell coordinates to valid range
    CellKey clampCell(int x, int y) const;
};

} // namespace EcoSim

#endif // ECOSIM_WORLD_SPATIAL_INDEX_HPP
