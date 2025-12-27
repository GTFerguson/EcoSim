---
title: Core Classes API Reference
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: reference
tags: [genetics, api, reference, core]
---

# Core Classes API Reference

**Audience:** Developer
**Type:** Reference

---

## Overview

This document covers the foundational classes of the EcoSim genetics system. These classes handle the representation and manipulation of genetic information at the most basic level:

- **Gene** - Individual gene with two alleles (diploid)
- **GeneDefinition** - Blueprint defining gene constraints and effects
- **Chromosome** - Collection of linked genes
- **Genome** - Complete genetic makeup (8 chromosomes)
- **GeneRegistry** - Storage for gene definitions

All classes are in the `EcoSim::Genetics` namespace.

---

## Gene

**Header:** [`include/genetics/core/Gene.hpp`](include/genetics/core/Gene.hpp:18)

Represents a single gene with two alleles (diploid organism).

```cpp
namespace EcoSim::Genetics {

class Gene {
public:
    // Constructors
    Gene(const std::string& gene_id, const Allele& allele1, const Allele& allele2);
    Gene(const std::string& gene_id, const GeneValue& value);  // Homozygous convenience
    
    // Accessors
    const std::string& getId() const;
    const Allele& getAllele1() const;
    const Allele& getAllele2() const;
    bool isHeterozygous() const;
    
    // Expression
    GeneValue getExpressedValue(DominanceType dominance) const;
    float getNumericValue(DominanceType dominance) const;
    
    // Genetic operations
    void mutate(float mutation_rate, const GeneLimits& limits);
    static Gene crossover(const Gene& parent1, const Gene& parent2);
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `gene_id` | `const std::string&` | Unique identifier for this gene |
| `allele1` | `const Allele&` | First allele |
| `allele2` | `const Allele&` | Second allele |
| `value` | `const GeneValue&` | Value for both alleles (homozygous) |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getId()` | `const std::string&` | Gene identifier |
| `getAllele1()` | `const Allele&` | First allele reference |
| `getAllele2()` | `const Allele&` | Second allele reference |
| `isHeterozygous()` | `bool` | True if alleles differ |
| `getExpressedValue(dominance)` | `GeneValue` | Computed value based on dominance |
| `getNumericValue(dominance)` | `float` | Numeric expression value |
| `mutate(rate, limits)` | `void` | Apply random mutation |
| `crossover(p1, p2)` | `Gene` | Create offspring gene (static) |

---

## GeneDefinition

**Header:** [`include/genetics/core/Gene.hpp`](include/genetics/core/Gene.hpp:107)

Blueprint for gene types, defining constraints and effects.

```cpp
namespace EcoSim::Genetics {

class GeneDefinition {
public:
    GeneDefinition(const std::string& id, 
                   ChromosomeType chromosome,
                   const GeneLimits& limits,
                   DominanceType dominance = DominanceType::Incomplete);
    
    // Accessors
    const std::string& getId() const;
    ChromosomeType getChromosome() const;
    const GeneLimits& getLimits() const;
    DominanceType getDominance() const;
    const std::vector<EffectBinding>& getEffects() const;
    
    // Modifiers
    void addEffect(const EffectBinding& effect);
    
    // Value operations
    GeneValue createRandomValue() const;
    float clampValue(float value) const;
    float applyCreep(float value) const;
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `const std::string&` | Unique gene type identifier |
| `chromosome` | `ChromosomeType` | Which chromosome this gene belongs to |
| `limits` | `const GeneLimits&` | Value constraints (min, max, creep) |
| `dominance` | `DominanceType` | Dominance pattern (default: Incomplete) |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getId()` | `const std::string&` | Gene definition ID |
| `getChromosome()` | `ChromosomeType` | Target chromosome type |
| `getLimits()` | `const GeneLimits&` | Value constraints |
| `getDominance()` | `DominanceType` | Dominance pattern |
| `getEffects()` | `const std::vector<EffectBinding>&` | Pleiotropy effect bindings |
| `addEffect(effect)` | `void` | Add a pleiotropy effect |
| `createRandomValue()` | `GeneValue` | Random value within limits |
| `clampValue(value)` | `float` | Constrain value to limits |
| `applyCreep(value)` | `float` | Add small random variation |

---

## Chromosome

**Header:** [`include/genetics/core/Chromosome.hpp`](include/genetics/core/Chromosome.hpp:20)

Collection of linked genes that are inherited together.

