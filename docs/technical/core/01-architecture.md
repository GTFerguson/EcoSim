---
title: Genetics System Architecture
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: guide
tags: [genetics, architecture, SOLID, core]
---

# Genetics System Architecture

## Overview

The genetics system implements a biologically-inspired genome model following **SOLID principles**. It provides diploid inheritance (two alleles per gene), multiple dominance patterns, phenotype expression with environmental modulation, and extensible gene definitions.

The architecture separates concerns cleanly:
- **Core**: Data structures for genes, chromosomes, and genomes
- **Expression**: Computing observable traits from genetic data
- **Interfaces**: Segregated interfaces for organism capabilities
- **Defaults**: Pre-built gene definitions for different organism types

## Directory Structure

```
include/genetics/
├── core/                   # Core data structures
│   ├── Gene.hpp            # Gene and GeneDefinition classes
│   ├── Chromosome.hpp      # Chromosome container with O(1) lookup
│   ├── Genome.hpp          # Complete genetic makeup (8 chromosomes)
│   ├── GeneRegistry.hpp    # Gene definition storage (DI, not singleton)
│   └── GeneticTypes.hpp    # Enums, type aliases, helper structs
│
├── expression/             # Phenotype computation
│   ├── Phenotype.hpp       # Genotype → expressed traits bridge
│   ├── PhenotypeCache.hpp  # Performance caching (SRP)
│   ├── EnvironmentState.hpp # Environment data only (SRP)
│   └── OrganismState.hpp   # Organism state data only (SRP)
│
├── interfaces/             # Segregated interfaces (ISP)
│   ├── IPositionable.hpp   # Position in world
│   ├── ILifecycle.hpp      # Age and lifespan
│   ├── IGeneticOrganism.hpp # Genome and phenotype access
│   └── IReproducible.hpp   # Reproduction capability
│
├── defaults/               # Gene definitions
│   ├── UniversalGenes.hpp  # Unified 88-gene system (all organisms)
│   └── PlantGenes.hpp      # Plant-specific gene definitions
│
└── organisms/              # Organism implementations
    └── Plant.hpp           # Reference plant implementation
```

## Core Components

### Gene and Allele

The [`Gene`](../../../include/genetics/core/Gene.hpp:18) class represents a diploid gene with two alleles:

```cpp
class Gene {
    std::string id_;
    Allele allele1_;
    Allele allele2_;
};
```

**Key features:**
- **Diploid representation**: Two [`Allele`](../../../include/genetics/core/GeneticTypes.hpp:44) instances per gene
- **GeneValue variant**: Supports `float`, `int`, `bool`, `std::string` via `std::variant`
- **Expression strength**: Each allele has a 0.0-1.0 strength for dormant/active states
- **Dominance-based expression**: [`getExpressedValue()`](../../../include/genetics/core/Gene.hpp:64) computes final value

The [`GeneDefinition`](../../../include/genetics/core/Gene.hpp:107) class serves as a blueprint:

```cpp
class GeneDefinition {
    std::string id_;
    ChromosomeType chromosome_;
    GeneLimits limits_;
    DominanceType dominance_;
    std::vector<EffectBinding> effects_;  // Pleiotropy support
};
```

### Chromosome

The [`Chromosome`](../../../include/genetics/core/Chromosome.hpp:20) class groups linked genes:

- **8 chromosome types** defined in [`ChromosomeType`](../../../include/genetics/core/GeneticTypes.hpp:12):
  - Morphology, Sensory, Metabolism, Locomotion
  - Behavior, Reproduction, Environmental, Lifespan

- **Linked inheritance**: Genes on the same chromosome are inherited together
- **Recombination**: [`crossover()`](../../../include/genetics/core/Chromosome.hpp:91) allows gene mixing between homologous chromosomes
- **O(1) gene lookup** via index map:

```cpp
std::vector<Gene> genes_;
std::unordered_map<std::string, size_t> gene_index_;  // O(1) lookup
```

### Genome

