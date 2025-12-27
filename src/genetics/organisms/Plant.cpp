/**
 * @file Plant.cpp
 * @brief Implementation of the Plant class
 *
 * This file implements the plant organism that uses the genetics system.
 * Plants share the same genome infrastructure as creatures but use
 * plant-specific genes defined in PlantGenes.hpp.
 *
 * Phase 2.3: Added defense system, food production, emergent dispersal,
 * energy budget integration, and rendering support.
 */

#include "genetics/organisms/Plant.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "logging/Logger.hpp"
#include <random>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Static member initialization
// ============================================================================

unsigned int Plant::nextId_ = 1;

// ============================================================================
// Constructors
// ============================================================================

Plant::Plant(int x, int y, const GeneRegistry& registry)
    : x_(x)
    , y_(y)
    , registry_(&registry)
    , id_(nextId_++)
{
    // Create a random genome using plant genes
    genome_ = PlantGenes::createRandomGenome(registry);
    
    // Initialize phenotype
    phenotype_ = Phenotype(&genome_, &registry);
    updatePhenotype();
    
    // Log plant spawn
    logging::Logger::getInstance().plantSpawned(id_, "random", x_, y_);
}

Plant::Plant(int x, int y, const Genome& genome, const GeneRegistry& registry)
    : x_(x)
    , y_(y)
    , genome_(genome)
    , registry_(&registry)
    , id_(nextId_++)
{
    // Initialize phenotype from provided genome
    phenotype_ = Phenotype(&genome_, &registry);
    updatePhenotype();
    
    // Log plant spawn
    logging::Logger::getInstance().plantSpawned(id_, "offspring", x_, y_);
}

// ============================================================================
// Rule of Five: Copy and Move Semantics
// ============================================================================

Plant::Plant(const Plant& other)
    : x_(other.x_)
    , y_(other.y_)
    , age_(other.age_)
    , alive_(other.alive_)
    , health_(other.health_)
    , current_size_(other.current_size_)
    , mature_(other.mature_)
    , genome_(other.genome_)  // Deep copy of genome
    , registry_(other.registry_)
    , energyState_(other.energyState_)
    , fruitTimer_(other.fruitTimer_)
    , id_(nextId_++)  // New plant gets new ID
    , entityType_(other.entityType_)
{
    // CRITICAL: Rebind phenotype to THIS plant's genome, not the source's
    phenotype_ = Phenotype(&genome_, registry_);
    updatePhenotype();
}

Plant::Plant(Plant&& other) noexcept
    : x_(other.x_)
    , y_(other.y_)
    , age_(other.age_)
    , alive_(other.alive_)
    , health_(other.health_)
    , current_size_(other.current_size_)
    , mature_(other.mature_)
    , genome_(std::move(other.genome_))  // Move genome
    , registry_(other.registry_)
    , energyState_(std::move(other.energyState_))
    , fruitTimer_(other.fruitTimer_)
    , id_(other.id_)  // Keep same ID for moved plant
    , entityType_(other.entityType_)
{
    // CRITICAL: Rebind phenotype to THIS plant's genome after move
    phenotype_ = Phenotype(&genome_, registry_);
    updatePhenotype();
    
    // Invalidate moved-from object
    other.registry_ = nullptr;
    other.alive_ = false;
}

Plant& Plant::operator=(const Plant& other) {
    if (this != &other) {
        x_ = other.x_;
        y_ = other.y_;
        age_ = other.age_;
        alive_ = other.alive_;
        health_ = other.health_;
        current_size_ = other.current_size_;
        mature_ = other.mature_;
        genome_ = other.genome_;  // Deep copy
        registry_ = other.registry_;
        energyState_ = other.energyState_;
        fruitTimer_ = other.fruitTimer_;
        // id_ stays the same - this is the same plant
        entityType_ = other.entityType_;
        
        // CRITICAL: Rebind phenotype to THIS plant's genome
        phenotype_ = Phenotype(&genome_, registry_);
        updatePhenotype();
    }
    return *this;
}

