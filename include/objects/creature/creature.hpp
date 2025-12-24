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

// Sensory system includes (Phase 1)
#include "world/ScentLayer.hpp"

// New genetics system includes (M5 integration)
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/defaults/DefaultGenes.hpp"
#include "genetics/migration/LegacyGenomeAdapter.hpp"

// Creature-Plant interaction includes (Phase 2.4)
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"

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
#include <memory>
#include <optional>   //  optional<Direction> for scent navigation

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
    
    //  DRY refactoring: Scent detection constants
    const static float SCENT_DETECTION_BASE_RANGE;
    const static float SCENT_DETECTION_ACUITY_MULT;
    const static float DEFAULT_OLFACTORY_ACUITY;
    const static float DEFAULT_SCENT_PRODUCTION;
    
    //  DRY refactoring: Seed dispersal constants
    const static float BURR_SEED_VIABILITY;
    const static float GUT_SEED_SCARIFICATION_BONUS;
    const static float GUT_SEED_ACID_DAMAGE;
    const static float DEFAULT_GUT_TRANSIT_HOURS;
    const static float TICKS_PER_HOUR;
    
    //  DRY refactoring: Feeding interaction constants
    const static float FEEDING_MATE_BOOST;
    const static float DAMAGE_HUNGER_COST;
    const static float SEEKING_FOOD_MATE_PENALTY;
    
    //  DRY refactoring: Sense enhancement constants
    const static float COLOR_VISION_RANGE_BONUS;
    const static float SCENT_DETECTION_RANGE_BONUS;
    //  State Variables
    int       _x, _y;
    unsigned  _age;
    int       _id;  // Unique creature ID for logging
    Direction _direction;
    Profile   _profile;

    //  Will Variables 
    float _hunger, _thirst, _fatigue, _mate;

    //  How quickly the creature burns through food
    float     _metabolism = 0.001f;
    unsigned  _speed      = 1;

    //  Genetic information of the creature (legacy system)
    Genome    _genome;

    //============================================================================
    //  New Genetics System (M5 integration - for gradual migration)
    //============================================================================
    // Shared gene registry for all creatures (singleton-like pattern)
    static std::shared_ptr<EcoSim::Genetics::GeneRegistry> s_geneRegistry;
    
    // New genetics objects (optional, for gradual migration)
    std::unique_ptr<EcoSim::Genetics::Genome> _newGenome;
    std::unique_ptr<EcoSim::Genetics::Phenotype> _phenotype;
    
    // Flag to switch between legacy and new genetics systems
    bool _useNewGenetics = false;
    
    //============================================================================
    //  Creature-Plant Interaction Data (Phase 2.4)
    //============================================================================
    // Attached burrs: (plant dispersal strategy, origin X, origin Y, ticks attached)
    std::vector<std::tuple<int, int, int, int>> _attachedBurrs;
    
    // Gut seeds: (origin X*10000+Y encoded, viability, ticks remaining)
    std::vector<std::tuple<int, float, int>> _gutSeeds;
    
    // Shared feeding interaction calculator
    static std::unique_ptr<EcoSim::Genetics::FeedingInteraction> s_feedingInteraction;
    
    // Shared seed dispersal calculator
    static std::unique_ptr<EcoSim::Genetics::SeedDispersal> s_seedDispersal;

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
    
    // Copy/Move constructors and assignment operators (M5: required due to unique_ptr members)
    Creature(const Creature& other);
    Creature(Creature&& other) noexcept;
    Creature& operator=(const Creature& other);
    Creature& operator=(Creature&& other) noexcept;
    ~Creature() = default;
  
    //============================================================================
    //  New Genetics System - Static Methods (M5)
    //============================================================================
    /**
     * @brief Initialize the shared gene registry with default gene definitions.
     *        Should be called once at application startup before creating creatures.
     */
    static void initializeGeneRegistry();
    
    /**
     * @brief Get the shared gene registry (initializes if not already done).
     * @return Reference to the shared GeneRegistry
     */
    static EcoSim::Genetics::GeneRegistry& getGeneRegistry();
  
    //============================================================================
    //  New Genetics System - Instance Methods (M5)
    //============================================================================
    /**
     * @brief Enable or disable the new genetics system for this creature.
     *        When enabled, converts legacy genome to new format if not already done.
     * @param enable True to use new genetics, false for legacy
     */
    void enableNewGenetics(bool enable = true);
    
    /**
     * @brief Check if this creature uses the new genetics system.
     * @return True if new genetics is enabled
     */
    bool usesNewGenetics() const { return _useNewGenetics; }
    
    /**
     * @brief Get the new genome (if enabled).
     * @return Pointer to the new Genome, or nullptr if not using new genetics
     */
    const EcoSim::Genetics::Genome* getNewGenome() const;
    
    /**
     * @brief Get the phenotype (if new genetics enabled).
     * @return Pointer to the Phenotype, or nullptr if not using new genetics
     */
    const EcoSim::Genetics::Phenotype* getPhenotype() const;
    
    /**
     * @brief Update phenotype context with current environment and organism state.
     *        Should be called each tick or when environment changes significantly.
     * @param env Current environment state
     */
    void updatePhenotypeContext(const EcoSim::Genetics::EnvironmentState& env);

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
    int       getId         () const;
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
    // DRY refactoring: Added maxRadius parameter with default to support different search ranges
    template<typename Predicate>
    bool spiralSearch       (const std::vector<std::vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             Predicate predicate,
                             unsigned maxRadius = 0);  // 0 = use getSightRange()
    
    // DRY refactoring: Iterator for visiting all tiles in a spiral pattern
    // Unlike spiralSearch, this doesn't stop early - useful for finding closest match
    template<typename Visitor>
    void forEachTileInRange (unsigned maxRadius, Visitor visitor);
    // GENETICS-MIGRATION: Find and eat genetics-based plants
    bool  findGeneticsPlants(World &world, unsigned &feedingCounter);
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
    //  Sensory System (Phase 1 & 2)
    //============================================================================
    /**
     * @brief Deposit breeding pheromone when in breeding state.
     *        Creates a MATE_SEEKING scent deposit based on creature's olfactory genes.
     * @param layer The world's scent layer to deposit into
     * @param currentTick Current simulation tick for timestamp
     */
    void depositBreedingScent(EcoSim::ScentLayer& layer, unsigned int currentTick);
    
    /**
     * @brief Compute this creature's unique genetic scent signature.
     *        Used for mate recognition and kin detection.
     * @return 8-float array representing the creature's genetic fingerprint
     */
    std::array<float, 8> computeScentSignature() const;
    
    /**
     * @brief Detect the direction to a potential mate using scent trails.
     *        Uses OLFACTORY_ACUITY gene to determine detection range.
     * @param scentLayer The world's scent layer to query
     * @return Direction toward strongest compatible mate scent, or std::nullopt if none found
     */
    std::optional<Direction> detectMateDirection(const EcoSim::ScentLayer& scentLayer) const;
    
    /**
     * @brief Find the coordinates of the strongest mate scent in range.
     *        Uses OLFACTORY_ACUITY gene to determine detection range.
     * @param scentLayer The world's scent layer to query
     * @param outX Output parameter for scent X coordinate
     * @param outY Output parameter for scent Y coordinate
     * @return True if a valid scent was found, false otherwise
     */
    bool findMateScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const;
    
    /**
     * @brief Calculate genetic similarity between two scent signatures.
     * @param sig1 First signature
     * @param sig2 Second signature
     * @return Similarity value [0.0, 1.0] where 1.0 is identical
     */
    static float calculateSignatureSimilarity(
        const std::array<float, 8>& sig1,
        const std::array<float, 8>& sig2);

    //============================================================================
    //  Plant Interaction (Phase 2.4)
    //============================================================================
    /**
     * @brief Attempt to eat a plant using the new genetics-based feeding system.
     * @param plant The plant to eat (will be damaged)
     * @return FeedingResult with nutrition gained, damage taken, and seed info
     */
    EcoSim::Genetics::FeedingResult eatPlant(EcoSim::Genetics::Plant& plant);
    
    /**
     * @brief Check if creature can eat the given plant based on detection and access.
     * @param plant The plant to check
     * @return True if creature can detect and access the plant
     */
    bool canEatPlant(const EcoSim::Genetics::Plant& plant) const;
    
    /**
     * @brief Get the maximum range at which this creature can detect plants.
     * @return Detection range in world units
     */
    float getPlantDetectionRange() const;
    
    /**
     * @brief Attach a burr from a plant to this creature's fur/hide.
     * @param plant The plant providing the burr seeds
     */
    void attachBurr(const EcoSim::Genetics::Plant& plant);
    
    /**
     * @brief Process burr detachment based on grooming and time.
     * @return Vector of dispersal events for any detached burrs
     */
    std::vector<EcoSim::Genetics::DispersalEvent> detachBurrs();
    
    /**
     * @brief Check if creature has any burrs attached.
     * @return True if burrs are present
     */
    bool hasBurrs() const;
    
    /**
     * @brief Get pending dispersal events from attached burrs.
     * @return Vector of potential dispersal locations
     */
    std::vector<EcoSim::Genetics::DispersalEvent> getPendingBurrDispersal() const;
    
    /**
     * @brief Add seeds to gut for digestion and potential dispersal.
     * @param plant Source plant for the seeds
     * @param count Number of seeds consumed
     * @param viability Initial seed viability (0-1)
     */
    void consumeSeeds(const EcoSim::Genetics::Plant& plant, int count, float viability);
    
    /**
     * @brief Process gut seed passage over time.
     * @param ticksElapsed Number of ticks since last processing
     * @return Vector of dispersal events for any seeds passing through
     */
    std::vector<EcoSim::Genetics::DispersalEvent> processGutSeeds(int ticksElapsed);
    
    /**
     * @brief Initialize shared interaction calculators (call once at startup).
     */
    static void initializeInteractionSystems();

private:
    /**
     * @brief Overlay legacy genome values onto a new UniversalGenes genome.
     * @param newGenome The new genome to update with legacy values
     * @param legacy The legacy genome to read values from
     */
    void overlayLegacyGenome(EcoSim::Genetics::Genome& newGenome, const ::Genome& legacy);

public:
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
