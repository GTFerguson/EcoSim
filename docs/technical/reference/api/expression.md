---
title: Expression API Reference
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: reference
tags: [genetics, api, reference, expression, phenotype]
---

# Expression API Reference

**Audience:** Developer
**Type:** Reference

---

## Overview

This document covers the gene expression and state management classes. These classes handle the translation of genetic information (genotype) into observable traits (phenotype), modulated by environmental and organism state:

- **Phenotype** - Bridges genotype to expressed traits
- **PhenotypeCache** - Performance optimization for trait computation
- **EnvironmentState** - Environmental conditions for expression
- **OrganismState** - Organism internal state for expression
- **EnergyBudget** - Universal resource allocation framework
- **EnergyState** - Current energy reserves and costs

All classes are in the `EcoSim::Genetics` namespace.

---

## Phenotype

**Header:** [`include/genetics/expression/Phenotype.hpp`](include/genetics/expression/Phenotype.hpp:44)

Bridges genotype to expressed traits with environmental modulation.

```cpp
namespace EcoSim::Genetics {

class Phenotype {
public:
    // Constructors
    Phenotype();
    explicit Phenotype(const Genome* genome);
    Phenotype(const Genome* genome, const GeneRegistry* registry);
    
    // Configuration
    void setGenome(const Genome* genome);
    void setRegistry(const GeneRegistry* registry);
    void updateContext(const EnvironmentState& env, const OrganismState& org);
    
    // Trait access
    float getTrait(const std::string& trait_id) const;
    float computeTrait(const std::string& trait_id) const;
    bool hasTrait(const std::string& trait_id) const;
    const std::unordered_map<std::string, float>& getAllTraits() const;
    
    // Cache management
    void invalidateCache();
    float getCacheHitRate() const;
    
    // Validation
    bool isValid() const;
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `genome` | `const Genome*` | Pointer to genome (not owned) |
| `registry` | `const GeneRegistry*` | Pointer to registry (not owned) |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `setGenome(genome)` | `void` | Set genome pointer |
| `setRegistry(registry)` | `void` | Set registry pointer |
| `updateContext(env, org)` | `void` | Update environmental context |
| `getTrait(trait_id)` | `float` | Get cached trait (or compute) |
| `computeTrait(trait_id)` | `float` | Force recomputation |
| `hasTrait(trait_id)` | `bool` | Check if trait can be computed |
| `getAllTraits()` | `const std::unordered_map<...>&` | All computed traits |
| `invalidateCache()` | `void` | Force cache clear |
| `getCacheHitRate()` | `float` | Cache hit ratio (0.0-1.0) |
| `isValid()` | `bool` | True if genome and registry set |

---

## PhenotypeCache

**Header:** [`include/genetics/expression/PhenotypeCache.hpp`](include/genetics/expression/PhenotypeCache.hpp:15)

Manages caching for phenotype trait computation.

```cpp
namespace EcoSim::Genetics {

class PhenotypeCache {
public:
    using ComputeFunc = std::function<float()>;
    
    PhenotypeCache() = default;
    
    float getOrCompute(const std::string& trait_id, ComputeFunc compute_func);
    void invalidate(const std::string& trait_id);
    void invalidateAll();
    void checkInvalidation(const EnvironmentState& env, const OrganismState& org);
    float getCacheHitRate() const;
};

}
```

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getOrCompute(id, func)` | `float` | Get cached or compute |
| `invalidate(trait_id)` | `void` | Invalidate single trait |
| `invalidateAll()` | `void` | Invalidate entire cache |
| `checkInvalidation(env, org)` | `void` | Auto-invalidate on state change |
| `getCacheHitRate()` | `float` | Hit ratio for diagnostics |

---

## EnvironmentState

**Header:** [`include/genetics/expression/EnvironmentState.hpp`](include/genetics/expression/EnvironmentState.hpp:7)

Holds environment data for phenotype expression.

```cpp
namespace EcoSim::Genetics {

struct EnvironmentState {
    float temperature = 20.0f;      // Celsius
    float humidity = 0.5f;          // 0.0 to 1.0
    float time_of_day = 0.5f;       // 0.0 (midnight) to 1.0 (midnight)
    int terrain_type = 0;           // Terrain enum value
    int season = 0;                 // Season enum value
    
    EnvironmentState() = default;
    bool significantlyDifferent(const EnvironmentState& other, float threshold = 0.1f) const;
};

}
```

### Fields

| Field | Type | Range | Description |
|-------|------|-------|-------------|
| `temperature` | `float` | Any | Temperature in Celsius |
| `humidity` | `float` | 0.0-1.0 | Relative humidity |
| `time_of_day` | `float` | 0.0-1.0 | Time (0.5 = noon) |
| `terrain_type` | `int` | Terrain enum | Current terrain |
| `season` | `int` | Season enum | Current season |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `significantlyDifferent(other, threshold)` | `bool` | Check if states differ enough to invalidate cache |

---

## OrganismState

**Header:** [`include/genetics/expression/OrganismState.hpp`](include/genetics/expression/OrganismState.hpp:7)

Holds organism state data for phenotype expression.

```cpp
namespace EcoSim::Genetics {

struct OrganismState {
    float age_normalized = 0.0f;    // 0.0 = birth, 1.0 = max lifespan
    float energy_level = 1.0f;      // 0.0 to 1.0
    float health = 1.0f;            // 0.0 to 1.0
    bool is_pregnant = false;
    bool is_sleeping = false;
    
    OrganismState() = default;
    bool significantlyDifferent(const OrganismState& other, float threshold = 0.1f) const;
};

}
```