Plant& Plant::operator=(Plant&& other) noexcept {
    if (this != &other) {
        x_ = other.x_;
        y_ = other.y_;
        age_ = other.age_;
        alive_ = other.alive_;
        health_ = other.health_;
        current_size_ = other.current_size_;
        mature_ = other.mature_;
        genome_ = std::move(other.genome_);  // Move genome
        registry_ = other.registry_;
        energyState_ = std::move(other.energyState_);
        fruitTimer_ = other.fruitTimer_;
        id_ = other.id_;  // Take the ID from moved object
        entityType_ = other.entityType_;
        
        // CRITICAL: Rebind phenotype to THIS plant's genome after move
        phenotype_ = Phenotype(&genome_, registry_);
        updatePhenotype();
        
        // Invalidate moved-from object
        other.registry_ = nullptr;
        other.alive_ = false;
    }
    return *this;
}

// ============================================================================
// ILifecycle implementation
// ============================================================================

unsigned int Plant::getMaxLifespan() const {
    // Get lifespan directly from genome (more reliable than phenotype traits)
    // UniversalGenes::LIFESPAN is the foundation - all organisms use it
    // PlantGenes::LIFESPAN is legacy fallback only
    if (genome_.hasGene(UniversalGenes::LIFESPAN)) {
        const Gene& gene = genome_.getGene(UniversalGenes::LIFESPAN);
        // Use average of both alleles for consistent behavior
        GeneValue gv = gene.getExpressedValue(DominanceType::Incomplete);
        if (std::holds_alternative<float>(gv)) {
            return static_cast<unsigned int>(std::get<float>(gv));
        }
    }
    if (genome_.hasGene(PlantGenes::LIFESPAN)) {
        const Gene& gene = genome_.getGene(PlantGenes::LIFESPAN);
        GeneValue gv = gene.getExpressedValue(DominanceType::Incomplete);
        if (std::holds_alternative<float>(gv)) {
            return static_cast<unsigned int>(std::get<float>(gv));
        }
    }
    return 5000;  // Default lifespan
}

float Plant::getAgeNormalized() const {
    unsigned int maxLife = getMaxLifespan();
    if (maxLife == 0) return 1.0f;
    return static_cast<float>(age_) / static_cast<float>(maxLife);
}

void Plant::age(unsigned int ticks) {
    if (!alive_) return;
    
    age_ += ticks;
    
    // Note: Death by old age is now checked in update() after other lifecycle events
    // This ensures fruit production and other events can happen at max lifespan
}

// ============================================================================
// IGeneticOrganism implementation
// ============================================================================

void Plant::updatePhenotype() {
    // Create organism state for phenotype calculation
    OrganismState state;
    state.age_normalized = getAgeNormalized();
    state.health = health_;
    state.energy_level = 1.0f;  // Plants always have energy if alive
    
    // Update context with default environment (can be updated later)
    EnvironmentState env;
    phenotype_.updateContext(env, state);
}

// ============================================================================
// Helper: Read gene value from genome
// ============================================================================

float Plant::getGeneValueFromGenome(const char* geneId, float defaultValue) const {
    if (genome_.hasGene(geneId)) {
        const Gene& gene = genome_.getGene(geneId);
        GeneValue gv = gene.getExpressedValue(DominanceType::Incomplete);
        if (std::holds_alternative<float>(gv)) {
            return std::get<float>(gv);
        }
    }
    return defaultValue;
}

// ============================================================================
// Plant-specific getters
// ============================================================================

float Plant::getGrowthRate() const {
    // Read directly from genome for reliability
    return getGeneValueFromGenome(PlantGenes::GROWTH_RATE, 0.5f);
}

float Plant::getMaxSize() const {
    // Read directly from genome for reliability
    return getGeneValueFromGenome(PlantGenes::MAX_SIZE, 5.0f);
}

