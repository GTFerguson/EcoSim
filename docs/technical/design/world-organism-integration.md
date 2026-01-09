# World Generation ↔ Organism Integration Design

**Status:** ✅ Implemented
**Created:** 2026-01-09
**Last Updated:** 2026-01-09

> [!NOTE]
> **Implementation Status:** This design has been implemented. See:
> - [`EnvironmentalStress.hpp`](../../../include/genetics/expression/EnvironmentalStress.hpp:1) - Stress calculation functions
> - [`EnvironmentalStress.cpp`](../../../src/genetics/expression/EnvironmentalStress.cpp:1) - Implementation
> - [`test_environmental_stress.cpp`](../../../src/testing/genetics/test_environmental_stress.cpp:1) - Tests
> - [`test_world_organism_integration.cpp`](../../../src/testing/world/test_world_organism_integration.cpp:1) - Integration tests

---

## Executive Summary

This document describes how EcoSim's climate-based world generation system (22 Whittaker biomes) integrates with the organism systems (Plants and Creatures). The design enables meaningful environmental adaptation through:

1. **Fitness Gradients** - Organisms suffer progressive penalties outside their tolerance ranges
2. **Gene-Controlled Pathfinding** - Creatures avoid hostile biomes based on `ENVIRONMENTAL_SENSITIVITY` gene
3. **Biome-Specific Plant Prefabs** - Plants spawn in biomes matching their genetic tolerances
4. **Dual Cost System** - Temperature affects metabolism; terrain affects efficiency

### Key Design Principles

- **No behavioral restrictions** - Organisms can go anywhere, but pay costs
- **Gene-driven decisions** - All behavior emerges from evolvable genes
- **Continuous selection pressure** - Environmental effects apply every tick
- **Existing systems leveraged** - Uses health, energy, and pathfinding systems already in place

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                      ClimateWorldGenerator                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                 │
│  │ TileClimate │  │ BiomeBlend  │  │   Biome     │                 │
│  │ temperature │  │ primary()   │  │ Properties  │                 │
│  │ moisture    │  │ weights[]   │  │ vegetation  │                 │
│  │ elevation   │  │             │  │ movementCost│                 │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘                 │
└─────────┼────────────────┼────────────────┼─────────────────────────┘
          │                │                │
          ▼                ▼                ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    Extended EnvironmentState                         │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ Per-Tile Environment Query                                   │   │
│  │ - getTemperature(x, y) → TileClimate.temperature            │   │
│  │ - getMoisture(x, y) → TileClimate.moisture                  │   │
│  │ - getBiome(x, y) → TileClimate.biome()                      │   │
│  │ - getLightLevel(x, y) → BiomeProperties + time_of_day       │   │
│  │ - getMovementCost(x, y) → BiomeProperties.movementCost      │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
          │                                    │
          ▼                                    ▼
┌─────────────────────────────┐    ┌─────────────────────────────────┐
│       Plant System          │    │        Creature System          │
│  ┌───────────────────────┐  │    │  ┌───────────────────────────┐  │
│  │ Environmental Survival │  │    │  │ Pathfinding Integration   │  │
│  │ - Temperature stress   │  │    │  │ - calculateTileCost()     │  │
│  │ - Moisture stress      │  │    │  │ - ENVIRONMENTAL_SENSITIVITY│  │
│  │ - Light availability   │  │    │  └───────────────────────────┘  │
│  └───────────────────────┘  │    │  ┌───────────────────────────┐  │
│  ┌───────────────────────┐  │    │  │ Environmental Damage      │  │
│  │ Biome-Specific Prefabs │  │    │  │ - Temperature stress      │  │
│  │ - Tundra plants        │  │    │  │ - Energy drain            │  │
│  │ - Desert plants        │  │    │  │ - Health damage           │  │
│  │ - Rainforest plants    │  │    │  └───────────────────────────┘  │
│  └───────────────────────┘  │    │  ┌───────────────────────────┐  │
└─────────────────────────────┘    │  │ Metabolism Scaling        │  │
                                   │  │ - Thermoregulation cost   │  │
                                   │  │ - Terrain efficiency      │  │
                                   │  └───────────────────────────┘  │
                                   └─────────────────────────────────┘
