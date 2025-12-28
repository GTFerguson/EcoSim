---
title: Interactions API Reference
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: reference
tags: [genetics, api, reference, interactions, coevolution]
---

# Interactions API Reference

**Audience:** Developer
**Type:** Reference

---

## Overview

This document covers the system interaction classes. These classes handle ecological interactions between organisms and track coevolutionary dynamics:

- **FeedingInteraction** - Creature-plant feeding with coevolution
- **FeedingResult** - Result of a feeding attempt
- **SeedDispersal** - Seed dispersal mechanics
- **DispersalEvent** - Result of seed dispersal
- **CoevolutionTracker** - Tracks evolutionary arms races
- **CoevolutionStats** - Statistics for a tracked gene pair
- **ArmsRaceConfig** - Configuration for coevolution tracking

All classes are in the `EcoSim::Genetics` namespace.

---

## FeedingInteraction

**Header:** [`include/genetics/interactions/FeedingInteraction.hpp`](include/genetics/interactions/FeedingInteraction.hpp:59)

Handles creature-plant feeding interactions with coevolutionary dynamics.

```cpp
namespace EcoSim::Genetics {

class FeedingInteraction {
public:
    FeedingInteraction() = default;
    
    // Main interaction
    FeedingResult attemptToEatPlant(
        const Phenotype& creaturePhenotype,
        const Plant& plant,
        float creatureHunger = 1.0f
    ) const;
    
    // Detection phase
    bool canDetectPlant(const Phenotype& creature, const Plant& plant, float distance) const;
    float getDetectionRange(const Phenotype& creature, const Plant& plant) const;
    
    // Access phase
    float calculateThornDamage(const Phenotype& creature, const Plant& plant) const;
    float calculateToxinDamage(const Phenotype& creature, const Plant& plant) const;
    bool canOvercomeDefenses(const Phenotype& creature, const Plant& plant) const;
    
    // Digestion phase
    float calculateNutritionExtracted(const Phenotype& creature, const Plant& plant) const;
    float getDigestionEfficiency(const Phenotype& creature, const Plant& plant) const;
    
    // Seed dispersal
    float calculateSeedSurvivalRate(const Phenotype& creature, const Plant& plant) const;
    float calculateDispersalDistance(const Phenotype& creature) const;
    bool willCacheSeeds(const Phenotype& creature) const;
};

}
```

### Methods

#### Main Interaction

| Method | Returns | Description |
|--------|---------|-------------|
| `attemptToEatPlant(creature, plant, hunger)` | `FeedingResult` | Full feeding interaction result |

#### Detection Phase

| Method | Returns | Description |
|--------|---------|-------------|
| `canDetectPlant(creature, plant, distance)` | `bool` | Can creature detect plant at distance? |
| `getDetectionRange(creature, plant)` | `float` | Maximum detection range |

#### Access Phase

| Method | Returns | Description |
|--------|---------|-------------|
| `calculateThornDamage(creature, plant)` | `float` | Damage from thorns (reduced by hide) |
| `calculateToxinDamage(creature, plant)` | `float` | Damage from toxins (reduced by tolerance) |
| `canOvercomeDefenses(creature, plant)` | `bool` | Can creature overcome all defenses? |

#### Digestion Phase

| Method | Returns | Description |
|--------|---------|-------------|
| `calculateNutritionExtracted(creature, plant)` | `float` | Actual nutrition gained |
| `getDigestionEfficiency(creature, plant)` | `float` | Efficiency multiplier (0.0-1.0+) |

#### Seed Dispersal

| Method | Returns | Description |
|--------|---------|-------------|
| `calculateSeedSurvivalRate(creature, plant)` | `float` | Fraction of seeds surviving digestion |
| `calculateDispersalDistance(creature)` | `float` | Expected dispersal distance |
| `willCacheSeeds(creature)` | `bool` | Will creature cache seeds? |

---

## FeedingResult

**Header:** [`include/genetics/interactions/FeedingInteraction.hpp`](include/genetics/interactions/FeedingInteraction.hpp:25)

Result of a feeding attempt.

