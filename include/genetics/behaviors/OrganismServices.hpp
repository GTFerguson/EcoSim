/**
 * @file OrganismServices.hpp
 * @brief Dependency injection container for shared services used by organism behaviors
 *
 * Provides a centralized access point for services that behaviors need, enabling:
 * - Loose coupling between behaviors and concrete service implementations
 * - Easier testing through service mocking
 * - Consistent service initialization and lifetime management
 *
 * This is part of the Creature God Class decomposition effort.
 *
 * @see docs/technical/core/01-architecture.md
 */

#pragma once

#include <memory>

namespace EcoSim {

class Navigator;
class ScentLayer;
class World;

namespace Genetics {

class GeneRegistry;
class PerceptionSystem;
class HealthSystem;
class FeedingInteraction;
class CombatInteraction;
class SeedDispersal;

/**
 * @brief Dependency injection container for organism behavior services
 *
 * Aggregates all shared services that behaviors may need during execution.
 * Services are created lazily on first access and cached for the container's
 * lifetime.
 *
 * Usage:
 * @code
 *   OrganismServices services(registry, navigator, world);
 *   
 *   // Access services as needed
 *   HealthSystem& health = services.getHealthSystem();
 *   PerceptionSystem& perception = services.getPerceptionSystem();
 * @endcode
 *
 * @note This class owns the PerceptionSystem, HealthSystem, FeedingInteraction,
 * CombatInteraction, and SeedDispersal instances. External services (GeneRegistry,
 * Navigator, World) are referenced but not owned.
 */
class OrganismServices {
public:
    /**
     * @brief Construct with required external dependencies
     * @param registry Gene registry for gene lookups
     * @param navigator Navigator for pathfinding
     * @param world World for entity queries
     */
    OrganismServices(GeneRegistry& registry, 
                     Navigator& navigator,
                     World& world);
    
    /**
     * @brief Destructor - cleans up owned services
     */
    ~OrganismServices();
    
    OrganismServices(const OrganismServices&) = delete;
    OrganismServices& operator=(const OrganismServices&) = delete;
    
    OrganismServices(OrganismServices&&) noexcept;
    OrganismServices& operator=(OrganismServices&&) noexcept;
    
    /**
     * @brief Get the gene registry
     * @return Reference to the shared GeneRegistry
     */
    GeneRegistry& getGeneRegistry();
    const GeneRegistry& getGeneRegistry() const;
    
    /**
     * @brief Get the navigator for pathfinding
     * @return Reference to the shared Navigator
     */
    Navigator& getNavigator();
    const Navigator& getNavigator() const;
    
    /**
     * @brief Get the world for entity queries
     * @return Reference to the World
     */
    World& getWorld();
    const World& getWorld() const;
    
    /**
     * @brief Get the perception system
     * @return Reference to the PerceptionSystem (created on first access)
     */
    PerceptionSystem& getPerceptionSystem();
    const PerceptionSystem& getPerceptionSystem() const;
    
    /**
     * @brief Get the health system
     * @return Reference to the HealthSystem (created on first access)
     */
    HealthSystem& getHealthSystem();
    const HealthSystem& getHealthSystem() const;
    
    /**
     * @brief Get the feeding interaction calculator
     * @return Reference to the FeedingInteraction (created on first access)
     */
    FeedingInteraction& getFeedingInteraction();
    const FeedingInteraction& getFeedingInteraction() const;
    
    /**
     * @brief Get the combat interaction calculator
     * @return Reference to the CombatInteraction (created on first access)
     */
    CombatInteraction& getCombatInteraction();
    const CombatInteraction& getCombatInteraction() const;
    
    /**
     * @brief Get the seed dispersal calculator
     * @return Reference to the SeedDispersal (created on first access)
     */
    SeedDispersal& getSeedDispersal();
    const SeedDispersal& getSeedDispersal() const;

private:
    GeneRegistry& registry_;
    Navigator& navigator_;
    World& world_;
    
    std::unique_ptr<PerceptionSystem> perception_;
    std::unique_ptr<HealthSystem> health_;
    std::unique_ptr<FeedingInteraction> feeding_;
    std::unique_ptr<CombatInteraction> combat_;
    std::unique_ptr<SeedDispersal> dispersal_;
    
    void ensurePerceptionSystem() const;
    void ensureHealthSystem() const;
    void ensureFeedingInteraction() const;
    void ensureCombatInteraction() const;
    void ensureSeedDispersal() const;
};

} // namespace Genetics
} // namespace EcoSim
