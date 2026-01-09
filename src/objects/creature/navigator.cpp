/**
 * Title    : EcoSim - Navigator
 * Author   : Gary Ferguson
 * Created  : Oct 18th, 2019
 * Purpose  : This class provides pathfinding and movement utilities for creatures.
 *            Implements A* search algorithm for intelligent navigation around obstacles,
 *            plus helper methods for wandering, moving towards/away from targets, and
 *            boundary checking.
 *
 *            Phase 3 Enhancement: Environmental sensitivity in pathfinding.
 *            Creatures with high ENVIRONMENTAL_SENSITIVITY gene will avoid hostile
 *            biomes (temperatures outside their tolerance), while low-sensitivity
 *            creatures prioritize direct paths.
 */

#include "../../../include/objects/creature/navigator.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentalStress.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "world/EnvironmentSystem.hpp"

//  Adjusts movement cost for diagonal
const float Navigator::DIAG_ADJUST = 1.4f;

//  Static random engine for wander() - avoids recreation every call (CREATURE-009 fix)
static std::random_device s_rd;
static std::mt19937 s_wanderGen(s_rd());

using namespace std;
using namespace EcoSim::Genetics;

//================================================================================
//  PathfindingContext Implementation
//================================================================================

PathfindingContext PathfindingContext::fromCreature(
    const EcoSim::Genetics::Phenotype& phenotype,
    const EcoSim::EnvironmentSystem* envSystem
) {
    // Extract thermal adaptations from phenotype genes
    ThermalAdaptations adaptations =
        EnvironmentalStressCalculator::extractThermalAdaptations(phenotype);
    
    // Get base tolerance values from genes using getTrait() API
    float baseTolMin = phenotype.getTrait("TEMPERATURE_TOLERANCE_MIN");
    float baseTolMax = phenotype.getTrait("TEMPERATURE_TOLERANCE_MAX");
    
    // Calculate effective tolerance range with adaptations applied
    EffectiveToleranceRange range =
        EnvironmentalStressCalculator::calculateEffectiveTempRange(
            baseTolMin, baseTolMax, adaptations
        );
    
    // Get environmental sensitivity from gene (determines how much creature avoids danger)
    float sensitivity = phenotype.getTrait(UniversalGenes::ENVIRONMENTAL_SENSITIVITY);
    
    return PathfindingContext{
        range.tempMin,
        range.tempMax,
        sensitivity,
        envSystem
    };
}

float PathfindingContext::calculateTileCost(float baseCost, int x, int y) const {
    // If no environment system or sensitivity is negligible, use base cost only
    if (!envSystem || environmentalSensitivity < 0.01f) {
        return baseCost;
    }
    
    // Get temperature at the tile
    float temp = envSystem->getTemperature(x, y);
    
    // Calculate how many degrees outside tolerance the temperature is
    float degreesOutside = 0.0f;
    if (temp < effectiveTolMin) {
        degreesOutside = effectiveTolMin - temp;
    } else if (temp > effectiveTolMax) {
        degreesOutside = temp - effectiveTolMax;
    }
    // If temp is within tolerance, degreesOutside remains 0
    
    // Apply formula: cost = baseCost + (degreesOutside/10) * sensitivity * DANGER_WEIGHT_FACTOR
    // This makes tiles 10°C outside tolerance cost an extra sensitivity*10 movement units
    float environmentalCost = (degreesOutside / 10.0f) * environmentalSensitivity * DANGER_WEIGHT_FACTOR;
    
    return baseCost + environmentalCost;
}

//================================================================================
//  Helper methods
//================================================================================
/**
 *  Checks if a node with matching coordinates already exists within a coordinate set.
 *  Uses O(1) lookup with unordered_set instead of O(n) linear search (CREATURE-006 fix).
 *
 *  @param coordSet  The coordinate set to be searched.
 *  @param x         X coordinate to search for.
 *  @param y         Y coordinate to search for.
 *  @return          Whether the coordinates could be found within the set.
 */