```

---

## Part 1: Extended Environment State

### Current State

The existing [`EnvironmentState`](../../../include/genetics/expression/EnvironmentState.hpp) is a simple struct with static values:

```cpp
struct EnvironmentState {
    float temperature = 20.0f;      // Celsius
    float humidity = 0.5f;          // 0.0 to 1.0
    float time_of_day = 0.5f;       // 0.0 to 1.0
    int terrain_type = 0;
    int season = 0;
};
```

### Proposed Extension

Extend `EnvironmentState` to be a per-tile query interface backed by [`TileClimate`](../../../include/world/ClimateWorldGenerator.hpp):

```cpp
struct EnvironmentState {
    // Core environmental values (populated per-tile)
    float temperature = 20.0f;      // Celsius (-40 to +50)
    float moisture = 0.5f;          // 0.0 to 1.0 (replaces humidity)
    float elevation = 0.5f;         // 0.0 to 1.0 normalized
    float lightLevel = 1.0f;        // 0.0 to 1.0
    float time_of_day = 0.5f;       // 0.0 to 1.0
    
    // Biome information
    Biome primaryBiome = Biome::TEMPERATE_GRASSLAND;
    float vegetationDensity = 0.5f;
    float movementCostModifier = 1.0f;
    
    // Season
    Season season = Season::SPRING;
    
    // Factory method to create from TileClimate
    static EnvironmentState fromTileClimate(const TileClimate& climate, 
                                             float timeOfDay,
                                             Season season);
};
```

### EnvironmentSystem Integration

Modify [`EnvironmentSystem`](../../../include/world/EnvironmentSystem.hpp) to query `ClimateWorldGenerator` data:

```cpp
class EnvironmentSystem {
public:
    // Get complete environment state for a tile
    EnvironmentState getEnvironmentAt(int x, int y) const;
    
    // Individual queries (for performance when only one value needed)
    float getTemperature(int x, int y) const;
    float getMoisture(int x, int y) const;
    float getLightLevel(int x, int y) const;
    Biome getBiome(int x, int y) const;
    
    // Connect to climate data
    void setClimateMap(const std::vector<std::vector<TileClimate>>* climateMap);
    
private:
    const std::vector<std::vector<TileClimate>>* _climateMap = nullptr;
    const SeasonManager& _seasonManager;
    const WorldGrid& _grid;
};
```

---

## Part 2: Environmental Fitness Gradients

### Design Philosophy

Organisms don't die instantly outside their tolerance range. Instead, they experience **progressive fitness penalties** that:
- Drain energy faster (increased metabolism)
- Reduce health over time (environmental damage)
- Reduce efficiency (slower movement, reduced photosynthesis)

This creates selection pressure without mass extinctions at biome boundaries.

### Temperature Stress Calculation

```cpp
struct TemperatureStress {
    float stressLevel;      // 0.0 = comfortable, 1.0+ = severe stress
    float energyDrainRate;  // Additional energy cost per tick
    float healthDamageRate; // Health loss per tick
    bool lethal;            // Instant death threshold exceeded
};