```cpp
namespace EcoSim::Genetics {

class Chromosome {
public:
    explicit Chromosome(ChromosomeType type);
    
    // Accessors
    ChromosomeType getType() const;
    size_t size() const;
    const std::vector<Gene>& getGenes() const;
    
    // Gene operations
    void addGene(const Gene& gene);
    bool hasGene(const std::string& gene_id) const;
    std::optional<std::reference_wrapper<const Gene>> getGene(const std::string& gene_id) const;
    std::optional<std::reference_wrapper<Gene>> getGeneMutable(const std::string& gene_id);
    
    // Genetic operations
    void mutate(float mutation_rate, 
                const std::unordered_map<std::string, GeneDefinition>& definitions);
    static Chromosome crossover(const Chromosome& parent1, 
                                const Chromosome& parent2,
                                float recombination_rate);
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | `ChromosomeType` | The chromosome type (e.g., Morphology, Metabolism) |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getType()` | `ChromosomeType` | Chromosome type enum |
| `size()` | `size_t` | Number of genes |
| `getGenes()` | `const std::vector<Gene>&` | All genes for iteration |
| `addGene(gene)` | `void` | Add gene (throws if duplicate ID) |
| `hasGene(gene_id)` | `bool` | Check if gene exists |
| `getGene(gene_id)` | `std::optional<...>` | Get gene by ID (O(1) lookup) |
| `getGeneMutable(gene_id)` | `std::optional<...>` | Get mutable gene reference |
| `mutate(rate, defs)` | `void` | Mutate all genes |
| `crossover(p1, p2, rate)` | `Chromosome` | Create offspring chromosome (static) |

---

## Genome

**Header:** [`include/genetics/core/Genome.hpp`](include/genetics/core/Genome.hpp:22)

Complete genetic makeup containing all 8 chromosomes.

```cpp
namespace EcoSim::Genetics {

class Genome {
public:
    Genome();
    
    // Chromosome access
    const Chromosome& getChromosome(ChromosomeType type) const;
    Chromosome& getChromosomeMutable(ChromosomeType type);
    
    // Gene access (cross-chromosome)
    bool hasGene(const std::string& gene_id) const;
    const Gene& getGene(const std::string& gene_id) const;
    Gene& getGeneMutable(const std::string& gene_id);
    std::optional<std::reference_wrapper<const Gene>> tryGetGene(const std::string& gene_id) const;
    
    // Gene operations
    void addGene(const Gene& gene, ChromosomeType chromosome);
    std::vector<std::reference_wrapper<const Gene>> getAllGenes() const;
    size_t getTotalGeneCount() const;
    
    // Genetic operations
    static Genome crossover(const Genome& parent1, const Genome& parent2,
                            float recombination_rate = 0.5f);
    void mutate(float mutation_rate,
                const std::unordered_map<std::string, GeneDefinition>& definitions);
    float compare(const Genome& other) const;
    
    // Iteration
    using iterator = std::array<Chromosome, NUM_CHROMOSOMES>::iterator;
    using const_iterator = std::array<Chromosome, NUM_CHROMOSOMES>::const_iterator;
    
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
};

}
```

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getChromosome(type)` | `const Chromosome&` | Get chromosome by type |
| `getChromosomeMutable(type)` | `Chromosome&` | Get mutable chromosome |
| `hasGene(gene_id)` | `bool` | Check if gene exists anywhere |
| `getGene(gene_id)` | `const Gene&` | Get gene (throws if not found) |
| `getGeneMutable(gene_id)` | `Gene&` | Get mutable gene reference |
| `tryGetGene(gene_id)` | `std::optional<...>` | Get gene (no throw) |
| `addGene(gene, chromosome)` | `void` | Add gene to specified chromosome |
| `getAllGenes()` | `std::vector<...>` | Flattened view of all genes |
| `getTotalGeneCount()` | `size_t` | Total genes across all chromosomes |
| `crossover(p1, p2, rate)` | `Genome` | Create offspring genome (static) |
| `mutate(rate, defs)` | `void` | Mutate all chromosomes |
| `compare(other)` | `float` | Similarity (0.0 = different, 1.0 = identical) |

---

## GeneRegistry

**Header:** [`include/genetics/core/GeneRegistry.hpp`](include/genetics/core/GeneRegistry.hpp:25)

Storage for gene definitions, passed via dependency injection.

```cpp
namespace EcoSim::Genetics {

class GeneRegistry {
public:
    GeneRegistry() = default;
    
    // Copy disabled, move enabled
    GeneRegistry(const GeneRegistry&) = delete;
    GeneRegistry& operator=(const GeneRegistry&) = delete;
    GeneRegistry(GeneRegistry&&) = default;
    GeneRegistry& operator=(GeneRegistry&&) = default;
    
    // Registration
    void registerGene(const GeneDefinition& definition);
    void registerGene(GeneDefinition&& definition);
    
    // Lookup
    bool hasGene(const std::string& gene_id) const;
    const GeneDefinition& getDefinition(const std::string& gene_id) const;
    std::optional<std::reference_wrapper<const GeneDefinition>> 
        tryGetDefinition(const std::string& gene_id) const;
    
    // Iteration
    const std::unordered_map<std::string, GeneDefinition>& getAllDefinitions() const;
    std::vector<std::reference_wrapper<const GeneDefinition>> 
        getDefinitionsForChromosome(ChromosomeType type) const;
    std::vector<std::string> getAllGeneIds() const;
    