bool Navigator::nodeInCoordSet (const CoordSet &coordSet, int x, int y) {
  return coordSet.find({x, y}) != coordSet.end();
}

/**
 *  This checks if a node is valid and if so it will add that node to the set.
 *  (CREATURE-007 fix: closedCoords and openCoords passed by const reference)
 *
 *  @param openSet      The set being populated.
 *  @param openCoords   Coordinate set for O(1) open set lookup.
 *  @param closedCoords Coordinate set for O(1) closed set lookup.
 *  @param curTile      The tile to be checked.
 *  @param parent       The parent node.
 *  @param gCost        The cost to move from start to this tile.
 *  @param curX         X-position of tile.
 *  @param curY         Y-position of tile.
 *  @param endX         X-position of end tile.
 *  @param endY         Y-position of end tile.
 */
void Navigator::validateNode (set<Node, nodeCompare> &openSet,
                              CoordSet &openCoords,
                              const CoordSet &closedCoords,
                              const Tile &curTile,
                              const Node *parent,
                              const int &gCost,
                              const int &curX,
                              const int &curY,
                              const int &endX,
                              const int &endY) {
  if (curTile.isPassable() && !(curX == parent->getX() && curY == parent->getY())) {
    // Use O(1) coordinate lookup instead of O(n) set iteration (CREATURE-006 fix)
    if (!nodeInCoordSet(closedCoords, curX, curY) && !nodeInCoordSet(openCoords, curX, curY)) {
      Node newNode (curX, curY, parent);
      newNode.setG (gCost);
      newNode.setH (endX, endY);
      openSet.insert (newNode);
      openCoords.insert({curX, curY});
    }
  }
}

/**
 *  This method is used to check that the coordinates given are within the bounds.
 *
 *  @param x    X coordinate.
 *  @param y    Y coordinate.
 *  @param rows Number of rows in the given map.
 *  @param cols Number of columns in the given map.
 *  @return     Returns true if coordinates are within the 2D grid.
 */
bool Navigator::boundaryCheck (const int &x,    const int &y, 
                               const int &rows, const int &cols) {
  if (x < cols && x >= 0 && y < rows && y >= 0)
    return true;
  else
    return false;
}

/**
 *  Checks each neighbouring Tile object to the node given.
 *  These Tiles can then be turned into Nodes, validated, then
 *  added to the openSet.
 *
 *  Phase 3 Enhancement: When a PathfindingContext is provided, tile costs
 *  are modified based on environmental danger (temperature outside tolerance).
 *  This causes creatures with high ENVIRONMENTAL_SENSITIVITY to prefer safe routes.
 *
 *  @param map          A reference to the world map
 *  @param curNode      The current node being expanded.
 *  @param openSet      The open set for A* search.
 *  @param openCoords   Coordinate set for O(1) open set lookup.
 *  @param closedCoords Coordinate set for O(1) closed set lookup.
 *  @param rows         Number of rows in map.
 *  @param cols         Number of columns in map.
 *  @param endX         X position of end tile.
 *  @param endY         Y position of end tile.
 *  @param ctx          Optional PathfindingContext for environmental cost calculation.
 *                      If nullptr, uses base costs only (backward compatible).
 */
