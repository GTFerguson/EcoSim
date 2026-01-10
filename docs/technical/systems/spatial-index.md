---
title: Spatial Index System
created: 2025-12-29
updated: 2026-01-10
status: complete
audience: developer
type: reference
tags: [world, spatial, performance, optimization, plants]
---

# Spatial Index System

**Audience:** Developer
**Type:** Reference

---

## Overview

The SpatialIndex provides efficient neighbor queries for creatures using a grid-based spatial hashing approach. Instead of O(n) iteration over all creatures, queries check only relevant grid cells for O(k) average performance where k << n.

**Header:** [`include/world/SpatialIndex.hpp`](../../../include/world/SpatialIndex.hpp:1)
**Source:** [`src/world/SpatialIndex.cpp`](../../../src/world/SpatialIndex.cpp:1)

---

## Purpose

The simulation frequently needs to find creatures near a specific position:
- **Flocking behavior** - Finding nearby same-species creatures
- **Hunting behavior** - Looking for prey in range
- **Mating behavior** - Looking for potential mates
- **Perception system** - Detecting nearby entities
- **Combat** - Determining attackers in range

Without spatial indexing, these queries require O(n) iteration over all creatures. With spatial indexing, queries become O(k) average case by only checking entities in nearby grid cells, where k is typically much smaller than n.

---

## Design

### Grid-Based Spatial Hash

The index divides the world into uniform grid cells. Each cell stores pointers to creatures within that spatial region.

```
┌────────┬────────┬────────┬────────┐
│ Cell   │ Cell   │ Cell   │ Cell   │
│ (0,0)  │ (1,0)  │ (2,0)  │ (3,0)  │
├────────┼────────┼────────┼────────┤
│ Cell   │ Cell   │●○●     │ Cell   │
│ (0,1)  │ (1,1)  │ (2,1)  │ (3,1)  │  ● = creature
├────────┼────────┼────────┼────────┤
│ Cell   │   ○    │ Cell   │ Cell   │  ○ = query center
│ (0,2)  │ (1,2)  │ (2,2)  │ (3,2)  │
└────────┴────────┴────────┴────────┘
```

**Cell size:** 32 tiles (configurable via `DEFAULT_CELL_SIZE`)

A 32-tile cell size balances:
- Fewer cells to check per query (larger cells = fewer boundary crossings)
- Reasonable creature density per cell
- Good fit for typical perception ranges (sight range ~100 tiles = ~3 cells)

### Why Grid Instead of Quadtree?

A grid-based approach was chosen over a quadtree because:
- **Simpler implementation** - No tree balancing or complex traversal
- **Consistent O(1) update time** - No rebalancing on position changes
- **Predictable memory usage** - Fixed cell structure
- **Sufficient for simulation density** - Creature density is relatively uniform

---

## API Reference

```cpp
namespace EcoSim {

class SpatialIndex {
public:
    static constexpr int DEFAULT_CELL_SIZE = 32;
    
    /**
     * @brief Construct spatial index for given world dimensions.
     * @param worldWidth Width of world in tiles
     * @param worldHeight Height of world in tiles  
     * @param cellSize Size of each cell in tiles (default 32)
     */
    SpatialIndex(int worldWidth, int worldHeight, int cellSize = DEFAULT_CELL_SIZE);
    
    //==========================================================================
    // Core Operations
    //==========================================================================
    
    /**
     * @brief Add a creature to the index.
     * @param creature Pointer to creature (non-owning)
     */
    void insert(Creature* creature);
    
    /**
     * @brief Remove a creature from the index.
     * @param creature Pointer to creature to remove
     */
    void remove(Creature* creature);
    
    /**
     * @brief Update creature's position in the index.
     * 
     * More efficient than remove+insert when creature stays in same cell.
     * 
     * @param creature Pointer to creature
     * @param oldX Previous X position
     * @param oldY Previous Y position
     */
    void update(Creature* creature, float oldX, float oldY);
    
    /**
     * @brief Clear all creatures from the index.
     */
    void clear();
    
    /**
     * @brief Rebuild index from creature vector.
     * 
     * Use after loading saves or major changes.
     * 
     * @param creatures Vector of all creatures
     */
    void rebuild(std::vector<Creature>& creatures);
    
    //==========================================================================
    // Query Operations
    //==========================================================================
    
    /**
     * @brief Find all creatures within radius of a position.
     * @param x Center X position
     * @param y Center Y position
     * @param radius Search radius in tiles
     * @return Vector of creature pointers within radius
     */
    std::vector<Creature*> queryRadius(float x, float y, float radius) const;
    
    /**
     * @brief Find all creatures in a specific grid cell.
     * @param cellX Cell X coordinate (not tile coordinate)
     * @param cellY Cell Y coordinate (not tile coordinate)
     * @return Vector of creature pointers in cell
     */
    std::vector<Creature*> queryCell(int cellX, int cellY) const;
    
    /**
     * @brief Find all creatures in cell containing position and adjacent cells.
     * @param x Position X in tiles
     * @param y Position Y in tiles
     * @return Vector of creature pointers in nearby cells (3x3 grid)
     */
    std::vector<Creature*> queryNearbyCells(float x, float y) const;
    
    /**
     * @brief Query with custom filter predicate.
     * @param x Center X position
     * @param y Center Y position
     * @param radius Search radius
     * @param predicate Filter function returning true for matches
     * @return Filtered vector of creature pointers
     */
    std::vector<Creature*> queryWithFilter(
        float x, float y, float radius,
        std::function<bool(const Creature*)> predicate) const;
    
    /**
     * @brief Find single nearest creature matching predicate.
     * @param x Center X position
     * @param y Center Y position
     * @param maxRadius Maximum search radius
     * @param predicate Filter function (return true to consider)
     * @return Pointer to nearest matching creature, or nullptr
     */
    Creature* findNearest(
        float x, float y, float maxRadius,
        std::function<bool(const Creature*)> predicate) const;
    
    //==========================================================================
    // Utility
    //==========================================================================
    
    /**
     * @brief Get cell coordinates for a world position.
     * @param x World X position
     * @param y World Y position
     * @return Pair of (cellX, cellY)
     */
    std::pair<int, int> getCellCoords(float x, float y) const;
    
    /**
     * @brief Get total number of indexed creatures.
     */
    size_t size() const;
    
    /**
     * @brief Check if index is empty.
     */
    bool empty() const;
    
    /**
     * @brief Get cell size.
     */
    int getCellSize() const;
};

} // namespace EcoSim
```

