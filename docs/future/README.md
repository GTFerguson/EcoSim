---
title: EcoSim Future Development Roadmap
aliases: [roadmap, future-roadmap]
---

# EcoSim Future Development Roadmap

**Status:** 🎯 Active Planning
**Last Updated:** 2025-12-23

---

## Overview

This directory contains detailed plans for future EcoSim development. The current roadmap focuses on bridging the gap between **existing genes** (67+ defined) and **environmental systems** needed to make those genes meaningful.

### Current State

| System | Status |
|--------|--------|
| **Genes Defined** | ✅ 67+ genes in unified genome |
| **Environment** | ⚠️ Static (global 20°C, no wind, uniform light) |
| **Aquatic Life** | ❌ Water is terrain only, no adaptation |
| **Dynamic World** | ❌ No seasonal changes, static biomes |

---

## Development Phases

**Implementation Order: A → B → C → D → E → F**

### Phase A: Environment Foundation (🔴 HIGHEST PRIORITY)

**Goal:** Make existing environment-dependent genes functional

**Components:**
- Day/Night Cycle System
- Wind System (for seed dispersal)
- Per-Biome Temperature
- Varying Light Intensity (shade mechanics)
- Seasonal Cycle Engine

**Genes Enabled:**
- `TEMP_TOLERANCE_LOW/HIGH` → Survival depends on biome
- `LIGHT_REQUIREMENT` → Plants die in shade
- `SEED_AERODYNAMICS` → Wind affects dispersal
- `PHOTOSYNTHESIS` → Varies with light

**Timeline:** 3-4 weeks
**Detailed Plan:** [[environment/world-roadmap|World Environment Roadmap]]

---

### Phase B: Waste & Dispersal System (🔴 HIGH PRIORITY)

**Goal:** Enable proper endozoochory (seed dispersal via gut passage)

**Components:**
- Gut Transit Tracking
- Waste Deposition System
- Seed-in-Waste Germination

**Coevolutionary Impact:**
- Plants evolve fruit appeal + seed durability
- Animals evolve fruit preference + seed grinding
- Emergent mutualism from individual selection

**Timeline:** 2-3 weeks
**Detailed Plan:** [[environment/world-roadmap|World Environment Roadmap]]

---

### Phase C: Scripted Genetics System (GODk) (🟡 MEDIUM-HIGH PRIORITY)

**Goal:** Enable safe gene creation without C++ recompilation

**Vision:** Transform gene development from hardcoded C++ to modder-friendly JSON + custom scripting language.

**Components:**
- JSON Gene Schema & Loader
- Gene Parser & Validator CLI tool
- Custom Scripting Language (GeneScript)
- Behavior Scripting System

**Why After Phase B:**
- Demonstrates gene complexity justifying scripting
- Aquatic genes (Phase D) will be first scripted genes
- Enables rapid prototyping and community modding

**Timeline:** 4-6 weeks
**Full Vision:** [[genetics/godk|GODk]]
**Detailed Plan:** [[environment/world-roadmap|World Environment Roadmap]]

---

### Phase D: Aquatic Life (🟢 MEDIUM PRIORITY)

**Goal:** Enable organisms to inhabit aquatic environments

**New Systems:**
- Stamina System (movement costs)
- Oxygen/Lung Capacity
- Gill Efficiency (underwater breathing)
- Aquatic Locomotion Genes
- Aquatic Plant Adaptations

**Evolutionary Pathways:**
1. **Pure Aquatic** (fish-like): High gills, low land speed
2. **Amphibious** (frog-like): Balanced adaptations
3. **Diving Specialist** (otter-like): High lung capacity
4. **Terrestrial** (baseline): Land-focused, drowns easily

**Implementation:** All aquatic genes via Phase C GODk (JSON/GeneScript) as demonstration

**Timeline:** 2-3 weeks
**Detailed Plan:** [[environment/world-roadmap|World Environment Roadmap]]

---

### Phase E: Dynamic World Systems (🟢 MEDIUM PRIORITY)

**Goal:** Create dynamic environmental changes driving speciation

**Components:**
- **Dynamic Sea Levels:** Seasonal rise/fall creates temporary islands
- **Biome Transitions:** Temperature/moisture gradients shift biomes
- **Climate Zones:** Latitude affects temperature distribution

**Speciation Driver:**
- Populations isolated on seasonal islands
- Genetic drift during isolation periods
- Selection for aquatic vs terrestrial adaptations

**Timeline:** 2-3 weeks
**Detailed Plan:** [[environment/world-roadmap|World Environment Roadmap]]

---

### Phase F: Conditional Gene Expression (🟡 LOW-MEDIUM PRIORITY)

**Goal:** Enable genes to activate/deactivate based on environment

**Components:**
- Seasonal Gene Switching
- Hibernation Behavior (survive winter via dormancy)
- Fur/Coat Variation (shed summer, thicken winter)
- Future: Complex Metamorphosis

