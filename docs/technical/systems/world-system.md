---
title: World System Architecture
created: 2025-12-30
updated: 2025-12-30
status: complete
audience: developer
type: reference
tags: [world, architecture, systems, terrain]
---

# World System Architecture

**Audience:** Developer
**Type:** Reference

---

## Overview

The World system manages all aspects of the simulation environment: terrain, tiles, plants, corpses, seasons, and environmental queries. Rather than implementing everything in a monolithic class, World serves as a **coordinator** that delegates to focused subsystems.

**Header:** [`include/world/world.hpp`](../../../include/world/world.hpp:1)

---

## Architecture

The World class follows a **composition over inheritance** approach, owning specialized components that each handle one responsibility:

```
World (Coordinator)
├── grid()           → WorldGrid
├── generator()      → WorldGenerator (via unique_ptr)
├── scentLayer()     → ScentLayer
├── corpses()        → CorpseManager
├── seasons()        → SeasonManager
├── environment()    → EnvironmentSystem
├── plants()         → PlantManager
└── getCreatureIndex() → SpatialIndex (via unique_ptr)
```

### Component Summary

| Component | Responsibility | Header |
|-----------|---------------|--------|
| [`WorldGrid`](#worldgrid) | Tile storage with bounds checking | [`WorldGrid.hpp`](../../../include/world/WorldGrid.hpp:1) |
| [`WorldGenerator`](#worldgenerator) | Terrain generation with Simplex noise | [`WorldGenerator.hpp`](../../../include/world/WorldGenerator.hpp:1) |
| [`ScentLayer`](#scentlayer) | Scent-based communication grid | [`ScentLayer.hpp`](../../../include/world/ScentLayer.hpp:1) |
| [`CorpseManager`](#corpsemanager) | Corpse lifecycle and spatial queries | [`CorpseManager.hpp`](../../../include/world/CorpseManager.hpp:1) |
| [`SeasonManager`](#seasonmanager) | Time/calendar and seasonal state | [`SeasonManager.hpp`](../../../include/world/SeasonManager.hpp:1) |
| [`EnvironmentSystem`](#environmentsystem) | Per-tile environmental queries | [`EnvironmentSystem.hpp`](../../../include/world/EnvironmentSystem.hpp:1) |
| [`PlantManager`](#plantmanager) | Plant lifecycle and factory management | [`PlantManager.hpp`](../../../include/world/PlantManager.hpp:1) |
| [`SpatialIndex`](#spatialindex) | O(1) creature neighbor queries | [`SpatialIndex.hpp`](../../../include/world/SpatialIndex.hpp:1) |

---

## World Class

The main coordinator class that provides unified access to all subsystems.

```cpp
class World {
public:
    World(const MapGen& mapGen, const OctaveGen& octaveGen);
    
    //==========================================================================
    // Core Subsystem Accessors
    //==========================================================================
    EcoSim::WorldGrid& grid();
    const EcoSim::WorldGrid& grid() const;
    
    EcoSim::ScentLayer& scentLayer();
    const EcoSim::ScentLayer& scentLayer() const;
    
    EcoSim::CorpseManager& corpses();
    const EcoSim::CorpseManager& corpses() const;
    
    EcoSim::SeasonManager& seasons();
    const EcoSim::SeasonManager& seasons() const;
    
    EcoSim::EnvironmentSystem& environment();
    const EcoSim::EnvironmentSystem& environment() const;
    
    EcoSim::PlantManager& plants();
    const EcoSim::PlantManager& plants() const;
    
    //==========================================================================
    // Spatial Indexing
    //==========================================================================
    void initializeCreatureIndex();
    EcoSim::SpatialIndex* getCreatureIndex();
    const EcoSim::SpatialIndex* getCreatureIndex() const;
    void rebuildCreatureIndex(std::vector<Creature>& creatures);
    
    //==========================================================================
    // Terrain Generation
    //==========================================================================
    void simplexGen();
    // Configuration getters/setters: getSeed(), setSeed(), etc.
    
    //==========================================================================
    // Simulation Update
    //==========================================================================
    void updateAllObjects();
    void updateScentLayer();
    unsigned int getCurrentTick() const;
    
private:
    EcoSim::WorldGrid _grid;
    std::unique_ptr<EcoSim::WorldGenerator> _generator;
    EcoSim::ScentLayer _scentLayer;
    std::unique_ptr<EcoSim::SpatialIndex> _creatureIndex;
    std::unique_ptr<EcoSim::CorpseManager> _corpseManager;
    std::unique_ptr<EcoSim::SeasonManager> _seasonManager;
    std::unique_ptr<EcoSim::EnvironmentSystem> _environmentSystem;
    std::unique_ptr<EcoSim::PlantManager> _plantManager;
    unsigned int _currentTick;
};
```

---

## WorldGrid

Handles tile storage and access with bounds checking.

**Header:** [`include/world/WorldGrid.hpp`](../../../include/world/WorldGrid.hpp:1)

```cpp
namespace EcoSim {

class WorldGrid {
public:
    WorldGrid();
    
    // Dimensions
    void resize(unsigned int rows, unsigned int cols);
    unsigned int getRows() const;
    unsigned int getCols() const;
    
    // Tile access with bounds checking
    Tile& at(unsigned int row, unsigned int col);
    const Tile& at(unsigned int row, unsigned int col) const;
    bool isValidPosition(int row, int col) const;
    
    // Raw access (for legacy code)
    std::vector<std::vector<Tile>>& getRaw();
    const std::vector<std::vector<Tile>>& getRaw() const;
    
    // Iteration
    using iterator = std::vector<std::vector<Tile>>::iterator;
    iterator begin();
    iterator end();
};

} // namespace EcoSim
```

### Usage Example

```cpp
// Access via World
World world(mapGen, octaveGen);
Tile& tile = world.grid().at(10, 20);

// Check bounds before access
if (world.grid().isValidPosition(row, col)) {
    Tile& tile = world.grid().at(row, col);
}
```

---

## WorldGenerator

Generates terrain using Simplex noise algorithms.

**Header:** [`include/world/WorldGenerator.hpp`](../../../include/world/WorldGenerator.hpp:1)

```cpp
namespace EcoSim {

struct MapGen {
    unsigned int rows;
    unsigned int cols;
    double terrainLevel[7];
};

struct OctaveGen {
    double scale;
    double seed;
    double freq;
    double exponent;
    unsigned int terraces;
};

class WorldGenerator {
public:
    WorldGenerator(WorldGrid& grid, const MapGen& mapGen, const OctaveGen& octaveGen);
    
    // Generation
    void generate();
    void regenerate();
    
    // Configuration
    const MapGen& getMapGen() const;
    const OctaveGen& getOctaveGen() const;
    void setMapGen(const MapGen& mg);
    void setOctaveGen(const OctaveGen& og);
    
    // Individual parameter accessors
    double getSeed() const;
    void setSeed(double seed);
    // ... other parameter getters/setters
};

} // namespace EcoSim
```

### Generation Process

1. Initialize Simplex noise with seed
2. Sample noise at each tile position
3. Apply frequency and octave settings
4. Map noise values to terrain types via thresholds
5. Assign tile properties based on terrain

---

## CorpseManager

Manages corpse entities: creation, decay, and spatial queries.

**Header:** [`include/world/CorpseManager.hpp`](../../../include/world/CorpseManager.hpp:1)

```cpp
namespace EcoSim {

class CorpseManager {
public:
    CorpseManager();
    
    // Lifecycle
    void addCorpse(float x, float y, float size, 
                   const std::string& speciesName, 
                   float bodyCondition = 0.5f);
    void tick();
    void removeCorpse(world::Corpse* corpse);
    
    // Queries
    world::Corpse* findNearest(float x, float y, float maxRange);
    const std::vector<std::unique_ptr<world::Corpse>>& getCorpses() const;
    
    // Statistics
    size_t count() const;
};

} // namespace EcoSim
```

### Corpse Lifecycle

```
Creature dies
     │
     ▼
addCorpse() creates Corpse entity
     │
     ▼
tick() called each frame
     │ - Updates decay
     │ - Removes fully decayed
     ▼
Corpse removed when:
  - Fully consumed
  - Fully decayed
  - Manually removed
```

---

## SeasonManager

Manages time progression and seasonal state.

**Header:** [`include/world/SeasonManager.hpp`](../../../include/world/SeasonManager.hpp:1)

```cpp
namespace EcoSim {

enum class Season { Spring, Summer, Autumn, Winter };

class SeasonManager {
public:
    SeasonManager();
    
    // Time progression
    void tick();
    void advanceDay();
    
    // Queries
    Season getCurrentSeason() const;
    unsigned int getCurrentDay() const;
    unsigned int getCurrentYear() const;
    float getSeasonProgress() const;  // 0.0 to 1.0 within season
    
    // Environmental factors
    float getTemperatureModifier() const;
    float getDaylightHours() const;
    float getGrowthModifier() const;
};

} // namespace EcoSim
```

### Season Effects

| Season | Temperature | Daylight | Plant Growth |
|--------|-------------|----------|--------------|
| Spring | Warming | Increasing | High |
| Summer | Hot | Long | Moderate |
| Autumn | Cooling | Decreasing | Low |
| Winter | Cold | Short | Dormant |

---

## EnvironmentSystem

Provides per-tile environmental queries by combining grid data with seasonal state.

**Header:** [`include/world/EnvironmentSystem.hpp`](../../../include/world/EnvironmentSystem.hpp:1)

```cpp
namespace EcoSim {

class EnvironmentSystem {
public:
    EnvironmentSystem(const WorldGrid& grid, const SeasonManager& seasons);
    
    // Per-tile queries
    float getTemperature(int x, int y) const;
    float getHumidity(int x, int y) const;
    float getLightLevel(int x, int y) const;
    
    // Terrain queries
    bool isPassable(int x, int y) const;
    bool isWaterSource(int x, int y) const;
    TerrainType getTerrainType(int x, int y) const;
    
    // Growth conditions
    float getGrowthSuitability(int x, int y) const;
};

} // namespace EcoSim
```

### Environment Calculation

Environmental values combine:
1. **Base terrain values** - From tile type
2. **Elevation modifiers** - Higher = colder
3. **Seasonal modifiers** - From SeasonManager
4. **Time of day** - Affects light/temperature

---

## PlantManager

Manages plant lifecycle: spawning, growth, death, and iteration.

**Header:** [`include/world/PlantManager.hpp`](../../../include/world/PlantManager.hpp:1)

```cpp
namespace EcoSim {

class PlantManager {
public:
    PlantManager(WorldGrid& grid, GeneRegistry& registry);
    
    // Factory operations
    Plant& spawnPlant(int x, int y);
    Plant& spawnPlantWithGenome(int x, int y, const Genome& genome);
    
    // Lifecycle
    void tick();
    void removeDead();
    
    // Queries
    Plant* getPlantAt(int x, int y);
    const std::vector<std::unique_ptr<Plant>>& getPlants() const;
    size_t count() const;
    
    // Iteration
    template<typename Func>
    void forEachPlant(Func&& func);
};

} // namespace EcoSim
```

### Plant Lifecycle

```
PlantManager::spawnPlant()
     │
     ▼
Plant created with genetics
     │
     ▼
tick() updates each frame:
  - Age progression
  - Growth based on environment
  - Seed production
  - Energy from photosynthesis
     │
     ▼
removeDead() cleans up:
  - Dead plants removed
  - Tile references cleared
```

---

## SpatialIndex

See [[spatial-index]] for comprehensive documentation.

Provides O(1) average-case neighbor queries for creatures using grid-based spatial hashing.

```cpp
// Basic usage via World
SpatialIndex* index = world.getCreatureIndex();
if (index) {
    auto nearby = index->queryRadius(x, y, radius);
}
```

---

## Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                         World                                │
│                      (Coordinator)                           │
└─────────────────────────────────────────────────────────────┘
         │         │         │         │         │
         ▼         ▼         ▼         ▼         ▼
    ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐
    │WorldGrid│ │WorldGen │ │Corpse   │ │Season   │ │Plant    │
    │         │ │         │ │Manager  │ │Manager  │ │Manager  │
    └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘
         │         │              │         │              │
         └─────────┴──────────────┴─────────┴──────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │EnvironmentSystem│
                    │  (queries both  │
                    │ grid + seasons) │
                    └─────────────────┘
```

---

## Usage Patterns

### Accessing Subsystems

```cpp
World world(mapGen, octaveGen);

// Grid access
Tile& tile = world.grid().at(row, col);

// Season queries
Season current = world.seasons().getCurrentSeason();

// Environment queries
float temp = world.environment().getTemperature(x, y);

// Plant operations
world.plants().spawnPlant(x, y);
world.plants().tick();

// Corpse operations
world.corpses().addCorpse(x, y, size, species);
auto* nearest = world.corpses().findNearest(x, y, range);
```

### Simulation Loop

```cpp
void simulationTick(World& world, std::vector<Creature>& creatures) {
    // Update time/seasons
    world.seasons().tick();
    
    // Update plants
    world.plants().tick();
    world.plants().removeDead();
    
    // Update corpses
    world.corpses().tick();
    
    // Update scent layer
    world.updateScentLayer();
    
    // Rebuild spatial index
    world.rebuildCreatureIndex(creatures);
    
    // Update creatures (use spatial index for queries)
    for (auto& creature : creatures) {
        creature.update(world);
    }
}
```

---

## Testing

Tests are organized by component:

| Test File | Component |
|-----------|-----------|
| [`test_world_grid.cpp`](../../../src/testing/world/test_world_grid.cpp:1) | WorldGrid |
| [`test_world_generator.cpp`](../../../src/testing/world/test_world_generator.cpp:1) | WorldGenerator |
| [`test_corpse_manager.cpp`](../../../src/testing/world/test_corpse_manager.cpp:1) | CorpseManager |
| [`test_season_manager.cpp`](../../../src/testing/world/test_season_manager.cpp:1) | SeasonManager |
| [`test_environment_system.cpp`](../../../src/testing/world/test_environment_system.cpp:1) | EnvironmentSystem |
| [`test_plant_manager.cpp`](../../../src/testing/world/test_plant_manager.cpp:1) | PlantManager |
| [`test_spatial_index.cpp`](../../../src/testing/world/test_spatial_index.cpp:1) | SpatialIndex |

Run all world tests:

```bash
./genetics_tests "[world]"
```

---

## Design Rationale

### Why Decompose World?

The original World class had grown to handle:
- Tile storage and access
- Terrain generation
- Plant management
- Corpse management
- Season/time tracking
- Environmental queries
- Spatial indexing

This violated the Single Responsibility Principle and made testing difficult.

### Benefits of Component Architecture

1. **Testability** - Each component can be tested in isolation
2. **Maintainability** - Changes isolated to relevant component
3. **Clarity** - Clear ownership and responsibilities
4. **Flexibility** - Components can evolve independently
5. **Documentation** - Each component self-documenting

### Component Ownership

| Component | Lifetime | Access |
|-----------|----------|--------|
| WorldGrid | Owned by World (value) | Direct reference |
| WorldGenerator | Owned by World (unique_ptr) | Via World methods |
| ScentLayer | Owned by World (value) | Direct reference |
| CorpseManager | Owned by World (unique_ptr) | Direct reference |
| SeasonManager | Owned by World (unique_ptr) | Direct reference |
| EnvironmentSystem | Owned by World (unique_ptr) | Direct reference |
| PlantManager | Owned by World (unique_ptr) | Direct reference |
| SpatialIndex | Owned by World (unique_ptr) | Pointer (may be null) |

---

## See Also

**Systems:**
- [[spatial-index]] - Detailed spatial index documentation
- [[scent-system]] - Scent layer communication
- [[behavior-system]] - Uses World for environment queries

**Reference:**
- [[../reference/api/organisms]] - Plant implementation details
- [[../reference/api/core-classes]] - Core genetics classes
