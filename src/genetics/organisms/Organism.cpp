#include "genetics/organisms/Organism.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/OrganismConstants.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include <algorithm>

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
{
    // Rebind phenotype to point to THIS organism's genome
    rebindPhenotypeGenome();

    // Invalidate moved-from object
    other.alive_ = false;
    other.registry_ = nullptr;
}

Organism& Organism::operator=(Organism&& other) noexcept {
    if (this != &other) {
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
    bool hasResources = heterotrophy_->hunger > Constants::BREED_COST && heterotrophy_->thirst > Constants::BREED_COST;
    bool isHealthy = health_ > getMaxHealth() * 0.25f;
    return isMature() && hasResources && isHealthy && motivation_ == Motivation::Amorous;
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

} // namespace Genetics
} // namespace EcoSim
