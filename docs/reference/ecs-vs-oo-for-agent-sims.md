---
title: ECS vs OO Composition for Agent-Based Simulations
created: 2026-04-15
updated: 2026-04-15
status: active
tags: [architecture, performance, ecs, simulation, agent-based]
---

# ECS vs OO Composition for Agent-Based Simulations

Evidence base for choosing between Entity-Component-System (ECS) architectures and OO-with-composition for EcoSim's unified-organism refactor. The question is not "is ECS faster in theory" but "does ECS pay off for a 1k–100k organism sim with gene-driven dynamic capabilities on modest hardware, and is migrating toward it worth the refactor cost?"

> [!NOTE]
> This doc grades sources by evidence tier. Most ECS knowledge is Tier 5 (practitioner) because the discipline is dominated by library authors and game devs, not academic researchers. A few Tier 1–2 papers exist for the distributed/GPU endpoint.

## 1. ECS architecture families and their tradeoffs

Two dominant implementation strategies exist, with a clear tradeoff axis:

| Strategy | Exemplars | Iteration cost | Component add/remove cost | Best for |
|---|---|---|---|---|
| **Archetype** (table per unique component set) | Flecs, Unity DOTS, Bevy, Legion | Lowest — cache-coherent SoA | **High** — copy all components when set changes | Homogeneous populations, rare mutation |
| **Sparse-set** (per-component sparse array keyed by entity ID) | EnTT, Specs, Shipyard | Higher — pointer chasing across sparse sets | **Low** — O(1) insert/remove | Heterogeneous populations, frequent mutation |

