/**
 * @file PlantSpatialIndex.cpp
 * @brief Implementation of grid-based spatial index for plants.
 */

#include "../../include/world/PlantSpatialIndex.hpp"
#include "../../include/genetics/organisms/Plant.hpp"
#include <cmath>
#include <algorithm>

namespace EcoSim {

PlantSpatialIndex::PlantSpatialIndex(int worldWidth, int worldHeight, int cellSize)
    : worldWidth_(worldWidth)
    , worldHeight_(worldHeight)
    , cellSize_(cellSize)
    , plantCount_(0) {
    cellsX_ = (worldWidth + cellSize - 1) / cellSize;
    cellsY_ = (worldHeight + cellSize - 1) / cellSize;
}

//==============================================================================
// Core Operations
//==============================================================================

void PlantSpatialIndex::insert(Genetics::Plant* plant, int x, int y) {
    if (!plant) return;
    
    auto [cellX, cellY] = getCellCoords(static_cast<float>(x), static_cast<float>(y));
    CellKey key = clampCell(cellX, cellY);
    grid_[key].push_back(plant);
    ++plantCount_;
}

void PlantSpatialIndex::remove(Genetics::Plant* plant, int x, int y) {
    if (!plant) return;
    
    auto [cellX, cellY] = getCellCoords(static_cast<float>(x), static_cast<float>(y));
    CellKey key = clampCell(cellX, cellY);
    
    auto it = grid_.find(key);
    if (it != grid_.end()) {
        auto& vec = it->second;
        auto plantIt = std::find(vec.begin(), vec.end(), plant);
        if (plantIt != vec.end()) {
            vec.erase(plantIt);
            --plantCount_;
            
            // Remove empty cells to save memory
            if (vec.empty()) {
                grid_.erase(it);
            }
        }
    }
}

void PlantSpatialIndex::clear() {
    grid_.clear();
    plantCount_ = 0;
}

//==============================================================================
// Query Operations
//==============================================================================

std::vector<Genetics::Plant*> PlantSpatialIndex::queryRadius(float x, float y, float radius) const {
    std::vector<Genetics::Plant*> results;
    
    // Calculate which cells to check based on radius
    int cellRadius = static_cast<int>(std::ceil(radius / cellSize_)) + 1;
    auto [centerCellX, centerCellY] = getCellCoords(x, y);
    
    float radiusSquared = radius * radius;
    
    // Check all cells within the radius
    for (int dy = -cellRadius; dy <= cellRadius; ++dy) {
        for (int dx = -cellRadius; dx <= cellRadius; ++dx) {
            int checkCellX = centerCellX + dx;
            int checkCellY = centerCellY + dy;
            
            // Skip out-of-bounds cells
            if (checkCellX < 0 || checkCellX >= cellsX_ ||
                checkCellY < 0 || checkCellY >= cellsY_) {
                continue;
            }
            
            CellKey key{checkCellX, checkCellY};
            auto it = grid_.find(key);
            if (it == grid_.end()) continue;
            
            // Check each plant in the cell
            for (Genetics::Plant* plant : it->second) {
                if (!plant) continue;
                
                float px = static_cast<float>(plant->getX());
                float py = static_cast<float>(plant->getY());
                float dx2 = px - x;
                float dy2 = py - y;
                float distSquared = dx2 * dx2 + dy2 * dy2;
                
                if (distSquared <= radiusSquared) {
                    results.push_back(plant);
                }
            }
        }
    }
    
    return results;
}

std::vector<Genetics::Plant*> PlantSpatialIndex::queryCell(int cellX, int cellY) const {
    CellKey key = clampCell(cellX, cellY);
    auto it = grid_.find(key);
    if (it != grid_.end()) {
        return it->second;
    }
    return {};
}

//==============================================================================
// Utility
//==============================================================================

std::pair<int, int> PlantSpatialIndex::getCellCoords(float x, float y) const {
    int cellX = static_cast<int>(x) / cellSize_;
    int cellY = static_cast<int>(y) / cellSize_;
    return {cellX, cellY};
}

size_t PlantSpatialIndex::size() const {
    return plantCount_;
}

bool PlantSpatialIndex::empty() const {
    return plantCount_ == 0;
}

PlantSpatialIndex::CellKey PlantSpatialIndex::clampCell(int x, int y) const {
    x = std::max(0, std::min(x, cellsX_ - 1));
    y = std::max(0, std::min(y, cellsY_ - 1));
    return CellKey{x, y};
}

} // namespace EcoSim
