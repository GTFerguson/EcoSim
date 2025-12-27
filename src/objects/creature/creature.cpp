/**
 * Title    : EcoSim - Creature
 * Author   : Gary Ferguson
 * Created  : Oct 18th, 2019
 * Purpose  : This replicates the behaviour of a basic creature or agent
 *            that is capable of decision making and has it's own genetics.
*/

#include "../../../include/objects/creature/creature.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/classification/CreatureTaxonomy.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/systems/PerceptionSystem.hpp"  // Phase 4: Scent-based plant detection
#include "logging/Logger.hpp"
#include <unordered_map>

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
//  Static Member Initialization - Logging
//================================================================================
static int s_nextCreatureId = 1;

//================================================================================
//  Static Member Initialization - Creature-Plant Interactions
//================================================================================
std::unique_ptr<EcoSim::Genetics::FeedingInteraction> Creature::s_feedingInteraction = nullptr;
std::unique_ptr<EcoSim::Genetics::SeedDispersal> Creature::s_seedDispersal = nullptr;

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

//  Movement system constants (Phase 1: Float Movement)
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
      _worldX(other._worldX), _worldY(other._worldY),
      _age(other._age),
      _id(other._id),
      _direction(other._direction),
      _profile(other._profile),
      _motivation(other._motivation),
      _action(other._action),
      _health(other._health),
      _inCombat(other._inCombat),
      _isFleeing(other._isFleeing),
      _targetId(other._targetId),
      _combatCooldown(other._combatCooldown),
      _hunger(other._hunger),
      _thirst(other._thirst),
      _fatigue(other._fatigue),
      _mate(other._mate),
      _metabolism(other._metabolism),
      _speed(other._speed),
      _archetype(other._archetype) {
    // Deep copy unique_ptr members if they exist
    if (other._genome) {
        _genome = std::make_unique<EcoSim::Genetics::Genome>(*other._genome);
    }
    if (other._phenotype) {
        _phenotype = std::make_unique<EcoSim::Genetics::Phenotype>(_genome.get(), &getGeneRegistry());
    }
    // Increment archetype population for the copy
    if (_archetype) {
        _archetype->incrementPopulation();
    }
}

/**
 * Move constructor
 */
Creature::Creature(Creature&& other) noexcept
    : GameObject(std::move(other)),
      _worldX(other._worldX), _worldY(other._worldY),
      _age(other._age),
      _id(other._id),
      _direction(other._direction),
      _profile(other._profile),
      _motivation(other._motivation),
      _action(other._action),
      _health(other._health),
      _inCombat(other._inCombat),
      _isFleeing(other._isFleeing),
      _targetId(other._targetId),
      _combatCooldown(other._combatCooldown),
      _hunger(other._hunger),
      _thirst(other._thirst),
      _fatigue(other._fatigue),
      _mate(other._mate),
      _metabolism(other._metabolism),
      _speed(other._speed),
      _genome(std::move(other._genome)),
      _phenotype(std::move(other._phenotype)),
      _archetype(other._archetype) {
    // Transfer archetype without incrementing - just null out source
    other._archetype = nullptr;
}

/**
 * Copy assignment operator
 */
Creature& Creature::operator=(const Creature& other) {
    if (this != &other) {
        // Decrement old archetype population before reassignment
        if (_archetype) {
            _archetype->decrementPopulation();
        }
        
        GameObject::operator=(other);
        _worldX = other._worldX;
        _worldY = other._worldY;
        _age = other._age;
        _id = other._id;
        _direction = other._direction;
        _profile = other._profile;
        _motivation = other._motivation;
        _action = other._action;
        _health = other._health;
        _inCombat = other._inCombat;
        _isFleeing = other._isFleeing;
        _targetId = other._targetId;
        _combatCooldown = other._combatCooldown;
        _hunger = other._hunger;
        _thirst = other._thirst;
        _fatigue = other._fatigue;
        _mate = other._mate;
        _metabolism = other._metabolism;
        _speed = other._speed;
        
        // Deep copy unique_ptr members
        if (other._genome) {
            _genome = std::make_unique<EcoSim::Genetics::Genome>(*other._genome);
        } else {
            _genome.reset();
        }
        if (other._phenotype) {
            _phenotype = std::make_unique<EcoSim::Genetics::Phenotype>(_genome.get(), &getGeneRegistry());
        } else {
            _phenotype.reset();
        }
        
        // Copy archetype and increment population
        _archetype = other._archetype;
        if (_archetype) {
            _archetype->incrementPopulation();
        }
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
        
        GameObject::operator=(std::move(other));
        _worldX = other._worldX;
        _worldY = other._worldY;
        _age = other._age;
        _id = other._id;
        _direction = other._direction;
        _profile = other._profile;
        _motivation = other._motivation;
        _action = other._action;
        _health = other._health;
        _inCombat = other._inCombat;
        _isFleeing = other._isFleeing;
        _targetId = other._targetId;
        _combatCooldown = other._combatCooldown;
        _hunger = other._hunger;
        _thirst = other._thirst;
        _fatigue = other._fatigue;
        _mate = other._mate;
        _metabolism = other._metabolism;
        _speed = other._speed;
        _genome = std::move(other._genome);
        _phenotype = std::move(other._phenotype);
        
        // Transfer archetype - don't increment, just null source
        _archetype = other._archetype;
        other._archetype = nullptr;
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
}

//================================================================================
//  Setters
//================================================================================
void Creature::setAge     (unsigned age) { _age    = age;    }
void Creature::setHunger  (float hunger) { _hunger = hunger; }
void Creature::setThirst  (float thirst) { _thirst = thirst; }
void Creature::setFatigue (float fatigue) { _fatigue = fatigue; }
void Creature::setMate    (float mate)   { _mate   = mate;   }
void Creature::setXY      (int x, int y) { _worldX = static_cast<float>(x); _worldY = static_cast<float>(y); }
void Creature::setX       (int x)        { _worldX = static_cast<float>(x); }
void Creature::setY       (int y)        { _worldY = static_cast<float>(y); }
void Creature::setMotivation(Motivation m) { _motivation = m; }
void Creature::setAction(Action a)       { _action = a; }

// Float position setters (Phase 1: Float Movement)
void Creature::setWorldPosition(float x, float y) { _worldX = x; _worldY = y; }
void Creature::setWorldX(float x) { _worldX = x; }
void Creature::setWorldY(float y) { _worldY = y; }

//================================================================================
//  Getters
//================================================================================
unsigned  Creature::getAge        () const { return _age;                   }
int       Creature::getId         () const { return _id;                    }
float     Creature::getHunger     () const { return _hunger;                }
float     Creature::getThirst     () const { return _thirst;                }
float     Creature::getFatigue    () const { return _fatigue;               }
float     Creature::getMate       () const { return _mate;                  }
float     Creature::getMetabolism () const { return _metabolism;            }
unsigned  Creature::getSpeed      () const { return _speed;                 }
int       Creature::getX          () const { return tileX();                }
int       Creature::getY          () const { return tileY();                }

// Float position getters (Phase 1: Float Movement)
int       Creature::tileX         () const { return static_cast<int>(_worldX); }
int       Creature::tileY         () const { return static_cast<int>(_worldY); }
float     Creature::getWorldX     () const { return _worldX;                }
float     Creature::getWorldY     () const { return _worldY;                }

// Movement speed calculation (Phase 1: Float Movement)
float Creature::getMovementSpeed() const {
    // Get gene values from phenotype if available
    float locomotion = DEFAULT_LEG_LENGTH;  // Base movement capability
    float legLength = DEFAULT_LEG_LENGTH;   // Stride length
    float bodyMass = DEFAULT_BODY_MASS;     // Weight affects speed
    
    if (_phenotype) {
        // Use LOCOMOTION gene for movement speed capability
        if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::LOCOMOTION)) {
            locomotion = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::LOCOMOTION);
        }
        // Use MAX_SIZE as proxy for body mass (larger creatures are heavier)
        if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE)) {
            bodyMass = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE);
            // Normalize max_size - gene value is 0.5-20.0, scale to reasonable mass range
            bodyMass = 0.5f + (bodyMass / 20.0f) * 1.5f;  // Range: 0.5 to 2.0
        }
        // Leg length could be derived from body morphology genes
        // For now, use locomotion as a proxy
        legLength = 0.3f + locomotion * 0.7f;  // Range: 0.3 to 1.0
    }
    
    // Speed formula: baseSpeed = (locomotion * legLength) / sqrt(mass)
    float speed = (BASE_MOVEMENT_SPEED * locomotion * legLength) / std::sqrt(bodyMass);
    
    // Ensure minimum speed
    return std::max(MIN_MOVEMENT_SPEED, speed);
}
Direction Creature::getDirection  () const { return _direction; }
Profile   Creature::getProfile    () const { return _profile;   }

