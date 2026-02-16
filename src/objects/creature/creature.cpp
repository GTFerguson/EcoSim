/**
 * Title    : EcoSim - Creature
 * Author   : Gary Ferguson
 * Created  : Oct 18th, 2019
 * Purpose  : This replicates the behaviour of a basic creature or agent
 *            that is capable of decision making and has it's own genetics.
*/

#include "../../../include/objects/creature/creature.hpp"
#include "objects/creature/CreatureCombat.hpp"
#include "objects/creature/CreatureScent.hpp"
#include "objects/creature/CreaturePlantInteraction.hpp"
#include "objects/creature/CreatureSerialization.hpp"
#include "objects/creature/CreatureResourceSearch.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "world/SpatialIndex.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/classification/BiomeAdaptation.hpp"
#include "genetics/classification/CreatureTaxonomy.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "logging/Logger.hpp"
#include <unordered_map>
#include <optional>

// Debug logging for behavior/profile diagnostics - set to 1 to enable verbose logging
#ifndef CREATURE_BEHAVIOR_DEBUG_LOG
#define CREATURE_BEHAVIOR_DEBUG_LOG 0
#endif

#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/behaviors/FeedingBehavior.hpp"
#include "genetics/behaviors/HuntingBehavior.hpp"
#include "genetics/behaviors/MatingBehavior.hpp"
#include "genetics/behaviors/MovementBehavior.hpp"
#include "genetics/behaviors/RestBehavior.hpp"
#include "genetics/behaviors/ThirstBehavior.hpp"
#include "genetics/behaviors/ZoochoryBehavior.hpp"
#include "genetics/systems/HealthSystem.hpp"

using namespace std;

//================================================================================
//  DietInfo Helper
//================================================================================
struct DietInfo {
    char character;
    std::string namePrefix;
};

static const DietInfo& getDietInfo(EcoSim::Genetics::DietType diet) {
    using DietType = EcoSim::Genetics::DietType;
    static const std::unordered_map<DietType, DietInfo> dietMap = {
        { DietType::HERBIVORE, { 'H', "Her" } },
        { DietType::FRUGIVORE, { 'F', "Fru" } },
        { DietType::OMNIVORE,  { 'O', "Omn" } },
        { DietType::CARNIVORE, { 'C', "Car" } },
        { DietType::NECROVORE, { 'N', "Nec" } }
    };
    
    auto it = dietMap.find(diet);
    if (it != dietMap.end()) {
        return it->second;
    }
    static DietInfo defaultInfo = { 'O', "Omn" };  // Default to omnivore
    return defaultInfo;
}

//================================================================================
//  Static Member Initialization - New Genetics System
//================================================================================
std::shared_ptr<EcoSim::Genetics::GeneRegistry> Creature::s_geneRegistry = nullptr;

//================================================================================
//  Static Member Initialization - Creature-specific ID Counter
//================================================================================
int Creature::nextCreatureId_ = 0;

//================================================================================
//  Static Member Initialization - Logging
//================================================================================
static int s_nextCreatureId = 1;

void Creature::resetIdCounter(int nextId) {
    s_nextCreatureId = nextId;
}

int Creature::getNextId() {
    return s_nextCreatureId;
}

//================================================================================
//  Static Member Initialization - Creature-Plant Interactions
//================================================================================
std::unique_ptr<EcoSim::Genetics::FeedingInteraction> Creature::s_feedingInteraction = nullptr;
std::unique_ptr<EcoSim::Genetics::SeedDispersal> Creature::s_seedDispersal = nullptr;

//================================================================================
//  Static Member Initialization - Behavior System Services
//================================================================================
std::unique_ptr<EcoSim::Genetics::PerceptionSystem> Creature::s_perceptionSystem = nullptr;
std::unique_ptr<EcoSim::Genetics::CombatInteraction> Creature::s_combatInteraction = nullptr;

//================================================================================
//  Constants
//================================================================================
const float Creature::DIAG_ADJUST       = 1.4f;
const float Creature::RESOURCE_LIMIT    = 10.0f;
const float Creature::BREED_COST        = 3.0f;
const float Creature::IDEAL_SIMILARITY  = 0.8f;
const float Creature::PENALTY_EXPONENT  = 1.5f;
const float Creature::INIT_FATIGUE      = 0.0f;
const float Creature::PREY_CALORIES     = 3.0f;
//  Constants for managing creature death
const float Creature::STARVATION_POINT  = -0.1f;
const float Creature::DEHYDRATION_POINT = -0.1f;
const float Creature::DISCOMFORT_POINT  = -3.0f;  // Increased from -1.0f to allow starvation to occur first
//  What fraction of resources is shared
const unsigned Creature::RESOURCE_SHARED = 4;

//  Scent detection constants (extracted from duplicated magic numbers)
const float Creature::SCENT_DETECTION_BASE_RANGE    = 100.0f;  // Base scent detection range
const float Creature::SCENT_DETECTION_ACUITY_MULT   = 100.0f;  // Multiplier for olfactory acuity bonus
const float Creature::DEFAULT_OLFACTORY_ACUITY      = 0.5f;    // Default detection ability
const float Creature::DEFAULT_SCENT_PRODUCTION      = 0.5f;    // Default pheromone production rate

//  Seed dispersal constants
const float Creature::BURR_SEED_VIABILITY           = 0.85f;   // Viability for burr-dispersed seeds
const float Creature::GUT_SEED_SCARIFICATION_BONUS  = 1.15f;   // Bonus for optimal gut transit
const float Creature::GUT_SEED_ACID_DAMAGE          = 0.9f;    // Penalty for prolonged gut transit
const float Creature::DEFAULT_GUT_TRANSIT_HOURS     = 6.0f;    // Default seed transit time
const float Creature::TICKS_PER_HOUR                = 10.0f;   // Conversion factor for time

//  Feeding interaction constants
const float Creature::FEEDING_MATE_BOOST            = 2.0f;    // Mate (comfort) boost from successful feeding
const float Creature::DAMAGE_HUNGER_COST            = 0.5f;    // Hunger cost per point of damage
const float Creature::SEEKING_FOOD_MATE_PENALTY     = 0.5f;    // Comfort reduction while seeking food (multiplied by comfDec)

//  Sense enhancement constants
const float Creature::COLOR_VISION_RANGE_BONUS      = 0.3f;    // Range bonus from color vision
const float Creature::SCENT_DETECTION_RANGE_BONUS   = 0.5f;    // Range bonus from scent detection

//  Movement system constants
const float Creature::BASE_MOVEMENT_SPEED           = 0.5f;    // Base speed multiplier (tiles per tick)
const float Creature::MIN_MOVEMENT_SPEED            = 0.1f;    // Minimum speed floor
const float Creature::DEFAULT_LEG_LENGTH            = 0.5f;    // Default leg length for creatures without gene
const float Creature::DEFAULT_BODY_MASS             = 1.0f;    // Default body mass for creatures without gene


//================================================================================
//  Copy/Move Constructors and Assignment Operators (M5)
//================================================================================
/**
 * Copy constructor - performs deep copy of unique_ptr members
 */
