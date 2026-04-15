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

// Sensory system includes
#include "world/ScentLayer.hpp"

// New genetics system includes
#include "genetics/organisms/Organism.hpp"
#include "genetics/core/OrganismConstants.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/defaults/UniversalGenes.hpp"  // For DietType

// Creature-Plant interaction includes
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"

// Classification system
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/classification/BiomeAdaptation.hpp"

// Behavior system
#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/interactions/CombatInteraction.hpp"

// Environmental stress system
#include "genetics/expression/EnvironmentalStress.hpp"

// JSON serialization
#include <nlohmann/json.hpp>

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

 //  8-direction compass lives on MobilityComponent; alias keeps the
 //  unqualified name available to existing global-namespace callers.
using Direction = EcoSim::Genetics::Direction;

//  Motivation and Action enums live in genetics/core/MotivationAction.hpp
//  (included via Organism.hpp). Aliased here so existing unqualified
//  uses in creature-area code continue to work.
using Motivation = EcoSim::Genetics::Motivation;
using Action     = EcoSim::Genetics::Action;

//  Wound state enum for health system
enum class WoundState { Healthy, Injured, Wounded, Critical, Dead };


/**
 * @brief Creature organism that extends the Organism base class
 *
 * Creatures inherit shared functionality (position, lifecycle, growth, genetics)
 * from Organism and add creature-specific behaviors like movement, hunting,
 * mating, and complex decision-making through the behavior system.
 *
 * Key differences from plants:
 * - Creatures move (have precise float world coordinates)
 * - Creatures have needs (hunger, thirst, fatigue, mate drive)
 * - Creatures reproduce sexually
 * - Creatures have complex behaviors via BehaviorController
 */
