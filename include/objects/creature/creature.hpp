#ifndef CREATURE_H
#define CREATURE_H

// CREATURE-017 fix: Removed SIZE() macro that polluted global namespace
// If needed elsewhere, use: template<typename T, size_t N> constexpr size_t arraySize(T(&)[N]) { return N; }

/**
 * Title    : Ecosim - Creature
 * Author   : Gary Ferguson
 * Created  : May 18, 2019
 * Purpose  : This is a class for the creation of autonomous artificial life
 *            agents, intended to be used for creating immersive wildfile in
 *            a virtual environment. They each can have their own genome, which
 *            through evolutionary algorithms will adapt to their environment
 *            and show emergent behaviour.
 */

#include "../../statistics/statistics.hpp"
#include "../../world/world.hpp"
#include "../../world/tile.hpp"
#include "../gameObject.hpp"
#include "../food.hpp"
#include "navigator.hpp"
#include "genome.hpp"

#include <stdlib.h>   //  abs
#include <algorithm>  //  max
#include <iostream>   //  cerr
#include <cmath>      //  sqrt
#include <string>
#include <vector>
#include <array>
#include <list>
#include <sstream>
#include <functional>

class World;

 //  Allows simple 8-direction system
enum class Direction { N, E, S, W, NE, NW, SE, SW, none };
//  Makes code for the profiles cleaner
enum class Profile { thirsty, hungry, breed, sleep, migrate };

class Creature: public GameObject {
  private:
    // Adjustment to cost for diagonal movements
    const static float DIAG_ADJUST;
    const static float RESOURCE_LIMIT; 
    const static float INIT_FATIGUE;
    const static float BREED_COST;
    const static float IDEAL_SIMILARITY;
    const static float PENALTY_EXPONENT;
    const static float PREY_CALORIES;
    //  Values for death thresholds
    const static float STARVATION_POINT;
    const static float DEHYDRATION_POINT;
    const static float DISCOMFORT_POINT;
    //  What fraction of resources is shared
    const static unsigned RESOURCE_SHARED;
    //  State Variables
    int       _x, _y; 
    unsigned  _age;
    Direction _direction;
    Profile   _profile;

    //  Will Variables 
    float _hunger, _thirst, _fatigue, _mate;

    //  How quickly the creature burns through food
    float     _metabolism = 0.001f; 
    unsigned  _speed      = 1;

    //  Genetic information of the creature
    Genome    _genome;

  public:
    //============================================================================
    //  Constructors
    //============================================================================
    Creature (const int &x,
              const int &y,
              const float &hunger, 
              const float &thirst,
              const Genome &genes);
    Creature (const int &x, const int &y, const Genome &genes);
    Creature (const std::string &name,
              const std::string &desc,
              const bool &passable,
              const char &character,
              const unsigned &colour,
              const int &x,
              const int &y,
              const unsigned &age,
              const std::string &profile,
              const std::string &direction,
              const float &hunger,
              const float &thirst,
              const float &fatigue,
              const float &mate,
              const float &metabolism,
              const unsigned &speed,
              const Genome &genes);
  
    //============================================================================
    //  Setters
    //============================================================================
    void setAge     (unsigned age);
    void setHunger  (float hunger);
    void setThirst  (float thirst);
    void setFatigue (float fatigue);
    void setMate    (float mate);
    void setXY      (int x, int y);
    void setX       (int x);
    void setY       (int y);

    //============================================================================
    //  Getters
    //============================================================================
    float     getTMate      () const;
    unsigned  getAge        () const;
    float     getHunger     () const;
    float     getThirst     () const;
    float     getFatigue    () const;
    float     getMate       () const;
    float     getMetabolism () const;
    unsigned  getSpeed      () const;
    int       getX          () const;
    int       getY          () const;
    Direction getDirection  () const;
    Profile   getProfile    () const;
    //  Genome Getters (CREATURE-010 fix: return by const reference)
    const Genome& getGenome () const;
    unsigned  getLifespan   () const;
    unsigned  getSightRange () const;
    float     getTHunger    () const;
    float     getTThirst    () const;
    float     getTFatigue   () const;
    float     getComfInc    () const;
    float     getComfDec    () const;
    Diet      getDiet       () const;
    bool      ifFlocks      () const;
    unsigned  getFlee       () const;
    unsigned  getPursue     () const;

    //============================================================================
    //  Behaviours
    //============================================================================
    void migrateProfile     (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index);
    void hungryProfile      (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index, 
                             GeneralStats &gs);
    void thirstyProfile     (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index);
    void breedProfile       (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index, 
                             GeneralStats &gs);
    bool  flock             (World &world, std::vector<Creature> &creatures);
    void  update            ();
    short deathCheck        () const;
    float shareResource     (const int &amount, float &resource);
    float shareFood         (const int &amount);
    float shareWater        (const int &amount);
    void  decideBehaviour   ();
    bool  foodCheck         (const std::vector<std::vector<Tile>> &map,
                             const unsigned &rows,
                             const unsigned &cols,
                             const int &x,
                             const int &y);
    bool  waterCheck        (const std::vector<std::vector<Tile>> &map,
                             const unsigned &rows,
                             const unsigned &cols,
                             const int &x,
                             const int &y);
    // CREATURE-013 fix: Generic spiral search helper to eliminate duplication
    template<typename Predicate>
    bool spiralSearch       (const std::vector<std::vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             Predicate predicate);
    bool  findFood          (std::vector<std::vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             unsigned int &foodCounter);
    bool  findWater         (const std::vector<std::vector<Tile>> &map,
                             const int &rows, const int &cols);
    bool  findMate          (std::vector<std::vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             std::vector<Creature> &c,
                             const unsigned &index,
                             unsigned &birthCounter);
    bool  findPrey          (std::vector<std::vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             std::vector<Creature> &c,
                             unsigned &preyAte);
    void  changeDirection   (const int &xChange, const int &yChange);
    float calculateDistance (const int &goalX, const int &goalY) const;
    void  movementCost      (const float &distance);

    //============================================================================
    //  Breeding
    //============================================================================
    float     checkFitness  (const Creature &c2) const;
    Creature  breedCreature (Creature &mate);

    //============================================================================
    //  Variable Generators
    //============================================================================
    char        generateChar ();
    std::string generateName ();
    //============================================================================
    //  To String 
    //============================================================================
    Profile     stringToProfile   (const std::string &str);
    Direction   stringToDirection (const std::string &str);
    std::string profileToString   () const;
    std::string directionToString () const;
    std::string toString          () const;
};

#endif
