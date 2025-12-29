#include "world/SpatialIndex.hpp"
#include "objects/creature/creature.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace EcoSim {

SpatialIndex::SpatialIndex(int worldWidth, int worldHeight, int cellSize)
    : worldWidth_(worldWidth)
    , worldHeight_(worldHeight)
    , cellSize_(cellSize)
    , creatureCount_(0)
{
    // Calculate number of cells needed (round up to cover entire world)
    cellsX_ = (worldWidth + cellSize - 1) / cellSize;
    cellsY_ = (worldHeight + cellSize - 1) / cellSize;
}

//==============================================================================
// Core Operations
//==============================================================================

void SpatialIndex::insert(Creature* creature) {
    if (!creature) return;
    
    auto [cellX, cellY] = getCellCoords(creature->getWorldX(), creature->getWorldY());
    CellKey key{cellX, cellY};
    grid_[key].push_back(creature);
    ++creatureCount_;
}

void SpatialIndex::remove(Creature* creature) {
    if (!creature) return;
    
    auto [cellX, cellY] = getCellCoords(creature->getWorldX(), creature->getWorldY());
    CellKey key{cellX, cellY};
    
    auto it = grid_.find(key);
    if (it != grid_.end()) {
        auto& vec = it->second;
        auto pos = std::find(vec.begin(), vec.end(), creature);
        if (pos != vec.end()) {
            vec.erase(pos);
            --creatureCount_;
            
            // Clean up empty cells to prevent unbounded memory growth
            if (vec.empty()) {
                grid_.erase(it);
            }
        }
    }
}

void SpatialIndex::update(Creature* creature, float oldX, float oldY) {
    if (!creature) return;
    
    auto [oldCellX, oldCellY] = getCellCoords(oldX, oldY);
    auto [newCellX, newCellY] = getCellCoords(creature->getWorldX(), creature->getWorldY());
    
    // Skip update if creature stayed in same cell
    if (oldCellX == newCellX && oldCellY == newCellY) {
        return;
    }
    
    // Remove from old cell
    CellKey oldKey{oldCellX, oldCellY};
    auto oldIt = grid_.find(oldKey);
    if (oldIt != grid_.end()) {
        auto& vec = oldIt->second;
        auto pos = std::find(vec.begin(), vec.end(), creature);
        if (pos != vec.end()) {
            vec.erase(pos);
            if (vec.empty()) {
                grid_.erase(oldIt);
            }
        }
    }
    
    // Insert into new cell
    CellKey newKey{newCellX, newCellY};
    grid_[newKey].push_back(creature);
}

void SpatialIndex::clear() {
    grid_.clear();
    creatureCount_ = 0;
}

void SpatialIndex::rebuild(std::vector<Creature>& creatures) {
    clear();
    for (Creature& c : creatures) {
        insert(&c);
    }
}

//==============================================================================
// Query Operations
//==============================================================================

std::vector<Creature*> SpatialIndex::queryRadius(float x, float y, float radius) const {
    std::vector<Creature*> results;
    
    if (radius <= 0) return results;
    
    // Calculate cell range to check
    int minCellX = static_cast<int>(x - radius) / cellSize_;
    int maxCellX = static_cast<int>(x + radius) / cellSize_;
    int minCellY = static_cast<int>(y - radius) / cellSize_;
    int maxCellY = static_cast<int>(y + radius) / cellSize_;
    
    // Clamp to grid bounds
    minCellX = std::max(0, minCellX);
    maxCellX = std::min(cellsX_ - 1, maxCellX);
    minCellY = std::max(0, minCellY);
    maxCellY = std::min(cellsY_ - 1, maxCellY);
    
    float radiusSq = radius * radius;
    
    // Check each cell in range
    for (int cy = minCellY; cy <= maxCellY; ++cy) {
        for (int cx = minCellX; cx <= maxCellX; ++cx) {
            auto it = grid_.find(CellKey{cx, cy});
            if (it == grid_.end()) continue;
            
            // Check each creature in cell
            for (Creature* c : it->second) {
                float dx = c->getWorldX() - x;
                float dy = c->getWorldY() - y;
                if (dx * dx + dy * dy <= radiusSq) {
                    results.push_back(c);
                }
            }
        }
    }
    
    return results;
}

std::vector<Creature*> SpatialIndex::queryCell(int cellX, int cellY) const {
    CellKey key = clampCell(cellX, cellY);
    auto it = grid_.find(key);
    if (it != grid_.end()) {
        return it->second;
    }
    return {};
}

