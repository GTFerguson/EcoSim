---
title: Ecosystem & Creature Behavior Improvements
created: 2025-12-24
updated: 2026-04-16
status: partially-shipped
tags: [ecosystem, creatures, behavior, world, future]
---

# Ecosystem & Creature Behavior Improvements

Improvements to creature behavior, world mechanics, and ecosystem dynamics.

---

## Already shipped (audit 2026-04-14)

The following items from earlier versions of this doc have been shipped via the unified-organism migration and related work. They are retained here as an audit trail but are no longer "improvements to make."

- ✅ **Health system** — `Organism::getHealth()`, `Creature::takeDamage()`, `WoundState` enum at `include/objects/creature/creature.hpp:98, 637, 677`. Full combat system documented at `docs/technical/systems/combat-system.md`.
- ✅ **Spatial indexing** — `include/world/SpatialIndex.hpp`, `PlantSpatialIndex.hpp`. Doc at `docs/technical/systems/spatial-index.md`.
- ✅ **Decomposition / corpse system** — `include/world/Corpse.hpp`, `CorpseManager.hpp` with decay timers, toxicity, freshness.
- ⚠️ **Plant resource depletion infrastructure** — the `EnergyBudget` system (`include/genetics/expression/EnergyBudget.hpp`), `PlantEnergyCalculator` (`include/genetics/organisms/PlantEnergyCalculator.hpp`), and `Plant::energyState_` are all shipped. Only the feeding-drain connection is missing. See [[../plants/resource-system]] for the concept-match audit and the minimal MVP fix.

---

## High Priority

Critical for meaningful simulation dynamics.

### Resource Management

- [ ] **Connect feeding to plant energy drain** (MVP-blocking, 1–2 days) — All infrastructure shipped; one missing hookup. `Plant::getNutrientValue()` needs to be routed through `energyState_.currentEnergy` and `FeedingInteraction` needs to call a new `Plant::consumeEnergy()`. Without this, plants are unlimited food and creature populations starve after overshooting carrying capacity (visible in headless sim as 50+ starvations / 1000 ticks). **Flagged as the single highest-leverage unblocking task as of 2026-04-16.** See [[../plants/resource-system]].

- [ ] **Tissue-type resource differentiation** (post-MVP) — Full plan proposes per-tissue resource pools (leaves, fruit, stems, seeds, roots), each with different energy density, defense, regeneration, and creature-adaptation access requirements. Richer plant economy layered on top of the MVP fix. **Full design document:** [[../plants/resource-system]]

- [ ] **Unified propagule gene system** — Fruit/seed genes are artificially separated. Single `wrapper_investment` gene creates spectrum: seeds → nuts → drupes → berries. Eating wrapper disperses seeds; eating seeds destroys them. **Full design document:** [[../plants/propagule-genes]]

---

## Medium Priority

Ecosystem depth and balance improvements.

### Predator-Prey Dynamics

- [ ] **Carnivores don't hunt each other** - Same-species predators ignored
  - Add `genetic_similarity` threshold to hunting logic
  - Intra-species competition for territory/mates

### Decomposition System

- [ ] **Decomposition system** - No corpse decay or food freshness
  - Add freshness/age to food items
  - Corpse-to-nutrient conversion
  - Scavenger food source (ties to scavenger diet fix in genetics)

### Time-Based Mechanics

- [ ] **Seasonal fruit production** - `time_of_day` exists but no seasonal cycles
  - Tie plant reproduction to seasonal calendar
  - Different plants fruit at different times
  - Migration patterns based on food availability

### Performance

- [ ] **Spatial indexing for plant-creature interactions** - Proximity queries inefficient
  - Implement spatial partitioning (quadtree or grid)
  - Efficient neighbor lookups for feeding, sensing
  - Critical for large population simulations

---

## Low Priority

Future enhancements for ecosystem realism.

### Resource Distribution

- [ ] **Dynamic food spawning based on biome** - Food spawns uniformly
  - Different biomes produce different food types/amounts
  - Seasonal variation in food availability
  - Carrying capacity per biome

### Population Dynamics

- [ ] **Population density effects** - No overcrowding mechanics
  - Disease spread at high density
  - Resource competition scaling
  - Migration pressure

---

## Dependencies

Many ecosystem improvements depend on roadmap phases:

| Improvement | Depends On |
|-------------|------------|
| Seasonal fruit | Phase A (Seasonal Cycle Engine) |
| Decomposition | Phase B (Waste & Dispersal) |
| Food spawning | Phase A (Per-Biome Temperature) |
| Spatial indexing | None (can implement now) |

---

## Implementation Notes

### Creature Changes

Health system changes primarily affect:
- [`creature.hpp`](include/objects/creature/creature.hpp)
- [`creature.cpp`](src/objects/creature/creature.cpp)

### Food System Changes

Food depletion affects:
- [`food.hpp`](include/objects/food.hpp)
- [`food.cpp`](src/objects/food.cpp)
- [`spawner.cpp`](src/objects/spawner.cpp)

### World Changes

Spatial indexing and decomposition affect:
- [`world.hpp`](include/world/world.hpp)
- [`world.cpp`](src/world/world.cpp)

---

## See Also

- [[genetics/improvements]] - Genetics-specific improvements
- [[ui/improvements]] - UI and rendering improvements
- [[README]] - Overall roadmap with phase dependencies

### Detailed Design Documents

- [[plants/resource-system]] - **Critical fix** for unlimited plant food issue (38 hours)
- [[plants/propagule-genes]] - Unified fruit/seed reproductive gene system
