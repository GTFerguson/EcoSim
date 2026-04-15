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
    //  New Genetics System - Static Methods
    //============================================================================
    /**
     * @brief Initialize the shared gene registry with default gene definitions.
     *        Should be called once at application startup before creating creatures.
     */
    static void initializeGeneRegistry();
    
    /**
     * @brief Reset the static ID counter to a specific value.
     *        Used after loading saves to synchronize ID counter with loaded creatures.
     * @param nextId The next ID to use for new creatures (typically max loaded ID + 1)
     */
    static void resetIdCounter(int nextId);
    
    /**
     * @brief Get the current next ID value (for debugging/testing).
     * @return The next ID that will be assigned to a new creature
     */
    static int getNextId();
    
    /**
     * @brief Get the shared gene registry (initializes if not already done).
     * @return Reference to the shared GeneRegistry
     */
    static EcoSim::Genetics::GeneRegistry& getGeneRegistry();
    
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
    //  ILifecycle overrides - Gene-dependent lifespan
    //============================================================================
    
    /**
     * @brief Get maximum lifespan based on genome
     * @return Maximum age before natural death
     */
    unsigned int getMaxLifespan() const override;
    
    /**
     * @brief Check if creature is alive
     * @return True if alive (considers hunger, thirst, health, age)
     */
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
    
    //============================================================================
    //  ILifecycle Interface Getters (use Organism's age_)
    //============================================================================
    unsigned int getAge() const override { return Organism::getAge(); }
    float getAgeNormalized() const override;
    void age(unsigned int ticks = 1) override;
    
    float     getHunger     () const;
    float     getThirst     () const;
    float     getFatigue    () const;
    float     getMate       () const;
    float     getMetabolism () const;
    unsigned  getSpeed      () const;
    Direction   getDirection  () const;
    Motivation  getMotivation () const;
    Action      getAction     () const;
    
    //============================================================================
    //  Float Position Getters
    //============================================================================
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
    
    //============================================================================
    //  Growth State Getters (use Organism's members)
    //============================================================================
    /**
     * @brief Get current physical size.
     * @return Current size (0.0 to maxSize_)
     */
    float getCurrentSize() const { return Organism::getCurrentSize(); }
    
    /**
     * @brief Check if creature has reached maturity.
     * @return True if creature has reached 50% of max size
     */
    bool isMature() const { return Organism::isMature(); }
    
    /**
     * @brief Get ratio of current size to max size.
     * @return Size ratio (0.0 to 1.0)
     */
    float getSizeRatio() const { return Organism::getSizeRatio(); }
    
    /**
     * @brief Set current size (for deserialization).
     * @param size New current size value
     */
    void setCurrentSize(float s) { currentSize_ = s; }
    
    /**
     * @brief Set maximum size (for deserialization).
     * @param size New maximum size value
     */
    void setMaxSize(float s) { maxSize_ = s; }
    
    /**
     * @brief Set maturity state (for deserialization).
     * @param mature New maturity state
     */
    void setMature(bool m) { mature_ = m; }
    
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
    float getHealth() const { return Organism::getHealth(); }
    
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
     * - Reduces health_ directly
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
     * @brief Get current environmental stress state.
     * @return Reference to current temperature stress for UI/debugging
     */
    const EcoSim::Genetics::TemperatureStress& getEnvironmentalStress() const {
        static const EcoSim::Genetics::TemperatureStress kDefault{};
        return thermal_ ? thermal_->currentStress : kDefault;
    }
    
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
    
    /**
     * @brief Get biome adaptation (environmental specialization).
     * @return Pointer to BiomeAdaptation flyweight, or nullptr if temperate
     */
    const EcoSim::Genetics::BiomeAdaptation* getBiomeAdaptation() const;
    
    /**
     * @brief Reclassify biome adaptation based on current genome.
     *
     * Call this after modifying thermal genes (fur density, fat layer, temp tolerance)
     * to update the biome classification. Automatically updates population tracking.
     */
    void reclassifyBiomeAdaptation();
    
    /**
     * @brief Get full label combining archetype and biome (e.g., "Arctic Pack", "Jungle Titan").
     * @return Full label string with biome prefix and archetype suffix
     *
     * Uses the BiomeAdaptation's getFullLabel() method which combines
     * the biome prefix with the archetype's role suffix.
     * Falls back to archetype label alone if no biome adaptation.
     */
    std::string getFullLabel() const;
    
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
    //  Core Simulation Methods
    //============================================================================
    short deathCheck        () const;
    float shareResource     (const int &amount, float &resource);
    float shareFood         (const int &amount);
    float shareWater        (const int &amount);
    void  changeDirection   (const int &xChange, const int &yChange);
    float calculateDistance (const int &goalX, const int &goalY) const;
    void  movementCost      (const float &distance);

    //============================================================================
    //  Breeding
    //============================================================================
    float     checkFitness  (const Creature &c2) const;

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
    //============================================================================
    //  Variable Generators
    //============================================================================
    char        generateChar ();
    [[deprecated("Use CreatureTaxonomy::generateScientificName() instead")]]
    std::string generateName ();
    //============================================================================
    //  To String
    //============================================================================
    Direction   stringToDirection (const std::string &str);
    std::string directionToString () const;
    std::string toString          () const;
    
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