float Plant::getNutrientValue() const {
    // Scale nutrient value by current size
    float baseValue = getGeneValueFromGenome(PlantGenes::NUTRIENT_VALUE, 25.0f);
    float sizeRatio = current_size_ / getMaxSize();
    return baseValue * sizeRatio;
}

float Plant::getWaterNeed() const {
    return getGeneValueFromGenome(PlantGenes::WATER_NEED, 0.5f);
}

float Plant::getLightNeed() const {
    return getGeneValueFromGenome(PlantGenes::LIGHT_NEED, 0.5f);
}

float Plant::getHardiness() const {
    return getGeneValueFromGenome(PlantGenes::HARDINESS, 0.5f);
}

float Plant::getColorHue() const {
    return getGeneValueFromGenome(PlantGenes::COLOR_HUE, 120.0f);
}

float Plant::getSpreadDistance() const {
    return getGeneValueFromGenome(PlantGenes::SPREAD_DISTANCE, 5.0f);
}

int Plant::getSeedCount() const {
    if (!mature_) return 0;
    
    // Seed production scales with plant size and health
    float baseSeedCount = getGeneValueFromGenome(PlantGenes::SEED_PRODUCTION, 5.0f);
    float sizeRatio = current_size_ / getMaxSize();
    return static_cast<int>(baseSeedCount * sizeRatio * health_);
}

bool Plant::canSpreadSeeds() const {
    return alive_ && mature_ && health_ > 0.5f && getSeedCount() > 0;
}

bool Plant::canSurviveTemperature(float temperature) const {
    // Use the more reliable getGeneValueFromGenome helper
    // Note: PlantFactory sets temperature tolerance using UniversalGenes, not PlantGenes
    float tempLow = getGeneValueFromGenome(UniversalGenes::TEMP_TOLERANCE_LOW, 5.0f);
    float tempHigh = getGeneValueFromGenome(UniversalGenes::TEMP_TOLERANCE_HIGH, 35.0f);
    return temperature >= tempLow && temperature <= tempHigh;
}

// ============================================================================
// Update and state management
// ============================================================================

void Plant::update(const EnvironmentState& env) {
    if (!alive_) return;
    
    // Update phenotype with current environment
    OrganismState state;
    state.age_normalized = getAgeNormalized();
    state.health = health_;
    state.energy_level = 1.0f;
    phenotype_.updateContext(env, state);
    
    // Grow the plant (includes maturity check)
    grow(env);
    
    // Increment fruit timer for production cooldown
    fruitTimer_++;
    
    // Age the plant (just increments age counter)
    age(1);
    
    // Check death conditions (health-based)
    checkDeathConditions(env);
    
    // Check for death from old age AFTER other lifecycle events
    // Use > (not >=) so plant can act at exactly max lifespan
    float maxLifespan = static_cast<float>(getMaxLifespan());
    if (static_cast<float>(age_) > maxLifespan) {
        alive_ = false;
        health_ = 0.0f;
        
        // Log plant death from old age
        logging::Logger::getInstance().plantDied(id_, "plant", "old_age", age_);
    }
}

