# World & Environment Systems Roadmap

**Status:** 🎯 Active Roadmap
**Created:** 2025-12-23
**Last Updated:** 2026-01-08

---

## Executive Summary

This roadmap addresses the critical gap between **defined genes** (67+ in UniversalGenes) and **environmental systems** that give those genes meaning. Many genes reference environmental conditions that don't yet exist in the simulation, such as wind for seed dispersal, varying temperatures for tolerance genes, and dynamic light for photosynthesis.

Additionally, this roadmap now includes **Genesis-ready features** (A6-A8) that enable god-game mechanics like terraforming, water manipulation, and disaster events.

### Current State Summary

| System | Status | Note |
|--------|--------|------|
| **Genes Defined** | ✅ 67+ genes | Comprehensive unified genome |
| **Basic World** | ✅ Complete | Terrain, elevation, biomes |
| **Environment Variables** | ⚠️ Static | Single global temp/humidity/light |
| **Wind System** | ❌ Missing | `SEED_AERODYNAMICS` gene has no effect |
| **Dynamic Temperature** | ❌ Missing | All tiles are 20°C always |
| **Varying Light** | ❌ Missing | No shade, no day/night impact |
| **Seasonal Cycles** | ❌ Missing | Field exists but unused |
| **Aquatic Genes** | ❌ Missing | Water is just passable/impassable |
| **Tile Mutability** | ❌ Missing | Tiles immutable after generation |
| **Per-Tile Water** | ❌ Missing | No rivers, lakes, or dynamic flooding |
| **Event System** | ❌ Missing | No disasters or propagating effects |

### Roadmap Phases

**Implementation Order: A → B → C → D → E → F**

```
Phase A: Environment Foundation
   ↓
Phase B: Waste & Dispersal System
   ↓
Phase C: Scripted Genetics (GODk)
   ↓
Phase D: Aquatic Life
   ↓
Phase E: Dynamic World
   ↓
Phase F: Conditional Expression
```

---

## Phase A: Environment Foundation

**Priority:** 🔴 HIGHEST  
**Goal:** Make existing environment-dependent genes functional  
**Prerequisites:** None

### Objectives

Create dynamic environmental systems that vary across space and time, enabling genes like `TEMP_TOLERANCE_*`, `LIGHT_REQUIREMENT`, `SEED_AERODYNAMICS`, and `WATER_REQUIREMENT` to have meaningful effects on organism survival and reproduction.

### Components

#### A1: Day/Night Cycle System

**Why:** Drives both light and temperature fluctuations

**Implementation:**
- Add `time_of_day` tick counter (0-24000, where 12000 = full day)
- Calculate sun position: `sunAngle = (time_of_day / 24000) * 2π`
- Light intensity curve: `intensity = max(0, sin(sunAngle))`
- Day: 0.8-1.0 light intensity
- Dawn/Dusk: 0.3-0.8 intensity (gradual)
- Night: 0.0-0.2 intensity (moonlight)

**Affects:**
- Plant photosynthesis rates (higher during day)
- Temperature (cooler at night)
- Creature visibility/hunting success
- Energy costs (nocturnal vs diurnal strategies)

**New Fields:**
```cpp
// In World class
unsigned int _timeOfDay = 0;        // 0-24000 ticks
float _dayLength = 12000.0f;        // Configurable day duration
float _currentLightLevel = 1.0f;    // Global base light level
```

#### A2: Wind System

**Why:** Enables `SEED_AERODYNAMICS`, `SEED_MASS` genes to function

**Implementation:**
- Wind has direction (0-360°) and speed (0-20 m/s)
- Wind changes gradually over time (Perlin noise or sine waves)
- Can be directional prevailing winds (e.g., westerlies)
- Affects seed dispersal calculations in `SeedDispersal.cpp`

**New Fields:**
```cpp
// In World class
float _windSpeed = 5.0f;           // m/s
float _windDirection = 0.0f;       // degrees (0 = North)
float _windChangeRate = 0.1f;      // How fast wind changes
```

**Integration Points:**
- `SeedDispersal::disperseByWind()` - already has parameters, just needs real data
- Potential future: Wind resistance affects creature movement costs

#### A3: Per-Biome Temperature

**Why:** Makes `TEMP_TOLERANCE_LOW/HIGH` meaningful

**Current State:** Global 20°C for everything

**Implementation:**
- Base temperature per `TerrainType`:
  - DESERT_SAND: 35°C base
  - SNOW/PEAKS: -5°C base
  - FOREST: 18°C base
  - WATER: 15°C base
- Temperature affected by:
  - Elevation (lapse rate: -6.5°C per 1000m)
  - Time of day (±10°C variation)
  - Season (±15°C variation)

**New Method:**
```cpp
float World::calculateTemperature(int x, int y) const {
    Tile& tile = _grid[x][y];
    float baseTemp = getBaseTemperatureForTerrain(tile.getTerrainType());
    float elevationMod = -0.0065f * tile.getElevation();
    float timeOfDayMod = getDailyTempVariation();
    float seasonMod = getSeasonalTempVariation();
    return baseTemp + elevationMod + timeOfDayMod + seasonMod;
}
```

#### A4: Varying Light Intensity

**Why:** Makes `LIGHT_REQUIREMENT` functional, enables shade mechanics

**Implementation:**
- Base light from day/night cycle
- Per-tile light modifiers:
  - FOREST: 0.3-0.5 light (shade)
  - PLAINS: 1.0 light (full sun)
  - DEEP_WATER: 0.1 light (light penetration)
- Plant height creates dynamic shade (tall plants shade neighbors)

**New Method:**
```cpp
float World::calculateLightLevel(int x, int y) const {
    float baseLight = _currentLightLevel;  // From day/night
    float terrainMod = getLightModifierForTerrain(x, y);
    float shadeMod = calculatePlantShade(x, y);
    return baseLight * terrainMod * shadeMod;
}
```

#### A5: Seasonal Cycle Engine

**Why:** Foundation for all seasonal effects

**Implementation:**
- 4 seasons: Spring, Summer, Autumn, Winter
- Each season lasts configurable ticks (e.g., 10,000 ticks)
- Smooth transitions between seasons
- Affects:
  - Temperature ranges
  - Day length (longer summer days, shorter winter days)
  - Plant growth rates
  - Future: Migration triggers, hibernation

**New Fields:**
```cpp
enum class Season { SPRING, SUMMER, AUTUMN, WINTER };

// In World class
Season _currentSeason = Season::SPRING;
unsigned int _ticksInSeason = 0;
unsigned int _ticksPerSeason = 10000;
```

#### A6: Tile Mutability API

**Why:** Enables runtime terrain modification for god-game mechanics (Genesis), terraforming tools, and dynamic world events.

