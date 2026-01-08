---
title: Genetic Systems Improvements
created: 2025-12-24
updated: 2025-12-24
status: planned
tags: [genetics, future, improvements]
---

# Genetic Systems Improvements

Future improvements specifically for the genetics system, gene expression, and related mechanics.

---

## High Priority

Critical genetics and balance issues.

### Diet System

- [ ] **Scavenger diet type misdefined** - Scavengers should eat carrion, not "eat anything"
  - Rename diet type or clarify behavior
  - Implement corpse system with `toxin_tolerance` gene
  - Requires: Corpse/decomposition system from [[environment/ecosystem-improvements]]

### Defense Genes

- [ ] **Prey defensive adaptations not implemented** - `toxin`, `hide`, `spikes` genes exist but are unused
  - Extend [`FeedingInteraction`](include/genetics/interactions/FeedingInteraction.hpp) for creature-creature defense calculations
  - `toxin` → Damage to predator on consumption
  - `hide` → Reduced detection probability
  - `spikes` → Damage to predator on attack

---

## Medium Priority

Gene expression and configuration improvements.

### Configuration

- [ ] **JSON-based species template loading** - Currently hardcoded in PlantFactory
  - Load species definitions from JSON files
  - Enable modding without recompilation
  - Foundation for [[genetics/godk|GODk]] system

### Performance

- [ ] **Batch energy budget calculations** - Currently calculated individually per organism
  - Group organisms by similar phenotype profiles
  - Amortize repeated calculations
  - Consider caching common expression results in [`PhenotypeCache`](include/genetics/expression/PhenotypeCache.hpp)

---

## Low Priority

Analysis and advanced features.

### Visualization & Analysis

- [ ] **Multi-generational coevolution visualization** - [`CoevolutionTracker`](include/genetics/interactions/CoevolutionTracker.hpp) exists but no UI
  - Gene frequency graphs over time
  - Predator-prey adaptation correlation
  - Arms race visualization

- [ ] **Speciation detection** - No automated detection of population divergence
  - Track genetic distance between populations
  - Alert when populations become reproductively isolated
  - Visualize phylogenetic trees

- [ ] **Ecological niche analysis** - No tools to understand gene-environment fit
  - Map gene expression to environmental conditions
  - Identify gene clusters associated with biomes
  - Fitness landscape visualization

---

## Known Limitations

Current limitations informing improvement priorities.

### DIET_TYPE Deprecation Warning

The legacy `DIET_TYPE` gene still exists for backward compatibility but generates deprecation warnings. Migration to emergent diet system recommended:
- Instead of explicit diet type, use combination of:
  - `plant_digestion_efficiency`
  - `meat_digestion_efficiency` 
  - `fruit_preference`
  - `scavenger_tolerance`

### Coevolution Tracker Memory

[`CoevolutionTracker`](include/genetics/interactions/CoevolutionTracker.hpp) stores gene frequency history in memory. For very long simulations:
- May need periodic pruning
- Consider disk serialization for history
- Add configurable history depth

### Species Template Flexibility

Current templates use hardcoded gene ranges in [`PlantFactory`](src/genetics/organisms/PlantFactory.cpp). Future work:
- JSON configuration files
- Runtime template creation
- Template inheritance

### Test Threshold Sensitivity

Some plant tests use lenient thresholds due to phenotype expression variability:
- Expression depends on environmental state
- Random factors in gene expression
- May need refinement as system matures

---

## Implementation Notes

### Gene Implementation Files

New genes and modifications:
- [`DefaultGenes.hpp`](include/genetics/defaults/DefaultGenes.hpp) / [`.cpp`](src/genetics/defaults/DefaultGenes.cpp)
- [`PlantGenes.hpp`](include/genetics/defaults/PlantGenes.hpp) / [`.cpp`](src/genetics/defaults/PlantGenes.cpp)
- [`UniversalGenes.hpp`](include/genetics/defaults/UniversalGenes.hpp) / [`.cpp`](src/genetics/defaults/UniversalGenes.cpp)

### Interaction System

Defense gene implementation:
- [`FeedingInteraction.hpp`](include/genetics/interactions/FeedingInteraction.hpp) / [`.cpp`](src/genetics/interactions/FeedingInteraction.cpp)

### Expression System

Energy and phenotype optimization:
- [`EnergyBudget.hpp`](include/genetics/expression/EnergyBudget.hpp) / [`.cpp`](src/genetics/expression/EnergyBudget.cpp)
- [`PhenotypeCache.hpp`](include/genetics/expression/PhenotypeCache.hpp) / [`.cpp`](src/genetics/expression/PhenotypeCache.cpp)

---

## See Also

- [[blocking-issues]] - Critical blocking issues requiring immediate attention
- [[environment/ecosystem-improvements]] - Creature behavior and world mechanics
- [[ui/improvements]] - UI and rendering improvements
- [[genetics/unified-organism-genome]] - Unified genome architecture
- [[behavior/sensory-phases]] - Sensory system development plans
- [[genetics/godk]] - Scripted genetics system (future modding API)
