---
title: Visibility Systems
created: 2026-04-14
status: proposed
priority: critical
tags: [ui, visualization, visibility, genesis-mvp, inspection]
---

# Visibility Systems

**Status: proposed — MVP-blocking.**

The real-time layer of world visibility that makes Genesis legible to the player. A sibling plan to [[../infrastructure/chronicle-and-species-registry|chronicle-and-species-registry]], which covers the *historical* layer. Together they form the visibility-systems scope that Genesis MVP depends on.

## Why this plan exists separately from `ui/improvements.md`

`ui/improvements.md` is a polish checklist — click-to-select, stress in inspector, scent overlays, zoom center, fullscreen. It is narrow, tactical, and appropriate as a backlog of small improvements. The visibility-systems scope is different: it is the **framework for making a simulation readable**, and without it Genesis has no MVP. Separating the two keeps `ui/improvements.md` as a clean polish tracker and this doc as the feature plan.

The user articulated the requirement: *"we need better visibility systems into the world which the chronicle system could be a part of"* — and later *"a large issue with it right now is all creatures are just squares of varying sizes with no real detail."* Both point at the same gap: the engine runs but the player cannot see what's happening or visually distinguish creatures from one another.

---

## Scope

The visibility-systems plan covers **six feature clusters**. Each is a distinct subsystem that can ship incrementally; none is individually as large as the chronicle plan.

### 1. Real-time inspection framework

The foundation. A generalised panel system that renders structured views of any selected entity (creature, plant, species, region, event). Currently the `ImGuiOverlay` has an inspector list selector at `src/rendering/ImGuiOverlay.cpp:777` but no click-to-select from the world and no extensible panel system.

**Required:**

- **Click-to-select from the world view.** Currently `SDL2InputHandler.cpp:365-416` forwards mouse events generically; no world-pick. Add spatial-index-backed click picking that selects the topmost entity under the cursor.
- **Tabbed inspector panels.** Current, Genetics, Energy, Lineage, Social, History tabs. Each panel reads from a different subsystem.
- **Multi-entity selection** for population-level queries ("show me the average genome of the Grey Wretches").
- **Inspector-driven navigation** — selecting a parent or offspring in the lineage tab should switch inspector focus to that creature.

### 2. Population graphs

Time-series visualisation of population counts, gene frequencies, and ecological metrics. Currently the `Statistics` system tracks aggregates (see `include/statistics/statistics.hpp`) but there is no real-time graph view in the UI.

**Required:**

