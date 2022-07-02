#ifndef SPAWNER_H
#define SPAWNER_H

/**
 *  Title   : Ecosim - Spawner
 *  Author  : Gary Ferguson
 *  Date    : Oct 24th, 2019
 *  Purpose : Used to create a GameObject that will spawn 
 *            other GameObjects as specified.
 */

#include <string.h>
#include <vector>
#include "gameObject.hpp"
#include "food.hpp"

class Spawner: public GameObject {
  private:
    unsigned int _timer;
    unsigned int _rate;
    unsigned int _minRadius;
    unsigned int _maxRadius;

    //  This is the object spawned 
    Food _foodObj;
  public:
    //============================================================================
    //  Constructor
    //============================================================================
    Spawner (const std::string &name,        const std::string &desc,
             const bool &passable,           const char &character,
             const unsigned int &colour,     const unsigned int &rate,
             const unsigned int &minRadius,  const unsigned int &maxRadius,
             const Food &foodObj);

    Spawner (const std::string &name, const std::string &desc, const bool &passable,
             const char &character,         const unsigned int &colour,
             const unsigned int &rate,      const unsigned int &timer,
             const unsigned int &minRadius, const unsigned int &maxRadius,
             const Food &foodObj);
    //============================================================================
    //  Getters
    //============================================================================
    unsigned int getRate      () const;
    unsigned int getTimer     () const;
    unsigned int getMinRadius () const;
    unsigned int getMaxRadius () const;
    Food         getObject    () const;
    //  Setters
    void setTimer (const unsigned int &time);

    //============================================================================
    //  Spawn Handling
    //============================================================================
    bool    canSpawn       ();
    int     genCoordinate  (const int &init, const int &limit);
    std::vector<int>  genCoordinates (const int &x,             const int &y,
                                      const unsigned int &rows, const unsigned int &cols);

    //============================================================================
    //  To String  
    //============================================================================
    std::string toString () const;
};

#endif