void Plant::grow(const EnvironmentState& env) {
    if (!alive_) return;
    
    float maxSize = getMaxSize();
    if (current_size_ >= maxSize) {
        // Already at max size
        if (!mature_) mature_ = true;
        return;
    }
    
    // Base growth rate from genes
    float growthRate = getGrowthRate();
    
    // Modify growth based on environment
    // Light factor: plants need light to grow (via photosynthesis)
    float lightFactor = 1.0f;
    float lightNeed = getLightNeed();
    // time_of_day 0.5 = noon = full light
    float availableLight = std::sin(env.time_of_day * 3.14159f);
    if (availableLight < lightNeed) {
        lightFactor = availableLight / (lightNeed + 0.01f);
    }
    
    // Humidity/water factor
    float waterFactor = 1.0f;
    float waterNeed = getWaterNeed();
    if (env.humidity < waterNeed) {
        waterFactor = env.humidity / (waterNeed + 0.01f);
    }
    
    // Temperature factor - growth is reduced if temperature is extreme
    float tempFactor = 1.0f;
    if (!canSurviveTemperature(env.temperature)) {
        // Outside survival range, no growth and take damage
        tempFactor = 0.0f;
        takeDamage(0.01f);
    } else {
        // Optimal temperature around 20-25°C
        float optimalTemp = 22.5f;
        float tempDiff = std::abs(env.temperature - optimalTemp);
        tempFactor = std::max(0.5f, 1.0f - (tempDiff / 30.0f));
    }
    
    // Calculate actual growth
    // Note: Multiplier of 0.003 allows berry bushes to mature in ~900 ticks
    float effectiveGrowth = growthRate * lightFactor * waterFactor * tempFactor * 0.003f;
    current_size_ = std::min(current_size_ + effectiveGrowth, maxSize);
    
    // Check for maturity (can reproduce when at 50% of max size)
    if (!mature_ && current_size_ >= maxSize * 0.5f) {
        mature_ = true;
    }
}

void Plant::checkDeathConditions(const EnvironmentState& env) {
    // Death by age is handled in age()
    
    // Death by health
    if (health_ <= 0.0f) {
        alive_ = false;
        
        // Log plant death from health depletion
        logging::Logger::getInstance().plantDied(id_, "plant", "health_depleted", age_);
        return;
    }
    
    // Death by extreme temperature (prolonged exposure)
    if (!canSurviveTemperature(env.temperature)) {
        // Already taking damage in grow(), health will eventually drop to 0
    }
}

void Plant::takeDamage(float amount) {
    if (!alive_) return;
    
    // Reduce damage by hardiness
    float hardiness = getHardiness();
    float actualDamage = amount * (1.0f - hardiness * 0.5f);  // Max 50% damage reduction
    
    health_ -= actualDamage;
    health_ = std::max(0.0f, health_);
    
    if (health_ <= 0.0f) {
        alive_ = false;
        
        // Log plant death from damage
        logging::Logger::getInstance().plantDied(id_, "plant", "damage", age_);
    }
}

std::unique_ptr<Plant> Plant::produceOffspring(const GeneRegistry& registry) const {
    if (!canSpreadSeeds()) return nullptr;
    
    // Create offspring genome through mutation (asexual reproduction for plants)
    Genome offspringGenome = genome_;
    
    // Apply mutation
    float mutationRate = 0.05f;  // 5% mutation rate
    offspringGenome.mutate(mutationRate, registry.getAllDefinitions());
    
    // Calculate offspring position based on spread distance
    static thread_local std::mt19937 rng(std::random_device{}());
    float spread = getSpreadDistance();
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> distDist(1.0f, spread);
    
    float angle = angleDist(rng);
    float distance = distDist(rng);
    
    int offspringX = x_ + static_cast<int>(std::cos(angle) * distance);
    int offspringY = y_ + static_cast<int>(std::sin(angle) * distance);
    
    // Create and return offspring
    return std::make_unique<Plant>(offspringX, offspringY, offspringGenome, registry);
}

// ============================================================================
// Phase 2.3: Defense System
// ============================================================================

float Plant::getToxicity() const {
    return getGeneValueFromGenome(UniversalGenes::TOXIN_PRODUCTION, 0.0f);
}

float Plant::getThornDamage() const {
    return getGeneValueFromGenome(UniversalGenes::THORN_DENSITY, 0.0f);
}

float Plant::getRegrowthRate() const {
    return getGeneValueFromGenome(UniversalGenes::REGROWTH_RATE, 0.3f);
}