The [`Genome`](../../../include/genetics/core/Genome.hpp:22) class holds all 8 chromosomes:

```cpp
class Genome {
    std::array<Chromosome, NUM_CHROMOSOMES> chromosomes_;
    mutable std::unordered_map<std::string, std::pair<ChromosomeType, size_t>> gene_cache_;
    mutable bool cache_valid_ = false;
};
```

**Key features:**
- **Array of 8 Chromosome objects**: Fixed-size, efficient access
- **Cross-chromosome gene lookup with caching**: Lazy cache rebuild on first lookup
- **Crossover operator**: [`Genome::crossover()`](../../../include/genetics/core/Genome.hpp:44) combines parent genomes
- **Mutation operator**: [`mutate()`](../../../include/genetics/core/Genome.hpp:48) applies random changes
- **Similarity comparison**: [`compare()`](../../../include/genetics/core/Genome.hpp:52) returns 0.0-1.0 similarity

### GeneRegistry

The [`GeneRegistry`](../../../include/genetics/core/GeneRegistry.hpp:25) stores gene definitions:

```cpp
class GeneRegistry {
    std::unordered_map<std::string, GeneDefinition> definitions_;
};
```

**Key design decisions:**
- **Dependency injection**: Not a singleton—passed to functions that need it
- **Copy disabled**: Prevents accidental registry duplication
- **Move enabled**: Allows ownership transfer
- **Thread-safe reads**: Definitions are immutable once registered

### Phenotype

The [`Phenotype`](../../../include/genetics/expression/Phenotype.hpp:44) class bridges genotype to expressed traits:

```cpp
class Phenotype {
    const Genome* genome_ = nullptr;
    const GeneRegistry* registry_ = nullptr;
    mutable PhenotypeCache cache_;
    EnvironmentState environment_;
    OrganismState organism_state_;
};
```

**Key features:**
- **Genome → traits bridge**: Converts genetic data to observable characteristics
- **Age modulation curves**: Traits vary across lifespan
- **Environment modulation**: External factors affect expression
- **Caching system**: [`PhenotypeCache`](../../../include/genetics/expression/PhenotypeCache.hpp:15) for performance

## SOLID Principles Applied

### SRP (Single Responsibility Principle)

The original `ExpressionContext` was split into focused classes:

| Class | Responsibility |
|-------|---------------|
| [`EnvironmentState`](../../../include/genetics/expression/EnvironmentState.hpp:7) | Environment data (temperature, humidity, time) |
| [`OrganismState`](../../../include/genetics/expression/OrganismState.hpp:7) | Organism state (age, energy, health) |
| [`PhenotypeCache`](../../../include/genetics/expression/PhenotypeCache.hpp:15) | Caching logic and invalidation |
| [`Phenotype`](../../../include/genetics/expression/Phenotype.hpp:44) | Expression computation only |

### ISP (Interface Segregation Principle)

Organisms implement only the interfaces they need:

| Interface | Methods | Purpose |
|-----------|---------|---------|
| [`IPositionable`](../../../include/genetics/interfaces/IPositionable.hpp:7) | `getX()`, `getY()`, `setPosition()` | World position |
| [`ILifecycle`](../../../include/genetics/interfaces/ILifecycle.hpp:7) | `getAge()`, `isAlive()`, `age()` | Age and death |
| [`IGeneticOrganism`](../../../include/genetics/interfaces/IGeneticOrganism.hpp:11) | `getGenome()`, `getPhenotype()` | Genetic access |
| [`IReproducible<T>`](../../../include/genetics/interfaces/IReproducible.hpp:8) | `canReproduce()`, `reproduce()` | Breeding |

Example: A [`Plant`](../../../include/genetics/organisms/Plant.hpp:42) implements `IPositionable`, `ILifecycle`, and `IGeneticOrganism` but not complex movement interfaces.

### DIP (Dependency Inversion Principle)

The `GeneRegistry` is injected rather than accessed via singleton:

```cpp
// ✅ Good: Dependency injection
Phenotype phenotype(&genome, &registry);

// ❌ Bad: Singleton pattern (not used)
Phenotype phenotype(&genome, &GeneRegistry::getInstance());
```