```cpp
namespace EcoSim::Genetics {

struct FeedingResult {
    bool success = false;           // Did creature successfully eat?
    float nutritionGained = 0.0f;   // Calories extracted
    float damageReceived = 0.0f;    // From thorns, toxins
    float plantDamage = 0.0f;       // Damage dealt to plant
    bool seedsConsumed = false;     // Did creature eat seeds?
    bool seedsDestroyed = false;    // Were seeds ground up?
    float seedsToDisperse = 0;      // Seeds passing through intact
    std::string description;        // For logging/UI
};

}
```

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `success` | `bool` | Whether feeding succeeded |
| `nutritionGained` | `float` | Energy gained by creature |
| `damageReceived` | `float` | Damage taken by creature |
| `plantDamage` | `float` | Damage dealt to plant |
| `seedsConsumed` | `bool` | Whether seeds were eaten |
| `seedsDestroyed` | `bool` | Whether seeds were destroyed |
| `seedsToDisperse` | `float` | Seeds for dispersal |
| `description` | `std::string` | Human-readable description |

---

## SeedDispersal

**Header:** [`include/genetics/interactions/SeedDispersal.hpp`](include/genetics/interactions/SeedDispersal.hpp:73)

Handles seed dispersal mechanics for all six strategies.

```cpp
namespace EcoSim::Genetics {

class SeedDispersal {
public:
    SeedDispersal();
    explicit SeedDispersal(unsigned int randomSeed);
    
    // Strategy-specific dispersal
    DispersalEvent disperseByGravity(const Plant& plant) const;
    DispersalEvent disperseByWind(const Plant& plant, float windSpeed, float windDirection) const;
    DispersalEvent disperseByAnimalFruit(
        const Plant& plant, const Phenotype& creature, int creatureX, int creatureY) const;
    DispersalEvent disperseByAnimalBurr(
        const Plant& plant, const Phenotype& creature, int creatureX, int creatureY) const;
    DispersalEvent disperseByExplosive(const Plant& plant) const;
    DispersalEvent disperseByVegetative(const Plant& plant) const;
    
    // Automatic dispersal
    DispersalEvent disperse(const Plant& plant, const EnvironmentState* env = nullptr) const;
    
    // Burr mechanics
    bool willBurrAttach(const Plant& plant, const Phenotype& creature) const;
    bool willBurrDetach(const Phenotype& creature, int ticksAttached) const;
    float calculateExpectedBurrDistance(const Plant& plant, const Phenotype& creature) const;
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `randomSeed` | `unsigned int` | Seed for random number generation (optional) |

### Strategy-Specific Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `disperseByGravity(plant)` | `DispersalEvent` | Heavy seeds fall near parent |
| `disperseByWind(plant, speed, dir)` | `DispersalEvent` | Wind-carried seeds |
| `disperseByAnimalFruit(plant, creature, x, y)` | `DispersalEvent` | Seeds via digestion |
| `disperseByAnimalBurr(plant, creature, x, y)` | `DispersalEvent` | Seeds via fur attachment |
| `disperseByExplosive(plant)` | `DispersalEvent` | Pod ejection |
| `disperseByVegetative(plant)` | `DispersalEvent` | Clonal spread |

### Automatic Dispersal

| Method | Returns | Description |
|--------|---------|-------------|
| `disperse(plant, env)` | `DispersalEvent` | Use plant's primary strategy |

### Burr Mechanics

| Method | Returns | Description |
|--------|---------|-------------|
| `willBurrAttach(plant, creature)` | `bool` | Will burr attach to creature? |
| `willBurrDetach(creature, ticks)` | `bool` | Will burr detach after ticks? |
| `calculateExpectedBurrDistance(plant, creature)` | `float` | Expected travel distance |

---

## DispersalEvent

**Header:** [`include/genetics/interactions/SeedDispersal.hpp`](include/genetics/interactions/SeedDispersal.hpp:25)

Result of seed dispersal.

```cpp
namespace EcoSim::Genetics {

struct DispersalEvent {
    int originX = 0;                              // Parent plant X
    int originY = 0;                              // Parent plant Y
    int targetX = 0;                              // Seed landing X
    int targetY = 0;                              // Seed landing Y
    float seedViability = 1.0f;                   // Germination probability (0-1)
    DispersalStrategy method = DispersalStrategy::GRAVITY;
    std::string disperserInfo;                    // "wind", "creature_id:123", etc.
    
    float getDistance() const;                    // Euclidean distance
};

}
```

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `originX` | `int` | Parent plant X coordinate |
| `originY` | `int` | Parent plant Y coordinate |
| `targetX` | `int` | Landing X coordinate |
| `targetY` | `int` | Landing Y coordinate |
| `seedViability` | `float` | Germination probability (0.0-1.0) |
| `method` | `DispersalStrategy` | Dispersal method used |
| `disperserInfo` | `std::string` | Additional information (e.g., creature ID) |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getDistance()` | `float` | Euclidean distance from origin to target |

---

## CoevolutionTracker

