#ifndef CREATURE_H
#define CREATURE_H

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
#include "navigator.hpp"

// Sensory system includes (Phase 1)
#include "world/ScentLayer.hpp"

// New genetics system includes (M5 integration)
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/defaults/UniversalGenes.hpp"  // For DietType
#include "genetics/interfaces/ILifecycle.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"

// Creature-Plant interaction includes (Phase 2.4)
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"

// Classification system (Phase 2 - Unified Identity)
#include "genetics/classification/ArchetypeIdentity.hpp"

// Behavior system (Phase 3 - Creature God Class decomposition)
#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/interactions/CombatInteraction.hpp"

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

// DietType is now defined in genetics/defaults/UniversalGenes.hpp
// Import via: using EcoSim::Genetics::DietType;
using EcoSim::Genetics::DietType;

 //  Allows simple 8-direction system
enum class Direction { N, E, S, W, NE, NW, SE, SW, none };

//  Motivation enum - what the creature currently wants/needs most
enum class Motivation { Hungry, Thirsty, Amorous, Tired, Content };

//  Action enum - what the creature is currently doing
enum class Action {
    Idle,       // No specific action
    Wandering,  // Moving randomly
    Searching,  // Looking for something (food/water/mate)
    Navigating, // Moving toward a target
    Eating,     // Consuming food
    Grazing,    // Eating plants (herbivore)
    Hunting,    // Pursuing prey (predator)
    Chasing,    // Actively chasing target
    Attacking,  // In combat
    Fleeing,    // Running away from threat
    Drinking,   // Consuming water
    Courting,   // Looking for mate
    Mating,     // Breeding
    Resting     // Sleeping/recovering
};

//  Wound state enum for health system
enum class WoundState { Healthy, Injured, Wounded, Critical, Dead };

//  Deprecated - use Motivation instead
enum class Profile { thirsty, hungry, breed, sleep, migrate };

