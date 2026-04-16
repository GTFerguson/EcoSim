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
#include "genetics/systems/HealthSystem.hpp"   // canonical WoundState
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

//  Canonical WoundState lives in genetics/systems/HealthSystem.hpp.
//  Alias it into the global namespace so existing unqualified callers
//  (renderer overlay, serialisation) keep working unchanged.
using WoundState = EcoSim::Genetics::WoundState;


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
class Creature: public EcoSim::Genetics::Organism {
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
    
    // Reproduction lives on Organism base — Creature only supplies the
    // factory hook below to construct the concrete subtype.
    // updateThermalCache now on Organism base.

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
    // Copy assignment is deleted because Organism's is (complex ownership);
    // move assignment defers entirely to Organism::operator=. The
    // destructor is Organism's — no Creature-specific cleanup remains.
    Creature& operator=(const Creature& other) = delete;
    Creature& operator=(Creature&& other) noexcept;
    ~Creature() noexcept override = default;
  
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
    
    // getWorldX / getWorldY / setWorldPosition now all live on Organism.
    // The base reads mobility_ when present (creatures, future mobile
    // plants) and falls back to tile-center otherwise — one code path
    // covers precise-position movers and sessile organisms.
    
    //============================================================================
    //  ILifecycle overrides
    //  All ILifecycle methods (isAlive, getMaxLifespan, age, getAgeNormalized)
    //  now use Organism defaults. Organism::isAlive folds in deathCheck,
    //  which is safe for plants after gating the heterotroph-specific
    //  branches on component presence.
    //============================================================================

    //============================================================================
    //  IGenetic overrides
    //============================================================================
    
    /// Recalculate expressed traits from genome
    //  updatePhenotype uses Organism base default (just invalidates cache).
    
    //============================================================================
    //  IReproducible — fully on Organism base now. Creature only supplies
    //  makeOffspring (below) to construct the concrete subtype.
    //============================================================================

protected:
    std::unique_ptr<EcoSim::Genetics::Organism> makeOffspring(
        std::unique_ptr<EcoSim::Genetics::Genome> offspringGenome,
        int x, int y) override;
public:
    
    //============================================================================
    //  Organism overrides - Growth system
    //============================================================================
    //
    //  getMaxSize is Organism's default (returns maxSize_) — creatures keep
    //  maxSize_ at the 1.0 default and derive gene-scaled sizing via
    //  phenotype at the subsystems that need it. Plant overrides
    //  getMaxSize to read PlantGenes::MAX_SIZE directly.
    //
    //  grow and updatePhenotypeContext live on Organism. Plant overrides
    //  grow to add photosynthesis-driven growth.

    //============================================================================
    //  Genetics System - Instance Methods
    //============================================================================
    
    /**
     * @brief Get the expressed value of a gene from the phenotype.
     * @param geneId The gene ID to look up
     * @return The expressed trait value, or 0.0 if not found or not using new genetics
     */
    //  getExpressedValue now lives on Organism base.

    //  heal now lives on Organism base (uses getMaxHealth which is also on base).

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
    //  getAge, getCurrentSize, isMature, getSizeRatio, getHealth already
    //  inherited from Organism/ILifecycle base. Trivial forwarders removed.
    //============================================================================

    // getWoundState now on Organism base — global WoundState is an alias
    // for EcoSim::Genetics::WoundState, so the unqualified name still works.

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

    // checkFitness folded into Organism::isCompatibleWith default
    // (Genome::compare > 0.3 threshold matches the historical impl).

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
    //  Behavior system methods (getBehaviorController, updateWithBehaviors,
    //  buildBehaviorContext, initializeBehaviorController) now live on
    //  Organism base. Use getOrganismBehaviorController() to access the
    //  controller directly.
    //============================================================================

public:
    //  generateChar, generateName, stringToDirection, directionToString
    //  now live on Organism base. CreatureSerialization::toString is a
    //  free function; call it directly at the (one) production call site
    //  and test sites rather than going through a member forwarder.

    //============================================================================
    //  JSON Serialization
    //============================================================================
    //  Creature serialization is free-function only, lives in
    //  CreatureSerialization.hpp (toJson / fromJson / toString +
    //  enum<->string helpers). Member forwarders were removed during the
    //  class collapse — callers invoke the free functions directly.
};

#endif