**Header:** [`include/genetics/interactions/CoevolutionTracker.hpp`](include/genetics/interactions/CoevolutionTracker.hpp:95)

Tracks population-level gene frequencies to detect evolutionary arms races.

```cpp
namespace EcoSim::Genetics {

class CoevolutionTracker {
public:
    CoevolutionTracker();
    explicit CoevolutionTracker(const ArmsRaceConfig& config);
    
    // Population recording
    void recordCreatureGeneration(const std::vector<Creature>& creatures);
    void recordPlantGeneration(const std::vector<Plant>& plants);
    void advanceGeneration();
    
    // Arms race detection
    bool isArmsRaceActive(const std::string& creatureGene, const std::string& plantGene) const;
    float getCoevolutionStrength(const std::string& creatureGene, const std::string& plantGene) const;
    
    // Statistics retrieval
    CoevolutionStats getCoevolutionStats(
        const std::string& creatureGene, const std::string& plantGene) const;
    std::vector<CoevolutionStats> getActiveArmsRaces() const;
    std::vector<CoevolutionStats> getAllTrackedPairs() const;
    
    // Configuration
    void addTrackedPair(const std::string& creatureGene, const std::string& plantGene);
    void removeTrackedPair(const std::string& creatureGene, const std::string& plantGene);
    void setConfig(const ArmsRaceConfig& config);
    const ArmsRaceConfig& getConfig() const;
    
    // History
    int getCurrentGeneration() const;
    std::vector<std::pair<int, float>> getCreatureGeneHistory(const std::string& geneName) const;
    std::vector<std::pair<int, float>> getPlantGeneHistory(const std::string& geneName) const;
    void clearHistory();
    void reset();
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | `const ArmsRaceConfig&` | Configuration for tracking (optional) |

### Population Recording Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `recordCreatureGeneration(creatures)` | `void` | Record creature population gene frequencies |
| `recordPlantGeneration(plants)` | `void` | Record plant population gene frequencies |
| `advanceGeneration()` | `void` | Move to next generation |

### Arms Race Detection Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `isArmsRaceActive(creature, plant)` | `bool` | Is this pair in active arms race? |
| `getCoevolutionStrength(creature, plant)` | `float` | Strength of coevolutionary signal |

### Statistics Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getCoevolutionStats(creature, plant)` | `CoevolutionStats` | Full stats for gene pair |
| `getActiveArmsRaces()` | `std::vector<CoevolutionStats>` | All active arms races |
| `getAllTrackedPairs()` | `std::vector<CoevolutionStats>` | All tracked pairs |

### Configuration Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `addTrackedPair(creature, plant)` | `void` | Add gene pair to track |
| `removeTrackedPair(creature, plant)` | `void` | Stop tracking gene pair |
| `setConfig(config)` | `void` | Update configuration |
| `getConfig()` | `const ArmsRaceConfig&` | Get current configuration |

### History Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getCurrentGeneration()` | `int` | Current generation number |
| `getCreatureGeneHistory(name)` | `std::vector<...>` | (generation, mean value) pairs |
| `getPlantGeneHistory(name)` | `std::vector<...>` | (generation, mean value) pairs |
| `clearHistory()` | `void` | Clear historical data |
| `reset()` | `void` | Full reset including tracked pairs |

---

## CoevolutionStats

**Header:** [`include/genetics/interactions/CoevolutionTracker.hpp`](include/genetics/interactions/CoevolutionTracker.hpp:35)

Statistics for a tracked coevolutionary gene pair.

```cpp
namespace EcoSim::Genetics {

struct CoevolutionStats {
    std::string creatureGene;           // Name of creature gene being tracked
    std::string plantGene;              // Name of plant gene being tracked
    float correlationCoefficient;       // Pearson correlation (-1 to 1)
    int generationsTracked;             // Number of generations with data
    std::string trend;                  // "escalating", "stable", "declining", "oscillating"
    float creatureMeanValue;            // Mean creature gene value this generation
    float plantMeanValue;               // Mean plant gene value this generation
    float creatureVariance;             // Variance in creature gene
    float plantVariance;                // Variance in plant gene
};

}
```

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `creatureGene` | `std::string` | Creature gene ID |
| `plantGene` | `std::string` | Plant gene ID |
| `correlationCoefficient` | `float` | Pearson correlation (-1.0 to 1.0) |
| `generationsTracked` | `int` | Number of generations recorded |
| `trend` | `std::string` | Current trend descriptor |
| `creatureMeanValue` | `float` | Current generation mean (creature) |
| `plantMeanValue` | `float` | Current generation mean (plant) |
| `creatureVariance` | `float` | Gene variance (creature) |
| `plantVariance` | `float` | Gene variance (plant) |