void Navigator::checkNeighbours (const vector<vector<Tile>> &map,
                                 const Node &curNode,
                                 set<Node, nodeCompare> &openSet,
                                 CoordSet &openCoords,
                                 const CoordSet &closedCoords,
                                 const int &rows,
                                 const int &cols,
                                 const int &endX,
                                 const int &endY,
                                 const PathfindingContext* ctx) {
  int curX, curY;
  float gCost;  // Changed to float to support environmental costs
  Tile curTile;
  //  Diagonals
  for (int xMod = -1; xMod < 2; xMod += 2) {
    for (int yMod = -1; yMod < 2; yMod += 2) {
      curX = curNode.getX() + xMod;
      curY = curNode.getY() + yMod;

      if (boundaryCheck (curX, curY, rows, cols)) {
        // Base diagonal cost
        float baseCost = static_cast<float>(DIAG_COST);
        // Apply environmental cost if context provided
        gCost = curNode.getG() + (ctx ? ctx->calculateTileCost(baseCost, curX, curY) : baseCost);
        curTile = map.at(curX).at(curY);
        validateNode  (openSet, openCoords, closedCoords, curTile, &curNode,
                       static_cast<int>(gCost), curX, curY, endX, endY);
      }
    }
  }

  //  Up + Down
  for (int yMod = -1; yMod < 2; yMod += 2) {
    curX = curNode.getX();
    curY = curNode.getY() + yMod;

    if (boundaryCheck (curX, curY, rows, cols)) {
      // Base orthogonal cost
      float baseCost = static_cast<float>(NORM_COST);
      // Apply environmental cost if context provided
      gCost = curNode.getG() + (ctx ? ctx->calculateTileCost(baseCost, curX, curY) : baseCost);
      curTile = map.at(curX).at(curY);
      validateNode  (openSet, openCoords, closedCoords, curTile, &curNode,
                     static_cast<int>(gCost), curX, curY, endX, endY);
    }
  }

  //  Left + Right
  for (int xMod = -1; xMod < 2; xMod += 2) {
    curX = curNode.getX() + xMod;
    curY = curNode.getY();

    if (boundaryCheck (curX, curY, rows, cols)) {
      // Base orthogonal cost
      float baseCost = static_cast<float>(NORM_COST);
      // Apply environmental cost if context provided
      gCost = curNode.getG() + (ctx ? ctx->calculateTileCost(baseCost, curX, curY) : baseCost);
      curTile = map.at(curX).at(curY);
      validateNode  (openSet, openCoords, closedCoords, curTile, &curNode,
               static_cast<int>(gCost), curX, curY, endX, endY);
    }
  }
}

void Navigator::movementCost (Creature &c, const int &x, const int &y) {
  if(x != 0 || y != 0) {
    //  First get non-diagonal movement by getting absolute difference
    int nondiag = abs (x - y);
    int diag    = max (x,  y) - nondiag;
    
    //  Times diag by 1.4 to adjust for extra cost
    c.setHunger (c.getHunger() - (c.getMetabolism() * (nondiag + diag * DIAG_ADJUST)));
  }
}

//================================================================================
//  Movement methods
//================================================================================
/**
 *  Performs an A* search (or as close as I could estimate to).
 *
 *  @param c      The creature that is trying to navigate.
 *  @param map    A reference to the world map.
 *  @param rows   Number of rows on the map.
 *  @param cols   Number of columns on the map.
 *  @param endX   End x pos.
 *  @param endY   End y pos.
 *  @return       Whether a path could be found or not.
 */
