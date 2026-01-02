#include "genetics/organisms/Organism.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include <algorithm>

namespace EcoSim {
namespace Genetics {

// Static member initialization
int Organism::nextId_ = 1;

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

float Organism::getMaxHealth() const {
    // Default max health is 1.0, subclasses may override based on genes
    return 1.0f;
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
