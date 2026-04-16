#include "genetics/organisms/Organism.hpp"
#include "genetics/organisms/OrganismFactory.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/classification/BiomeAdaptation.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/OrganismConstants.hpp"
#include "genetics/core/RandomEngine.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace EcoSim {
namespace Genetics {

// Static member initialization
int Organism::nextId_ = 1;
int Organism::nextCreatureId_ = 0;

std::shared_ptr<GeneRegistry>     Organism::s_geneRegistry     = nullptr;
std::unique_ptr<FeedingInteraction>  Organism::s_feedingInteraction = nullptr;
std::unique_ptr<SeedDispersal>       Organism::s_seedDispersal     = nullptr;
std::unique_ptr<PerceptionSystem>    Organism::s_perceptionSystem  = nullptr;
std::unique_ptr<CombatInteraction>   Organism::s_combatInteraction = nullptr;

Organism::Organism(int x, int y, Genome genome, const GeneRegistry& registry)
    : x_(x)
    , y_(y)
    , age_(0)
    , alive_(true)
    , health_(1.0f)
    , currentSize_(0.1f)
    , maxSize_(1.0f)
    , mature_(false)
    , genome_(std::move(genome))
    , phenotype_(&genome_, &registry)
    , registry_(&registry)
    , id_(nextId_++)
{
}

std::unique_ptr<Organism> Organism::makeOffspring(
    std::unique_ptr<Genome> offspringGenome, int x, int y) {
    if (!offspringGenome || !registry_) return nullptr;
    return OrganismFactory::fromGenome(
        std::move(*offspringGenome), x, y, *registry_);
}

Organism::~Organism() {
    // Release archetype / biome flyweight population counts this
    // organism was holding. Moved-from organisms have identity_ reset
    // to nullptr by the move ctor/assignment, so they skip this path.
    if (identity_) {
        if (identity_->archetype) {
            identity_->archetype->decrementPopulation();
        }
        if (identity_->biomeAdaptation) {
            identity_->biomeAdaptation->decrementPopulation();
        }
    }
}

Organism::Organism(Organism&& other) noexcept
    : x_(other.x_)
    , y_(other.y_)
    , age_(other.age_)
    , alive_(other.alive_)
    , health_(other.health_)
    , currentSize_(other.currentSize_)
    , maxSize_(other.maxSize_)
    , mature_(other.mature_)
    , genome_(std::move(other.genome_))
    , phenotype_(std::move(other.phenotype_))
    , registry_(other.registry_)
    , id_(other.id_)
    , motivation_(other.motivation_)
    , action_(other.action_)
    , mobility_(std::move(other.mobility_))
    , heterotrophy_(std::move(other.heterotrophy_))
    , autotrophy_(std::move(other.autotrophy_))
    , reproduction_(std::move(other.reproduction_))
    , combat_(std::move(other.combat_))
    , thermal_(std::move(other.thermal_))
    , identity_(std::move(other.identity_))
    , organismBehaviorController_(std::move(other.organismBehaviorController_))
    , pendingOffspring_(std::move(other.pendingOffspring_))
{
    // Rebind phenotype to point to THIS organism's genome
    rebindPhenotypeGenome();

    // Invalidate moved-from object
    other.alive_ = false;
    other.registry_ = nullptr;
}

Organism& Organism::operator=(Organism&& other) noexcept {
    if (this != &other) {
        // The identity we're about to overwrite holds a live reference to
        // an ArchetypeIdentity (and optional BiomeAdaptation) flyweight.
        // Release those population counts before the move replaces the
        // pointer; the identity coming in from `other` carries its own
        // live reference that stays valid on this object post-move.
        if (identity_ && identity_->archetype) {
            identity_->archetype->decrementPopulation();
        }
        if (identity_ && identity_->biomeAdaptation) {
            identity_->biomeAdaptation->decrementPopulation();
        }

        x_ = other.x_;
        y_ = other.y_;
        age_ = other.age_;
        alive_ = other.alive_;
        health_ = other.health_;
        currentSize_ = other.currentSize_;
        maxSize_ = other.maxSize_;
        mature_ = other.mature_;
        genome_ = std::move(other.genome_);
        phenotype_ = std::move(other.phenotype_);
        registry_ = other.registry_;
        id_ = other.id_;
        motivation_ = other.motivation_;
        action_ = other.action_;
        mobility_     = std::move(other.mobility_);
        heterotrophy_ = std::move(other.heterotrophy_);
        autotrophy_   = std::move(other.autotrophy_);
        reproduction_ = std::move(other.reproduction_);
        combat_       = std::move(other.combat_);
        thermal_      = std::move(other.thermal_);
        identity_     = std::move(other.identity_);
        organismBehaviorController_ = std::move(other.organismBehaviorController_);
        pendingOffspring_ = std::move(other.pendingOffspring_);

        // Rebind phenotype to point to THIS organism's genome
        rebindPhenotypeGenome();

        // Invalidate moved-from object
        other.alive_ = false;
        other.registry_ = nullptr;
    }
    return *this;
}

void Organism::age(unsigned int ticks) {
    age_ += ticks;
    
    // Check for death by old age
    if (age_ >= getMaxLifespan()) {
        die();
    }
}

float Organism::getAgeNormalized() const {
    unsigned int maxLifespan = getMaxLifespan();
    if (maxLifespan == 0) {
        return 0.0f;
    }
    return static_cast<float>(age_) / static_cast<float>(maxLifespan);
}

void Organism::updatePhenotype() {
    if (registry_) {
        phenotype_.invalidateCache();
    }
}

unsigned int Organism::getMaxLifespan() const {
    return getLifespan();
}

bool Organism::canReproduce() const {
    if (!heterotrophy_) return false;
    // Minimum resources to bear the cost of reproduction. Relaxed from
    // strict BREED_COST (3.0) — creatures with ~30% hunger can still
    // reproduce, at the expense of post-breed starvation risk.
    const float minEnergy = Constants::BREED_COST * 0.5f;
    bool hasResources = heterotrophy_->hunger  > minEnergy
                     && heterotrophy_->thirst > minEnergy;
    bool isHealthy = health_ > getMaxHealth() * 0.25f;
    // Reproductive urge (mate accumulator) must be positive.
    bool hasUrge = !reproduction_ || reproduction_->mate > 0.0f;
    return isMature() && hasResources && isHealthy && hasUrge;
}

float Organism::getReproductiveUrge() const {
    if (!reproduction_) return 0.0f;
    float urge = (reproduction_->mate + 3.0f) / (Constants::RESOURCE_LIMIT + 3.0f);
    return std::max(0.0f, std::min(1.0f, urge));
}

float Organism::getReproductionEnergyCost() const {
    return Constants::BREED_COST;
}

ReproductionMode Organism::getReproductionMode() const {
    return ReproductionMode::SEXUAL;
}

bool Organism::isCompatibleWith(const Organism& other) const {
    if (getReproductionMode() != other.getReproductionMode()) return false;
    if (getReproductionMode() == ReproductionMode::ASEXUAL)   return false;
    // Sexual partners need non-trivial genetic similarity. Threshold 0.3
    // matches the historical Creature::isCompatibleWith behaviour.
    return genome_.compare(other.genome_) > 0.3f;
}

std::unique_ptr<Organism> Organism::reproduce(const Organism* partner) {
    if (!canReproduce()) return nullptr;
    // Dispatch by reproduction mode rather than partner presence:
    // sexual organisms require a partner (no asexual fallback), and
    // asexual organisms ignore any partner that's passed in.
    if (getReproductionMode() == ReproductionMode::SEXUAL) {
        if (!partner) return nullptr;
        return sexualBreed(const_cast<Organism&>(*partner));
    }
    return asexualBreed();
}

std::unique_ptr<Organism> Organism::sexualBreed(Organism& mate) {
    if (!registry_) return nullptr;

    // Charge breeding cost to both parents (heterotrophs only). The
    // reduced-fraction cost matches the previous Creature::breedCreature
    // tuning — full BREED_COST starves creatures hovering at minimum.
    const float cost = Constants::BREED_COST * 0.3f;
    if (heterotrophy_) {
        heterotrophy_->hunger -= cost;
        heterotrophy_->thirst -= cost;
    }
    if (mate.heterotrophy_) {
        mate.heterotrophy_->hunger -= cost;
        mate.heterotrophy_->thirst -= cost;
    }

    // Seed offspring's hunger/thirst from a share of each parent's
    // current resources. Plants don't carry these so the totals stay 0
    // and the offspring uses its constructor defaults.
    float hunger = 0.0f, thirst = 0.0f;
    if (heterotrophy_) {
        hunger += shareResource(Constants::RESOURCE_SHARED, heterotrophy_->hunger);
        thirst += shareResource(Constants::RESOURCE_SHARED, heterotrophy_->thirst);
    }
    if (mate.heterotrophy_) {
        hunger += shareResource(Constants::RESOURCE_SHARED, mate.heterotrophy_->hunger);
        thirst += shareResource(Constants::RESOURCE_SHARED, mate.heterotrophy_->thirst);
    }
    hunger = std::min(hunger, Constants::RESOURCE_LIMIT);
    thirst = std::min(thirst, Constants::RESOURCE_LIMIT);

    // Reset mate drive on both parents so they don't immediately re-mate.
    if (reproduction_)      reproduction_->mate = 0.0f;
    if (mate.reproduction_) mate.reproduction_->mate = 0.0f;

    // Genome crossover + mutation.
    Genome crossed = Genome::crossover(genome_, mate.genome_, 0.5f);
    crossed.mutate(0.05f, registry_->getAllDefinitions());

    auto offspring = makeOffspring(
        std::make_unique<Genome>(std::move(crossed)), x_, y_);

    if (offspring && offspring->heterotrophy()) {
        offspring->heterotrophy()->hunger = hunger;
        offspring->heterotrophy()->thirst = thirst;
    }
    return offspring;
}

// Resource-sharing helpers — used by sexualBreed and (historically) by
// the legacy creature plant-feeding/seeding paths. Lifted here so the
// genetics library can link them without depending on ecosim_core.
float Organism::shareResource(const int& amount, float& resource) {
    (void)amount;  // legacy parameter — divisor is fixed at RESOURCE_SHARED
    if (resource <= 0.0f) return 0.0f;
    float shared = resource / Constants::RESOURCE_SHARED;
    resource -= shared;
    return shared;
}

float Organism::shareFood(const int& amount) {
    if (!heterotrophy_) return 0.0f;
    return shareResource(amount, heterotrophy_->hunger);
}

float Organism::shareWater(const int& amount) {
    if (!heterotrophy_) return 0.0f;
    return shareResource(amount, heterotrophy_->thirst);
}

std::unique_ptr<Organism> Organism::asexualBreed() {
    if (!registry_) return nullptr;

    Genome offspring = genome_;
    offspring.mutate(0.05f, registry_->getAllDefinitions());

    int childX = x_;
    int childY = y_;
    float spread = getOffspringSpreadDistance();
    if (spread > 0.0f) {
        float angle = RandomEngine::randomFloat(0.0f, 2.0f * 3.14159265f);
        float distance = RandomEngine::randomFloat(1.0f, spread);
        childX = x_ + static_cast<int>(std::cos(angle) * distance);
        childY = y_ + static_cast<int>(std::sin(angle) * distance);
    }

    return makeOffspring(
        std::make_unique<Genome>(std::move(offspring)), childX, childY);
}

void Organism::setWorldPosition(float x, float y) {
    if (mobility_) { mobility_->worldX = x; mobility_->worldY = y; }
    x_ = static_cast<int>(x);
    y_ = static_cast<int>(y);
}

float Organism::getWorldX() const {
    return mobility_ ? mobility_->worldX : (static_cast<float>(x_) + 0.5f);
}

float Organism::getWorldY() const {
    return mobility_ ? mobility_->worldY : (static_cast<float>(y_) + 0.5f);
}

void Organism::grow() {
    if (mature_) return;
    float nutritionFactor = 0.5f;  // baseline for unfed organisms
    if (heterotrophy_) {
        // Hunger is on 0-RESOURCE_LIMIT (0-10) scale. Normalise against
        // RESOURCE_LIMIT to get a 0-1 nutrition factor (prior code used
        // /100 which always clamped to minimum).
        nutritionFactor = std::clamp(
            heterotrophy_->hunger / Constants::RESOURCE_LIMIT, 0.2f, 1.5f);
    }
    float baseGrowthRate = 0.001f;
    float ageFactor = (age_ < 1000) ? 1.5f : 1.0f;
    float growthIncrement = baseGrowthRate * nutritionFactor * ageFactor;
    currentSize_ = std::min(currentSize_ + growthIncrement, maxSize_);
    if (!mature_ && currentSize_ >= maxSize_ * 0.5f) {
        mature_ = true;
    }
}

unsigned Organism::getLifespan() const {
    if (phenotype_.hasTrait(UniversalGenes::LIFESPAN)) {
        return static_cast<unsigned>(phenotype_.getTrait(UniversalGenes::LIFESPAN));
    }
    return 500000;
}

float Organism::getMaxHealth() const {
    // MAX_SIZE-based scaling — larger organisms have more health.
    // Historical Creature override, now the single canonical version.
    if (phenotype_.hasTrait("max_size")) {
        return phenotype_.getTrait("max_size") * 10.0f;
    }
    return 100.0f;  // Fallback default
}

void Organism::setHealth(float health) {
    float maxHealth = getMaxHealth();
    health_ = std::max(0.0f, std::min(health, maxHealth));
    
    if (health_ <= 0.0f) {
        die();
    }
}

void Organism::damage(float amount) {
    if (amount > 0.0f) {
        setHealth(health_ - amount);
    }
}

void Organism::heal(float amount) {
    if (amount > 0.0f && alive_) {
        setHealth(health_ + amount);
    }
}

void Organism::rebindPhenotypeGenome() {
    // The Phenotype needs to reference this organism's genome after move
    // Phenotype::setGenome() updates the internal genome pointer
    phenotype_.setGenome(&genome_);
}

// ============================================================================
// Display / identity getters — flyweight-backed replacement for the former
// GameObject base class API.
// ============================================================================

const std::string& Organism::getName() const {
    if (identity_) {
        if (!identity_->speciesName.empty()) return identity_->speciesName;
        if (identity_->archetype) return identity_->archetype->getLabel();
    }
    static const std::string empty;
    return empty;
}

char Organism::getChar() const {
    if (identity_ && identity_->archetype) return identity_->archetype->getRenderChar();
    return '?';
}

EntityType Organism::getEntityType() const {
    if (identity_ && identity_->archetype) return identity_->archetype->getEntityType();
    return EntityType::CREATURE;
}

bool Organism::getPassable() const {
    if (identity_ && identity_->archetype) return identity_->archetype->getPassable();
    return true;
}

const std::string& Organism::getDesc() const {
    if (identity_ && identity_->archetype) return identity_->archetype->getDesc();
    static const std::string empty;
    return empty;
}

unsigned int Organism::getColour() const {
    // Plants and creatures both drive their rendering colour from the
    // color_hue trait via phenotype. When the gene is absent (defensive
    // path during construction), fall back to the legacy default.
    if (phenotype_.hasTrait(UniversalGenes::COLOR_HUE)) {
        return static_cast<unsigned int>(phenotype_.getTrait(UniversalGenes::COLOR_HUE));
    }
    return 1;
}

std::string Organism::toString() const {
    std::ostringstream ss;
    ss  << "\"" << getName()  << "\","
        << "\"" << getDesc()  << "\","
        << "\"" << getChar()  << "\","
        << getColour()   << ","
        << getPassable();
    return ss.str();
}

} // namespace Genetics
} // namespace EcoSim