bool Navigator::astarSearch (Creature &c,
                             const vector<vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             const int &endX,
                             const int &endY,
                             const PathfindingContext* ctx) {
  int timeOut = 0;
  //  Sets of tiles already evaluated
  set<Node, nodeCompare> closedSet;
  //  Set of currently discovered tiles not yet evaluated
  set<Node, nodeCompare> openSet;
  //  Coordinate sets for O(1) lookup (CREATURE-006 fix)
  CoordSet closedCoords;
  CoordSet openCoords;

  //  Put starting node on set using stack allocation (CREATURE-008 fix)
  Node sNode (c.getX(), c.getY());
  sNode.setG (0);
  sNode.setH (endX, endY);
  openSet.insert (sNode);
  openCoords.insert({c.getX(), c.getY()});

  //  While openSet is not empty iterate through
  for (set<Node>::iterator iter = openSet.begin();
      !openSet.empty(); iter = openSet.begin()) {
    //  Check if at end goal
    if (iter->getX() == endX && iter->getY() == endY) {
      //  Retrace the nodes to build a path
      list<Node> path;
      Node pathNode = *iter;
      //  While pathNode has a parent node
      while (pathNode.getParent() != nullptr) {
        path.push_front (pathNode);
        pathNode = *pathNode.getParent();
      }
      
      moveTowards (c, map, rows, cols, path.begin()->getX(), path.begin()->getY());

      openSet.erase (iter);
      openSet.clear ();
      return true;
    } else {
      //  Push and Pop first so parent pointers don't get mixed up
      //  Store the node before erasing since iterator becomes invalid after erase
      Node currentNode = *iter;
      int nodeX = currentNode.getX();
      int nodeY = currentNode.getY();
      closedSet.insert (currentNode);
      closedCoords.insert({nodeX, nodeY});
      openSet.erase (iter);
      openCoords.erase({nodeX, nodeY});
      //  Find the inserted node in closedSet to pass to checkNeighbours
      auto closedIter = closedSet.find(currentNode);
      // Pass PathfindingContext to enable environmental cost calculation
      checkNeighbours (map, *closedIter, openSet, openCoords, closedCoords, rows, cols, endX, endY, ctx);
      timeOut++;
    }

    if (timeOut > MAX_NODES)
      break;
  }
  return false;
}

/**
 *  This method makes a creature move in a random direction using smooth movement.
 *  (CREATURE-009 fix: Uses static random engine instead of recreating each call)
 *  Phase 1: Float Movement System - creatures now move smoothly in random directions.
 *
 *  @param c    The creature moving.
 *  @param map  A reference to the 2D grid of the world.
 *  @param rows Number of rows on the map.
 *  @param cols Number of columns on the map.
 */
void Navigator::wander (Creature &c,
                        const vector<vector<Tile>> &map,
                        const unsigned rows,
                        const unsigned cols) {
  static uniform_int_distribution<short> change(-1, 1);

	int targetTileX = c.tileX() + (change(s_wanderGen));
	int targetTileY = c.tileY() + (change(s_wanderGen));
  moveTowards (c, map, rows, cols, targetTileX, targetTileY);
}

/**
 *  Simply moves the creature towards a target using smooth float movement.
 *  Phase 1: Float Movement System - creatures now move gradually toward
 *  their target tile, with movement speed determined by getMovementSpeed().
 *
 *  @param goalX  x pos of the target tile.
 *  @param goalY  y pos of the target tile.
 */
void Navigator::moveTowards (Creature &c,
                             const vector<vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             const int &goalX,
                             const int &goalY) {
  // Calculate the target position as the center of the goal tile
  // This gives a smooth movement toward the tile center
  float targetX = static_cast<float>(goalX) + 0.5f;
  float targetY = static_cast<float>(goalY) + 0.5f;
  
  // Move toward the target using float coordinates
  // deltaTime = 1.0 (one tick) by default
  move(c, map, rows, cols, targetX, targetY, 1.0f);
}

/**
 *  Simply moves the creature away from the target using smooth float movement.
 *  Phase 1: Float Movement System - creatures now move gradually away from
 *  threats, with movement speed determined by getMovementSpeed().
 *
 *  @param avoidX X-position to move away from.
 *  @param avoidY y-position to move away from.
 */
