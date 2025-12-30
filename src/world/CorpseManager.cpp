/**
 * @file CorpseManager.cpp
 * @brief Implementation of CorpseManager for corpse lifecycle management
 */

#include "../../include/world/CorpseManager.hpp"
#include <algorithm>

namespace EcoSim {

//==============================================================================
// Construction
//==============================================================================

CorpseManager::CorpseManager(size_t maxCorpses)
    : _maxCorpses(maxCorpses) {
}

//==============================================================================
// Corpse Lifecycle
//==============================================================================

void CorpseManager::addCorpse(float x, float y, float size, 
                               const std::string& speciesName, float bodyCondition) {
    // If at capacity, remove the most decayed corpse first
    if (_corpses.size() >= _maxCorpses) {
        auto it = std::max_element(_corpses.begin(), _corpses.end(),
            [](const auto& a, const auto& b) {
                return a->getDecayProgress() < b->getDecayProgress();
            });
        if (it != _corpses.end()) {
            _corpses.erase(it);
        }
    }
    
    _corpses.push_back(std::make_unique<world::Corpse>(x, y, size, speciesName, bodyCondition));
}

void CorpseManager::tick() {
    // Advance decay for all corpses
    for (auto& corpse : _corpses) {
        corpse->tick();
    }
    
    // Remove fully decayed corpses
    removeExpiredCorpses();
}

void CorpseManager::removeExpiredCorpses() {
    _corpses.erase(
        std::remove_if(_corpses.begin(), _corpses.end(),
            [](const auto& c) { return c->isFullyDecayed(); }),
        _corpses.end()
    );
}

void CorpseManager::removeCorpse(world::Corpse* corpse) {
    _corpses.erase(
        std::remove_if(_corpses.begin(), _corpses.end(),
            [corpse](const auto& c) { return c.get() == corpse; }),
        _corpses.end()
    );
}

void CorpseManager::clear() {
    _corpses.clear();
}

//==============================================================================
// Queries
//==============================================================================

std::vector<world::Corpse*> CorpseManager::getCorpsesAt(int x, int y) {
    std::vector<world::Corpse*> result;
    
    for (auto& corpse : _corpses) {
        if (corpse->getTileX() == x && corpse->getTileY() == y) {
            result.push_back(corpse.get());
        }
    }
    
    return result;
}

std::vector<const world::Corpse*> CorpseManager::getCorpsesAt(int x, int y) const {
    std::vector<const world::Corpse*> result;
    
    for (const auto& corpse : _corpses) {
        if (corpse->getTileX() == x && corpse->getTileY() == y) {
            result.push_back(corpse.get());
        }
    }
    
    return result;
}

std::vector<world::Corpse*> CorpseManager::getCorpsesInRadius(float x, float y, float radius) {
    std::vector<world::Corpse*> result;
    float radiusSq = radius * radius;
    
    for (auto& corpse : _corpses) {
        if (distanceSquared(corpse->getX(), corpse->getY(), x, y) <= radiusSq) {
            result.push_back(corpse.get());
        }
    }
    
    return result;
}

std::vector<const world::Corpse*> CorpseManager::getCorpsesInRadius(float x, float y, float radius) const {
    std::vector<const world::Corpse*> result;
    float radiusSq = radius * radius;
    
    for (const auto& corpse : _corpses) {
        if (distanceSquared(corpse->getX(), corpse->getY(), x, y) <= radiusSq) {
            result.push_back(corpse.get());
        }
    }
    
    return result;
}

world::Corpse* CorpseManager::findNearest(float x, float y, float maxRange) {
    world::Corpse* nearest = nullptr;
    float nearestDistSq = maxRange * maxRange;
    
    for (auto& corpse : _corpses) {
        // Skip exhausted corpses
        if (corpse->isExhausted()) continue;
        
        float distSq = distanceSquared(corpse->getX(), corpse->getY(), x, y);
        
        if (distSq < nearestDistSq) {
            nearestDistSq = distSq;
            nearest = corpse.get();
        }
    }
    
    return nearest;
}

const world::Corpse* CorpseManager::findNearest(float x, float y, float maxRange) const {
    const world::Corpse* nearest = nullptr;
    float nearestDistSq = maxRange * maxRange;
    
    for (const auto& corpse : _corpses) {
        // Skip exhausted corpses
        if (corpse->isExhausted()) continue;
        
        float distSq = distanceSquared(corpse->getX(), corpse->getY(), x, y);
        
        if (distSq < nearestDistSq) {
            nearestDistSq = distSq;
            nearest = corpse.get();
        }
    }
    
    return nearest;
}

const std::vector<std::unique_ptr<world::Corpse>>& CorpseManager::getAll() const {
    return _corpses;
}

//==============================================================================
// Statistics
//==============================================================================

size_t CorpseManager::count() const {
    return _corpses.size();
}

bool CorpseManager::empty() const {
    return _corpses.empty();
}

float CorpseManager::getTotalNutrition() const {
    float total = 0.0f;
    for (const auto& corpse : _corpses) {
        total += corpse->getNutritionalValue();
    }
    return total;
}

float CorpseManager::getTotalNutritionAt(int x, int y) const {
    float total = 0.0f;
    for (const auto& corpse : _corpses) {
        if (corpse->getTileX() == x && corpse->getTileY() == y) {
            total += corpse->getNutritionalValue();
        }
    }
    return total;
}

//==============================================================================
// Private Helpers
//==============================================================================

float CorpseManager::distanceSquared(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

} // namespace EcoSim
