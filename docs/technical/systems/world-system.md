---
title: World System Architecture
created: 2025-12-30
updated: 2026-01-09
status: complete
audience: developer
type: reference
tags: [world, architecture, systems, terrain, climate, biomes]
---

# World System Architecture

**Audience:** Developer
**Type:** Reference

---

## Overview

The World system manages all aspects of the simulation environment: terrain generation, climate simulation, biome assignment, tiles, plants, corpses, seasons, and environmental queries. Rather than implementing everything in a monolithic class, World serves as a **coordinator** that delegates to focused subsystems.

The world generation system uses a **climate-based approach** with 22 Whittaker biomes determined by temperature and moisture. This creates geographically coherent worlds where biomes emerge naturally from climate simulation rather than arbitrary elevation bands.

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

## WorldGenerator (Legacy)

The original terrain generator using elevation-only Simplex noise. Still available but superseded by ClimateWorldGenerator for new worlds.

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
    void generate();
    void regenerate();
    // Configuration accessors...
};

} // namespace EcoSim
```

---

## ClimateWorldGenerator

**The primary world generation system** using climate-based biome determination.

**Header:** [`include/world/ClimateWorldGenerator.hpp`](../../../include/world/ClimateWorldGenerator.hpp:1)

### Generation Pipeline

The climate generator uses a multi-pass approach:

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Generation Pipeline                               │
├─────────────────────────────────────────────────────────────────────┤
│  Phase 1: Continental Shape                                         │
│  └── Low-frequency Simplex noise → Land/Ocean mask                  │
│                                                                      │
│  Phase 2: Elevation Detail                                          │
│  └── Ridged multifractal noise → Mountain ranges                    │
│  └── Tectonic plate ridges → Realistic mountain chains              │
│                                                                      │
│  Phase 3: Climate Calculation                                       │
│  └── Temperature = f(latitude, elevation)                           │
│  └── Moisture = f(coast distance, wind, rain shadow)                │
│                                                                      │
│  Phase 4: Biome Determination                                       │
│  └── Whittaker diagram lookup (temperature × moisture)              │
│  └── Biome blending for smooth ecotones                             │
│                                                                      │
│  Phase 5: Water Features                                            │
│  └── River tracing (downhill flow)                                  │
│  └── Lake formation (terrain depressions)                           │
└─────────────────────────────────────────────────────────────────────┘
```

### 22 Whittaker Biomes

Biomes are determined by temperature and moisture following the Whittaker classification:

| Category | Biomes |
|----------|--------|
| **Aquatic** | Ocean Deep, Ocean Shallow, Ocean Coast, Freshwater |
| **Cold** | Ice Sheet, Tundra, Taiga |
| **Temperate** | Boreal Forest, Temperate Rainforest, Temperate Forest, Temperate Grassland |
| **Warm** | Tropical Rainforest, Tropical Seasonal Forest, Savanna |
| **Dry** | Hot Desert, Cold Desert, Steppe, Shrubland |
| **Alpine** | Alpine Meadow, Alpine Tundra, Mountain Bare, Glacier |

### Core Data Structures

```cpp
namespace EcoSim {

// Per-tile climate data
struct TileClimate {
    float elevation;      // 0.0 - 1.0 normalized
    float temperature;    // Celsius (-40 to +50)
    float moisture;       // 0.0 - 1.0 precipitation index
    float waterLevel;     // For rivers/lakes
    
    BiomeBlend biomeBlend;  // Weighted blend for ecotones
    TerrainFeature feature; // River, lake, beach, etc.
    
    Biome biome() const;    // Primary biome
    bool isLand() const;
    bool hasWater() const;
    float getVegetationDensity() const;
    float getMovementCost() const;
};

// Biome blending for smooth transitions
struct BiomeBlend {
    BiomeWeight contributions[4];  // Up to 4 biomes
    int count;
    
    Biome primary() const;
    float getBlendedVegetationDensity() const;
    float getBlendedMovementCost() const;
};

// Properties for each biome
struct BiomeProperties {
    const char* name;
    float temperatureModifier;
    float evaporationRate;
    float vegetationDensity;
    float movementCost;
    bool supportsPlants;
    bool supportsTrees;
    TerrainType terrainType;  // For rendering
};

} // namespace EcoSim
```