// Genetics-only getters - derive all values from phenotype
unsigned Creature::getLifespan() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::LIFESPAN)) {
        return static_cast<unsigned>(
            _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::LIFESPAN)
        );
    }
    return 500000;  // Default lifespan
}

unsigned Creature::getSightRange() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::SIGHT_RANGE)) {
        return static_cast<unsigned>(
            _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::SIGHT_RANGE)
        );
    }
    return 100;  // Default sight range
}

// Phenotype-derived behavioral thresholds
float Creature::getTHunger() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::HUNGER_THRESHOLD)) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::HUNGER_THRESHOLD);
    }
    return 3.0f;  // Default hunger threshold
}

float Creature::getTThirst() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::THIRST_THRESHOLD)) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::THIRST_THRESHOLD);
    }
    return 3.0f;  // Default thirst threshold
}

float Creature::getTFatigue() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::FATIGUE_THRESHOLD)) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::FATIGUE_THRESHOLD);
    }
    return 3.0f;  // Default fatigue threshold
}

float Creature::getTMate() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::MATE_THRESHOLD)) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::MATE_THRESHOLD);
    }
    return 3.0f;  // Default mate threshold
}

float Creature::getComfInc() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::COMFORT_INCREASE)) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::COMFORT_INCREASE);
    }
    return 0.01f;  // Default comfort increase
}

float Creature::getComfDec() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::COMFORT_DECREASE)) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::COMFORT_DECREASE);
    }
    return 0.01f;  // Default comfort decrease
}

// Diet is now emergent from digestion genes
DietType Creature::getDietType() const {
    if (_phenotype) {
        return _phenotype->calculateDietType();
    }
    return DietType::OMNIVORE;  // Default
}

bool Creature::ifFlocks() const {
    // Flocking behavior derived from social genes
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::HUNT_INSTINCT)) {
        // Low hunt instinct means more social/flocking tendency
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::HUNT_INSTINCT) < 0.5f;
    }
    return true;  // Default to flocking
}

unsigned Creature::getFlee() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::FLEE_THRESHOLD)) {
        return static_cast<unsigned>(_phenotype->getTrait(EcoSim::Genetics::UniversalGenes::FLEE_THRESHOLD));
    }
    return 10;  // Default flee distance
}

