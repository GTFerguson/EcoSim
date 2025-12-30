#ifndef ECOSIM_WORLD_WORLDGRID_HPP
#define ECOSIM_WORLD_WORLDGRID_HPP

/**
 * @file WorldGrid.hpp
 * @brief Thin wrapper around 2D tile storage with bounds checking
 * 
 * WorldGrid provides a clean interface for tile storage and access,
 * separating storage concerns from world generation and simulation logic.
 */

#include "tile.hpp"
#include <vector>
#include <stdexcept>

namespace EcoSim {

/**
 * @class WorldGrid
 * @brief 2D grid of tiles with bounds-checked access
 * 
 * Provides storage for world tiles with:
 * - Bounds-checked access via at()
 * - Fast unchecked access via operator()
 * - Dimension queries
 * - Iteration support for range-based for loops
 */
class WorldGrid {
public:
    //==========================================================================
    // Construction
    //==========================================================================
    
    /**
     * @brief Construct an empty grid (must call resize before use)
     */
    WorldGrid() = default;
    
    /**
     * @brief Construct a grid with specified dimensions
     * @param width Number of columns (x dimension)
     * @param height Number of rows (y dimension)
     */
    WorldGrid(unsigned int width, unsigned int height);
    
    /**
     * @brief Construct a grid with dimensions and default tile
     * @param width Number of columns (x dimension)
     * @param height Number of rows (y dimension)
     * @param defaultTile Tile to initialize all cells with
     */
    WorldGrid(unsigned int width, unsigned int height, const Tile& defaultTile);
    
    //==========================================================================
    // Dimension Queries
    //==========================================================================
    
    /**
     * @brief Get the width (number of columns) of the grid
     * @return Width in tiles
     */
    unsigned int width() const { return _width; }
    
    /**
     * @brief Get the height (number of rows) of the grid
     * @return Height in tiles
     */
    unsigned int height() const { return _height; }
    
    /**
     * @brief Check if coordinates are within grid bounds
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return true if (x, y) is within bounds
     */
    bool inBounds(int x, int y) const {
        return x >= 0 && x < static_cast<int>(_width) &&
               y >= 0 && y < static_cast<int>(_height);
    }
    
    /**
     * @brief Check if the grid has been initialized with dimensions
     * @return true if grid has non-zero dimensions
     */
    bool isInitialized() const { return _width > 0 && _height > 0; }
    
    //==========================================================================
    // Tile Access
    //==========================================================================
    
    /**
     * @brief Access a tile with bounds checking
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return Reference to the tile at (x, y)
     * @throws std::out_of_range if coordinates are out of bounds
     */
    Tile& at(int x, int y);
    
    /**
     * @brief Access a tile with bounds checking (const version)
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return Const reference to the tile at (x, y)
     * @throws std::out_of_range if coordinates are out of bounds
     */
    const Tile& at(int x, int y) const;
    
    /**
     * @brief Fast unchecked tile access
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return Reference to the tile at (x, y)
     * @note No bounds checking - undefined behavior if out of bounds
     */
    Tile& operator()(unsigned int x, unsigned int y) {
        return _tiles[x][y];
    }
    
    /**
     * @brief Fast unchecked tile access (const version)
     * @param x X coordinate (column)
     * @param y Y coordinate (row)
     * @return Const reference to the tile at (x, y)
     * @note No bounds checking - undefined behavior if out of bounds
     */
    const Tile& operator()(unsigned int x, unsigned int y) const {
        return _tiles[x][y];
    }
    
    //==========================================================================
    // Grid Management
    //==========================================================================
    
    /**
     * @brief Resize the grid to new dimensions
     * @param width New width (columns)
     * @param height New height (rows)
     * @note Existing tiles are discarded
     */
    void resize(unsigned int width, unsigned int height);
    
    /**
     * @brief Resize the grid with a default tile
     * @param width New width (columns)
     * @param height New height (rows)
     * @param defaultTile Tile to initialize all cells with
     */
    void resize(unsigned int width, unsigned int height, const Tile& defaultTile);
    
    //==========================================================================
    // Raw Access (for backward compatibility and performance-critical code)
    //==========================================================================
    
    /**
     * @brief Get direct access to the underlying tile storage
     * @return Reference to the 2D vector of tiles
     * @note Use with caution - bypasses bounds checking
     * @deprecated Prefer at() or operator() for new code
     */
    std::vector<std::vector<Tile>>& raw() { return _tiles; }
    
    /**
     * @brief Get direct access to the underlying tile storage (const)
     * @return Const reference to the 2D vector of tiles
     * @deprecated Prefer at() or operator() for new code
     */
    const std::vector<std::vector<Tile>>& raw() const { return _tiles; }
    
    //==========================================================================
    // Iteration Support
    //==========================================================================
    
    /**
     * @brief Iterator for traversing all tiles in the grid
     * 
     * Iterates in row-major order: (0,0), (1,0), ..., (width-1,0), (0,1), ...
     */
    class Iterator {
    public:
        struct TileRef {
            Tile& tile;
            unsigned int x;
            unsigned int y;
        };
        
        Iterator(WorldGrid* grid, unsigned int x, unsigned int y)
            : _grid(grid), _x(x), _y(y) {}
        
        TileRef operator*() {
            return TileRef{(*_grid)(_x, _y), _x, _y};
        }
        
        Iterator& operator++() {
            ++_x;
            if (_x >= _grid->width()) {
                _x = 0;
                ++_y;
            }
            return *this;
        }
        
        bool operator!=(const Iterator& other) const {
            return _x != other._x || _y != other._y;
        }
        
    private:
        WorldGrid* _grid;
        unsigned int _x;
        unsigned int _y;
    };
    
    /**
     * @brief Const iterator for traversing all tiles in the grid
     */
    class ConstIterator {
    public:
        struct ConstTileRef {
            const Tile& tile;
            unsigned int x;
            unsigned int y;
        };
        
        ConstIterator(const WorldGrid* grid, unsigned int x, unsigned int y)
            : _grid(grid), _x(x), _y(y) {}
        
        ConstTileRef operator*() const {
            return ConstTileRef{(*_grid)(_x, _y), _x, _y};
        }
        
        ConstIterator& operator++() {
            ++_x;
            if (_x >= _grid->width()) {
                _x = 0;
                ++_y;
            }
            return *this;
        }
        
        bool operator!=(const ConstIterator& other) const {
            return _x != other._x || _y != other._y;
        }
        
    private:
        const WorldGrid* _grid;
        unsigned int _x;
        unsigned int _y;
    };
    
    Iterator begin() { return Iterator(this, 0, 0); }
    Iterator end() { return Iterator(this, 0, _height); }
    ConstIterator begin() const { return ConstIterator(this, 0, 0); }
    ConstIterator end() const { return ConstIterator(this, 0, _height); }
    
private:
    std::vector<std::vector<Tile>> _tiles;
    unsigned int _width = 0;
    unsigned int _height = 0;
};

} // namespace EcoSim

#endif // ECOSIM_WORLD_WORLDGRID_HPP