### Fields

| Field | Type | Range | Description |
|-------|------|-------|-------------|
| `age_normalized` | `float` | 0.0-1.0 | Age as fraction of lifespan |
| `energy_level` | `float` | 0.0-1.0 | Current energy |
| `health` | `float` | 0.0-1.0 | Current health |
| `is_pregnant` | `bool` | - | Pregnancy state |
| `is_sleeping` | `bool` | - | Sleep state |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `significantlyDifferent(other, threshold)` | `bool` | Check if states differ enough to invalidate cache |

---

## EnergyBudget

**Header:** [`include/genetics/expression/EnergyBudget.hpp`](include/genetics/expression/EnergyBudget.hpp:65)

Implements the Universal Resource Allocation Framework for energy management.

```cpp
namespace EcoSim::Genetics {

class EnergyBudget {
public:
    EnergyBudget() = default;
    
    // Main calculations
    float calculateMaintenanceCost(const Genome& genome, const GeneRegistry& registry) const;
    float calculateSpecialistBonus(const Phenotype& phenotype) const;
    float calculateMetabolicOverhead(const Phenotype& phenotype) const;
    
    // Energy state management
    EnergyState updateEnergy(const EnergyState& current, float income, float activity) const;
    bool isStarving(const EnergyState& state) const;
    bool canReproduce(const EnergyState& state, float reproductionCost) const;
    
    // Configuration
    float getStarvationThreshold() const;
    void setStarvationThreshold(float threshold);
};

}
```

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `calculateMaintenanceCost(genome, registry)` | `float` | Sum of gene maintenance costs |
| `calculateSpecialistBonus(phenotype)` | `float` | 1.0-1.3 multiplier for dietary specialists |
| `calculateMetabolicOverhead(phenotype)` | `float` | Overhead for multi-system organisms |
| `updateEnergy(state, income, activity)` | `EnergyState` | Update energy for one tick |
| `isStarving(state)` | `bool` | True if energy <= 10% of max |
| `canReproduce(state, cost)` | `bool` | True if enough energy for reproduction |
| `getStarvationThreshold()` | `float` | Current starvation threshold |
| `setStarvationThreshold(threshold)` | `void` | Configure starvation threshold |

---

## EnergyState

**Header:** [`include/genetics/expression/EnergyBudget.hpp`](include/genetics/expression/EnergyBudget.hpp:45)

Current energy reserves and expenditure tracking.

```cpp
namespace EcoSim::Genetics {

struct EnergyState {
    float currentEnergy = 100.0f;    // Current energy reserves
    float maxEnergy = 100.0f;        // Maximum storage capacity
    float baseMetabolism = 1.0f;     // Base energy burn per tick
    float maintenanceCost = 0.0f;    // Gene maintenance per tick
    float activityCost = 0.0f;       // Movement, sensing, etc.
    float totalExpenditure = 0.0f;   // Sum of all costs
    
    float getEnergyRatio() const;    // 0.0 to 1.0
};

}
```

### Fields

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `currentEnergy` | `float` | 100.0 | Current energy reserves |
| `maxEnergy` | `float` | 100.0 | Maximum storage capacity |
| `baseMetabolism` | `float` | 1.0 | Base energy burn per tick |
| `maintenanceCost` | `float` | 0.0 | Gene maintenance per tick |
| `activityCost` | `float` | 0.0 | Movement, sensing, etc. |
| `totalExpenditure` | `float` | 0.0 | Sum of all costs |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getEnergyRatio()` | `float` | Ratio of current to max (0.0-1.0) |

---

## Usage Example

```cpp
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnergyBudget.hpp"

using namespace EcoSim::Genetics;

// Create phenotype from genome
Genome genome = createSomeGenome();
GeneRegistry registry = createRegistry();

Phenotype phenotype(&genome, &registry);

// Update context for environmental effects
EnvironmentState env;
env.temperature = 25.0f;
env.humidity = 0.7f;
env.season = 1; // Summer

OrganismState org;
org.age_normalized = 0.3f;
org.energy_level = 0.8f;

phenotype.updateContext(env, org);

// Access expressed traits
float speed = phenotype.getTrait("movement_speed");
float metabolism = phenotype.getTrait("metabolism_rate");

// Energy budget calculations
EnergyBudget budget;
float maintenance = budget.calculateMaintenanceCost(genome, registry);

EnergyState state;
state.currentEnergy = 80.0f;
state.maintenanceCost = maintenance;

// Update energy for this tick
float foodIncome = 10.0f;
float activityCost = 5.0f;
EnergyState newState = budget.updateEnergy(state, foodIncome, activityCost);

if (budget.isStarving(newState)) {
    // Handle starvation
}
```

---

## See Also

**Core Documentation:**
- [[../../core/01-architecture]] - System architecture overview
- [[../../core/02-getting-started]] - Quick start tutorial
- [[../../core/03-extending]] - Extension guide

**Reference:**
- [[../quick-reference]] - Quick reference cheat sheet
- [[../genes]] - Complete gene catalog
- [[core-classes]] - Core genetics classes
- [[organisms]] - Organism implementations
- [[interactions]] - System interactions
- [[interfaces]] - Interface definitions

**Design:**
- [[../../design/resource-allocation-design]] - Energy budget design rationale