unsigned Creature::getPursue() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::PURSUE_THRESHOLD)) {
        return static_cast<unsigned>(_phenotype->getTrait(EcoSim::Genetics::UniversalGenes::PURSUE_THRESHOLD));
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
        // Use UniversalGenes (58+ genes) instead of DefaultGenes for Phase 2.1 coevolution support
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
 * Get the phenotype.
 */
const EcoSim::Genetics::Phenotype* Creature::getPhenotype() const {
    return _phenotype.get();
}

/**
 * Update phenotype context with current environment and organism state.
 * Should be called each tick or when environment changes significantly.
 */
void Creature::updatePhenotypeContext(const EcoSim::Genetics::EnvironmentState& env) {
    if (_phenotype) {
        EcoSim::Genetics::OrganismState orgState;
        
        // Calculate normalized age (0.0 = birth, 1.0 = max lifespan)
        unsigned lifespan = getLifespan();  // Use phenotype-derived value
        if (lifespan > 0) {
            orgState.age_normalized = static_cast<float>(_age) / static_cast<float>(lifespan);
        } else {
            orgState.age_normalized = 0.0f;
        }
        
        // Convert hunger level to energy (higher hunger = lower energy)
        // _hunger ranges from about -1 to RESOURCE_LIMIT (10.0)
        orgState.energy_level = std::max(0.0f, std::min(1.0f, _hunger / RESOURCE_LIMIT));
        
        // Health is always 1.0 for now (could be expanded later)
        orgState.health = 1.0f;
        
        _phenotype->updateContext(env, orgState);
        
        // Clamp health when maxHealth changes due to modulation
        // This ensures _health never exceeds current maxHealth
        float maxHP = getMaxHealth();
        if (_health > maxHP) {
            _health = maxHP;
        }
    }
}

//================================================================================
//  Behaviours
//================================================================================
void Creature::migrateProfile (World &world,
                               vector<Creature> &creatures,
                               const unsigned index) {
  setAction(Action::Wandering);
  
  if (ifFlocks()) {
    if (flock (world, creatures))
      return;
  }

  Navigator::wander (*this, world.getGrid(), world.getRows(), world.getCols());
}

void Creature::hungryProfile (World &world,
                              vector<Creature> &creatures,
                              const unsigned index,
                              GeneralStats &gs) {
  bool actionTaken = false;
  DietType dietType = getDietType();
  
  // Set initial searching action
  setAction(Action::Searching);
  
  if (dietType == DietType::CARNIVORE || dietType == DietType::NECROVORE) {
    setAction(Action::Hunting);
    actionTaken = findPrey
      (world.getGrid(), world.getRows(), world.getCols(), creatures, gs.deaths.predator);
  } else {
    // LEGACY REMOVAL: Only genetics-based plants now
    actionTaken = findGeneticsPlants(world, gs.feeding);
  }

  if (!actionTaken) {
    setAction(Action::Wandering);
    migrateProfile (world, creatures, index);
  }
}

void Creature::thirstyProfile (World &world,
                               vector<Creature> &creatures,
                               const unsigned index) {
  setAction(Action::Searching);
  
  if (!findWater(world.getGrid(), world.getRows(), world.getCols())) {
    setAction(Action::Wandering);
    migrateProfile (world, creatures, index);
  }
}

void Creature::breedProfile (World &world,
                             vector<Creature> &creatures,
                             const unsigned index,
                             GeneralStats &gs) {
  // Set courting action while seeking mate
  setAction(Action::Courting);
  
  // Deposit breeding scent each tick while in breeding mode
  depositBreedingScent(world.getScentLayer(), world.getCurrentTick());
  
  if (!findMate (world.getGrid(), world.getRows(),
        world.getCols(), creatures, index, gs.births)) {
    // Visual mate finding failed - try scent-based navigation
    int scentTargetX, scentTargetY;
    
    if (findMateScent(world.getScentLayer(), scentTargetX, scentTargetY)) {
      // Found a scent trail - navigate toward scent source using A* pathfinding
      Navigator::astarSearch(*this, world.getGrid(), world.getRows(), world.getCols(),
                            scentTargetX, scentTargetY);
    } else {
      // No scent detected - fall back to wandering
      migrateProfile (world, creatures, index);
    }
  }
}

/**
 *  Uses a flocking algorithm to gather creatures together  
 *
 *  @param world      The world object.
 *  @param creatures  A vector of all creatures.
 *  @return           True if an actiom was taken.
 */
bool Creature::flock (World &world, vector<Creature> &creatures) {
	Creature *closestC = NULL;
  float closestDistance = getSightRange();

	//	Find closest creature
  for (Creature & creature : creatures) {
    //  Exclude itself from the check
		if (&creature != this) {
      float distance = calculateDistance(creature.getX(), creature.getY());

			if (distance < closestDistance) {
				closestC = &creature;
				closestDistance = distance;
			}
		}
	}

	if (closestC != NULL) {
    int cX = closestC->getX();
    int cY = closestC->getY();
    unsigned mapWidth   = world.getCols();
    unsigned mapHeight  = world.getRows();

    float distance = calculateDistance(cX, cY);

    unsigned flee   = getFlee();
    unsigned pursue = getPursue();

		if (distance < flee) {			
      Navigator::moveAway 
        (*this, world.getGrid(), mapHeight,  mapWidth, cX, cY);	
			return true;

    //  Creature is too far, pursue
		} else if (flee < pursue && distance > pursue) {	
			Direction cDir = closestC->getDirection();

      //  Predict where target is moving
      switch (cDir) {
        case Direction::N:  cY -= distance; break;
        case Direction::S:  cY += distance; break;
        case Direction::E:  cX += distance; break;
        case Direction::W:  cX -= distance; break;
        case Direction::NE: cX += distance/2; cY -= distance/2; break;
        case Direction::NW: cX -= distance/2; cY -= distance/2; break;
        case Direction::SE: cX += distance/2; cY += distance/2; break;
        case Direction::SW: cX -= distance;   cY += distance/2; break;
        case Direction::none: default:                          break;
      }

      //  Clamp values
      if      (cX > mapWidth - 1)   cX = mapWidth - 1;
      else if (cX < 0)              cX = 0;
      if      (cY > mapHeight - 1)  cY = mapHeight - 1;
      else if (cY < 0)              cY = 0;

      return Navigator::astarSearch
        (*this, world.getGrid(), mapHeight, mapWidth, cX, cY);
		}
	}
  return false;
}


/**
 *  This method updates the creatures variables relevant to one turn within the
 *  simulation.
 */
void Creature::update () {
  decideBehaviour();

  //  Sleeping slows down metabolism
  float change = _metabolism;
  if (_profile == Profile::sleep) {
    _fatigue  -= _metabolism;
    change    /= 2;
  } else {
    _fatigue  += _metabolism;
  }

  // Track energy before changes for starvation logging
  float hungerBefore = _hunger;
  
  _hunger -= change;
  _thirst -= change;
  
  // Log when creature enters critical starvation zone (approaching death)
  if (hungerBefore > STARVATION_POINT && _hunger <= STARVATION_POINT + 0.5f) {
    logging::Logger::getInstance().starvation(_id, hungerBefore, _hunger);
  }
  
  _age++;
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
  if      (_age     > getLifespan())      return 1;
  else if (_hunger  < STARVATION_POINT)   return 2;
  else if (_thirst  < DEHYDRATION_POINT)  return 3;
  else if (_mate    < DISCOMFORT_POINT)   return 4;

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
  return shareResource (amount, _hunger); 
}

float Creature::shareWater (const int& amount) { 
  return shareResource (amount, _thirst); 
}

/**
 *  Based on the creatures needs and priorities decides their behaviour by setting
 *  the creature to the relevant profile.
 *
 *  @todo implementing profile elasticity and more nuanced decision making
 *  This prevents creatures getting stuck switching between resource profiles
 */
void Creature::decideBehaviour () {
  bool seekingResource =
    (_profile == Profile::hungry  && _hunger < getTHunger()) ||
    (_profile == Profile::thirsty && _thirst < getTThirst());
  bool isAsleep = _profile == Profile::sleep && _fatigue > 0.0f;

  if (seekingResource) {
    _mate   -= getComfDec() * 0.3f;  // Slowed discomfort buildup (Phase 1 fix)
  } else if (!isAsleep) {
    // Calculate the priority of each behaviour
    // Using std::array instead of vector to avoid heap allocation each tick
    std::array<float, 4> priorities = {{
      getTThirst()  - _thirst,
      getTHunger()  - _hunger,
      _mate         - getTMate(),
      _fatigue      - getTFatigue()
    }};

    auto highestPriority =
      max_element  (priorities.begin(), priorities.end());
    unsigned pIndex = static_cast<unsigned>(distance(priorities.begin(), highestPriority));

    if (*highestPriority  > 0.0f) {
      // Profile enum has 5 values: thirsty(0), hungry(1), breed(2), sleep(3), migrate(4)
      // priorities array only covers first 4, so pIndex is always valid (0-3)
      static constexpr unsigned MAX_PRIORITY_INDEX = 3;
      if (pIndex > MAX_PRIORITY_INDEX) {
        pIndex = MAX_PRIORITY_INDEX;  // Fallback to sleep if somehow out of range
      }
      _profile = static_cast<Profile>(pIndex);
      switch (_profile) {
        case Profile::thirsty: case Profile::hungry:
          _mate -= getComfDec() * 0.3f;  // Slowed discomfort buildup (Phase 1 fix)
          break;
        default:
          break;
      }

    //  All needs have been met
    } else {
      _mate     += getComfInc();
      _profile   = Profile::migrate;
    }
  }
}

// Legacy Food system removed - this is a stub for any remaining callers
static unsigned dietToFoodID(DietType diet) {
  // Legacy Food IDs no longer used - return 0 as placeholder
  return 0;
}

// LEGACY REMOVAL: foodCheck is deprecated - use findGeneticsPlants instead
// This stub remains for compatibility but always returns false
bool Creature::foodCheck (const vector<vector<Tile>> &map,
                          const unsigned &rows,
                          const unsigned &cols,
                          const int &x,
                          const int &y) {
  // Legacy Food system removed - creatures now only eat genetics Plants
  return false;
}

bool Creature::waterCheck (const vector<vector<Tile>> &map,
                           const unsigned &rows,
                           const unsigned &cols,
                           const int &x,
                           const int &y) {
  if (Navigator::boundaryCheck (x, y, rows, cols)) {
    if (map.at(x).at(y).isSource()) {
      if (Navigator::astarSearch (*this, map, rows, cols, x, y))
        return true; 
    }
  }
  return false;
}

// This template method performs an expanding square search pattern
// and calls the predicate for each position until it returns true
template<typename Predicate>
bool Creature::spiralSearch (const vector<vector<Tile>> &map,
                             const int &rows,
                             const int &cols,
                             Predicate predicate,
                             unsigned maxRadius) {
  // If maxRadius is 0, use sight range as default
  if (maxRadius == 0) {
    maxRadius = getSightRange();
  }
  // Use tile positions for grid-based search (Phase 1: Float Movement)
  int tx = tileX();
  int ty = tileY();
  //  While the search radius does not exceed the available space
  for (int radius = 1; radius < static_cast<int>(maxRadius); radius++) {
    //  Top and Bottom Lines
    for (int xMod = -radius; xMod <= radius; xMod++) {
      int curX = tx + xMod;
      int curY = ty + radius;
      if (predicate(curX, curY)) return true;
      curY = ty - radius;
      if (predicate(curX, curY)) return true;
    }

    //  Right and Left Lines (excluding corners already checked)
    for (int yMod = -radius + 1; yMod <= radius - 1; yMod++) {
      int curX = tx + radius;
      int curY = ty + yMod;
      if (predicate(curX, curY)) return true;
      curX = tx - radius;
      if (predicate(curX, curY)) return true;
    }
  }
  return false;
}

// Unlike spiralSearch, this doesn't stop early - useful for finding closest match
// Visitor receives (x, y) coordinates and returns void
template<typename Visitor>
void Creature::forEachTileInRange(unsigned maxRadius, Visitor visitor) {
  // Use tile positions for grid-based iteration (Phase 1: Float Movement)
  int tx = tileX();
  int ty = tileY();
  for (unsigned radius = 1; radius < maxRadius; radius++) {
    // Top and Bottom edges
    for (int xMod = -static_cast<int>(radius); xMod <= static_cast<int>(radius); xMod++) {
      int curX = tx + xMod;
      // Top edge
      int curY = ty - static_cast<int>(radius);
      visitor(curX, curY);
      // Bottom edge
      curY = ty + static_cast<int>(radius);
      visitor(curX, curY);
    }
    
    // Left and Right edges (excluding corners already visited)
    for (int yMod = -static_cast<int>(radius) + 1; yMod <= static_cast<int>(radius) - 1; yMod++) {
      int curY = ty + yMod;
      // Left edge
      int curX = tx - static_cast<int>(radius);
      visitor(curX, curY);
      // Right edge
      curX = tx + static_cast<int>(radius);
      visitor(curX, curY);
    }
  }
}

//================================================================================
//  Genetics-Based Plant Finding
//================================================================================

/**
 *  Search for nearby genetics-based plants and attempt to eat them.
 *  This method searches the creature's sight range for edible plants
 *  and navigates toward or eats them using the genetics feeding system.
 *
 *  @param world          Reference to the world object.
 *  @param feedingCounter Counter for tracking feeding events.
 *  @return               True if an action was taken (eating or moving toward plant).
 */
bool Creature::findGeneticsPlants(World &world, unsigned &feedingCounter) {
  using namespace EcoSim::Genetics;
  
  // Ensure phenotype is available
  if (!_phenotype) {
    return false;
  }
  
  vector<vector<Tile>> &map = world.getGrid();
  const int rows = world.getRows();
  const int cols = world.getCols();
  
  // Check if we're standing on a tile with edible plants
  // Use tile positions for grid access (Phase 1: Float Movement)
  Tile &currentTile = map[tileX()][tileY()];
  auto &plantsHere = currentTile.getPlants();
  
  for (auto &plantPtr : plantsHere) {
    if (!plantPtr || !plantPtr->isAlive()) {
      continue;
    }
    
    // Check if we can eat this plant (detection + defenses)
    bool canEat = canEatPlant(*plantPtr);
    
    if (canEat) {
      // Attempt to eat the plant
      FeedingResult result = eatPlant(*plantPtr);
      
      if (result.success) {
        setAction(Action::Grazing);
        feedingCounter++;
        
        // Successful feeding significantly increases mate (reduces discomfort)
        // This creates proper survival pressure tied to feeding success
        // Eating should be very rewarding to offset the continuous drain while searching
        _mate += 2.0f;  // Large fixed boost from eating - enough to survive ~200+ ticks
        if (_mate > RESOURCE_LIMIT) _mate = RESOURCE_LIMIT;
        
        // Log the feeding event
        logging::Logger::getInstance().feeding(
          _id, plantPtr->getId(), result.success, result.nutritionGained, result.damageReceived
        );
        
        return true;
      }
    }
  }
  
  // Search nearby tiles for edible plants
  Plant* closestPlant = nullptr;
  int closestX = -1, closestY = -1;
  float closestDistance = getPlantDetectionRange();
  
  unsigned maxRadius = static_cast<unsigned>(getPlantDetectionRange());
  forEachTileInRange(maxRadius, [&](int checkX, int checkY) {
    // Boundary check for valid tile coordinates
    if (!Navigator::boundaryCheck(checkX, checkY, rows, cols)) {
      return;  // Continue to next tile
    }
    
    Tile &tile = map[checkX][checkY];
    auto &plants = tile.getPlants();
    for (auto &plantPtr : plants) {
      if (plantPtr && plantPtr->isAlive() && canEatPlant(*plantPtr)) {
        float distance = calculateDistance(checkX, checkY);
        if (distance < closestDistance) {
          closestDistance = distance;
          closestPlant = plantPtr.get();
          closestX = checkX;
          closestY = checkY;
        }
      }
    }
  });
  
  // If we found a plant, navigate toward it
  if (closestPlant != nullptr && closestX >= 0 && closestY >= 0) {
    // Decrement mate while seeking food (discomfort from hunger)
    _mate -= getComfDec() * 0.5f;  // Less harsh than not finding any food
    return Navigator::astarSearch(*this, map, rows, cols, closestX, closestY);
  }
  
  // Scent-based fallback - try to smell food when vision fails
  // Only try scent detection if creature has meaningful scent ability
  if (hasScentDetection()) {
    int scentTargetX = -1, scentTargetY = -1;
    
    if (findFoodScent(world.getScentLayer(), scentTargetX, scentTargetY)) {
      // Decrement mate while following scent (discomfort from hunger)
      _mate -= getComfDec() * SEEKING_FOOD_MATE_PENALTY;
      return Navigator::astarSearch(*this, map, rows, cols, scentTargetX, scentTargetY);
    }
  }
  
  return false;
}

//================================================================================
//  Scent Detection Helper (Phase 4)
//================================================================================

/**
 * Check if creature has meaningful scent detection capability.
 * Used to determine if scent-based fallback navigation is available.
 * @return True if scent_detection trait exceeds threshold (0.1)
 */
bool Creature::hasScentDetection() const {
  if (!_phenotype) {
    return false;
  }
  
  // Check if scent_detection trait exists, default to 0.0 if not
  float scentDetection = 0.0f;
  if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION)) {
    scentDetection = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION);
  }
  return scentDetection > 0.1f;  // Threshold for meaningful scent ability
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
  if (!_phenotype) {
    return false;
  }
  
  // Get scent detection range from phenotype
  // Uses SCENT_DETECTION_BASE_RANGE and SCENT_DETECTION_ACUITY_MULT constants
  float scentDetection = DEFAULT_OLFACTORY_ACUITY;
  if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION)) {
    scentDetection = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION);
  }
  
  // Calculate detection range: base + scent_detection * multiplier
  int detectionRange = static_cast<int>(SCENT_DETECTION_BASE_RANGE + scentDetection * SCENT_DETECTION_ACUITY_MULT);
  
  // Query the scent layer for strongest FOOD_TRAIL scent in range
  int scentX = tileX(), scentY = tileY();
  EcoSim::ScentDeposit strongestScent = scentLayer.getStrongestScentInRadius(
    tileX(), tileY(), detectionRange,
    EcoSim::ScentType::FOOD_TRAIL,
    scentX, scentY
  );
  
  // Check if we found a valid scent with meaningful intensity
  if (strongestScent.intensity <= 0.0f) {
    return false;
  }
  
  // Check if scent is at our current position (shouldn't navigate to self)
  if (scentX == tileX() && scentY == tileY()) {
    return false;
  }
  
  // TODO: Future enhancement - filter by edibility using scent signature
  // For now, we follow any FOOD_TRAIL scent and verify edibility on arrival
  // The creature will check canEatPlant() when it reaches the food source
  
  // Return the scent coordinates for A* pathfinding
  outX = scentX;
  outY = scentY;
  return true;
}