Creature::Creature(const Creature& other)
    : GameObject(other),
      Organism(other.x_, other.y_, other.genome_, getGeneRegistry()),
      _worldX(other._worldX), _worldY(other._worldY),
      _direction(other._direction),
      _motivation(other._motivation),
      _action(other._action),
      _inCombat(other._inCombat),
      _isFleeing(other._isFleeing),
      _targetId(other._targetId),
      _combatCooldown(other._combatCooldown),
      _metabolism(other._metabolism),
      _speed(other._speed),
      _archetype(other._archetype),
      _biomeAdaptation(other._biomeAdaptation),
      _attachedBurrs(other._attachedBurrs),
      _gutSeeds(other._gutSeeds),
      creatureId_(nextCreatureId_++) {
    // Copy Organism state that isn't set in constructor
    age_ = other.age_;
    health_ = other.health_;
    currentSize_ = other.currentSize_;
    maxSize_ = other.maxSize_;
    mature_ = other.mature_;
    needs_ = other.needs_;

    // Increment archetype population for the copy
    if (_archetype) {
        _archetype->incrementPopulation();
    }
    // Increment biome adaptation population for the copy
    if (_biomeAdaptation) {
        _biomeAdaptation->incrementPopulation();
    }

    // Behavior controller is NOT copied - use lazy initialization
    behaviorController_ = nullptr;
}

/**
 * Move constructor
 */
Creature::Creature(Creature&& other) noexcept
    : GameObject(std::move(other)),
      Organism(std::move(other)),
      _worldX(other._worldX), _worldY(other._worldY),
      _direction(other._direction),
      _motivation(other._motivation),
      _action(other._action),
      _inCombat(other._inCombat),
      _isFleeing(other._isFleeing),
      _targetId(other._targetId),
      _combatCooldown(other._combatCooldown),
      _metabolism(other._metabolism),
      _speed(other._speed),
      _archetype(other._archetype),
      _biomeAdaptation(other._biomeAdaptation),
      _attachedBurrs(std::move(other._attachedBurrs)),
      _gutSeeds(std::move(other._gutSeeds)),
      creatureId_(other.creatureId_)
{
    // Transfer archetype without incrementing - just null out source
    other._archetype = nullptr;
    // Transfer biome adaptation without incrementing - just null out source
    other._biomeAdaptation = nullptr;
}

/**
 * Copy assignment operator
 *
 * Since Organism has copy disabled, we must manually copy all state.
 * This creates a new genome/phenotype rather than using Organism's copy assignment.
 */
Creature& Creature::operator=(const Creature& other) {
    if (this != &other) {
        // Decrement old archetype population before reassignment
        if (_archetype) {
            _archetype->decrementPopulation();
        }
        // Decrement old biome adaptation population before reassignment
        if (_biomeAdaptation) {
            _biomeAdaptation->decrementPopulation();
        }
        
        GameObject::operator=(other);
        
        // Copy Organism state manually (Organism has copy disabled)
        x_ = other.x_;
        y_ = other.y_;
        age_ = other.age_;
        alive_ = other.alive_;
        health_ = other.health_;
        currentSize_ = other.currentSize_;
        maxSize_ = other.maxSize_;
        mature_ = other.mature_;
        genome_ = other.genome_;  // Genome has copy constructor
        phenotype_ = EcoSim::Genetics::Phenotype(&genome_, &getGeneRegistry());
        registry_ = other.registry_;
        id_ = other.id_;
        
        // Copy Creature-specific state
        _worldX = other._worldX;
        _worldY = other._worldY;
        _direction = other._direction;
        _motivation = other._motivation;
        _action = other._action;
        _inCombat = other._inCombat;
        _isFleeing = other._isFleeing;
        _targetId = other._targetId;
        _combatCooldown = other._combatCooldown;
        _metabolism = other._metabolism;
        _speed = other._speed;
        needs_ = other.needs_;

        // Copy archetype and increment population
        _archetype = other._archetype;
        if (_archetype) {
            _archetype->incrementPopulation();
        }

        // Copy biome adaptation and increment population
        _biomeAdaptation = other._biomeAdaptation;
        if (_biomeAdaptation) {
            _biomeAdaptation->incrementPopulation();
        }

        // Copy vector members
        _attachedBurrs = other._attachedBurrs;
        _gutSeeds = other._gutSeeds;

        // Reset behavior controller - lazy initialization will recreate it when needed
        behaviorController_.reset();
    }
    return *this;
}

/**
 * Move assignment operator
 */
Creature& Creature::operator=(Creature&& other) noexcept {
    if (this != &other) {
        // Decrement old archetype population before reassignment
        if (_archetype) {
            _archetype->decrementPopulation();
        }
        // Decrement old biome adaptation population before reassignment
        if (_biomeAdaptation) {
            _biomeAdaptation->decrementPopulation();
        }
        
        GameObject::operator=(std::move(other));
        Organism::operator=(std::move(other));
        
        // Move Creature-specific state
        _worldX = other._worldX;
        _worldY = other._worldY;
        _direction = other._direction;
        _motivation = other._motivation;
        _action = other._action;
        _inCombat = other._inCombat;
        _isFleeing = other._isFleeing;
        _targetId = other._targetId;
        _combatCooldown = other._combatCooldown;
        _metabolism = other._metabolism;
        _speed = other._speed;
        
        // Transfer archetype - don't increment, just null source
        _archetype = other._archetype;
        other._archetype = nullptr;
        
        // Transfer biome adaptation - don't increment, just null source
        _biomeAdaptation = other._biomeAdaptation;
        other._biomeAdaptation = nullptr;
        
        // Move vector members
        _attachedBurrs = std::move(other._attachedBurrs);
        _gutSeeds = std::move(other._gutSeeds);
        
        // Move behavior controller from source
        behaviorController_ = std::move(other.behaviorController_);
    }
    return *this;
}

//================================================================================
//  Destructor
//================================================================================
/**
 * Destructor - decrements archetype population when creature dies
 */
Creature::~Creature() {
    if (_archetype) {
        _archetype->decrementPopulation();
    }
    if (_biomeAdaptation) {
        _biomeAdaptation->decrementPopulation();
    }
}

//================================================================================
//  Setters
//================================================================================
void Creature::setAge     (unsigned age) { age_    = age;    }
void Creature::setHunger  (float hunger) { needs_.energy = hunger; }
void Creature::setThirst  (float thirst) { needs_.hydration = thirst; }
void Creature::setFatigue (float fatigue) { needs_.fatigue = fatigue; }
void Creature::setMate    (float mate)   { needs_.reproductiveUrge = mate; }
void Creature::setXY      (int x, int y) { _worldX = static_cast<float>(x); _worldY = static_cast<float>(y); }
void Creature::setX       (int x)        { _worldX = static_cast<float>(x); }
void Creature::setY       (int y)        { _worldY = static_cast<float>(y); }
void Creature::setMotivation(Motivation m) { _motivation = m; }
void Creature::setAction(Action a)       { _action = a; }

// Float position setters
void Creature::setWorldPosition(float x, float y) { _worldX = x; _worldY = y; }
void Creature::setWorldX(float x) { _worldX = x; }
void Creature::setWorldY(float y) { _worldY = y; }

//================================================================================
//  Getters
//================================================================================
//================================================================================
//  ILifecycle Interface Implementation
//================================================================================
unsigned int Creature::getMaxLifespan() const {
    return getLifespan();
}

float Creature::getAgeNormalized() const {
    unsigned int lifespan = getLifespan();
    if (lifespan == 0) return 0.0f;
    return static_cast<float>(age_) / static_cast<float>(lifespan);
}

bool Creature::isAlive() const {
    return deathCheck() == 0;
}

void Creature::age(unsigned int ticks) {
    age_ += ticks;
}
float     Creature::getHunger     () const { return needs_.energy;           }
float     Creature::getThirst     () const { return needs_.hydration;       }
float     Creature::getFatigue    () const { return needs_.fatigue;          }
float     Creature::getMate       () const { return needs_.reproductiveUrge; }
float     Creature::getMetabolism () const { return _metabolism;            }
unsigned  Creature::getSpeed      () const { return _speed;                 }