Archetype implementations "excel at large-scale iteration through cache efficiency" but "incur higher composition change costs"; sparse-set implementations "enable cheaper entity modifications but scale poorly during iteration" (Tier 5: [Run-time Performance Comparison of Sparse-set and Archetype ECS](https://diglib.eg.org/items/6e291ae6-e32c-4c21-a89b-021fd9986ede), Eurographics UK 2025).

The canonical archetype concern: "rapidly adding and removing status effects can force entities to migrate between tables, causing memory copies" (Tier 5: [Sander Mertens — Building an ECS #2: Archetypes](https://ajmmertens.medium.com/building-an-ecs-2-archetypes-and-vectorization-fe21690805f9)). In Specs, add/remove is O(1); in Legion (archetype), it is O(N) where N is the entity's current component count (Tier 5: [Specs and Legion, two very different approaches to ECS](https://csherratt.github.io/blog/posts/specs-and-legion/)).

## 2. The entity-count threshold — when ECS starts to pay off

Archetype ECS outperforms sparse-set "only somewhere above 100,000 or 1 million entities" on iteration workloads (Tier 5: [ECS 1: Inheritance vs Composition and ECS Background](https://leatherbee.org/index.php/2019/09/12/ecs-1-inheritance-vs-composition-and-ecs-background/)). Below that, the iteration win is marginal while the mutation cost remains proportionally heavy.

A real case study shows the downside is not theoretical: converting Samurai Gunn 2 to archetypal storage achieved 0.5 ms snapshots but dropped debug-build performance **from 300–500 FPS to just 5 FPS** — a catastrophic regression the author deemed unacceptable (Tier 5: [Moonside Games — Archetypal ECS Considered Harmful?](https://moonside.games/posts/archetypal-ecs-considered-harmful/)). The author notes that "when entities are treated very dynamically, you cause an explosion of archetypes" and advocates sparse storage with filters for most real games, citing Overwatch as a shipping counterexample.

Conversely, contrarian-to-the-contrarian: the "Your ECS Probably Still Sucks" author recommends archetype ECS but only after non-trivial additional optimizations (64-entity slice aggregates, bit filtering, chunk-level change detection, sticky caches) — the baseline archetype ECS is not sufficient on its own (Tier 5: [Your ECS Probably Still Sucks — Practical Tips](https://gist.github.com/Dreaming381/89d65f81b9b430ffead443a2d430defc)).

## 3. How shipping sim games actually handle thousands of agents

No major shipping simulation game uses textbook ECS as the core architecture. Instead, they use domain-specific optimizations targeting the actual bottleneck.

### Factorio — chunk-local allocation + deterministic single-thread

The bottleneck was cache invalidation from interleaved entity updates. Solution: "organise memory allocation so that everything on the same chunk or related to specific tasks uses its own memory allocator, with each chunk having dedicated memory for its entities." Parallelism is introduced via Jacobi-style chunk coloring (3×3 grid numbered 1–9, same-number chunks update in parallel). Most of the sim remains **single-threaded** because determinism is required for multiplayer (Tier 5: [Factorio FFF #215 — Multithreading issues](https://factorio.com/blog/post/fff-215); [FFF #421 — Optimizations 2.0](https://factorio.com/blog/post/fff-421)). This is effectively archetype pooling at the spatial level, without the ECS abstraction.

### Stellaris — agent grouping (abandoning per-individual simulation)

Original design simulated individual "pops" (~100k+ late-game), hit a wall. Patch 4.0 grouped pops "by Species, Strata, Ethics, and Faction" with pop numbers scaled up 100× — **the game now simulates groups, not individuals**, and user-facing granularity is emulated on top (Tier 5: [Stellaris Patch 4.0 notes](https://stellaris.paradoxwikis.com/Patch_4.0)). The architectural lesson: at >100k agents, the question stops being "how do I iterate them efficiently" and becomes "do they need to exist individually at all."

### Cities: Skylines 2 — activity gating (skip idle agents)

"Agents mostly remain inactive at their home or workplace, and when active, the only significant computational cost is navigation" (Tier 5: [CS2 technical explanation of simulation speed](https://steamcommunity.com/app/949230/discussions/0/4031347929700006128/)). CS1 had a hard 65k-agent limit; CS2 removes it by keeping inactive agents dormant. Again, the data layout is less important than *what you skip*.

### Dwarf Fortress — algorithmic bottleneck, not data layout

Line-of-sight is O(n²) and consumes the majority of CPU; "over 60% of processing time is in units taking their turns, of which less than 10% is actually pathfinding related" (Tier 5: [DF Wiki — Maximizing framerate](https://dwarffortresswiki.org/index.php/Maximizing_framerate)). Mitigations are algorithmic (LOS capped at 26 tiles, spatial pruning, disabling temperature sim). DF is OO, single-threaded, and scales poorly — but switching to ECS would not fix any of its actual problems because the bottleneck is algorithmic complexity.

### RimWorld — ThinkTree hierarchy + hand optimization

Pawn behavior uses a hierarchical ThinkTree to select jobs, with WorkGivers implementing broad searches. The game is "surprisingly unoptimized for its class" and caps out around ~1k pawns before stuttering. The primary performance footguns are WorkGivers that scan the whole map each tick (Tier 5: [RimWorld Performance Analyzer docs](https://github.com/simplyWiri/Dubs-Performance-Analyzer); [RimWorld optimization community guide](https://steamcommunity.com/sharedfiles/filedetails/?id=3150465850)). Again — OO architecture, algorithmic bottleneck, not a data-layout problem.

## 4. The academic endpoint — distributed and GPU batch sims

**TeraAgent** (Tier 2: [Breitwieser et al., 2025, arXiv 2509.24063](https://arxiv.org/abs/2509.24063)): distributed agent-based simulation engine. Simulates half a trillion agents, 84× improvement over prior state-of-the-art BioDynaMo. Key mechanisms: direct-access serialization and delta-encoded communication. **Distributed**, not single-machine — not applicable to EcoSim's target of running on modest hardware.

**Madrona** (Tier 2: [Shacklett et al., SIGGRAPH 2023](https://dl.acm.org/doi/10.1145/3592427)): GPU-accelerated ECS for batch environment simulation. 11×–33× speedup on HideSeek/Overcooked RL training workloads. "Entities sorted by environment ID allow threads working on neighboring entities to access the same shared environment state, leading to coherent and cache-friendly accesses." **GPU batch**, aimed at RL training where many parallel environments matter — not single-environment detail sim.

**FLAME GPU** (Tier 4: [A Framework for Megascale ABM on GPUs](https://www.jasss.org/11/4/10.html), JASSS 2008): x-machine agents stored in per-state linked lists, CUDA kernels for bulk state transitions. Demonstrates GPU parallelization model for agents. Still GPU-bound.

**ABM benchmark on HPC** (Tier 2: [Rousset et al., 2018, *J. Supercomputing*](https://link.springer.com/article/10.1007/s11227-018-2688-8)): benchmarks HPC ABM frameworks (Repast, FLAME, D-MASON) on standardized workloads. Confirms single-machine architectures bottleneck at ~10k–100k agents; distributed architectures are required beyond that.

Takeaway: the academic frontier for massive-scale ABM is GPU/distributed. None of it is directly applicable to a single-machine ecology sim. The papers are valuable as a ceiling reference — they define what's achievable if performance is the *only* concern — but the complexity cost is not justified for EcoSim's scale.

## 5. What does this mean for EcoSim?

**EcoSim's actual parameters:**
- Target scale: 1k–100k organisms (below the archetype-wins threshold)
- Single-machine, single-threaded bias (determinism matters for reproducibility)
- Gene-driven dynamic capabilities (component attach at birth, change rare; mutation happens at reproduction, not per-tick)
- Heterogeneous population (plants, creatures, future hybrids)
- Likely bottlenecks: spatial queries, AI decision-making, pathfinding — **not** component iteration

**Where ECS would help:**
- Cache-coherent iteration over all organisms for tick-level passive processes (metabolism, aging, fatigue decay) — this is genuine but likely a small fraction of total CPU at EcoSim scale
- Bulk operations on the sessile population (if plants ever get numerous enough to matter)

**Where ECS would hurt:**
- Spatial queries still need external indices — ECS doesn't help with "what's within 10 tiles of me"
- AI decision-making (behavior controller) is inherently per-entity sequential — ECS iteration benefits don't apply
- Archetype churn at reproduction bursts if we adopt archetype ECS (mitigated by the fact that reproduction is rare relative to ticks)
- Refactor cost: rewriting `Creature`/`Plant`/`PlantManager`/`World::takeTurn()` is enormous, and the payoff at our scale is marginal

**Middle path — "OO with pooled archetypes":** Use `std::unique_ptr<Component>` composition (B1 from the design discussion) with optional later optimization to group organisms by their component set into separate vectors for cache-coherent iteration. This is effectively manual archetype pooling without committing to an ECS framework. It preserves refactor incrementality and defers the "textbook ECS" decision until profiling actually justifies it.

The shipping sim games confirm this path: Factorio's chunk-local allocator is archetype pooling by spatial locality; Stellaris's pop grouping is archetype pooling by ethic/species; neither uses an off-the-shelf ECS. The pattern is "apply archetype-like memory grouping where profiling shows it matters," not "start with ECS and hope the rest falls out."

## 6. Recommendation for EcoSim

1. **Phase 3 should start with B1 (OO composed components)** — `std::unique_ptr<MobilityComponent>` etc. on a unified `Organism` class. This is the plan on the table.
2. **Defer any ECS commitment until post-Phase-3 profiling** — measure where the CPU actually goes before committing to a data-layout rewrite. Likely candidates: spatial queries, behavior evaluation, pathfinding.
3. **Reserve "archetype pooling" as a later optimization** — if profiling shows iteration over all organisms is hot, introduce pool vectors keyed by component signature without migrating to a framework.
4. **Skip full archetype ECS** (Flecs, Bevy, Unity DOTS) — the refactor cost is not justified at 1k–100k entities with heterogeneous populations and frequent behavior evaluation. The industry wisdom is unanimous: textbook archetype ECS is overkill below ~100k homogeneous entities.
5. **Do not plan a two-step OO→ECS migration** — if ECS ever becomes justified (unlikely), it will be a rewrite, not a refactor. One committed rewrite is cheaper than two if both are inevitable, but in EcoSim's case the second one is not inevitable.

The decision reduces to: start with OO composition, measure, pool if needed. Do not pre-optimize into an architecture whose wins don't manifest until you're 10× past the target population.

## References

- **Shacklett et al. (2023)** — An Extensible, Data-Oriented Architecture for High-Performance, Many-World Simulation. *ACM Trans. Graphics* (SIGGRAPH 2023). [madrona-engine.github.io](https://madrona-engine.github.io/shacklett_siggraph23.pdf) (Tier 2)
- **Breitwieser et al. (2025)** — TeraAgent: A Distributed Agent-Based Simulation Engine for Simulating Half a Trillion Agents. arXiv:2509.24063. (Tier 2)
- **Rousset et al. (2018)** — Designing a benchmark for the performance evaluation of agent-based simulation applications on HPC. *Journal of Supercomputing*. [Springer](https://link.springer.com/article/10.1007/s11227-018-2688-8) (Tier 2)
- **Run-time Performance Comparison of Sparse-set and Archetype ECS** — Eurographics UK 2025. [diglib.eg.org](https://diglib.eg.org/items/6e291ae6-e32c-4c21-a89b-021fd9986ede) (Tier 4 — conference paper, academic but not peer-reviewed at high bar)
- **Moonside Games — Archetypal ECS Considered Harmful?** [moonside.games](https://moonside.games/posts/archetypal-ecs-considered-harmful/) (Tier 5 — shipping-game practitioner, Samurai Gunn 2 case study)
- **Dreaming381 — Your ECS Probably Still Sucks** [GitHub gist](https://gist.github.com/Dreaming381/89d65f81b9b430ffead443a2d430defc) (Tier 5 — contrarian practitioner)
- **Sander Mertens — ECS FAQ** [GitHub](https://github.com/SanderMertens/ecs-faq); **Building an ECS #2: Archetypes** [Medium](https://ajmmertens.medium.com/building-an-ecs-2-archetypes-and-vectorization-fe21690805f9) (Tier 5 — Flecs author, pro-archetype bias noted)
- **C. Sherratt — Specs and Legion, two very different approaches to ECS** [csherratt.github.io](https://csherratt.github.io/blog/posts/specs-and-legion/) (Tier 5)
- **abeimler/ecs_benchmark** — community ECS framework benchmarks. [GitHub](https://github.com/abeimler/ecs_benchmark) (Tier 5)
- **LeatherBee Games — ECS Inheritance vs Composition** [leatherbee.org](https://leatherbee.org/index.php/2019/09/12/ecs-1-inheritance-vs-composition-and-ecs-background/) (Tier 5)
- **Factorio Friday Facts #215** — Multithreading issues. [factorio.com](https://factorio.com/blog/post/fff-215) (Tier 5)
- **Factorio Friday Facts #421** — Optimizations 2.0. [factorio.com](https://factorio.com/blog/post/fff-421) (Tier 5)
- **Stellaris Patch 4.0 — Pop rework** [paradoxwikis.com](https://stellaris.paradoxwikis.com/Patch_4.0) (Tier 5)
- **Cities: Skylines 2 — technical explanation of simulation speed** [Steam discussion](https://steamcommunity.com/app/949230/discussions/0/4031347929700006128/) (Tier 5)
- **Dwarf Fortress Wiki — Maximizing framerate** [dwarffortresswiki.org](https://dwarffortresswiki.org/index.php/Maximizing_framerate) (Tier 5)
- **Dubs Performance Analyzer — RimWorld profiling tool** [GitHub](https://github.com/simplyWiri/Dubs-Performance-Analyzer) (Tier 5)