### Trend Values

| Trend | Description |
|-------|-------------|
| `"escalating"` | Both genes increasing together (arms race) |
| `"stable"` | Values not changing significantly |
| `"declining"` | Both genes decreasing |
| `"oscillating"` | Values cycling up and down |

---

## ArmsRaceConfig

**Header:** [`include/genetics/interactions/CoevolutionTracker.hpp`](include/genetics/interactions/CoevolutionTracker.hpp:55)

Configuration for coevolution tracking.

```cpp
namespace EcoSim::Genetics {

struct ArmsRaceConfig {
    float correlationThreshold = 0.5f;             // Min correlation to consider linked
    float escalationThreshold = 0.1f;              // Min change per generation for escalation
    int minGenerationsForTrend = 5;                // Minimum generations to detect trend
    int maxHistoryGenerations = 100;               // How many generations to keep
    std::vector<std::pair<std::string, std::string>> trackedPairs;
};

}
```

### Fields

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `correlationThreshold` | `float` | 0.5 | Minimum correlation for linked genes |
| `escalationThreshold` | `float` | 0.1 | Minimum change per generation |
| `minGenerationsForTrend` | `int` | 5 | Generations needed for trend detection |
| `maxHistoryGenerations` | `int` | 100 | Maximum history to retain |
| `trackedPairs` | `std::vector<...>` | - | Gene pairs to track |

---

## Default Tracked Coevolutionary Pairs

The CoevolutionTracker tracks these gene pairs by default:

| Creature Gene | Plant Gene | Relationship |
|--------------|------------|--------------|
| `TOXIN_TOLERANCE` | `TOXIN_PRODUCTION` | Defense vs Tolerance |
| `TOXIN_METABOLISM` | `TOXIN_PRODUCTION` | Defense vs Metabolism |
| `HIDE_THICKNESS` | `THORN_DENSITY` | Physical defense vs protection |
| `SEED_DESTRUCTION_RATE` | `SEED_COAT_DURABILITY` | Seed predation vs survival |
| `COLOR_VISION` | `FRUIT_APPEAL` | Fruit detection vs attraction |

---

## Usage Example

```cpp
#include "genetics/interactions/FeedingInteraction.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/interactions/CoevolutionTracker.hpp"

using namespace EcoSim::Genetics;

// Feeding interaction
FeedingInteraction feeding;
Plant berry = createBerryBush();
Phenotype creaturePhenotype = getCreaturePhenotype();

FeedingResult result = feeding.attemptToEatPlant(creaturePhenotype, berry, 0.8f);
if (result.success) {
    creature.gainEnergy(result.nutritionGained);
    creature.takeDamage(result.damageReceived);
    berry.takeDamage(result.plantDamage);
}

// Seed dispersal
SeedDispersal dispersal;
DispersalEvent event = dispersal.disperseByWind(berry, 5.0f, 45.0f);
if (event.seedViability > 0.5f) {
    trySpawnPlant(event.targetX, event.targetY, berry.getGenome());
}

// Coevolution tracking
ArmsRaceConfig config;
config.correlationThreshold = 0.6f;
config.minGenerationsForTrend = 10;

CoevolutionTracker tracker(config);
tracker.addTrackedPair("TOXIN_TOLERANCE", "TOXIN_PRODUCTION");

// Each generation...
tracker.recordCreatureGeneration(allCreatures);
tracker.recordPlantGeneration(allPlants);
tracker.advanceGeneration();

// Check for arms races
if (tracker.isArmsRaceActive("TOXIN_TOLERANCE", "TOXIN_PRODUCTION")) {
    CoevolutionStats stats = tracker.getCoevolutionStats("TOXIN_TOLERANCE", "TOXIN_PRODUCTION");
    std::cout << "Arms race trend: " << stats.trend << std::endl;
    std::cout << "Correlation: " << stats.correlationCoefficient << std::endl;
}
```

---

## See Also

**Core Documentation:**
- [[../../core/01-architecture]] - System architecture overview
- [[../../core/03-extending]] - Extension guide

**Reference:**
- [[../quick-reference]] - Quick reference cheat sheet
- [[../genes]] - Complete gene catalog
- [[core-classes]] - Core genetics classes
- [[expression]] - Phenotype and state management
- [[organisms]] - Organism implementations
- [[interfaces]] - Interface definitions

**Design:**
- [[../../design/coevolution]] - Coevolution design rationale
- [[../../design/propagation]] - Seed dispersal design