TemperatureStress calculateTemperatureStress(
    float currentTemp,
    float toleranceLow,
    float toleranceHigh
) {
    TemperatureStress result = {0.0f, 0.0f, 0.0f, false};
    
    // Calculate distance from tolerance range
    float distanceFromTolerance = 0.0f;
    
    if (currentTemp < toleranceLow) {
        distanceFromTolerance = toleranceLow - currentTemp;
    } else if (currentTemp > toleranceHigh) {
        distanceFromTolerance = currentTemp - toleranceHigh;
    }
    
    if (distanceFromTolerance <= 0.0f) {
        return result;  // Within tolerance, no stress
    }
    
    // Stress zones:
    // 0-5°C outside: Mild discomfort
    // 5-15°C outside: Moderate stress  
    // 15-25°C outside: Severe stress
    // 25°C+ outside: Lethal
    
    result.stressLevel = distanceFromTolerance / 25.0f;  // Normalized 0-1+
    
    // Energy drain: Linear scaling
    // At 10°C outside tolerance: 50% extra energy cost
    result.energyDrainRate = distanceFromTolerance * 0.05f;
    
    // Health damage: Only starts at moderate stress
    // At 10°C outside tolerance: 0.5% health per tick
    if (distanceFromTolerance > 5.0f) {
        result.healthDamageRate = (distanceFromTolerance - 5.0f) * 0.025f;
    }
    
    // Lethal threshold
    if (distanceFromTolerance > 25.0f) {
        result.lethal = true;
        result.healthDamageRate = 5.0f;  // Rapid death
    }
    
    return result;
}
```

### Moisture Stress (Plants Only)

Plants have moisture requirements that create similar gradients:

```cpp
float calculateMoistureStress(
    float currentMoisture,
    float waterRequirement,
    float waterStorage
) {
    // Plant's effective water need based on genes
    // waterRequirement: How much water plant needs (0=drought-tolerant, 1=water-loving)
    // waterStorage: How much plant can store (succulent adaptation)
    
    float effectiveNeed = waterRequirement * (1.0f - waterStorage * 0.5f);
    float moistureDeficit = effectiveNeed - currentMoisture;
    
    if (moistureDeficit <= 0.0f) {
        return 0.0f;  // Adequate moisture
    }
    
    // Stress increases with deficit
    return moistureDeficit * 2.0f;  // 0-2 range typical
}
```

### Application Points

**For Creatures** - Applied in creature update loop:

```cpp
void Creature::update(const EnvironmentState& env) {
    // Calculate temperature stress
    float toleranceLow = getPhenotype().getTrait("temp_tolerance_low");
    float toleranceHigh = getPhenotype().getTrait("temp_tolerance_high");
    auto stress = calculateTemperatureStress(env.temperature, toleranceLow, toleranceHigh);
    
    // Apply energy drain (increased metabolism)
    float baseMetabolism = getBaseMetabolismCost();
    float adjustedMetabolism = baseMetabolism * (1.0f + stress.energyDrainRate);
    drainEnergy(adjustedMetabolism);
    
    // Apply health damage
    if (stress.healthDamageRate > 0.0f) {
        damage(stress.healthDamageRate);
    }
    
    // Lethal check
    if (stress.lethal) {
        die(DeathCause::ENVIRONMENTAL);
    }
}
```

**For Plants** - Applied in plant update loop:

```cpp
void Plant::update(const EnvironmentState& env) {
    // Temperature stress
    float toleranceLow = getPhenotype().getTrait("temp_tolerance_low");
    float toleranceHigh = getPhenotype().getTrait("temp_tolerance_high");
    auto tempStress = calculateTemperatureStress(env.temperature, toleranceLow, toleranceHigh);
    
    // Moisture stress
    float waterReq = getPhenotype().getTrait("water_requirement");
    float waterStorage = getPhenotype().getTrait("water_storage");
    float moistureStress = calculateMoistureStress(env.moisture, waterReq, waterStorage);
    
    // Combined stress affects growth rate
    float combinedStress = tempStress.stressLevel + moistureStress;
    float growthModifier = 1.0f / (1.0f + combinedStress);
    
    // Apply modified growth
    grow(growthModifier);
    
    // Apply damage from severe stress
    if (tempStress.healthDamageRate > 0.0f) {
        takeDamage(tempStress.healthDamageRate);
    }
    if (moistureStress > 1.0f) {
        takeDamage((moistureStress - 1.0f) * 0.1f);  // Drought damage
    }
}
```

---

## Part 3: Gene-Controlled Pathfinding

### New Gene: ENVIRONMENTAL_SENSITIVITY

This gene controls how much creatures weight environmental danger in pathfinding decisions:

```cpp
// In UniversalGenes.cpp - registerBehaviorGenes()

// Environmental Sensitivity - weighs danger in pathfinding
// [0.0, 2.0], creep 0.1
// 0.0 = Risk-taker, ignores environmental danger
// 1.0 = Normal sensitivity
// 2.0 = Risk-averse, strongly avoids hostile biomes
// Cost: 0.02 - minimal neural cost for danger assessment
GeneDefinition envSensitivity(ENVIRONMENTAL_SENSITIVITY, ChromosomeType::Behavior,
    GeneLimits(0.0f, 2.0f, 0.1f), DominanceType::Incomplete);
envSensitivity.addEffect(EffectBinding("behavior", "environmental_sensitivity", 
    EffectType::Direct, 1.0f));
envSensitivity.setMaintenanceCost(0.02f);
envSensitivity.setCostScaling(1.0f);
envSensitivity.setModulationPolicy(TraitModulationPolicy::NEVER);
registry.tryRegisterGene(std::move(envSensitivity));
```

### Gene Constants

```cpp
// In UniversalGenes.hpp
static constexpr const char* ENVIRONMENTAL_SENSITIVITY = "environmental_sensitivity";
```

### Pathfinding Integration

Modify the pathfinding cost calculation to incorporate environmental danger:

```cpp
// In navigator.cpp or pathfinding system

float calculateTileCost(int x, int y, const Creature& creature, 
                        const EnvironmentSystem& envSystem) {
    // Base terrain movement cost (already exists)
    float baseCost = envSystem.getMovementCost(x, y);
    
    // Calculate environmental danger for this creature
    float environmentalDanger = calculateEnvironmentalDanger(x, y, creature, envSystem);
    
    // Get creature's sensitivity to danger
    float sensitivity = creature.getPhenotype().getTrait("environmental_sensitivity");
    
    // Danger weight factor - tune this for balance
    // Higher = danger matters more in pathfinding
    constexpr float DANGER_WEIGHT_FACTOR = 10.0f;
    
    // Calculate danger cost
    float dangerCost = environmentalDanger * sensitivity * DANGER_WEIGHT_FACTOR;
    
    return baseCost + dangerCost;
}

