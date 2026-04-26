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
#include "genetics/organisms/OrganismFactory.hpp"
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
#include "genetics/behaviors/BehaviorRegistry.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/behaviors/FeedingBehavior.hpp"
#include "genetics/behaviors/HuntingBehavior.hpp"
#include "genetics/behaviors/MatingBehavior.hpp"
#include "genetics/behaviors/MovementBehavior.hpp"
#include "genetics/behaviors/RestBehavior.hpp"
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
//  Shared static services (s_geneRegistry, s_feedingInteraction,
//  s_seedDispersal, s_perceptionSystem, s_combatInteraction) and the
//  nextCreatureId_ counter are defined in src/genetics/organisms/Organism.cpp.
//================================================================================
static int s_nextCreatureId = 1;

void EcoSim::Genetics::Organism::resetIdCounter(int nextId) {
    s_nextCreatureId = nextId;
}

int EcoSim::Genetics::Organism::getNextId() {
    return s_nextCreatureId;
}

//================================================================================
//  Constants — definitions now live inline in
//  include/genetics/core/OrganismConstants.hpp. Creature::FOO aliases
//  re-expose them so existing callers compile unchanged.
//================================================================================


//================================================================================
//  Copy/Move Constructors and Assignment Operators (M5)
//================================================================================
/**
 * Copy constructor - performs deep copy of unique_ptr members
 */
Creature::Creature(const Creature& other)
    : Organism(other.x_, other.y_, other.genome_, getGeneRegistry()) {
    motivation_ = other.motivation_;
    action_ = other.action_;
    // Copy Organism state that isn't set in constructor
    age_ = other.age_;
    health_ = other.health_;
    currentSize_ = other.currentSize_;
    maxSize_ = other.maxSize_;
    mature_ = other.mature_;

    // Deep-copy components (unique_ptr can't be copied, Organism copy is disabled)
    if (other.mobility_) {
        attachMobility(std::make_unique<EcoSim::Genetics::MobilityComponent>(*other.mobility_));
    }
    if (other.heterotrophy_) {
        attachHeterotrophy(std::make_unique<EcoSim::Genetics::HeterotrophyComponent>(*other.heterotrophy_));
    }
    if (other.reproduction_) {
        attachReproduction(std::make_unique<EcoSim::Genetics::ReproductionComponent>(*other.reproduction_));
    }
    if (other.combat_) {
        attachCombat(std::make_unique<EcoSim::Genetics::CombatComponent>(*other.combat_));
    }
    if (other.thermal_) {
        attachThermal(std::make_unique<EcoSim::Genetics::ThermalComponent>(*other.thermal_));
    }
    // Identity: new copy gets a fresh sequentialId but inherits archetype/biome
    attachIdentity(std::make_unique<EcoSim::Genetics::IdentityComponent>());
    identity_->sequentialId = nextCreatureId_++;
    if (other.identity_) {
        identity_->archetype       = other.identity_->archetype;
        identity_->biomeAdaptation = other.identity_->biomeAdaptation;
    }

    // Increment archetype population for the copy
    if (identity_->archetype) {
        identity_->archetype->incrementPopulation();
    }
    // Increment biome adaptation population for the copy
    if (identity_->biomeAdaptation) {
        identity_->biomeAdaptation->incrementPopulation();
    }

    // Behavior controller is NOT copied - use lazy initialization
    organismBehaviorController_ = nullptr;
}

/**
 * Move constructor
 */
// Move constructor and move assignment are Organism's — defaulted on Creature.
// Organism's move operators handle motivation/action, all components (including
// identity with its archetype/biome flyweight pointers), BehaviorController,
// and pending offspring. Organism::operator= also decrements the outgoing
// archetype/biome populations before the move.
Creature::Creature(Creature&& other) noexcept = default;
Creature& Creature::operator=(Creature&& other) noexcept = default;

// Destructor is Organism's — ~Organism decrements archetype/biome
// populations for any organism holding an identity component.

