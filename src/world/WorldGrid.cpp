/**
 * @file WorldGrid.cpp
 * @brief Implementation of WorldGrid - 2D tile storage wrapper
 */

#include "../../include/world/WorldGrid.hpp"
#include <sstream>

namespace EcoSim {

WorldGrid::WorldGrid(unsigned int width, unsigned int height)
    : _width(width), _height(height) {
    resize(width, height);
}

WorldGrid::WorldGrid(unsigned int width, unsigned int height, const Tile& defaultTile)
    : _width(width), _height(height) {
    resize(width, height, defaultTile);
}

Tile& WorldGrid::at(int x, int y) {
    if (!inBounds(x, y)) {
        std::ostringstream ss;
        ss << "WorldGrid::at() - coordinates (" << x << ", " << y 
           << ") out of bounds (grid size: " << _width << "x" << _height << ")";
        throw std::out_of_range(ss.str());
    }
    return _tiles[static_cast<unsigned int>(x)][static_cast<unsigned int>(y)];
}

const Tile& WorldGrid::at(int x, int y) const {
    if (!inBounds(x, y)) {
        std::ostringstream ss;
        ss << "WorldGrid::at() - coordinates (" << x << ", " << y 
           << ") out of bounds (grid size: " << _width << "x" << _height << ")";
        throw std::out_of_range(ss.str());
    }
    return _tiles[static_cast<unsigned int>(x)][static_cast<unsigned int>(y)];
}

void WorldGrid::resize(unsigned int width, unsigned int height) {
    _width = width;
    _height = height;
    
    _tiles.resize(width);
    for (unsigned int x = 0; x < width; ++x) {
        _tiles[x].resize(height);
    }
}

void WorldGrid::resize(unsigned int width, unsigned int height, const Tile& defaultTile) {
    _width = width;
    _height = height;
    
    _tiles.resize(width);
    for (unsigned int x = 0; x < width; ++x) {
        _tiles[x].resize(height, defaultTile);
    }
}

} // namespace EcoSim
