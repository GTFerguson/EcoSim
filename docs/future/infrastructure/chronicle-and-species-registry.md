---
title: Chronicle System & Species Registry
status: planned
tags: [infrastructure, persistence, chronicle, species-registry, pipeline]
---

# Chronicle System & Species Registry

Core infrastructure required for both the Genesis chronicle/event-log feature and future Dominion-mode pipeline-readiness. **These two features need the same infrastructure — building the chronicle system IS building the Dominion handoff.**

**Context: chronicle is a subset of a broader "visibility systems" need.** The player cannot engage with an invisible simulation. Beyond the persistent chronicle/history described here, Genesis MVP also needs real-time visibility into current world state — population graphs, gene-pool views, family trees, event feeds, map overlays, inspection UI. The chronicle provides the *historical* visibility layer (what happened over time); the other visibility systems provide the *current state* layer (what's happening right now). This plan covers only the historical side. The current-state visibility systems are the scope of [[../ui/improvements|UI improvements]] and should be planned in parallel — Genesis MVP needs both.

## Why this exists

Three independent design needs converged on the same underlying data requirements:

1. **Genesis chronicle book** — Genesis's core payoff is *the world you produced* made readable as a physical-feeling **chronicle book** the player can flip through at the end of a run. Envisioned as an illuminated-manuscript-styled artifact (CK2's Chronicle feature is the closest shipped precedent — see [[../../reference/historical-narrative-generation#author-then-fire-crusader-kings-rimworld-wildermyth|reference doc §CK2 Chronicle]]) with chapters by era, species biographies with portraits, notable events as illustrated spreads, and dynasty/lineage trees as fold-out pages. Without this, sandbox mode is "numbers going up" instead of "I made something worth telling." The book IS the accomplishment — the sandbox mode is designed around producing it.
2. **Shareable world books** — the chronicle is meant to be **shareable**. A player should be able to export a completed world and publish it, and other players should be able to browse world books online and discover stories that emerged from other people's simulations. This is the distant-future target that motivates some architectural decisions now (export format, metadata, self-contained rendering); it is not on the near-term roadmap but the data design must not preclude it. Imagine a library of Fortune-style worlds each with their Gardenbacks and Grey Wretches, readable without running the sim.
3. **Dominion handoff** — When Dominion mode ships (post-Genesis), it needs to import evolved species as locked "biology cards" that constrain civ actions. This is the Indonesian Gentleman Central Civ I pattern (post 22486926, species cards at 22487015+) documented in the Evolution Games reference doc. Without persistent species identity and ancestry, Dominion can't consume Genesis's output.

All three needs require:
- A **species registry** with stable IDs (so "the Pusmal" is a referenceable thing, not a recomputed classification)
- **Ancestry chains** stored in organism state (so "all descendants of founder X" is queryable)
- **Event persistence** in the save file (so history survives across sessions)
- **Species biology snapshots** (so a species's current traits can be exported at will)
- **Self-contained export format** (so a world book can be shared without requiring the sim engine to read it)

Building them together is substantially cheaper than building them separately. Failing to build them now means retrofitting later when Dominion ships and again when sharing comes online.

### What distinguishes EcoSim's chronicle from Dwarf Fortress's

Dwarf Fortress is the canonical blueprint for the chronicle problem and the [[../../reference/historical-narrative-generation#simulate-then-filter-dwarf-fortress|reference doc]] documents its architecture in detail. EcoSim's contribution to this problem space is distinct in one specific way: **EcoSim approaches history generation from an emergent-evolution angle, whereas DF's species and civilizations are scripted templates instantiated by world-gen.** DF's dwarves are always dwarves, its dragons are always dragons — the species roster is authored. EcoSim's species are products of actual genetic drift across generations of sim runtime; every "species" that appears in the chronicle is something the player (or their predecessor simulation) watched evolve from whatever came before it.

The consequence: a DF chronicle can tell you *what the dwarves did*; an EcoSim chronicle can tell you *why the Pusmal exist at all*, what their body plan traces back to, which environmental pressures shaped each of their notable traits, and which evolutionary dead-ends branched off their lineage. DF lacks this dynamism because its content substrate is static; EcoSim's content substrate is the gene pool itself.

This is what makes the chronicle work worth doing despite DF already shipping a version of it. DF is still the blueprint for the *architecture* — typed events, stable entity IDs, post-hoc culling, decoupled rendering, Legends-mode browsable index — but EcoSim's chronicle has material DF cannot produce: the evolutionary backstory of everything in the world. Every species biography is not just *"here is what the Pusmal did"* but *"here is what the Pusmal became, and what they evolved from, and what they might have become instead."*

This framing should guide scope decisions: chronicle features that surface evolutionary history (lineage trees, phenotype drift timelines, ancestral-form illustrations, convergent-evolution callouts) are high-priority because they are the differentiating contribution. Chronicle features that merely replicate DF's approach (chronological event lists, faction histories, battles) are lower-priority because DF already does them and the value-add for EcoSim is the evolutionary layer on top.

## Current state (audited 2026-04-14)

A concrete audit of the codebase established that most of this infrastructure does not yet exist. Individual-level data is rich; species-level history is empty.

| Capability | Present? | Location |
|---|---|---|
| Species as first-class entity | ❌ No — computed on demand | `CreatureTaxonomy::generateScientificName()` at `src/objects/creature/creature.cpp:2013` |
| Stable species ID | ❌ No | — |
| Lineage beyond immediate parents | ❌ No — parent IDs only exist in CSV log, not organism state | `Logger.hpp:225, 251` |
| Founder / dynasty concept | ❌ No | — |
| Persistent event log | ❌ No — Logger flushes CSV at runtime, not serialized to save file | `Logger.hpp:319` |
| Speciation / extinction event records | ⚠️ Signaling only (not records) | `Logger.hpp:268` |
| Species biology snapshot | ❌ No — would need to aggregate genomes by hand | — |
| Organism serialization | ✅ Yes | `CreatureSerialization.hpp`, `fileHandling.hpp:145-184` |
| World state save/load | ⚠️ Partial — individual creatures/plants yes, no species or lineage registry | `fileHandling.hpp` |

The archetype flyweights in `ArchetypeIdentity.hpp` (population counts per archetype) are the closest existing approximation to species-level state but are not linked to lineage history.

## Required components

### Component 1 — Species Registry

A persistent mapping of species identity to stable IDs, populated as genome clusters emerge from the simulation.

**Responsibilities:**
- Detect when a new species has emerged and assign a stable ID
- Track species lifespan: first-seen tick, last-seen tick, current population
- Store player-assignable or procedurally-generated names that lock into canon once assigned
- Handle sub-species and convergent evolution cases correctly
- Expose "species at tick T" queries for historical reconstruction

#### Proposed species definition (starting point, to be interrogated)

Defining a species is famously contested in evolutionary biology — every classical concept (biological, phylogenetic, morphological, genetic, ecological) has known failure modes, and picking one arbitrarily will produce taxonomy that disagrees with either intuition or the simulation's own mechanics. EcoSim has a structural advantage over real taxonomy: it has **perfect ancestry information for free** (from Component 2) and **perfect reproductive-compatibility information** as a direct side effect of normal play. That eliminates the hardest parts of the real-world problem.

Proposed composite definition on three axes, composed so the failure modes of each are patched by the others:

**Axis 1 — Reproductive isolation (primary, clean in sim).**
The existing mating system knows whether two organisms can produce viable offspring. When genetic drift between two sub-populations crosses the threshold where mating produces no fertile offspring (or reliably none), they are separate species. This is the **Biological Species Concept** (Mayr 1942), which fails in real biology because the mating experiment cannot be run at scale — in a sim it runs constantly as a side effect of play. Concrete detectable event: `SpeciationEvent` when the compatibility barrier forms.

**Axis 2 — Lineage ancestry / monophyly (resolves convergent evolution).**
Two organisms only belong to the same species if they share a recent common ancestor (walked via the ancestry chains in Component 2). This directly addresses convergent evolution: two lineages that independently evolve similar phenotypes and even similar genotypes are **never** classified as the same species, because the ancestry check separates them. Real biology's phylogenetic methods struggle because gene-tree reconstruction is noisy; the sim just walks parent pointers and finds the last common ancestor directly.

**Axis 3 — Genetic distance (secondary, for sub-species).**
When two populations in the same phylogenetic cluster are still reproductively compatible but have drifted genetically past some threshold, they are **sub-species** — incipient speciation, the state before the reproductive barrier forms. This gives the player something to watch before a full speciation event: *"the Hastun of the north are drifting from the southern population; they may diverge."* Concrete detectable event: `SubspeciesEmergenceEvent` with divergence metric.

#### Convergent evolution handling

When two lineages with distinct ancestry converge on similar trait profiles, the registry marks them as **distinct species with a `convergent_with: [species_id]` cross-reference**. This is genuine narrative material — convergent evolution is inherently story-worthy in real biology, and the sim can detect it automatically because it sees both the phenotype similarity and the ancestral divergence. Component 3's event log should emit a `ConvergenceEvent` flagging the shared traits.

#### Archetype vs species (orthogonal concepts)

The existing `ArchetypeIdentity` flyweights measure a **niche / ecological role**, not an ancestry. Two species from completely different lineages can both be "Pack Hunters" — they share an archetype without sharing a species. Archetypes are ecologically meaningful and phylogenetically meaningless; that is the correct behaviour for a niche label and should not be conflated with species identity.

- **Species** answers *"who is this"* — ancestry + reproductive compatibility
- **Archetype** answers *"what does it do"* — ecological role regardless of ancestry

A predator → prey extinction cascade is an archetype-level story; a dynasty rising and falling is a species-level story. Both are valid chronicle entries but they key on different indices.

#### Open design questions (still unresolved)

- **Threshold tuning** — the two arbitrary numbers are (a) the genetic-distance threshold that triggers sub-species recognition, and (b) the mating-fertility threshold below which two populations are considered reproductively isolated. Both need empirical tuning from actual simulation runs; there is no principled prior value. Start conservative (high thresholds, few species) and tune down based on observed speciation frequency.
- **Ring species** — populations where A↔B and B↔C are compatible but A↔C are not. Real biology gives up and calls them edge cases; the sim can report the compatibility graph honestly and label them as a special "ring" sub-case.
- **Hybrid species** — when two species interbreed and produce a stable third. Provisionally: treat hybrid populations as a sub-species of whichever parent has more shared ancestry, with a `hybrid_origin: [parent_a, parent_b]` tag. Real biology has no clean answer here either.
- **Asexual lineages** — reproductive isolation is undefined for them. Fall back to genetic distance + ancestry only.
- **Naming policy** — auto-name via `CreatureTaxonomy::generateScientificName()`, player-supplied override, or both? If both, player names override but the auto-name is kept as a "also known as".
- **Retirement** — do extinct species stay in the registry indefinitely (good for history) or get archived after N ticks (good for memory)? Probably archived to disk but never deleted; extinction is itself a chronicle event that readers will query.
- **Clustering cost** — detecting species emergence requires comparing genomes across the current population. How does this interact with the existing `SpatialIndex` and tick-rate budget? May need to run on a slower cadence than the main sim tick.

### Component 2 — Ancestry Chains

Parent pointers stored in organism state so descendants can be walked without relying on log parsing.

**Responsibilities:**
- Add `parentId1`, `parentId2` fields to `Organism` and `Creature` (currently only in Logger output)
- Support "all descendants of founder X" queries up to configurable depth
- Optional: flag founder individuals (first of a new species, first to cross a biome boundary, etc.) as special ancestors with extra metadata

**Open design questions:**
- Depth tradeoff: how deep do ancestry walks need to go? (Dominion imports need at least "founder of lineage", maybe more.)
- Storage cost: every organism carrying two integer parent IDs is cheap; heavier metadata (generation number, inbreeding coefficient) has scaling implications.
- Pruning: do dead ancestors stay in memory, on disk, or get GC'd?

### Component 3 — Persistent Event Log

Notable events (not raw sim events — the filtered subset) serialized to save files.

**Responsibilities:**
- Define an event record schema: `{ tick, type, location, species_ids, founder_id, payload }`
- Emit notable-signal events from filter layers (first/last of species, population thresholds, mutation fixation, ecological firsts, speciation, mass extinction, etc.)
- Persist to save file alongside creatures and plants
- Expose query API: "events between tick A and B", "events about species X", "events of type Y"

**Open design questions:**
- Storage cost: how many events should a long run accumulate? Needs a size budget and a retention policy. Dwarf Fortress's post-hoc culling pass (drop historical figures that never interacted with named things) is a cheap bulk filter to consider — see the [[../../reference/historical-narrative-generation#simulate-then-filter-dwarf-fortress|reference doc]].
- Filter design: which signals are worth persisting? The [[../../reference/historical-narrative-generation|historical narrative generation research]] documents Kreminski's story sifting literature (Felt, Winnow, Select-the-Unexpected) as the formal framework for this; the "statistical rarity as significance proxy" idea from Kreminski et al. 2022 is the clearest candidate for automated scoring.
- Relationship to existing Logger: should Logger become the upstream raw-event bus, with the filter layer sitting between it and the persistent event log? RimWorld's hard split between ambient sim (not logged as story) and Storyteller incidents (the only things that become narrative) is a live precedent; see reference doc §Author-then-fire.

### Component 4 — Species Biology Snapshots

On-demand or periodic captures of a species's current trait distribution, stored as "biology cards".

**Responsibilities:**
- Aggregate genome and phenotype data across a species's current population
- Produce a structured record: movement traits, dietary role, social structure, reproductive strategy, notable fixed mutations, environmental tolerances
- Expose to save file and to the UI's Species Chronicle view
- Be consumable as a Dominion character-sheet import when that mode ships

**Open design questions:**
- Capture cadence: snapshot on every save, on speciation events, on explicit player request?
- Schema evolution: what happens when new genes are added? Snapshots must remain readable.
- Aggregation method: use mean trait values? Modal phenotype? Full distribution? Each has different costs and fidelity.

## Dependencies and build ordering

All four components share infrastructure and should be built as one coherent project, though they can ship incrementally:

```
  Species Registry (1) ───┐
            │             │
            ↓             ↓
  Ancestry Chains (2)   Notable Events (3)
            │             │
            └─────┬───────┘
                  ↓
        Biology Snapshots (4)
```

- **Component 1 (Species Registry)** is the foundation — without stable species IDs, events cannot be meaningfully attached to species and snapshots have nothing to key on.
- **Components 2 and 3** can be built in parallel once 1 exists.
- **Component 4** depends on 1 (needs species IDs) and benefits from 2 (lineage info in snapshots).

Minimum shippable chronicle: Components 1 + 3 + a small set of notable-signal filters. Biology snapshots (4) can be added once Dominion mode is closer to shipping.

## Where this fits in the engine roadmap

This is a **cross-cutting infrastructure requirement**, not a standalone phase in the A–F sequence. It is a **prerequisite for Genesis shipping** in its full form (the "goal-less but milestone-rich" sandbox mode depends on a working chronicle). Per [[../README#game-mode-shipping|the mode shipping plan]]:

- Genesis **minimum** shipping (after Phase A) can ship with a bare-bones chronicle: species registry + extinction/first-of-species events only.
- Genesis **recommended** shipping (after Phase B) needs a more complete chronicle including coevolution milestones and mutation fixation.
- Genesis **strong** shipping (after Phase E) needs the full narrative-pattern layer including speciation, adaptive radiation, and mass extinction detection.

This suggests the chronicle work should be **sequenced alongside Phases A and B**, not deferred until after them.

## Informing research (in flight)

The detailed design of the filter and pattern-detection layers is deferred to in-flight research documented at [[../../reference/historical-narrative-generation]]. Key questions that research should answer:

- How does Dwarf Fortress's Legends system structure its historical event table?
- What does Max Kreminski's "story sifting" literature say about filter pipeline design?
- What patterns do Crusader Kings, RimWorld, Caves of Qud, and Wildermyth use to surface narratively significant events from raw simulation data?

Once that research doc lands, update this plan with concrete filter/pattern proposals grounded in the literature.

## The chronicle book as UI surface

The chronicle's primary presentation is a **book** that the player can read through. This is the user's stated vision and it is central to why this system exists — the book is not a visualisation of data, it is the artifact the sandbox mode produces. Design signposts (not commitments):

- **Book metaphor as first-class UI.** Illuminated-manuscript style, page-turn interaction, table of contents, chapter titles by era or geological age, searchable index. CK2's Chronicle feature is the closest shipped precedent — the reference doc describes it as "an Anglo-Saxon-Chronicle-styled per-dynasty history book [...] paginated, illuminated-manuscript-styled scrollable book [...] with exportable text."
- **Chapters structured around narrative patterns, not ticks.** Events aren't listed chronologically first — they're organised by theme: The First Life, The Age of Giants, The Great Divergence, The Silent Centuries, The Age of Mind (first sapience). The chronology is a back-of-book appendix for readers who want it.
- **Species biographies as dedicated pages.** Each notable species gets a page with a portrait (auto-rendered from the species biology snapshot), inherited traits, ecological role, notable descendants, and the events that define its arc.
- **Lineage trees as fold-out pages.** Dominion-facing "what descended from what" visualisation, readable by both modes.
- **Maps as endpapers.** Current-world map on inside front cover; ancient world map (from earlier eras) on inside back cover. A natural use of the geological upheaval events that Fortune uses as chapter boundaries.
- **Player-assigned names lock into canon.** The "Hugo Packageberg" mechanic from Bord Quest — if the player names a species or a notable event, that name is voiced earnestly by the book's prose, not bracketed as user input.

This is UI design scope and is deferred until the underlying data is in place. It is documented here so the data structures are designed knowing the book is the consumer, not a raw event log viewer.

## Shareable export format

The user's longer-term vision is a shareable "world library" where players browse other people's completed worlds like browsing books. This is **not on the near-term roadmap** and will not be implemented until long after Genesis ships, but the data design now must not preclude it. Implications:

- **Self-contained export.** The chronicle must be exportable as a single file (candidate formats: self-contained HTML with embedded images and CSS, PDF, or a compact binary bundle with a dedicated reader). The sim engine must not be required to read it.
- **Metadata for discovery.** World exports need a manifest: era range, species count, notable events summary, "cover image" (probably the final-era world map), tags or player-supplied genre labels.
- **Deterministic rendering.** If two players both export the same world state, the book should render identically, so shared books are verifiable and comparable.
- **Size budget.** A shareable world book needs to fit a realistic download — the technical budget this imposes on the raw event log is substantial. Most raw events can be filtered/culled before export; the book only carries what the filter layer kept.
- **Privacy, moderation, hosting.** If and when sharing infrastructure is built, the usual social-platform concerns apply. Not this plan's scope.

The minimum requirement from this plan's perspective: **a standalone world-book exporter must be technically possible given the data model**, even if the actual exporter is built much later.

## Open questions (not yet decided)

- **Storage budget:** how much save-file growth is acceptable per in-game year? How much fits in a shareable world-book export?
- **Backward compatibility:** the audit shows `CreatureSerialization` is active — migration path for existing saves?
- **Book rendering engine:** build a dedicated book viewer into the game client, or generate HTML/PDF on export and delegate rendering to the player's browser?
- **Multiplayer implications:** if Genesis supports shared/async multiplayer, do events have author attribution? Can two players claim the same world or fork each other's?
- **Naming canonicalisation:** when a player names a species, does the name propagate retroactively into prior events in the book?
- **Performance:** species clustering over large populations needs a spatial/genetic index — how does this interact with the existing `SpatialIndex`?
- **Cover-image generation:** what goes on the book's "cover" when shared? Auto-generated from world biome map? Player-assigned? Both?

## See also

- [[../../game-concepts/tg-evolution-games]] — research source for why this infrastructure matters
- [[../../game-concepts/genesis-god-sim]] — the primary consumer of the chronicle system
- [[../../reference/historical-narrative-generation]] — in-flight research informing the filter and pattern layers
- [[../README]] — engine development roadmap (game mode shipping section references this plan)
- `include/logging/Logger.hpp` — the existing Logger class this plan's filter layer sits atop
