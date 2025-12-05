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
#include "creature.hpp"

class Creature;

// CREATURE-018 fix: Convert macros to type-safe constexpr constants
// These are now properly scoped and type-safe
namespace NavigatorConstants {
  constexpr int NORM_COST = 10;   // Cost for orthogonal movement
  constexpr int DIAG_COST = 14;   // Cost for diagonal movement (approx sqrt(2) * 10)
  constexpr int MAX_NODES = 200;  // Maximum nodes to expand in A* search
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
                                 const int &endY);
    static void movementCost    (Creature &c, const int &x,const int &y);


    //============================================================================
    //  Movement methods
    //============================================================================
    static bool astarSearch     (Creature &c,
                                 const std::vector<std::vector<Tile>> &map,
                                 const int &rows,
                                 const int &cols, 
                                 const int &endX,
                                 const int &endY);
    static void wander          (Creature &c, const std::vector<std::vector<Tile>> &map,
                                 const unsigned rows, const unsigned cols);
    static void moveTowards     (Creature &c, 
                                 const std::vector<std::vector<Tile>> &map,
                                 const int &rows,
                                 const int &cols,
                                 const int &goalX,
                                 const int &goalY);
    static void moveAway        (Creature &c, 
                                 const std::vector<std::vector<Tile>> &map,
                                 const int &rows,
                                 const int &cols,
                                 const int &awayX,
                                 const int &awayY); 
};

#endif