//================================================================================
//  Setters
//================================================================================
void EcoSim::Genetics::Organism::setAge     (unsigned age_val) { age_    = age_val;    }
void EcoSim::Genetics::Organism::setHunger  (float hunger)  { if (heterotrophy_) heterotrophy_->hunger  = hunger;  }
void EcoSim::Genetics::Organism::setThirst  (float thirst)  { if (heterotrophy_) heterotrophy_->thirst  = thirst;  }
void EcoSim::Genetics::Organism::setFatigue (float fatigue) { if (heterotrophy_) heterotrophy_->fatigue = fatigue; }
void EcoSim::Genetics::Organism::setMate    (float mate)    { if (reproduction_) reproduction_->mate    = mate;    }
void EcoSim::Genetics::Organism::setXY      (int x, int y) { if (mobility_) { mobility_->worldX = static_cast<float>(x); mobility_->worldY = static_cast<float>(y); } x_ = x; y_ = y; }
void EcoSim::Genetics::Organism::setX       (int x)        { if (mobility_) mobility_->worldX = static_cast<float>(x); x_ = x; }
void EcoSim::Genetics::Organism::setY       (int y)        { if (mobility_) mobility_->worldY = static_cast<float>(y); y_ = y; }
void EcoSim::Genetics::Organism::setMotivation(Motivation m) { motivation_ = m; }
void EcoSim::Genetics::Organism::setAction(Action a)       { action_ = a; }

// Float position setters — setWorldPosition in Organism.cpp for vtable.
void EcoSim::Genetics::Organism::setWorldX(float x) { if (mobility_) mobility_->worldX = x; }
void EcoSim::Genetics::Organism::setWorldY(float y) { if (mobility_) mobility_->worldY = y; }

//================================================================================
//  Getters
//================================================================================
//================================================================================
//  ILifecycle Interface Implementation
//================================================================================
// isAlive, getMaxLifespan, getAgeNormalized, age — Organism defaults handle
// all of these. Organism::isAlive folds in deathCheck() which is now safe
// for plants (heterotroph-specific branches are gated on component presence).

float     EcoSim::Genetics::Organism::getHunger     () const { return heterotrophy_ ? heterotrophy_->hunger  : 0.0f; }
float     EcoSim::Genetics::Organism::getThirst     () const { return heterotrophy_ ? heterotrophy_->thirst  : 0.0f; }
float     EcoSim::Genetics::Organism::getFatigue    () const { return heterotrophy_ ? heterotrophy_->fatigue : 0.0f; }
float     EcoSim::Genetics::Organism::getMate       () const { return reproduction_ ? reproduction_->mate    : 0.0f; }
float     EcoSim::Genetics::Organism::getMetabolism () const { return heterotrophy_ ? heterotrophy_->metabolism : 0.0f; }
unsigned  EcoSim::Genetics::Organism::getSpeed      () const { return mobility_ ? mobility_->speed : 1u; }

// Float position getters
// getX(), getY(), getWorldX(), getWorldY() are now defined inline in creature.hpp
// delegating to Organism or using mobility_->worldX/mobility_->worldY directly
int       EcoSim::Genetics::Organism::tileX         () const { return mobility_ ? static_cast<int>(mobility_->worldX) : x_; }
int       EcoSim::Genetics::Organism::tileY         () const { return mobility_ ? static_cast<int>(mobility_->worldY) : y_; }

// Movement speed calculation
float EcoSim::Genetics::Organism::getMovementSpeed() const {
    using namespace EcoSim::Genetics::Constants;
    float locomotion = DEFAULT_LEG_LENGTH;
    float legLength  = DEFAULT_LEG_LENGTH;
    float bodyMass   = DEFAULT_BODY_MASS;

    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::LOCOMOTION)) {
        locomotion = phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::LOCOMOTION);
    }
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE)) {
        bodyMass = phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE);
        bodyMass = 0.5f + (bodyMass / 20.0f) * 1.5f;
    }
    legLength = 0.3f + locomotion * 0.7f;

    float speed = (BASE_MOVEMENT_SPEED * locomotion * legLength) / std::sqrt(bodyMass);
    return std::max(MIN_MOVEMENT_SPEED, speed);
}
Direction EcoSim::Genetics::Organism::getDirection  () const { return mobility_ ? mobility_->direction : Direction::none; }


float EcoSim::Genetics::Organism::getTHunger() const {
    if (phenotype_.hasTrait(UniversalGenes::HUNGER_THRESHOLD)) {
        return phenotype_.getTrait(UniversalGenes::HUNGER_THRESHOLD);
    }
    return 3.0f;
}