/**
 *  This method searches for the sight range of the creature for suitable food to
 *  consume. If the creature is already on suitable food it is eaten.
 *
 *  @param map  A reference to the world map.
 *  @param rows Number of rows on the map.
 *  @param cols Number of columns on the map.
 *  @return     Returns true turn has been spent.
 */
bool Creature::findFood (vector<vector<Tile>> &map,
                         const int &rows,
                         const int &cols,
                         unsigned &foodCounter) {
  // LEGACY REMOVAL: Legacy Food system has been removed
  // Creatures now only eat genetics-based Plants via findGeneticsPlants()
  // This stub remains for API compatibility but always returns false
  return false;
}

/**
 *  Simulates a basic line of sight system used for
 *  searching the environment for water.
 *
 *  @param map  A reference to the world map.
 *  @param rows Number of rows on the map.
 *  @param cols Number of columns on the map.
 *  @return     Returns true turn has been spent.
 */
bool Creature::findWater (const vector<vector<Tile>> &map,
                          const int &rows, const int &cols) {
  //  If on water source drink from it (Phase 1: Float Movement - use tile positions)
  if (map.at(tileX()).at(tileY()).isSource()) {
    setAction(Action::Drinking);
    _thirst = RESOURCE_LIMIT;
    return true;
  }

  return spiralSearch(map, rows, cols, [this, &map, rows, cols](int curX, int curY) {
    return waterCheck(map, rows, cols, curX, curY);
  });
}

/**
 *  Has the current agent search for a suitable mate, if found a new offspring 
 *  will be created with the mate.
 *
 *  @param map          A reference to the world map.
 *  @param rows         Number of rows on the map.
 *  @param cols         Number of columns on the map.
 *  @param c            A vector of all creatures.
 *  @param index        The creature currently moving.
 *  @param birthCounter Aids in keeping count of creatures born.
 *  @param return       True if action taken.
 */
bool Creature::findMate (vector<vector<Tile>> &map,
                         const int &rows,
                         const int &cols,
                         vector<Creature> &creatures,
                         const unsigned &index,
                         unsigned &birthCounter) {
  float bestDesirability = -1.0f;
  Creature *bestMate = NULL;

  //  Attempt to find the most desirable mate
  //  GENETICS-MIGRATION: Removed legacy Diet check. Mate compatibility is now
  //  determined by genetic similarity via checkFitness(), not diet type.
  //  This allows creatures with different feeding adaptations to still breed
  //  if they are genetically compatible.
  unsigned sightRange = getSightRange();
  
  for (Creature & creature : creatures) {
    bool isPotentialMate = &creature != this
        && creature.getProfile() == Profile::breed;

    if (isPotentialMate) {
      //  Calculate the distance between the creatures (Phase 1: Float Movement - use tile positions)
      unsigned diffX = abs(tileX() - creature.getX());
      unsigned diffY = abs(tileY() - creature.getY());

      if (diffX < sightRange && diffY < sightRange) {
        float desirability = checkFitness (creature);
        if (desirability > bestDesirability) {
          bestMate = &creature;
          bestDesirability = desirability;
        }
      }
    }
  }

  //  If mate is found seek it
  if (bestMate != NULL) {
    int mateX = bestMate->getX();
    int mateY = bestMate->getY();
    // Phase 1: Float Movement - use tile positions for grid-based distance
    unsigned diffX = abs(tileX() - bestMate->getX());
    unsigned diffY = abs(tileY() - bestMate->getY());

    if (diffX <= 1 && diffY <= 1) {
      setAction(Action::Mating);
      creatures.push_back (breedCreature(*bestMate));
      birthCounter++;
      return true;

    //  Move towards ideal mate
    } else {
      return Navigator::astarSearch (*this, map, rows, cols, mateX, mateY);
    }
  }

  return false;
}


/**
 *  Used by carnivorous creatures to seek out prey.
 *
 *  @param map          A reference to the world map.
 *  @param rows         Number of rows on the map.
 *  @param cols         Number of columns on the map.
 *  @param c            A vector of all creatures.
 *  @param return       True if action taken.
 */