### Configuration

```cpp
struct ClimateGeneratorConfig {
    // Dimensions
    unsigned int width = 500;
    unsigned int height = 500;
    
    // Terrain shape
    float seaLevel = 0.30f;
    bool isIsland = true;
    
    // Climate parameters
    float equatorPosition = 0.5f;
    float temperatureRange = 70.0f;
    float baseTemperature = 15.0f;
    float lapseRate = 6.5f;  // °C per 1000m
    
    // Moisture parameters
    float coastalMoistureDecay = 50.0f;
    int rainShadowDistance = 100;
    
    // Water features
    bool generateRivers = true;
    int maxRivers = 20;
    bool generateLakes = true;
    
    // Random seed
    unsigned int seed = 0;
};
```

### Usage Example

```cpp
// Create generator with configuration
ClimateGeneratorConfig config;
config.width = 500;
config.height = 500;
config.seed = 12345;
config.generateRivers = true;

ClimateWorldGenerator generator(config);

// Generate world
WorldGrid grid;
generator.generate(grid);

// Access climate data
const TileClimate& climate = generator.getClimate(x, y);
float temp = climate.temperature;
Biome biome = climate.biome();

// Get biome properties
const BiomeProperties& props = ClimateWorldGenerator::getBiomeProperties(biome);
bool canGrowTrees = props.supportsTrees;
```

### Climate Calculation Details

**Temperature:**
- Base temperature at equator, sea level: 15°C
- Latitude effect: ±35°C from equator to poles
- Elevation effect: -6.5°C per 1000m (lapse rate)
- Local noise variation: ±5°C

**Moisture:**
- Exponential decay from coastlines
- Rain shadow behind mountains (westerly winds)
- Local noise variation for variety

### Biome Lookup Table

The Whittaker diagram maps (temperature, moisture) to biomes:

```
              Moisture →
          Arid    Dry    Moderate   Wet    Saturated
        ┌───────┬───────┬─────────┬───────┬──────────┐
 Frozen │ Ice   │ Ice   │ Tundra  │Tundra │ Glacier  │
        ├───────┼───────┼─────────┼───────┼──────────┤
 Cold   │CldDsrt│Tundra │ Taiga   │ Taiga │ Boreal   │
        ├───────┼───────┼─────────┼───────┼──────────┤
 Cool   │Steppe │Shrub  │TempForst│TempFst│TempRain  │
 Temp   ├───────┼───────┼─────────┼───────┼──────────┤
        │Steppe │TempGrs│TempForst│TempFst│TempRain  │
        ├───────┼───────┼─────────┼───────┼──────────┤
 Warm   │HotDsrt│Savanna│ Savanna │TropSea│TropRain  │
        ├───────┼───────┼─────────┼───────┼──────────┤
 Hot    │HotDsrt│HotDsrt│ Savanna │TropSea│TropRain  │
        └───────┴───────┴─────────┴───────┴──────────┘
```

### River Generation

Rivers flow from high-moisture mountain sources downhill:

1. Find river sources (high elevation + high moisture)
2. Trace downhill path using steepest descent
3. Accumulate flow from tributaries
4. Form lakes at terrain depressions
5. Continue until reaching ocean or evaporating

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
- [[environmental-stress]] - Environmental fitness gradients

**Design:**
- [[../design/world-generation]] - Climate-based generation design rationale
- [[../design/world-organism-integration]] - Organism-environment interaction design

**Reference:**
- [[../reference/api/organisms]] - Plant implementation details
- [[../reference/api/core-classes]] - Core genetics classes

**Tests:**
- [`test_world_organism_integration.cpp`](../../../src/testing/world/test_world_organism_integration.cpp:1) - Integration tests