float EcoSim::Genetics::Organism::getTThirst() const {
    if (phenotype_.hasTrait(UniversalGenes::THIRST_THRESHOLD)) {
        return phenotype_.getTrait(UniversalGenes::THIRST_THRESHOLD);
    }
    return 3.0f;
}

float EcoSim::Genetics::Organism::getTFatigue() const {
    if (phenotype_.hasTrait(UniversalGenes::FATIGUE_THRESHOLD)) {
        return phenotype_.getTrait(UniversalGenes::FATIGUE_THRESHOLD);
    }
    return 3.0f;
}

float EcoSim::Genetics::Organism::getTMate() const {
    if (phenotype_.hasTrait(UniversalGenes::MATE_THRESHOLD)) {
        return phenotype_.getTrait(UniversalGenes::MATE_THRESHOLD);
    }
    return 3.0f;
}

float EcoSim::Genetics::Organism::getComfInc() const {
    if (phenotype_.hasTrait(UniversalGenes::COMFORT_INCREASE)) {
        return phenotype_.getTrait(UniversalGenes::COMFORT_INCREASE);
    }
    return 0.01f;
}

float EcoSim::Genetics::Organism::getComfDec() const {
    if (phenotype_.hasTrait(UniversalGenes::COMFORT_DECREASE)) {
        return phenotype_.getTrait(UniversalGenes::COMFORT_DECREASE);
    }
    return 0.01f;
}


//================================================================================
//  New Genetics System - Static Methods (M5)
//================================================================================
/**
 * Initialize the shared gene registry with default gene definitions.
 * Should be called once at application startup before creating creatures.
 */
void EcoSim::Genetics::Organism::initializeGeneRegistry() {
    if (!s_geneRegistry) {
        s_geneRegistry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
        EcoSim::Genetics::UniversalGenes::registerDefaults(*s_geneRegistry);
    }
}

EcoSim::Genetics::GeneRegistry& EcoSim::Genetics::Organism::getGeneRegistry() {
    if (!s_geneRegistry) {
        initializeGeneRegistry();
    }
    return *s_geneRegistry;
}

//================================================================================
//  New Genetics System - Instance Methods
//================================================================================


void EcoSim::Genetics::Organism::updateThermalCache() {
    if (!thermal_) return;
    thermal_->adaptations = EnvironmentalStressCalculator::extractThermalAdaptations(phenotype_);
    thermal_->baseTempLow = phenotype_.getTrait(UniversalGenes::TEMP_TOLERANCE_LOW);
    thermal_->baseTempHigh = phenotype_.getTrait(UniversalGenes::TEMP_TOLERANCE_HIGH);
    thermal_->toleranceRange = EnvironmentalStressCalculator::calculateEffectiveTempRange(
        thermal_->baseTempLow, thermal_->baseTempHigh, thermal_->adaptations);
    thermal_->cacheDirty = false;
}

// updatePhenotype is virtual override on Organism already — Creature's used
// to just call updatePhenotypeContext with default env. Leave the simpler
// Organism::updatePhenotype (which just invalidates the phenotype cache)
// in place; callers that need the full context refresh call
// updatePhenotypeContext directly. Creature's own override removed.

//================================================================================
//  Behaviours (legacy Profile methods removed — now handled by BehaviorController)
//================================================================================

// grow() moved to Organism base. Plant keeps its own (plant-specific logic).


// shareResource / shareFood / shareWater moved to Organism.cpp so the
// genetics library can link sexualBreed without depending on ecosim_core.

//================================================================================
//  Scent Detection Helper
//================================================================================

/**
 * Check if creature has meaningful scent detection capability.
 * Used to determine if scent-based fallback navigation is available.
 * @return True if scent_detection trait exceeds threshold (0.1)
 */