bool Plant::canRegenerate() const {
    if (!alive_ || health_ >= 1.0f) return false;
    
    // Need minimum energy and regrowth ability
    float regrowth = getRegrowthRate();
    // Calculate surplus as energy above maintenance costs
    float surplus = energyState_.currentEnergy - energyState_.maintenanceCost - energyState_.baseMetabolism;
    bool hasEnergy = surplus > 0.0f;
    
    return regrowth > 0.1f && hasEnergy;
}

void Plant::regenerate() {
    if (!canRegenerate()) return;
    
    float regrowthRate = getRegrowthRate();
    
    // Regenerate health proportional to regrowth rate
    float healthGain = regrowthRate * 0.01f;  // 1% max per tick
    health_ = std::min(1.0f, health_ + healthGain);
    
    // Also regenerate size if damaged
    float maxSize = getMaxSize();
    if (current_size_ < maxSize * 0.9f) {
        current_size_ = std::min(maxSize, current_size_ + regrowthRate * 0.001f);
    }
}

// ============================================================================
// Phase 2.3: Food Production (Hybrid Model)
// ============================================================================

bool Plant::canProduceFruit() const {
    if (!alive_ || !mature_) return false;
    
    // Check size maturity: at least 50% of max size
    float maxSize = getMaxSize();
    if (current_size_ < maxSize * 0.5f) return false;
    
    // Check age maturity: at least 10% of lifespan (reduced from 25% for achievability)
    float maturityAge = getMaxLifespan() * 0.10f;
    if (age_ < static_cast<unsigned int>(maturityAge)) return false;
    
    // Check fruit production rate - zero means no fruit production
    float fruitRate = getFruitProductionRate();
    if (fruitRate <= 0.01f) return false;
    
    // Check cooldown timer (higher rate = faster production)
    // With fruitRate ~0.8 (berry bush), cooldown is ~111 ticks
    // With fruitRate ~0.2 (oak), cooldown is ~333 ticks
    unsigned int cooldown = static_cast<unsigned int>(100.0f / (fruitRate + 0.1f));
    return fruitTimer_ >= cooldown;
}

// NOTE: produceFruit() method removed - Food class has been deleted.
// Creatures now feed directly on plants via FeedingInteraction.
// The nutrition value comes from Plant::getNutrientValue() and related genes.

float Plant::getFruitProductionRate() const {
    return getGeneValueFromGenome(UniversalGenes::FRUIT_PRODUCTION_RATE, 0.3f);
}

bool Plant::canSpreadVegetatively() const {
    if (!alive_ || !mature_) return false;
    
    // Check size maturity: at least 50% of max size (same as fruit production)
    float maxSize = getMaxSize();
    if (current_size_ < maxSize * 0.5f) return false;
    
    // Check age maturity: at least 10% of lifespan (same as fruit production)
    float maturityAge = getMaxLifespan() * 0.10f;
    if (age_ < static_cast<unsigned int>(maturityAge)) return false;
    
    // Check runner production - this is the key trait for vegetative spread
    float runnerProd = getRunnerProduction();
    if (runnerProd < 0.5f) return false;  // Need significant runner production
    
    // Vegetative reproduction uses a timer similar to fruit, but based on runner production
    unsigned int cooldown = static_cast<unsigned int>(150.0f / (runnerProd + 0.1f));
    return fruitTimer_ >= cooldown;
}

float Plant::getRunnerProduction() const {
    return getGeneValueFromGenome(UniversalGenes::RUNNER_PRODUCTION, 0.0f);
}

float Plant::getFruitAppeal() const {
    return getGeneValueFromGenome(UniversalGenes::FRUIT_APPEAL, 0.5f);
}

// ============================================================================
// Phase 2.3: Seed Properties
// ============================================================================

float Plant::getSeedMass() const {
    return getGeneValueFromGenome(UniversalGenes::SEED_MASS, 0.5f);
}

float Plant::getSeedAerodynamics() const {
    return getGeneValueFromGenome(UniversalGenes::SEED_AERODYNAMICS, 0.2f);
}