// Float position getters
// getX(), getY(), getWorldX(), getWorldY() are now defined inline in creature.hpp
// delegating to Organism or using _worldX/_worldY directly
int       Creature::tileX         () const { return static_cast<int>(_worldX); }
int       Creature::tileY         () const { return static_cast<int>(_worldY); }

// Movement speed calculation
float Creature::getMovementSpeed() const {
    // Get gene values from phenotype if available
    float locomotion = DEFAULT_LEG_LENGTH;  // Base movement capability
    float legLength = DEFAULT_LEG_LENGTH;   // Stride length
    float bodyMass = DEFAULT_BODY_MASS;     // Weight affects speed
    
    // Use LOCOMOTION gene for movement speed capability
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::LOCOMOTION)) {
        locomotion = phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::LOCOMOTION);
    }
    // Use MAX_SIZE as proxy for body mass (larger creatures are heavier)
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE)) {
        bodyMass = phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE);
        // Normalize max_size - gene value is 0.5-20.0, scale to reasonable mass range
        bodyMass = 0.5f + (bodyMass / 20.0f) * 1.5f;  // Range: 0.5 to 2.0
    }
    // Leg length could be derived from body morphology genes
    // For now, use locomotion as a proxy
    legLength = 0.3f + locomotion * 0.7f;  // Range: 0.3 to 1.0
    
    // Speed formula: baseSpeed = (locomotion * legLength) / sqrt(mass)
    float speed = (BASE_MOVEMENT_SPEED * locomotion * legLength) / std::sqrt(bodyMass);
    
    // Ensure minimum speed
    return std::max(MIN_MOVEMENT_SPEED, speed);
}
Direction Creature::getDirection  () const { return _direction; }

// Genetics-only getters - derive all values from phenotype
unsigned Creature::getLifespan() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::LIFESPAN)) {
        return static_cast<unsigned>(
            phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::LIFESPAN)
        );
    }
    return 500000;  // Default lifespan
}

unsigned Creature::getSightRange() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::SIGHT_RANGE)) {
        return static_cast<unsigned>(
            phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::SIGHT_RANGE)
        );
    }
    return 100;  // Default sight range
}

// Phenotype-derived behavioral thresholds
float Creature::getTHunger() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::HUNGER_THRESHOLD)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::HUNGER_THRESHOLD);
    }
    return 3.0f;  // Default hunger threshold
}

float Creature::getTThirst() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::THIRST_THRESHOLD)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::THIRST_THRESHOLD);
    }
    return 3.0f;  // Default thirst threshold
}

float Creature::getTFatigue() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::FATIGUE_THRESHOLD)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::FATIGUE_THRESHOLD);
    }
    return 3.0f;  // Default fatigue threshold
}

float Creature::getTMate() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::MATE_THRESHOLD)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::MATE_THRESHOLD);
    }
    return 3.0f;  // Default mate threshold
}

float Creature::getComfInc() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::COMFORT_INCREASE)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::COMFORT_INCREASE);
    }
    return 0.01f;  // Default comfort increase
}

float Creature::getComfDec() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::COMFORT_DECREASE)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::COMFORT_DECREASE);
    }
    return 0.01f;  // Default comfort decrease
}

// Diet is now emergent from digestion genes
DietType Creature::getDietType() const {
    return phenotype_.calculateDietType();
}

bool Creature::ifFlocks() const {
    // Flocking behavior derived from social genes
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::HUNT_INSTINCT)) {
        // Low hunt instinct means more social/flocking tendency
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::HUNT_INSTINCT) < 0.5f;
    }
    return true;  // Default to flocking
}

unsigned Creature::getFlee() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::FLEE_THRESHOLD)) {
        return static_cast<unsigned>(phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::FLEE_THRESHOLD));
    }
    return 10;  // Default flee distance
}

unsigned Creature::getPursue() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::PURSUE_THRESHOLD)) {
        return static_cast<unsigned>(phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::PURSUE_THRESHOLD));
    }
    return 20;  // Default pursue distance
}

//================================================================================
//  New Genetics System - Static Methods (M5)
//================================================================================
/**
 * Initialize the shared gene registry with default gene definitions.
 * Should be called once at application startup before creating creatures.
 */
void Creature::initializeGeneRegistry() {
    if (!s_geneRegistry) {
        s_geneRegistry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
        EcoSim::Genetics::UniversalGenes::registerDefaults(*s_geneRegistry);
    }
}

/**
 * Get the shared gene registry (initializes if not already done).
 */
EcoSim::Genetics::GeneRegistry& Creature::getGeneRegistry() {
    if (!s_geneRegistry) {
        initializeGeneRegistry();
    }
    return *s_geneRegistry;
}

//================================================================================
//  New Genetics System - Instance Methods
//================================================================================

/**
 * Update phenotype context with current environment and organism state.
 * Should be called each tick or when environment changes significantly.
 */
void Creature::updatePhenotypeContext(const EcoSim::Genetics::EnvironmentState& env) {
    EcoSim::Genetics::OrganismState orgState;
    
    // Calculate normalized age (0.0 = birth, 1.0 = max lifespan)
    unsigned lifespan = getLifespan();  // Use phenotype-derived value
    if (lifespan > 0) {
        orgState.age_normalized = static_cast<float>(age_) / static_cast<float>(lifespan);
    } else {
        orgState.age_normalized = 0.0f;
    }
    
    // Convert hunger level to energy (higher hunger = lower energy)
    // _hunger ranges from about -1 to RESOURCE_LIMIT (10.0)
    orgState.energy_level = std::max(0.0f, std::min(1.0f, needs_.energy / RESOURCE_LIMIT));
    
    // Health is always 1.0 for now (could be expanded later)
    orgState.health = 1.0f;
    
    phenotype_.updateContext(env, orgState);
    
    // Mark thermal cache dirty when phenotype context changes
    _thermalCacheDirty = true;
    
    // Clamp health when maxHealth changes due to modulation
    // This ensures health_ never exceeds current maxHealth
    float maxHP = getMaxHealth();
    if (health_ > maxHP) {
        health_ = maxHP;
    }
}

/**
 * Update thermal adaptation cache from phenotype.
 * Extracts ThermalAdaptations and calculates EffectiveToleranceRange.
 */
void Creature::updateThermalCache() {
    using namespace EcoSim::Genetics;
    
    _cachedThermalAdaptations = EnvironmentalStressCalculator::extractThermalAdaptations(phenotype_);
    _cachedBaseTempLow = phenotype_.getTrait(UniversalGenes::TEMP_TOLERANCE_LOW);
    _cachedBaseTempHigh = phenotype_.getTrait(UniversalGenes::TEMP_TOLERANCE_HIGH);
    _cachedToleranceRange = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        _cachedBaseTempLow, _cachedBaseTempHigh, _cachedThermalAdaptations);
    _thermalCacheDirty = false;
}

/**
 * Recalculate phenotype from genome (Organism interface).
 * Simple wrapper that calls updatePhenotypeContext with default environment.
 */
void Creature::updatePhenotype() {
    EcoSim::Genetics::EnvironmentState defaultEnv;
    updatePhenotypeContext(defaultEnv);
}

//================================================================================
//  Behaviours (legacy Profile methods removed — now handled by BehaviorController)
//================================================================================