    // Utility
    void clear();
    size_t size() const;
    bool empty() const;
};

}
```

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `registerGene(def)` | `void` | Register definition (throws if duplicate) |
| `hasGene(gene_id)` | `bool` | Check if definition exists |
| `getDefinition(gene_id)` | `const GeneDefinition&` | Get definition (throws if not found) |
| `tryGetDefinition(gene_id)` | `std::optional<...>` | Get definition (no throw) |
| `getAllDefinitions()` | `const std::unordered_map<...>&` | All registered definitions |
| `getDefinitionsForChromosome(type)` | `std::vector<...>` | Definitions for chromosome |
| `getAllGeneIds()` | `std::vector<std::string>` | All gene IDs |
| `clear()` | `void` | Remove all definitions |
| `size()` | `size_t` | Number of definitions |
| `empty()` | `bool` | True if no definitions |

---

## Enumerations

### ChromosomeType

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:12)

```cpp
enum class ChromosomeType {
    Morphology,      // Size, color, appearance
    Sensory,         // Sight, hearing, smell
    Metabolism,      // Diet, hunger rates, energy
    Locomotion,      // Speed, movement patterns
    Behavior,        // Aggression, sociality, flee/pursue
    Reproduction,    // Mating thresholds, offspring
    Environmental,   // Temperature tolerance, habitat
    Lifespan         // Longevity, aging
};
```

### DominanceType

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:27)

```cpp
enum class DominanceType {
    Complete,       // One allele fully masks the other
    Incomplete,     // Blended expression (average)
    Codominant,     // Both alleles expressed
    Overdominant    // Heterozygote advantage (hybrid vigor)
};
```

### EffectType

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:35)

```cpp
enum class EffectType {
    Direct,         // Gene value directly becomes trait value
    Additive,       // Contributes to sum
    Multiplicative, // Multiplier effect
    Threshold,      // Only active above/below threshold
    Conditional     // Context-dependent
};
```

### GeneCategory

**Header:** [`include/genetics/defaults/UniversalGenes.hpp`](include/genetics/defaults/UniversalGenes.hpp:15)

```cpp
enum class GeneCategory {
    Universal,      // All organisms (lifespan, size, metabolism)
    Mobility,       // Movement-related (locomotion, navigation, sight)
    Autotrophy,     // Self-feeding (photosynthesis, roots, water storage)
    Heterotrophy,   // Other-feeding (hunting, digestion, diet)
    Reproduction    // Breeding (offspring, spreading, mate selection)
};
```

---

## Type Aliases

### GeneValue

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:24)

```cpp
using GeneValue = std::variant<float, int, bool, std::string>;
```

Flexible gene value type supporting:
- `float` - Continuous values (most common)
- `int` - Discrete values (enum-like)
- `bool` - Binary traits
- `std::string` - Named variants

---

## Structs

### Allele

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:44)

```cpp
struct Allele {
    GeneValue value;
    float expression_strength = 1.0f;  // 0.0 to 1.0
    
    Allele() = default;
    explicit Allele(GeneValue val, float strength = 1.0f);
};
```

| Field | Type | Description |
|-------|------|-------------|
| `value` | `GeneValue` | The allele's genetic value |
| `expression_strength` | `float` | Expression level (0.0 = dormant, 1.0 = active) |

### GeneLimits

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:54)

```cpp
struct GeneLimits {
    float min_value;
    float max_value;
    float creep_amount;
    
    GeneLimits(float min_val, float max_val, float creep);
};
```

| Field | Type | Description |
|-------|------|-------------|
| `min_value` | `float` | Minimum allowed value |
| `max_value` | `float` | Maximum allowed value |
| `creep_amount` | `float` | Random variation during inheritance |

### EffectBinding

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:64)

```cpp
struct EffectBinding {
    std::string target_domain;   // e.g., "locomotion", "metabolism"
    std::string target_trait;    // e.g., "movement_speed", "energy_consumption"
    EffectType effect_type;
    float scale_factor;
    
    EffectBinding(const std::string& domain, const std::string& trait, 
                  EffectType type, float scale);
};
```

| Field | Type | Description |
|-------|------|-------------|
| `target_domain` | `std::string` | Category of affected trait |
| `target_trait` | `std::string` | Specific trait affected |
| `effect_type` | `EffectType` | How gene affects trait |
| `scale_factor` | `float` | Magnitude of effect |

---

## Constants

### NUM_CHROMOSOMES

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:77)

```cpp
constexpr int NUM_CHROMOSOMES = 8;
```

---

## Utility Functions

### chromosomeTypeToString

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:80)

```cpp
const char* chromosomeTypeToString(ChromosomeType type);
```

Converts chromosome type enum to string representation.

### stringToChromosomeType

**Header:** [`include/genetics/core/GeneticTypes.hpp`](include/genetics/core/GeneticTypes.hpp:83)

```cpp
std::optional<ChromosomeType> stringToChromosomeType(const std::string& str);
```

Parses string to chromosome type, returns `nullopt` if invalid.

---

## See Also

**Core Documentation:**
- [[../../core/01-architecture]] - System architecture overview
- [[../../core/02-getting-started]] - Quick start tutorial
- [[../../core/03-extending]] - Extension guide

**Reference:**
- [[../quick-reference]] - Quick reference cheat sheet
- [[../genes]] - Complete gene catalog
- [[expression]] - Phenotype and state management
- [[organisms]] - Organism implementations
- [[interactions]] - System interactions
- [[interfaces]] - Interface definitions
