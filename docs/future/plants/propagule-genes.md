---
title: Unified Propagule Gene System
created: 2025-12-26
updated: 2025-12-26
status: proposed
priority: medium
tags: [design, plants, reproduction, genetics, future]
---

# Unified Propagule Gene System

## Overview

This document describes a future enhancement to the plant reproductive system: a **unified propagule gene set** that models the full spectrum of plant reproductive structures (from naked seeds to fleshy fruits) using a single cohesive genetic architecture.

> [!NOTE]
> **Prerequisite**: This system builds on the [[resource-system|Plant Resource Depletion System]]. Implement that first before considering this enhancement.

## Motivation

### The Biological Reality

In nature, a "fruit" is simply a seed distribution mechanism—the fleshy wrapper exists solely to attract animal dispersers who will carry seeds elsewhere. All plants produce seeds, but only *some* wrap them in fruit.

```
                    REPRODUCTIVE STRUCTURE
                           │
           ┌───────────────┴───────────────┐
           │                               │
      SEED (embryo)                   FRUIT (optional wrapper)
      - The actual offspring         - Attracts dispersers
      - Energy reserve               - Protects seed during transport
      - Protective coat              - Digestible or not
```

### Problem with Current Gene Separation

Currently, we have separate genes for fruit and seeds:

| Fruit-Related Genes | Seed-Related Genes |
|--------------------|--------------------|
| `fruit_appeal` | `seed_mass` |
| `fruit_production_rate` | `seed_coat_durability` |
| `fruiting_season_length` | `offspring_count` |
| | `seed_energy_reserve` |

This creates an artificial split that:
- Doesn't model that fruit CONTAINS seeds
- Makes it unclear how eating fruit affects seed dispersal
- Treats "fruit-bearing" and "non-fruit-bearing" plants as fundamentally different
- Misses the continuous spectrum from naked seeds → nuts → berries

---

## Proposed Solution: PropaguleGenes

A "propagule" is the complete dispersal unit—seed + optional wrapper. This unified system uses a single gene set that creates a natural spectrum.

### Gene Structure

```cpp
/**
 * @brief Unified genes for plant reproductive structures (propagules)
 * 
 * A "propagule" is the complete dispersal unit - seed + optional wrapper.
 * This unified system creates a natural spectrum:
 *   naked seeds → nuts → drupes → berries → fleshy fruits
 */
struct PropaguleGenes {
    // WRAPPER genes (the "fruit" component)
    float wrapper_investment;     ///< 0.0 = naked seed, 1.0 = thick fleshy fruit
    float wrapper_appeal;         ///< Attractiveness to dispersers (color, scent, sweetness)
    float wrapper_nutrition;      ///< Energy content of wrapper per unit
    float wrapper_protection;     ///< Physical protection for seed inside
    
    // CORE genes (the "seed" component - always present)
    float core_energy_reserve;    ///< Energy for germination (seed endosperm)
    float core_protection;        ///< Seed coat durability (survives digestion?)
    float core_count;             ///< Seeds per propagule (1 = drupe, many = berry)
    float core_mass;              ///< Mass of individual seed
    
    // TIMING genes
    float production_rate;        ///< How fast propagules are produced
    float season_length;          ///< Duration of production window
};
```

### The Wrapper Investment Spectrum

The key innovation is `wrapper_investment` [0.0 - 1.0], which creates a continuous spectrum:

```
wrapper_investment:  0.0          0.3          0.6          0.9
                      │            │            │            │
                      ▼            ▼            ▼            ▼
               ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
               │  NAKED   │  │   NUT    │  │  DRUPE   │  │  BERRY   │
               │   SEED   │  │          │  │          │  │          │
               │          │  │  thin    │  │  fleshy  │  │  very    │
               │  no wrap │  │  shell   │  │  + pit   │  │  fleshy  │
               └──────────┘  └──────────┘  └──────────┘  └──────────┘
               
               Grass        Oak          Cherry       Tomato
               Dandelion    Walnut       Plum         Grape
               Pine         Hazelnut     Peach        Blueberry
```

---

## Real-World Examples as Gene Values