void Creature::grow() {
    if (mature_) return;  // Already fully grown
    
    // Growth rate based on nutrition (energy relative to needs)
    float nutritionFactor = std::clamp(needs_.energy / 100.0f, 0.1f, 1.5f);
    
    // Base growth rate from genetics (could use METABOLISM gene)
    float baseGrowthRate = 0.001f;  // Small increment per tick
    
    // Age factor - grow faster when young
    float ageFactor = 1.0f;
    if (age_ < 1000) {
        ageFactor = 1.5f;  // Juveniles grow faster
    }
    
    // Calculate growth increment
    float growthIncrement = baseGrowthRate * nutritionFactor * ageFactor;
    
    // Apply growth
    currentSize_ = std::min(currentSize_ + growthIncrement, maxSize_);
    
    // Check maturity (50% of max size, matching Plant)
    if (currentSize_ >= maxSize_ * 0.5f) {
        mature_ = true;
    }
}

/**
 *  This method checks the relevant values of a creature to determine
 *  whether it is dead or not. A creature can die of old age, starvation,
 *  dehydration, and of discomfort.
 *
 *  @param c The creature being checked.
 *  @return  Returns 0 if the creature is not dead.
 *            1 - Old age
 *            2 - Starvation
 *            3 - Dehydration
 *            4 - Discomfort
 */
short Creature::deathCheck () const {
  //  First check creatures age against limit remove if dead
  if      (age_            > getLifespan())      return 1;
  else if (needs_.energy   < STARVATION_POINT)   return 2;
  else if (needs_.hydration < DEHYDRATION_POINT) return 3;
  else if (needs_.reproductiveUrge < DISCOMFORT_POINT) return 4;
  else if (health_         <= 0.0f)              return 5;  // Combat death

  return 0;
}

/**
 *  This method allows a creature to share a resource with another.
 *  It is given variable to divide it's current amount by to work out
 *  what is to be given.
 *  
 *  @param amount The lower the value the higher the amount
 *  @return       The amount of a resource given.
 */
float Creature::shareResource (const int& amount, float& resource) {
  float shared;

  if (resource > 0.0f) {
    shared = resource / RESOURCE_SHARED;
    resource -= shared;
  } else {
    shared = 0.0f;
  }

  return shared;
}

float Creature::shareFood (const int& amount) {
  return shareResource (amount, needs_.energy);
}

float Creature::shareWater (const int& amount) {
  return shareResource (amount, needs_.hydration);
}

//================================================================================
//  Scent Detection Helper
//================================================================================

/**
 * Check if creature has meaningful scent detection capability.
 * Used to determine if scent-based fallback navigation is available.
 * @return True if scent_detection trait exceeds threshold (0.1)
 */
bool Creature::hasScentDetection() const {
  return CreatureScent::hasScentDetection(*this);
}

/**
 * Find the coordinates of food scent (plant scent) in range.
 * Returns target coordinates for A* pathfinding navigation.
 * Uses FOOD_TRAIL scent type from plants that have deposited scent.
 *
 * @param scentLayer The world's scent layer to query
 * @param outX Output parameter for scent X coordinate
 * @param outY Output parameter for scent Y coordinate
 * @return True if a valid food scent was found, false otherwise
 */
bool Creature::findFoodScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const {
  return CreatureScent::findFoodScent(*this, scentLayer, outX, outY);
}


//  TODO Change to degree system
/**
 *  Changes the direction the creature is facing to be used for line of sight
 *  as well as allowing other creatures to better predict where they are going
 *  as to allow them to more effectively move towards them.
 *
 *  @param xChange  Movement made along x-axis
 *  @param yChange  Movement made along y-axis
 */
void Creature::changeDirection (const int &xChange, const int &yChange) {
  if (xChange == 1) {
    if      (yChange ==  1) _direction = Direction::SE;
    else if (yChange == -1) _direction = Direction::NE;
    else                    _direction = Direction::E;
  } else if (xChange == -1) {
    if      (yChange ==  1) _direction = Direction::SW;
    else if (yChange == -1) _direction = Direction::NW;
    else                    _direction = Direction::W;
  } else {
    if      (yChange ==  1) _direction = Direction::S;
    else if (yChange == -1) _direction = Direction::N;
    else                    _direction = Direction::none;
  }
}

/**
 *  Calculates the euclidean distance between two points.
 *
 *  @param goalX  The x-coordinate of the destination.
 *  @param goalY  The y-coordinate of the destination.
 */
float Creature::calculateDistance (const int &goalX, const int &goalY) const {
  // Phase 1: Float Movement - use tile positions for grid-based distance calculation
  float xDist = tileX() - goalX;
  float yDist = tileY() - goalY;
  return sqrt(xDist * xDist + yDist * yDist);
}

void Creature::movementCost (const float &distance) {
  //  First get non-diagonal movement by getting absolute difference
  needs_.energy -= _metabolism * distance;
}

//================================================================================
//  Breeding
//================================================================================
/**
 *  The fitness is calculated as a float value between 0 and 1.5. The higher the
 *  value returned the better the fitness, and therefore the more likely it is to
 *  breed. The proximity is given half the weight of the similarity.
 *
 *  desirability = proximity / 2 + similarity
 *
 *  GENETICS-MIGRATION: Now uses new genetics genome comparison instead of legacy genome.
 *  Compares genome allele values to determine genetic similarity.
 *
 *  @param c2 The creature who's fitness is being calculated.
 *  @return   The desirability of this pairing.
 */
float Creature::checkFitness (const Creature &c2) const {
  float distance   = calculateDistance(c2.getX(), c2.getY());
  unsigned sight = getSightRange();
  float proximity  = 1.0f - distance / static_cast<float>(sight);
  
  // Calculate genetic similarity using Genome::compare()
  // genome_ is always valid (value member inherited from Organism)
  float similarity = genome_.compare(c2.getGenome());

  //  Penalise if too similar
  if (similarity > IDEAL_SIMILARITY) {
    similarity -= pow (similarity - IDEAL_SIMILARITY, PENALTY_EXPONENT);
  }

  return proximity / 2 + similarity;
}

/**
 *  This method takes two creatures and splices their DNA together to
 *  create offspring based on them.
 *
 *  GENETICS-MIGRATION: Now uses new genetics system for breeding.
 *  Performs gene-by-gene crossover with random selection from each parent.
 *
 *  @param c1   First parent of the new creature.
 *  @param c2   Second parent of the new creature.
 *  @return     Offspring created from the parents combined DNA.
 */
Creature Creature::breedCreature (Creature &mate) {
  //  Charge the cost to breed to parents
  needs_.energy -= Creature::BREED_COST; needs_.hydration -= Creature::BREED_COST;
  mate.setHunger (mate.getHunger() - Creature::BREED_COST);
  mate.setThirst (mate.getThirst() - Creature::BREED_COST);

  //  Give the offspring a quarter of each parents resources
  float hunger = shareFood(RESOURCE_SHARED)  + mate.shareFood(RESOURCE_SHARED);
  float thirst = shareWater(RESOURCE_SHARED) + mate.shareWater(RESOURCE_SHARED);
  //  Erroneous value check to be safe
  if (hunger > RESOURCE_LIMIT) hunger = RESOURCE_LIMIT;
  if (thirst > RESOURCE_LIMIT) thirst = RESOURCE_LIMIT;

  //  Reset the parents mating levels
  needs_.reproductiveUrge = 0.0f; mate.setMate (0.0f);

  // Create offspring genome using Genome::crossover()
  // genome_ is always valid (value member inherited from Organism)
  EcoSim::Genetics::Genome crossed = EcoSim::Genetics::Genome::crossover(genome_, mate.getGenome(), 0.5f);
  
  // Apply mutation (5% rate)
  crossed.mutate(0.05f, getGeneRegistry().getAllDefinitions());
  
  std::unique_ptr<EcoSim::Genetics::Genome> offspringGenome =
      std::make_unique<EcoSim::Genetics::Genome>(std::move(crossed));

  Creature offspring(tileX(), tileY(), hunger, thirst, std::move(offspringGenome));
  
  // Log the birth event
  logging::Logger::getInstance().creatureBorn(
    offspring.getId(),
    offspring.generateName(),
    id_,
    mate.getId()
  );
  
  return offspring;
}