**Current State:** Tiles are set during generation and effectively immutable during simulation.

**Implementation:**
```cpp
// WorldGrid additions - single tile operations
void WorldGrid::setElevation(int x, int y, float elevation);
void WorldGrid::setTerrainType(int x, int y, TerrainType type);
void WorldGrid::setMoisture(int x, int y, float moisture);

// Bulk operations for area effects
void WorldGrid::modifyElevationArea(int cx, int cy, int radius,
                                     float delta, FalloffType falloff);
void WorldGrid::setTerrainArea(int cx, int cy, int radius,
                                TerrainType type, float probability);

// Terrain recalculation after elevation changes
void WorldGrid::recalculateTerrainFromElevation(int x, int y);
void WorldGrid::recalculateTerrainArea(int cx, int cy, int radius);
```

**Falloff Types:**
```cpp
enum class FalloffType {
    FLAT,       // Uniform effect across radius
    LINEAR,     // Linear decrease from center
    QUADRATIC,  // Smooth falloff (most natural)
    GAUSSIAN    // Bell curve (best for natural features)
};
```

**Use Cases:**
- God tools: Raise/lower terrain
- Dynamic events: Volcanic eruptions, earthquakes
- Erosion simulation (future)
- Player terraforming in Genesis

#### A7: Per-Tile Water State

**Why:** Enables localized water features (rivers, lakes, ponds) beyond the global sea level system in Phase E. Critical for Genesis flooding mechanics and realistic hydrology.

**Current State:** Water is binary - tiles are either water terrain or not. No dynamic water levels.

**Implementation:**
```cpp
// Tile additions
struct Tile {
    float elevation;
    float waterLevel = 0.0f;      // Height of water surface at this tile
    float waterFlow = 0.0f;       // Flow rate (for rivers)
    TerrainType baseTerrainType;  // Terrain underneath water
    
    bool isFlooded() const { return waterLevel > elevation; }
    float waterDepth() const { return std::max(0.f, waterLevel - elevation); }
    bool hasWater() const { return waterLevel > 0.0f; }
    
    // Effective terrain considers flooding
    TerrainType getEffectiveTerrainType() const {
        if (isFlooded()) {
            float depth = waterDepth();
            if (depth > 2.0f) return TerrainType::DEEP_WATER;
            if (depth > 0.5f) return TerrainType::SHALLOW_WATER;
            return TerrainType::FLOODED;  // New type
        }
        return baseTerrainType;
    }
};
```

**Water Source Types:**
```cpp
struct WaterSource {
    int x, y;
    float flowRate;           // Volume per tick
    WaterSourceType type;     // SPRING, RAIN, RIVER_INLET
    bool isActive = true;
};

// World manages water sources
std::vector<WaterSource> _waterSources;
void addWaterSource(int x, int y, float flowRate, WaterSourceType type);
void removeWaterSource(int x, int y);
```

**Water Simulation (simplified):**
```cpp
void World::updateWater() {
    // 1. Add water from sources
    for (auto& source : _waterSources) {
        if (source.isActive) {
            _grid.at(source.x, source.y).waterLevel += source.flowRate;
        }
    }
    
    // 2. Flow water to lower neighbors (simplified)
    for (auto [tile, x, y] : _grid) {
        if (tile.waterLevel > tile.elevation) {
            distributeWaterToNeighbors(x, y);
        }
    }
    
    // 3. Evaporation based on temperature
    for (auto [tile, x, y] : _grid) {
        float temp = calculateTemperature(x, y);
        float evapRate = 0.001f * (temp / 20.0f);
        tile.waterLevel = std::max(0.f, tile.waterLevel - evapRate);
    }
}
```

**Integration with Sea Level (Phase E):**
- Global sea level affects coastal tiles
- Per-tile water handles inland water
- Rivers flow to sea when sea level drops
- Flooding when sea level rises

#### A8: Event Propagation System

**Why:** Disasters and environmental events need to spread over time. Fire spreads to adjacent forest, floods flow downhill, disease spreads through populations. Critical for Genesis disasters and dynamic ecosystems.

**Implementation:**
```cpp
// Base event class
class WorldEvent {
public:
    virtual ~WorldEvent() = default;
    virtual void tick(World& world) = 0;
    virtual bool isComplete() const = 0;
    virtual std::string getName() const = 0;
    
protected:
    int _startTick;
    int _duration;
};

// Event manager
class EventManager {
public:
    void addEvent(std::unique_ptr<WorldEvent> event);
    void tick(World& world);  // Called each simulation tick
    void cancelEvent(const std::string& name);
    std::vector<WorldEvent*> getActiveEvents() const;
    
private:
    std::vector<std::unique_ptr<WorldEvent>> _activeEvents;
};
```

**Fire Event:**
```cpp
class FireEvent : public WorldEvent {
public:
    FireEvent(int x, int y, float intensity);
    
    void tick(World& world) override {
        // Spread to adjacent forest/grass tiles
        for (auto& cell : _activeCells) {
            // Consume vegetation
            world.burnVegetation(cell.x, cell.y, cell.intensity);
            
            // Spread probability based on:
            // - Wind direction and speed
            // - Terrain type (forest spreads faster)
            // - Moisture level
            spreadToNeighbors(world, cell);
        }
        
        // Cells burn out over time
        removeExtinguishedCells();
    }
    
private:
    std::vector<FireCell> _activeCells;
    float _spreadProbability = 0.3f;
};
```

**Flood Event:**
```cpp
class FloodEvent : public WorldEvent {
public:
    FloodEvent(int x, int y, float waterVolume, int duration);
    
    void tick(World& world) override {
        // Add water at source
        world.getGrid().at(_sourceX, _sourceY).waterLevel += _flowRate;
        
        // Water naturally distributes via A7 water simulation
        // This just maintains the source
        
        _ticksRemaining--;
    }
    
private:
    int _sourceX, _sourceY;
    float _flowRate;
    int _ticksRemaining;
};
```

**Drought Event:**
```cpp
class DroughtEvent : public WorldEvent {
public:
    DroughtEvent(int cx, int cy, int radius, int duration);
    
    void tick(World& world) override {
        for (int dx = -_radius; dx <= _radius; dx++) {
            for (int dy = -_radius; dy <= _radius; dy++) {
                int x = _cx + dx, y = _cy + dy;
                if (!world.getGrid().inBounds(x, y)) continue;
                
                Tile& tile = world.getGrid().at(x, y);
                
                // Accelerate evaporation
                tile.waterLevel *= 0.95f;
                
                // Reduce moisture
                tile.moisture *= 0.98f;
                
                // Plants suffer
                world.stressPlants(x, y, 0.1f);
            }
        }
    }
};
```

