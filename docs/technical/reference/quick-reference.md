---
title: Genetics Quick Reference
created: 2025-12-24
updated: 2025-12-24
status: complete
tags: [genetics, reference, cheat-sheet]
---

# Genetics Quick Reference Cheat Sheet

> [!TIP]
> This is a compact reference for developers familiar with the genetics system.
> For detailed documentation, see [[genes]] and [[api/core-classes]].

---

## Core Types

| Type | Header | Description |
|------|--------|-------------|
| `GeneValue` | `GeneticTypes.hpp` | `std::variant<float, int, bool, std::string>` |
| `Allele` | `GeneticTypes.hpp` | `{GeneValue value, float expression_strength}` |
| `GeneLimits` | `GeneticTypes.hpp` | `{float min, float max, float creep}` |
| `Gene` | `Gene.hpp` | Two alleles (diploid) + ID |
| `GeneDefinition` | `Gene.hpp` | Blueprint: limits, dominance, chromosome |
| `Chromosome` | `Chromosome.hpp` | Collection of linked genes |
| `Genome` | `Genome.hpp` | All 8 chromosomes |
| `Phenotype` | `Phenotype.hpp` | Expressed traits from genome |
| `EnergyBudget` | `EnergyBudget.hpp` | Resource allocation calculator |

---

## Chromosome Types

| Enum Value | ID | Purpose | Example Genes |
|------------|:--:|---------|---------------|
| `Morphology` | 0 | Size, appearance | `max_size`, `color_hue`, `fur_density` |
| `Sensory` | 1 | Perception | `sight_range`, `color_vision` |
| `Metabolism` | 2 | Energy, digestion | `metabolism_rate`, `photosynthesis` |
| `Locomotion` | 3 | Movement | `locomotion` |
| `Behavior` | 4 | Actions | `flee_threshold`, `hunt_instinct` |
| `Reproduction` | 5 | Breeding | `offspring_count`, `seed_mass` |
| `Environmental` | 6 | Habitat | `temp_tolerance_low/high` |
| `Lifespan` | 7 | Longevity | `lifespan` |

**Constant:** `NUM_CHROMOSOMES = 8`

---

## Dominance Types

| Type | Formula | Example |
|------|---------|---------|
| `Complete` | Stronger allele only | Blood type A vs O |
| `Incomplete` | `(a1 + a2) / 2` | Most continuous traits |
| `Codominant` | Both expressed | `color_hue` (blended) |
| `Overdominant` | Heterozygote > homozygote | `hardiness` (hybrid vigor) |

**Incomplete dominance expression:**
```
expressed = maternal * (1 - d) + paternal * d
where d = 0.5 for incomplete
```

---

## Gene Categories (88 active, 89 total)

| Category | Count | Expression |
|----------|:-----:|------------|
| Universal | 7 | All organisms |
| Mobility | 5 | Creatures high, Plants dormant |
| Autotrophy | 5 | Plants high, Creatures dormant |
| Heterotrophy (original) | 6 | Creatures |
| Heterotrophy (coevolution) | 7 | Creatures + carniv. plants |
| Morphology | 9 | Creatures |
| Olfactory | 4 | All |
| Behavior | 5 | Creatures |
| Health/Healing | 3 | Creatures |
| Combat Weapons | 13 | Creatures |
| Combat Defense | 2 | Creatures |
| Combat Behavior | 4 | Creatures |
| Seed Interaction | 2 | Creatures |
| Plant Defense | 6 | Plants |
| Reproduction | 6 | All |
| Seed Propagation | 5 | Plants |

---

## Common Code Snippets

### Get Trait Value
```cpp
float speed = phenotype.getTrait("locomotion");
float size = phenotype.getTrait("max_size");
```

### Check Gene Exists
```cpp
if (genome.hasGene("photosynthesis")) { ... }
if (registry.hasGene("locomotion")) { ... }
```

### Create Gene with Limits
```cpp
GeneDefinition def("my_gene", 
    ChromosomeType::Metabolism,
    GeneLimits(0.0f, 1.0f, 0.05f),  // min, max, creep
    DominanceType::Incomplete);
registry.registerGene(def);
```

### Access Gene from Genome
```cpp
const Gene& gene = genome.getGene("locomotion");
float value = gene.getNumericValue(DominanceType::Incomplete);
// Or with safe access:
auto optGene = genome.tryGetGene("locomotion");
if (optGene) { float v = optGene->get().getNumericValue(...); }
```

### Create Organism Genomes
```cpp
auto creatureGenome = UniversalGenes::createCreatureGenome(registry);
auto plantGenome = UniversalGenes::createPlantGenome(registry);
auto randomGenome = UniversalGenes::createRandomGenome(registry);
```

### Crossover + Mutation
```cpp
Genome offspring = Genome::crossover(parent1, parent2, 0.5f);
offspring.mutate(0.01f, registry.getAllDefinitions());
```

### Update Phenotype Context
```cpp
EnvironmentState env;
env.temperature = 25.0f;
env.humidity = 0.6f;

OrganismState org;
org.age_normalized = creature.getAgeNormalized();
org.energy_level = creature.getEnergy() / creature.getMaxEnergy();

phenotype.updateContext(env, org);
```

---

## Key Formulas

### Dominance Expression
```cpp
// Incomplete dominance (most common)
expressed = (allele1.value + allele2.value) / 2.0f;

// With expression strength
expressed = a1.value * a1.strength + a2.value * a2.strength;
```

### Mutation Range
```cpp
mutated = value + random(-creep, +creep);
mutated = clamp(mutated, limits.min, limits.max);
```

### Energy Cost (daily maintenance)
```
cost = base_cost * expression_level * scaling_factor
```