//================================================================================
//  IReproducible Interface Implementation
//================================================================================

/**
 * Check if creature can reproduce.
 * Uses existing fitness checks: mature, healthy, sufficient resources.
 */
bool Creature::canReproduce() const {
    bool hasResources = needs_.energy > BREED_COST && needs_.hydration > BREED_COST;
    bool isHealthy = health_ > getMaxHealth() * 0.25f;
    
    return isMature() && hasResources && isHealthy && _motivation == Motivation::Amorous;
}

/**
 * Get reproductive urge (normalized 0.0-1.0).
 * Based on internal _mate state normalized to resource limit.
 */
float Creature::getReproductiveUrge() const {
    // _mate typically ranges from negative (discomfort) to RESOURCE_LIMIT
    // Normalize to 0.0-1.0 range
    float urge = (needs_.reproductiveUrge + 3.0f) / (RESOURCE_LIMIT + 3.0f);  // Offset for negative values
    return std::max(0.0f, std::min(1.0f, urge));
}

/**
 * Get energy cost of reproduction.
 * Returns BREED_COST, potentially adjusted by genetics in the future.
 */
float Creature::getReproductionEnergyCost() const {
    // Currently uses static BREED_COST; could be gene-driven in future
    return BREED_COST;
}

/**
 * Get reproduction mode.
 * Creatures always reproduce sexually.
 */
EcoSim::Genetics::ReproductionMode Creature::getReproductionMode() const {
    return EcoSim::Genetics::ReproductionMode::SEXUAL;
}

/**
 * Check compatibility with another organism for mating.
 * Uses existing checkFitness logic based on genetic similarity.
 */
bool Creature::isCompatibleWith(const EcoSim::Genetics::Organism& other) const {
    // Try to cast to Creature (only creatures can mate with creatures)
    const Creature* otherCreature = dynamic_cast<const Creature*>(&other);
    if (!otherCreature) {
        return false;  // Can't mate with non-creatures
    }
    
    // Use existing fitness check - returns desirability score
    // Consider compatible if fitness is above a threshold
    float fitness = checkFitness(*otherCreature);
    return fitness > 0.3f;  // Minimum compatibility threshold
}

/**
 * Reproduce to create offspring.
 * Wraps existing breedCreature logic but returns Organism pointer.
 *
 * @todo Return concrete Creature type once Creature/Plant are unified into Organism
 */
std::unique_ptr<EcoSim::Genetics::Organism> Creature::reproduce(
    const EcoSim::Genetics::Organism* partner) {
    
    // Sexual reproduction requires a partner
    if (!partner) {
        return nullptr;
    }
    
    // Partner must be a Creature
    // @todo Remove dynamic_cast once Creature/Plant are unified into Organism
    Creature* mateCreature = const_cast<Creature*>(
        dynamic_cast<const Creature*>(partner)
    );
    if (!mateCreature) {
        return nullptr;  // Can't mate with non-creatures
    }
    
    // Check if we can actually reproduce
    if (!canReproduce()) {
        return nullptr;
    }
    
    // Use existing breedCreature logic
    Creature offspring = breedCreature(*mateCreature);
    
    // Return as unique_ptr<Organism>
    return std::make_unique<Creature>(std::move(offspring));
}

//================================================================================
//  Sensory System Methods (Phase 1)
//================================================================================

/**
 * Compute this creature's unique genetic scent signature.
 * Delegates to CreatureScent::computeScentSignature.
 */
std::array<float, 8> Creature::computeScentSignature() const {
  return CreatureScent::computeScentSignature(*this);
}

/**
 * Deposit breeding pheromone when in breeding state.
 * Delegates to CreatureScent::depositBreedingScent.
 */
void Creature::depositBreedingScent(EcoSim::ScentLayer& layer, unsigned int currentTick) {
  CreatureScent::depositBreedingScent(*this, layer, currentTick);
}

//================================================================================
//  Scent Detection Methods (Phase 2: Gradient Navigation)
//================================================================================

/**
 * Calculate genetic similarity between two scent signatures.
 * Delegates to CreatureScent::calculateSignatureSimilarity.
 */
float Creature::calculateSignatureSimilarity(
    const std::array<float, 8>& sig1,
    const std::array<float, 8>& sig2) {
  return CreatureScent::calculateSignatureSimilarity(sig1, sig2);
}

/**
 * Detect the direction to a potential mate using scent trails.
 * Delegates to CreatureScent::detectMateDirection.
 */
std::optional<Direction> Creature::detectMateDirection(const EcoSim::ScentLayer& scentLayer) const {
  return CreatureScent::detectMateDirection(*this, scentLayer);
}

/**
 * Find the coordinates of the strongest mate scent in range.
 * Delegates to CreatureScent::findMateScent.
 */
bool Creature::findMateScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const {
  return CreatureScent::findMateScent(*this, scentLayer, outX, outY);
}

//================================================================================
//  Plant Interaction Methods
//================================================================================

/**
	* Initialize shared interaction calculators.
	* Should be called once at application startup.
	*/
void Creature::initializeInteractionSystems() {
	   if (!s_feedingInteraction) {
	       s_feedingInteraction = std::make_unique<EcoSim::Genetics::FeedingInteraction>();
	   }
	   if (!s_seedDispersal) {
	       s_seedDispersal = std::make_unique<EcoSim::Genetics::SeedDispersal>();
	   }
}

/**
	* Attempt to eat a plant using genetics-based feeding calculations.
	* Delegates core logic to CreaturePlantInteraction namespace.
	*/
EcoSim::Genetics::FeedingResult Creature::eatPlant(EcoSim::Genetics::Plant& plant) {
	   using namespace EcoSim::Genetics;
	   
	   // Ensure interaction systems are initialized
	   if (!s_feedingInteraction) {
	       initializeInteractionSystems();
	   }
	   
	   
	   // Delegate to namespace function for core feeding logic
	   FeedingResult result = CreaturePlantInteraction::eatPlant(
	       *this, plant, *s_feedingInteraction
	   );
	   
	   // Handle seed consumption (requires access to _gutSeeds member)
	   if (result.success && result.seedsConsumed && result.seedsToDisperse > 0) {
	       CreaturePlantInteraction::consumeSeeds(
	           *this, plant, static_cast<int>(result.seedsToDisperse),
	           1.0f - (result.seedsDestroyed ? 0.5f : 0.0f), _gutSeeds
	       );
	   }
	   
	   return result;
}

/**
	* Check if creature can eat the given plant.
	* Delegates to CreaturePlantInteraction namespace.
	*/
bool Creature::canEatPlant(const EcoSim::Genetics::Plant& plant) const {
	   if (!s_feedingInteraction) {
	       const_cast<Creature*>(this)->initializeInteractionSystems();
	   }
	   
	   
	   return CreaturePlantInteraction::canEatPlant(*this, plant, *s_feedingInteraction);
}

