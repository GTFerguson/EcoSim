---
title: Getting Started with the Genetics System
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: tutorial
tags: [genetics, tutorial, getting-started]
---

# Getting Started with the Genetics System

**Time Required:** ~15-20 minutes  
**Prerequisites:** Basic C++ knowledge, familiarity with EcoSim project structure

---

## Overview

This guide walks you through the core concepts of EcoSim's genetics system and demonstrates practical usage with working code examples. By the end, you'll be able to:

- Understand the genetics data model (genes, chromosomes, genomes)
- Create organisms with genetic traits
- Access expressed trait values through the Phenotype system
- Perform reproduction (crossover) and mutation

---

## Key Concepts

### Genes and Alleles (Diploid System)

Every organism in EcoSim is **diploid**, meaning each gene has **two alleles** — one inherited from each parent. The [`Gene`](include/genetics/core/Gene.hpp:18) class represents a single gene:

```cpp
// Heterozygous gene (different alleles)
Allele a1(1.0f, 1.0f);  // value, expression strength
Allele a2(2.0f, 0.8f);
Gene gene("speed", a1, a2);

// Homozygous gene (same value for both alleles)
Gene simple_gene("size", GeneValue(5.0f));
```

When the gene is expressed, the **dominance type** determines how alleles combine: complete dominance uses the dominant allele, incomplete dominance averages them, and overdominance boosts heterozygous expression.

### Chromosomes (8 Types)

Genes are organized into **8 chromosomes**, each grouping related traits:

| Chromosome | Purpose | Example Genes |
|------------|---------|---------------|
| Lifespan | Longevity | `lifespan`, `hardiness` |
| Metabolism | Energy processing | `metabolism_rate`, `digestive_efficiency` |
| Sensory | Perception | `sight_range`, `scent_detection` |
| Movement | Locomotion | `locomotion`, `navigation_ability` |
| Behavior | Instincts | `hunt_instinct`, `flee_threshold` |
| Morphology | Physical traits | `max_size`, `color_hue` |
| Reproduction | Breeding | `offspring_count`, `spread_distance` |
| Special | Unique traits | Organism-specific genes |

### Genome (Complete Genetic Material)

The [`Genome`](include/genetics/core/Genome.hpp:22) class contains all 8 chromosomes and provides gene lookup across them:

```cpp
Genome genome;
genome.addGene(lifespan_gene, ChromosomeType::Lifespan);
genome.addGene(speed_gene, ChromosomeType::Movement);

// Access any gene by ID
const Gene& gene = genome.getGene("lifespan");
```

### Phenotype (Expressed Traits)

The [`Phenotype`](include/genetics/expression/Phenotype.hpp:46) bridges genotype to actual trait values. It handles dominance expression, age modulation, and environmental effects:

```cpp
Phenotype phenotype(&genome, &registry);
float speed = phenotype.getTrait("locomotion");  // Get expressed value
```

### GeneRegistry (Gene Definitions)

The [`GeneRegistry`](include/genetics/core/GeneRegistry.hpp:25) stores blueprints ([`GeneDefinition`](include/genetics/core/Gene.hpp:122)) for all gene types, defining their limits, dominance patterns, and which chromosome they belong to:

```cpp
GeneRegistry registry;
UniversalGenes::registerDefaults(registry);  // Registers 67+ genes
```

---

## Your First Gene

Let's define a custom gene and use it in an organism.

### Step 1: Define the Gene

Gene definitions specify the gene's constraints and behavior. Look at how existing genes are defined in [`UniversalGenes.hpp`](include/genetics/defaults/UniversalGenes.hpp):

```cpp
#include "genetics/core/Gene.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneticTypes.hpp"

using namespace EcoSim::Genetics;

// Define a custom "aggression" gene
GeneDefinition aggression_def(
    "aggression",                    // Unique gene ID
    ChromosomeType::Behavior,        // Which chromosome
    GeneLimits{0.0f, 1.0f, 0.05f},  // min, max, creep (mutation step)
    DominanceType::Incomplete        // How alleles combine
);
```

### Step 2: Register in GeneRegistry

```cpp
GeneRegistry registry;

// Register default genes first (recommended)
UniversalGenes::registerDefaults(registry);

// Add your custom gene
registry.registerGene(aggression_def);
```

> [!TIP]
> Use [`tryRegisterGene()`](include/genetics/core/GeneRegistry.hpp:61) instead of `registerGene()` if duplicate registration should be handled gracefully (returns false instead of throwing).

### Step 3: Create an Organism with the Gene

```cpp
// Create a genome with your custom gene
Genome genome = UniversalGenes::createCreatureGenome(registry);

// The gene now exists in the genome
if (genome.hasGene("aggression")) {
    const Gene& gene = genome.getGene("aggression");
    // Gene has random values within limits
}
```

### Step 4: Access via Phenotype

```cpp
Phenotype phenotype(&genome, &registry);

// Get the expressed trait value
float aggression_level = phenotype.getTrait("aggression");
// Returns value between 0.0 and 1.0 based on alleles and dominance
```

