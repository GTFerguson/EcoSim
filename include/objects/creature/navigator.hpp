#ifndef NAVIGATOR_H
#define NAVIGATOR_H

/**
 * Title    : Ecosim - Node
 * Author   : Gary Ferguson
 * Created  : January 18th, 2020
 * Purpose  : For A* search algorithm.
 */

#include <vector>
#include <random>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "creature.hpp"

// Forward declarations
class Creature;

namespace EcoSim {
    class EnvironmentSystem;
    namespace Genetics {
        class Phenotype;
    }
}

// Type-safe constexpr constants (properly scoped)
namespace NavigatorConstants {
  constexpr int NORM_COST = 10;    // Cost for orthogonal movement
  constexpr int DIAG_COST = 14;    // Cost for diagonal movement (approx sqrt(2) * 10)
  constexpr int MAX_NODES = 1000;  // Maximum nodes to expand in A* search (increased for large 500x500 maps)
}

// For backward compatibility with existing code using the old macro names
constexpr int NORM_COST = NavigatorConstants::NORM_COST;
constexpr int DIAG_COST = NavigatorConstants::DIAG_COST;
constexpr int MAX_NODES = NavigatorConstants::MAX_NODES;

// Hash function for coordinate pairs (for O(1) lookup in A* search)
struct CoordHash {
    std::size_t operator()(const std::pair<int, int>& coord) const {
        // Combine hashes using a common technique
        return std::hash<int>()(coord.first) ^ (std::hash<int>()(coord.second) << 16);
    }
};

// Type alias for coordinate set with O(1) lookup
using CoordSet = std::unordered_set<std::pair<int, int>, CoordHash>;

/**
 * @brief Per-search cache for tile environmental costs
 *
 * Caches the computed environmental cost for each visited tile during
 * a single A* search. Since the environment doesn't change during a search,
 * this reduces temperature queries from up to 1,600 per search to ~200
 * (one per unique visited tile).
 */
struct PathfindingCostCache {
    std::unordered_map<uint64_t, float> tileCosts;
    
    /// Convert x,y to a single key for fast lookup
    static uint64_t makeKey(int x, int y) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) |
               static_cast<uint64_t>(static_cast<uint32_t>(y));
    }
    
    /// Clear the cache (call at start of each new search)
    void clear() { tileCosts.clear(); }
    
    /// Reserve capacity for expected number of tiles
    void reserve(size_t expectedTiles) { tileCosts.reserve(expectedTiles); }
    
    /**
     * @brief Get cached cost for a tile
     * @param x Tile X coordinate
     * @param y Tile Y coordinate
     * @return Cached cost, or -1.0f if not cached
     */
    float getCachedCost(int x, int y) const {
        auto key = makeKey(x, y);
        auto it = tileCosts.find(key);
        return (it != tileCosts.end()) ? it->second : -1.0f;
    }
    
    /**
     * @brief Store a computed cost in the cache
     * @param x Tile X coordinate
     * @param y Tile Y coordinate
     * @param cost The computed environmental cost
     */
    void setCachedCost(int x, int y, float cost) {
        tileCosts[makeKey(x, y)] = cost;
    }
};

/**
 * @brief Context for environmental pathfinding calculations
 *
 * Pre-computed creature data and environment access for efficient
 * per-tile cost calculation during A* search. Using a context struct
 * avoids changing existing function signatures significantly.
 */
struct PathfindingContext {
    /// Creature's effective minimum tolerable temperature (with adaptations)
    float effectiveTolMin = -30.0f;
    
    /// Creature's effective maximum tolerable temperature (with adaptations)
    float effectiveTolMax = 60.0f;
    
    /// Creature's environmental sensitivity gene value [0.0, 2.0]
    float environmentalSensitivity = 1.0f;
    
    /// Environment system for temperature queries (non-owning, may be null)
    const EcoSim::EnvironmentSystem* envSystem = nullptr;
    
    /// Per-search cost cache (mutable since caching doesn't change logical state)
    mutable PathfindingCostCache costCache;
    
    /// Danger weight factor for cost calculation
    static constexpr float DANGER_WEIGHT_FACTOR = 10.0f;
    
    /**
     * @brief Calculate total movement cost including environmental factors
     * @param baseCost Base movement cost (NORM_COST or DIAG_COST)
     * @param x Target tile X coordinate
     * @param y Target tile Y coordinate
     * @return Total cost with environmental danger penalty applied
     *
     * Uses per-search caching to avoid redundant temperature queries.
     */
    float calculateTileCost(float baseCost, int x, int y) const;
    
    /**
     * @brief Clear the cost cache (call at start of each new search)
     */
    void clearCache() const { costCache.clear(); }
    
    /**
     * @brief Create context from creature phenotype
     * @param phenotype The creature's expressed traits
     * @param envSystem Environment system for temperature queries (may be null)
     * @return Populated PathfindingContext ready for use
     */
    static PathfindingContext fromCreature(
        const EcoSim::Genetics::Phenotype& phenotype,
        const EcoSim::EnvironmentSystem* envSystem);
        
    /**
     * @brief Create default context (no environmental awareness)
     * @return Context with environmental costs disabled
     */
    static PathfindingContext noEnvironment() {
        return PathfindingContext{};
    }
};

