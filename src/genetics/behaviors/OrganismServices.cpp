/**
 * @file OrganismServices.cpp
 * @brief Implementation of the OrganismServices dependency injection container
 */

#include "genetics/behaviors/OrganismServices.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/systems/HealthSystem.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "objects/creature/navigator.hpp"
#include "world/world.hpp"

namespace EcoSim {
namespace Genetics {

OrganismServices::OrganismServices(GeneRegistry& registry, 
                                   Navigator& navigator,
                                   World& world)
    : registry_(registry)
    , navigator_(navigator)
    , world_(world)
    , perception_(nullptr)
    , health_(nullptr)
    , feeding_(nullptr)
    , combat_(nullptr)
    , dispersal_(nullptr)
{
}

OrganismServices::~OrganismServices() = default;

OrganismServices::OrganismServices(OrganismServices&& other) noexcept
    : registry_(other.registry_)
    , navigator_(other.navigator_)
    , world_(other.world_)
    , perception_(std::move(other.perception_))
    , health_(std::move(other.health_))
    , feeding_(std::move(other.feeding_))
    , combat_(std::move(other.combat_))
    , dispersal_(std::move(other.dispersal_))
{
}

OrganismServices& OrganismServices::operator=(OrganismServices&& other) noexcept {
    if (this != &other) {
        perception_ = std::move(other.perception_);
        health_ = std::move(other.health_);
        feeding_ = std::move(other.feeding_);
        combat_ = std::move(other.combat_);
        dispersal_ = std::move(other.dispersal_);
    }
    return *this;
}

GeneRegistry& OrganismServices::getGeneRegistry() {
    return registry_;
}

const GeneRegistry& OrganismServices::getGeneRegistry() const {
    return registry_;
}

Navigator& OrganismServices::getNavigator() {
    return navigator_;
}

const Navigator& OrganismServices::getNavigator() const {
    return navigator_;
}

World& OrganismServices::getWorld() {
    return world_;
}

const World& OrganismServices::getWorld() const {
    return world_;
}

void OrganismServices::ensurePerceptionSystem() const {
    if (!perception_) {
        const_cast<OrganismServices*>(this)->perception_ = 
            std::make_unique<PerceptionSystem>();
    }
}

void OrganismServices::ensureHealthSystem() const {
    if (!health_) {
        const_cast<OrganismServices*>(this)->health_ = 
            std::make_unique<HealthSystem>();
    }
}

void OrganismServices::ensureFeedingInteraction() const {
    if (!feeding_) {
        const_cast<OrganismServices*>(this)->feeding_ = 
            std::make_unique<FeedingInteraction>();
    }
}

void OrganismServices::ensureCombatInteraction() const {
    if (!combat_) {
        const_cast<OrganismServices*>(this)->combat_ = 
            std::make_unique<CombatInteraction>();
    }
}

void OrganismServices::ensureSeedDispersal() const {
    if (!dispersal_) {
        const_cast<OrganismServices*>(this)->dispersal_ = 
            std::make_unique<SeedDispersal>();
    }
}

PerceptionSystem& OrganismServices::getPerceptionSystem() {
    ensurePerceptionSystem();
    return *perception_;
}

const PerceptionSystem& OrganismServices::getPerceptionSystem() const {
    ensurePerceptionSystem();
    return *perception_;
}

HealthSystem& OrganismServices::getHealthSystem() {
    ensureHealthSystem();
    return *health_;
}

const HealthSystem& OrganismServices::getHealthSystem() const {
    ensureHealthSystem();
    return *health_;
}

FeedingInteraction& OrganismServices::getFeedingInteraction() {
    ensureFeedingInteraction();
    return *feeding_;
}

const FeedingInteraction& OrganismServices::getFeedingInteraction() const {
    ensureFeedingInteraction();
    return *feeding_;
}

CombatInteraction& OrganismServices::getCombatInteraction() {
    ensureCombatInteraction();
    return *combat_;
}

const CombatInteraction& OrganismServices::getCombatInteraction() const {
    ensureCombatInteraction();
    return *combat_;
}

SeedDispersal& OrganismServices::getSeedDispersal() {
    ensureSeedDispersal();
    return *dispersal_;
}

const SeedDispersal& OrganismServices::getSeedDispersal() const {
    ensureSeedDispersal();
    return *dispersal_;
}

} // namespace Genetics
} // namespace EcoSim