| Plant Type | wrapper_investment | wrapper_appeal | core_protection | core_count | Strategy |
|------------|-------------------|----------------|-----------------|------------|----------|
| **Dandelion** | 0.0 | 0.0 | 0.1 | 1 | Wind dispersal, no wrapper |
| **Grass** | 0.0 | 0.0 | 0.2 | 1 | Simple seed, no protection |
| **Pine** | 0.05 | 0.1 | 0.3 | 1 | Winged seed in cone |
| **Acorn** | 0.1 | 0.2 | 0.9 | 1 | Cached by animals, tough coat |
| **Walnut** | 0.15 | 0.15 | 0.95 | 1 | Very hard shell |
| **Cherry** | 0.7 | 0.9 | 0.8 | 1 | Fleshy, attractive, tough pit |
| **Plum** | 0.75 | 0.85 | 0.75 | 1 | Classic drupe |
| **Tomato** | 0.8 | 0.7 | 0.3 | 50+ | Very fleshy, many soft seeds |
| **Grape** | 0.85 | 0.9 | 0.4 | 3-4 | Multiple seeds in fleshy berry |
| **Coconut** | 0.9 | 0.1 | 0.95 | 1 | Huge wrapper, water dispersal |
| **Strawberry** | 0.6 | 0.95 | 0.2 | 200+ | Seeds ON outside of wrapper |

---

## Feeding Mechanics Integration

### Consumption Result Structure

```cpp
/**
 * @brief Result of eating a propagule (fruit/seed)
 */
struct PropaguleConsumptionResult {
    float wrapper_nutrition;      ///< Calories from wrapper (if any)
    float seed_nutrition;         ///< Calories from seed (if consumed)
    int seeds_destroyed;          ///< Seeds that were digested
    int seeds_to_disperse;        ///< Seeds that survived for dispersal
};
```

### Consumption Logic

```cpp
PropaguleConsumptionResult consumePropagule(
    const Phenotype& creature,
    const PropaguleGenes& propagule,
    bool targetingWrapper)        // Did creature bite the wrapper or crack the seed?
{
    PropaguleConsumptionResult result;
    
    if (propagule.wrapper_investment > 0.1f && targetingWrapper) {
        // Creature is eating the fleshy wrapper
        result.wrapper_nutrition = propagule.wrapper_investment * 
                                   propagule.wrapper_nutrition * 
                                   creature.getTrait("plant_digestion_efficiency");
        
        // Seeds may survive passage through digestive system
        float digestive_strength = creature.getTrait("stomach_acidity");
        float seed_survival_chance = propagule.core_protection - digestive_strength;
        
        for (int i = 0; i < propagule.core_count; i++) {
            if (random() < seed_survival_chance) {
                result.seeds_to_disperse++;
            } else if (random() < 0.5f) {
                // Seed destroyed but provided some nutrition
                result.seeds_destroyed++;
                result.seed_nutrition += propagule.core_energy_reserve * 0.3f;
            }
        }
    } else {
        // Creature is targeting the seed directly (cracking nuts, eating naked seeds)
        float access_ability = 1.0f;
        
        // Hard seeds require jaw strength
        if (propagule.core_protection > 0.5f) {
            float jaw = creature.getTrait("jaw_strength");
            if (jaw < propagule.core_protection) {
                access_ability = jaw / propagule.core_protection;
            }
        }
        
        // All seeds targeted are destroyed (no dispersal)
        result.seeds_destroyed = propagule.core_count;
        result.seed_nutrition = propagule.core_count * 
                               propagule.core_energy_reserve * 
                               access_ability *
                               creature.getTrait("plant_digestion_efficiency");
    }
    
    return result;
}
```

### Key Behavioral Differences

| Action | Outcome |
|--------|---------|
| **Eat wrapper (fruit)** | Nutrition from flesh, seeds may survive digestion and disperse |
| **Eat seed directly** | High nutrition from seed, but seed destroyed (no dispersal) |
| **Cache seeds (nuts)** | Seeds may survive to germinate where cached |

---

## PlantTissue Simplification

With the PropaguleGenes system, we simplify the tissue types:

### Current (Complex)

```cpp
enum class PlantTissue {
    Leaf,
    Stem,
    Fruit,      // Separate type
    Seed,       // Separate type
    Root,
    Bark
};
```

### Proposed (Simplified)

```cpp
enum class PlantTissue {
    Leaf,       ///< Photosynthetic tissue
    Stem,       ///< Structural tissue (includes bark)
    Propagule,  ///< Reproductive structure (unified seed + optional wrapper)
    Root        ///< Underground storage/absorption
};

// The propagule's gene values determine its behavior, not the tissue type
```

---

## Migration Path from Current Genes