float calculateEnvironmentalDanger(int x, int y, const Creature& creature,
                                   const EnvironmentSystem& envSystem) {
    float danger = 0.0f;
    
    // Get tile environment
    float temp = envSystem.getTemperature(x, y);
    
    // Get creature's tolerance
    float toleranceLow = creature.getPhenotype().getTrait("temp_tolerance_low");
    float toleranceHigh = creature.getPhenotype().getTrait("temp_tolerance_high");
    
    // Calculate temperature danger
    if (temp < toleranceLow) {
        danger += (toleranceLow - temp) / 10.0f;  // Normalized
    } else if (temp > toleranceHigh) {
        danger += (temp - toleranceHigh) / 10.0f;
    }
    
    // Could add moisture danger for aquatic/desert creatures in future
    
    return danger;
}
```

### Evolutionary Dynamics

This creates interesting trade-offs:

| Sensitivity | Behavior | Selection Pressure |
|-------------|----------|-------------------|
| **0.0-0.3** | Risk-taker | Can access isolated resources but higher mortality |
| **0.8-1.2** | Balanced | Good survival, moderate exploration |
| **1.5-2.0** | Risk-averse | Excellent survival but may miss opportunities |

**Expected Evolution:**
- In resource-rich, safe biomes: Sensitivity drifts randomly
- In fragmented habitats: Lower sensitivity selected (need to cross hostile zones)
- In uniform hostile environments: Higher tolerance genes selected instead

---

## Part 4: Plant Prefab System

### Design Overview

Plants spawn via a prefab system that matches plant templates to appropriate biomes based on their genetic tolerances. This ensures plants spawn where they can survive while still allowing natural selection to optimize populations.

### BiomePlantTemplate Structure

```cpp
struct BiomePlantTemplate {
    std::string templateName;
    
    // Tolerance ranges (used for both spawning and survival)
    float tempToleranceLow;
    float tempToleranceHigh;
    float moistureMin;
    float moistureMax;
    
    // Genetic variance for offspring
    float toleranceVariance = 2.0f;  // °C variance in temperature genes
    float moistureVariance = 0.1f;   // Moisture gene variance
    
    // Biomes this template is suitable for
    std::vector<Biome> suitableBiomes;
    
    // Base genome for this template
    Genome baseGenome;
    
