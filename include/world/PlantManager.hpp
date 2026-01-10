#pragma once
/**
 * @file PlantManager.hpp
 * @brief Manages genetics-based plant infrastructure
 */

#include "WorldGrid.hpp"
#include "ScentLayer.hpp"
#include "ClimateWorldGenerator.hpp"
#include "../genetics/core/GeneRegistry.hpp"
#include "../genetics/organisms/Plant.hpp"
#include "../genetics/organisms/PlantFactory.hpp"
#include "../genetics/defaults/UniversalGenes.hpp"
#include "../genetics/expression/EnvironmentState.hpp"
#include "../genetics/interactions/SeedDispersal.hpp"

#include <memory>
#include <string>
#include <random>
#include <vector>
#include <iostream>
#include <functional>

namespace EcoSim {

// Forward declarations
class EnvironmentSystem;

namespace Genetics {
    class BiomeVariantFactory;
}

/**
 * @class PlantManager
 * @brief Manages plant creation, lifecycle, and seed dispersal
 * 
 * Responsibilities:
 * - Plant system initialization (registry, factory, templates)
 * - Adding plants to the world (individual and bulk)
 * - Plant lifecycle updates (growth, fruit production, death)
 * - Seed dispersal and reproduction
 * - Environment state management
 * - Plant scent emission
 */
class PlantManager {
public:
    //==========================================================================
    // Construction
    //==========================================================================
    
    /**
     * @brief Construct a PlantManager
     * @param grid Reference to the world grid for plant placement
     * @param scents Reference to the scent layer for plant odors
     */
    PlantManager(WorldGrid& grid, ScentLayer& scents);
    
    /**
     * @brief Destructor - defined in .cpp for unique_ptr with forward-declared type
     */
    ~PlantManager();
    
    /**
     * @brief Set the environment system for per-tile environment queries
     * @param envSystem Pointer to the environment system (non-owning)
     *
     * When set, plants will receive location-specific environmental data
     * (temperature, moisture, light) based on the climate map. If not set
     * or nullptr, falls back to the global _currentEnvironment.
     */
    void setEnvironmentSystem(const EnvironmentSystem* envSystem);
    
    //==========================================================================
    // Initialization
    //==========================================================================
    
    /**
     * @brief Initialize the plant factory and gene registry
     * 
     * Creates the gene registry, registers default genes, creates the plant
     * factory, and registers default species templates (berry_bush, oak_tree,
     * grass, thorn_bush).
     */
    void initialize();
    
    /**
     * @brief Check if the plant system is initialized
     * @return true if factory and registry are ready
     */
    bool isInitialized() const;
    
    //==========================================================================
    // Population
    //==========================================================================
    
    /**
     * @brief Add plants throughout the world within elevation range
     * @param lowElev Minimum elevation for plant placement
     * @param highElev Maximum elevation for plant placement
     * @param rate Percentage chance (1-100) of plant placement per tile
     * @param species Species template name (e.g., "berry_bush", "oak_tree")
     */
    void addPlants(unsigned lowElev, unsigned highElev,
                   unsigned rate, const std::string& species);
    
    /**
     * @brief Add a single plant at a specific location
     * @param x X coordinate
     * @param y Y coordinate
     * @param species Species template name
     * @return true if plant was added successfully
     */
    bool addPlant(int x, int y, const std::string& species);
    
    /**
     * @brief Add plants based on biome types throughout the world
     * @param rate Percentage chance (1-100) of plant placement per eligible tile
     *
     * Uses BiomeVariantFactory to create biome-appropriate plants:
     * - Tundra/Taiga/Ice: Tundra moss (cold-adapted ground cover)
     * - Desert/Steppe: Desert cactus (heat-tolerant succulents)
     * - Tropical/Savanna: Rainforest vine (lush fruit-bearers)
     * - Temperate/Forest: Mixed berry_bush, oak_tree, grass, thorn_bush
     *
     * Water biomes (ocean, freshwater, rivers, lakes) are skipped.
     * Requires EnvironmentSystem to be connected for biome data.
     */
    void addPlantsByBiome(unsigned rate);
    
    /**
     * @brief Add a biome-appropriate plant at a specific location
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if plant was added successfully
     *
     * Queries the biome at the location and creates an appropriate plant variant.
     * Requires EnvironmentSystem to be connected for biome data.
     */
    bool addBiomePlant(int x, int y);
    
    //==========================================================================
    // Lifecycle
    //==========================================================================
    
    /**
     * @brief Main update loop - process all plants
     * @param currentTick The current simulation tick (for scent timestamps)
     * 
     * Handles:
     * - Plant growth updates via environment state
     * - Dead plant removal
     * - Scent emission from fruiting plants
     * - Seed dispersal and offspring spawning
     */
    void tick(unsigned currentTick);
    
    //==========================================================================
    // Environment
    //==========================================================================
    
    /**
     * @brief Get the current environment state
     * @return Mutable reference to environment state
     */
    Genetics::EnvironmentState& environment();
    
    /**
     * @brief Get the current environment state (const)
     * @return Const reference to environment state
     */
    const Genetics::EnvironmentState& environment() const;
    
    /**
     * @brief Update the environment state
     * @param temperature Current temperature
     * @param lightLevel Light level (0.0 to 1.0, mapped to time_of_day)
     * @param waterAvailability Water availability (0.0 to 1.0, mapped to humidity)
     */
    void updateEnvironment(float temperature, float lightLevel, float waterAvailability);
    
    //==========================================================================
    // Access
    //==========================================================================
    
    /**
     * @brief Get the plant factory (for direct plant creation)
     * @return Pointer to plant factory, or nullptr if not initialized
     */
    Genetics::PlantFactory* factory();
    
    /**
     * @brief Get the plant factory (const)
     * @return Const pointer to plant factory, or nullptr if not initialized
     */
    const Genetics::PlantFactory* factory() const;
    
    /**
     * @brief Get the plant gene registry
     * @return Shared pointer to the gene registry
     */
    std::shared_ptr<Genetics::GeneRegistry> registry();
    
    /**
     * @brief Get the plant gene registry (const)
     * @return Const shared pointer to the gene registry
     */
    std::shared_ptr<const Genetics::GeneRegistry> registry() const;

private:
    WorldGrid& _grid;
    ScentLayer& _scents;
    const EnvironmentSystem* _environmentSystem = nullptr;
    
    std::shared_ptr<Genetics::GeneRegistry> _plantRegistry;
    std::unique_ptr<Genetics::PlantFactory> _plantFactory;
    std::unique_ptr<Genetics::BiomeVariantFactory> _biomeFactory;
    Genetics::EnvironmentState _currentEnvironment;  // Fallback when no environment system
    Genetics::SeedDispersal _seedDispersal;
    
    std::mt19937 _rng;
    
    /**
     * @brief Helper to select plant species based on biome
     * @param biome The biome at the target location
     * @return Function that creates the appropriate plant at given coordinates
     */
    std::function<Genetics::Plant(int, int)> selectPlantForBiome(Biome biome);
};

} // namespace EcoSim