---

## Creating an Organism

### Using PlantFactory

The [`PlantFactory`](include/genetics/organisms/PlantFactory.hpp:98) provides template-based plant creation:

```cpp
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

using namespace EcoSim::Genetics;

// Setup
auto registry = std::make_shared<GeneRegistry>();
UniversalGenes::registerDefaults(*registry);

PlantFactory factory(registry);
factory.registerDefaultTemplates();  // Berry Bush, Oak Tree, Grass, Thorn Bush

// Create a berry bush at position (10, 20)
Plant bush = factory.createFromTemplate("berry_bush", 10, 20);

// Access its phenotype
Phenotype& phenotype = bush.getPhenotype();
float growth = phenotype.getTrait("max_size");
float fruit = phenotype.getTrait("fruit_production_rate");
```

### Basic Lifecycle Operations

```cpp
// Get position
int x = bush.getX();
int y = bush.getY();

// Check if alive
if (bush.isAlive()) {
    // Update the plant (call each tick)
    bush.update();
}

// Get current age
int age = bush.getAge();
```

### Sexual Reproduction

```cpp
// Create offspring from two parents
Plant parent1 = factory.createFromTemplate("berry_bush", 0, 0);
Plant parent2 = factory.createFromTemplate("berry_bush", 5, 5);

// Offspring inherits genes from both parents with mutations
Plant offspring = factory.createOffspring(parent1, parent2, 10, 10);
```

---

## Common Patterns

### Quick Reference

| Operation | Code |
|-----------|------|
| Get trait value | `phenotype.getTrait("TRAIT_NAME")` |
| Check if trait exists | `phenotype.hasTrait("TRAIT_NAME")` |
| Get all traits | `phenotype.getAllTraits()` |
| Force recompute | `phenotype.computeTrait("TRAIT_NAME")` |
| Invalidate cache | `phenotype.invalidateCache()` |

### Getting a Trait Value

```cpp
// Simple lookup (uses cache)
float speed = phenotype.getTrait(UniversalGenes::LOCOMOTION);

// Check existence first
if (phenotype.hasTrait("custom_trait")) {
    float value = phenotype.getTrait("custom_trait");
}
```

### Creating a Gene with Defaults

```cpp
// Use gene definition to create a random value
const GeneDefinition& def = registry.getDefinition("lifespan");
GeneValue random_value = def.createRandomValue();

// Create gene with that value (homozygous)
Gene gene("lifespan", random_value);
```

### Sexual Reproduction (Crossover)

```cpp
// Gene-level crossover
Gene offspring_gene = Gene::crossover(parent1_gene, parent2_gene);

// Genome-level crossover
Genome offspring = Genome::crossover(parent1_genome, parent2_genome);

// With custom recombination rate (default 0.5)
Genome offspring = Genome::crossover(parent1, parent2, 0.7f);
```

### Mutation

```cpp
// Get gene definitions for mutation constraints
const auto& definitions = registry.getAllDefinitions();

// Apply mutation (rate = probability per gene)
genome.mutate(0.01f, definitions);  // 1% mutation rate

// Gene-level mutation
GeneLimits limits{0.0f, 100.0f, 5.0f};
gene.mutate(0.05f, limits);
```

### Emergent Diet Type

```cpp
// Diet is calculated from digestion genes, not stored directly
DietType diet = phenotype.calculateDietType();

// Returns: HERBIVORE, CARNIVORE, FRUGIVORE, or OMNIVORE
const char* diet_str = Phenotype::dietTypeToString(diet);
```

---

## Next Steps

Now that you understand the basics, explore these topics for deeper knowledge:

- **[[01-architecture]]** — System design and component relationships
- **[[03-extending]]** — Guide to adding new genes and organism types
- **[[../reference/api/core-classes]]** — Complete API documentation

---

## Prerequisites

This guide assumes familiarity with:
- Basic C++17 (templates, smart pointers, std::variant)
- EcoSim project structure (see `BUILDING.md`)

---

## See Also

**Core Documentation:**
- [[01-architecture]] - System architecture overview
- [[03-extending]] - Extension guide

**Reference:**
- [[../reference/quick-reference]] - Quick reference cheat sheet
- [[../reference/genes]] - Complete gene catalog
- [[../reference/api/core-classes]] - Core API reference
- [[../reference/api/expression]] - Expression API
- [[../reference/api/organisms]] - Organism API

**Source Files:**
- [`Gene.hpp`](../../../include/genetics/core/Gene.hpp) — Gene and GeneDefinition classes
- [`Genome.hpp`](../../../include/genetics/core/Genome.hpp) — Complete genome implementation
- [`Phenotype.hpp`](../../../include/genetics/expression/Phenotype.hpp) — Trait expression system
- [`UniversalGenes.hpp`](../../../include/genetics/defaults/UniversalGenes.hpp) — All gene constants