std::vector<Creature*> SpatialIndex::queryNearbyCells(float x, float y) const {
    std::vector<Creature*> results;
    
    auto [centerCellX, centerCellY] = getCellCoords(x, y);
    
    // Check 3x3 grid of cells centered on the position
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int cellX = centerCellX + dx;
            int cellY = centerCellY + dy;
            
            // Skip out-of-bounds cells
            if (cellX < 0 || cellX >= cellsX_ || cellY < 0 || cellY >= cellsY_) {
                continue;
            }
            
            auto it = grid_.find(CellKey{cellX, cellY});
            if (it != grid_.end()) {
                results.insert(results.end(), it->second.begin(), it->second.end());
            }
        }
    }
    
    return results;
}

std::vector<Creature*> SpatialIndex::queryWithFilter(
    float x, float y, float radius,
    std::function<bool(const Creature*)> predicate) const 
{
    std::vector<Creature*> results;
    
    if (radius <= 0 || !predicate) return results;
    
    // Calculate cell range to check
    int minCellX = static_cast<int>(x - radius) / cellSize_;
    int maxCellX = static_cast<int>(x + radius) / cellSize_;
    int minCellY = static_cast<int>(y - radius) / cellSize_;
    int maxCellY = static_cast<int>(y + radius) / cellSize_;
    
    // Clamp to grid bounds
    minCellX = std::max(0, minCellX);
    maxCellX = std::min(cellsX_ - 1, maxCellX);
    minCellY = std::max(0, minCellY);
    maxCellY = std::min(cellsY_ - 1, maxCellY);
    
    float radiusSq = radius * radius;
    
    // Check each cell in range
    for (int cy = minCellY; cy <= maxCellY; ++cy) {
        for (int cx = minCellX; cx <= maxCellX; ++cx) {
            auto it = grid_.find(CellKey{cx, cy});
            if (it == grid_.end()) continue;
            
            // Check each creature in cell
            for (Creature* c : it->second) {
                float dx = c->getWorldX() - x;
                float dy = c->getWorldY() - y;
                if (dx * dx + dy * dy <= radiusSq && predicate(c)) {
                    results.push_back(c);
                }
            }
        }
    }
    
    return results;
}

Creature* SpatialIndex::findNearest(
    float x, float y, float maxRadius,
    std::function<bool(const Creature*)> predicate) const 
{
    if (maxRadius <= 0 || !predicate) return nullptr;
    
    Creature* nearest = nullptr;
    float nearestDistSq = std::numeric_limits<float>::max();
    
    // Calculate cell range to check
    int minCellX = static_cast<int>(x - maxRadius) / cellSize_;
    int maxCellX = static_cast<int>(x + maxRadius) / cellSize_;
    int minCellY = static_cast<int>(y - maxRadius) / cellSize_;
    int maxCellY = static_cast<int>(y + maxRadius) / cellSize_;
    
    // Clamp to grid bounds
    minCellX = std::max(0, minCellX);
    maxCellX = std::min(cellsX_ - 1, maxCellX);
    minCellY = std::max(0, minCellY);
    maxCellY = std::min(cellsY_ - 1, maxCellY);
    
    float maxRadiusSq = maxRadius * maxRadius;
    
    // Check each cell in range
    for (int cy = minCellY; cy <= maxCellY; ++cy) {
        for (int cx = minCellX; cx <= maxCellX; ++cx) {
            auto it = grid_.find(CellKey{cx, cy});
            if (it == grid_.end()) continue;
            
            // Check each creature in cell
            for (Creature* c : it->second) {
                float dx = c->getWorldX() - x;
                float dy = c->getWorldY() - y;
                float distSq = dx * dx + dy * dy;
                
                if (distSq <= maxRadiusSq && distSq < nearestDistSq && predicate(c)) {
                    nearest = c;
                    nearestDistSq = distSq;
                }
            }
        }
    }
    
    return nearest;
}

//==============================================================================
// Utility
//==============================================================================

std::pair<int, int> SpatialIndex::getCellCoords(float x, float y) const {
    int cellX = static_cast<int>(x) / cellSize_;
    int cellY = static_cast<int>(y) / cellSize_;
    
    // Clamp to valid range
    cellX = std::max(0, std::min(cellX, cellsX_ - 1));
    cellY = std::max(0, std::min(cellY, cellsY_ - 1));
    
    return {cellX, cellY};
}

size_t SpatialIndex::size() const {
    return creatureCount_;
}

bool SpatialIndex::empty() const {
    return creatureCount_ == 0;
}

SpatialIndex::CellKey SpatialIndex::clampCell(int x, int y) const {
    return CellKey{
        std::max(0, std::min(x, cellsX_ - 1)),
        std::max(0, std::min(y, cellsY_ - 1))
    };
}

} // namespace EcoSim