/**
	* Get maximum plant detection range based on creature's senses.
	* Delegates to CreaturePlantInteraction namespace.
	*/
float Creature::getPlantDetectionRange() const {
	   
	   return CreaturePlantInteraction::getPlantDetectionRange(*this);
}

/**
	* Attach a burr from a plant to this creature.
	* Delegates to CreaturePlantInteraction namespace.
	*/
void Creature::attachBurr(const EcoSim::Genetics::Plant& plant) {
	   if (!s_seedDispersal) {
	       initializeInteractionSystems();
	   }
	   
	   
	   CreaturePlantInteraction::attachBurr(*this, plant, *s_seedDispersal, _attachedBurrs);
}

/**
	* Process burr detachment and return dispersal events.
	* Delegates to CreaturePlantInteraction namespace.
	*/
std::vector<EcoSim::Genetics::DispersalEvent> Creature::detachBurrs() {
	   if (!s_seedDispersal) {
	       return {};
	   }
	   
	   return CreaturePlantInteraction::detachBurrs(*this, *s_seedDispersal, _attachedBurrs);
}

/**
	* Check if creature has burrs attached.
	* Delegates to CreaturePlantInteraction namespace.
	*/
bool Creature::hasBurrs() const {
	   return CreaturePlantInteraction::hasBurrs(_attachedBurrs);
}

/**
	* Get pending dispersal events from attached burrs.
	* Delegates to CreaturePlantInteraction namespace.
	*/
std::vector<EcoSim::Genetics::DispersalEvent> Creature::getPendingBurrDispersal() const {
	   return CreaturePlantInteraction::getPendingBurrDispersal(*this, _attachedBurrs);
}

/**
	* Add seeds to gut for digestion and dispersal.
	* Delegates to CreaturePlantInteraction namespace.
	*/
void Creature::consumeSeeds(const EcoSim::Genetics::Plant& plant, int count, float viability) {
	   CreaturePlantInteraction::consumeSeeds(*this, plant, count, viability, _gutSeeds);
}

/**
	* Process gut seed passage and return dispersal events.
	* Delegates to CreaturePlantInteraction namespace.
	*/
std::vector<EcoSim::Genetics::DispersalEvent> Creature::processGutSeeds(int ticksElapsed) {
	   return CreaturePlantInteraction::processGutSeeds(*this, ticksElapsed, _gutSeeds);
}

//================================================================================
//  Variable Generators
//================================================================================
/**
 *  Generates a character representation of a creature based on it's diet.
 *  DRY refactoring: Now uses centralized DietInfo lookup table.
 *
 *  @param diet The diet used to generate the character.
 *  @return     The character used to represent the creature.
 */
char Creature::generateChar () {
  // Use archetype render character if available
  if (_archetype) {
    return _archetype->getRenderChar();
  }
  // Fallback to diet-based character
  return getDietInfo(getDietType()).character;
}

/**
 *  This method generates a species name for a specimen based on their genetics.
 *  Not only does this provide some interesting flavour text to the simulation,
 *  it also gives something that is a lot quicker to examine to see if certain
 *  subspecies have become dominant.
 *  DRY refactoring: Diet prefix now uses centralized DietInfo lookup table.
 *
 *  @return The creatures species name.
 */
string Creature::generateName () {
  string name = getDietInfo(getDietType()).namePrefix;

  if (ifFlocks()) {
    unsigned  flee      = getFlee ();
    int       diffFlock = flee - getPursue();

    //  Fleeing behaviour
    if (diffFlock > 0) {
      if (flee < 10)  name += "tim";  
      else            name += "gax";

    //  Flocking behaviour
    } else {
      if      (flee < 10) name += "milia";
      else if (flee < 20) name += "micus";
      else                name += "verec";
    }

  //  Independant behaviour
  } else {
    name += "mita";
  }

  name += " ";

  unsigned lifespan = getLifespan ();
  if      (lifespan < 250000) name += "Brevi";
  else if (lifespan < 500000) name += "Aevi";
  else if (lifespan < 750000) name += "Diu";
  else                        name += "Perti";

  unsigned sight = getSightRange ();
  if      (sight < 60)  name += "caecus";
  else if (sight < 120) name += "visus";
  else                  name += "sensus";

  return name;
}

//================================================================================
//  To String
//================================================================================
Direction Creature::stringToDirection (const string &str) {
  return CreatureSerialization::stringToDirection(str);
}

/**
 *  This method converts the direction to a human readable string.
 *  Delegates to CreatureSerialization namespace.
 *
 *  @return A human readable string representation of the profile.
 */
string Creature::directionToString () const {
  return CreatureSerialization::directionToString(_direction);
}

/**
 *  This converts the creature objects member variables to a string that can
 *  be read by a user or saved to a file.
 *  Delegates to CreatureSerialization namespace.
 *
 *  @return A string representation of the creature.
 */
string Creature::toString () const {
  return CreatureSerialization::toString(*this);
}

//================================================================================
//  New Genetics System Getters
//================================================================================

/**
 * Get the scientific name for this creature.
 * Uses CreatureTaxonomy for dynamic scientific name generation.
 */
std::string Creature::getScientificName() const {
    // Use CreatureTaxonomy for dynamic scientific name generation
    // genome_ is always valid (value member inherited from Organism)
    return EcoSim::Genetics::CreatureTaxonomy::generateScientificName(genome_);
}

/**
 * Get the archetype label for this creature.
 * Uses ArchetypeIdentity if available, falls back to diet type.
 */
std::string Creature::getArchetypeLabel() const {
    // Use archetype label if available
    if (_archetype) {
        return _archetype->getLabel();
    }
    // Fallback to diet type if no archetype assigned
    return EcoSim::Genetics::Phenotype::dietTypeToString(getDietType());
}

/**
 * Get the biome adaptation for this creature.
 * Returns nullptr if no biome adaptation assigned (shouldn't happen normally).
 */
const EcoSim::Genetics::BiomeAdaptation* Creature::getBiomeAdaptation() const {
    return _biomeAdaptation;
}

/**
 * Get the full label combining biome and archetype.
 * Examples: "Arctic Pack", "Jungle Tyrant", or "Pack Hunter" (for temperate).
 */
std::string Creature::getFullLabel() const {
    if (_biomeAdaptation && _archetype) {
        return _biomeAdaptation->getFullLabel(_archetype);
    }
    return getArchetypeLabel();
}

/**
 * Reclassify biome adaptation based on current genome.
 * Call this after modifying thermal genes (fur density, fat layer, temp tolerance)
 * to update the biome classification. Automatically updates population tracking.
 */
void Creature::reclassifyBiomeAdaptation() {
    // Decrement old biome adaptation population
    if (_biomeAdaptation) {
        _biomeAdaptation->decrementPopulation();
    }
    
    // Reclassify based on current genome state
    _biomeAdaptation = EcoSim::Genetics::CreatureTaxonomy::classifyBiomeAdaptation(genome_);
    
    // Increment new biome adaptation population
    if (_biomeAdaptation) {
        _biomeAdaptation->incrementPopulation();
    }
}

//================================================================================
//  Health System
//================================================================================

/**
 * Get maximum health value.
 * Based on MAX_SIZE gene for creature mass scaling.
 */
float Creature::getMaxHealth() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE) * 10.0f;
    }
    return RESOURCE_LIMIT * 10.0f;  // 100.0f default
}

/**
 * Apply damage to creature's health.
 */
void Creature::takeDamage(float amount) {
    if (amount <= 0.0f) return;
    health_ = std::max(0.0f, health_ - amount);
}

/**
 * Heal creature's health.
 */