Benefits:
- Easier testing with mock registries
- Clear ownership and lifetime
- No hidden global state

## Data Flow

```
┌─────────────────┐
│  GeneRegistry   │ ← Gene definitions (limits, dominance, effects)
└────────┬────────┘
         │ lookup
         ▼
┌─────────────────┐
│     Genome      │ ← Gene values (allele pairs)
│  ┌───────────┐  │
│  │Chromosome1│  │
│  │Chromosome2│  │
│  │    ...    │  │
│  │Chromosome8│  │
│  └───────────┘  │
└────────┬────────┘
         │ expression
         ▼
┌─────────────────┐
│    Phenotype    │ ← Context-aware modulation
│ + Environment   │
│ + OrganismState │
│ + Cache         │
└────────┬────────┘
         │ traits
         ▼
┌─────────────────┐
│    Organism     │ ← Behavior driven by traits
│    Behavior     │
└─────────────────┘
```

## Expression System

### Dominance Types

Defined in [`DominanceType`](../../../include/genetics/core/GeneticTypes.hpp:27):

| Type | Formula | Example |
|------|---------|---------|
| **Complete** | `allele1.strength > allele2.strength ? allele1 : allele2` | Eye color |
| **Incomplete** | `(allele1 + allele2) / 2` | Flower color blending |
| **Codominant** | Both expressed simultaneously | Blood type AB |
| **Overdominant** | `max(allele1, allele2) * 1.1` | Heterozygote advantage |

### Age Modulation

Implemented in [`applyAgeModulation()`](../../../include/genetics/expression/Phenotype.hpp:164):

```
Expression Strength
    │
1.0 │         ┌─────────────────┐
    │        ╱                   ╲
0.8 │       ╱                     ╲────
    │      ╱
0.6 │─────╱
    │
0.0 └────┬───┬─────────────────┬───┬────
         0  0.1               0.8  1.0   Age (normalized)
           Juvenile   Adult    Elderly
```

- **Juvenile (0-10%)**: Expression ramps 60% → 100%
- **Adult (10-80%)**: Full 100% expression
- **Elderly (80-100%)**: Linear decline to 80%

### Environment Modulation

Implemented in [`applyEnvironmentModulation()`](../../../include/genetics/expression/Phenotype.hpp:173):

| Factor | Affects | Effect |
|--------|---------|--------|
| **Temperature** | Metabolism traits | Cold slows, heat speeds |
| **Humidity** | Locomotion traits | Affects movement efficiency |
| **Light (time_of_day)** | Photosynthesis traits | Peak at noon |
| **Terrain** | Various | Terrain-specific bonuses |
| **Season** | Growth, reproduction | Seasonal activity patterns |

## Gene Categories

The [`UniversalGenes`](../../../include/genetics/defaults/UniversalGenes.hpp:36) system provides **88 genes** across **13 categories**:

| Category | Count | Example Genes | Primary Users |
|----------|-------|---------------|---------------|
| **Universal** | 7 | `lifespan`, `max_size`, `metabolism_rate`, `color_hue`, `hardiness` | All organisms |
| **Mobility** | 5 | `locomotion`, `sight_range`, `navigation_ability`, `flee_threshold` | Creatures (dormant in plants) |
| **Heterotrophy** | 6 | `hunt_instinct`, `plant_digestion_efficiency`, `meat_digestion_efficiency` | Creatures |
| **Morphology** | 9 | `gut_length`, `tooth_sharpness`, `tooth_grinding`, `stomach_acidity`, `jaw_strength`, `hide_thickness` | Creatures (diet adaptation) |
| **Olfactory** | 7 | `scent_production`, `olfactory_acuity`, `scent_masking`, `toxin_tolerance`, `fruit_preference` | All organisms |
| **Reproduction** | 7 | `offspring_count`, `spread_distance`, `mate_threshold`, `gestation_period` | All organisms |
| **Behavior** | 8 | `sweetness_preference`, `caching_instinct`, `grooming_frequency`, `exploration_drive` | Creatures |
| **Health** | 4 | `healing_rate`, `immune_strength`, `disease_resistance`, `wound_clotting` | All organisms |
| **Combat Weapons** | 6 | `claw_size`, `bite_force`, `horn_size`, `constriction_strength` | Creatures |
| **Combat Defense** | 6 | `shell_thickness`, `scale_density`, `speed_burst`, `camouflage` | Creatures |
| **Combat Behavior** | 5 | `aggression`, `territorial_instinct`, `pack_tactics`, `threat_display` | Creatures |
| **Seed Interaction** | 2 | `seed_caching_preference`, `scatter_tendency` | Creatures |
| **Plant-only** | 16 | `photosynthesis`, `root_depth`, `toxin_production`, `thorn_density` | Plants |