void Navigator::moveAway (Creature &c,
                          const vector<vector<Tile>> &map,
                          const int &rows,
                          const int &cols,
                          const int &avoidX,
                          const int &avoidY) {
  int curTileX = c.tileX();
  int curTileY = c.tileY();

  //  If at same tile location, wander randomly
  if (avoidX == curTileX && avoidY == curTileY) {
    wander (c, map, rows, cols);

  } else {
    // Calculate target tile (opposite direction from threat)
    int targetTileX = curTileX;
    int targetTileY = curTileY;
    
    //  Movement along X-Axis (away from avoidX)
    if      (avoidX < curTileX) { targetTileX++; }
    else if (avoidX > curTileX) { targetTileX--; }
    //  Movement along Y-Axis (away from avoidY)
    if      (avoidY < curTileY) { targetTileY++; }
    else if (avoidY > curTileY) { targetTileY--; }

    if (boundaryCheck(targetTileX, targetTileY, rows, cols)) {
      if (map.at(targetTileX).at(targetTileY).isPassable()) {
        // Calculate target as center of target tile
        float targetX = static_cast<float>(targetTileX) + 0.5f;
        float targetY = static_cast<float>(targetTileY) + 0.5f;
        
        // Move toward target using float coordinates
        move(c, map, rows, cols, targetX, targetY, 1.0f);
      }
    }
  }
}

//================================================================================
//  Float Movement System
//================================================================================

/**
 *  Move creature toward a target position using float coordinates.
 *  Movement is based on creature's getMovementSpeed() gene-derived value.
 *  Creature moves fractionally each tick, enabling visible speed differences.
 *
 *  @param c        Creature to move
 *  @param map      World grid for collision checking
 *  @param rows     Number of rows in map
 *  @param cols     Number of columns in map
 *  @param targetX  Target X coordinate (float, world space)
 *  @param targetY  Target Y coordinate (float, world space)
 *  @param deltaTime Time elapsed since last update (typically 1.0 per tick)
 *  @return         true if creature reached target (within arrival threshold)
 */
bool Navigator::move(Creature &c,
                           const vector<vector<Tile>> &map,
                           const int &rows,
                           const int &cols,
                           float targetX,
                           float targetY,
                           float deltaTime) {
  // Arrival threshold - consider "arrived" if within this distance
  constexpr float ARRIVAL_THRESHOLD = 0.1f;
  
  // Get current position
  float curX = c.getWorldX();
  float curY = c.getWorldY();
  
  // Calculate direction vector to target
  float dx = targetX - curX;
  float dy = targetY - curY;
  float dist = sqrt(dx * dx + dy * dy);
  
  // Check if already at target
  if (dist <= ARRIVAL_THRESHOLD) {
    return true;  // Arrived
  }
  
  // Calculate movement amount based on speed and deltaTime
  float speed = c.getMovementSpeed();
  float moveAmount = std::min(speed * deltaTime, dist);  // Don't overshoot
  
  // Normalize direction and calculate new position
  float normX = dx / dist;
  float normY = dy / dist;
  float newWorldX = curX + normX * moveAmount;
  float newWorldY = curY + normY * moveAmount;
  
  // Check if new tile position is valid (for collision)
  int newTileX = static_cast<int>(newWorldX);
  int newTileY = static_cast<int>(newWorldY);
  int curTileX = static_cast<int>(curX);
  int curTileY = static_cast<int>(curY);
  
  // If moving to a new tile, check passability
  if (newTileX != curTileX || newTileY != curTileY) {
    // Boundary check
    if (!boundaryCheck(newTileX, newTileY, rows, cols)) {
      return false;  // Can't move - out of bounds
    }
    
    // Passability check
    if (!map.at(newTileX).at(newTileY).isPassable()) {
      return false;  // Can't move - blocked
    }
  }
  
  // Update creature position using float coordinates
  c.setWorldPosition(newWorldX, newWorldY);
  
  // Apply movement cost
  c.movementCost(moveAmount);
  
  // Update direction for animation/prediction
  int xChange = (dx > 0.1f) ? 1 : (dx < -0.1f) ? -1 : 0;
  int yChange = (dy > 0.1f) ? 1 : (dy < -0.1f) ? -1 : 0;
  c.changeDirection(xChange, yChange);
  
  // Return whether we've arrived
  return (dist - moveAmount) <= ARRIVAL_THRESHOLD;
}