bool Creature::findPrey (vector<vector<Tile>> &map,
                         const int &rows,
                         const int &cols,
                         vector<Creature> &creatures,
                         unsigned &preyAte) {
  float closestDistance = getSightRange();
  vector<Creature>::iterator creature     = creatures.begin();
  vector<Creature>::iterator closestPrey  = creature;

  while (creature != creatures.end()) {
    //  Exclude predators (carnivores and necrovores)
    DietType creatureDiet = creature->getDietType();
    if (creatureDiet != DietType::CARNIVORE && creatureDiet != DietType::NECROVORE) {
      //  Calculate the distance between the creatures
      float distance = calculateDistance(creature->getX(), creature->getY());

      if (distance < closestDistance) {
        closestPrey = creature;
        closestDistance = distance;
      }
    }
    creature++;
  }

  //  If prey is found seek it
  if (closestDistance < getSightRange()) {
    int preyX = closestPrey->getX();
    int preyY = closestPrey->getY();
    // Phase 1: Float Movement - use tile positions for grid-based distance
    unsigned xDist = abs(tileX() - preyX);
    unsigned yDist = abs(tileY() - preyY);

    // If on prey, engage in combat
    if (xDist <= 1 && yDist <= 1) {
      using namespace EcoSim::Genetics;
      
      // Set combat state
      setInCombat(true);
      setTargetId(closestPrey->getId());
      
      // Check cooldown before attacking
      if (_combatCooldown <= 0) {
        setAction(Action::Attacking);
        
        // Use CombatInteraction to resolve attack
        if (_phenotype && closestPrey->getPhenotype()) {
          // Select best attack and resolve it
          CombatAction action = CombatInteraction::selectBestAction(
              *_phenotype, *closestPrey->getPhenotype());
          
          AttackResult result = CombatInteraction::resolveAttack(
              *_phenotype, *closestPrey->getPhenotype(), action);
          
          // Capture health BEFORE applying damage for accurate logging
          float defenderHealthBefore = closestPrey->getHealth();
          
          // Apply damage to prey
          closestPrey->takeDamage(result.finalDamage);
          
          // Log combat attack using detailed combat event
          logging::CombatLogEvent logEvent;
          logEvent.attackerId = _id;
          logEvent.defenderId = closestPrey->getId();
          logEvent.attackerName = generateName();
          logEvent.defenderName = closestPrey->generateName();
          logEvent.weapon = action.weapon;
          logEvent.primaryDamageType = result.primaryType;
          logEvent.rawDamage = result.rawDamage;
          logEvent.finalDamage = result.finalDamage;
          logEvent.effectivenessMultiplier = result.effectivenessMultiplier;
          logEvent.defenseUsed = CombatInteraction::getCounteringDefense(result.primaryType);
          logEvent.defenseValue = CombatInteraction::getDefenseProfile(*closestPrey->getPhenotype())
                                  .getDefenseForType(logEvent.defenseUsed);
          logEvent.attackerHealthBefore = getHealth();
          logEvent.attackerHealthAfter = getHealth();
          logEvent.attackerMaxHealth = getMaxHealth();
          logEvent.defenderHealthBefore = defenderHealthBefore;  // Use captured value
          logEvent.defenderHealthAfter = closestPrey->getHealth();
          logEvent.defenderMaxHealth = closestPrey->getMaxHealth();
          logEvent.hit = result.hit;
          logEvent.causedBleeding = result.causedBleeding;
          logEvent.defenderDied = (closestPrey->getHealth() <= 0.0f);
          
          logging::Logger::getInstance().combatEvent(logEvent);
          
          // Set cooldown based on weapon type (use weapon's base cooldown)
          _combatCooldown = getWeaponStats(action.weapon).baseCooldown;
        } else {
          // Fallback: basic damage without genetics
          closestPrey->takeDamage(5.0f);
          _combatCooldown = 3;
        }
        
        // Check if prey died
        if (closestPrey->getHealth() <= 0.0f) {
          // Prey killed - gain calories
          float calories = PREY_CALORIES + closestPrey->getHunger();
          _hunger += calories;
          if (_hunger > RESOURCE_LIMIT) _hunger = RESOURCE_LIMIT;
          
          // Log the creature death event (before erasing from vector)
          logging::Logger::getInstance().creatureDied(
            closestPrey->getId(),
            closestPrey->generateName(),
            "combat",
            closestPrey->getHunger(),
            static_cast<int>(closestPrey->getAge())
          );
          
          // Log the feeding event (carnivore eating prey)
          logging::Logger::getInstance().feeding(
            _id,                    // predator ID
            closestPrey->getId(),   // prey ID (used in place of plant ID)
            true,                   // success
            calories,               // nutrition gained
            0.0f                    // damage received (prey is already dead)
          );
          
          creatures.erase(closestPrey);
          preyAte++;
          
          // Clear combat state
          setInCombat(false);
          setTargetId(-1);
        }
        // Else: prey survives, combat continues next tick
      } else {
        // On cooldown - decrement and wait
        _combatCooldown--;
        setAction(Action::Attacking);  // Still in combat stance
      }
      
      return true;

    //  Move towards prey
    } else {
      setAction(Action::Chasing);
      return Navigator::astarSearch (*this, map, rows, cols, preyX, preyY);
    }
  }

  return false;
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
  _hunger -= _metabolism * distance;
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
  float similarity = 0.5f;  // Default moderate similarity
  if (_genome && c2.getGenome()) {
    // Use Genome's built-in compare() method
    similarity = _genome->compare(*c2.getGenome());
  }

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
  _hunger -= Creature::BREED_COST; _thirst -= Creature::BREED_COST;
  mate.setHunger (mate.getHunger() - Creature::BREED_COST);
  mate.setThirst (mate.getThirst() - Creature::BREED_COST);

  //  Give the offspring a quarter of each parents resources
  float hunger = shareFood(RESOURCE_SHARED)  + mate.shareFood(RESOURCE_SHARED);
  float thirst = shareWater(RESOURCE_SHARED) + mate.shareWater(RESOURCE_SHARED);
  //  Erroneous value check to be safe
  if (hunger > RESOURCE_LIMIT) hunger = RESOURCE_LIMIT;
  if (thirst > RESOURCE_LIMIT) thirst = RESOURCE_LIMIT;

  //  Reset the parents mating levels
  _mate = 0.0f; mate.setMate (0.0f);

  // Create offspring genome using Genome::crossover()
  std::unique_ptr<EcoSim::Genetics::Genome> offspringGenome;
  
  if (_genome && mate.getGenome()) {
    // Use Genome's built-in crossover method (recombination_rate = 0.5f)
    EcoSim::Genetics::Genome crossed = EcoSim::Genetics::Genome::crossover(*_genome, *mate.getGenome(), 0.5f);
    
    // Apply mutation (5% rate)
    crossed.mutate(0.05f, getGeneRegistry().getAllDefinitions());
    
    offspringGenome = std::make_unique<EcoSim::Genetics::Genome>(std::move(crossed));
  } else if (_genome) {
    // Clone my genome if mate has none
    offspringGenome = std::make_unique<EcoSim::Genetics::Genome>(*_genome);
  } else {
    // Neither parent has genome - create default
    offspringGenome = std::make_unique<EcoSim::Genetics::Genome>();
  }

  Creature offspring(tileX(), tileY(), hunger, thirst, std::move(offspringGenome));
  
  // Log the birth event
  logging::Logger::getInstance().creatureBorn(
    offspring.getId(),
    offspring.generateName(),
    _id,
    mate.getId()
  );
  
  return offspring;
}

//================================================================================
//  Sensory System Methods (Phase 1)
//================================================================================

/**
 * Compute this creature's unique genetic scent signature.
 * Creates an 8-float fingerprint based on genome characteristics.
 */
std::array<float, 8> Creature::computeScentSignature() const {
  std::array<float, 8> signature;
  
  // Use genome traits to create a unique fingerprint
  // This allows related creatures to have similar signatures
  
  // Component 0: Diet-based (0.0-0.2 range based on diet type)
  signature[0] = static_cast<float>(static_cast<int>(getDietType())) * 0.2f;
  
  // Component 1: Lifespan-based (normalized)
  signature[1] = std::min(1.0f, static_cast<float>(getLifespan()) / 1000000.0f);
  
  // Component 2: Sight range (normalized)
  signature[2] = std::min(1.0f, static_cast<float>(getSightRange()) / 200.0f);
  
  // Component 3: Metabolism-based (normalized)
  signature[3] = std::min(1.0f, getMetabolism() * 100.0f);
  
  // Component 4: Social behavior (flee/pursue ratio)
  float fleeVal = static_cast<float>(getFlee());
  float pursueVal = static_cast<float>(getPursue());
  signature[4] = (pursueVal > 0) ? std::min(1.0f, fleeVal / pursueVal) : 0.5f;
  
  // Component 5: Flocking tendency
  signature[5] = ifFlocks() ? 0.8f : 0.2f;
  
  // Components 6-7: Use olfactory genes if available
  if (_phenotype) {
    signature[6] = _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_SIGNATURE_VARIANCE) ?
                   _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::SCENT_SIGNATURE_VARIANCE) : 0.5f;
    signature[7] = _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION) ?
                   _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION) : 0.5f;
  } else {
    // Derive from other traits for legacy creatures
    signature[6] = std::min(1.0f, getTHunger() + getTThirst());
    signature[7] = std::min(1.0f, getTFatigue() + getTMate());
  }
  
  return signature;
}

/**
 * Deposit breeding pheromone when in breeding state.
 * Creates a MATE_SEEKING scent deposit based on creature's olfactory genes.
 */