class Creature: public GameObject,
                public EcoSim::Genetics::Organism {
  public:
    //============================================================================
    //  Constants live in genetics/core/OrganismConstants.hpp in the
    //  EcoSim::Genetics::Constants namespace. Imported at the top of
    //  this header for backward-compat access via Creature::FOO, which
    //  is re-exposed as static constexpr members below.
    //============================================================================
    static constexpr float RESOURCE_LIMIT            = EcoSim::Genetics::Constants::RESOURCE_LIMIT;
    static constexpr float BREED_COST                = EcoSim::Genetics::Constants::BREED_COST;
    static constexpr float DIAG_ADJUST               = EcoSim::Genetics::Constants::DIAG_ADJUST;
    static constexpr float IDEAL_SIMILARITY          = EcoSim::Genetics::Constants::IDEAL_SIMILARITY;
    static constexpr float PENALTY_EXPONENT          = EcoSim::Genetics::Constants::PENALTY_EXPONENT;
    static constexpr float INIT_FATIGUE              = EcoSim::Genetics::Constants::INIT_FATIGUE;
    static constexpr float PREY_CALORIES             = EcoSim::Genetics::Constants::PREY_CALORIES;
    static constexpr float STARVATION_POINT          = EcoSim::Genetics::Constants::STARVATION_POINT;
    static constexpr float DEHYDRATION_POINT         = EcoSim::Genetics::Constants::DEHYDRATION_POINT;
    static constexpr float DISCOMFORT_POINT          = EcoSim::Genetics::Constants::DISCOMFORT_POINT;
    static constexpr unsigned RESOURCE_SHARED        = EcoSim::Genetics::Constants::RESOURCE_SHARED;
    static constexpr float SCENT_DETECTION_BASE_RANGE  = EcoSim::Genetics::Constants::SCENT_DETECTION_BASE_RANGE;
    static constexpr float SCENT_DETECTION_ACUITY_MULT = EcoSim::Genetics::Constants::SCENT_DETECTION_ACUITY_MULT;
    static constexpr float DEFAULT_OLFACTORY_ACUITY    = EcoSim::Genetics::Constants::DEFAULT_OLFACTORY_ACUITY;
    static constexpr float DEFAULT_SCENT_PRODUCTION    = EcoSim::Genetics::Constants::DEFAULT_SCENT_PRODUCTION;
    static constexpr float BURR_SEED_VIABILITY          = EcoSim::Genetics::Constants::BURR_SEED_VIABILITY;
    static constexpr float GUT_SEED_SCARIFICATION_BONUS = EcoSim::Genetics::Constants::GUT_SEED_SCARIFICATION_BONUS;
    static constexpr float GUT_SEED_ACID_DAMAGE         = EcoSim::Genetics::Constants::GUT_SEED_ACID_DAMAGE;
    static constexpr float DEFAULT_GUT_TRANSIT_HOURS    = EcoSim::Genetics::Constants::DEFAULT_GUT_TRANSIT_HOURS;
    static constexpr float TICKS_PER_HOUR               = EcoSim::Genetics::Constants::TICKS_PER_HOUR;
    static constexpr float FEEDING_MATE_BOOST        = EcoSim::Genetics::Constants::FEEDING_MATE_BOOST;
    static constexpr float DAMAGE_HUNGER_COST        = EcoSim::Genetics::Constants::DAMAGE_HUNGER_COST;
    static constexpr float SEEKING_FOOD_MATE_PENALTY = EcoSim::Genetics::Constants::SEEKING_FOOD_MATE_PENALTY;
    static constexpr float COLOR_VISION_RANGE_BONUS    = EcoSim::Genetics::Constants::COLOR_VISION_RANGE_BONUS;
    static constexpr float SCENT_DETECTION_RANGE_BONUS = EcoSim::Genetics::Constants::SCENT_DETECTION_RANGE_BONUS;
    static constexpr float BASE_MOVEMENT_SPEED = EcoSim::Genetics::Constants::BASE_MOVEMENT_SPEED;
    static constexpr float MIN_MOVEMENT_SPEED  = EcoSim::Genetics::Constants::MIN_MOVEMENT_SPEED;
    static constexpr float DEFAULT_LEG_LENGTH  = EcoSim::Genetics::Constants::DEFAULT_LEG_LENGTH;
    static constexpr float DEFAULT_BODY_MASS   = EcoSim::Genetics::Constants::DEFAULT_BODY_MASS;

  private:
    
    //============================================================================
    //  Creature-specific ID Counter
    //============================================================================
    //  All per-instance state now lives on Organism base + components.
    //  Shared services (s_geneRegistry, s_feedingInteraction, s_seedDispersal,
    //  s_perceptionSystem, s_combatInteraction) and the nextCreatureId_
    //  counter also live on Organism base. Nothing Creature-specific
    //  remains on the class except method implementations.
    
    //============================================================================
    //  Private Breeding Implementation
    //============================================================================
    /**
     * @brief Internal breeding implementation (use reproduce() for public interface)
     * @param mate The mating partner
     * @return New offspring creature
     */
    Creature  breedCreature (Creature &mate);
    
    /**
     * @brief Update thermal adaptation cache from phenotype.
     *
     * Extracts ThermalAdaptations and calculates EffectiveToleranceRange
     * from phenotype traits. Called when _thermalCacheDirty is true.
     */
    void updateThermalCache();

  protected:
    //============================================================================
    //  Internal State Accessors (for testing and derived classes)
    //============================================================================
    // Enables unit testing of creature state transitions without exposing
    // implementation details in the public API. Test fixtures can inherit
    // from Creature to access these methods.
    
    float getInternalEnergy() const        { return heterotrophy_ ? heterotrophy_->hunger  : 0.0f; }
    float getInternalHydration() const     { return heterotrophy_ ? heterotrophy_->thirst  : 0.0f; }
    float getInternalFatigueLevel() const  { return heterotrophy_ ? heterotrophy_->fatigue : 0.0f; }
    float getInternalMatingUrge() const    { return reproduction_ ? reproduction_->mate    : 0.0f; }
    float getInternalHealthValue() const   { return health_; }
    bool getInternalCombatFlag() const           { return combat_ && combat_->inCombat; }
    bool getInternalFleeingFlag() const          { return combat_ && combat_->isFleeing; }
    int getInternalTargetCreatureId() const      { return combat_ ? combat_->targetId : -1; }
    int getInternalCombatCooldownTicks() const   { return combat_ ? combat_->combatCooldown : 0; }

    // Plant interaction state (lives on HeterotrophyComponent)
    static inline const std::vector<std::tuple<int, int, int, int>> kEmptyBurrs{};
    static inline const std::vector<std::tuple<int, float, int>> kEmptyGutSeeds{};
    const std::vector<std::tuple<int, int, int, int>>& getInternalAttachedBurrs() const {
        return heterotrophy_ ? heterotrophy_->attachedBurrs : kEmptyBurrs;
    }
    const std::vector<std::tuple<int, float, int>>& getInternalGutSeeds() const {
        return heterotrophy_ ? heterotrophy_->gutSeeds : kEmptyGutSeeds;
    }

    // Allow mutable access for test setup (requires heterotrophy_ to exist)
    std::vector<std::tuple<int, int, int, int>>& getInternalAttachedBurrsMutable() {
        return heterotrophy_->attachedBurrs;
    }
    std::vector<std::tuple<int, float, int>>& getInternalGutSeedsMutable() {
        return heterotrophy_->gutSeeds;
    }

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
    
   
   // Copy/Move constructors and assignment operators (required due to unique_ptr members)
    Creature(const Creature& other);
    Creature(Creature&& other) noexcept;
    Creature& operator=(const Creature& other);
    Creature& operator=(Creature&& other) noexcept;
    ~Creature() noexcept override;  // Decrements archetype and biome population counts
  
    //============================================================================
    //  Static methods (initializeGeneRegistry, getGeneRegistry, resetIdCounter,
    //  getNextId) all now live on Organism base; Creature inherits them.
    //============================================================================

    /**
     * @brief Get creature-specific sequential ID.
     *        Unlike getId() which is shared by all organisms, this ID
     *        is sequential only among creatures (0, 1, 2...).
     * @return Creature-specific ID
     */
    int getCreatureId() const { return identity_ ? identity_->sequentialId : -1; }

    /**
     * @brief Set creature-specific ID (for deserialization only).
     *        WARNING: Only use when loading saved games.
     * @param id The creature ID to set
     */
    void setCreatureId(int id) { if (identity_) identity_->sequentialId = id; }
    
    /**
     * @brief Reset the creature-specific ID counter.
     *        Used when starting a new game or after loading saves.
     * @param next The next creature ID to assign (default 0)
     */
    static void resetCreatureIdCounter(int next = 0) { nextCreatureId_ = next; }
  
    //============================================================================
    //  IPositionable overrides - Creatures have precise float movement
    //============================================================================
    
    /**
     * @brief Get world X coordinate (float precision)
     * @return X position in world coordinates
     *
     * Creatures have precise float positions for smooth movement.
     */
    float getWorldX() const override { return mobility_ ? mobility_->worldX : static_cast<float>(x_); }

    /**
     * @brief Get world Y coordinate (float precision)
     * @return Y position in world coordinates
     */
    float getWorldY() const override { return mobility_ ? mobility_->worldY : static_cast<float>(y_); }
    
    /**
     * @brief Set world position
     * @param x New X position
     * @param y New Y position
     *
     * Updates both world coordinates and tile coordinates.
     */
    void setWorldPosition(float x, float y) override;
    
    //============================================================================
    //  ILifecycle overrides
    //  getMaxLifespan, age, getAgeNormalized now use Organism defaults.
    //  isAlive overridden here because it needs deathCheck().
    //============================================================================
    bool isAlive() const override;
    
    //============================================================================
    //  IGenetic overrides
    //============================================================================
    
    /// Recalculate expressed traits from genome
    void updatePhenotype() override;
    
    //============================================================================
    //  IReproducible overrides - Sexual reproduction
    //============================================================================
    
    /**
     * @brief Check if creature can reproduce
     * @return true if mature, healthy, and has sufficient resources
     */
    bool canReproduce() const override;
    
    /**
     * @brief Get reproductive urge (normalized 0.0-1.0)
     * @return Mating drive based on _mate internal state
     */
    float getReproductiveUrge() const override;
    
    /**
     * @brief Get energy cost of reproduction
     * @return BREED_COST adjusted by genetics
     */
    float getReproductionEnergyCost() const override;
    
    /**
     * @brief Get reproduction mode
     * @return Always SEXUAL for creatures
     */
    EcoSim::Genetics::ReproductionMode getReproductionMode() const override;
    
    /**
     * @brief Check compatibility with another organism for mating
     * @param other The other organism
     * @return true if compatible based on species/archetype
     */
    bool isCompatibleWith(const EcoSim::Genetics::Organism& other) const override;
    
    /**
     * @brief Reproduce to create offspring
     * @param partner Partner for sexual reproduction (required for creatures)
     * @return Offspring as Organism pointer, or nullptr if reproduction fails
     */
    std::unique_ptr<EcoSim::Genetics::Organism> reproduce(
        const EcoSim::Genetics::Organism* partner = nullptr) override;
    
    //============================================================================
    //  Organism overrides - Growth system
    //============================================================================
    
    /**
     * @brief Get maximum size from phenotype
     * @return Maximum size the creature can reach
     */
    float getMaxSize() const override { return maxSize_; }
    
    /**
     * @brief Perform growth for this tick
     * Growth depends on nutrition and age factors.
     */
    void grow() override;
    
    void updatePhenotypeContext(const EcoSim::Genetics::EnvironmentState& env);

    //============================================================================
    //  Genetics System - Instance Methods
    //============================================================================
    
    /**
     * @brief Get the expressed value of a gene from the phenotype.
     * @param geneId The gene ID to look up
     * @return The expressed trait value, or 0.0 if not found or not using new genetics
     */
    //  getExpressedValue now lives on Organism base.

    //============================================================================
    //  Creature overrides heal to cap at organism getMaxHealth
    //============================================================================
    void  heal(float amount);

    //============================================================================
    //  Inherited from Organism base:
    //  - tile coordinates (tileX, tileY)
    //  - movement speed (getMovementSpeed)
    //  - health percent / wound severity / healing rate
    //  - damage (takeDamage)
    //  - combat state (setInCombat, setTargetId, setCombatCooldown, setFleeing,
    //                  isInCombat, isFleeing, getTargetId, getCombatCooldown)
    //  - expressed gene query (getExpressedValue)
    //============================================================================
    //============================================================================
    //  ILifecycle Interface Getters (use Organism's age_)
    //============================================================================
    unsigned int getAge() const override { return Organism::getAge(); }

    //============================================================================
    //  Growth State (forwards to Organism base)
    //============================================================================
    float getCurrentSize() const { return Organism::getCurrentSize(); }
    bool  isMature() const { return Organism::isMature(); }
    float getSizeRatio() const { return Organism::getSizeRatio(); }
    void  setCurrentSize(float s) { currentSize_ = s; }
    void  setMaxSize(float s) { maxSize_ = s; }
    void  setMature(bool m) { mature_ = m; }
    float getHealth() const { return Organism::getHealth(); }

    // getWoundState remains here because its return type uses the
    // global Direction/WoundState alias and Organism's signature uses
    // EcoSim::Genetics::WoundState — they're the same type via alias
    // but the declaration syntax differs; untangled later.
    WoundState getWoundState() const;
    
    /**
     * @brief Get current environmental stress state.
     * @return Reference to current temperature stress for UI/debugging
     */
    const EcoSim::Genetics::TemperatureStress& getEnvironmentalStress() const {
        static const EcoSim::Genetics::TemperatureStress kDefault{};
        return thermal_ ? thermal_->currentStress : kDefault;
    }
    
    // Taxonomy/classification getters now on Organism base:
    // getArchetypeLabel, getScientificName, getBiomeAdaptation,
    // reclassifyBiomeAdaptation, getFullLabel
    
    //  Genetics-Derived Getters now on Organism base:
    //  getLifespan, getSightRange, getTHunger/Thirst/Fatigue/Mate,
    //  getComfInc/Dec, getDietType, ifFlocks, getFlee, getPursue

    //============================================================================
    //  Core Simulation Methods
    //============================================================================
    //  Core simulation helpers (deathCheck, shareResource, shareFood,
    //  shareWater, changeDirection, calculateDistance, movementCost)
    //  now live on Organism base.

    //============================================================================
    //  Breeding
    //============================================================================
    float     checkFitness  (const Creature &c2) const;

    //============================================================================
    //  Sensory System — all scent/signature methods now on Organism base:
    //  depositBreedingScent, computeScentSignature, detectMateDirection,
    //  findMateScent, hasScentDetection, findFoodScent,
    //  calculateSignatureSimilarity.
    //============================================================================

    //  Plant interaction methods (eatPlant, canEatPlant, attachBurr,
    //  detachBurrs, hasBurrs, getPendingBurrDispersal, consumeSeeds,
    //  processGutSeeds, getPlantDetectionRange, initializeInteractionSystems)
    //  now live on Organism base.

    //============================================================================
    //  Behavior System
    //============================================================================
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
    //  generateChar, generateName, stringToDirection, directionToString
    //  now live on Organism base.
    //  toString stays on Creature because it uses GameObject::toString
    //  which isn't on the Organism (genetics) base class.
    virtual std::string toString() const override;
    
    //============================================================================
    //  JSON Serialization
    //============================================================================
    /**
     * @brief Serialize creature state to JSON.
     * @return JSON object containing all serializable creature state
     *
     * Includes identity, state, position, health, combat, behavior, and genome.
     * Phenotype is regenerated from genome on load, not serialized.
     */
    nlohmann::json toJson() const;
    
    /**
     * @brief Deserialize creature from JSON.
     * @param j JSON object containing creature state
     * @param mapWidth World width for position validation
     * @param mapHeight World height for position validation
     * @return Reconstructed Creature
     *
     * Genome is loaded from JSON and phenotype is regenerated.
     * Invalid positions are clamped to world bounds.
     */
    static Creature fromJson(const nlohmann::json& j, int mapWidth, int mapHeight);
    
    //============================================================================
    //  Enum Conversion Helpers (for serialization)
    //============================================================================
    static std::string woundStateToString(WoundState state);
    static WoundState stringToWoundState(const std::string& str);
    static std::string motivationToString(Motivation m);
    static Motivation stringToMotivation(const std::string& str);
    static std::string actionToString(Action a);
    static Action stringToAction(const std::string& str);
};

#endif
