---
title: Ecosystem & Creature Behavior Improvements
created: 2025-12-24
updated: 2025-12-26
status: planned
tags: [ecosystem, creatures, behavior, world, future]
---

# Ecosystem & Creature Behavior Improvements

Improvements to creature behavior, world mechanics, and ecosystem dynamics.

---

## High Priority

Critical for meaningful simulation dynamics.

### Combat & Health

- [ ] **Health system required for combat** - Creatures need health for meaningful interactions
  - Add `_health` field to `Creature` (like Plant has)
  - Add damage/healing mechanics
  - Foundation for predator-prey injury system, non-fatal encounters

### Resource Management

- [ ] **Plant resource depletion system** - Plants provide unlimited nutrition (CRITICAL)
  - Track tissue-specific resources (leaves, fruit, stems, roots)
  - Feeding consumes plant resources
  - Photosynthesis-leaf dependency for regeneration
  - Root investment enables regrowth after grazing
  - **Full design document:** [[plants/resource-system]]

- [ ] **Unified propagule gene system** - Fruit/seed genes are artificially separated
  - Single `wrapper_investment` gene creates spectrum: seeds → nuts → drupes → berries
  - Eating wrapper disperses seeds; eating seeds destroys them
  - **Full design document:** [[plants/propagule-genes]]

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

- [[blocking-issues]] - Critical blocking issues (includes hunting extinction bug)
- [[genetics/improvements]] - Genetics-specific improvements
- [[ui/improvements]] - UI and rendering improvements
- [[README]] - Overall roadmap with phase dependencies

### Detailed Design Documents

- [[plants/resource-system]] - **Critical fix** for unlimited plant food issue (38 hours)
- [[plants/propagule-genes]] - Unified fruit/seed reproductive gene system