void Creature::depositBreedingScent(EcoSim::ScentLayer& layer, unsigned int currentTick) {
  // Only deposit if in breeding profile
  if (_profile != Profile::breed) {
    return;
  }
  
  // Get scent production rate from phenotype
  // Uses DEFAULT_SCENT_PRODUCTION constant
  float scentProduction = DEFAULT_SCENT_PRODUCTION;
  float scentMasking = 0.0f;     // Default - no masking
  
  if (_phenotype) {
    if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION)) {
      scentProduction = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION);
    }
    if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_MASKING)) {
      scentMasking = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::SCENT_MASKING);
    }
  }
  
  // Apply scent masking - high masking reduces effective production
  float effectiveProduction = scentProduction * (1.0f - scentMasking * 0.8f);
  
  // Only deposit if effective production is significant
  if (effectiveProduction < 0.05f) {
    return;
  }
  
  // Create scent deposit
  EcoSim::ScentDeposit deposit;
  deposit.type = EcoSim::ScentType::MATE_SEEKING;
  deposit.creatureId = _id;
  deposit.intensity = effectiveProduction;
  deposit.signature = computeScentSignature();
  deposit.tickDeposited = currentTick;
  
  // Decay rate based on intensity - stronger scents last longer
  // Base decay: 50-200 ticks depending on intensity
  deposit.decayRate = static_cast<unsigned int>(50 + effectiveProduction * 150);
  
  // Deposit at creature's current position
  layer.deposit(tileX(), tileY(), deposit);
}

//================================================================================
//  Scent Detection Methods (Phase 2: Gradient Navigation)
//================================================================================

/**
 * Calculate genetic similarity between two scent signatures.
 * Uses cosine similarity for comparing 8-dimensional signature vectors.
 */
float Creature::calculateSignatureSimilarity(
    const std::array<float, 8>& sig1,
    const std::array<float, 8>& sig2) {
  // Calculate cosine similarity
  float dotProduct = 0.0f;
  float norm1 = 0.0f;
  float norm2 = 0.0f;
  
  for (size_t i = 0; i < 8; ++i) {
    dotProduct += sig1[i] * sig2[i];
    norm1 += sig1[i] * sig1[i];
    norm2 += sig2[i] * sig2[i];
  }
  
  if (norm1 == 0.0f || norm2 == 0.0f) {
    return 0.0f;
  }
  
  return dotProduct / (std::sqrt(norm1) * std::sqrt(norm2));
}

/**
 * Detect the direction to a potential mate using scent trails.
 * Uses OLFACTORY_ACUITY gene to determine detection range.
 * Filters by genetic similarity to ensure same-species recognition.
 */
std::optional<Direction> Creature::detectMateDirection(const EcoSim::ScentLayer& scentLayer) const {
  // Only detect if we're in breeding mode
  if (_profile != Profile::breed) {
    return std::nullopt;
  }
  
  // Get olfactory acuity from phenotype (default to moderate if not available)
  // Uses DEFAULT_OLFACTORY_ACUITY constant
  float olfactoryAcuity = DEFAULT_OLFACTORY_ACUITY;
  if (_phenotype) {
    if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY)) {
      olfactoryAcuity = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY);
    }
  }
  
  // Calculate detection range: base + acuity * multiplier
  // Range: SCENT_DETECTION_BASE_RANGE (acuity=0) to BASE+MULT (acuity=1)
  // This allows creatures to detect mates at typical population distances (avg ~116 tiles)
  // Uses SCENT_DETECTION_BASE_RANGE and SCENT_DETECTION_ACUITY_MULT constants
  int detectionRange = static_cast<int>(SCENT_DETECTION_BASE_RANGE + olfactoryAcuity * SCENT_DETECTION_ACUITY_MULT);
  
  // Query the scent layer for strongest MATE_SEEKING scent in range
  int scentX = tileX(), scentY = tileY();
  EcoSim::ScentDeposit strongestScent = scentLayer.getStrongestScentInRadius(
    tileX(), tileY(), detectionRange,
    EcoSim::ScentType::MATE_SEEKING,
    scentX, scentY
  );
  
  // Check if we found a valid scent
  if (strongestScent.intensity <= 0.0f || strongestScent.creatureId == _id) {
    // No scent found, or it's our own scent
    return std::nullopt;
  }
  
  // No genetic similarity filter - any MATE_SEEKING scent from another creature is valid
  // The findMate() method already handles compatibility checks when creatures meet
  
  // Check if scent is at our current position (we've arrived!)
  if (scentX == tileX() && scentY == tileY()) {
    return Direction::none;  // At the scent source
  }
  
  // Calculate direction toward scent source
  int dx = scentX - tileX();
  int dy = scentY - tileY();
  
  // Convert delta to one of 8 directions
  Direction result = Direction::none;
  
  if (dx > 0 && dy > 0) {
    result = Direction::SE;
  } else if (dx > 0 && dy < 0) {
    result = Direction::NE;
  } else if (dx > 0 && dy == 0) {
    result = Direction::E;
  } else if (dx < 0 && dy > 0) {
    result = Direction::SW;
  } else if (dx < 0 && dy < 0) {
    result = Direction::NW;
  } else if (dx < 0 && dy == 0) {
    result = Direction::W;
  } else if (dx == 0 && dy > 0) {
    result = Direction::S;
  } else if (dx == 0 && dy < 0) {
    result = Direction::N;
  }
  
  return result;
}

/**
 * Find the coordinates of the strongest mate scent in range.
 * Returns target coordinates for A* pathfinding navigation.
 *
 * Mate compatibility is determined by genetic similarity in checkFitness() 
 * when creatures actually meet. Creatures follow any breeding pheromone to 
 * increase chances of finding compatible mates.
 */
bool Creature::findMateScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const {
  // Only detect if we're in breeding mode
  if (_profile != Profile::breed) {
    return false;
  }
  
  // Get olfactory acuity from phenotype (default to moderate if not available)
  float olfactoryAcuity = DEFAULT_OLFACTORY_ACUITY;
  if (_phenotype) {
    if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY)) {
      olfactoryAcuity = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY);
    }
  }
  
  // Calculate detection range: base + acuity * multiplier
  int detectionRange = static_cast<int>(
    SCENT_DETECTION_BASE_RANGE + olfactoryAcuity * SCENT_DETECTION_ACUITY_MULT
  );
  
  // Query the scent layer for strongest MATE_SEEKING scent in range
  int scentX = tileX(), scentY = tileY();
  EcoSim::ScentDeposit strongestScent = scentLayer.getStrongestScentInRadius(
    tileX(), tileY(), detectionRange,
    EcoSim::ScentType::MATE_SEEKING,
    scentX, scentY
  );
  
  // Check if we found a valid scent (not our own)
  if (strongestScent.intensity <= 0.0f || strongestScent.creatureId == _id) {
    return false;
  }
  
  // Check if scent is at our current position (shouldn't navigate to self)
  if (scentX == tileX() && scentY == tileY()) {
    return false;
  }
  
  // Return the scent coordinates for A* pathfinding
  outX = scentX;
  outY = scentY;
  return true;
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
	*/
EcoSim::Genetics::FeedingResult Creature::eatPlant(EcoSim::Genetics::Plant& plant) {
	   using namespace EcoSim::Genetics;
	   
	   // Ensure interaction systems are initialized
	   if (!s_feedingInteraction) {
	       initializeInteractionSystems();
	   }
	   
	   // Need new genetics system enabled for plant interactions
	   if (!_phenotype) {
	       FeedingResult result;
	       result.success = false;
	       result.description = "New genetics system not enabled";
	       return result;
	   }
	   
	   // Calculate hunger level (0-1, higher = more desperate)
	   float hungerLevel = 1.0f - std::max(0.0f, std::min(1.0f, _hunger / RESOURCE_LIMIT));
	   
	   // Attempt the feeding interaction
	   FeedingResult result = s_feedingInteraction->attemptToEatPlant(
	       *_phenotype, plant, hungerLevel
	   );
	   
	   if (result.success) {
	       // Apply nutrition gained
	       _hunger += result.nutritionGained;
	       if (_hunger > RESOURCE_LIMIT) {
	           _hunger = RESOURCE_LIMIT;
	       }
	       
	       // Apply damage from plant defenses
	       // For now, damage reduces hunger (representing injury cost)
	       float totalDamage = result.damageReceived;
	       _hunger -= totalDamage * 0.5f;  // Damage costs energy to heal
	       
	       // Apply damage to the plant
	       plant.takeDamage(result.plantDamage);
	       
	       // Handle seed consumption
	       if (result.seedsConsumed && result.seedsToDisperse > 0) {
	           consumeSeeds(plant, static_cast<int>(result.seedsToDisperse),
	                       1.0f - (result.seedsDestroyed ? 0.5f : 0.0f));
	       }
	   }
	   
	   return result;
}

/**
	* Check if creature can eat the given plant.
	* When called for a plant on the same tile, assume distance = 0.
	*/
