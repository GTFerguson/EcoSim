---
title: Spatial Index System
created: 2025-12-29
updated: 2025-12-29
status: complete
audience: developer
type: reference
tags: [world, spatial, performance, optimization]
---

# Spatial Index System

**Audience:** Developer
**Type:** Reference

---

## Overview

The SpatialIndex provides O(1) neighbor queries for creatures and other spatial entities. It uses a grid-based spatial hashing approach to efficiently track entity positions and quickly retrieve nearby entities.

**Header:** [`include/world/SpatialIndex.hpp`](include/world/SpatialIndex.hpp:1)
**Source:** [`src/world/SpatialIndex.cpp`](src/world/SpatialIndex.cpp:1)

---

## Purpose

The simulation frequently needs to find creatures near a specific position:
- Hunting behavior looking for prey
- Mating behavior looking for potential mates
- Perception system detecting nearby entities
- Combat determining attackers in range
- Zoochory (seed dispersal) finding nearby creatures

Without spatial indexing, these queries require O(n) iteration over all creatures. With spatial indexing, queries become O(1) average case by only checking entities in nearby grid cells.

---

## API

```cpp
namespace EcoSim {

class SpatialIndex {
public:
    /**
     * @brief Construct a spatial index
     * @param worldWidth Width of the world in tiles
     * @param worldHeight Height of the world in tiles
     * @param cellSize Size of each grid cell (default: 10)
     */
    SpatialIndex(int worldWidth, int worldHeight, int cellSize = 10);
    
    /**
     * @brief Add an entity at a position
     * @param entityId Unique identifier for the entity
     * @param x X coordinate
     * @param y Y coordinate
     */
    void add(int entityId, int x, int y);
    
    /**
     * @brief Remove an entity from the index
     * @param entityId Entity to remove
     */
    void remove(int entityId);
    
    /**
     * @brief Update an entity's position
     * @param entityId Entity to update
     * @param newX New X coordinate
     * @param newY New Y coordinate
     */
    void update(int entityId, int newX, int newY);
    
    /**
     * @brief Get all entities within a radius
     * @param x Center X coordinate
     * @param y Center Y coordinate
     * @param radius Search radius
     * @return Vector of entity IDs within radius
     */
    std::vector<int> getNeighbors(int x, int y, int radius) const;
    
    /**
     * @brief Get all entities in a specific cell
     * @param cellX Cell X index
     * @param cellY Cell Y index
     * @return Vector of entity IDs in cell
     */
    std::vector<int> getEntitiesInCell(int cellX, int cellY) const;
    
    /**
     * @brief Clear all entities from the index
     */
    void clear();
    
    /**
     * @brief Get the number of tracked entities
     */
    size_t size() const;
};

}
```

---

## Usage Example

```cpp
#include "world/SpatialIndex.hpp"

// Create index for 200x200 world with 10-tile cells
SpatialIndex index(200, 200, 10);

// Add creatures
index.add(creature1.getId(), creature1.getX(), creature1.getY());
index.add(creature2.getId(), creature2.getX(), creature2.getY());

// Find neighbors within 15 tiles of position (50, 75)
std::vector<int> nearbyIds = index.getNeighbors(50, 75, 15);

for (int id : nearbyIds) {
    // Process nearby creature
}

// Update position when creature moves
index.update(creature1.getId(), newX, newY);

// Remove when creature dies
index.remove(creature1.getId());
```

---

## Performance

### Time Complexity

| Operation | Average | Worst Case |
|-----------|---------|------------|
| `add()` | O(1) | O(1) |
| `remove()` | O(1) | O(n) per cell |
| `update()` | O(1) | O(n) per cell |
| `getNeighbors()` | O(k) | O(n) |

Where:
- n = total entities
- k = entities in checked cells (typically << n)

### Space Complexity

O(n + c) where:
- n = number of entities
- c = number of grid cells

### Cell Size Considerations

| Cell Size | Pros | Cons |
|-----------|------|------|
| Small (5) | Fewer entities per cell, faster neighbor queries | More cells, higher memory overhead |
| Medium (10) | Good balance | Default choice |
| Large (20) | Lower memory | More entities per cell, slower queries |

Recommended cell size: approximately equal to typical query radius.

---

## Design Decisions

### Grid-Based vs. Quadtree

A grid-based approach was chosen over a quadtree because:
- Simpler implementation
- Consistent O(1) update time (quadtrees can require rebalancing)
- Predictable memory usage
- Sufficient for the simulation's entity density

### Entity Position Tracking

The index maintains a separate map of entity positions to enable O(1) updates without searching the old cell. This trades memory for update performance.

---

## Integration Points

### Creature Movement

When creatures move, their position in the spatial index must be updated:

```cpp
void Creature::moveTo(int newX, int newY, SpatialIndex& index) {
    index.update(getId(), newX, newY);
    x_ = newX;
    y_ = newY;
}
```

### Perception System

The perception system uses spatial queries to find visible/smellable entities:

```cpp
std::vector<int> visibleCreatures = 
    spatialIndex.getNeighbors(creature.getX(), creature.getY(), sightRange);
```

### Hunting Behavior

Hunting behavior queries for nearby prey:

```cpp
auto nearbyIds = spatialIndex.getNeighbors(hunter.getX(), hunter.getY(), detectionRadius);
// Filter to find valid prey
```

---

## Testing

Tests are located in [`src/testing/world/test_spatial_index.cpp`](src/testing/world/test_spatial_index.cpp:1):

- Basic add/remove operations
- Position updates
- Neighbor queries at various radii
- Edge cases (world boundaries, empty regions)
- Performance benchmarks

---

## See Also

**Systems:**
- [[behavior-system]] - Uses spatial queries for creature interactions
- [[perception-system]] - Uses spatial queries for sensory detection

**Reference:**
- [[../reference/api/organisms]] - Creature position management