void Creature::heal(float amount) {
    if (amount <= 0.0f) return;
    health_ = std::min(getMaxHealth(), health_ + amount);
}

/**
 * Set combat state.
 */
void Creature::setInCombat(bool combat) {
    _inCombat = combat;
}

/**
 * Set combat target ID.
 */
void Creature::setTargetId(int targetId) {
    _targetId = targetId;
}

/**
 * Set combat cooldown ticks.
 */
void Creature::setCombatCooldown(int cooldown) {
    _combatCooldown = cooldown;
}

/**
 * Set fleeing state.
 */
void Creature::setFleeing(bool fleeing) {
    _isFleeing = fleeing;
}

/**
 * Get health as percentage (0.0 to 1.0).
 */
float Creature::getHealthPercent() const {
    float maxHealth = getMaxHealth();
    if (maxHealth <= 0.0f) return 0.0f;
    return std::max(0.0f, std::min(1.0f, getHealth() / maxHealth));
}

/**
 * Get wound state based on health percentage.
 */
WoundState Creature::getWoundState() const {
    float percent = getHealthPercent();
    if (percent > 0.75f) return WoundState::Healthy;
    if (percent > 0.50f) return WoundState::Injured;
    if (percent > 0.25f) return WoundState::Wounded;
    if (percent > 0.0f) return WoundState::Critical;
    return WoundState::Dead;
}

/**
 * Get wound severity (0.0 to 1.0).
 */
float Creature::getWoundSeverity() const {
    return 1.0f - getHealthPercent();
}

/**
 * Get healing rate per tick.
 */
float Creature::getHealingRate() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::REGENERATION_RATE)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::REGENERATION_RATE) * 0.001f;
    }
    return 0.001f;  // Default healing rate
}

//================================================================================
//  Combat System (Stub implementations for ImGui compatibility)
//================================================================================

/**
 * Check if creature is currently in combat.
 */
bool Creature::isInCombat() const {
    return _inCombat;
}

/**
 * Check if creature is currently fleeing.
 */
bool Creature::isFleeing() const {
    return _isFleeing;
}

/**
 * Get current combat target ID.
 */
int Creature::getTargetId() const {
    return _targetId;
}

/**
 * Get remaining combat cooldown ticks.
 */
int Creature::getCombatCooldown() const {
    return _combatCooldown;
}

/**
 * Get creature's current motivation state.
 */
Motivation Creature::getMotivation() const {
    return _motivation;
}

/**
 * Get creature's current action state.
 */
Action Creature::getAction() const {
    return _action;
}

/**
 * Get expressed value of a gene from the phenotype.
 */
float Creature::getExpressedValue(const std::string& geneId) const {
    if (phenotype_.hasTrait(geneId)) {
        return phenotype_.getTrait(geneId);
    }
    return 0.0f;
}

//================================================================================
//  New Genetics Constructors
//================================================================================

/**
 * Constructor with new genetics genome only.
 */
Creature::Creature(int x, int y, std::unique_ptr<EcoSim::Genetics::Genome> genome)
    : GameObject(),
      Organism(x, y, std::move(*genome), getGeneRegistry()),
      _worldX(static_cast<float>(x)),
      _worldY(static_cast<float>(y)),
      _direction(Direction::none),
      _motivation(Motivation::Content),
      _action(Action::Idle),
      _inCombat(false),
      _isFleeing(false),
      _targetId(-1),
      _combatCooldown(0),
      _metabolism(0.001f),
      _speed(1),
      _archetype(nullptr),
      creatureId_(nextCreatureId_++) {

    // Initialize needs
    needs_.energy = 1.0f;
    needs_.hydration = 1.0f;
    needs_.fatigue = INIT_FATIGUE;
    needs_.reproductiveUrge = 0.0f;
    needs_.maxEnergy = RESOURCE_LIMIT;
    needs_.maxHydration = RESOURCE_LIMIT;

    // Set metabolism from phenotype (Organism already created phenotype_)
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE)) {
        _metabolism = phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE) * 0.001f;
    } else {
        _metabolism = 0.001f;
    }

    // Classify archetype from genome (after phenotype is ready)
    _archetype = EcoSim::Genetics::CreatureTaxonomy::classifyArchetype(genome_);
    if (_archetype) {
        _archetype->incrementPopulation();
    }

    // Classify biome adaptation from genome
    _biomeAdaptation = EcoSim::Genetics::CreatureTaxonomy::classifyBiomeAdaptation(genome_);
    if (_biomeAdaptation) {
        _biomeAdaptation->incrementPopulation();
    }

    _character = generateChar();
    _name = generateName();

    // Initialize health to max health (phenotype must be ready)
    health_ = getMaxHealth();

    // Initialize phenotype context
    EcoSim::Genetics::EnvironmentState defaultEnv;
    updatePhenotypeContext(defaultEnv);
}

/**
 * Constructor with new genetics genome and initial hunger/thirst values.
 */
Creature::Creature(int x, int y, float hunger, float thirst,
                   std::unique_ptr<EcoSim::Genetics::Genome> genome)
    : GameObject(),
      Organism(x, y, std::move(*genome), getGeneRegistry()),
      _worldX(static_cast<float>(x)),
      _worldY(static_cast<float>(y)),
      _direction(Direction::none),
      _motivation(Motivation::Content),
      _action(Action::Idle),
      _inCombat(false),
      _isFleeing(false),
      _targetId(-1),
      _combatCooldown(0),
      _metabolism(0.001f),
      _speed(1),
      _archetype(nullptr),
      creatureId_(nextCreatureId_++) {

    // Initialize needs from parameters
    needs_.energy = hunger;
    needs_.hydration = thirst;
    needs_.fatigue = INIT_FATIGUE;
    needs_.reproductiveUrge = 0.0f;
    needs_.maxEnergy = RESOURCE_LIMIT;
    needs_.maxHydration = RESOURCE_LIMIT;

    // Set metabolism from phenotype (Organism already created phenotype_)
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE)) {
        _metabolism = phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE) * 0.001f;
    } else {
        _metabolism = 0.001f;
    }

    // Classify archetype from genome (after phenotype is ready)
    _archetype = EcoSim::Genetics::CreatureTaxonomy::classifyArchetype(genome_);
    if (_archetype) {
        _archetype->incrementPopulation();
    }

    // Classify biome adaptation from genome
    _biomeAdaptation = EcoSim::Genetics::CreatureTaxonomy::classifyBiomeAdaptation(genome_);
    if (_biomeAdaptation) {
        _biomeAdaptation->incrementPopulation();
    }

    _character = generateChar();
    _name = generateName();

    // Initialize health to max health (phenotype must be ready)
    health_ = getMaxHealth();

    // Initialize phenotype context
    EcoSim::Genetics::EnvironmentState defaultEnv;
    updatePhenotypeContext(defaultEnv);
}

//================================================================================
//  Behavior System Integration (Phase 3: God Class Decomposition)
//================================================================================

/**
 * Initialize the behavior controller with all behaviors.
 * Called once during creature construction or on first use.
 */