bool Creature::canEatPlant(const EcoSim::Genetics::Plant& plant) const {
	   using namespace EcoSim::Genetics;
	   
	   if (!s_feedingInteraction) {
	       const_cast<Creature*>(this)->initializeInteractionSystems();
	   }
	   
	   if (!_phenotype) {
	       return false;
	   }
	   
	   // For plants on the same tile as creature, use distance 0
	   // Plant.getX()/getY() may not be set correctly for tile-stored plants
	   // The calling code in findGeneticsPlants already verifies the plant is reachable
	   float distance = 0.0f;
	   if (plant.getX() >= 0 && plant.getY() >= 0) {
	       // Only calculate distance if plant has valid coordinates
	       distance = calculateDistance(plant.getX(), plant.getY());
	   }
	   
	   // Detection check - with distance 0, should always pass
	   bool canDetect = s_feedingInteraction->canDetectPlant(*_phenotype, plant, distance);
	   if (!canDetect) {
	       return false;
	   }
	   
	   return s_feedingInteraction->canOvercomeDefenses(*_phenotype, plant);
}

/**
	* Get maximum plant detection range based on creature's senses.
	*/
float Creature::getPlantDetectionRange() const {
	   using namespace EcoSim::Genetics;
	   
	   if (!s_feedingInteraction) {
	       const_cast<Creature*>(this)->initializeInteractionSystems();
	   }
	   
	   if (!_phenotype) {
	       return static_cast<float>(getSightRange());
	   }
	   
	   // Use the feeding interaction's detection range calculation
	   // For a "generic" plant, use default values
	   float colorVision = _phenotype->hasTrait(UniversalGenes::COLOR_VISION) ?
	                       _phenotype->getTrait(UniversalGenes::COLOR_VISION) : 0.5f;
	   float scentDetection = _phenotype->hasTrait(UniversalGenes::SCENT_DETECTION) ?
	                          _phenotype->getTrait(UniversalGenes::SCENT_DETECTION) : 0.5f;
	   
	   // Base range from sight, enhanced by color vision and scent
	   float baseRange = static_cast<float>(getSightRange());
	   float enhancement = 1.0f + (colorVision * 0.3f) + (scentDetection * 0.5f);
	   
	   return baseRange * enhancement;
}

/**
	* Attach a burr from a plant to this creature.
	*/
void Creature::attachBurr(const EcoSim::Genetics::Plant& plant) {
	   using namespace EcoSim::Genetics;
	   
	   if (!s_seedDispersal) {
	       initializeInteractionSystems();
	   }
	   
	   if (!_phenotype) {
	       return;
	   }
	   
	   // Check if burr will attach based on plant hook strength and creature fur
	   if (s_seedDispersal->willBurrAttach(plant, *_phenotype)) {
	       // Store burr info: (dispersal strategy as int, originX, originY, ticksAttached)
	       _attachedBurrs.push_back(std::make_tuple(
	           static_cast<int>(plant.getPrimaryDispersalStrategy()),
	           plant.getX(),
	           plant.getY(),
	           0  // Just attached
	       ));
	   }
}

/**
	* Process burr detachment and return dispersal events.
	*/
std::vector<EcoSim::Genetics::DispersalEvent> Creature::detachBurrs() {
	   using namespace EcoSim::Genetics;
	   
	   std::vector<DispersalEvent> events;
	   
	   if (!s_seedDispersal || !_phenotype) {
	       return events;
	   }
	   
	   auto it = _attachedBurrs.begin();
	   while (it != _attachedBurrs.end()) {
	       int ticksAttached = std::get<3>(*it);
	       
	       // Check if burr detaches
	       if (s_seedDispersal->willBurrDetach(*_phenotype, ticksAttached)) {
	           // Create dispersal event at current creature location
	           DispersalEvent event;
	           event.originX = std::get<1>(*it);
	           event.originY = std::get<2>(*it);
	           event.targetX = tileX();
	           event.targetY = tileY();
	           event.method = static_cast<DispersalStrategy>(std::get<0>(*it));
	           event.disperserInfo = "creature_burr_detach";
	           event.seedViability = 0.85f;  // Good viability for burr dispersal
	           
	           events.push_back(event);
	           it = _attachedBurrs.erase(it);
	       } else {
	           // Increment ticks attached
	           std::get<3>(*it) = ticksAttached + 1;
	           ++it;
	       }
	   }
	   
	   return events;
}

/**
	* Check if creature has burrs attached.
	*/
bool Creature::hasBurrs() const {
	   return !_attachedBurrs.empty();
}

/**
	* Get pending dispersal events from attached burrs.
	*/
std::vector<EcoSim::Genetics::DispersalEvent> Creature::getPendingBurrDispersal() const {
	   using namespace EcoSim::Genetics;
	   
	   std::vector<DispersalEvent> events;
	   
	   for (const auto& burr : _attachedBurrs) {
	       DispersalEvent event;
	       event.originX = std::get<1>(burr);
	       event.originY = std::get<2>(burr);
	       event.targetX = tileX();  // Current creature position
	       event.targetY = tileY();
	       event.method = static_cast<DispersalStrategy>(std::get<0>(burr));
	       event.disperserInfo = "creature_burr_pending";
	       event.seedViability = 0.85f;
	       
	       events.push_back(event);
	   }
	   
	   return events;
}

/**
	* Add seeds to gut for digestion and dispersal.
	*/
void Creature::consumeSeeds(const EcoSim::Genetics::Plant& plant, int count, float viability) {
	   if (!_phenotype) {
	       return;
	   }
	   
	   // Calculate gut transit time from phenotype
	   float gutTransit = _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::GUT_TRANSIT_TIME) ?
	                      _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::GUT_TRANSIT_TIME) : 6.0f;
	   
	   // Convert hours to ticks (assuming ~10 ticks per hour)
	   int transitTicks = static_cast<int>(gutTransit * 10.0f);
	   
	   // Encode origin position as single int for storage
	   int encodedOrigin = plant.getX() * 10000 + plant.getY();
	   
	   // Add each seed to gut
	   for (int i = 0; i < count; ++i) {
	       _gutSeeds.push_back(std::make_tuple(encodedOrigin, viability, transitTicks));
	   }
}

/**
	* Process gut seed passage and return dispersal events.
	*/