High-cost genes: `locomotion` (1.5), `sight_range` (1.2), `toxicity` (1.5)

### Specialist Bonus
```cpp
// Pure herbivore: plant > 0.7 && meat < 0.3
// Pure carnivore: meat > 0.7 && plant < 0.3
bonus = 1.0f to 1.3f for specialists
```

---

## Emergent Types

### Diet Type Calculation
```cpp
float plant = getTrait("plant_digestion_efficiency");
float meat = getTrait("meat_digestion_efficiency");
float cell = getTrait("cellulose_breakdown");
float sweet = getTrait("sweetness_preference");

if (plant > 0.6 && sweet > 0.7 && cell < 0.3) return FRUGIVORE;
if (plant > 0.7 && cell > 0.5)                return HERBIVORE;
if (meat > 0.7 && plant < 0.3)                return CARNIVORE;
if (plant > 0.4 && meat > 0.4)                return OMNIVORE;
return SCAVENGER;
```

### Dispersal Strategy
```cpp
float mass = getTrait("seed_mass");
float aero = getTrait("seed_aerodynamics");
float hook = getTrait("seed_hook_strength");
float appeal = getTrait("fruit_appeal");
float coat = getTrait("seed_coat_durability");
float force = getTrait("explosive_pod_force");
float runner = getTrait("runner_production");

if (runner > 0.7)                    return VEGETATIVE;
if (force > 5.0)                     return EXPLOSIVE;
if (hook > 0.6)                      return ANIMAL_BURR;
if (appeal > 0.6 && coat > 0.5)      return ANIMAL_FRUIT;
if (mass < 10.0 && aero > 0.6)       return WIND;
return GRAVITY;
```

### Organism Archetype
```cpp
float loco = getTrait("locomotion");
float photo = getTrait("photosynthesis");
float hunt = getTrait("hunt_instinct");

if (loco > 0.5 && photo < 0.3) return CREATURE;
if (loco < 0.3 && photo > 0.5) return PLANT;
if (loco > 0.5 && photo > 0.5) return MOBILE_PLANT;
if (loco < 0.3 && hunt > 0.5)  return SESSILE_CREATURE; // carniv plant
return HYBRID;
```

---

## Gene ID Quick Reference

### Universal (all organisms)
`lifespan`, `max_size`, `metabolism_rate`, `color_hue`, `hardiness`, `temp_tolerance_low`, `temp_tolerance_high`

### Mobility (creatures)
`locomotion`, `sight_range`, `navigation_ability`, `flee_threshold`, `pursue_threshold`

### Autotrophy (plants)
`photosynthesis`, `root_depth`, `water_storage`, `light_requirement`, `water_requirement`

### Heterotrophy (creatures)
`hunt_instinct`, `hunger_threshold`, `digestive_efficiency`, `plant_digestion_efficiency`, `meat_digestion_efficiency`, `cellulose_breakdown`, `toxin_tolerance`, `toxin_metabolism`, `scent_detection`, `color_vision`

### Morphology
`gut_length`, `tooth_sharpness`, `tooth_grinding`, `stomach_acidity`, `jaw_strength`, `jaw_speed`, `hide_thickness`, `fur_density`, `mucus_protection`

### Plant Defense
`toxin_production`, `thorn_density`, `regrowth_rate`, `fruit_production_rate`, `seed_coat_durability`, `fruit_appeal`

### Seed Propagation
`seed_mass`, `seed_aerodynamics`, `seed_hook_strength`, `explosive_pod_force`, `runner_production`

---

## Troubleshooting Checklist

| Problem | Check |
|---------|-------|
| "Gene not found" | Is gene registered in `GeneRegistry`? |
| | Did you call `UniversalGenes::registerDefaults()`? |
| Trait returns 0 | Is gene on correct chromosome? |
| | Is `expression_strength` > 0? |
| | Is Phenotype linked to valid Genome? |
| Mutations not happening | Is `mutation_rate` > 0? |
| | Are `GeneLimits` passed to `mutate()`? |
| Phenotype stale | Call `invalidateCache()` after genome change |
| | Call `updateContext()` after env change |
| Crossover always same | Check `recombination_rate` (try 0.5) |
| Energy always depleting | Check `calculateMaintenanceCost()` return |
| | High-cost genes active? |

---

## File Locations

| Component | Header | Source |
|-----------|--------|--------|
| Gene types | `include/genetics/core/GeneticTypes.hpp` | `src/genetics/core/GeneticTypes.cpp` |
| Gene/GeneDefinition | `include/genetics/core/Gene.hpp` | `src/genetics/core/Gene.cpp` |
| Genome | `include/genetics/core/Genome.hpp` | `src/genetics/core/Genome.cpp` |
| Registry | `include/genetics/core/GeneRegistry.hpp` | `src/genetics/core/GeneRegistry.cpp` |
| Phenotype | `include/genetics/expression/Phenotype.hpp` | `src/genetics/expression/Phenotype.cpp` |
| UniversalGenes | `include/genetics/defaults/UniversalGenes.hpp` | `src/genetics/defaults/UniversalGenes.cpp` |
| Plant | `include/genetics/organisms/Plant.hpp` | `src/genetics/organisms/Plant.cpp` |

---

## See Also

**Core Documentation:**
- [[../core/01-architecture]] - System architecture overview
- [[../core/02-getting-started]] - Quick start tutorial
- [[../core/03-extending]] - Extension guide

**Reference:**
- [[genes]] - Complete gene catalog with all 88 genes
- [[api/core-classes]] - Core API documentation
- [[api/expression]] - Expression API
- [[api/organisms]] - Organism API

**Design:**
- [[../design/resource-allocation]] - Energy budget details
- [[../design/coevolution]] - Coevolution mechanics
