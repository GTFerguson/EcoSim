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
#include "creature.hpp"

//  Movement Costs
#define NORM_COST   10
#define DIAG_COST   14
#define MAX_NODES   200

class Creature;

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
    static bool  nodeInSet      (std::set<Node, nodeCompare> &set,
                                 const Node &node);
    static void  validateNode   (std::set<Node, nodeCompare> &openSet,
                                 std::set<Node,nodeCompare> closedSet, 
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
                                 const std::set<Node,
                                 nodeCompare> &closedSet,
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
