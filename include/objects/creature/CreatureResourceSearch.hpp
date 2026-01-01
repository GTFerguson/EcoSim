#ifndef CREATURE_RESOURCE_SEARCH_HPP
#define CREATURE_RESOURCE_SEARCH_HPP

/**
 * @file CreatureResourceSearch.hpp
 * @brief Resource search functionality extracted from Creature class
 *
 * This module handles plant finding, water finding, and mate finding.
 * Part of Phase 1e creature decomposition to improve maintainability.
 */

#include <vector>
#include <optional>
#include <utility>

// Forward declarations
class Creature;
class World;
class Tile;

namespace EcoSim {
namespace Genetics {
class Plant;
}
}

namespace CreatureResourceSearch {

//============================================================================
//  Water Finding
//============================================================================

/**
 * @brief Check if a tile contains a water source and path to it.
 *
 * Validates tile coordinates, checks for water source, and attempts
 * to navigate to the water using A* pathfinding.
 *
 * @param creature The creature searching for water
 * @param map Reference to the world map grid
 * @param rows Number of rows on the map
 * @param cols Number of columns on the map
 * @param x X coordinate to check
 * @param y Y coordinate to check
 * @return True if water found and path initiated
 */
bool waterCheck(
    Creature& creature,
    const std::vector<std::vector<Tile>>& map,
    unsigned rows,
    unsigned cols,
    int x,
    int y);

/**
 * @brief Search for water using spiral search pattern.
 *
 * Uses the creature's sight range to search for water sources.
 * If standing on water, drinks immediately. Otherwise uses spiral
 * search to find nearest water source.
 *
 * @param creature The creature searching for water
 * @param map Reference to the world map grid
 * @param rows Number of rows on the map
 * @param cols Number of columns on the map
 * @return True if water found and action taken (drinking or navigating)
 */
bool findWater(
    Creature& creature,
    const std::vector<std::vector<Tile>>& map,
    int rows,
    int cols);

//============================================================================
//  Plant Finding
//============================================================================

/**
 * @brief Search for nearby genetics-based plants and attempt to eat them.
 *
 * Searches the creature's sight range for edible plants and navigates
 * toward or eats them using the genetics feeding system. Falls back to
 * scent-based navigation when visual search fails.
 *
 * @param creature The creature searching for plants
 * @param world Reference to the world object
 * @param feedingCounter Counter for tracking feeding events
 * @return True if an action was taken (eating or moving toward plant)
 */
bool findGeneticsPlants(
    Creature& creature,
    World& world,
    unsigned& feedingCounter);

//============================================================================
//  Mate Finding
//============================================================================

/**
 * @brief Find the best compatible mate within sight range.
 *
 * Scans all creatures in visual range for compatible mates (breeding
 * profile, genetic compatibility). Uses checkFitness() to select the
 * most desirable mate based on proximity and genetic similarity.
 *
 * @param creature The creature searching for a mate
 * @param creatures Vector of all creatures in the world
 * @return Pointer to best mate, or nullptr if none found
 */
Creature* findBestMate(
    const Creature& creature,
    std::vector<Creature>& creatures);

/**
 * @brief Navigate toward a mate or return true if adjacent.
 *
 * If mate is adjacent (within 1 tile), returns true without moving.
 * Otherwise initiates A* pathfinding toward the mate.
 *
 * @param creature The creature seeking the mate
 * @param map Reference to the world map grid
 * @param rows Number of rows on the map
 * @param cols Number of columns on the map
 * @param mate The target mate creature
 * @return True if mate is adjacent or navigation started
 */
bool navigateToMate(
    Creature& creature,
    const std::vector<std::vector<Tile>>& map,
    int rows,
    int cols,
    const Creature& mate);

/**
 * @brief Check if mate is adjacent (within 1 tile).
 *
 * @param creature The searching creature
 * @param mate The target mate
 * @return True if within 1 tile in both X and Y
 */
bool isMateAdjacent(
    const Creature& creature,
    const Creature& mate);

} // namespace CreatureResourceSearch

#endif // CREATURE_RESOURCE_SEARCH_HPP
