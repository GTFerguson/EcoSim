/**
 * @file Plant.cpp
 * @brief Implementation of the Plant class
 *
 * This file implements the plant organism that uses the genetics system.
 * Plants share the same genome infrastructure as creatures but use
 * plant-specific genes defined in PlantGenes.hpp.
 */

#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantEnergyCalculator.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/OrganismState.hpp"
#include "genetics/expression/EnvironmentalStress.hpp"
#include "genetics/core/RandomEngine.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/behaviors/BehaviorController.hpp"
#include "genetics/behaviors/PlantLifecycleTick.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/classification/PlantTaxonomy.hpp"
#include "genetics/components/IdentityComponent.hpp"
#include "genetics/organisms/OrganismFactory.hpp"
#include "logging/Logger.hpp"
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

// Plant constructors delegate all component attachment + archetype
// classification + behavior-controller wiring to OrganismFactory. For
// plant-flavoured genomes this produces Autotrophy + Reproduction +
// Identity (plus BehaviorController with PlantLifecycleTick) — the same
// component set the legacy attachPlantComponents helper produced.
//
// Once call sites migrate to OrganismFactory::fromGenome directly (B.4),
// these constructors disappear and PlantFactory likewise routes through
// the Organism factory.

Plant::Plant(int x, int y, const GeneRegistry& registry)
    : Organism(x, y, PlantGenes::createRandomGenome(registry), registry)
{
    updatePhenotype();
    auto sig = OrganismFactory::classifyComponentSet(genome_);
    OrganismFactory::attachComponents(*this, sig);

    logging::Logger::getInstance().plantSpawned(
        static_cast<unsigned int>(getId()), "random", x_, y_);
}

Plant::Plant(int x, int y, const Genome& genome, const GeneRegistry& registry)
    : Organism(x, y, genome, registry)
{
    updatePhenotype();
    auto sig = OrganismFactory::classifyComponentSet(genome_);
    OrganismFactory::attachComponents(*this, sig);

    logging::Logger::getInstance().plantSpawned(
        static_cast<unsigned int>(getId()), "offspring", x_, y_);
}

// ============================================================================
// Rule of Five: Copy and Move Semantics
// ============================================================================

Plant::Plant(const Plant& other)
    : Organism(other.x_, other.y_, other.genome_, *other.registry_)
    , energyState_(other.energyState_)
    , fruitTimer_(other.fruitTimer_)
{
    // Copy lifecycle state
    if (!other.alive_) {
        die();
    }
    setHealth(other.health_);
    setCurrentSize(other.currentSize_);
    if (other.mature_) {
        setMature(true);
    }

    // Copy age manually since it's not passed through base constructor
    for (unsigned int i = 0; i < other.age_; ++i) {
        incrementAge();
    }

    updatePhenotype();

    // Attach a fresh identity with the same archetype classification; the
    // copy is a distinct living instance, so bump the archetype population.
    attachIdentity(std::make_unique<IdentityComponent>());
    if (other.identity_) {
        identity_->archetype = other.identity_->archetype;
        identity_->biomeAdaptation = other.identity_->biomeAdaptation;
    }
    if (identity_->archetype) {
        identity_->archetype->incrementPopulation();
    }
}

Plant::Plant(Plant&& other) noexcept
    : Organism(std::move(other))
    , energyState_(std::move(other.energyState_))
    , fruitTimer_(other.fruitTimer_)
{
    // Invalidate moved-from object
    other.fruitTimer_ = 0;
}

Plant& Plant::operator=(const Plant& other) {
    if (this != &other) {
        // Decrement old archetype population before reassignment
        if (identity_ && identity_->archetype) {
            identity_->archetype->decrementPopulation();
        }

        // Copy position (set directly on base class protected members)
        x_ = other.x_;
        y_ = other.y_;

        // Copy genetics
        genome_ = other.genome_;
        registry_ = other.registry_;

        // Copy lifecycle state
        age_ = other.age_;
        alive_ = other.alive_;
        health_ = other.health_;
        currentSize_ = other.currentSize_;
        mature_ = other.mature_;

        // Copy Plant-specific state
        energyState_ = other.energyState_;
        fruitTimer_ = other.fruitTimer_;

        // Attach a fresh identity, copying the archetype flyweight pointer
        attachIdentity(std::make_unique<IdentityComponent>());
        if (other.identity_) {
            identity_->archetype = other.identity_->archetype;
            identity_->biomeAdaptation = other.identity_->biomeAdaptation;
        }
        if (identity_->archetype) {
            identity_->archetype->incrementPopulation();
        }

        // Rebind phenotype to THIS plant's genome
        rebindPhenotypeGenome();
        updatePhenotype();
    }
    return *this;
}