---

## Usage Examples

### Basic Radius Query

```cpp
#include "world/SpatialIndex.hpp"

// Create index for 200x200 world with default 32-tile cells
SpatialIndex index(200, 200);

// Add creatures during initialization
for (Creature& c : creatures) {
    index.insert(&c);
}

// Find neighbors within 15 tiles of position (50, 75)
std::vector<Creature*> nearby = index.queryRadius(50.0f, 75.0f, 15.0f);

for (Creature* c : nearby) {
    // Process nearby creature
}
```

### Filtered Query (Find Prey)

```cpp
// Find herbivores within hunting range
auto prey = index.queryWithFilter(
    hunter.getWorldX(), hunter.getWorldY(), huntRange,
    [](const Creature* c) {
        return c->getDietType() != DietType::CARNIVORE;
    }
);
```

### Find Nearest Match

```cpp
// Find nearest potential mate
Creature* mate = index.findNearest(
    seeker.getWorldX(), seeker.getWorldY(), sightRange,
    [&seeker](const Creature* c) {
        return c != &seeker && c->getProfile() == Profile::breed;
    }
);
```

### Rebuild After Save Load

```cpp
// After loading creatures from save file
world.rebuildCreatureIndex(creatures);
```

---

## Integration with World

The [`World`](../../../include/world/world.hpp:1) class owns the creature spatial index:

```cpp
class World {
private:
    std::unique_ptr<EcoSim::SpatialIndex> _creatureIndex;
    
public:
    /**
     * @brief Get the creature spatial index.
     * @return Pointer to index, or nullptr if not initialized
     */
    EcoSim::SpatialIndex* getCreatureIndex();
    const EcoSim::SpatialIndex* getCreatureIndex() const;
    
    /**
     * @brief Rebuild the creature index from scratch.
     * Called after loading saves or when index becomes stale.
     * @param creatures Reference to creature vector
     */
    void rebuildCreatureIndex(std::vector<Creature>& creatures);
    
private:
    void initializeCreatureIndex();
};
```

### Index Lifecycle

1. **Initialization**: Created when World initializes (`initializeCreatureIndex()`)
2. **Per-tick rebuild**: `rebuildCreatureIndex()` called each tick (O(n))
3. **Queries**: O(k) where k is creatures in checked cells

---

## Integration with BehaviorContext

The [`BehaviorContext`](../../../include/genetics/behaviors/BehaviorContext.hpp:1) provides spatial index access to behaviors:

```cpp
struct BehaviorContext {
    // ... other fields ...
    SpatialIndex* creatureIndex = nullptr;  ///< Spatial index for O(1) creature neighbor queries
    // ...
};
```

Behaviors can use this for efficient neighbor queries:

```cpp
bool HuntingBehavior::execute(IGeneticOrganism& organism, BehaviorContext& ctx) {
    if (ctx.creatureIndex) {
        // Use spatial index for O(k) prey detection
        auto nearby = ctx.creatureIndex->queryRadius(
            organism.getWorldX(), organism.getWorldY(), huntRange);
        // ... find prey among nearby creatures
    }
}
```