float Plant::getSeedHookStrength() const {
    return getGeneValueFromGenome(UniversalGenes::SEED_HOOK_STRENGTH, 0.0f);
}

float Plant::getSeedCoatDurability() const {
    return getGeneValueFromGenome(UniversalGenes::SEED_COAT_DURABILITY, 0.5f);
}

// ============================================================================
// Phase 2.3: Emergent Dispersal Strategy
// ============================================================================

DispersalStrategy Plant::getPrimaryDispersalStrategy() const {
    // Get all relevant traits (now read directly from genome)
    float seedMass = getSeedMass();
    float aerodynamics = getSeedAerodynamics();
    float hookStrength = getSeedHookStrength();
    float fruitAppeal = getFruitAppeal();
    float seedDurability = getSeedCoatDurability();
    
    // Get runner production (vegetative strategy)
    float runnerProd = getGeneValueFromGenome(UniversalGenes::RUNNER_PRODUCTION, 0.0f);
    
    // Get explosive pod force
    float explosiveForce = getGeneValueFromGenome(UniversalGenes::EXPLOSIVE_POD_FORCE, 0.0f);
    
    // Strategy emerges from gene thresholds (order matters!)
    
    // 1. Vegetative: high runner production prioritizes clonal spreading
    if (runnerProd > 0.7f) {
        return DispersalStrategy::VEGETATIVE;
    }
    
    // 2. Explosive: mechanical seed launching
    if (explosiveForce > 0.5f) {
        return DispersalStrategy::EXPLOSIVE;
    }
    
    // 3. Wind: light seeds with aerodynamic features
    if (seedMass < 0.2f && aerodynamics > 0.6f) {
        return DispersalStrategy::WIND;
    }
    
    // 4. Animal burr: hooks attach to fur
    if (hookStrength > 0.6f) {
        return DispersalStrategy::ANIMAL_BURR;
    }
    
    // 5. Animal fruit: attractive fruit with durable seeds
    if (fruitAppeal > 0.6f && seedDurability > 0.5f) {
        return DispersalStrategy::ANIMAL_FRUIT;
    }
    
    // 6. Default: gravity dispersal
    return DispersalStrategy::GRAVITY;
}

// ============================================================================
// Phase 2.3: Energy Budget Integration
// ============================================================================

void Plant::setEnergyState(const EnergyState& state) {
    energyState_ = state;
}

EnergyState& Plant::getEnergyState() {
    return energyState_;
}

const EnergyState& Plant::getEnergyState() const {
    return energyState_;
}

// ============================================================================
// Phase 2.3: Rendering Support
// ============================================================================

EntityType Plant::getEntityType() const {
    return entityType_;
}

void Plant::setEntityType(EntityType type) {
    entityType_ = type;
}

char Plant::getRenderCharacter() const {
    // Return character based on entity type
    switch (entityType_) {
        case EntityType::PLANT_BERRY_BUSH:
            return 'B';  // Berry bush
        case EntityType::PLANT_OAK_TREE:
            return 'T';  // Tree
        case EntityType::PLANT_GRASS:
            return '"';  // Grass blades
        case EntityType::PLANT_THORN_BUSH:
            return '*';  // Thorny
        case EntityType::PLANT_GENERIC:
        default:
            // For generic plants, choose based on size
            if (current_size_ < 1.0f) return '.';       // Seedling
            else if (current_size_ < 3.0f) return 'p';  // Small plant
            else if (current_size_ < 7.0f) return 'P';  // Medium plant
            else return 'T';                             // Large = tree
    }
}