- **Population over time** — per species, stacked by lineage or archetype.
- **Gene frequency histograms** — distribution of a selected gene across a species (e.g., "show me the temperature tolerance distribution across Grey Wretches").
- **Ecological metrics** — biodiversity index (Simpson or Shannon), biomass pyramid, predator/prey ratio, average fitness.
- **Zoomable time range** — from recent ticks to full-run history (historical data comes from the chronicle's persistent event log).
- **Cross-species comparison** — overlay two or more species on the same graph.

### 3. Gene-pool distribution views

Visualise the *diversity* of genes within a species or population — not just averages but the spread. Essential for the player to see whether a species is converging, diverging, or stable.

**Required:**

- **Per-gene distribution curves.** For each gene in a species, show a histogram of allele values across the live population.
- **Gene correlation matrix.** Which genes are correlated in the current gene pool? Useful for spotting co-evolution and linkage disequilibrium.
- **Inbreeding coefficient / genetic diversity index.** One-number summary of a population's genetic health.
- **Phenotype space projection.** 2D projection (PCA or similar) of the population in gene space, so the player can *see* clustering, drift, and speciation events visually.

### 4. Family / lineage trees

**Live** lineage views, distinct from the chronicle's historical lineage chronicles. A player should be able to click on a creature and see its ancestors (as deep as Component 2 of the chronicle plan allows) plus its currently-living descendants and cousins.

**Required:**

- **Ancestor walk** — trace parent pointers backward, display as a tree or list.
- **Descendant tree** — all living descendants of a selected creature, grouped by generation.
- **Founder highlight** — if a creature is descended from a notable founder (e.g., the first individual of its species), mark the founder and the lineage path.
- **Cross-species marriage lines** (where hybrids exist) — visualise genetic inheritance from multiple parent species.

This feature depends on the chronicle's **Component 2 (ancestry chains)** being shipped first. The live tree reads from the same parent-pointer data that the chronicle stores.

### 5. Event feeds (current state layer)

A real-time feed of notable events happening *right now* in the simulation, complementing the chronicle's historical event log. The chronicle records what happened over time; the event feed shows what's happening this tick.

**Required:**

- **Filtered live event stream** — births, deaths, first-of-species, speciation events, intervention effects, chronicle event emissions.
- **Priority filtering** — player can mute routine events (births, routine deaths) and surface only narratively significant ones.
- **Click-through navigation** — clicking an event in the feed jumps the camera to the event location and selects the involved entities.
- **"Notifications" pop-up style** for high-priority events (first sapience, mass extinction, major speciation) so the player doesn't miss them even if the feed is scrolled.

### 6. Map overlays

Layered visualisation on the main world view. Toggle-able overlay layers that show normally-invisible data.

**Required:**

- **Scent overlays** — visualise pheromone gradients (kin, territory, alarm, mate-available). Already partly on the `ui/improvements.md` polish list; promote here as a first-class feature.
- **Territory overlays** — show which species controls which areas.
- **Biome / environment overlays** — temperature, humidity, light, water per tile.
- **Population density heatmap** — per-species hotspots.
- **Influence maps** — predator/prey pressure, food availability, divine-favour concentration.
- **Selection highlighting** — when a creature is selected, highlight its territory, its kin group, and its visual range.

---

## Creature appearance (cross-cutting, foundational)

A dedicated sub-feature that cuts across Tier 1 (in-world rendering) and Tier 2 (inspector). Currently all creatures render as squares of varying sizes — a critical visibility gap.

**Approach (grounded in research, see [[../../reference/procedural-creature-appearance]]):** Two-tier gene-driven procedural creature appearance.

### Tier 1 — Runtime in-world rendering

- **Creatures-style ATT attachment points** — modular body parts indexed by per-part genes (Creatures Cyberlife 1996 precedent). Each creature has head, torso, limbs, tail sprite indices plus parametric attachment offsets.
- **Pattern layer baked at birth** — generate a small per-creature texture from the appearance genes using a CPPN (for symmetric markings) or reaction-diffusion (Turing-style coat patterns). Cache on the creature; never regenerate at runtime.
- **GPU instancing** — all creatures rendered in one draw call via a shared texture atlas with per-instance UV offsets and tint vectors. Scales to thousands of creatures visible simultaneously.
- **Bake once at birth, never regenerate.** Appearance is fixed from birth to death. If genes change mid-life (metamorphosis, life stage), re-bake at the transition event, not continuously.

### Tier 2 — Creature inspector images

- **Same modular composition at higher resolution** — reuse body-part atlas at inspector scale.
- **Optional CPPN detail layers** — additional pattern passes (pelt variation, scar patterns, asymmetric features) affordable for one-creature-at-a-time.
- **L-system appendage branching** — for creatures with branching features (antlers, horns, coral growths). Cheap enough for on-demand generation.
- **Cached per-creature** keyed by creature ID; regenerate only if appearance genes change.

### Appearance genes

The appearance genome includes at minimum:

- **Body part indices** — which sprite for head, torso, limbs, tail (Creatures-style breed slots)
- **Attachment offsets** — parametric positions where body parts attach
- **Size scalars** — per-part size multipliers
- **Pattern parameters** — CPPN weights or reaction-diffusion reaction constants
- **Colour base + variation** — primary colour, secondary colour, tint rules
- **Symmetry modifiers** — degree of bilateral symmetry

Appearance genes are inherited with mutation like any other gene pool, so **breeding mixes appearance** alongside biology. A pack-hunter × fish-eater child might have the mother's head sprite, father's limb sprites, and an intermediate pattern texture — visually recognisable as both lineages' descendant. This is the Creatures breed-slot mechanism translated to EcoSim.

### Techniques explicitly ruled out

From the research (full rationale in [[../../reference/procedural-creature-appearance#6-techniques-definitively-ruled-out-for-tier-1]]):

- **Stable Diffusion / diffusion models** — too slow for Tier 1 (~50ms for *one* creature at best). Ruled out.
- **GANs** — same cost profile. Ruled out.
- **Per-frame procedural generation** of any kind — the architecture depends on bake-once-at-birth.

Stable Diffusion *could* be viable for Tier 2 inspector images with genome-hash caching, but deployment complexity is far beyond the rest of the engine and CPPN + modular sprites at higher resolution covers the design goal. Not recommended as default.

---

## MVP prioritisation

Not all of the six clusters ship together. For Genesis MVP, the minimum viable visibility system is:

| Cluster | MVP? | Minimum viable form |
|---------|------|---------------------|
| 1. Real-time inspection framework | ✅ | Click-to-select from world + Current + Genetics + Energy tabs. Lineage tab depends on chronicle Component 2 shipping. |
| 2. Population graphs | ✅ | Population-over-time per species, at minimum. Gene frequency histograms and diversity indices can ship post-MVP. |
| 3. Gene-pool distribution views | ⚠️ Partial | Per-gene distribution curves for the inspector's Genetics tab. Correlation matrices and phenotype space projection are post-MVP. |
| 4. Family / lineage trees | ⚠️ Partial | Live ancestor walk + living descendant tree — requires chronicle Component 2. Founder highlighting and hybrid lines are post-MVP. |
| 5. Event feeds | ✅ | Live event stream with priority filtering and click-through navigation. Pop-up notifications can ship post-MVP. |
| 6. Map overlays | ⚠️ Partial | Scent overlays (already on polish list) + population density heatmap. Influence maps and selection highlighting are post-MVP. |
| **Creature appearance** | ✅ | **Tier 1 only** (modular sprites + GPU instancing + simple CPPN pattern). Tier 2 inspector images post-MVP. **Ruled out: SD/GAN/per-frame generation.** |

**MVP total:** clusters 1, 2 (partial), 5, creature appearance Tier 1, plus partials of 3, 4, 6.

**Deferred to post-MVP:** the fuller forms of clusters 3, 4, 6; creature appearance Tier 2; pop-up notifications; multi-entity selection; cross-species comparison.

---

## Dependencies

- **Chronicle Component 2 (ancestry chains)** — required for the Lineage inspector tab and live family trees. Must land before those features can ship.
- **Chronicle Component 3 (persistent event log)** — feeds the historical side of population graphs (gene frequency over long time ranges). Live current-state graphs can ship first; historical graphs depend on the chronicle.
- **Species registry (chronicle Component 1)** — required for per-species graphs, gene-pool views, and most inspector queries. First dependency.
- **Existing `Statistics` aggregates** — already shipped; population graphs can build on top.
- **Existing `ScentLayer`** — already shipped; scent overlays just need UI work.

## What this plan does NOT cover

- **Chronicle book UI** — the flippable illuminated-manuscript rendering of the world's history. That is a dedicated feature in the chronicle plan, not part of real-time visibility.
- **Scenario mode UI** — objective tracker, hint system, tutorial scaffolding. Belongs in a Genesis-scenario plan doc.
- **Genre/menu polish** — main menu, settings, save/load dialogs. Belongs in `ui/improvements.md`.
- **Creature animation** — movement physics, idle behaviours, attack animations. A separate concern; creature appearance is static pose, not animation.

## Open design questions

- **Click-to-select depth** — should click select the topmost entity, cycle through overlapping entities on repeat clicks, or show a multi-select menu?
- **Overlay interaction** — when multiple overlays are toggled on, how are they composited (colour blending, stacking, cycled)?
- **Performance budget for real-time graphs** — how often should time-series data be sampled? Per tick is too much; per 100 ticks may be too coarse. Empirical tuning required.
- **Chronicle book as a separate UI mode or integrated into inspector** — if the chronicle book is its own modal experience, the inspector's History tab becomes a secondary view. If integrated, they share a single UI. Decide before implementing either.
- **Creature appearance gene count** — how many distinct appearance genes does the appearance genome need? The research recommends body-part indices + attachment offsets + size scalars + pattern parameters + colour + symmetry. Minimum viable set needs a design pass.

## See also

- [[../../reference/procedural-creature-appearance]] — the research reference doc this plan consumes for creature appearance
- [[../infrastructure/chronicle-and-species-registry]] — historical visibility (sibling plan)
- [[../../game-concepts/genesis-god-sim#mvp-shipping-path-current-thinking]] — Genesis MVP prerequisites where visibility systems are listed as the biggest gap
- [[improvements]] — narrow polish backlog (distinct from this plan)
- [[../../technical/systems/spatial-index]] — spatial indexing that click-to-select will use
- [[../../technical/systems/scent-system]] — scent layer that scent overlays will visualise
- [[../../technical/systems/behavior-system]] — what inspector panels will surface per creature
