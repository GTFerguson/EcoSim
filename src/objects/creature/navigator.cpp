#include "../../../include/objects/creature/navigator.hpp"

//  Adjusts movement cost for diagonal
const float Navigator::DIAG_ADJUST = 1.4f;

using namespace std;

//================================================================================
//  Helper methods
//================================================================================
/**
 *  Checks if a node already exists within a given set.
 *
 *  @param mySet  The set to be searched.
 *  @param node   Node to be searched for.
 *  @return       Whether the node could be found within the set.
 */
bool Navigator::nodeInSet (set<Node, nodeCompare> &mySet, const Node &node) {
  for (set<Node>::iterator iter = mySet.begin(); iter != mySet.end(); ++iter)
    if (iter->compare(node) == 1)
      return true;

  return false;
}

/**
 *  This checks if a node is valid and if so it will add that node to the set.
 *
 *  @param openSet    The set being populated.
 *  @param closedSet  A copy of the of the nodes already visited.
 *  @param curTile    The tile to be checked.
 *  @param gCost      The cost to move from start to this tile.
 *  @param curX       X-position of tile.
 *  @param curY       Y-position of tile.
 *  @param newX       X-position of previous tile.
 *  @param newY       Y-position of previous tile.
 */
void Navigator::validateNode (set<Node, nodeCompare> &openSet,
                              set<Node, nodeCompare> closedSet, 
                              const Tile &curTile,
                              const Node *parent,
                              const int &gCost, 
                              const int &curX,
                              const int &curY, 
                              const int &endX,
                              const int &endY) {
  if (curTile.isPassable() && !(curX == parent->getX() && curY == parent->getY())) {
    Node newNode (curX, curY, parent);
    newNode.setG (gCost);
    newNode.setH (endX, endY);

    if (!nodeInSet(closedSet, newNode) && !nodeInSet(openSet, newNode)) {
      openSet.insert (newNode);
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
 *  @param map        A reference to the world map
 *  @param closedSet  A copy of the of the nodes already visited.
 *  @param curTile    The tile to be checked.
 *  @param gCost      The cost to move from start to this tile.
 *  @param curX       X position of tile.
 *  @param curY       Y position of tile.
 *  @param newX       X position of previous tile.
 *  @param newY       Y position of previous tile.
 */
void Navigator::checkNeighbours (const vector<vector<Tile>> &map,
                                 const Node &curNode,
                                 set<Node, nodeCompare> &openSet, 
                                 const set<Node,
                                 nodeCompare> &closedSet, 
                                 const int &rows,
                                 const int &cols, 
                                 const int &endX,
                                 const int &endY) {
  int curX, curY, gCost;
  Tile curTile;
  //  Diagonals
  for (int xMod = -1; xMod < 2; xMod += 2) {
    for (int yMod = -1; yMod < 2; yMod += 2) {
      curX = curNode.getX() + xMod;
      curY = curNode.getY() + yMod;

      if (boundaryCheck (curX, curY, rows, cols)) {
        gCost   = curNode.getG() + DIAG_COST;
        curTile = map.at(curX).at(curY);
        validateNode  (openSet, closedSet, curTile, &curNode, 
                       gCost, curX, curY, endX, endY);
      }
    }
  }

  //  Up + Down
  for (int yMod = -1; yMod < 2; yMod += 2) {
    curX = curNode.getX();
    curY = curNode.getY() + yMod;

    if (boundaryCheck (curX, curY, rows, cols)) {
      gCost   = curNode.getG() + NORM_COST;
      curTile = map.at(curX).at(curY);
      validateNode  (openSet, closedSet, curTile, &curNode, 
                     gCost, curX, curY, endX, endY);
    }
  }

  //  Left + Right
  for (int xMod = -1; xMod < 2; xMod += 2) {
    curX = curNode.getX() + xMod;
    curY = curNode.getY();

    if (boundaryCheck (curX, curY, rows, cols)) {
      gCost   = curNode.getG() + NORM_COST;
      curTile = map.at(curX).at(curY);
      validateNode  (openSet, closedSet, curTile, &curNode, 
               gCost, curX, curY, endX, endY);
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
 *  @param seek   Whether to seek or flee. 
 *  @return       Whether a path could be found or not.
 */
bool Navigator::astarSearch (Creature &c,
                             const vector<vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             const int &endX,
                             const int &endY) {
  int timeOut = 0;
  //  Sets of tiles already evaluated 
  set<Node, nodeCompare> closedSet;
  //  Set of currently discovered tiles not yet evaluated
  set<Node, nodeCompare> openSet;

  //  Put starting node on set
  Node *sNode = new Node (c.getX(), c.getY());
  sNode->setG (0);
  sNode->setH (endX, endY);
  openSet.insert (*sNode);  
  delete sNode;

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
      closedSet.insert (*iter);
      openSet.erase (iter);
      iter = closedSet.begin();
      checkNeighbours (map, *iter, openSet, closedSet, rows, cols, endX, endY);
      timeOut++;
    }

    if (timeOut > MAX_NODES)
      break;
  }
  return false;
}

/**
 *  This method makes a creature move in a random direction.
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
  random_device rd; 
  mt19937 gen(rd());
  uniform_int_distribution<short> change    (-1, 1);

	int x = c.getX() + (change(gen));
	int y = c.getY() + (change(gen));
  moveTowards (c, map, rows, cols, x, y);
}

// TODO Allow movement as float values to enable creatures to move
//      in the spaces between grid squares.
/**
 *  Simply moves the creature towards a target.
 *
 *  @param goalX  x pos of the target.
 *  @param goalY  y pos of the target.
 */
void Navigator::moveTowards (Creature &c,
                             const vector<vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             const int &goalX,
                             const int &goalY) { 
  int xChange =  0;
  int yChange =  0;
  int newX    = c.getX();
  int newY    = c.getY();

  //  Movement along X-Axis
  if      (goalX > c.getX()) { newX++; xChange =  1; }
  else if (goalX < c.getX()) { newX--; xChange = -1; }
  //  Movement along Y-Axis
  if      (goalY > c.getY()) { newY++; yChange =  1; } 
  else if (goalY < c.getY()) { newY--; yChange = -1; }

  if (boundaryCheck(newX, newY, rows, cols)) {
      if (map.at(newX).at(newY).isPassable()) {
        c.setX(newX); c.setY(newY);
        float dist = c.calculateDistance (newX, newY);
        c.movementCost    (dist);
        c.changeDirection (xChange, yChange);
    }
  }
}

/**
 *  Simply moves the creature away from the target.
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
  int xChange = 0;
  int yChange = 0;
  int newX = c.getX();
  int newY = c.getY();

  //  If at same location
  if (avoidX == newX && avoidY == newY) {
    wander (c, map, rows, cols);

  } else {
    //  Movement along X-Axis
    if      (avoidX < newX) { newX++; xChange =  1; }
    else if (avoidX > newX) { newX--; xChange = -1; }
    //  Movement along Y-Axis
    if      (avoidY < newY) { newY++; yChange =  1; } 
    else if (avoidY > newY) { newY--; yChange = -1; }

    if (boundaryCheck(newX, newY, rows, cols)) {
      if (map.at(newX).at(newY).isPassable()) {
        c.setX(newX); c.setY(newY);
        float dist = c.calculateDistance (newX, newY);
        c.movementCost    (dist);
        c.changeDirection (xChange, yChange);
      }
    }
  }
}