std::vector<EcoSim::Genetics::DispersalEvent> Creature::processGutSeeds(int ticksElapsed) {
	   using namespace EcoSim::Genetics;
	   
	   std::vector<DispersalEvent> events;
	   
	   auto it = _gutSeeds.begin();
	   while (it != _gutSeeds.end()) {
	       int ticksRemaining = std::get<2>(*it) - ticksElapsed;
	       
	       if (ticksRemaining <= 0) {
	           // Seed is ready to be dispersed
	           int encodedOrigin = std::get<0>(*it);
	           float viability = std::get<1>(*it);
	           
	           // Decode origin position
	           int originX = encodedOrigin / 10000;
	           int originY = encodedOrigin % 10000;
	           
	           // Create dispersal event at current creature location
	           DispersalEvent event;
	           event.originX = originX;
	           event.originY = originY;
	           event.targetX = tileX();
	           event.targetY = tileY();
	           event.method = DispersalStrategy::ANIMAL_FRUIT;
	           event.disperserInfo = "creature_gut_passage";
	           
	           // Viability affected by gut passage
	           // Optimal transit time (4-12 hours) can improve viability through scarification
	           float transitHours = (_phenotype && _phenotype->hasTrait(UniversalGenes::GUT_TRANSIT_TIME)) ?
	                                _phenotype->getTrait(UniversalGenes::GUT_TRANSIT_TIME) : 6.0f;
	           
	           if (transitHours >= 4.0f && transitHours <= 12.0f) {
	               viability = std::min(1.0f, viability * 1.15f);  // Scarification bonus
	           } else if (transitHours > 12.0f) {
	               viability *= 0.9f;  // Acid damage
	           }
	           
	           event.seedViability = viability;
	           events.push_back(event);
	           
	           it = _gutSeeds.erase(it);
	       } else {
	           // Update remaining time
	           std::get<2>(*it) = ticksRemaining;
	           ++it;
	       }
	   }
	   
	   return events;
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
Profile Creature::stringToProfile (const string &str) {
  static const std::unordered_map<string, Profile> profileMap = {
    {"hungry",  Profile::hungry},
    {"thirsty", Profile::thirsty},
    {"sleep",   Profile::sleep},
    {"breed",   Profile::breed},
    {"migrate", Profile::migrate}
  };

  auto it = profileMap.find(str);
  return (it != profileMap.end()) ? it->second : Profile::migrate;
}

Direction Creature::stringToDirection (const string &str) {
  static const std::unordered_map<string, Direction> directionMap = {
    {"SE",   Direction::SE},
    {"NE",   Direction::NE},
    {"E",    Direction::E},
    {"SW",   Direction::SW},
    {"NW",   Direction::NW},
    {"W",    Direction::W},
    {"S",    Direction::S},
    {"N",    Direction::N},
    {"none", Direction::none}
  };

  auto it = directionMap.find(str);
  return (it != directionMap.end()) ? it->second : Direction::none;
}

/**
 *  This method converts the profile to a human readable string.
 *
 *  @return A human readable string representation of the profile.
 */
string Creature::profileToString () const {
  switch(_profile) {
    case Profile::hungry:   return "hungry";
    case Profile::thirsty:  return "thirsty";
    case Profile::sleep:    return "sleep";
    case Profile::breed:    return "breed";
    case Profile::migrate:  return "migrate";
    default:                return "error";
  }
}

/**
 *  This method converts the direction to a human readable string.
 *
 *  @return A human readable string representation of the profile.
 */  
string Creature::directionToString () const {
  switch (_direction) {
    case Direction::SE:   return "SE";
    case Direction::NE:   return "NE";
    case Direction::E:    return "E";
    case Direction::SW:   return "SW";
    case Direction::NW:   return "NW";
    case Direction::W:    return "W";
    case Direction::S:    return "S";
    case Direction::N:    return "N";
    case Direction::none: return "none";
    default:              return "error";
  }
}

/**
 *  This converts the creature objects member variables to a string that can
 *  be read by a user or saved to a file.
 *
 *  @return A string representation of the creature.
 */
string Creature::toString () const {
  ostringstream ss;
  ss  << this->GameObject::toString ()  << ","
      << tileX() << "," << tileY() << "," << _age << ","
      << directionToString ()           << ","
      << profileToString ()             << ","
      << _hunger      << ","
      << _thirst      << ","
      << _fatigue     << ","
      << _mate        << ","
      << _metabolism  << ","
      << _speed;

  return ss.str();
}

//================================================================================
//  New Genetics System Getters
//================================================================================

/**
 * Get the new genetics genome pointer.
 * Returns nullptr if new genetics system is not enabled.
 */
const EcoSim::Genetics::Genome* Creature::getGenome() const {
    return _genome.get();
}

/**
 * Get the mutable new genetics genome pointer.
 * Returns nullptr if new genetics system is not enabled.
 */
EcoSim::Genetics::Genome* Creature::getGenomeMutable() {
    return _genome.get();
}

/**
 * Get the scientific name for this creature.
 * Uses CreatureTaxonomy for dynamic scientific name generation.
 */
std::string Creature::getScientificName() const {
    // Use CreatureTaxonomy for dynamic scientific name generation
    if (_genome) {
        return EcoSim::Genetics::CreatureTaxonomy::generateScientificName(*_genome);
    }
    // Fallback to stored name
    return _name;
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

//================================================================================
//  Health System
//================================================================================

/**
 * Get current health value.
 * Now uses the actual _health member for combat system.
 * Clamped to maxHealth to handle cases where max health decreased after initialization.
 */
float Creature::getHealth() const {
    return _health;
}

/**
 * Get maximum health value.
 * Based on MAX_SIZE gene for creature mass scaling.
 */
float Creature::getMaxHealth() const {
    if (_phenotype && _phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE)) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE) * 10.0f;
    }
    return RESOURCE_LIMIT * 10.0f;  // 100.0f default
}

/**
 * Apply damage to creature's health.
 */
void Creature::takeDamage(float amount) {
    if (amount <= 0.0f) return;
    _health = std::max(0.0f, _health - amount);
}

/**
 * Heal creature's health.
 */
void Creature::heal(float amount) {
    if (amount <= 0.0f) return;
    _health = std::min(getMaxHealth(), _health + amount);
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
    if (_phenotype) {
        return _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::REGENERATION_RATE) * 0.001f;
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
    switch (_profile) {
        case Profile::hungry:   return Motivation::Hungry;
        case Profile::thirsty:  return Motivation::Thirsty;
        case Profile::breed:    return Motivation::Amorous;
        case Profile::sleep:    return Motivation::Tired;
        case Profile::migrate:
        default:                return Motivation::Content;
    }
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
    if (_phenotype && _phenotype->hasTrait(geneId)) {
        return _phenotype->getTrait(geneId);
    }
    return 0.0f;
}

//================================================================================
//  New Motivation System Behavior Wrappers
//================================================================================

/**
 * Hungry behavior - wrapper for hungryProfile.
 * Part of the new Motivation/Action system that delegates to legacy Profile implementation.
 */
void Creature::hungryBehavior(World &world,
                              std::vector<Creature> &creatures,
                              const unsigned index,
                              GeneralStats &gs) {
    hungryProfile(world, creatures, index, gs);
}

/**
 * Thirsty behavior - wrapper for thirstyProfile.
 */
void Creature::thirstyBehavior(World &world,
                               std::vector<Creature> &creatures,
                               const unsigned index) {
    thirstyProfile(world, creatures, index);
}

/**
 * Amorous behavior - wrapper for breedProfile.
 */
void Creature::amorousBehavior(World &world,
                               std::vector<Creature> &creatures,
                               const unsigned index,
                               GeneralStats &gs) {
    breedProfile(world, creatures, index, gs);
}

/**
 * Content behavior - wrapper for migrateProfile.
 */
void Creature::contentBehavior(World &world,
                               std::vector<Creature> &creatures,
                               const unsigned index) {
    setAction(Action::Idle);
    migrateProfile(world, creatures, index);
}

/**
 * Tired behavior - sleep is passive, handled by update().
 * This just does a simple wander while resting.
 */
void Creature::tiredBehavior(World &world,
                             std::vector<Creature> &creatures,
                             const unsigned index) {
    setAction(Action::Resting);
    // Sleep is mostly passive (metabolism changes happen in update())
    // When tired, just rest in place or move minimally
    Navigator::wander(*this, world.getGrid(), world.getRows(), world.getCols());
}

//================================================================================
//  New Genetics Constructors
//================================================================================

/**
 * Constructor with new genetics genome only.
 */
Creature::Creature(int x, int y, std::unique_ptr<EcoSim::Genetics::Genome> genome)
    : _worldX(static_cast<float>(x)),
      _worldY(static_cast<float>(y)),
      _age(0),
      _id(s_nextCreatureId++),
      _direction(Direction::none),
      _profile(Profile::migrate),
      _motivation(Motivation::Content),
      _action(Action::Idle),
      _hunger(1.0f),
      _thirst(1.0f),
      _fatigue(INIT_FATIGUE),
      _mate(0.0f),
      _speed(1),
      _genome(std::move(genome)) {
    
    // Initialize phenotype from genome
    _phenotype = std::make_unique<EcoSim::Genetics::Phenotype>(
        _genome.get(), &getGeneRegistry()
    );
    
    // Set metabolism from phenotype
    if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE)) {
        _metabolism = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE) * 0.001f;
    } else {
        _metabolism = 0.001f;
    }
    
    // Classify archetype from genome (after phenotype is ready)
    // Note: Population increment happens here, decrement in destructor
    if (_genome) {
        _archetype = EcoSim::Genetics::CreatureTaxonomy::classifyArchetype(*_genome);
            if (_archetype) {
                _archetype->incrementPopulation();
            }
        }
        
        _character = generateChar();
        _name = generateName();
        
        // Initialize health to max health (phenotype must be ready)
        _health = getMaxHealth();
        
        // Initialize phenotype context
        EcoSim::Genetics::EnvironmentState defaultEnv;
        updatePhenotypeContext(defaultEnv);
    }

/**
 * Constructor with new genetics genome and initial hunger/thirst values.
 */
Creature::Creature(int x, int y, float hunger, float thirst,
                   std::unique_ptr<EcoSim::Genetics::Genome> genome)
    : _worldX(static_cast<float>(x)),
      _worldY(static_cast<float>(y)),
      _age(0),
      _id(s_nextCreatureId++),
      _direction(Direction::none),
      _profile(Profile::migrate),
      _motivation(Motivation::Content),
      _action(Action::Idle),
      _hunger(hunger),
      _thirst(thirst),
      _fatigue(INIT_FATIGUE),
      _mate(0.0f),
      _speed(1),
      _genome(std::move(genome)) {
    
    // Initialize phenotype from genome
    _phenotype = std::make_unique<EcoSim::Genetics::Phenotype>(
        _genome.get(), &getGeneRegistry()
    );
    
    // Set metabolism from phenotype
    if (_phenotype->hasTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE)) {
        _metabolism = _phenotype->getTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE) * 0.001f;
    } else {
        _metabolism = 0.001f;
    }
    
    // Classify archetype from genome (after phenotype is ready)
    // Note: Population increment happens here, decrement in destructor
    if (_genome) {
        _archetype = EcoSim::Genetics::CreatureTaxonomy::classifyArchetype(*_genome);
        if (_archetype) {
            _archetype->incrementPopulation();
        }
    }
    
    _character = generateChar();
    _name = generateName();
    
    // Initialize health to max health (phenotype must be ready)
    _health = getMaxHealth();
    
    // Initialize phenotype context
    EcoSim::Genetics::EnvironmentState defaultEnv;
    updatePhenotypeContext(defaultEnv);
}