Color Plant::getRenderColor() const {
    // Convert COLOR_HUE gene (HSV) to RGB color
    float hue = getColorHue();
    
    // Simple HSV to RGB conversion (assuming S=1.0, V=1.0)
    float h = hue / 60.0f;
    int i = static_cast<int>(h) % 6;
    float f = h - std::floor(h);
    
    unsigned char v = 255;  // Full value
    unsigned char p = 0;    // S = 1.0, so p = 0
    unsigned char q = static_cast<unsigned char>(255 * (1.0f - f));
    unsigned char t = static_cast<unsigned char>(255 * f);
    
    switch (i) {
        case 0: return Color(v, t, p);
        case 1: return Color(q, v, p);
        case 2: return Color(p, v, t);
        case 3: return Color(p, q, v);
        case 4: return Color(t, p, v);
        case 5: return Color(v, p, q);
        default: return Color(0, 255, 0);  // Default green
    }
}

// ============================================================================
// Phase 1: Scent System
// ============================================================================

std::array<float, 8> Plant::getScentSignature() const {
    std::array<float, 8> signature;
    
    // Core plant traits for scent
    signature[0] = getFruitAppeal();      // How appealing the fruit smells
    signature[1] = getToxicity();         // Toxin warning scent
    signature[2] = getThornDamage();      // Physical defense indicator
    signature[3] = getHardiness();        // Toughness indicator
    
    // Plant-ness indicator (inverted: 1.0 = definitely plant, 0.0 = not plant)
    // This helps creatures distinguish plant scents from creature scents
    signature[4] = 1.0f;  // Plants always have this at 1.0
    
    // Encode plant ID in remaining slots for source identification
    // Use float representation of ID components
    signature[5] = static_cast<float>(id_ % 1000) / 1000.0f;
    signature[6] = static_cast<float>((id_ / 1000) % 1000) / 1000.0f;
    signature[7] = static_cast<float>((id_ / 1000000) % 1000) / 1000.0f;
    
    return signature;
}

float Plant::getScentProductionRate() const {
    // Plants require SCENT_PRODUCTION gene to emit scent
    // Returns 0.0f if the gene is not present
    return getGeneValueFromGenome(UniversalGenes::SCENT_PRODUCTION, 0.0f);
}

// ============================================================================
// Phase 2.3: Serialization
// ============================================================================

std::string Plant::toString() const {
    std::ostringstream ss;
    
    // Format: id,x,y,age,alive,health,current_size,mature,entityType,fruitTimer
    // Note: Full genome serialization to be added in future phase
    ss << id_ << ","
       << x_ << ","
       << y_ << ","
       << age_ << ","
       << (alive_ ? 1 : 0) << ","
       << health_ << ","
       << current_size_ << ","
       << (mature_ ? 1 : 0) << ","
       << static_cast<int>(entityType_) << ","
       << fruitTimer_;
    
    return ss.str();
}

std::unique_ptr<Plant> Plant::fromString(const std::string& data,
                                          const GeneRegistry& registry) {
    std::istringstream ss(data);
    std::string token;
    
    // Parse fields
    unsigned int id, age, fruitTimer;
    int x, y, entityTypeInt;
    bool alive, mature;
    float health, currentSize;
    
    if (!std::getline(ss, token, ',')) return nullptr;
    id = std::stoul(token);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    x = std::stoi(token);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    y = std::stoi(token);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    age = std::stoul(token);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    alive = (std::stoi(token) != 0);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    health = std::stof(token);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    currentSize = std::stof(token);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    mature = (std::stoi(token) != 0);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    entityTypeInt = std::stoi(token);
    
    if (!std::getline(ss, token, ',')) return nullptr;
    fruitTimer = std::stoul(token);
    
    // Create plant with random genome (genome serialization to be added)
    // Note: This creates a new random genome, not the original
    auto plant = std::make_unique<Plant>(x, y, registry);
    plant->setId(id);
    plant->age_ = age;
    plant->alive_ = alive;
    plant->health_ = health;
    plant->current_size_ = currentSize;
    plant->mature_ = mature;
    plant->entityType_ = static_cast<EntityType>(entityTypeInt);
    plant->fruitTimer_ = fruitTimer;
    
    return plant;
}

} // namespace Genetics
} // namespace EcoSim