**Energy Impact:**
- Hibernation reduces metabolism to 10%
- Enables winter survival outside tolerance range
- Evolves naturally in cold climate populations

**Timeline:** 2-3 weeks
**Detailed Plan:** [[environment/world-roadmap|World Environment Roadmap]]

---

## Game Mode Shipping

The six engine phases above build the **capability substrate** for EcoSim's game modes. Modes are shipped separately once the engine is mature enough to support them, per the unified-engine plan in [[../game-concepts/README|Game Concepts]]. Development is sequential but runtime is unified: once a mode ships it operates alongside every other mode on the same persistent world.

### Genesis Mode — first shipping mode

**What it is:** The god-sim / scenario-based evolution sandbox — the mode closest to the current engine's strengths. See [[../game-concepts/genesis-god-sim|Genesis]].

**Current thinking on shipping path (updated 2026-04-14):** Genesis MVP can begin being built on the **existing engine**, not after Phase A. The real blockers are:

1. **Unified-organism-genome Phase 2** ([[genetics/unified-organism-genome]] — currently in progress) — the gene migration must complete before MVP work starts
2. **Polish on existing systems** — cleanup pass before new feature work
3. **Visibility systems** (biggest gap) — the player cannot engage with an invisible simulation. Includes chronicle infrastructure but also real-time inspection UI, population graphs, gene-pool views, family trees, event feeds, map overlays.
4. **Chronicle infrastructure** ([[infrastructure/chronicle-and-species-registry]]) — a subset of visibility systems, specifically the persistence layer for species/ancestry/events/biology snapshots

**Phases A–F are enhancement, not MVP prerequisite.** They upgrade Genesis from "playable" to "rich" by adding more environmental dynamics to observe, more gene effects to track, more speciation pressure to witness — but Genesis MVP ships without them waiting. The earlier "Phase A minimum / Phase B recommended" framing from this doc's previous version was correct for a *feature-complete* god game; MVP ships earlier.

**Cross-cutting system (grown, not phased):** [[genetics/neurodevelopment|Neurodevelopment]] — the cognitive architecture plan is **not a dedicated phase**. Brain regions are implemented incrementally as specific features need them. Genesis MVP ships with whichever regions the divine-favour mechanic requires (likely Hippocampus + Amygdala minimum) and the rest come online as later features depend on them. See the core design stance at the top of the neurodevelopment doc.

**Phases A–F still matter post-MVP.** Each phase enhances Genesis:
- Phase A: more environmental dynamics for the chronicle to surface
- Phase B: coevolution dynamics visible in the chronicle book
- Phase C (GODk): modder gene authoring
- Phase D: aquatic lineages
- Phase E: dynamic world drives observable speciation — biggest Genesis upgrade
- Phase F: seasonal phenotype polish

MVP → Phase B → Phase E is probably the practical sequencing for making Genesis feel progressively richer across iterations.

### Dominion Mode and Ark Mode

**Timing:** to be decided based on Genesis reception and engine maturity. These modes add new capabilities on top of the shared engine — named individuals and tribal management for Dominion, player avatar and real-time exploration for Ark — rather than replacing anything. Scheduling happens after Genesis ships.

