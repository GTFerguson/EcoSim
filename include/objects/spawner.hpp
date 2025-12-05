#ifndef SPAWNER_H
#define SPAWNER_H

/**
 *  Title   : Ecosim - Spawner
 *  Author  : Gary Ferguson
 *  Date    : Oct 24th, 2019
 *  Purpose : Used to create a GameObject that will spawn
 *            other GameObjects as specified.
 */

#include <string>
#include <utility>
#include "gameObject.hpp"
#include "food.hpp"

/**
 * @class Spawner
 * @brief Represents objects that periodically spawn Food items.
 *
 * Spawner objects (like trees or plants) extend GameObject with timer-based
 * spawning mechanics. They generate Food items at a configurable rate within
 * a specified radius range. The spawner tracks time internally and determines
 * when new food should appear in the simulation.
 */
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
    Spawner (const std::string &name,   const std::string &desc,
             bool passable,             char character,
             unsigned int colour,       unsigned int rate,
             unsigned int minRadius,    unsigned int maxRadius,
             const Food &foodObj);

    Spawner (const std::string &name, const std::string &desc, bool passable,
             char character,            unsigned int colour,
             unsigned int rate,         unsigned int timer,
             unsigned int minRadius,    unsigned int maxRadius,
             const Food &foodObj);
    //============================================================================
    //  Getters
    //============================================================================
    /**
     * @brief Gets the spawn rate (ticks between spawns).
     * @return The rate value.
     */
    unsigned int    getRate      () const;
    
    /**
     * @brief Gets the current timer value.
     * @return The timer value.
     */
    unsigned int    getTimer     () const;
    
    /**
     * @brief Gets the minimum spawn radius.
     * @return The minimum radius value.
     */
    unsigned int    getMinRadius () const;
    
    /**
     * @brief Gets the maximum spawn radius.
     * @return The maximum radius value.
     */
    unsigned int    getMaxRadius () const;
    
    /**
     * @brief Gets the Food object that this spawner creates.
     * @return Reference to the Food template object.
     */
    const Food&     getObject    () const;
    
    //  Setters
    /**
     * @brief Sets the timer value.
     * @param time The new timer value.
     */
    void setTimer (unsigned int time);

    //============================================================================
    //  Spawn Handling
    //============================================================================
    /**
     * @brief Checks if spawner can spawn (query only, no side effects).
     * @return true if timer > rate, false otherwise.
     */
    bool canSpawn           () const;
    
    /**
     * @brief Increments timer and checks if spawn should occur.
     *
     * Resets timer to 0 when spawn condition is met.
     * @return true if object should be spawned, false otherwise.
     */
    bool tickAndCheckSpawn  ();
    
    /**
     * @brief Generates a single coordinate within the spawn radius.
     * @param init The current position of the spawner.
     * @param limit The highest value that could be generated.
     * @return The generated coordinate.
     */
    int  genCoordinate      (int init, int limit);
    
    /**
     * @brief Generates spawn coordinates within the radius range.
     * @param x The current x position of the spawner.
     * @param y The current y position of the spawner.
     * @param rows The number of rows on the map.
     * @param cols The number of columns on the map.
     * @return A pair of coordinates (x, y) for the spawned object.
     */
    std::pair<int, int> genCoordinates (int x, int y, unsigned int rows, unsigned int cols);

    //============================================================================
    //  To String
    //============================================================================
    /**
     * @brief Converts the spawner to a string representation.
     * @return A comma-separated string with all spawner properties.
     */
    std::string toString () const override;
};

#endif