**Olfactory System Genes:**
- `scent_production` - Rate of general scent emission
- `scent_signature_variance` - Genetic signature variation
- `olfactory_acuity` - Ability to distinguish scent types
- `scent_masking` - Reduce own scent detectability

**Expression Strength Convention:**
- `1.0` = Fully active gene
- `0.5` = Partially active
- `0.1` = Dormant but evolvable
- `0.0` = Completely suppressed

For complete gene reference, see [[../reference/genes]] or [`UniversalGenes.hpp`](../../../include/genetics/defaults/UniversalGenes.hpp).

## Performance Considerations

### Gene Lookup: O(1)

Both `Chromosome` and `Genome` maintain hash maps for constant-time gene access:

```cpp
// Chromosome level
std::unordered_map<std::string, size_t> gene_index_;

// Genome level (cross-chromosome)
mutable std::unordered_map<std::string, std::pair<ChromosomeType, size_t>> gene_cache_;
```

### Phenotype Caching

The [`PhenotypeCache`](../../../include/genetics/expression/PhenotypeCache.hpp:15) avoids redundant computation:

```cpp
float getOrCompute(const std::string& trait_id, ComputeFunc compute_func);
```

- **Lazy computation**: Only computes when requested
- **Automatic invalidation**: On significant state changes
- **Hit rate tracking**: [`getCacheHitRate()`](../../../include/genetics/expression/PhenotypeCache.hpp:34) for diagnostics

### Memory Layout

- **Chromosomes**: `std::array<Chromosome, 8>` for cache-friendly iteration
- **Genes**: `std::vector<Gene>` per chromosome, contiguous memory
- **Traits**: `std::unordered_map<std::string, float>` for flexible access

## Thread Safety

Current guarantees:
- **GeneRegistry**: Thread-safe reads after all genes registered
- **Genome**: Not thread-safe (organisms own their genomes)
- **Phenotype**: Not thread-safe (tied to single organism)

For multi-threaded simulations, each organism should own its complete genetic state.

## Extension Points

1. **New genes**: Add to [`UniversalGenes`](../../../include/genetics/defaults/UniversalGenes.hpp) or create new registry
2. **New organisms**: Implement the segregated interfaces
3. **New dominance**: Extend [`DominanceType`](../../../include/genetics/core/GeneticTypes.hpp:27) enum
4. **New effects**: Add [`EffectType`](../../../include/genetics/core/GeneticTypes.hpp:35) values
5. **Custom modulation**: Override phenotype computation methods

## See Also

**Core Documentation:**
- [[02-getting-started]] - Quick start tutorial
- [[03-extending]] - How to extend the genetics system

**Reference:**
- [[../reference/quick-reference]] - Quick reference cheat sheet
- [[../reference/genes]] - Complete gene catalog
- [[../reference/api/core-classes]] - Core API reference
- [[../reference/api/expression]] - Expression API reference

**Systems:**
- [[../systems/scent-system]] - Olfactory communication system

**Design:**
- [[../design/coevolution]] - Creature-plant coevolution design
- [[../design/propagation]] - Seed dispersal strategies
- [[../design/resource-allocation]] - Energy budget design