See [[../game-concepts/README#development-path|Game Concepts → Development Path]] for the full mode-level plan.

---

## Deferred Features

### Sensory System Phases 3-6

**Status:** Deferred (after Phase E)  
**Priority:** HIGH (once environment systems complete)

**Includes:**
- Phase 3: Auditory System (hearing/frequency-based)
- Phase 4: Exploration Memory (spatial memory, unvisited areas)
- Phase 5: Full Mating Integration (sound + scent + memory)
- Phase 6: Predator-Prey Sensory Applications

**Rationale:** Environment systems must come first to make existing genes functional. Sensory expansion will enhance behavior complexity afterward.

**Details:** [[behavior/sensory-phases|Sensory System Phases]]

---

### Nutrient Recycling

**Status:** Far Future  
**Prerequisite:** Phase B (waste deposition)

**Includes:**
- Decomposition rates
- Soil nutrient levels
- Plant nutrient uptake
- Nutrient competition

**Note:** Phase B creates infrastructure (waste deposits), but nutrient tracking is a separate complex system for later.

---

### Complex Metamorphosis

**Status:** Far Future  
**Prerequisites:** Phase F + Life Stage System + Morphological Changes

**Vision:**
- Life stage-based gene expression
- Larval stage: Aquatic genes active
- Adult stage: Terrestrial genes active
- Example: Aquatic larva → Terrestrial adult (frog-like)

**Complexity:** Extremely high, requires multiple foundational systems

---

## Timeline Summary

| Phase | Duration | Cumulative |
|-------|----------|------------|
| A: Environment Foundation | 3-4 weeks | 4 weeks |
| B: Waste & Dispersal | 2-3 weeks | 7 weeks |
| C: Scripted Genetics (GODk) | 4-6 weeks | 13 weeks |
| D: Aquatic Life | 2-3 weeks | 16 weeks |
| E: Dynamic World | 2-3 weeks | 19 weeks |
| F: Conditional Expression | 2-3 weeks | 22 weeks |
| **TOTAL** | **15-22 weeks** | |

**Parallelization:** Some components within phases can be developed simultaneously with multiple contributors.

**Game mode shipping relative to this timeline:**

| Milestone | Prerequisite | Notes |
|-----------|-------------|-------|
| **Genesis MVP** | Gene migration complete + polish + visibility systems | Ships on existing engine without waiting for Phase A. This is the current target. |
| **Genesis enhanced (coevolution visible)** | MVP + Phase A + Phase B | Adds seasonal/wind/temperature dynamics and coevolution payoff |
| **Genesis enhanced (speciation-driving)** | MVP + Phase E | Dynamic world drives observable speciation; the full god-sim reward loop |
| **Dominion** | TBD post-Genesis | Additive mode on shared engine |
| **Ark** | TBD post-Dominion | Additive mode on shared engine |

---

## Document Index

### Improvement Tracking (by System)

- **[[genetics/improvements|Genetic Improvements]]** - Genetics-specific improvements
- **[[environment/ecosystem-improvements|Ecosystem Improvements]]** - Creature behavior and world mechanics
- **[[ui/improvements|UI Improvements]]** - UI and rendering improvements

### Planning Documents

- **[[README]]** ← You are here (roadmap overview)
- **[[environment/world-roadmap|World Environment Roadmap]]** - Detailed implementation specifications for all phases
- **[[genetics/godk|GODk]]** - Full GODk vision (JSON genes, modding API, marketplace)
- **[[behavior/sensory-phases|Sensory System Phases]]** - Deferred sensory expansion phases

### Implementation Plans

- **[`plans/omnivore-trade-offs-design.md`](../../plans/omnivore-trade-offs-design.md)** - Dietary coevolution mechanics

### Completed/Historical Documents

- **[[genetics/unified-organism-genome|Unified Organism Genome]]** - ✅ Design exploration (UniversalGenes now implemented in codebase)

---

## Rationale for Phase Order

**Why A → B → C → D → E → F?**

1. **Phase A First:** Existing genes reference environmental conditions that don't exist. Must implement environment before adding more genes.

2. **Phase B Before C:** Demonstrates gene interaction complexity and validates need for scripting system.

3. **Phase C (GODk) Before D:** Aquatic genes will be first major scripted genes, demonstrating the system's power.

4. **Phase D Before E:** Need aquatic adaptations working before dynamic sea levels create aquatic selection pressure.

5. **Phase E Uses Seasons from A:** Sea level changes depend on seasonal cycle engine.

6. **Phase F Last:** Conditional expression requires seasons (A) and dynamic environment (E) to be meaningful.

---

## Success Metrics

### Overall Project Success
- [ ] All 67+ defined genes have observable effects on survival/reproduction
- [ ] Emergent speciation driven by environmental isolation
- [ ] Community creates 3+ gene packs via GODk
- [ ] Distinct aquatic and terrestrial evolutionary lineages
- [ ] Observable coevolution between plants and animals

### Phase-Specific Metrics

See detailed plans in:
- [[environment/world-roadmap|World Environment Roadmap]] - Full success criteria for each phase
- [[genetics/godk|GODk]] - GODk-specific validation and community metrics

---

## Contributing

This roadmap is iterative and will be refined based on:
- Implementation experience
- Performance testing
- Community feedback
- Emergent gameplay patterns

### How to Provide Feedback

1. **Implementation Challenges:** Document in phase-specific sections
2. **Balance Issues:** Log in success metrics tracking
3. **New Ideas:** Add to deferred features or future extensions
4. **Timeline Adjustments:** Update based on actual completion times

---

## Questions & Discussion

### Why prioritize environment over more genes?

**Answer:** We have 67+ genes, many referencing environmental conditions that don't exist (wind for seed dispersal, temperature variation for tolerance genes, dynamic light for photosynthesis). Adding more genes compounds the problem. Environment systems make existing genes meaningful.

### Why scripted genes (GODk) so early?

**Answer:** After implementing Phase A (environment) and Phase B (waste), the complexity of gene interactions will be evident. GODk enables:
- Rapid prototyping of new genes
- Community modding without C++ knowledge
- Safer experimentation (sandboxed scripts can't crash simulation)
- First demonstration: All aquatic genes in Phase D via GODk

### When will sensory phases 3-6 be implemented?

**Answer:** After Phase E (Dynamic World). Environment systems take priority, but sensory expansion is HIGH priority afterward to enhance predator-prey dynamics and mating behaviors.

### What about nutrient cycling?

**Answer:** Phase B creates infrastructure (waste deposits with nutrient values), but full nutrient cycling (soil levels, plant uptake, decomposition) is far future due to complexity. The foundation will be ready when needed.

---

**Last Updated:** 2025-12-23  
**Next Review:** After Phase A Completion  
**Maintained By:** Development Team