void Creature::initializeBehaviorController() {
    using namespace EcoSim::Genetics;
    
    if (behaviorController_) {
        return;  // Already initialized
    }
    
    // Initialize static services if needed (following existing pattern with s_feedingInteraction)
    if (!s_perceptionSystem) {
        s_perceptionSystem = std::make_unique<PerceptionSystem>();
    }
    if (!s_combatInteraction) {
        s_combatInteraction = std::make_unique<CombatInteraction>();
    }
    if (!s_feedingInteraction) {
        s_feedingInteraction = std::make_unique<FeedingInteraction>();
    }
    if (!s_seedDispersal) {
        s_seedDispersal = std::make_unique<SeedDispersal>();
    }
    
    behaviorController_ = std::make_unique<BehaviorController>();
    
    // Register behaviors in priority order
    // RestBehavior - CRITICAL priority when exhausted (no dependencies)
    behaviorController_->addBehavior(std::make_unique<RestBehavior>());

    // ThirstBehavior - HIGH priority when dehydrated (no dependencies)
    behaviorController_->addBehavior(std::make_unique<ThirstBehavior>());

    // HuntingBehavior - HIGH priority for carnivores
    behaviorController_->addBehavior(std::make_unique<HuntingBehavior>(
        *s_combatInteraction, *s_perceptionSystem));

    // FeedingBehavior - NORMAL priority for herbivores
    behaviorController_->addBehavior(std::make_unique<FeedingBehavior>(
        *s_feedingInteraction, *s_perceptionSystem));

    // MatingBehavior - NORMAL priority when ready to mate
    behaviorController_->addBehavior(std::make_unique<MatingBehavior>(
        *s_perceptionSystem, *s_geneRegistry));

    // ZoochoryBehavior - LOW priority for seed dispersal
    behaviorController_->addBehavior(std::make_unique<ZoochoryBehavior>(
        *s_seedDispersal));

    // MovementBehavior - IDLE priority as default fallback (no dependencies)
    behaviorController_->addBehavior(std::make_unique<MovementBehavior>());
}

/**
 * Build a behavior context from current creature and world state.
 * Creates an immutable snapshot for behavior execution.
 *
 * @note The organismState pointer in the returned context points to
 * a static local, which is valid for the duration of the behavior
 * update cycle. Do not store the pointer beyond a single tick.
 */
EcoSim::Genetics::BehaviorContext Creature::buildBehaviorContext(
    World& world,
    EcoSim::ScentLayer& scentLayer,
    unsigned int currentTick) const {
    
    using namespace EcoSim::Genetics;
    
    // Create organism state snapshot
    // Using thread_local static for safe temporary storage
    thread_local OrganismState tempState;
    tempState.age_normalized = getAgeNormalized();
    tempState.energy_level = std::max(0.0f, std::min(1.0f, needs_.energyRatio()));
    tempState.health = getHealthPercent();
    
    BehaviorContext ctx;
    ctx.scentLayer = &scentLayer;
    ctx.world = &world;
    ctx.organismState = &tempState;
    ctx.creatureIndex = world.getCreatureIndex();
    ctx.deltaTime = 1.0f;  // One tick = 1.0 time unit
    ctx.currentTick = currentTick;
    ctx.worldRows = world.getRows();
    ctx.worldCols = world.getCols();
    
    return ctx;
}

/**
 * Execute behavior update using the BehaviorController.
 * Replaces legacy decideBehaviour() + profile-based execution.
 */
EcoSim::Genetics::BehaviorResult Creature::updateWithBehaviors(EcoSim::Genetics::BehaviorContext& ctx) {
    using namespace EcoSim::Genetics;

    // Ensure behavior controller is initialized
    if (!behaviorController_) {
        initializeBehaviorController();
    }

    // Update the behavior controller — selects and executes the highest priority behavior
    BehaviorResult result = behaviorController_->update(*this, ctx);

    // Apply energy cost from behavior execution
    if (result.executed && result.energyCost > 0.0f) {
        needs_.energy -= result.energyCost;
    }

    // Derive Motivation/Action from the currently active behavior
    std::string behaviorId = behaviorController_->getCurrentBehaviorId();
    if (behaviorId == "feeding") {
        _motivation = Motivation::Hungry;
        _action = Action::Grazing;
    } else if (behaviorId == "hunting") {
        _motivation = Motivation::Hungry;
        _action = Action::Hunting;
    } else if (behaviorId == "thirst") {
        _motivation = Motivation::Thirsty;
        _action = result.completed ? Action::Drinking : Action::Searching;
    } else if (behaviorId == "mating") {
        _motivation = Motivation::Amorous;
        _action = Action::Courting;
    } else if (behaviorId == "rest") {
        _motivation = Motivation::Tired;
        _action = Action::Resting;
    } else if (behaviorId == "movement") {
        _motivation = Motivation::Content;
        _action = Action::Wandering;
    } else {
        _motivation = Motivation::Content;
        _action = Action::Idle;
    }

    // Passive lifecycle (shared framework — grow, metabolize, stress, reproduce, age)
    EnvironmentState env = phenotype_.getEnvironment();
    tickLifecycle(env);

    return result;
}

//================================================================================
//  Lifecycle Overrides (Organism::tickLifecycle framework)
//================================================================================

void Creature::tickMetabolism(const EcoSim::Genetics::EnvironmentState& /* env */) {
    float change = _metabolism;
    bool isResting = (_motivation == Motivation::Tired);
    if (isResting) {
        needs_.fatigue -= _metabolism;
        change /= 2;
    } else {
        needs_.fatigue += _metabolism;
    }

    // Update thermal cache if phenotype has changed
    if (_thermalCacheDirty) {
        updateThermalCache();
    }

    float currentTemp = phenotype_.getEnvironment().temperature;

    if (std::abs(currentTemp - _lastProcessedTemp) > 0.1f) {
        _currentEnvironmentalStress = EcoSim::Genetics::EnvironmentalStressCalculator::calculateTemperatureStress(
            currentTemp, _cachedBaseTempLow, _cachedBaseTempHigh, _cachedThermalAdaptations);
        _lastProcessedTemp = currentTemp;
    }

    // Apply energy drain multiplier from environmental stress
    change *= _currentEnvironmentalStress.energyDrainMultiplier;

    needs_.energy -= change;
    needs_.hydration -= change;
}

void Creature::tickEnvironmentalStress(const EcoSim::Genetics::EnvironmentState& /* env */) {
    if (_currentEnvironmentalStress.healthDamageRate > 0.0f) {
        float damage = _currentEnvironmentalStress.healthDamageRate * getMaxHealth();
        takeDamage(damage);
    }
}

void Creature::tickReproductiveDrive() {
    if (_motivation == Motivation::Hungry || _motivation == Motivation::Thirsty) {
        needs_.reproductiveUrge -= getComfDec() * 0.3f;
    } else if (_motivation == Motivation::Content) {
        needs_.reproductiveUrge += getComfInc();
    }
}

//================================================================================
//  Enum Conversion Helpers (for serialization)
//================================================================================

std::string Creature::woundStateToString(WoundState state) {
    return CreatureSerialization::woundStateToString(state);
}

WoundState Creature::stringToWoundState(const std::string& str) {
    return CreatureSerialization::stringToWoundState(str);
}

std::string Creature::motivationToString(Motivation m) {
    return CreatureSerialization::motivationToString(m);
}

Motivation Creature::stringToMotivation(const std::string& str) {
    return CreatureSerialization::stringToMotivation(str);
}

std::string Creature::actionToString(Action a) {
    return CreatureSerialization::actionToString(a);
}

Action Creature::stringToAction(const std::string& str) {
    return CreatureSerialization::stringToAction(str);
}

//================================================================================
//  JSON Serialization
//================================================================================

nlohmann::json Creature::toJson() const {
    return CreatureSerialization::toJson(*this);
}

Creature Creature::fromJson(const nlohmann::json& j, int mapWidth, int mapHeight) {
    return CreatureSerialization::fromJson(j, mapWidth, mapHeight);
}