**Disease Event (Future - ties to population density):**
```cpp
class DiseaseEvent : public WorldEvent {
    // Spreads through creature populations
    // Affected by density, movement, genetic resistance
};
```

**Event Triggers (for Genesis god tools):**
```cpp
// World methods for triggering events
void World::triggerFire(int x, int y, float intensity);
void World::triggerFlood(int x, int y, float volume, int duration);
void World::triggerDrought(int x, int y, int radius, int duration);
void World::triggerVolcano(int x, int y);  // Combines fire + elevation change
void World::triggerEarthquake(int cx, int cy, float magnitude);  // Elevation shifts
```

### Genes That Become Functional

| Gene | Current | After Phase A |
|------|---------|---------------|
| `TEMP_TOLERANCE_LOW/HIGH` | Unused | Survival depends on biome temperature |
| `LIGHT_REQUIREMENT` | Unused | Plants die in shade if high requirement |
| `WATER_REQUIREMENT` | Unused | Affects growth near water sources |
| `SEED_AERODYNAMICS` | Constant wind | Real wind variations affect dispersal |
| `PHOTOSYNTHESIS` | Constant rate | Varies with light intensity |

### Success Metrics

**A1-A5 (Core Environment):**
- [ ] Day/night cycle: Temperature varies ±10°C
- [ ] Desert tiles: 30-40°C, Snow tiles: -10 to 5°C
- [ ] Forest tiles: 30-50% light reduction
- [ ] Wind speed: Varies 0-15 m/s over time
- [ ] Seasons: 4 distinct seasons with smooth transitions
- [ ] Survival impact: Creatures outside temp tolerance die faster

**A6-A8 (Genesis-Ready):**
- [ ] Tile mutability: Elevation can be modified at runtime
- [ ] Bulk operations: Area effects with falloff work correctly
- [ ] Water sources: Can add/remove water sources dynamically
- [ ] Water flow: Water distributes to lower neighbors
- [ ] Evaporation: Water levels decrease based on temperature
- [ ] Event system: Fire spreads to adjacent flammable tiles
- [ ] Event system: Floods add water over duration
- [ ] Event system: Droughts reduce moisture in area
- [ ] Integration: Events respect wind direction (fire spread)

### Implementation Files

**New Files:**
- `include/world/EnvironmentSystem.hpp` - Core environment calculations
- `src/world/EnvironmentSystem.cpp` - Implementation
- `include/world/WaterSystem.hpp` - Per-tile water state and flow
- `src/world/WaterSystem.cpp` - Water simulation
- `include/world/EventManager.hpp` - Event propagation system
- `src/world/EventManager.cpp` - Event management
- `include/world/events/WorldEvent.hpp` - Base event class
- `include/world/events/FireEvent.hpp` - Fire propagation
- `include/world/events/FloodEvent.hpp` - Flood mechanics
- `include/world/events/DroughtEvent.hpp` - Drought effects
- `src/world/events/FireEvent.cpp` - Implementation
- `src/world/events/FloodEvent.cpp` - Implementation
- `src/world/events/DroughtEvent.cpp` - Implementation

**Modified Files:**
- `include/world/world.hpp` - Add environment system fields
- `src/world/world.cpp` - Integrate environment updates
- `include/world/WorldGrid.hpp` - Add tile mutation methods
- `src/world/WorldGrid.cpp` - Implement tile mutation
- `include/world/tile.hpp` - Add waterLevel, moisture fields
- `src/world/tile.cpp` - Water state methods
- `include/genetics/expression/EnvironmentState.hpp` - Extend with per-tile data
- `src/genetics/organisms/Plant.cpp` - Use dynamic light/temp
- `src/genetics/interactions/SeedDispersal.cpp` - Use real wind data

---

## Phase B: Waste & Dispersal System

**Priority:** 🔴 HIGH  
**Goal:** Enable proper endozoochory (seed dispersal through animal gut passage)  
**Prerequisites:** Phase A (for environmental viability checks)

### Objectives

Implement creature excretion and waste deposition, allowing seeds to pass through digestive systems and be deposited at new locations. This is the primary dispersal method for many plant species and creates realistic nutrient cycling (though full nutrient mechanics are future work).

### Current State

**Existing Infrastructure:**
- ✅ `GUT_TRANSIT_TIME` gene defined (0.5-4.0 hours)
- ✅ `SEED_DESTRUCTION_RATE` gene defined (0-1, grinding vs intact passage)
- ✅ `SEED_COAT_DURABILITY` gene defined (0-1, survival chance)
- ✅ `SeedDispersal::disperseByAnimalFruit()` method exists
- ❌ No gut tracking system
- ❌ No waste deposition
- ❌ No seed-in-waste germination

### Components

#### B1: Gut Transit Tracking

**Implementation:**
- Add `GutContents` struct to track ingested items:
  ```cpp
  struct GutContents {
      struct IngestedItem {
          std::string itemType;        // "seed", "food", etc.
          int tickIngested;
          std::shared_ptr<Genome> seedGenome;  // For seeds
          float viability;             // Survival chance
      };
      std::vector<IngestedItem> contents;
      int capacity = 10;               // Max items in gut
  };
  ```
