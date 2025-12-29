---
title: Expression API Reference
created: 2025-12-24
updated: 2025-12-29
status: complete
audience: developer
type: reference
tags: [genetics, api, reference, expression, phenotype, modulation]
---

# Expression API Reference

**Audience:** Developer
**Type:** Reference

---

## Overview

This document covers the gene expression and state management classes. These classes handle the translation of genetic information (genotype) into observable traits (phenotype), modulated by environmental and organism state:

- **Phenotype** - Bridges genotype to expressed traits
- **PhenotypeCache** - Performance optimization for trait computation
- **PhenotypeUtils** - Utility functions for effect calculations
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
    float computeTraitRaw(const std::string& trait_id) const;  // No modulations
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
| `getTrait(trait_id)` | `float` | Get cached trait (or compute) with policy-based modulation |
| `computeTrait(trait_id)` | `float` | Force recomputation with modulations |
| `computeTraitRaw(trait_id)` | `float` | Get raw genetic value (no age/env/state modulation) |
| `hasTrait(trait_id)` | `bool` | Check if trait can be computed |
| `getAllTraits()` | `const std::unordered_map<...>&` | All computed traits |
| `invalidateCache()` | `void` | Force cache clear |
| `getCacheHitRate()` | `float` | Cache hit ratio (0.0-1.0) |
| `isValid()` | `bool` | True if genome and registry set |

---

## TraitModulationPolicy

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:44)

Enum controlling how organism state affects trait expression. Each gene definition stores a modulation policy.

```cpp
namespace EcoSim::Genetics {

enum class TraitModulationPolicy {
    NEVER,           // Immutable physical structure (e.g., hide_thickness, teeth)
    HEALTH_ONLY,     // Metabolic efficiency traits (reduced when injured)
    ENERGY_GATED,    // Production traits (capacity vs actual output)
    CONSUMER_APPLIED // Performance traits (modulate at use-time, not phenotype)
};

}
```

### Policy Behaviors

| Policy | Phenotype Returns | Organism State Effect |
|--------|-------------------|----------------------|
| `NEVER` | Raw genetic value | No modulation ever applied |
| `HEALTH_ONLY` | Health-modulated value | Reduced 70-100% when health < 50% |
| `ENERGY_GATED` | Raw capacity | Consumer checks energy before production |
| `CONSUMER_APPLIED` | Raw genetic value | Consumer applies context-specific modulation |

### Policy Distribution

| Policy | Gene Count | Examples |
|--------|:----------:|----------|
| `NEVER` | 72 | hide_thickness, teeth_sharpness, max_size |
| `HEALTH_ONLY` | 8 | digestive_efficiency, photosynthesis |
| `ENERGY_GATED` | 3 | toxin_production, regeneration_rate |
| `CONSUMER_APPLIED` | 6 | locomotion, sight_range, scent_detection |

### GeneDefinition Policy Access

```cpp
// In GeneDefinition class
TraitModulationPolicy getModulationPolicy() const;
void setModulationPolicy(TraitModulationPolicy policy);
```

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

## PhenotypeUtils

**Header:** [`include/genetics/expression/PhenotypeUtils.hpp`](include/genetics/expression/PhenotypeUtils.hpp:16)

Utility functions for phenotype effect calculations. Extracted to reduce code duplication.

```cpp
namespace EcoSim::Genetics::PhenotypeUtils {

struct AccumulatedEffect {
    float value = 0.0f;
    bool found_contribution = false;
};

AccumulatedEffect applyEffect(const AccumulatedEffect& accumulated,
                               EffectType effect_type,
                               float gene_value,
                               float scale_factor);

}
```

### AccumulatedEffect Structure

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `value` | `float` | 0.0f | Current accumulated value |
| `found_contribution` | `bool` | false | Whether any gene contributed |

### Functions

| Function | Returns | Description |
|----------|---------|-------------|
| `applyEffect(accumulated, type, value, scale)` | `AccumulatedEffect` | Apply single effect to accumulator |

### Effect Type Behaviors

| EffectType | Behavior |
|------------|----------|
| `Direct` | Gene value becomes trait value (overwrites) |
| `Additive` | Adds `gene_value * scale_factor` to sum |
| `Multiplicative` | Multiplies existing value (starts at 1.0 if first) |
| `Threshold` | Only contributes if `gene_value >= scale_factor` |
| `Conditional` | Context-dependent (simplified as additive) |

### Design Notes

- Extracted from `Phenotype::computeTrait()` and `Phenotype::computeTraitRaw()`
- Eliminates duplicate switch statements for effect processing
- Used internally by Phenotype - not typically called directly

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
org.health = 0.4f;  // Injured creature

phenotype.updateContext(env, org);

// Access expressed traits (policy-based modulation)
float metabolism = phenotype.getTrait("metabolism_rate");  // HEALTH_ONLY: reduced
float hideThickness = phenotype.getTrait("hide_thickness"); // NEVER: unchanged
float locomotion = phenotype.getTrait("locomotion");        // CONSUMER_APPLIED: raw value

// Get raw genetic value (no modulation at all)
float rawMetabolism = phenotype.computeTraitRaw("metabolism_rate");

// Consumer-applied modulation example (in MovementBehavior)
float baseSpeed = phenotype.getTrait("locomotion");  // Returns raw (CONSUMER_APPLIED)
float effectiveSpeed = baseSpeed * org.energy_level * org.health;  // Consumer applies

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
- [[../../design/resource-allocation]] - Energy budget design rationale