---

## Integration with Creature::flock()

The [`Creature::flock()`](../../../src/objects/creature/creature.cpp:680) method uses spatial queries for efficient neighbor finding:

```cpp
bool Creature::flock(World &world, vector<Creature> &creatures) {
    Creature *closestC = NULL;
    float closestDistance = getSightRange();
    
    // Use O(1) spatial query instead of O(n) iteration
    EcoSim::SpatialIndex* spatialIndex = world.getCreatureIndex();
    
    if (spatialIndex) {
        std::vector<Creature*> nearby = spatialIndex->queryRadius(
            tileX(), tileY(), static_cast<int>(getSightRange())
        );
        
        for (Creature* creature : nearby) {
            if (creature != this) {
                float distance = calculateDistance(creature->getX(), creature->getY());
                if (distance < closestDistance) {
                    closestC = creature;
                    closestDistance = distance;
                }
            }
        }
    } else {
        // Fallback to O(n) iteration when spatial index not available
        for (Creature& creature : creatures) {
            // ... original O(n) logic
        }
    }
    // ... rest of flocking logic
}
```

---

## PlantSpatialIndex

In addition to the creature [`SpatialIndex`](../../../include/world/SpatialIndex.hpp:1), EcoSim provides a separate [`PlantSpatialIndex`](../../../include/world/PlantSpatialIndex.hpp:23) for fast plant neighbor queries.

**Header:** [`include/world/PlantSpatialIndex.hpp`](../../../include/world/PlantSpatialIndex.hpp:1)
**Source:** [`src/world/PlantSpatialIndex.cpp`](../../../src/world/PlantSpatialIndex.cpp:1)

### Key Differences from Creature Index

| Aspect | SpatialIndex (Creatures) | PlantSpatialIndex |
|--------|--------------------------|-------------------|
| **Movement** | Creatures move; requires per-tick rebuild | Plants are static; incremental updates only |
| **Update Strategy** | Full rebuild each tick | Insert/remove on plant add/death |
| **Position Source** | Creature reads own position | Caller provides position |

Plants don't move after spawning, so the index only needs updates when plants are added (growth, seed germination) or removed (death, harvesting). This makes incremental updates practical and efficient.

### API Reference

```cpp
namespace EcoSim {

class PlantSpatialIndex {
public:
    static constexpr int DEFAULT_CELL_SIZE = 32;
    
    /**
     * @brief Construct spatial index for given world dimensions.
     * @param worldWidth Width of world in tiles
     * @param worldHeight Height of world in tiles
     * @param cellSize Size of each cell in tiles (default 32)
     */
    PlantSpatialIndex(int worldWidth, int worldHeight, int cellSize = DEFAULT_CELL_SIZE);
    
    //==========================================================================
    // Core Operations
    //==========================================================================
    
    /**
     * @brief Add a plant to the index.
     * @param plant Pointer to plant (non-owning)
     * @param x X position of plant
     * @param y Y position of plant
     */
    void insert(Genetics::Plant* plant, int x, int y);
    
    /**
     * @brief Remove a plant from the index.
     * @param plant Pointer to plant to remove
     * @param x X position of plant
     * @param y Y position of plant
     */
    void remove(Genetics::Plant* plant, int x, int y);
    
    /**
     * @brief Clear all plants from the index.
     */
    void clear();
    
    //==========================================================================
    // Query Operations
    //==========================================================================
    
    /**
     * @brief Find all plants within radius of a position.
     * @param x Center X position
     * @param y Center Y position
     * @param radius Search radius in tiles
     * @return Vector of plant pointers within radius
     */
    std::vector<Genetics::Plant*> queryRadius(float x, float y, float radius) const;
    
    /**
     * @brief Find all plants in a specific grid cell.
     * @param cellX Cell X coordinate (not tile coordinate)
     * @param cellY Cell Y coordinate (not tile coordinate)
     * @return Vector of plant pointers in cell
     */
    std::vector<Genetics::Plant*> queryCell(int cellX, int cellY) const;
    
    //==========================================================================
    // Utility
    //==========================================================================
    
    std::pair<int, int> getCellCoords(float x, float y) const;
    size_t size() const;
    bool empty() const;
    int getCellSize() const;
};

} // namespace EcoSim
```

### Integration with PlantManager

The [`PlantManager`](../../../include/world/PlantManager.hpp:1) owns the plant spatial index and maintains it incrementally:

