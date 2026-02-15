#pragma once

#include <vector>

// Forward declarations for external types (global namespace)
class World;

namespace EcoSim {

// Forward declarations for external types
class ScentLayer;
class SpatialIndex;

namespace Genetics {

// Forward declarations
struct OrganismState;
class Organism;

/**
 * @brief Context passed to behaviors during execution
 *
 * Provides behaviors with access to the simulation environment
 * without coupling them directly to concrete implementations.
 *
 * @note This struct uses raw pointers for optional dependencies.
 * Behaviors should null-check before use.
 */
struct BehaviorContext {
    const ScentLayer* scentLayer = nullptr;  ///< Scent layer for olfactory perception
    World* world = nullptr;                   ///< World access for entity queries
    const OrganismState* organismState = nullptr;  ///< Current organism state (energy, health, etc.)
    SpatialIndex* creatureIndex = nullptr;    ///< Spatial index for O(1) creature neighbor queries
    float deltaTime = 1.0f;                   ///< Time since last tick (normalized)
    unsigned int currentTick = 0;             ///< Current simulation tick
    int worldRows = 0;                        ///< World height in tiles
    int worldCols = 0;                        ///< World width in tiles

    /**
     * @brief Query nearby organisms via spatial index
     *
     * Wraps SpatialIndex::queryRadius() and returns Organism* pointers.
     * Implemented in ecosim_core (which knows Creature inherits Organism).
     *
     * @return Empty vector if creatureIndex is null.
     */
    std::vector<Organism*> queryNearbyOrganisms(float x, float y, float radius) const;
};

} // namespace Genetics
} // namespace EcoSim
