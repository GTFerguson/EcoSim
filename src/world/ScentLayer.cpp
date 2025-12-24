/**
 * @file ScentLayer.cpp
 * @brief Implementation of the scent-based environmental layer
 * 
 * Phase 1 of the Sensory System. Provides sparse, performant scent storage
 * that enables creatures to detect and follow pheromone trails for mate finding.
 */

#include "world/ScentLayer.hpp"
#include <algorithm>
#include <cmath>

namespace EcoSim {

ScentLayer::ScentLayer()
    : _width(0)
    , _height(0)
    , _decayInterval(10)
    , _lastDecayTick(0)
{
}

ScentLayer::ScentLayer(int width, int height, unsigned int decayInterval)
    : _width(width)
    , _height(height)
    , _decayInterval(decayInterval)
    , _lastDecayTick(0)
{
}

void ScentLayer::initialize(int width, int height) {
    _width = width;
    _height = height;
    clear();
}

void ScentLayer::deposit(int x, int y, const ScentDeposit& scent) {
    if (!isInBounds(x, y)) {
        return;
    }
    
    auto key = std::make_pair(x, y);
    auto& deposits = _scents[key];
    
    // Check if we should refresh an existing scent from same creature of same type
    for (auto& existing : deposits) {
        if (existing.creatureId == scent.creatureId && existing.type == scent.type) {
            // Refresh the scent instead of adding duplicate
            existing.intensity = std::max(existing.intensity, scent.intensity);
            existing.tickDeposited = scent.tickDeposited;
            existing.decayRate = scent.decayRate;
            existing.signature = scent.signature;
            return;
        }
    }
    
    // Add new scent deposit
    deposits.push_back(scent);
}

std::vector<ScentDeposit> ScentLayer::getScentsAt(int x, int y) const {
    if (!isInBounds(x, y)) {
        return {};
    }
    
    auto key = std::make_pair(x, y);
    auto it = _scents.find(key);
    
    if (it != _scents.end()) {
        return it->second;
    }
    
    return {};
}

std::vector<ScentDeposit> ScentLayer::getScentsOfType(int x, int y, ScentType type) const {
    std::vector<ScentDeposit> result;
    
    auto allScents = getScentsAt(x, y);
    for (const auto& scent : allScents) {
        if (scent.type == type) {
            result.push_back(scent);
        }
    }
    
    return result;
}

ScentDeposit ScentLayer::getStrongestScentInRadius(
    int centerX, int centerY, int radius,
    ScentType type, int& outX, int& outY) const 
{
    ScentDeposit strongest;
    float strongestIntensity = 0.0f;
    bool found = false;
    
    // Search in a square area (could optimize to circle if needed)
    int minX = std::max(0, centerX - radius);
    int maxX = std::min(_width - 1, centerX + radius);
    int minY = std::max(0, centerY - radius);
    int maxY = std::min(_height - 1, centerY + radius);
    
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            // Check if within circular radius
            int dx = x - centerX;
            int dy = y - centerY;
            if (dx * dx + dy * dy > radius * radius) {
                continue;
            }
            
            auto scents = getScentsOfType(x, y, type);
            for (const auto& scent : scents) {
                if (scent.intensity > strongestIntensity) {
                    strongest = scent;
                    strongestIntensity = scent.intensity;
                    outX = x;
                    outY = y;
                    found = true;
                }
            }
        }
    }
    
    if (!found) {
        outX = centerX;
        outY = centerY;
    }
    
    return strongest;
}

std::vector<std::tuple<ScentDeposit, int, int>> ScentLayer::getScentsInRadius(
    int centerX, int centerY, int radius, ScentType type) const
{
    std::vector<std::tuple<ScentDeposit, int, int>> result;
    
    // Search in a square area then filter to circle
    int minX = std::max(0, centerX - radius);
    int maxX = std::min(_width - 1, centerX + radius);
    int minY = std::max(0, centerY - radius);
    int maxY = std::min(_height - 1, centerY + radius);
    
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            // Check if within circular radius
            int dx = x - centerX;
            int dy = y - centerY;
            if (dx * dx + dy * dy > radius * radius) {
                continue;
            }
            
            auto scents = getScentsOfType(x, y, type);
            for (const auto& scent : scents) {
                result.push_back(std::make_tuple(scent, x, y));
            }
        }
    }
    
    return result;
}

void ScentLayer::update(unsigned int currentTick) {
    // Only process decay at configured interval for performance
    if (currentTick - _lastDecayTick >= _decayInterval) {
        processDecay(currentTick);
        _lastDecayTick = currentTick;
    }
}

void ScentLayer::processDecay(unsigned int currentTick) {
    // Iterate through all tiles with scents
    auto it = _scents.begin();
    while (it != _scents.end()) {
        auto& deposits = it->second;
        
        // Remove decayed scents from this tile
        deposits.erase(
            std::remove_if(deposits.begin(), deposits.end(),
                [currentTick](const ScentDeposit& s) {
                    return s.isDecayed(currentTick);
                }),
            deposits.end()
        );
        
        // Update remaining scent intensities
        for (auto& scent : deposits) {
            scent.intensity = scent.getDecayedIntensity(currentTick);
            // Reset tick so the new intensity becomes the baseline
            scent.tickDeposited = currentTick;
        }
        
        // Remove tile entry if no scents remain
        if (deposits.empty()) {
            it = _scents.erase(it);
        } else {
            ++it;
        }
    }
}

void ScentLayer::clear() {
    _scents.clear();
}

void ScentLayer::removeScentsFromCreature(int creatureId) {
    auto it = _scents.begin();
    while (it != _scents.end()) {
        auto& deposits = it->second;
        
        deposits.erase(
            std::remove_if(deposits.begin(), deposits.end(),
                [creatureId](const ScentDeposit& s) {
                    return s.creatureId == creatureId;
                }),
            deposits.end()
        );
        
        if (deposits.empty()) {
            it = _scents.erase(it);
        } else {
            ++it;
        }
    }
}

size_t ScentLayer::getActiveTileCount() const {
    return _scents.size();
}

size_t ScentLayer::getTotalScentCount() const {
    size_t total = 0;
    for (const auto& [coords, deposits] : _scents) {
        total += deposits.size();
    }
    return total;
}

} // namespace EcoSim