//  Node for A* Search
class Node {
  private:
    //  Node coordinates
    int _x, _y, _prevX, _prevY;
    /*  g - cost to get to tile
        h - Estimated cost from start to finish */
    int _g, _h;
    const Node *_parent;

  public:
    //  Start Node Constructor
    Node (const int &x, const int &y) {
      _parent = nullptr;
      _x = x; _y = y;
    };

    //  Constructor w/ Parent
    Node (const int &x, const int &y, const Node *parent) {
      _parent = parent;
      _x = x; _y = y;
    };

    //  Getters
    int getX () const { return _x; };
    int getY () const { return _y; };
    int getG () const { return _g; };
    int getH () const { return _h; };
    const Node *getParent () const {  return _parent; };
    // f - equal to the sum of g and h
    int getF () { return _g + _h; };

    //  Setters
    void setX (const int &x) {  _x = x; };
    void setY (const int &y) {  _y = y; };
    void setG (const int &g) {  _g = g; };

    void setH (const int &endX, const int &endY) {
      unsigned xDist = abs (_x - endX);
      unsigned yDist = abs (_y - endY);         
      //  Manhattan Distance (four directions)
      // _h = (xDist + yDist) * NORM_COST;
      //  Diagonal Distance (eight directions)
      // _h = std::max(xDist, yDist) * (NORM_COST);
      _h = calculateDistance (xDist, yDist);
            
    };

    /**
     *  Used for checking if node exists within a set. 
     *
     *  @param other This is usually the newer node.
     *  @return 0 - no match & worse h, 1 - match.
     */
    bool compare (const Node &other) const {
      //  No Match
      if (_x != other.getX()) return 0;
      if (_y != other.getY()) return 0;
      if (_parent != other.getParent()) return 0;

      //  if existing h is more than new skip
      if (_h > other.getH()) return 0;

      return 1;
    };

    unsigned calculateDistance (const unsigned &xDist, const unsigned &yDist) {
      unsigned nondiag  = (xDist > yDist) ? xDist - yDist : yDist - xDist;
      unsigned diag     = std::max(xDist, yDist) - nondiag;

      return nondiag * NORM_COST + diag * DIAG_COST;
    }

};

class Navigator {
  private:
    //  Adjusts movement cost for diagonal
    static const float DIAG_ADJUST;

    struct nodeCompare {
        bool operator () (const Node& first, const Node& second) const {
            return (first.getH() < second.getH());
        }
    };

  public:
    //============================================================================
    //  Helper methods
    //============================================================================
    static bool  nodeInCoordSet (const CoordSet &coordSet, int x, int y);
    static void  validateNode   (std::set<Node, nodeCompare> &openSet,
                                 CoordSet &openCoords,
                                 const CoordSet &closedCoords,
                                 const Tile &curTile, const Node *parent,
                                 const int &gCost,
                                 const int &curX,
                                 const int &curY,
                                 const int &endX,
                                 const int &endY);
    static bool boundaryCheck   (const int &x,
                                 const int &y,
                                 const int &rows,
                                 const int &cols);
    static void checkNeighbours (const std::vector<std::vector<Tile>> &map,
                                 const Node &curNode,
                                 std::set<Node, nodeCompare> &openSet,
                                 CoordSet &openCoords,
                                 const CoordSet &closedCoords,
                                 const int &rows,
                                 const int &cols,
                                 const int &endX,
                                 const int &endY,
                                 const PathfindingContext* ctx = nullptr);
    static void movementCost    (Creature &c, const int &x,const int &y);


    //============================================================================
    //  Movement methods
    //============================================================================
    static bool astarSearch     (Creature &c,
                                 const std::vector<std::vector<Tile>> &map,
                                 const int &rows,
                                 const int &cols,
                                 const int &endX,
                                 const int &endY,
                                 const PathfindingContext* ctx = nullptr);
    static bool wander          (Creature &c, const std::vector<std::vector<Tile>> &map,
                                 const unsigned rows, const unsigned cols);
    static bool moveTowards     (Creature &c,
                                 const std::vector<std::vector<Tile>> &map,
                                 const int &rows,
                                 const int &cols,
                                 const int &goalX,
                                 const int &goalY);
    static bool moveAway        (Creature &c,
                                 const std::vector<std::vector<Tile>> &map,
                                 const int &rows,
                                 const int &cols,
                                 const int &awayX,
                                 const int &awayY);
    
    //============================================================================
    //  Float Movement System
    //============================================================================
    /**
     * @brief Move creature toward a target position using float coordinates.
     *        Movement is based on creature's getMovementSpeed() and deltaTime.
     *        Creatures move fractionally each tick, enabling visible speed differences.
     *
     * @param c Creature to move
     * @param map World grid for collision checking
     * @param rows Number of rows in map
     * @param cols Number of columns in map
     * @param targetX Target X coordinate (float, world space)
     * @param targetY Target Y coordinate (float, world space)
     * @param deltaTime Time elapsed since last update (in ticks, typically 1.0)
     * @return true if creature reached target (within small epsilon)
     */
    static bool move            (Creature &c,
                                 const std::vector<std::vector<Tile>> &map,
                                 const int &rows,
                                 const int &cols,
                                 float targetX,
                                 float targetY,
                                 float deltaTime = 1.0f);
};

#endif
