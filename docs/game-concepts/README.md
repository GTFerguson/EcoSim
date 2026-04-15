---
title: EcoSim Game Concepts
created: 2026-01-08
status: exploration
tags: [game-design, concepts, planning]
---

# EcoSim Game Concepts

This directory contains game design concepts that could be built using the EcoSim engine. Each concept leverages the engine's unique strengths in different ways.

## Architectural Vision

EcoSim is **one engine supporting several game modes, all sharing a single persistent world**. The game concepts below — Genesis, Dominion, The Ark — are not separate products; they are **modes the player can move between** on the same simulated world. A player might foster ecosystem growth in Genesis (god-sim) mode, zoom into Dominion (tribal colony) mode to guide a specific emerging tribe, then zoom further into The Ark (RPG) mode to play as a Keeper walking that same world as an individual character. Actions in any mode accumulate into the same world state; abandoned projects in one mode become landmarks or hooks for another. The player's engagement cadence is asynchronous — the world ticks whether they are active or not, and each mode is an entry point rather than a requirement.

**Influences.** This architecture takes direct philosophical inspiration from:

- **Dwarf Fortress** — Fortress and Adventurer modes ship as two concurrent ways to play on the same persistent generated world. Your abandoned fortress becomes explorable ruins for your adventurer; historical figures from world-gen become NPCs you can meet. DF's Legends mode is the closest shipped precedent for EcoSim's envisioned chronicle system; see [[../reference/historical-narrative-generation#simulate-then-filter-dwarf-fortress|reference doc]].
- **Crusader Kings 2** — dynasty-driven emergent narrative on top of a basic genetic system (conceptually adjacent to EcoSim's genetics). CK2's **Chronicle** feature — an illuminated-manuscript-styled per-dynasty history book populated by script-hooked events — is the closest shipped precedent for the "chronicle book as the accomplishment" framing that Genesis sandbox mode is designed around. CK3 notably dropped the Chronicle and the community has been trying to fill the gap with mods and save-scrapers ever since; see [[../reference/historical-narrative-generation#author-then-fire-crusader-kings-rimworld-wildermyth|reference doc]].
- **Black & White 1 & 2** (Lionhead Studios, 2001/2005) — the canonical god game. Villages generate **belief** through the player's miracles and moral example; belief spreads geographically; the player is pulled toward picking favoured villages to cultivate rather than treating the world uniformly. Genesis's divine-favour mechanic (see [[genesis-god-sim#divine-mythology-champions|Genesis → Divine Mythology & Champions]]) is a direct translation of the B&W belief system into an evolutionary sim: species replace villages, favour replaces belief, and the player is pulled toward cradling one species toward sapience so it idolises them as creator-god.
- **/tg/ Evolution Games** ([[tg-evolution-games]]) — community-game precedent for the same pipeline pattern. Indonesian Gentleman's Primordial → Central Civ → Bord Quest ran a single evolved world across sandbox, civ, and individual-narrative modes over years of threads.

**Development is sequential, runtime is unified.** We ship one mode first — Genesis — and once it is in players' hands we extend the shared engine to add Dominion, then The Ark. Each new mode is additive: it operates on the same world alongside the previous modes, never replacing them. A player running a Genesis scenario today should still be able to run it when Dominion and The Ark ship later, and should be able to switch into those modes on the same world state.

---

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

## Inspiration References

### [[tg-evolution-games]] - /tg/ Collaborative Evolution Games

A reference doc on the decade-old forum tradition of collaborative evolution games on 4chan's /tg/ board (Primordial Evolution Game, Species Saga, etc.). Analyses what makes them engaging through the lens of academic work on emergent narrative (Kreminski & Wardrip-Fruin 2021) and forum-based collaborative storytelling (Zalka 2012), and maps those mechanisms to possible EcoSim directions.

**Why it's here:** Not a game concept — a source of design inspiration. The format is a long-running natural experiment in collaborative evolution that EcoSim could draw from for spectator/multiplayer and chronicle-view features.

---

## Mode Comparison

Genesis, Dominion, and The Ark are **concurrent modes on one shared world**, not alternatives to choose between. The matrix below describes what each mode offers the player, not which game they are playing.

| Aspect | Genesis Mode | Dominion Mode | Ark Mode |
|--------|--------------|---------------|----------|
| **Zoom level** | World / population | Tribe / lineage | Individual character |
| **Player role** | God / overseer | Tribe leader | Active Keeper avatar |
| **Creatures in scope** | Hundreds–thousands | 5–30 named | Team roster + storage |
| **Attachment level** | Low — population level | High — individuals matter | Medium — team favorites |
| **Gameplay style** | Strategic observation | Colony management | Action RPG exploration |
| **Primary engagement** | Scenario objectives | Survival + growth | Boss fights + breeding |
| **Multiplayer potential** | Competing ecosystems | Tribal warfare | Trading + PvP battles |
| **World state contribution** | Shapes selection pressure, terrain, species pool | Shapes tribe history, settlements, culture | Shapes quest history, named NPCs, landmarks |

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

### Sequential build, additive modes

Shipping order is sequential — Genesis first, then Dominion, then The Ark — but each new mode is **additive** rather than a successor. Once shipped, a mode remains playable on every existing world; new modes extend the shared engine and operate alongside the older ones on the same world state.

```
Genesis Mode (God Sim)              ──┐
    │                                 │
    │ Extends engine with:            ├── All three coexist
    │ Named individuals, recruitment, │   on the same world
    │ character attachment            │   once each is shipped
    ↓                                 │
Dominion Mode (Tribal/Colony)       ──┤
    │                                 │
    │ Extends engine with:            │
    │ Player avatar, real-time        │
    │ exploration, boss battles       │
    ↓                                 │
Ark Mode (Action RPG)               ──┘
```

### Build order

1. **Phase 1:** Enhance core engine (environment systems per roadmap)
2. **Phase 2:** Build Genesis mode — closest to current engine, ship it first
3. **Phase 3:** Extend engine to add Dominion mode — named individuals, tribal management on the same world
4. **Phase 4:** Extend engine to add Ark mode — player avatar and real-time exploration on the same world

Each phase adds capability to the shared engine without breaking prior modes. Work on Genesis creates the persistent-world foundation Dominion and Ark inherit; Dominion's systems (named individuals, tribal state) become the substrate Ark's Keepers walk through.

**Engine phase prerequisites:** see [[../future/README#game-mode-shipping|Roadmap → Game Mode Shipping]] for the mapping between the engine phases (A–F) and the mode shipping milestones. Genesis's minimum prerequisite is Phase A (Environment Foundation); its recommended shipping window is after Phase B or Phase E depending on how much coevolution and speciation should be visible at launch.

---

## See Also

- [[../future/README]] - Engine development roadmap
- [[../technical/README]] - Technical documentation
- [[../technical/design/coevolution]] - Coevolution system design