    // Spawn weight (relative frequency)
    float spawnWeight = 1.0f;
};
```

### Biome-Specific Plant Templates

```cpp
void initializePlantTemplates(std::vector<BiomePlantTemplate>& templates) {
    
    // === COLD BIOMES ===
    
    // Tundra Plants (lichens, mosses, hardy shrubs)
    templates.push_back({
        .templateName = "tundra_moss",
        .tempToleranceLow = -30.0f,
        .tempToleranceHigh = 10.0f,
        .moistureMin = 0.3f,
        .moistureMax = 0.8f,
        .suitableBiomes = {Biome::TUNDRA, Biome::ALPINE_TUNDRA},
        .baseGenome = createTundraMossGenome()
    });
    
    templates.push_back({
        .templateName = "arctic_willow",
        .tempToleranceLow = -25.0f,
        .tempToleranceHigh = 15.0f,
        .moistureMin = 0.4f,
        .moistureMax = 0.9f,
        .suitableBiomes = {Biome::TUNDRA, Biome::TAIGA},
        .baseGenome = createArcticWillowGenome()
    });
    
    // Taiga Plants (conifers, hardy undergrowth)
    templates.push_back({
        .templateName = "taiga_conifer",
        .tempToleranceLow = -20.0f,
        .tempToleranceHigh = 20.0f,
        .moistureMin = 0.4f,
        .moistureMax = 0.8f,
        .suitableBiomes = {Biome::TAIGA, Biome::BOREAL_FOREST},
        .baseGenome = createTaigaConiferGenome()
    });
    
    // === TEMPERATE BIOMES ===
    
    templates.push_back({
        .templateName = "temperate_deciduous",
        .tempToleranceLow = -5.0f,
        .tempToleranceHigh = 30.0f,
        .moistureMin = 0.4f,
        .moistureMax = 0.8f,
        .suitableBiomes = {Biome::TEMPERATE_FOREST, Biome::TEMPERATE_GRASSLAND},
        .baseGenome = createTemperateDeciduousGenome()
    });
    
    templates.push_back({
        .templateName = "prairie_grass",
        .tempToleranceLow = -10.0f,
        .tempToleranceHigh = 35.0f,
        .moistureMin = 0.2f,
        .moistureMax = 0.6f,
        .suitableBiomes = {Biome::TEMPERATE_GRASSLAND, Biome::STEPPE},
        .baseGenome = createPrairieGrassGenome()
    });
    
    // === HOT/DRY BIOMES ===
    
    templates.push_back({
        .templateName = "desert_cactus",
        .tempToleranceLow = 5.0f,
        .tempToleranceHigh = 50.0f,
        .moistureMin = 0.0f,
        .moistureMax = 0.3f,
        .suitableBiomes = {Biome::DESERT_HOT, Biome::SHRUBLAND},
        .baseGenome = createDesertCactusGenome()
    });
    
    templates.push_back({
        .templateName = "savanna_acacia",
        .tempToleranceLow = 10.0f,
        .tempToleranceHigh = 45.0f,
        .moistureMin = 0.2f,
        .moistureMax = 0.5f,
        .suitableBiomes = {Biome::SAVANNA, Biome::SHRUBLAND},
        .baseGenome = createSavannaAcaciaGenome()
    });
    
    // === TROPICAL BIOMES ===
    
    templates.push_back({
        .templateName = "rainforest_canopy",
        .tempToleranceLow = 15.0f,
        .tempToleranceHigh = 40.0f,
        .moistureMin = 0.7f,
        .moistureMax = 1.0f,
        .suitableBiomes = {Biome::TROPICAL_RAINFOREST},
        .baseGenome = createRainforestCanopyGenome()
    });
    
    templates.push_back({
        .templateName = "tropical_fern",
        .tempToleranceLow = 18.0f,
        .tempToleranceHigh = 35.0f,
        .moistureMin = 0.6f,
        .moistureMax = 1.0f,
        .suitableBiomes = {Biome::TROPICAL_RAINFOREST, Biome::TROPICAL_SEASONAL_FOREST},
        .baseGenome = createTropicalFernGenome()
    });
}
```

### Plant Spawning Algorithm

```cpp
class BiomePlantSpawner {
public:
    Plant spawnPlantForBiome(int x, int y, const TileClimate& climate) {
        // Find suitable templates for this biome
        std::vector<BiomePlantTemplate*> candidates;
        float totalWeight = 0.0f;
        
        for (auto& template : _templates) {
            if (isTemplateViable(template, climate)) {
                candidates.push_back(&template);
                totalWeight += template.spawnWeight;
            }
        }
        
        if (candidates.empty()) {
            // No suitable template - use generic hardy plant
            return createGenericPlant(x, y);
        }
        
        // Weighted random selection
        float roll = randomFloat(0.0f, totalWeight);
        float cumulative = 0.0f;
        BiomePlantTemplate* selected = nullptr;
        
        for (auto* candidate : candidates) {
            cumulative += candidate->spawnWeight;
            if (roll <= cumulative) {
                selected = candidate;
                break;
            }
        }
        
        // Create plant with genetic variance
        return createPlantFromTemplate(x, y, *selected);
    }
    
private:
    bool isTemplateViable(const BiomePlantTemplate& tmpl, const TileClimate& climate) {
        // Check if template is for this biome
        bool biomeMatch = std::find(tmpl.suitableBiomes.begin(), 
                                    tmpl.suitableBiomes.end(),
                                    climate.biome()) != tmpl.suitableBiomes.end();
        if (!biomeMatch) return false;
        
        // Check temperature compatibility
        if (climate.temperature < tmpl.tempToleranceLow - 5.0f) return false;
        if (climate.temperature > tmpl.tempToleranceHigh + 5.0f) return false;
        
        // Check moisture compatibility
        if (climate.moisture < tmpl.moistureMin - 0.1f) return false;
        if (climate.moisture > tmpl.moistureMax + 0.1f) return false;
        
        return true;
    }
    
    Plant createPlantFromTemplate(int x, int y, const BiomePlantTemplate& tmpl) {
        // Clone base genome
        Genome genome = tmpl.baseGenome;
        
        // Apply variance to tolerance genes
        mutateGeneWithVariance(genome, TEMP_TOLERANCE_LOW, tmpl.toleranceVariance);
        mutateGeneWithVariance(genome, TEMP_TOLERANCE_HIGH, tmpl.toleranceVariance);
        mutateGeneWithVariance(genome, WATER_REQUIREMENT, tmpl.moistureVariance);
        
        return Plant(x, y, genome, _registry);
    }
};
```

---

## Part 5: Creature Prefab System

### BiomeCreatureTemplate Structure

Similar to plants, creatures have biome-appropriate templates:

```cpp
struct BiomeCreatureTemplate {
    std::string templateName;
    
