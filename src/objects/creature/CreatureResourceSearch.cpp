/**
 * @file CreatureResourceSearch.cpp
 * @brief Resource search functionality extracted from Creature class
 *
 * This module handles plant finding, water finding, and mate finding.
 * Part of Phase 1e creature decomposition to improve maintainability.
 */

#include "objects/creature/CreatureResourceSearch.hpp"
#include "objects/creature/creature.hpp"
#include "objects/creature/navigator.hpp"
#include "objects/creature/CreatureScent.hpp"
#include "objects/creature/CreaturePlantInteraction.hpp"
#include "world/world.hpp"
#include "world/tile.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "logging/Logger.hpp"

#include <cmath>
#include <algorithm>

namespace CreatureResourceSearch {

//============================================================================
//  Water Finding
//============================================================================

bool waterCheck(
    Creature& creature,
    const std::vector<std::vector<Tile>>& map,
    unsigned rows,
    unsigned cols,
    int x,
    int y) {
    
    if (Navigator::boundaryCheck(x, y, rows, cols)) {
        if (map.at(x).at(y).isSource()) {
            if (Navigator::astarSearch(creature, map, rows, cols, x, y)) {
                return true;
            }
        }
    }
    return false;
}

bool findWater(
    Creature& creature,
    const std::vector<std::vector<Tile>>& map,
    int rows,
    int cols) {
    
    // If on water source, drink from it
    if (map.at(creature.tileX()).at(creature.tileY()).isSource()) {
        creature.setAction(Action::Drinking);
        creature.setThirst(Creature::RESOURCE_LIMIT);
        return true;
    }
    
    // Use spiral search pattern
    unsigned sightRange = creature.getSightRange();
    int tx = creature.tileX();
    int ty = creature.tileY();
    
    for (int radius = 1; radius < static_cast<int>(sightRange); radius++) {
        // Top and Bottom Lines
        for (int xMod = -radius; xMod <= radius; xMod++) {
            int curX = tx + xMod;
            int curY = ty + radius;
            if (waterCheck(creature, map, rows, cols, curX, curY)) return true;
            curY = ty - radius;
            if (waterCheck(creature, map, rows, cols, curX, curY)) return true;
        }
        
        // Right and Left Lines (excluding corners already checked)
        for (int yMod = -radius + 1; yMod <= radius - 1; yMod++) {
            int curX = tx + radius;
            int curY = ty + yMod;
            if (waterCheck(creature, map, rows, cols, curX, curY)) return true;
            curX = tx - radius;
            if (waterCheck(creature, map, rows, cols, curX, curY)) return true;
        }
    }
    return false;
}

//============================================================================
//  Plant Finding
//============================================================================

/**
 * @deprecated This O(r²) tile iteration implementation is superseded by
 * Creature::findGeneticsPlants() which uses PlantSpatialIndex for O(1) queries.
 * Kept for A/B benchmark comparison - enable PLANT_SEARCH_USE_TILE_ITERATION
 * in creature.cpp to use this legacy implementation for performance testing.
 */
bool findGeneticsPlants(
    Creature& creature,
    World& world,
    unsigned& feedingCounter) {
    
    using namespace EcoSim::Genetics;
    
    // Ensure phenotype is available
    if (!creature.getPhenotype().hasTrait("LIFESPAN")) {
        return false;
    }
    
    std::vector<std::vector<Tile>>& map = world.getGrid();
    const int rows = world.getRows();
    const int cols = world.getCols();
    
    // Check if we're standing on a tile with edible plants
    Tile& currentTile = map[creature.tileX()][creature.tileY()];
    auto& plantsHere = currentTile.getPlants();
    
    for (auto& plantPtr : plantsHere) {
        if (!plantPtr || !plantPtr->isAlive()) {
            continue;
        }
        
        // Check if we can eat this plant (detection + defenses)
        bool canEat = creature.canEatPlant(*plantPtr);
        
        if (canEat) {
            // Attempt to eat the plant
            FeedingResult result = creature.eatPlant(*plantPtr);
            
            if (result.success) {
                creature.setAction(Action::Grazing);
                feedingCounter++;
                
                // Successful feeding significantly increases mate (reduces discomfort)
                float mate = creature.getMate();
                mate += 2.0f;  // Large fixed boost from eating
                if (mate > Creature::RESOURCE_LIMIT) mate = Creature::RESOURCE_LIMIT;
                creature.setMate(mate);
                
                // Log the feeding event
                logging::Logger::getInstance().feeding(
                    creature.getId(), plantPtr->getId(), result.success,
                    result.nutritionGained, result.damageReceived
                );
                
                return true;
            }
        }
    }
    
    // Search nearby tiles for edible plants
    Plant* closestPlant = nullptr;
    int closestX = -1, closestY = -1;
    float closestDistance = creature.getPlantDetectionRange();
    
    unsigned maxRadius = static_cast<unsigned>(creature.getPlantDetectionRange());
    int tx = creature.tileX();
    int ty = creature.tileY();
    
    // Iterate through tiles in range
    for (unsigned radius = 1; radius < maxRadius; radius++) {
        // Top and Bottom edges
        for (int xMod = -static_cast<int>(radius); xMod <= static_cast<int>(radius); xMod++) {
            int curX = tx + xMod;
            
            // Top edge
            int curY = ty - static_cast<int>(radius);
            if (Navigator::boundaryCheck(curX, curY, rows, cols)) {
                Tile& tile = map[curX][curY];
                auto& plants = tile.getPlants();
                for (auto& plantPtr : plants) {
                    if (plantPtr && plantPtr->isAlive() && creature.canEatPlant(*plantPtr)) {
                        float distance = creature.calculateDistance(curX, curY);
                        if (distance < closestDistance) {
                            closestDistance = distance;
                            closestPlant = plantPtr.get();
                            closestX = curX;
                            closestY = curY;
                        }
                    }
                }
            }
            
            // Bottom edge
            curY = ty + static_cast<int>(radius);
            if (Navigator::boundaryCheck(curX, curY, rows, cols)) {
                Tile& tile = map[curX][curY];
                auto& plants = tile.getPlants();
                for (auto& plantPtr : plants) {
                    if (plantPtr && plantPtr->isAlive() && creature.canEatPlant(*plantPtr)) {
                        float distance = creature.calculateDistance(curX, curY);
                        if (distance < closestDistance) {
                            closestDistance = distance;
                            closestPlant = plantPtr.get();
                            closestX = curX;
                            closestY = curY;
                        }
                    }
                }
            }
        }
        
        // Left and Right edges (excluding corners already visited)
        for (int yMod = -static_cast<int>(radius) + 1; yMod <= static_cast<int>(radius) - 1; yMod++) {
            int curY = ty + yMod;
            
            // Left edge
            int curX = tx - static_cast<int>(radius);
            if (Navigator::boundaryCheck(curX, curY, rows, cols)) {
                Tile& tile = map[curX][curY];
                auto& plants = tile.getPlants();
                for (auto& plantPtr : plants) {
                    if (plantPtr && plantPtr->isAlive() && creature.canEatPlant(*plantPtr)) {
                        float distance = creature.calculateDistance(curX, curY);
                        if (distance < closestDistance) {
                            closestDistance = distance;
                            closestPlant = plantPtr.get();
                            closestX = curX;
                            closestY = curY;
                        }
                    }
                }
            }
            
            // Right edge
            curX = tx + static_cast<int>(radius);
            if (Navigator::boundaryCheck(curX, curY, rows, cols)) {
                Tile& tile = map[curX][curY];
                auto& plants = tile.getPlants();
                for (auto& plantPtr : plants) {
                    if (plantPtr && plantPtr->isAlive() && creature.canEatPlant(*plantPtr)) {
                        float distance = creature.calculateDistance(curX, curY);
                        if (distance < closestDistance) {
                            closestDistance = distance;
                            closestPlant = plantPtr.get();
                            closestX = curX;
                            closestY = curY;
                        }
                    }
                }
            }
        }
    }
    
    // If we found a plant, navigate toward it
    if (closestPlant != nullptr && closestX >= 0 && closestY >= 0) {
        // Decrement mate while seeking food (discomfort from hunger)
        float mate = creature.getMate();
        mate -= creature.getComfDec() * 0.5f;  // Less harsh than not finding any food
        creature.setMate(mate);
        return Navigator::astarSearch(creature, map, rows, cols, closestX, closestY);
    }
    
    // Scent-based fallback - try to smell food when vision fails
    if (creature.hasScentDetection()) {
        int scentTargetX = -1, scentTargetY = -1;
        
        if (creature.findFoodScent(world.getScentLayer(), scentTargetX, scentTargetY)) {
            // Decrement mate while following scent (discomfort from hunger)
            float mate = creature.getMate();
            mate -= creature.getComfDec() * CreaturePlantInteraction::SEEKING_FOOD_MATE_PENALTY;
            creature.setMate(mate);
            return Navigator::astarSearch(creature, map, rows, cols, scentTargetX, scentTargetY);
        }
    }
    
    return false;
}

//============================================================================
//  Mate Finding
//============================================================================

Creature* findBestMate(
    const Creature& creature,
    std::vector<Creature>& creatures) {
    
    float bestDesirability = -1.0f;
    Creature* bestMate = nullptr;
    
    // Attempt to find the most desirable mate
    unsigned sightRange = creature.getSightRange();
    
    for (Creature& other : creatures) {
        bool isPotentialMate = &other != &creature
            && other.getProfile() == Profile::breed;
        
        if (isPotentialMate) {
            // Calculate the distance between the creatures
            unsigned diffX = abs(creature.tileX() - other.getX());
            unsigned diffY = abs(creature.tileY() - other.getY());
            
            if (diffX < sightRange && diffY < sightRange) {
                float desirability = creature.checkFitness(other);
                if (desirability > bestDesirability) {
                    bestMate = &other;
                    bestDesirability = desirability;
                }
            }
        }
    }
    
    return bestMate;
}

bool navigateToMate(
    Creature& creature,
    const std::vector<std::vector<Tile>>& map,
    int rows,
    int cols,
    const Creature& mate) {
    
    int mateX = mate.getX();
    int mateY = mate.getY();
    
    if (isMateAdjacent(creature, mate)) {
        creature.setAction(Action::Mating);
        return true;
    }
    
    // Move towards mate
    return Navigator::astarSearch(creature, map, rows, cols, mateX, mateY);
}

bool isMateAdjacent(
    const Creature& creature,
    const Creature& mate) {
    
    unsigned diffX = abs(creature.tileX() - mate.getX());
    unsigned diffY = abs(creature.tileY() - mate.getY());
    
    return (diffX <= 1 && diffY <= 1);
}

} // namespace CreatureResourceSearch