| Current Gene | PropaguleGenes Equivalent | Notes |
|--------------|---------------------------|-------|
| `fruit_appeal` | `wrapper_appeal` | Identical concept |
| `fruit_production_rate` | `production_rate` | Identical concept |
| `seed_mass` | `core_mass` | Identical concept |
| `seed_coat_durability` | `core_protection` | Identical concept |
| `offspring_count` | `core_count` | Identical concept |
| `seed_energy_reserve` | `core_energy_reserve` | New gene in base system |
| `fruiting_season_length` | `season_length` | New gene in base system |
| (none) | `wrapper_investment` | **Key new gene** |
| (none) | `wrapper_nutrition` | Could derive from `nutrient_value` |
| (none) | `wrapper_protection` | Physical seed protection |

---

## Evolutionary Implications

This unified system enables realistic evolutionary pressures:

### Dispersal Strategy Evolution

1. **Wind-dispersed plants** evolve:
   - Low `wrapper_investment` (no fruit weight)
   - Low `core_mass` (light seeds)
   - High `production_rate` (many seeds compensate for random dispersal)

2. **Animal-dispersed (frugivore) plants** evolve:
   - High `wrapper_investment` (attractive fruit)
   - High `wrapper_appeal` (colorful, sweet, fragrant)
   - Low `core_protection` (seeds survive digestion)

3. **Cache-dispersed (nut) plants** evolve:
   - Low `wrapper_investment` (hard shell, not fleshy)
   - High `core_protection` (survives squirrel teeth, winter)
   - High `core_energy_reserve` (worth caching)
   - Low `wrapper_appeal` (not sweet, not colorful)

4. **Water-dispersed plants** evolve:
   - High `wrapper_investment` (buoyant husk like coconut)
   - Low `wrapper_appeal` (not eaten, floats away)
   - High `wrapper_protection` (waterproof)

### Coevolution Examples

| Plant Strategy | Creature Coevolution |
|----------------|---------------------|
| High `wrapper_appeal`, low `core_protection` | Frugivores that eat fruit, disperse seeds |
| High `core_protection`, high `core_energy_reserve` | Seed-cachers (squirrels) that forget some caches |
| High `wrapper_investment`, medium `core_protection` | Competition: some eat fruit (disperse), some crack seeds (destroy) |

---

## Implementation Considerations

### New Creature Gene: `stomach_acidity`

>[!Dev note]
>I think these gene already exists.

To properly model seed survival through digestion, we need:

```cpp
// New gene for UniversalGenes
Gene("stomach_acidity", 
     GeneCategory::Heterotrophy,
     0.0f, 1.0f,   // Range
     "Digestive acid strength - high destroys seeds, low allows passage");
```

**Trade-off**: High stomach acidity = better meat digestion, but destroys seeds (can't be seed disperser)

### PlantResources Integration

```cpp
struct PlantResources {
    // Existing tissue resources...
    float leaf_resources;
    float stem_resources;
    float root_resources;
    
    // Replaces separate fruit_resources and seed_resources
    float propagule_resources;    ///< Combined reproductive tissue
    float max_propagule_capacity; ///< Scales with reproduction genes
    float propagule_regen_rate;   ///< Based on season and phase
    
    // Propagule properties from genes
    PropaguleGenes propagule_type; ///< What kind of propagule this plant makes
};
```

---

## Advantages Over Current System

1. **Biological Accuracy**: Models the real spectrum of plant reproductive strategies
2. **Emergent Behavior**: Different dispersal strategies evolve naturally based on ecosystem pressures
3. **Cleaner Architecture**: Single unified system instead of parallel fruit/seed systems
4. **Richer Coevolution**: More nuanced creature-plant relationships emerge
5. **Future Extensibility**: Easy to add new dispersal mechanisms (ant dispersal, explosive dehiscence)

---

## Prerequisites

Before implementing PropaguleGenes:

1. ✅ [[resource-system|Plant Resource Depletion System]] - Basic resource tracking
2. ⬜ Growth Phase System - Propagule production tied to Mature/Senescent phases
3. ⬜ Seasonal System - Production timing
4. ⬜ Seed Dispersal System - Where dispersed seeds land and germinate

---

## See Also

- [[resource-system]] - Foundation system (implement first)
- [[../technical/design/coevolution]] - Creature-plant coevolution
- [[../technical/design/propagation]] - Seed dispersal strategies
- [[../technical/genetics/plant-propagation-strategies]] - Current implementation notes

---

**Document Purpose:** Future enhancement proposal for a unified reproductive gene system that models the full spectrum from naked seeds to fleshy fruits.

**Status:** Proposed for implementation after Plant Resource Depletion System is complete.