bool EcoSim::Genetics::Organism::hasScentDetection() const {
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
bool EcoSim::Genetics::Organism::findFoodScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const {
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
void EcoSim::Genetics::Organism::changeDirection (const int &xChange, const int &yChange) {
  if (!mobility_) return;
  if (xChange == 1) {
    if      (yChange ==  1) mobility_->direction = Direction::SE;
    else if (yChange == -1) mobility_->direction = Direction::NE;
    else                    mobility_->direction = Direction::E;
  } else if (xChange == -1) {
    if      (yChange ==  1) mobility_->direction = Direction::SW;
    else if (yChange == -1) mobility_->direction = Direction::NW;
    else                    mobility_->direction = Direction::W;
  } else {
    if      (yChange ==  1) mobility_->direction = Direction::S;
    else if (yChange == -1) mobility_->direction = Direction::N;
    else                    mobility_->direction = Direction::none;
  }
}

/**
 *  Calculates the euclidean distance between two points.
 *
 *  @param goalX  The x-coordinate of the destination.
 *  @param goalY  The y-coordinate of the destination.
 */
float EcoSim::Genetics::Organism::calculateDistance (const int &goalX, const int &goalY) const {
  float xDist = tileX() - goalX;
  float yDist = tileY() - goalY;
  return sqrt(xDist * xDist + yDist * yDist);
}

void EcoSim::Genetics::Organism::movementCost (const float &distance) {
  if (heterotrophy_) {
    heterotrophy_->hunger -= heterotrophy_->metabolism * distance;
  }
}

//================================================================================
//  Breeding
//
//  Pipeline (sexualBreed / asexualBreed / reproduce / isCompatibleWith)
//  lives on Organism. Creature only supplies the factory hook below.
//  checkFitness folded into Organism::isCompatibleWith — proximity is no
//  longer factored in (MatingBehavior::findMate already gates by sight
//  range, so re-weighting by distance was double counting). Pure genetic
//  similarity threshold remains.
//================================================================================
std::unique_ptr<EcoSim::Genetics::Organism> Creature::makeOffspring(
    std::unique_ptr<EcoSim::Genetics::Genome> offspringGenome,
    int x, int y) {
    if (!offspringGenome) return nullptr;
    auto offspring = std::make_unique<Creature>(x, y, std::move(offspringGenome));
    logging::Logger::getInstance().creatureBorn(
        offspring->getId(),
        offspring->generateName(),
        id_,
        -1   // mate id not threaded through the factory yet — improve later
    );
    return offspring;
}

//================================================================================
//  Sensory System Methods (Phase 1)
//================================================================================

/**
 * Compute this creature's unique genetic scent signature.
 * Delegates to CreatureScent::computeScentSignature.
 */
std::array<float, 8> EcoSim::Genetics::Organism::computeScentSignature() const {
  return CreatureScent::computeScentSignature(*this);
}

void EcoSim::Genetics::Organism::depositBreedingScent(EcoSim::ScentLayer& layer, unsigned int currentTick) {
  CreatureScent::depositBreedingScent(*this, layer, currentTick);
}

//================================================================================
//  Scent Detection Methods (Phase 2: Gradient Navigation)
//================================================================================

/**
 * Calculate genetic similarity between two scent signatures.
 * Delegates to CreatureScent::calculateSignatureSimilarity.
 */
float EcoSim::Genetics::Organism::calculateSignatureSimilarity(
    const std::array<float, 8>& sig1,
    const std::array<float, 8>& sig2) {
  return CreatureScent::calculateSignatureSimilarity(sig1, sig2);
}

std::optional<Direction> EcoSim::Genetics::Organism::detectMateDirection(const EcoSim::ScentLayer& scentLayer) const {
  return CreatureScent::detectMateDirection(*this, scentLayer);
}

bool EcoSim::Genetics::Organism::findMateScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const {
  return CreatureScent::findMateScent(*this, scentLayer, outX, outY);
}

//================================================================================
//  Plant Interaction Methods
//================================================================================

/**
	* Initialize shared interaction calculators.
	* Should be called once at application startup.
	*/
void EcoSim::Genetics::Organism::initializeInteractionSystems() {
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
EcoSim::Genetics::FeedingResult EcoSim::Genetics::Organism::eatPlant(EcoSim::Genetics::Plant& plant) {
    using namespace EcoSim::Genetics;
    if (!s_feedingInteraction) initializeInteractionSystems();
    FeedingResult result = CreaturePlantInteraction::eatPlant(*this, plant, *s_feedingInteraction);
    if (result.success && result.seedsConsumed && result.seedsToDisperse > 0 && heterotrophy_) {
        CreaturePlantInteraction::consumeSeeds(
            *this, plant, static_cast<int>(result.seedsToDisperse),
            1.0f - (result.seedsDestroyed ? 0.5f : 0.0f), heterotrophy_->gutSeeds);
    }
    return result;
}

bool EcoSim::Genetics::Organism::canEatPlant(const EcoSim::Genetics::Plant& plant) const {
    if (!s_feedingInteraction) const_cast<Organism*>(this)->initializeInteractionSystems();
    return CreaturePlantInteraction::canEatPlant(*this, plant, *s_feedingInteraction);
}

float EcoSim::Genetics::Organism::getPlantDetectionRange() const {
    return CreaturePlantInteraction::getPlantDetectionRange(*this);
}

void EcoSim::Genetics::Organism::attachBurr(const EcoSim::Genetics::Plant& plant) {
    if (!s_seedDispersal) initializeInteractionSystems();
    if (heterotrophy_) {
        CreaturePlantInteraction::attachBurr(*this, plant, *s_seedDispersal, heterotrophy_->attachedBurrs);
    }
}

std::vector<EcoSim::Genetics::DispersalEvent> EcoSim::Genetics::Organism::detachBurrs() {
    if (!s_seedDispersal || !heterotrophy_) return {};
    return CreaturePlantInteraction::detachBurrs(*this, *s_seedDispersal, heterotrophy_->attachedBurrs);
}

bool EcoSim::Genetics::Organism::hasBurrs() const {
    return heterotrophy_ && CreaturePlantInteraction::hasBurrs(heterotrophy_->attachedBurrs);
}

std::vector<EcoSim::Genetics::DispersalEvent> EcoSim::Genetics::Organism::getPendingBurrDispersal() const {
    if (!heterotrophy_) return {};
    return CreaturePlantInteraction::getPendingBurrDispersal(*this, heterotrophy_->attachedBurrs);
}

void EcoSim::Genetics::Organism::consumeSeeds(const EcoSim::Genetics::Plant& plant, int count, float viability) {
    if (!heterotrophy_) return;
    CreaturePlantInteraction::consumeSeeds(*this, plant, count, viability, heterotrophy_->gutSeeds);
}

std::vector<EcoSim::Genetics::DispersalEvent> EcoSim::Genetics::Organism::processGutSeeds(int ticksElapsed) {
    if (!heterotrophy_) return {};
    return CreaturePlantInteraction::processGutSeeds(*this, ticksElapsed, heterotrophy_->gutSeeds);
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
char EcoSim::Genetics::Organism::generateChar () {
  if (identity_ && identity_->archetype) {
    return identity_->archetype->getRenderChar();
  }
  return getDietInfo(getDietType()).character;
}


//================================================================================
//  To String
//================================================================================
Direction EcoSim::Genetics::Organism::stringToDirection (const std::string &str) {
  return CreatureSerialization::stringToDirection(str);
}

std::string EcoSim::Genetics::Organism::directionToString () const {
  return CreatureSerialization::directionToString(mobility_ ? mobility_->direction : Direction::none);
}

//================================================================================
//  New Genetics System Getters
//================================================================================

/**
 * Get the scientific name for this creature.
 * Uses CreatureTaxonomy for dynamic scientific name generation.
 */
std::string EcoSim::Genetics::Organism::getScientificName() const {
    return EcoSim::Genetics::CreatureTaxonomy::generateScientificName(genome_);
}

std::string EcoSim::Genetics::Organism::getArchetypeLabel() const {
    if (identity_ && identity_->archetype) {
        return identity_->archetype->getLabel();
    }
    return EcoSim::Genetics::Phenotype::dietTypeToString(getDietType());
}

const EcoSim::Genetics::BiomeAdaptation* EcoSim::Genetics::Organism::getBiomeAdaptation() const {
    return identity_ ? identity_->biomeAdaptation : nullptr;
}

std::string EcoSim::Genetics::Organism::getFullLabel() const {
    if (identity_ && identity_->biomeAdaptation && identity_->archetype) {
        return identity_->biomeAdaptation->getFullLabel(identity_->archetype);
    }
    return getArchetypeLabel();
}

void EcoSim::Genetics::Organism::reclassifyBiomeAdaptation() {
    if (!identity_) return;
    if (identity_->biomeAdaptation) {
        identity_->biomeAdaptation->decrementPopulation();
    }
    identity_->biomeAdaptation = EcoSim::Genetics::CreatureTaxonomy::classifyBiomeAdaptation(genome_);
    if (identity_->biomeAdaptation) {
        identity_->biomeAdaptation->incrementPopulation();
    }
}

//================================================================================
//  Health System
//================================================================================

/**
 * Get maximum health value.
 * Based on MAX_SIZE gene for creature mass scaling.
 */
// Creature::getMaxHealth removed — Organism::getMaxHealth is the single
// canonical implementation (MAX_SIZE gene based).

void EcoSim::Genetics::Organism::takeDamage(float amount) {
    if (amount <= 0.0f) return;
    health_ = std::max(0.0f, health_ - amount);
}

// Creature::heal removed — Organism::heal does the same thing now that
// getMaxHealth is MAX_SIZE-based on the base class.

void EcoSim::Genetics::Organism::setInCombat(bool combat) {
    if (combat_) combat_->inCombat = combat;
}

void EcoSim::Genetics::Organism::setTargetId(int targetId) {
    if (combat_) combat_->targetId = targetId;
}

void EcoSim::Genetics::Organism::setCombatCooldown(int cooldown) {
    if (combat_) combat_->combatCooldown = cooldown;
}

void EcoSim::Genetics::Organism::setFleeing(bool fleeing) {
    if (combat_) combat_->isFleeing = fleeing;
}

float EcoSim::Genetics::Organism::getHealthPercent() const {
    float maxHealth = getMaxHealth();
    if (maxHealth <= 0.0f) return 0.0f;
    return std::max(0.0f, std::min(1.0f, getHealth() / maxHealth));
}

EcoSim::Genetics::WoundState EcoSim::Genetics::Organism::getWoundState() const {
    float percent = getHealthPercent();
    if (percent > 0.75f) return WoundState::Healthy;
    if (percent > 0.50f) return WoundState::Injured;
    if (percent > 0.25f) return WoundState::Wounded;
    if (percent > 0.0f)  return WoundState::Critical;
    return WoundState::Dead;
}

float EcoSim::Genetics::Organism::getWoundSeverity() const {
    return 1.0f - getHealthPercent();
}

float EcoSim::Genetics::Organism::getHealingRate() const {
    if (phenotype_.hasTrait(EcoSim::Genetics::UniversalGenes::REGENERATION_RATE)) {
        return phenotype_.getTrait(EcoSim::Genetics::UniversalGenes::REGENERATION_RATE) * 0.001f;
    }
    return 0.001f;  // Default healing rate
}

bool EcoSim::Genetics::Organism::isInCombat() const {
    return combat_ && combat_->inCombat;
}

bool EcoSim::Genetics::Organism::isFleeing() const {
    return combat_ && combat_->isFleeing;
}

int EcoSim::Genetics::Organism::getTargetId() const {
    return combat_ ? combat_->targetId : -1;
}

int EcoSim::Genetics::Organism::getCombatCooldown() const {
    return combat_ ? combat_->combatCooldown : 0;
}

/**
 * Get creature's current motivation state.
 */
Motivation EcoSim::Genetics::Organism::getMotivation() const {
    return motivation_;
}

/**
 * Get creature's current action state.
 */
Action EcoSim::Genetics::Organism::getAction() const {
    return action_;
}

/**
 * Get expressed value of a gene from the phenotype.
 */
float EcoSim::Genetics::Organism::getExpressedValue(const std::string& geneId) const {
    if (phenotype_.hasTrait(geneId)) {
        return phenotype_.getTrait(geneId);
    }
    return 0.0f;
}

//================================================================================
//  New Genetics Constructors
//================================================================================

/**
 * Constructor with new genetics genome only — delegates to OrganismFactory.
 */
Creature::Creature(int x, int y, std::unique_ptr<EcoSim::Genetics::Genome> genome)
    : Organism(x, y, std::move(*genome), getGeneRegistry()) {

    // Classify + attach via the factory. For creature-flavoured genomes
    // this produces the same component set the legacy ctor did
    // (mobility + heterotrophy + combat + thermal + reproduction + identity)
    // plus archetype/biome classification, speciesName, initial needs,
    // and phenotype context. Once B.4 migrates call sites, these ctors
    // disappear and OrganismFactory::fromGenome is called directly.
    auto sig = EcoSim::Genetics::OrganismFactory::classifyComponentSet(genome_);
    EcoSim::Genetics::OrganismFactory::attachComponents(*this, sig);

    // Creature-specific post-attach: sequential display ID for the UI.
    identity_->sequentialId = nextCreatureId_++;
}

/**
 * Constructor with new genetics genome and initial hunger/thirst values.
 */
Creature::Creature(int x, int y, float hunger, float thirst,
                   std::unique_ptr<EcoSim::Genetics::Genome> genome)
    : Organism(x, y, std::move(*genome), getGeneRegistry()) {

    auto sig = EcoSim::Genetics::OrganismFactory::classifyComponentSet(genome_);
    EcoSim::Genetics::OrganismFactory::attachComponents(*this, sig);

    identity_->sequentialId = nextCreatureId_++;

    // Override starting hunger/thirst with caller-supplied values. Factory
    // seeded them at RESOURCE_LIMIT by default.
    if (heterotrophy_) {
        heterotrophy_->hunger = hunger;
        heterotrophy_->thirst = thirst;
    }
}

//================================================================================
//  Behavior System Integration (Phase 3: God Class Decomposition)
//================================================================================

/**
 * Initialize the behavior controller with all behaviors.
 * Called once during creature construction or on first use.
 */
// getBehaviorController is inherited via Organism::getOrganismBehaviorController.

void EcoSim::Genetics::Organism::initializeBehaviorController() {
    using namespace EcoSim::Genetics;

    if (organismBehaviorController_) {
        return;  // Already initialized
    }

    // Lazy-init shared services (same pattern as before)
    if (!s_perceptionSystem)  s_perceptionSystem  = std::make_unique<PerceptionSystem>();
    if (!s_combatInteraction) s_combatInteraction = std::make_unique<CombatInteraction>();
    if (!s_feedingInteraction) s_feedingInteraction = std::make_unique<FeedingInteraction>();
    if (!s_seedDispersal)     s_seedDispersal     = std::make_unique<SeedDispersal>();

    organismBehaviorController_ = std::make_unique<BehaviorController>();

    // Delegate to BehaviorRegistry — gene expression decides which behaviors attach.
    BehaviorServices services;
    services.perception    = s_perceptionSystem.get();
    services.combat        = s_combatInteraction.get();
    services.feeding       = s_feedingInteraction.get();
    services.seedDispersal = s_seedDispersal.get();
    services.geneRegistry  = s_geneRegistry.get();

    BehaviorRegistry::attachBehaviorsFor(*organismBehaviorController_, phenotype_, services);
}

/**
 * Build a behavior context from current creature and world state.
 * Creates an immutable snapshot for behavior execution.
 *
 * @note The organismState pointer in the returned context points to
 * a static local, which is valid for the duration of the behavior
 * update cycle. Do not store the pointer beyond a single tick.
 */
EcoSim::Genetics::BehaviorContext EcoSim::Genetics::Organism::buildBehaviorContext(
    World& world,
    EcoSim::ScentLayer& scentLayer,
    unsigned int currentTick) const {
    
    using namespace EcoSim::Genetics;
    
    // Create organism state snapshot
    // Using thread_local static for safe temporary storage
    thread_local OrganismState tempState;
    tempState.age_normalized = getAgeNormalized();
    tempState.energy_level = std::max(0.0f, std::min(1.0f, heterotrophy_->hunger / Constants::RESOURCE_LIMIT));
    tempState.hydration = std::max(0.0f, std::min(1.0f, heterotrophy_->thirst / Constants::RESOURCE_LIMIT));
    tempState.fatigue = heterotrophy_->fatigue;
    tempState.reproductive_urge = reproduction_->mate;
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
EcoSim::Genetics::BehaviorResult EcoSim::Genetics::Organism::updateWithBehaviors(EcoSim::Genetics::BehaviorContext& ctx) {
    using namespace EcoSim::Genetics;

    // Ensure behavior controller is initialized
    if (!organismBehaviorController_) {
        initializeBehaviorController();
    }

    // Update the behavior controller — selects and executes the highest priority behavior
    BehaviorResult result = organismBehaviorController_->update(*this, ctx);

    // Apply energy cost from behavior execution. Negative energy cost
    // (e.g. FeedingBehavior reports -nutritionGained on successful feed)
    // flows through as a hunger refill.
    if (result.executed && heterotrophy_) {
        heterotrophy_->hunger -= result.energyCost;
        heterotrophy_->hunger = std::min(heterotrophy_->hunger, Constants::RESOURCE_LIMIT);
    }

    // Derive Motivation/Action from the currently active behavior
    std::string behaviorId = organismBehaviorController_->getCurrentBehaviorId();
    if (behaviorId == "feeding") {
        motivation_ = Motivation::Hungry;
        action_ = Action::Grazing;
    } else if (behaviorId == "hunting") {
        motivation_ = Motivation::Hungry;
        action_ = Action::Hunting;
    } else if (behaviorId == "thirst") {
        motivation_ = Motivation::Thirsty;
        action_ = result.completed ? Action::Drinking : Action::Searching;
    } else if (behaviorId == "mating") {
        motivation_ = Motivation::Amorous;
        action_ = Action::Courting;
    } else if (behaviorId == "rest") {
        motivation_ = Motivation::Tired;
        action_ = Action::Resting;
    } else if (behaviorId == "movement") {
        motivation_ = Motivation::Content;
        action_ = Action::Wandering;
    } else {
        motivation_ = Motivation::Content;
        action_ = Action::Idle;
    }

    // Passive lifecycle — inline steps (previously Organism::tickLifecycle framework)
    {
        EnvironmentState env = phenotype_.getEnvironment();
        updatePhenotypeContext(env);
        grow();

        // Metabolism: drain energy/hydration, accumulate fatigue
        float change = heterotrophy_->metabolism;
        bool isResting = (motivation_ == Motivation::Tired);
        if (isResting) {
            heterotrophy_->fatigue -= heterotrophy_->metabolism;
            change /= 2;
        } else {
            heterotrophy_->fatigue += heterotrophy_->metabolism;
        }

        if (thermal_->cacheDirty) {
            updateThermalCache();
        }

        float currentTemp = phenotype_.getEnvironment().temperature;
        if (std::abs(currentTemp - thermal_->lastProcessedTemp) > 0.1f) {
            thermal_->currentStress = EnvironmentalStressCalculator::calculateTemperatureStress(
                currentTemp, thermal_->baseTempLow, thermal_->baseTempHigh, thermal_->adaptations);
            thermal_->lastProcessedTemp = currentTemp;
        }

        change *= thermal_->currentStress.energyDrainMultiplier;
        heterotrophy_->hunger -= change;
        heterotrophy_->thirst -= change;

        // Environmental stress: temperature/stress health damage
        if (thermal_->currentStress.healthDamageRate > 0.0f) {
            float damage = thermal_->currentStress.healthDamageRate * getMaxHealth();
            takeDamage(damage);
        }

        // Reproductive drive: slow passive accumulation scaled by well-being.
        // Hungry creatures gain drive slowly; well-fed ones gain faster.
        // Starving creatures lose drive. Cap at RESOURCE_LIMIT so urge
        // saturates at the max mating readiness.
        if (reproduction_) {
            float wellbeing = 0.5f;  // neutral default
            if (heterotrophy_) {
                wellbeing = std::clamp(
                    heterotrophy_->hunger / Constants::RESOURCE_LIMIT, -0.5f, 1.0f);
            }
            // 0.05/tick baseline — a fed creature reaches the mating
            // threshold (mate=7) in ~140 ticks. Low enough that starving
            // creatures still deprioritise mating, high enough that healthy
            // populations produce offspring within a reasonable sim window.
            float rate = 0.05f * wellbeing;
            reproduction_->mate = std::clamp(
                reproduction_->mate + rate, -3.0f, Constants::RESOURCE_LIMIT);
        }

        ++age_;
    }

    return result;
}

//================================================================================
//  JSON Serialization
//  (Enum<->string converters live in CreatureSerialization; the Creature::
//  forwarders had no callers and were removed during the class collapse.)
//================================================================================

// Creature::toJson / fromJson forwarders removed — call sites use
// CreatureSerialization::toJson / fromJson directly.