Plant& Plant::operator=(Plant&& other) noexcept {
    if (this != &other) {
        // Organism::operator=(&&) decrements the outgoing archetype/biome
        // populations before moving identity_ in from `other`.
        Organism::operator=(std::move(other));

        energyState_ = std::move(other.energyState_);
        fruitTimer_ = other.fruitTimer_;
        other.fruitTimer_ = 0;
    }
    return *this;
}

// ~Plant is Organism's — ~Organism decrements archetype/biome populations
// for any organism holding an identity component.

// ============================================================================
// ILifecycle implementation
// ============================================================================

unsigned int Plant::getMaxLifespan() const {
    // Get lifespan directly from genome (more reliable than phenotype traits)
    // UniversalGenes::LIFESPAN is the foundation - all organisms use it
    // PlantGenes::LIFESPAN is legacy fallback only
    if (genome_.hasGene(UniversalGenes::LIFESPAN)) {
        const Gene& gene = genome_.getGene(UniversalGenes::LIFESPAN);
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

// ============================================================================
// IGenetic implementation
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
// Growth Cache
// ============================================================================

void Plant::ensureGrowthCacheComputed() const {
    if (growthCache_.isComputed) return;
    
    // Compute all growth-related values once from genome
    growthCache_.maxSize = getGeneValueFromGenome(PlantGenes::MAX_SIZE, 5.0f);
    growthCache_.growthRate = getGeneValueFromGenome(PlantGenes::GROWTH_RATE, 0.5f);
    growthCache_.lightNeed = getGeneValueFromGenome(PlantGenes::LIGHT_NEED, 0.5f);
    growthCache_.waterNeed = getGeneValueFromGenome(PlantGenes::WATER_NEED, 0.5f);
    growthCache_.tempToleranceLow = getGeneValueFromGenome(UniversalGenes::TEMP_TOLERANCE_LOW, 5.0f);
    growthCache_.tempToleranceHigh = getGeneValueFromGenome(UniversalGenes::TEMP_TOLERANCE_HIGH, 35.0f);
    growthCache_.waterStorage = getGeneValueFromGenome(UniversalGenes::WATER_STORAGE, 0.0f);
    
    growthCache_.isComputed = true;
}

// ============================================================================
// Plant-specific getters
// ============================================================================

float Plant::getGrowthRate() const {
    return getGeneValueFromGenome(PlantGenes::GROWTH_RATE, 0.5f);
}

float Plant::getMaxSize() const {
    return getGeneValueFromGenome(PlantGenes::MAX_SIZE, 5.0f);
}

float Plant::getNutrientValue() const {
    float baseValue = getGeneValueFromGenome(PlantGenes::NUTRIENT_VALUE, 25.0f);
    float sizeRatio = currentSize_ / getMaxSize();
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
    
    float baseSeedCount = getGeneValueFromGenome(PlantGenes::SEED_PRODUCTION, 5.0f);
    float sizeRatio = currentSize_ / getMaxSize();
    return static_cast<int>(baseSeedCount * sizeRatio * health_);
}

bool Plant::canSpreadSeeds() const {
    return alive_ && mature_ && health_ > 0.5f && getSeedCount() > 0;
}

bool Plant::canSurviveTemperature(float temperature) const {
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

    // Grow and age
    grow(env);
    fruitTimer_++;
    incrementAge();

    // Check death conditions (health-based)
    checkDeathConditions(env);

    // Check for death from old age AFTER other lifecycle events
    float maxLifespan = static_cast<float>(getMaxLifespan());
    if (static_cast<float>(age_) > maxLifespan) {
        die();
        health_ = 0.0f;

        logging::Logger::getInstance().plantDied(
            static_cast<unsigned int>(getId()), "plant", "old_age", age_);
    }
}

void Plant::grow() {
    EnvironmentState defaultEnv;
    grow(defaultEnv);
}

void Plant::grow(const EnvironmentState& env) {
    if (!alive_) return;
    
    // Ensure growth cache is computed (lazy initialization - no-op after first call)
    ensureGrowthCacheComputed();
    
    // Use cached values instead of repeated gene lookups
    float maxSize = growthCache_.maxSize;
    if (currentSize_ >= maxSize) {
        if (!mature_) setMature(true);
        return;
    }
    
    float growthRate = growthCache_.growthRate;
    float lightNeed = growthCache_.lightNeed;
    float waterNeed = growthCache_.waterNeed;
    float tempLow = growthCache_.tempToleranceLow;
    float tempHigh = growthCache_.tempToleranceHigh;
    float waterStorage = growthCache_.waterStorage;
    
    // Calculate combined environmental stress using the stress calculator
    CombinedPlantStress stress = EnvironmentalStressCalculator::calculatePlantStress(
        env, tempLow, tempHigh, waterNeed, waterStorage);
    
    // Apply environmental damage if any (temperature or moisture stress)
    // Gradual damage based on stress levels - not instant death
    float totalDamage = stress.combinedHealthDamage;
    if (totalDamage > 0.0f) {
        // Damage is already per-tick rate, scaled by max health (1.0)
        takeDamage(totalDamage);
    }
    
    // Calculate base growth using PlantEnergyCalculator for light/water factors
    bool canSurviveTemp = stress.temperature.severity != StressLevel::Lethal;
    float effectiveGrowth = PlantEnergyCalculator::calculatePhotosynthesisGrowth(
        env, lightNeed, waterNeed, growthRate, canSurviveTemp);
    
    // Apply combined growth modifier from stress system
    // This reduces growth when stressed by temperature or drought
    effectiveGrowth *= stress.combinedGrowthModifier;
    
    setCurrentSize(std::min(currentSize_ + effectiveGrowth, maxSize));
    
    // Check for maturity (can reproduce when at 50% of max size)
    if (!mature_ && currentSize_ >= maxSize * 0.5f) {
        setMature(true);
    }
}

void Plant::checkDeathConditions(const EnvironmentState& env) {
    // Death by health
    if (health_ <= 0.0f) {
        die();
        
        logging::Logger::getInstance().plantDied(
            static_cast<unsigned int>(getId()), "plant", "health_depleted", age_);
        return;
    }
    
    // Temperature damage is handled in grow()
    (void)env;  // Suppress unused warning
}

void Plant::takeDamage(float amount) {
    if (!alive_) return;
    
    // Reduce damage by hardiness
    float hardiness = getHardiness();
    float actualDamage = amount * (1.0f - hardiness * 0.5f);  // Max 50% damage reduction
    
    health_ -= actualDamage;
    health_ = std::max(0.0f, health_);
    
    if (health_ <= 0.0f) {
        die();
        
        logging::Logger::getInstance().plantDied(
            static_cast<unsigned int>(getId()), "plant", "damage", age_);
    }
}

// ============================================================================
// IReproducible interface implementation
// ============================================================================

bool Plant::canReproduce() const {
    return canSpreadSeeds();
}

float Plant::getReproductiveUrge() const {
    return mature_ && alive_ && health_ > 0.5f ? 1.0f : 0.0f;
}

float Plant::getReproductionEnergyCost() const {
    return PlantEnergyCalculator::calculateReproductionEnergy(getSeedCount());
}

ReproductionMode Plant::getReproductionMode() const {
    return ReproductionMode::ASEXUAL;
}

std::unique_ptr<Organism> Plant::makeOffspring(
    std::unique_ptr<Genome> offspringGenome, int x, int y) {
    if (!offspringGenome || !registry_) return nullptr;
    return std::make_unique<Plant>(x, y, *offspringGenome, *registry_);
}

// ============================================================================
// Defense System
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
    
    float regrowth = getRegrowthRate();
    float surplus = energyState_.currentEnergy - energyState_.maintenanceCost - energyState_.baseMetabolism;
    bool hasEnergy = surplus > 0.0f;
    
    return regrowth > 0.1f && hasEnergy;
}

void Plant::regenerate() {
    if (!canRegenerate()) return;
    
    float regrowthRate = getRegrowthRate();
    
    // Regenerate health proportional to regrowth rate
    float healthGain = regrowthRate * 0.01f;
    health_ = std::min(1.0f, health_ + healthGain);
    
    // Also regenerate size if damaged
    float maxSize = getMaxSize();
    if (currentSize_ < maxSize * 0.9f) {
        setCurrentSize(std::min(maxSize, currentSize_ + regrowthRate * 0.001f));
    }
}

// ============================================================================
// Food Production
// ============================================================================

bool Plant::canProduceFruit() const {
    if (!alive_ || !mature_) return false;
    
    float maxSize = getMaxSize();
    if (currentSize_ < maxSize * 0.5f) return false;
    
    float maturityAge = getMaxLifespan() * 0.10f;
    if (age_ < static_cast<unsigned int>(maturityAge)) return false;
    
    float fruitRate = getFruitProductionRate();
    if (fruitRate <= 0.01f) return false;
    
    unsigned int cooldown = static_cast<unsigned int>(100.0f / (fruitRate + 0.1f));
    return fruitTimer_ >= cooldown;
}

float Plant::getFruitProductionRate() const {
    return getGeneValueFromGenome(UniversalGenes::FRUIT_PRODUCTION_RATE, 0.3f);
}

bool Plant::canSpreadVegetatively() const {
    if (!alive_ || !mature_) return false;
    
    float maxSize = getMaxSize();
    if (currentSize_ < maxSize * 0.5f) return false;
    
    float maturityAge = getMaxLifespan() * 0.10f;
    if (age_ < static_cast<unsigned int>(maturityAge)) return false;
    
    float runnerProd = getRunnerProduction();
    if (runnerProd < 0.5f) return false;
    
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
// Seed Properties
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

float Plant::getExplosivePodForce() const {
    return getGeneValueFromGenome(UniversalGenes::EXPLOSIVE_POD_FORCE, 0.0f);
}

// ============================================================================
// Emergent Dispersal Strategy
// ============================================================================

DispersalStrategy Plant::getPrimaryDispersalStrategy() const {
    struct StrategyScore {
        DispersalStrategy strategy;
        float score;
    };
    
    std::vector<StrategyScore> scores = {
        {DispersalStrategy::VEGETATIVE, getRunnerProduction()},
        {DispersalStrategy::EXPLOSIVE, getExplosivePodForce()},
        {DispersalStrategy::WIND, (1.0f - getSeedMass()) * getSeedAerodynamics()},
        {DispersalStrategy::ANIMAL_BURR, getSeedHookStrength()},
        {DispersalStrategy::ANIMAL_FRUIT, getFruitAppeal() * getSeedCoatDurability()},
    };
    
    auto best = std::max_element(scores.begin(), scores.end(),
        [](const auto& a, const auto& b) { return a.score < b.score; });
    
    const float MIN_STRATEGY_THRESHOLD = 0.1f;
    
    if (best != scores.end() && best->score >= MIN_STRATEGY_THRESHOLD) {
        return best->strategy;
    }
    
    return DispersalStrategy::GRAVITY;
}

// ============================================================================
// Energy Budget Integration
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
// Rendering Support
// ============================================================================

Color Plant::getRenderColor() const {
    float hue = getColorHue();
    
    float h = hue / 60.0f;
    int i = static_cast<int>(h) % 6;
    float f = h - std::floor(h);
    
    unsigned char v = 255;
    unsigned char p = 0;
    unsigned char q = static_cast<unsigned char>(255 * (1.0f - f));
    unsigned char t = static_cast<unsigned char>(255 * f);
    
    switch (i) {
        case 0: return Color(v, t, p);
        case 1: return Color(q, v, p);
        case 2: return Color(p, v, t);
        case 3: return Color(p, q, v);
        case 4: return Color(t, p, v);
        case 5: return Color(v, p, q);
        default: return Color(0, 255, 0);
    }
}

// ============================================================================
// Scent System
// ============================================================================

std::array<float, 8> Plant::getScentSignature() const {
    std::array<float, 8> signature;
    
    signature[0] = getFruitAppeal();
    signature[1] = getToxicity();
    signature[2] = getThornDamage();
    signature[3] = getHardiness();
    signature[4] = 1.0f;  // Plants always have this at 1.0
    
    // Encode plant ID in remaining slots
    unsigned int id = static_cast<unsigned int>(getId());
    signature[5] = static_cast<float>(id % 1000) / 1000.0f;
    signature[6] = static_cast<float>((id / 1000) % 1000) / 1000.0f;
    signature[7] = static_cast<float>((id / 1000000) % 1000) / 1000.0f;
    
    return signature;
}

float Plant::getScentProductionRate() const {
    return getGeneValueFromGenome(UniversalGenes::SCENT_PRODUCTION, 0.0f);
}

// ============================================================================
// String Serialization
// ============================================================================

std::string Plant::toString() const {
    return toJson().dump();
}

std::unique_ptr<Plant> Plant::fromString(const std::string& data,
                                          const GeneRegistry& registry) {
    try {
        nlohmann::json j = nlohmann::json::parse(data);
        Plant plant = fromJson(j, registry);
        return std::make_unique<Plant>(std::move(plant));
    } catch (const nlohmann::json::exception& e) {
        return nullptr;
    }
}

// ============================================================================
// JSON Serialization
// ============================================================================

std::string Plant::dispersalStrategyToString(DispersalStrategy strategy) {
    switch (strategy) {
        case DispersalStrategy::GRAVITY:      return "GRAVITY";
        case DispersalStrategy::WIND:         return "WIND";
        case DispersalStrategy::ANIMAL_FRUIT: return "ANIMAL_FRUIT";
        case DispersalStrategy::ANIMAL_BURR:  return "ANIMAL_BURR";
        case DispersalStrategy::EXPLOSIVE:    return "EXPLOSIVE";
        case DispersalStrategy::VEGETATIVE:   return "VEGETATIVE";
        default:                              return "GRAVITY";
    }
}

DispersalStrategy Plant::stringToDispersalStrategy(const std::string& str) {
    if (str == "GRAVITY")      return DispersalStrategy::GRAVITY;
    if (str == "WIND")         return DispersalStrategy::WIND;
    if (str == "ANIMAL_FRUIT") return DispersalStrategy::ANIMAL_FRUIT;
    if (str == "ANIMAL_BURR")  return DispersalStrategy::ANIMAL_BURR;
    if (str == "EXPLOSIVE")    return DispersalStrategy::EXPLOSIVE;
    if (str == "VEGETATIVE")   return DispersalStrategy::VEGETATIVE;
    return DispersalStrategy::GRAVITY;
}

nlohmann::json Plant::toJson() const {
    nlohmann::json j;

    // Identity
    j["id"] = getId();

    // Species name and entity type come from the archetype flyweight.
    j["speciesName"] = getName();
    
    // Position
    j["position"] = {
        {"tileX", x_},
        {"tileY", y_},
        {"worldX", getWorldX()},
        {"worldY", getWorldY()}
    };
    
    // Lifecycle
    std::string stage;
    if (!alive_) {
        stage = "DEAD";
    } else if (mature_) {
        stage = "MATURE";
    } else if (currentSize_ >= getMaxSize() * 0.25f) {
        stage = "GROWING";
    } else {
        stage = "SEEDLING";
    }
    
    j["lifecycle"] = {
        {"stage", stage},
        {"currentAge", age_},
        {"stageAge", age_},
        {"isAlive", alive_}
    };
    
    // Resources
    j["resources"] = {
        {"storedEnergy", energyState_.currentEnergy},
        {"currentBiomass", currentSize_},
        {"currentHeight", currentSize_},
        {"health", health_}
    };
    
    // Reproduction
    j["reproduction"] = {
        {"hasSeeds", canSpreadSeeds()},
        {"seedCount", getSeedCount()},
        {"seedsProduced", 0},
        {"ticksSinceLastSeed", fruitTimer_},
        {"dispersalStrategy", dispersalStrategyToString(getPrimaryDispersalStrategy())},
        {"mature", mature_}
    };
    
    // Entity type stored for debugging / legacy readers; canonical value on
    // load is re-derived by PlantTaxonomy from the saved genome.
    j["entityType"] = static_cast<int>(getEntityType());

    // Genome
    j["genome"] = genome_.toJson();
    
    return j;
}

Plant Plant::fromJson(const nlohmann::json& j, const GeneRegistry& registry) {
    int tileX = j.at("position").at("tileX").get<int>();
    int tileY = j.at("position").at("tileY").get<int>();
    
    Genome genome = Genome::fromJson(j.at("genome"));
    
    Plant plant(tileX, tileY, genome, registry);
    
    // Restore lifecycle state
    if (j.contains("lifecycle")) {
        const auto& lifecycle = j.at("lifecycle");
        
        // Restore age
        unsigned int savedAge = lifecycle.value("currentAge", 0u);
        for (unsigned int i = 0; i < savedAge; ++i) {
            plant.incrementAge();
        }
        
        if (!lifecycle.value("isAlive", true)) {
            plant.die();
        }
    }
    
    // Restore resources
    if (j.contains("resources")) {
        const auto& resources = j.at("resources");
        plant.health_ = resources.value("health", 1.0f);
        plant.setCurrentSize(resources.value("currentBiomass", 0.1f));
        plant.energyState_.currentEnergy = resources.value("storedEnergy", 0.0f);
    }
    
    // Restore reproduction state
    if (j.contains("reproduction")) {
        const auto& repro = j.at("reproduction");
        plant.fruitTimer_ = repro.value("ticksSinceLastSeed", 0u);
        if (repro.value("mature", false)) {
            plant.setMature(true);
        }
    }
    
    // Entity type and archetype are re-derived from the genome by
    // attachPlantComponents(); the saved "entityType" field is retained
    // in the JSON for legacy/diagnostic readers but not consumed here.

    plant.updatePhenotype();
    
    return plant;
}

} // namespace Genetics
} // namespace EcoSim