    // Temperature tolerance
    float tempToleranceLow;
    float tempToleranceHigh;
    
    // Environmental sensitivity default
    float environmentalSensitivity = 1.0f;
    
    // Suitable biomes
    std::vector<Biome> suitableBiomes;
    
    // Base genome
    Genome baseGenome;
    
    // Spawn weight
    float spawnWeight = 1.0f;
};
```

### Example Creature Templates

```cpp
void initializeCreatureTemplates(std::vector<BiomeCreatureTemplate>& templates) {
    
    // Arctic predator (polar bear analog)
    templates.push_back({
        .templateName = "arctic_predator",
        .tempToleranceLow = -40.0f,
        .tempToleranceHigh = 15.0f,
        .environmentalSensitivity = 0.8f,  // Willing to venture into moderate cold
        .suitableBiomes = {Biome::TUNDRA, Biome::ICE_SHEET, Biome::TAIGA},
        .baseGenome = createArcticPredatorGenome()
    });
    
    // Tundra grazer (caribou analog)
    templates.push_back({
        .templateName = "tundra_grazer",
        .tempToleranceLow = -35.0f,
        .tempToleranceHigh = 20.0f,
        .environmentalSensitivity = 1.2f,  // Cautious, stays in safe zones
        .suitableBiomes = {Biome::TUNDRA, Biome::TAIGA, Biome::ALPINE_MEADOW},
        .baseGenome = createTundraGrazerGenome()
    });
    
    // Desert omnivore (coyote analog)
    templates.push_back({
        .templateName = "desert_omnivore",
        .tempToleranceLow = 0.0f,
        .tempToleranceHigh = 50.0f,
        .environmentalSensitivity = 0.6f,  // Risk-taker, explores widely
        .suitableBiomes = {Biome::DESERT_HOT, Biome::SHRUBLAND, Biome::STEPPE},
        .baseGenome = createDesertOmnivoreGenome()
    });
    
    // Temperate generalist (deer analog)
    templates.push_back({
        .templateName = "temperate_herbivore",
        .tempToleranceLow = -10.0f,
        .tempToleranceHigh = 35.0f,
        .environmentalSensitivity = 1.0f,  // Balanced
        .suitableBiomes = {Biome::TEMPERATE_FOREST, Biome::TEMPERATE_GRASSLAND, 
                          Biome::SAVANNA},
        .baseGenome = createTemperateHerbivoreGenome()
    });
    
    // Tropical specialist (jaguar analog)
    templates.push_back({
        .templateName = "jungle_predator",
        .tempToleranceLow = 15.0f,
        .tempToleranceHigh = 40.0f,
        .environmentalSensitivity = 1.5f,  // Stays in jungle
        .suitableBiomes = {Biome::TROPICAL_RAINFOREST, Biome::TROPICAL_SEASONAL_FOREST},
        .baseGenome = createJunglePredatorGenome()
    });
}
```

---

## Part 6: Example Scenarios

### Scenario 1: Arctic Fox in Tundra vs Desert

**Setup:**
- Arctic-adapted creature: `tempToleranceLow = -30°C`, `tempToleranceHigh = 15°C`
- `environmentalSensitivity = 1.0`

**In Tundra (-15°C):**
```
Temperature: -15°C (within tolerance)
Stress level: 0.0
Energy drain: 0% extra
Health damage: None
Pathfinding danger cost: 0.0
Result: Thrives, normal behavior
```

**At Tundra/Temperate Border (10°C):**
```
Temperature: 10°C (within tolerance but warm)
Stress level: 0.0
Energy drain: 0% extra
Pathfinding danger cost: 0.0
Result: Still comfortable, may venture south
```

**Entering Desert (40°C):**
```
Temperature: 40°C (25°C over tolerance)
Stress level: 1.0+ (severe)
Energy drain: +125% extra metabolism
Health damage: 0.5% per tick
Pathfinding danger cost: 25.0 (very high)