- When creature eats fruit:
  - Extract seed from fruit (based on fruit's plant genome)
  - Calculate survival chance: `seedCoatDurability vs seedDestructionRate`
  - Add to gut contents with timestamp
- Every tick: Check if `(currentTick - tickIngested) > transitTime`

**New Fields:**
```cpp
// In Creature class
GutContents _gutContents;
```

#### B2: Waste Deposition System

**Implementation:**
- When seed transit time expires:
  - Calculate final viability (may degrade further in gut)
  - Deposit `WasteObject` at creature's current location
  - Remove from gut contents
- Waste object contains:
  - Seed genome (if seed survived)
  - Nutrient value (for future nutrient cycling)
  - Visual representation (tile marking)
  - Decay timer (waste disappears after N ticks)

**New Class:**
```cpp
class WasteDeposit : public GameObject {
private:
    std::vector<SeedPacket> _seeds;  // Seeds in this deposit
    float _nutrientValue;
    int _depositTick;
    int _decayTime = 1000;  // Ticks until disappears
    
public:
    bool hasSeed() const;
    SeedPacket extractSeed();
    bool tryGerminate(World& world, int x, int y);
};
```

#### B3: Seed-in-Waste Germination

**Implementation:**
- Each tick, waste deposits have chance to germinate seeds
- Probability depends on:
  - Seed viability (from gut passage)
  - Environmental conditions (temp, moisture from Phase A)
  - Tile suitability (not on water, not impassable)
- If germination succeeds:
  - Create offspring plant using seed genome
  - Apply mutation
  - Place on tile
  - Remove seed from waste deposit

**Germination Check:**
```cpp
bool WasteDeposit::tryGerminate(World& world, int x, int y) {
    if (!hasSeed()) return false;
    
    SeedPacket seed = extractSeed();
    
    // Environmental checks
    float temp = world.calculateTemperature(x, y);
    float moisture = world.calculateMoisture(x, y);
    
    // Viability check
    if (random(0, 1) > seed.viability) return false;
    
    // Germinate!
    Plant offspring = PlantFactory::createOffspring(
        seed.genome, seed.genome, x, y
    );
    world.addGeneticsPlant(x, y, offspring);
    return true;
}
```

### Coevolutionary Dynamics Enabled

With this system, plant-animal coevolution becomes possible:

**For Plants:**
- Evolve attractive fruit (`FRUIT_APPEAL`)
- Evolve durable seed coats (`SEED_COAT_DURABILITY`)
- Trade-off: Appeal vs defense

**For Animals:**
- Evolve fruit preference (`SWEETNESS_PREFERENCE`)
- Evolve seed grinding (`SEED_DESTRUCTION_RATE`)
- Trade-off: Extract calories vs aid dispersal

**Emergent Mutualism:**
- Plants with high appeal + high durability get dispersed
- Animals with low grinding get more nutrient from fruit
- Beneficial relationship emerges from individual selection

### Success Metrics

- [ ] Seeds pass through creature guts based on `GUT_TRANSIT_TIME`
- [ ] Seed survival based on durability vs destruction
- [ ] Waste deposits appear at creature locations
- [ ] Seeds germinate from waste with realistic probability
- [ ] Dispersal distance: 50-500 tiles (depending on creature movement)
- [ ] Observable coevolution: Plants near frequently-visited areas have higher dispersal

### Implementation Files

**New Files:**
- `include/objects/WasteDeposit.hpp` - Waste object class
- `src/objects/WasteDeposit.cpp` - Implementation
- `include/objects/creature/GutContents.hpp` - Gut tracking structure

**Modified Files:**
- `include/objects/creature/creature.hpp` - Add `_gutContents` field
- `src/objects/creature/creature.cpp` - Gut transit logic
- `src/genetics/interactions/SeedDispersal.cpp` - Integration
- `src/world/world.cpp` - Waste deposition and germination

---

## Phase C: Scripted Genetics System (GODk)

**Priority:** 🟡 MEDIUM-HIGH  
**Goal:** Enable safe, extensible gene creation without C++ recompilation  
**Prerequisites:** Phase B (demonstrates gene complexity justifying scripting)

### Objectives

Transition from hardcoded C++ genes to a JSON-based gene definition system with optional custom scripting language. This enables modding, rapid prototyping, and complex emergent behaviors without touching the codebase.

**Why After Phase B:**
- Phases A and B demonstrate the complexity of gene interactions
- Aquatic genes (Phase D) will be first major scripted genes
- Validates the need for safer, more flexible gene creation

### Components

#### C.1: JSON Gene Schema & Loader

**Implementation:**
- Define JSON schema for gene definitions
- Implement `GODkLoader` class to parse JSON files
- Load genes at runtime into `GeneRegistry`
- Backward compatible with existing C++ genes

**Example Gene Definition:**
```json
{
  "version": "1.0",
  "gene_pack": {
    "id": "aquatic_adaptations",
    "name": "Aquatic Life Gene Pack",
    "author": "EcoSim Core"
  },
  "genes": [
    {
      "id": "aquatic_locomotion",
      "chromosome": "Mobility",
      "limits": {
        "min": 0.0,
        "max": 1.0,
        "creep": 0.05
      },
      "dominance": "complete",
      "description": "Swimming efficiency in water",
      "effects": [
        {
          "domain": "locomotion",
          "trait": "water_speed",
          "type": "multiplicative",
          "scale": 2.0
        },
        {
          "domain": "locomotion",
          "trait": "land_speed",
          "type": "multiplicative",
          "scale": 0.5,
          "note": "Trade-off: Good swimmer, poor walker"
        }
      ]
    }
  ]
}
```

**Validation:**
- Check schema compliance
- Verify unique gene IDs
- Warn on balance issues (extreme values)
- Detect circular dependencies

#### C.2: Gene Parser & Validator

**Implementation:**
- CLI tool: `godk validate <gene_pack.json>`
- Automated checks:
  - JSON syntax
  - Schema compliance
  - Gene ID uniqueness
  - Effect references to valid traits
  - Balance analysis (warn on extremes)
- Outputs warnings and suggestions

**Example Output:**
```
$ godk validate aquatic_pack.json

✓ JSON syntax valid
✓ All gene IDs unique
✓ Effect bindings valid
⚠ Warnings:
  - aquatic_locomotion: Land speed penalty (0.5x) may be too harsh
✓ Gene Pack Score: 92/100 (Excellent)
```

#### C.3: Custom Scripting Language (GeneScript)

**Implementation:**
- Simple expression language for gene effects
- Safe sandboxed execution
- Built-in functions for common calculations
- Access to phenotype traits and environment state

**GeneScript Example:**
```javascript
// Gene: aquatic_locomotion
function calculateWaterSpeed(phenotype, environment) {
    let baseSpeed = phenotype.locomotion;
    let waterDepth = environment.waterDepth;
    let expression = phenotype.getGeneExpression("aquatic_locomotion");
    
    if (waterDepth > 0) {
        return baseSpeed * (1.0 + expression);
    }
    return baseSpeed;
}

function calculateLandSpeed(phenotype, environment) {
    let baseSpeed = phenotype.locomotion;
    let expression = phenotype.getGeneExpression("aquatic_locomotion");
    
    // Trade-off: Better swimmer, worse walker
    return baseSpeed * (1.0 - 0.5 * expression);
}
```

**Safety Features:**
- No file I/O access
- Limited computation time
- Memory limits
- Cannot crash simulation
- Cannot access arbitrary memory

#### C.4: Behavior Scripting

**Implementation:**
- Trigger-based behavior system
- Genes can define custom behaviors
- Example: Hibernation triggered by low temperature

**Behavior Example:**
```json
{
  "behaviors": [
    {
      "id": "temperature_hibernation",
      "trigger": {
        "condition": "environment.temperature < phenotype.temp_tolerance_low"
      },
      "action": "enter_hibernation",
      "parameters": {
        "metabolism_multiplier": 0.1,
        "movement_disabled": true,
        "energy_drain": 0.001
      }
    }
  ]
}
```

### Migration Strategy

**Phase 1: Parallel Systems**
- Keep existing C++ genes
- Add JSON gene loading
- Both systems coexist

**Phase 2: Gradual Migration**
- Port simple genes to JSON
- Test extensively
- Keep complex genes in C++

**Phase 3: Full Scripting (Future)**
- All genes in JSON/GeneScript
- C++ becomes runtime only
- Maximum flexibility

### Success Metrics

- [ ] JSON gene loader working
- [ ] Validator catches common errors
- [ ] First scripted gene (aquatic_locomotion) functional
- [ ] Performance: <1% overhead vs C++ genes
- [ ] Safety: Scripted genes cannot crash simulation
- [ ] Community adoption: 3+ community gene packs

### Implementation Files

**New Files:**
- `include/godk/GODkLoader.hpp` - JSON loading
- `src/godk/GODkLoader.cpp` - Implementation
- `include/godk/GeneValidator.hpp` - Validation logic
- `src/godk/GeneValidator.cpp` - Implementation
- `include/godk/GeneScript.hpp` - Scripting engine
- `src/godk/GeneScript.cpp` - Interpreter
- `tools/godk_validator.cpp` - CLI validator tool

**Modified Files:**
- `include/genetics/core/GeneRegistry.hpp` - Support JSON genes
- `src/main.cpp` - Load gene packs at startup

### Documentation

- `docs/modding/godk-getting-started.md` - Tutorial
- `docs/modding/gene-schema.md` - JSON schema reference
- `docs/modding/genescript-api.md` - Scripting API
- `docs/modding/examples/` - Example gene packs

---

## Phase D: Aquatic Life

**Priority:** 🟢 MEDIUM  
**Goal:** Enable organisms to inhabit and adapt to aquatic environments  
**Prerequisites:** Phase A (environmental systems), Phase C (scripted genes preferred)

### Objectives

Transform water from "impassable terrain" or "walkable wet ground" into a full habitat with unique selective pressures. Enable evolution of aquatic specialists, amphibious generalists, and full aquatic-terrestrial transitions.

### New Genes

All aquatic genes should be defined via Phase C GODk (JSON/GeneScript) as a demonstration of the scripting system's capabilities.

#### D.1: Aquatic Locomotion

```json
{
  "id": "aquatic_locomotion",
  "range": [0.0, 1.0],
  "creep": 0.05,
  "description": "Swimming efficiency in water",
  "effects": {
    "water_speed": "base_speed * (1.0 + 2.0 * expression)",
    "land_speed": "base_speed * (1.0 - 0.5 * expression)"
  },
  "trade_off": "Better swimmer, poorer walker"
}
```

#### D.2: Stamina System

**Prerequisite for aquatic mechanics:**

```cpp
struct Stamina {
    float current = 100.0f;
    float maximum = 100.0f;
    float drainRate = 1.0f;    // Per tick while moving
    float regenRate = 2.0f;    // Per tick while resting
    
    bool canMove() const { return current > 0; }
    void drain(float amount) { current = max(0, current - amount); }
    void regen(float amount) { current = min(maximum, current + amount); }
};
```

**Stamina affects:**
- Movement speed (reduced when low stamina)
- Can't move when stamina = 0 (forced rest)
- Swimming drains stamina faster than walking
- Holding breath drains stamina (see lung capacity)

#### D.3: Lung Capacity & Oxygen

```json
{
  "id": "lung_capacity",
  "range": [0.5, 2.0],
  "creep": 0.1,
  "description": "How long can hold breath underwater",
  "effects": {
    "oxygen_max": "100 * expression",
    "oxygen_drain_rate": "5.0 / expression"
  }
}
```

**Oxygen Mechanics:**
- Oxygen meter: 0-100
- Drains while underwater (rate based on lung_capacity)
- When oxygen = 0: Health drains rapidly (drowning)
- Regenerates when surfaced (instant on land, gradual in shallow water)

#### D.4: Gill Efficiency

```json
{
  "id": "gill_efficiency",
  "range": [0.0, 1.0],
  "creep": 0.05,
  "description": "Extract oxygen from water",
  "effects": {
    "oxygen_regen_underwater": "5.0 * expression",
    "note": "High efficiency = can breathe underwater"
  }
}
```

**Gill Mechanics:**
- `gill_efficiency = 0.0`: No oxygen regen underwater (pure lung-breather)
- `gill_efficiency = 0.3`: Partial regen (amphibian)
- `gill_efficiency = 1.0`: Full regen (fish-like, never needs surface)

**Trade-off:**
- High gills: Can live underwater permanently
- Cost: Energy cost for maintaining gills

#### D.5: Supporting Genes

```json
{
  "genes": [
    {
      "id": "webbed_appendages",
      "range": [0.0, 1.0],
      "description": "Physical swimming adaptations",
      "effects": {
        "aquatic_locomotion_bonus": "+0.3 * expression",
        "land_locomotion_penalty": "-0.2 * expression"
      }
    },
    {
      "id": "dive_depth",
      "range": [0.0, 1.0],
      "description": "Tolerance for water pressure",
      "effects": {
        "max_safe_depth": "10 * expression",
        "note": "Future: Deep water tiles"
      }
    },
    {
      "id": "buoyancy_control",
      "range": [0.0, 1.0],
      "description": "Control depth in water column",
      "effects": {
        "energy_cost_swimming": "-0.5 * expression",
        "note": "Swim bladder efficiency"
      }
    }
  ]
}
```

### Aquatic Plants

**New Plant Genes:**

```json
{
  "genes": [
    {
      "id": "submersion_tolerance",
      "range": [0.0, 1.0],
      "description": "Can survive underwater",
      "effects": {
        "photosynthesis_underwater": "photosynthesis * expression * light_penetration"
      }
    },
    {
      "id": "water_anchoring",
      "range": [0.0, 1.0],
      "description": "Root or anchor to underwater substrate",
      "effects": {
        "survival_in_water": "expression > 0.5"
      }
    },
    {
      "id": "floating_leaves",
      "range": [0.0, 1.0],
      "description": "Leaves float on water surface",
      "effects": {
        "photosynthesis_in_water": "full efficiency when expression > 0.7"
      }
    }
  ]
}
```

**Plant Niches:**
- **Emergent:** Rooted underwater, leaves above surface (lotus)
- **Floating:** Entire plant floats (duckweed)
- **Submerged:** Entire plant underwater (seagrass)

### Movement Mechanics

**Current:**
- Creature can move if `tile.isPassable() == true`

**After Phase D:**
```cpp
bool Creature::canMoveTo(Tile& tile, World& world) {
    if (!tile.isPassable()) return false;
    
    // Check if tile is water
    if (tile.isWater()) {
        // Need aquatic locomotion to swim efficiently
        float aquaticSkill = phenotype.getTrait("aquatic_locomotion");
        
        // Or at least some stamina to struggle through
        if (aquaticSkill < 0.1 && stamina.current < 10) {
            return false;  // Can't swim, too tired
        }
        
        // Check oxygen if submerged
        if (tile.getWaterDepth() > 1.0) {
            if (oxygen.current <= 0) {
                return false;  // Would drown
            }
        }
    }
    
    return true;
}
```

### Evolutionary Pathways

**1. Pure Aquatic (Fish-like):**
- High `aquatic_locomotion` (0.8-1.0)
- High `gill_efficiency` (0.8-1.0)
- Low `lung_capacity` (0.3-0.5)
- **Trade-off:** Can't survive on land, excellent in water

**2. Amphibious (Frog-like):**
- Medium `aquatic_locomotion` (0.5-0.7)
- Medium `gill_efficiency` (0.3-0.5) or high `lung_capacity` (1.2-1.5)
- Balanced land/water speeds
- **Trade-off:** Master of neither, competent at both

**3. Diving Specialist (Otter-like):**
- High `lung_capacity` (1.5-2.0)
- Medium-high `aquatic_locomotion` (0.6-0.8)
- Low `gill_efficiency` (0-0.1)
- **Trade-off:** Land-based but can dive for food

**4. Terrestrial (Baseline):**
- Low `aquatic_locomotion` (0.0-0.2)
- No `gill_efficiency` (0.0)
- Medium `lung_capacity` (0.8-1.0)
- **Risk:** Drowns if stuck in deep water

### Success Metrics

- [ ] Stamina system: Creatures must rest periodically
- [ ] Oxygen system: Creatures drown without air/gills
- [ ] Swimming: Aquatic creatures move faster in water
- [ ] Land penalty: Aquatic creatures slower on land
- [ ] Plant survival: Aquatic plants thrive in water
- [ ] Evolution: Aquatic specialists emerge near water biomes
- [ ] Speciation: Distinct aquatic vs terrestrial lineages

### Implementation Files

**Scripted Genes:**
- `gene_packs/aquatic_adaptations.json` - All aquatic genes

**New C++ Systems:**
- `include/objects/creature/Stamina.hpp` - Stamina system
- `include/objects/creature/Oxygen.hpp` - Oxygen/breath system
- `src/objects/creature/Stamina.cpp` - Implementation
- `src/objects/creature/Oxygen.cpp` - Implementation

**Modified Files:**
- `include/objects/creature/creature.hpp` - Add stamina/oxygen fields
- `src/objects/creature/creature.cpp` - Movement mechanics
- `include/world/tile.hpp` - Add water depth queries
- `src/genetics/organisms/Plant.cpp` - Aquatic plant support

---

## Phase E: Dynamic World Systems

**Priority:** 🟢 MEDIUM  
**Goal:** Create dynamic environmental changes that drive speciation  
**Prerequisites:** Phase A (seasons), Phase D (aquatic adaptations)

### Objectives

Add temporal environmental changes that create and destroy habitats, isolating populations and driving speciation. Primary focus: seasonal sea level changes that create temporary islands.

### Components

#### E.1: Dynamic Sea Level System

**Concept:**
- Sea level rises and falls with seasons
- Creates temporary islands during low tide/dry season
- Floods land during high tide/wet season
- Populations become isolated → genetic drift → subspeciation

**Implementation:**
```cpp
class SeaLevelSystem {
private:
    float _baseSeaLevel = 120.0f;      // Base elevation for water
    float _seasonalVariation = 15.0f;  // ±15 elevation units
    float _currentSeaLevel;
    
public:
    void update(Season season, float seasonProgress) {
        // Sea level peaks in winter (wet season)
        float seasonalOffset = _seasonalVariation * 
            sin(seasonProgress * 2.0 * M_PI);
        _currentSeaLevel = _baseSeaLevel + seasonalOffset;
    }
    
    bool isTileFlooded(const Tile& tile) const {
        return tile.getElevation() < _currentSeaLevel;
    }
};
```

**Effects:**
- Summer (dry): Low sea level → islands emerge
- Winter (wet): High sea level → islands flood
- Coastal tiles transition: SAND ↔ SHALLOW_WATER
- Creates transient land bridges

**Population Isolation Example:**
```
Spring (Medium Sea Level):
  [Land] [Land] [Land]
  [Water] [Water] [Water]
  [Land] [Island] [Land]

Summer (Low Sea Level):
  [Land] [Land] [Land]
  [Land] [Land] [Land]  ← Land bridge forms
  [Land] [Island] [Land]
  
Winter (High Sea Level):
  [Water] [Water] [Water]  ← Coastal areas flood
  [Water] [Water] [Water]
  [Water] [Island] [Water] ← Island shrinks
```

**Evolutionary Pressure:**
- Aquatic creatures: Follow water, trapped in shrinking pools
- Terrestrial creatures: Trapped on seasonal islands
- Selection: Aquatic adaptations on wet side, terrestrial on dry side

#### E.2: Biome Transitions

**Implementation:**
- Temperature/moisture gradients → biome boundaries shift
- Seasonal: PLAINS ↔ SAVANNA based on moisture
- Long-term: Climate change → biome migration
- Plants must adapt or migrate with biome

**Transition Rules:**
```cpp
TerrainType calculateBiome(float temp, float moisture, int elevation) {
    if (elevation > 200) {
        if (temp < 0) return SNOW;
        if (temp < 10) return MOUNTAIN;
    }
    
    if (moisture < 0.3) {
        if (temp > 30) return DESERT_SAND;
        return SAVANNA;
    }
    
    if (moisture > 0.7) {
        if (temp > 15) return FOREST;
        return PLAINS;
    }
    
    return PLAINS;  // Default
}
```

**Dynamic Updates:**
- Each season: Recalculate biome for each tile
- Smooth transitions (not abrupt flips)
- Plants/creatures must adapt to shifting biomes

#### E.3: Climate Zones

**Implementation:**
- Latitude affects temperature (equator hot, poles cold)
- Creates stable climate zones:
  - Tropical (low latitude, high temp)
  - Temperate (mid latitude, moderate temp)
  - Polar (high latitude, low temp)
- Drives species distribution

**Latitude System:**
```cpp
float World::getLatitude(int y) const {
    // Normalize y to [0, 1], where 0 = south pole, 1 = north pole
    return static_cast<float>(y) / _mapGen.rows;
}

float World::getBaseTemperatureForLatitude(int y) const {
    float latitude = getLatitude(y);
    float latitudeFactor = cos(latitude * M_PI);  // -1 to 1
    
    // Tropical: 30°C, Polar: -10°C
    return 10.0f + 20.0f * latitudeFactor;
}
```

### Success Metrics

- [ ] Sea levels: Vary ±15 elevation units with seasons
- [ ] Islands: Emerge in summer, flood in winter
- [ ] Isolation: Populations on islands diverge genetically
- [ ] Biome shifts: Observable transitions over seasons
- [ ] Climate zones: Temperature correlates with latitude
- [ ] Speciation: Distinct populations in isolated areas

### Implementation Files

**New Files:**
- `include/world/SeaLevelSystem.hpp`
- `src/world/SeaLevelSystem.cpp`
- `include/world/BiomeTransition.hpp`
- `src/world/BiomeTransition.cpp`

**Modified Files:**
- `include/world/world.hpp` - Add sea level system
- `src/world/world.cpp` - Update tiles based on sea level
- `src/world/tile.cpp` - Handle biome transitions

---

## Phase F: Conditional Gene Expression

**Priority:** 🟡 LOW-MEDIUM  
**Goal:** Enable genes to activate/deactivate based on environmental conditions  
**Prerequisites:** Phase A (seasons), Phase E (dynamic environment)

### Objectives

Allow organisms to respond to environmental changes by modulating gene expression. Enables hibernation, seasonal coat changes, and eventually complex metamorphosis.

### Components

#### F.1: Seasonal Gene Switching

**Concept:**
- Genes have "active seasons"
- Expression automatically adjusts with season
- Example: Fur density high in winter, low in summer

**Implementation:**
```cpp
struct ConditionalExpression {
    std::string geneId;
    
    // Seasonal modulation
    float springMultiplier = 1.0f;
    float summerMultiplier = 1.0f;
    float autumnMultiplier = 1.0f;
    float winterMultiplier = 1.0f;
    
    // Temperature threshold
    float activeBelowTemp = 100.0f;  // Always active if temp < this
    float activeAboveTemp = -100.0f; // Always active if temp > this
    
    float calculateModifier(Season season, float temperature) const;
};
```

**Example: Winter Fur**
```json
{
  "id": "thick_fur",
  "conditional_expression": {
    "winter_multiplier": 2.0,
    "summer_multiplier": 0.5,
    "active_below_temp": 10.0
  }
}
```

#### F.2: Hibernation Behavior

**Concept:**
- Triggered when temperature drops below tolerance
- Creature enters dormant state:
  - Metabolism reduced to 10%
  - Cannot move
  - Energy drain minimized
  - Wakes when temperature rises

**Implementation:**
```cpp
class HibernationBehavior : public ConditionalBehavior {
public:
    bool shouldActivate(const Creature& c, const EnvironmentState& env) override {
        float tolerance = c.getPhenotype().getTrait("temp_tolerance_low");
        return env.temperature < tolerance - 5.0f;  // 5°C buffer
    }
    
    void onActivate(Creature& c) override {
        c.setMetabolismMultiplier(0.1f);
        c.setMovementEnabled(false);
        c.setProfile(Profile::rest);
    }
    
    bool shouldDeactivate(const Creature& c, const EnvironmentState& env) override {
        float tolerance = c.getPhenotype().getTrait("temp_tolerance_low");
        return env.temperature > tolerance;  // Warm enough
    }
    
    void onDeactivate(Creature& c) override {
        c.setMetabolismMultiplier(1.0f);
        c.setMovementEnabled(true);
    }
};
```

**Energy Savings:**
```
Normal creature: 0.01 energy/tick
Hibernating creature: 0.001 energy/tick (10x reduction)

Over 5000 tick winter:
  Normal: -50 energy (starves)
  Hibernating: -5 energy (survives easily)
```

#### F.3: Fur/Coat Variation

**Implementation:**
- `fur_density` gene expression varies with season
- High in winter → better insulation, slower heat loss
- Low in summer → faster cooling, prevents overheating

**Physics:**
```cpp
float calculateHeatLoss(const Creature& c, const EnvironmentState& env) {
    float tempDifference = c.bodyTemp - env.temperature;
    float furInsulation = c.getPhenotype().getTrait("fur_density");
    float heatLossRate = 0.1f / (1.0f + furInsulation);
    
    return tempDifference * heatLossRate;
}
```

**Seasonal Adaptation:**
```
Summer (30°C):
  - Fur sheds (expression: 0.3)
  - Less insulation
  - Easier to stay cool
  
Winter (0°C):
  - Fur grows thick (expression: 1.0)
  - High insulation
  - Retains body heat
```

#### F.4: Future: Complex Metamorphosis

**Vision (Post-Phase F):**
- Life stage-based expression
- Larval stage: Aquatic genes active
- Adult stage: Terrestrial genes active
- Full metamorphosis emerges from conditional expression

**Not Implemented Yet:**
- Requires life stage system
- Requires morphological changes
- Extremely complex

### Success Metrics

- [ ] Seasonal switching: Fur density changes with season
- [ ] Hibernation: Creatures survive winter through dormancy
- [ ] Energy efficiency: Hibernation reduces energy use by 90%
- [ ] Temperature survival: Hibernators survive outside tolerance range
- [ ] Coat variation: Visible difference in summer vs winter creatures
- [ ] Evolution: Hibernation evolves in cold climate populations

### Implementation Files

**New Files:**
- `include/genetics/expression/ConditionalExpression.hpp`
- `src/genetics/expression/ConditionalExpression.cpp`
- `include/behavior/HibernationBehavior.hpp`
- `src/behavior/HibernationBehavior.cpp`

**Modified Files:**
- `include/genetics/expression/Phenotype.hpp` - Support conditional traits
- `src/genetics/expression/Phenotype.cpp` - Calculate modulated expression
- `include/objects/creature/creature.hpp` - Hibernation state
- `src/objects/creature/creature.cpp` - Hibernation logic

---

## Implementation Timeline

**Note:** These are rough estimates and will be refined as each phase begins.

| Phase | Duration | Effort | Complexity |
|-------|----------|--------|------------|
| **A: Environment Foundation** | 4-6 weeks | High | Medium-High |
| **B: Waste & Dispersal** | 2-3 weeks | Medium | Medium |
| **C: Scripted Genetics (GODk)** | 4-6 weeks | Very High | High |
| **D: Aquatic Life** | 2-3 weeks | Medium | Medium |
| **E: Dynamic World** | 2-3 weeks | Medium | Medium-High |
| **F: Conditional Expression** | 2-3 weeks | Medium | Medium |
| **TOTAL** | 17-25 weeks | | |

**Phase A Breakdown:**
| Component | Duration | Notes |
|-----------|----------|-------|
| A1-A2: Day/Night + Wind | 1 week | Core time systems |
| A3-A4: Temperature + Light | 1 week | Per-tile calculations |
| A5: Seasons | 0.5 week | Builds on A1-A4 |
| A6: Tile Mutability | 1 week | API + bulk operations |
| A7: Per-Tile Water | 1-1.5 weeks | Water simulation |
| A8: Event Propagation | 1-1.5 weeks | Fire, flood, drought |

**Parallelization Opportunities:**
- Phase A can be split: A1-A5 (1 person), A6-A8 (another person)
- A6-A8 are Genesis-critical but can be developed in parallel with core environment
- C.1-C.2 (loader/validator) parallel with C.3 (scripting)

### Genesis Development Path

For **Genesis (God Sim)** specifically, the minimum viable path is:

```
Phase A (Complete) → Genesis Alpha
                   ↓
              God Tools Layer (2-3 weeks)
                   ↓
              Scenario System (1-2 weeks)
                   ↓
              Genesis Beta
```

**Genesis can launch after Phase A** with god tools built on top. Phases B-F enhance the simulation but aren't required for core Genesis gameplay.

---

## Deferred Features

These features were discussed but are explicitly deferred to future phases:

### Sensory System Phases 3-6

**Status:** Deferred  
**Rationale:** Environment systems take priority to make existing genes functional

**Includes:**
- Phase 3: Auditory System (hearing)
- Phase 4: Exploration Memory
- Phase 5: Full Mating Integration (with sound + scent)
- Phase 6: Predator-Prey Sensory Applications

**Future Priority:** HIGH (after Phase E)

**Reference:** `docs/future/sensory-system-phases.md`

### Nutrient Recycling

**Status:** Far Future  
**Rationale:** Waste deposition (Phase B) is prerequisite, but full nutrient cycling adds significant complexity

**Includes:**
- Decomposition rates
- Soil nutrient levels
- Plant nutrient uptake
- Nutrient competition

**Implementation Note:** Phase B creates infrastructure (waste deposits), but nutrient tracking is separate later phase.

### Complex Metamorphosis

**Status:** Far Future  
**Rationale:** Requires conditional expression (Phase F) + life stages + morphological changes

**Includes:**
- Life stage system (larva, juvenile, adult)
- Stage-specific gene expression
- Morphological transformations
- Example: Aquatic larva → Terrestrial adult

---

## Risk Assessment

### Phase A Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Performance hit from per-tile calculations | Medium | Medium | Spatial caching, update only changed tiles |
| Day/night affecting readability | Low | Low | Configurable, can disable for play |
| Biome temperatures too harsh | Medium | Medium | Start conservative, tune based on survival rates |

### Phase B Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Too many waste objects | Medium | Medium | Auto-cleanup after N ticks, limited spawns |
| Seed dispersal too effective | Low | Medium | Tune viability rates, germination probability |
| Gut capacity overwhelms creature | Low | Low | Hard cap at 10 items, FIFO eviction |

### Phase C Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Scripting performance | High | High | Compile to bytecode, cache results |
| Script security vulnerabilities | Medium | High | Sandboxing, no file I/O, execution limits |
| Community adoption low | Medium | Low | Good documentation, example packs |
| Maintenance burden | Medium | Medium | Automated testing, validation tools |

### Phase D Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Aquatic creatures stuck underwater | Medium | Medium | Pathfinding respects oxygen, flee to surface |
| Drowning deaths too frequent | Medium | High | Conservative oxygen drain rates, test extensively |
| Water pathfinding expensive | High | Medium | A* caching, limit pathfinding depth |

### Phase E Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Sea level changes too frequent | Low | Medium | Slow transitions (1000s of ticks) |
| Island populations go extinct | Medium | High | Start with large islands, gentle isolation |
| Biome transitions jarring | Low | Low | Smooth interpolation, gradual changes |

### Phase F Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Conditional expression too complex | Medium | Medium | Start simple (seasons only), iterate |
| Hibernation exploited | Low | Low | Energy costs for entering/exiting |
| Balance issues | Medium | Medium | Extensive testing, tunable parameters |

---

## Success Criteria

### Phase A Success
- ✅ All environment-dependent genes (temp, light, wind) have observable effects
- ✅ Creatures/plants die when outside tolerance ranges
- ✅ Seed dispersal varies with wind conditions
- ✅ Day/night cycle affects behavior and photosynthesis

### Phase B Success
- ✅ Seeds pass through guts and are deposited in waste
- ✅ Dispersal distance 50-500 tiles via animal movement
- ✅ Observable coevolution: Fruit appeal evolves with seed durability
- ✅ Germination success correlated with environmental suitability

### Phase C Success
- ✅ JSON genes load and function identically to C++ genes
- ✅ Validator catches common errors before runtime
- ✅ First community gene pack published and working
- ✅ Performance within 5% of C++ implementation
- ✅ Zero simulation crashes from scripted genes

### Phase D Success
- ✅ Distinct aquatic and terrestrial lineages emerge
- ✅ Amphibious generalists exist at water boundaries
- ✅ Aquatic plants provide food sources in water
- ✅ Drowning/oxygen mechanics function realistically
- ✅ Visible speciation driven by aquatic vs terrestrial niches

### Phase E Success
- ✅ Seasonal islands emerge and flood predictably
- ✅ Genetic divergence measurable in isolated populations
- ✅ Biome boundaries shift with climate
- ✅ Latitude-temperature gradient established
- ✅ Climate-driven migrations observable

### Phase F Success
- ✅ Hibernation enables winter survival in cold biomes
- ✅ Fur density varies seasonally in observable creatures
- ✅ Energy consumption reduced 90% during hibernation
- ✅ Conditional expression evolves (creatures in cold areas gain hibernation)
- ✅ No exploitation/balance issues

---

## Maintenance & Iteration

After each phase:

1. **Collect Metrics**
   - Performance profiling
   - Survival rates
   - Gene frequency changes
   - Community feedback

2. **Balance Tuning**
   - Adjust gene ranges
   - Tune energy costs
   - Fix exploits

3. **Documentation**
   - Update user guides
   - API documentation
   - Example scenarios

4. **Community Engagement**
   - Gather feedback
   - Share progress videos
   - Beta testing

---

## Related Documents

- [[../behavior/sensory-phases|Sensory System Phases]] - Deferred sensory expansion
- [[../genetics/godk|GODk]] - Full GODk vision
- [`include/genetics/defaults/UniversalGenes.hpp`](../../include/genetics/defaults/UniversalGenes.hpp) - Current gene definitions
- [`include/world/world.hpp`](../../include/world/world.hpp) - World system architecture
- [`plans/omnivore-trade-offs-design.md`](../../../plans/omnivore-trade-offs-design.md) - Dietary coevolution mechanics

---

**Last Updated:** 2025-12-23  
**Next Review:** After Phase A Completion
