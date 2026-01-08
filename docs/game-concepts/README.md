---
title: EcoSim Game Concepts
created: 2026-01-08
status: exploration
tags: [game-design, concepts, planning]
---

# EcoSim Game Concepts

This directory contains game design concepts that could be built using the EcoSim engine. Each concept leverages the engine's unique strengths in different ways.

## Engine Strengths

The EcoSim engine provides capabilities that enable genuinely novel gameplay:

| Capability | Unique Aspect | Game Potential |
|------------|---------------|----------------|
| **Real Genetics** | 88 genes, diploid inheritance, codominance | Breeding with actual depth |
| **Emergent Behavior** | Diet types, specializations emerge from continuous genes | Unpredictable evolution |
| **Coevolution** | Plants and creatures adapt to each other | Dynamic ecosystem balance |
| **Scent Communication** | Pheromone trails for mating and territory | Social/tribal mechanics |
| **Realistic Trade-offs** | Omnivores vs specialists, pleiotropy effects | Meaningful strategic choices |
| **Spatial Index** | Efficient large-population simulation | Massive living worlds |
| **Combat System** | Predator-prey with health and damage | Survival pressure |

---

## Game Concepts

### [[genesis-god-sim]] - Scenario-Based Evolution Sandbox

**Genre:** God Game / RTS Hybrid
**Core Experience:** Watch evolution unfold, make strategic interventions, complete scenario objectives

A god-game where players observe and guide evolution with structured challenges. Combines sandbox freedom with mission-based goals.

**Target Audience:** Strategy players who enjoy emergent systems

---

### [[dominion-tribal]] - Tribal Gene Cultivation

**Genre:** Colony Sim / Breeding Game Hybrid
**Core Experience:** Guide a small tribe of named creatures through generations, recruit genetics from the wild

Rimworld meets Pokémon breeding - character-driven gameplay where every creature matters and genetic decisions have generational consequences.

**Target Audience:** Players who enjoy attachment to characters and long-term planning

---

### [[ark-monster-collecting]] - Creature Collection RPG

**Genre:** Action RPG / Monster Collecting
**Core Experience:** Explore biomes as a Keeper, capture creatures, breed for traits, defeat Guardian bosses, compete in contests

Pokémon meets Monster Hunter - active exploration with your avatar, real genetic breeding depth, boss fights that demand counter-breeding strategies, and **creature contests** (speed trials, beauty pageants, survival challenges) that reward diverse breeding beyond combat optimization.

**Target Audience:** Action RPG fans who want creature collection with strategic depth

---

## Comparison Matrix

| Aspect | Genesis | Dominion | The Ark |
|--------|---------|----------|---------|
| **Player Role** | God/overseer | Tribe leader | Active Keeper avatar |
| **Creature Count** | Hundreds-thousands | 5-30 named | Team roster + storage |
| **Attachment Level** | Low - population level | High - individuals matter | Medium - team favorites |
| **Gameplay Style** | Strategic observation | Colony management | Action RPG exploration |
| **Primary Challenge** | Scenario objectives | Survival + growth | Boss fights + breeding |
| **Multiplayer Potential** | Competing ecosystems | Tribal warfare | Trading + PvP battles |

---

## Implementation Considerations

### Shared Requirements

All concepts would benefit from:
- Improved UI for viewing genetics and lineage
- Event/scenario system
- Achievement/legacy tracking
- Tutorial/onboarding systems

### Concept-Specific Requirements

See individual concept documents for detailed requirements.

---

## Development Path

### Progressive Complexity

The three concepts form a natural development progression, each building on the previous:

```
Genesis (God Sim)
    │
    │  Adds: Named individuals, recruitment mechanics,
    │        character attachment, tribal management
    ↓
Dominion (Tribal/Colony Sim)
    │
    │  Adds: Player avatar, real-time exploration,
    │        boss battles, RPG progression
    ↓
The Ark (Action RPG)
```

### Recommended Approach

1. **Phase 1:** Enhance core engine (environment systems per roadmap)
2. **Phase 2:** Build Genesis (god sim) - closest to current engine
3. **Phase 3:** Extend to Dominion (tribal) - adds individual focus
4. **Phase 4:** Evolve to The Ark (RPG) - adds player avatar and bosses

Each phase builds on the previous, with reusable systems carrying forward. Work on Genesis creates foundation for Dominion; Dominion's systems enable The Ark.

---

## See Also

- [[../future/README]] - Engine development roadmap
- [[../technical/README]] - Technical documentation
- [[../technical/design/coevolution]] - Coevolution system design
