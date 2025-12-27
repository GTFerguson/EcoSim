#pragma once

// Forward declarations for external types (global namespace)
class World;

namespace EcoSim {

// Forward declarations for external types
class ScentLayer;

namespace Genetics {

// Forward declaration for OrganismState
struct OrganismState;

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
    float deltaTime = 1.0f;                   ///< Time since last tick (normalized)
    unsigned int currentTick = 0;             ///< Current simulation tick
    int worldRows = 0;                        ///< World height in tiles
    int worldCols = 0;                        ///< World width in tiles
};

} // namespace Genetics
} // namespace EcoSim