```cpp
class PlantManager {
private:
    std::unique_ptr<PlantSpatialIndex> spatialIndex_;
    
public:
    /**
     * @brief Get the plant spatial index.
     */
    PlantSpatialIndex* getSpatialIndex();
    
    // Index is updated automatically when plants are added/removed
    void addPlant(Plant* plant, int x, int y) {
        // ... add plant to map ...
        spatialIndex_->insert(plant, x, y);
    }
    
    void removePlant(Plant* plant, int x, int y) {
        // ... remove plant from map ...
        spatialIndex_->remove(plant, x, y);
    }
};
```

### Usage in Creature Code

The [`findGeneticsPlants()`](../../../src/objects/creature/creature.cpp:1) method in creature behavior uses the spatial index for efficient plant discovery:

```cpp
std::vector<Genetics::Plant*> Creature::findGeneticsPlants(
    World& world, float range, bool edibleOnly
) {
    // Get plant spatial index from PlantManager
    auto* plantIndex = world.getPlantManager().getSpatialIndex();
    
    if (plantIndex) {
        // O(k) query - only checks plants in nearby cells
        return plantIndex->queryRadius(getWorldX(), getWorldY(), range);
    }
    
    // Fallback: O(r²) tile iteration if index unavailable
    return findPlantsByTileIteration(world, range, edibleOnly);
}
```

This transforms plant searches from O(r²) tile iteration (checking every tile in sight range) to O(k) average case (only checking plants in relevant grid cells).

### Performance Impact

| Search Method | Time Complexity | With 100-tile range |
|--------------|-----------------|---------------------|
| Tile iteration | O(r²) | ~10,000 tile checks |
| Spatial index | O(k) | ~3-9 cells, k plants |

For typical perception ranges (50-100 tiles), the spatial index reduces plant lookup time by 100-1000x.

---

## Performance Characteristics

### Time Complexity

| Operation | Average | Worst Case | Notes |
|-----------|---------|------------|-------|
| `insert()` | O(1) | O(1) | Hash map insertion |
| `remove()` | O(k) | O(k) | Linear scan within cell, k = creatures in cell |
| `update()` | O(1) | O(k) | O(1) if same cell, O(k) if cell changes |
| `queryRadius()` | O(k) | O(n) | k = creatures in checked cells |
| `queryCell()` | O(1) | O(1) | Direct cell lookup |
| `queryNearbyCells()` | O(k) | O(n) | 3x3 cell check |
| `findNearest()` | O(k) | O(n) | With distance comparison |
| `rebuild()` | O(n) | O(n) | Rebuilds entire index |

Where:
- n = total creatures in simulation
- k = creatures in checked cells (typically k << n)

### Space Complexity

O(n + c) where:
- n = number of creatures (stored pointers)
- c = number of active cells (hash map overhead)

### Per-Tick Cost

The current implementation rebuilds the entire index each tick:
- **Rebuild cost**: O(n) per tick
- **Query cost**: O(k) per query

This approach was chosen for simplicity over incremental updates. With typical creature populations (100-1000), rebuild is fast enough (~1ms) and eliminates complexity of tracking individual position changes.

### Cell Size Considerations

| Cell Size | Query Performance | Memory | Use Case |
|-----------|-------------------|--------|----------|
| 16 | Faster (fewer creatures/cell) | Higher (more cells) | High-density simulations |
| 32 (default) | Good balance | Moderate | General use |
| 64 | Slower (more creatures/cell) | Lower | Large worlds, sparse population |

**Recommendation**: Cell size ≈ typical perception radius / 3

---

## Testing

Tests are located in [`src/testing/world/test_spatial_index.cpp`](../../../src/testing/world/test_spatial_index.cpp:1):

### Test Groups

| Group | Tests |
|-------|-------|
| Basic Operations | Insert, remove, update, clear, rebuild |
| Radius Queries | Empty, single inside/outside, multiple, boundary cases |
| Boundary Precision | Exact boundary, just outside, diagonal, sub-tile |
| Edge Cases | Zero/negative radius, world boundaries, large queries |
| Performance | Large creature counts, repeated queries |

### Key Test Cases

```cpp
// Exact boundary inclusion
void test_queryRadius_exact_boundary_included();

// Diagonal distance calculation
void test_queryRadius_diagonal_boundary();

// Sub-tile precision
void test_queryRadius_sub_tile_precision();

// World edge handling
void test_queryRadius_world_corners();
```

---

## Future Improvements

Potential optimizations tracked in [[../../future/performance-improvements|Performance Improvements]]:

1. **Incremental updates**: Track position changes instead of full rebuild
2. **Spatial locality**: Sort creatures by cell for cache-friendly iteration
3. **Query result caching**: Cache common queries within a tick
4. **Multi-level grid**: Coarse grid for large queries, fine grid for small

---

## See Also

**Systems:**
- [[behavior-system]] - Uses spatial queries for creature interactions
- [[perception-system]] - Uses spatial queries for sensory detection

**Reference:**
- [[../reference/api/organisms]] - Creature position management