class Creature: public GameObject,
                public EcoSim::Genetics::ILifecycle,
                public EcoSim::Genetics::IGeneticOrganism {
  public:
    //============================================================================
    //  Public Constants (for balance analysis and external tools)
    //============================================================================
    const static float RESOURCE_LIMIT;
    const static float BREED_COST;
    
  private:
    // Adjustment to cost for diagonal movements
    const static float DIAG_ADJUST;
    const static float INIT_FATIGUE;
    const static float IDEAL_SIMILARITY;
    const static float PENALTY_EXPONENT;
    const static float PREY_CALORIES;
    //  Values for death thresholds
    const static float STARVATION_POINT;
    const static float DEHYDRATION_POINT;
    const static float DISCOMFORT_POINT;
    //  What fraction of resources is shared
    const static unsigned RESOURCE_SHARED;
    
    //  Scent detection constants
    const static float SCENT_DETECTION_BASE_RANGE;
    const static float SCENT_DETECTION_ACUITY_MULT;
    const static float DEFAULT_OLFACTORY_ACUITY;
    const static float DEFAULT_SCENT_PRODUCTION;
    
    //  Seed dispersal constants
    const static float BURR_SEED_VIABILITY;
    const static float GUT_SEED_SCARIFICATION_BONUS;
    const static float GUT_SEED_ACID_DAMAGE;
    const static float DEFAULT_GUT_TRANSIT_HOURS;
    const static float TICKS_PER_HOUR;
    
    //  Feeding interaction constants
    const static float FEEDING_MATE_BOOST;
    const static float DAMAGE_HUNGER_COST;
    const static float SEEKING_FOOD_MATE_PENALTY;
    
    //  Sense enhancement constants
    const static float COLOR_VISION_RANGE_BONUS;
    const static float SCENT_DETECTION_RANGE_BONUS;
    
    //  Movement system constants
    const static float BASE_MOVEMENT_SPEED;   // Base speed multiplier
    const static float MIN_MOVEMENT_SPEED;    // Minimum speed floor
    const static float DEFAULT_LEG_LENGTH;    // Default leg length for creatures without gene
    const static float DEFAULT_BODY_MASS;     // Default body mass for creatures without gene
    
    //  State Variables
    //  Float-based world coordinates
    float     _worldX, _worldY;   // Precise position in world coordinates
    unsigned  _age;
    int       _id;  // Unique creature ID for logging
    Direction _direction;
    Profile   _profile;          // Deprecated - use _motivation instead
    Motivation _motivation = Motivation::Content;  // Current motivation/drive
    Action    _action = Action::Idle;              // Current action being performed
    
    //  Health & Combat System
    float     _health = 10.0f;     // Current health (safe default, actual set by constructors)
    bool      _inCombat = false;   // Currently in combat
    bool      _isFleeing = false;  // Currently fleeing from threat
    int       _targetId = -1;      // ID of combat/pursuit target (-1 = none)
    int       _combatCooldown = 0; // Ticks until can attack again

    //  Will Variables
    float _hunger, _thirst, _fatigue, _mate;

    //  How quickly the creature burns through food
    float     _metabolism = 0.001f;
    unsigned  _speed      = 1;

    //============================================================================
    //  Genetics System
    //============================================================================
    // Shared gene registry for all creatures (singleton-like pattern)
    static std::shared_ptr<EcoSim::Genetics::GeneRegistry> s_geneRegistry;
    
    // Primary genetics objects
    std::unique_ptr<EcoSim::Genetics::Genome> _genome;
    std::unique_ptr<EcoSim::Genetics::Phenotype> _phenotype;
    
    //============================================================================
    //  Archetype Identity (Phase 2 - Unified Identity System)
    //============================================================================
    /**
     * @brief Shared archetype flyweight (non-owning)
     *
     * Points to one of the ArchetypeIdentity singleton objects.
     * Lifetime: The pointed-to object lives for program duration.
     * Never null after construction completes (unless legacy creature).
     *
     * @note Do NOT delete this pointer - it's a shared flyweight.
     */
    const EcoSim::Genetics::ArchetypeIdentity* _archetype = nullptr;
    
    //============================================================================
    //  Creature-Plant Interaction Data
    //============================================================================
    // Attached burrs: (plant dispersal strategy, origin X, origin Y, ticks attached)
    std::vector<std::tuple<int, int, int, int>> _attachedBurrs;
    
    // Gut seeds: (origin X*10000+Y encoded, viability, ticks remaining)
    std::vector<std::tuple<int, float, int>> _gutSeeds;
    
    // Shared feeding interaction calculator
    static std::unique_ptr<EcoSim::Genetics::FeedingInteraction> s_feedingInteraction;
    
    // Shared seed dispersal calculator
    static std::unique_ptr<EcoSim::Genetics::SeedDispersal> s_seedDispersal;

    //============================================================================
    //  Behavior System (Phase 3 - Creature God Class decomposition)
    //============================================================================
    std::unique_ptr<EcoSim::Genetics::BehaviorController> _behaviorController;
    
    // Shared services for behavior system
    static std::unique_ptr<EcoSim::Genetics::PerceptionSystem> s_perceptionSystem;
    static std::unique_ptr<EcoSim::Genetics::CombatInteraction> s_combatInteraction;

  public:
    //============================================================================
    //  Constructors
    //============================================================================
    /**
     * @brief Construct a creature with the new genetics system.
     * @param x X position in world
     * @param y Y position in world
     * @param genome New genetics genome (ownership transferred)
     */
    Creature(int x, int y, std::unique_ptr<EcoSim::Genetics::Genome> genome);
    
    /**
     * @brief Construct a creature with hunger/thirst values and genome.
     * @param x X position in world
     * @param y Y position in world
     * @param hunger Initial hunger value
     * @param thirst Initial thirst value
     * @param genome New genetics genome (ownership transferred)
     */
    Creature(int x, int y, float hunger, float thirst,
             std::unique_ptr<EcoSim::Genetics::Genome> genome);
    
   
   // Copy/Move constructors and assignment operators (M5: required due to unique_ptr members)
    Creature(const Creature& other);
    Creature(Creature&& other) noexcept;
    Creature& operator=(const Creature& other);
    Creature& operator=(Creature&& other) noexcept;
    ~Creature() noexcept override;  // Decrements archetype population count
  
    //============================================================================
    //  New Genetics System - Static Methods
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
    //  Genetics System - Instance Methods
    //============================================================================
    /**
     * @brief Get the genome (IGeneticOrganism interface).
     * @return Reference to the Genome
     */
    const EcoSim::Genetics::Genome& getGenome() const override;
    
    /**
     * @brief Get the genome (mutable, IGeneticOrganism interface).
     * @return Reference to the Genome
     */
    EcoSim::Genetics::Genome& getGenomeMutable() override;
    
    /**
     * @brief Get the phenotype (IGeneticOrganism interface).
     * @return Reference to the Phenotype
     */
    const EcoSim::Genetics::Phenotype& getPhenotype() const override;
    
    /**
     * @brief Update phenotype context with current environment and organism state.
     *        Should be called each tick or when environment changes significantly.
     * @param env Current environment state
     */
    void updatePhenotypeContext(const EcoSim::Genetics::EnvironmentState& env);
    
    /**
     * @brief Recalculate phenotype from genome (IGeneticOrganism interface).
     *        Simple wrapper that calls updatePhenotypeContext with default environment.
     */
    void updatePhenotype() override;
    
    /**
     * @brief Get the expressed value of a gene from the phenotype.
     * @param geneId The gene ID to look up
     * @return The expressed trait value, or 0.0 if not found or not using new genetics
     */
    float getExpressedValue(const std::string& geneId) const;

    //============================================================================
    //  Setters
    //============================================================================
    void setAge        (unsigned age);
    void setHunger     (float hunger);
    void setThirst     (float thirst);
    void setFatigue    (float fatigue);
    void setMate       (float mate);
    void setXY         (int x, int y);
    void setX          (int x);
    void setY          (int y);
    void setMotivation (Motivation m);
    void setAction     (Action a);
    
    //============================================================================
    //  Float Position Setters
    //============================================================================
    /**
     * @brief Set precise world coordinates.
     * @param x Float x-coordinate in world space
     * @param y Float y-coordinate in world space
     */
    void setWorldPosition(float x, float y);
    
    /**
     * @brief Set precise world X coordinate.
     * @param x Float x-coordinate in world space
     */
    void setWorldX(float x);
    
    /**
     * @brief Set precise world Y coordinate.
     * @param y Float y-coordinate in world space
     */
    void setWorldY(float y);

    //============================================================================
    //  Getters
    //============================================================================
    float     getTMate      () const;
    int       getId         () const override;
    
    //============================================================================
    //  ILifecycle Interface Implementation
    //============================================================================
    unsigned int getAge() const override;
    unsigned int getMaxLifespan() const override;
    float getAgeNormalized() const override;
    bool isAlive() const override;
    void age(unsigned int ticks = 1) override;
    float     getHunger     () const;
    float     getThirst     () const;
    float     getFatigue    () const;
    float     getMate       () const;
    float     getMetabolism () const;
    unsigned  getSpeed      () const;
    int       getX          () const override;
    int       getY          () const override;
    Direction   getDirection  () const;
    Profile     getProfile    () const;  // Deprecated - use getMotivation()
    Motivation  getMotivation () const;
    Action      getAction     () const;
    
    //============================================================================
    //  Float Position Getters
    //============================================================================
    /**
     * @brief Get precise world X coordinate.
     * @return Float x-coordinate in world space
     */
    float getWorldX() const;
    
    /**
     * @brief Get precise world Y coordinate.
     * @return Float y-coordinate in world space
     */
    float getWorldY() const;
    
    /**
     * @brief Get tile X coordinate (integer, for rendering/collision).
     *        Derives from world coordinates via truncation.
     * @return Integer tile x-coordinate
     */
    int tileX() const;
    
    /**
     * @brief Get tile Y coordinate (integer, for rendering/collision).
     *        Derives from world coordinates via truncation.
     * @return Integer tile y-coordinate
     */
    int tileY() const;
    
    /**
     * @brief Calculate movement speed based on genes.
     *        Formula: baseSpeed = (MOVEMENT_SPEED * LEG_LENGTH) / sqrt(MASS)
     * @return Calculated movement speed (tiles per tick)
     */
    float getMovementSpeed() const;
    
    /**
     * @brief Get maximum health based on genetics.
     *        Uses MASS gene as proxy (larger creatures have more health).
     * @return Maximum health value
     */
    float getMaxHealth() const;
    
    /**
     * @brief Get current health.
     * @return Current health value
     */
    float getHealth() const;
    
    /**
     * @brief Get health as percentage (0.0 to 1.0).
     * @return Health percentage
     */
    float getHealthPercent() const;
    
    /**
     * @brief Get current wound state.
     * @return WoundState enum value
     */
    WoundState getWoundState() const;
    
    /**
     * @brief Get wound severity (0.0 = healthy, 1.0 = critical).
     * @return Wound severity value
     */
    float getWoundSeverity() const;
    
    /**
     * @brief Get healing rate per tick.
     * @return Healing rate
     */
    float getHealingRate() const;
    
    /**
     * @brief Apply damage to creature's health.
     * @param amount Damage to apply (positive value)
     *
     * - Reduces _health directly
     * - Floors health at 0 (never negative)
     * - Zero/negative amounts are no-ops
     */
    void takeDamage(float amount);
    
    /**
     * @brief Heal creature's health.
     * @param amount Health to restore (positive value)
     *
     * - Caps at getMaxHealth()
     * - Zero/negative amounts are no-ops
     */
    void heal(float amount);
    
    /**
     * @brief Set combat state.
     * @param combat True if entering combat
     */
    void setInCombat(bool combat);
    
    /**
     * @brief Set combat target ID.
     * @param targetId Target creature's ID, or -1 for none
     */
    void setTargetId(int targetId);
    
    /**
     * @brief Set combat cooldown ticks.
     * @param cooldown Ticks until next attack
     */
    void setCombatCooldown(int cooldown);
    
    /**
     * @brief Set fleeing state.
     * @param fleeing True if fleeing
     */
    void setFleeing(bool fleeing);
    
    /**
     * @brief Check if creature is currently in combat.
     * @return True if in combat
     */
    bool isInCombat() const;
    
    /**
     * @brief Check if creature is currently fleeing.
     * @return True if fleeing
     */
    bool isFleeing() const;
    
    /**
     * @brief Get ID of current combat target.
     * @return Target creature ID, or -1 if none
     */
    int getTargetId() const;
    
    /**
     * @brief Get remaining combat cooldown ticks.
     * @return Cooldown ticks remaining
     */
    int getCombatCooldown() const;
    
    /**
     * @brief Get creature archetype label (e.g., "Herbivore", "Predator").
     * @return Archetype label string
     */
    std::string getArchetypeLabel() const;
    
    /**
     * @brief Get scientific name (species classification).
     * @return Scientific name string
     */
    std::string getScientificName() const;
    
    //  Genetics-Derived Getters (derived from new genetics system)
    unsigned  getLifespan   () const;
    unsigned  getSightRange () const;
    float     getTHunger    () const;
    float     getTThirst    () const;
    float     getTFatigue   () const;
    float     getComfInc    () const;
    float     getComfDec    () const;
    DietType  getDietType   () const;  // Derived from calculateDietType()
    bool      ifFlocks      () const;
    unsigned  getFlee       () const;
    unsigned  getPursue     () const;

    //============================================================================
    //  Behaviours - New Motivation/Action System
    //============================================================================
    void hungryBehavior     (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index,
                             GeneralStats &gs);
    void thirstyBehavior    (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index);
    void amorousBehavior    (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index,
                             GeneralStats &gs);
    void contentBehavior    (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index);
    void tiredBehavior      (World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index);
    
    //============================================================================
    //  Behaviours - Legacy Profile System (Deprecated)
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
    template<typename Predicate>
    bool spiralSearch       (const std::vector<std::vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             Predicate predicate,
                             unsigned maxRadius = 0);  // 0 = use getSightRange()
    template<typename Visitor>
    void forEachTileInRange (unsigned maxRadius, Visitor visitor);
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
    //  Sensory System
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
     * @brief Check if creature has meaningful scent detection capability.
     *        Used to determine if scent-based fallback navigation is available.
     * @return True if scent_detection trait exceeds threshold (0.1)
     */
    bool hasScentDetection() const;
    
    /**
     * @brief Find the coordinates of food scent (plant scent) in range.
     *        Used as a fallback when visual plant detection fails.
     * @param scentLayer The world's scent layer to query
     * @param outX Output parameter for scent X coordinate
     * @param outY Output parameter for scent Y coordinate
     * @return True if a valid food scent was found, false otherwise
     */
    bool findFoodScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const;
    
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
    //  Plant Interaction
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

    //============================================================================
    //  Behavior System (Phase 3 - Creature God Class decomposition)
    //============================================================================
    /**
     * @brief Get the behavior controller for this creature.
     * @return Pointer to BehaviorController, or nullptr if not initialized
     */
    EcoSim::Genetics::BehaviorController* getBehaviorController();
    const EcoSim::Genetics::BehaviorController* getBehaviorController() const;
    
    /**
     * @brief Update creature using the new behavior system.
     * @param ctx Behavior context with world access and timing
     * @return Result of behavior execution
     *
     * The BehaviorController selects and executes the highest priority
     * applicable behavior.
     */
    EcoSim::Genetics::BehaviorResult updateWithBehaviors(EcoSim::Genetics::BehaviorContext& ctx);
    
    /**
     * @brief Build a behavior context from current creature and world state.
     * @param world Reference to the world
     * @param scentLayer Reference to the scent layer
     * @param currentTick Current simulation tick
     * @return BehaviorContext populated with current state
     */
    EcoSim::Genetics::BehaviorContext buildBehaviorContext(
        World& world,
        EcoSim::ScentLayer& scentLayer,
        unsigned int currentTick) const;
    
    /**
     * @brief Initialize the behavior controller with default behaviors.
     *
     * Registers all standard behaviors (feeding, hunting, mating, rest, movement, zoochory).
     */
    void initializeBehaviorController();

public:
    //============================================================================
    //  Variable Generators
    //============================================================================
    char        generateChar ();
    [[deprecated("Use CreatureTaxonomy::generateScientificName() instead")]]
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