With sensitivity 1.0:
- Pathfinder adds 250 cost units to desert tiles
- Creature strongly prefers ANY other path
- If no alternative, will cross but suffer heavily
- Likely dies if journey is long
```

### Scenario 2: Desert Cactus Survival

**Setup:**
- Desert plant: `tempToleranceLow = 5°C`, `tempToleranceHigh = 50°C`
- `waterRequirement = 0.1`, `waterStorage = 0.8` (succulent)

**In Desert (moisture 0.15, temp 40°C):**
```
Temperature stress: 0.0 (within tolerance)
Moisture stress: 0.0 (low requirement + high storage)
Growth modifier: 1.0
Result: Thrives
```

**In Rainforest (moisture 0.95, temp 28°C):**
```
Temperature stress: 0.0 (within tolerance)
Moisture stress: 0.0 (excess water, no penalty for having too much)
Growth modifier: 1.0
Competition: Loses to fast-growing rainforest plants
Result: May survive but outcompeted
```

### Scenario 3: Generalist vs Specialist at Biome Edge

**Setup:** Savanna/Tropical Forest boundary

**Tropical Specialist:**
- `tempToleranceLow = 20°C`, `tempToleranceHigh = 35°C` (narrow)
- `environmentalSensitivity = 1.5` (risk-averse)

**Savanna Generalist:**
- `tempToleranceLow = 5°C`, `tempToleranceHigh = 40°C` (wide)
- `environmentalSensitivity = 0.8` (risk-tolerant)

**At Boundary (25°C, food in both biomes):**

| Creature | Savanna Access | Forest Access | Behavior |
|----------|---------------|---------------|----------|
| Specialist | High cost (near edge of tolerance) | Low cost | Stays in forest |
| Generalist | Low cost | Low cost | Uses both freely |

**Food scarcity in forest, abundance in savanna:**
- Specialist: May starve rather than venture into savanna
- Generalist: Moves to savanna, thrives
- Over generations: Generalist genes spread at boundary

---

## Part 7: Implementation Phases

### Phase 1: Environment Data Integration

**Objective:** Connect TileClimate data to organism systems

**Changes:**
1. Extend `EnvironmentState` struct with new fields
2. Modify `EnvironmentSystem` to query climate map
3. Add factory method `EnvironmentState::fromTileClimate()`
4. Update organism `update()` calls to use per-tile environment

**Files Modified:**

| File | Changes |
|------|---------|
| `include/genetics/expression/EnvironmentState.hpp` | Add new fields, factory method |
| `include/world/EnvironmentSystem.hpp` | Add climate map pointer, query methods |
| `src/world/EnvironmentSystem.cpp` | Implement per-tile queries |
| `src/genetics/organisms/Plant.cpp` | Use per-tile environment in update() |
| `src/objects/creature/creature.cpp` | Use per-tile environment in update() |

**Testing:**
- Verify temperature varies across biomes
- Verify moisture varies across biomes
- Verify organisms receive correct per-tile values

### Phase 2: Environmental Stress System

**Objective:** Implement fitness gradient mechanics

**Changes:**
1. Create `EnvironmentalStress.hpp` with calculation functions
2. Apply energy drain based on temperature stress
3. Apply health damage for severe stress
4. Apply growth modifiers for plants

**Files Modified/Created:**

| File | Changes |
|------|---------|
| `include/genetics/expression/EnvironmentalStress.hpp` | New - stress calculation functions |
| `src/genetics/expression/EnvironmentalStress.cpp` | New - implementation |
| `src/genetics/organisms/Plant.cpp` | Apply stress to growth/health |
| `src/objects/creature/creature.cpp` | Apply stress to energy/health |

**Testing:**
- Verify creatures in hostile biomes lose energy faster
- Verify severe stress causes health damage
- Verify plants grow slower under stress
- Tune constants for reasonable survival rates

### Phase 3: Gene-Controlled Pathfinding

**Objective:** Implement ENVIRONMENTAL_SENSITIVITY gene and pathfinding integration

**Changes:**
1. Add `ENVIRONMENTAL_SENSITIVITY` gene to UniversalGenes
2. Modify pathfinding to calculate environmental danger cost
3. Weight danger cost by creature's sensitivity gene

**Files Modified:**

| File | Changes |
|------|---------|
| `include/genetics/defaults/UniversalGenes.hpp` | Add gene constant |
| `src/genetics/defaults/UniversalGenes.cpp` | Register new gene |
| `src/objects/creature/navigator.cpp` | Add environmental cost to pathfinding |
| `include/objects/creature/navigator.hpp` | Add method signature for cost calculation |

**Testing:**
- Verify low-sensitivity creatures cross hostile biomes freely
- Verify high-sensitivity creatures avoid hostile biomes
- Verify pathfinding still works when all paths are hostile
- Verify gene affects only pathfinding, not actual damage

### Phase 4: Biome-Specific Prefabs

**Objective:** Create plant and creature templates for each biome

**Changes:**
1. Create `BiomePlantSpawner` class
2. Create `BiomeCreatureSpawner` class
3. Define templates for all 22 biomes
4. Integrate with existing spawn systems

**Files Created:**

| File | Purpose |
|------|---------|
| `include/world/BiomePlantSpawner.hpp` | Plant template system |
| `src/world/BiomePlantSpawner.cpp` | Implementation |
| `include/world/BiomeCreatureSpawner.hpp` | Creature template system |
| `src/world/BiomeCreatureSpawner.cpp` | Implementation |
| `data/biome_plant_templates.json` | Optional: JSON template definitions |
| `data/biome_creature_templates.json` | Optional: JSON template definitions |

**Testing:**
- Verify correct templates spawn in each biome
- Verify genetic variance produces population diversity
- Verify no crashes from missing templates
- Verify spawned organisms survive in their biomes

### Phase 5: Integration and Tuning

**Objective:** Full integration testing and constant tuning

**Activities:**
1. Run long simulations (10000+ ticks)
2. Monitor population distributions
3. Tune stress constants if populations collapse
4. Tune pathfinding weights if behavior is wrong
5. Add/modify templates as needed

**Success Metrics:**
- [ ] Distinct populations in each biome
- [ ] Biome boundaries are transition zones, not hard walls
- [ ] Some cross-biome migration occurs
- [ ] No mass extinctions from environmental stress alone
- [ ] Clear evolutionary adaptation over generations

---

## Appendix A: Gene Summary

### Existing Genes Used

| Gene | Range | Purpose in Integration |
|------|-------|------------------------|
| `TEMP_TOLERANCE_LOW` | -30°C to +30°C | Minimum survival temperature |
| `TEMP_TOLERANCE_HIGH` | +10°C to +60°C | Maximum survival temperature |
| `WATER_REQUIREMENT` | 0.0 to 1.0 | Plant moisture needs |
| `WATER_STORAGE` | 0.0 to 1.0 | Plant drought resistance |
| `LIGHT_REQUIREMENT` | 0.0 to 1.0 | Plant light needs |
| `PHOTOSYNTHESIS` | 0.0 to 1.0 | Plant energy production |

### New Genes

| Gene | Range | Purpose |
|------|-------|---------|
| `ENVIRONMENTAL_SENSITIVITY` | 0.0 to 2.0 | Pathfinding danger weighting |

### Future Gene Candidates

| Gene | Possible Purpose |
|------|------------------|
| `THERMOREGULATION` | Reduce temperature stress energy cost |
| `MOISTURE_PREFERENCE` | Add pathfinding cost for dry/wet areas |
| `ALTITUDE_ADAPTATION` | Reduce altitude-related stress |
| `SEASONAL_DORMANCY` | Enable hibernation in winter |

---

## Appendix B: Biome Properties Reference

From [`ClimateWorldGenerator.hpp`](../../../include/world/ClimateWorldGenerator.hpp):

| Biome | Temp Range | Moisture | Plants | Trees |
|-------|------------|----------|--------|-------|
| ICE_SHEET | < -20°C | Low | No | No |
| TUNDRA | -20 to 0°C | Low-Med | Yes | No |
| TAIGA | -10 to 10°C | Med | Yes | Yes |
| BOREAL_FOREST | -5 to 15°C | Med-High | Yes | Yes |
| TEMPERATE_RAINFOREST | 5 to 20°C | High | Yes | Yes |
| TEMPERATE_FOREST | 0 to 25°C | Med | Yes | Yes |
| TEMPERATE_GRASSLAND | -5 to 30°C | Low-Med | Yes | No |
| TROPICAL_RAINFOREST | 20 to 35°C | High | Yes | Yes |
| TROPICAL_SEASONAL_FOREST | 15 to 35°C | Med-High | Yes | Yes |
| SAVANNA | 15 to 40°C | Low-Med | Yes | Sparse |
| DESERT_HOT | 20 to 50°C | Very Low | Sparse | No |
| DESERT_COLD | -10 to 30°C | Very Low | Sparse | No |
| STEPPE | -5 to 35°C | Low | Yes | No |
| SHRUBLAND | 10 to 35°C | Low-Med | Yes | No |
| ALPINE_MEADOW | -5 to 15°C | Med | Yes | No |
| ALPINE_TUNDRA | -15 to 5°C | Low | Sparse | No |
| MOUNTAIN_BARE | -20 to 10°C | Low | No | No |
| GLACIER | < -10°C | Low | No | No |

---

## Related Documents

- [[../systems/behavior-system|Behavior System]] - Creature behavior architecture
- [[../../future/environment/world-roadmap|World Roadmap]] - Environmental systems roadmap
- [[../reference/gene-reference|Gene Reference]] - Complete gene documentation
