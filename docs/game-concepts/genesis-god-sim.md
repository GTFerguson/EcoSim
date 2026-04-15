---
title: Genesis - Scenario-Based Evolution Sandbox
created: 2026-01-08
status: concept
genre: God Game / RTS Hybrid
tags: [game-design, god-game, strategy, scenarios]
---

# Genesis: Creation, Evolution, and the Chronicle of a World

**Genre:** God Game / Evolution Sandbox with Scenario Mode
**Tagline:** *Shape the genesis of a world. Write its myths. Read its history.*

---

## MVP shipping path (current thinking)

Genesis MVP can begin being built on the existing engine once three prerequisites land, none of which are engine Phases A–F from `docs/future/README.md`:

1. **Gene migration completes** — the unified-organism-genome work ([[../future/genetics/unified-organism-genome]]), currently Phase 2 in progress (Creature/Plant still separate classes, UniversalGenes exists). This is the blocking dependency.
2. **Polish pass on existing systems** — not a new feature build, just cleanup on what's already implemented. TBD scope.
3. **Visibility systems** — the biggest gap. The chronicle system ([[../future/infrastructure/chronicle-and-species-registry]]) is *one component* of a broader need for better visibility into the world. Other visibility surfaces (real-time inspection UI, population graphs, gene-pool views, family trees, event feeds, map overlays, **creature appearance**) are what make the simulation legible to the player. Without them, the engine runs but the player can't see what's happening.

   **Creature appearance is a specific known problem.** Currently all creatures render as squares of varying sizes. This is a meaningful visibility gap — players cannot visually distinguish species, cannot see the phenotypic diversity the genes are producing, and cannot form attachment to specific lineages without a visual identity. The direction is **two-tier procedural creature rendering**: (a) cheap top-down representations for large-population in-world display, and (b) more detailed procedurally-generated images shown in creature inspectors. Both tiers should be **driven by genes**, so a lineage's visual identity evolves alongside its biology. This is a substantial technical challenge involving procedural image generation at scale; research in progress — see [[../reference/procedural-creature-appearance]] when it lands (not yet written).

Engine Phases A–F (environment foundation, waste/dispersal, GODk, aquatic, dynamic world, conditional expression) **enhance** Genesis but are not strictly required for MVP. They upgrade Genesis from "playable" to "rich" by giving the simulation more to surface — more environmental dynamics to observe, more gene effects to track, more speciation pressure to witness.

> [!NOTE]
> This is a more aggressive MVP timeline than the earlier Phase-A-minimum / Phase-B-recommended framing in the engine roadmap's Game Mode Shipping section. The earlier framing was correct for "Genesis as a feature-complete god game"; this MVP framing is correct for "start building Genesis now to get the basic shape in players' hands." Both are valid — MVP ships first, richness phases follow.

### Prerequisite summary

| Prerequisite | Status | Why |
|---|---|---|
| Unified-organism-genome Phase 2 | **In progress** (blocking) | MVP can't ship on a half-migrated gene system |
| Polish on existing systems | TBD | Cleanup pass before feature work |
| Visibility systems (including chronicle) | **Not started** (blocking) | Player cannot engage with an invisible simulation |
| Chronicle infrastructure (4 components) | **Not started** (blocking subset of visibility) | Species registry + ancestry + event log + biology snapshots |
| Creation-myth interactive onboarding | Not started | v1 feature — cheap to build once visibility exists |
| Divine favour mechanic | Not started | v1 feature — per-species scalar feeding off intervention tiers |
| Tiered intervention outcomes | Not started | v1 feature — modifies existing intervention system |
| Chronicle book UI | Not started | v1 feature — reads from chronicle data |

---

## Vision Statement

Genesis puts players in the role of the **deity who shapes a world from its first moments through its mythic age**. The player chooses the foundational myths that define the world, guides the evolution of life across generations, intervenes as a divine force when their creations struggle, and — if they invest the creative effort — selects champions, seeds religions, and writes the mythology that downstream history will carry forever.

Every run produces a **chronicle book**: an illuminated, readable record of the world the player made, organised by era, populated with species biographies and lineage trees, structured around the milestones and narrative events the player's choices produced. The book is the run's accomplishment. Scenarios provide structured challenges that give mechanical direction; the sandbox gives creative direction; both feed the same chronicle.

**Core Fantasy:** Being the god whose choices become the foundational myth of a world — and whose chronicle book is an artifact worth reading (and sharing) long after the run ends.

**On the name.** Genesis originally meant simply "the first mode to ship", but the design has grown into the literal meaning: the player participates in genesis — creation, world-formation, mythology. The creation myth is interactive, the god is the player, the chronicle is the book of everything that followed.

---

## Core Loop

```mermaid
graph LR
    A[Observe] --> B[Analyze]
    B --> C[Intervene]
    C --> D[Wait]
    D --> A
    D --> E[Achieve Objective]
```

1. **Observe** - Watch ecosystem dynamics unfold
2. **Analyze** - Identify genetic trends, population pressures, opportunities
3. **Intervene** - Make strategic changes (breeding, environment, events)
4. **Wait** - Allow generations to pass, see results
5. **Achieve** - Complete scenario objectives

---

## Intervention Tools

### Outcome Tiers

Every intervention rolls on a **five-tier outcome table** rather than resolving as a binary success/failure. The tier distribution depends on the player's **god level** (see Progression System below). Failure never stalls the world — every tier produces a story beat.

| Tier | Description | Typical in-fiction result |
|------|-------------|---------------------------|
| **Catastrophic** | Intervention backfires hard, creating new constraints | Targeted gene causes lethal pleiotropy; climate shift triggers wrong season; mass extinction hits a non-target species |
| **Side effect** | Intended effect partially lands, with unwanted consequences | Mutation fixes but reduces fertility; climate shift succeeds but disrupts migration; genetic injection spreads to sister species |
| **Partial** | Intended effect applies at reduced strength | Mutation reaches 40% of population instead of full fixation; climate shift applies to one biome only |
| **Clean** | Intended effect lands as designed | The intervention works as the player described |
| **Amplified** | Intended effect exceeds the designer's expectation | Mutation fixes *and* spreads to adjacent lineages; climate shift triggers adaptive radiation |

**Why tiered outcomes matter.** This system turns failure into *story*, not stalling. A critical-fail intervention produces a narrative consequence the chronicle records — the player's attempt to fix one problem created a new one, which is itself interesting history. The design pattern comes from Indonesian Gentleman's Central Civ I `d20` table (`1 = crit fail → workshop burns through from acidic mucus, posts 22487890/22487973`); see [[tg-evolution-games#dice-tiers-that-fail-narrate-not-just-fail-block|the research doc]]. Every roll advances the world somewhere.

**Anti-frustration rule.** If an intervention crit-fails three times in a row on the same target in the same scenario, the fourth attempt auto-succeeds at partial tier. Taken directly from Indonesian Gentleman's "three strikes → you get it anyway" fallback (OP post `22486926`): *"It's supposed to be your tribe learning from their mistakes."* Failure still has teeth; it does not trap the player in a death spiral.

### Breeding Controls

| Tool | Description | Strategic Use |
|------|-------------|---------------|
| **Matchmaker** | Force specific individuals to mate | Combine desired traits |
| **Fertility Blessing** | Boost reproduction rate for selected creature | Spread successful genetics |
| **Isolation** | Separate population into breeding groups | Create subspecies |
| **Migration** | Move creatures to new biomes | Apply selection pressure |

### Environmental Controls

| Tool | Description | Strategic Use |
|------|-------------|---------------|
| **Climate Shift** | Adjust temperature/moisture gradually | Force adaptation |
| **Resource Placement** | Add/remove food sources | Create competition or abundance |
| **Barrier Creation** | Add terrain obstacles | Geographic isolation |
| **Disaster Trigger** | Cause flood, drought, disease | Rapid selection event |

### Divine Interventions

Higher-cost tools for dramatic changes:

| Tool | Description | Cost |
|------|-------------|------|
| **Genetic Injection** | Add specific gene variant to population | High |
| **Mass Extinction** | Remove all of one species | Very High |
| **Genesis Event** | Spawn new creature type | Extreme |
| **Time Acceleration** | Speed up generations | Medium |

---

## Scenario Types

### Tutorial Scenarios

**The First Garden**
- Objective: Maintain stable herbivore population for 20 generations
- Teaches: Basic observation, food chain awareness
- Intervention allowed: Resource placement only

**Predator Introduction**
- Objective: Introduce carnivores without extinction cascade
- Teaches: Balance, population dynamics
- Intervention allowed: Breeding controls + migration

### Challenge Scenarios

**🏔️ Ice Age Survival**
```
Starting Condition: Temperate ecosystem with diverse population
Challenge: Temperature will drop 2°C per 10 generations
Objective: At least one species survives 100 generations
```

**Genetic Solutions:**
- Evolve fur density for insulation
- Evolve hibernation behavior
- Evolve migration instincts
- Evolve fat storage metabolism

**Strategic Approaches:**
1. *Specialist Path* - Breed extreme cold tolerance in one species
2. *Diversity Path* - Maintain multiple species, see what survives
3. *Migration Path* - Guide population to warmer regions

---

**🐺 Apex Predator**
```
Starting Condition: Ecosystem dominated by fast, agile herbivore
Challenge: Herbivore has no natural predator, overgrazing occurs
Objective: Establish stable predator-prey cycle
```

**Genetic Solutions:**
- Evolve hunting speed to catch prey
- Evolve pack behavior for coordinated hunts
- Evolve ambush tactics with camouflage
- Evolve endurance for pursuit hunting

**Strategic Approaches:**
1. *Speed Match* - Breed carnivores faster than prey
2. *Intelligence* - Develop pack hunting coordination
3. *Specialization* - Target young/weak prey with specific adaptations

---

**🌸 The Pollinator Pact**
```
Starting Condition: Self-pollinating plants, omnivore creatures
Challenge: Introduce pollinator-dependent plants
Objective: Stable mutualism between creatures and flowering plants
```

**Genetic Solutions:**
- Evolve color vision in creatures
- Evolve nectar attraction in plants
- Evolve flower-friendly mouth structures
- Evolve creature-friendly pollen delivery

**Coevolutionary Victory:**
Plants with high nectar + creatures with color vision = stable mutualism

---

**⚔️ Arms Race**
```
Starting Condition: Your species vs AI-guided rival species
Challenge: Both compete for limited resources
Objective: Your species achieves 60% population dominance
```

**Strategic Dimensions:**
- Combat effectiveness vs reproductive rate
- Specialist efficiency vs generalist flexibility
- Territory control vs resource extraction
- Defensive adaptations vs offensive capabilities

---

### Sandbox Mode

No objectives. Full tool access. Pure experimentation.

**Optional Challenges:**
- How many stable species can coexist?
- Can you create obligate mutualism?
- How fast can you speciate a population?
- What's the most extreme adaptation possible?

---

### Deferred: Geological Era Scenarios

**Status:** Deferred — explore later in the roadmap, not for initial Genesis shipping.

A distinct scenario category built around **mid-scenario geological or climatic upheavals** rather than static environmental sliders. Each scenario includes 1–3 scripted world-reshaping events (volcanic winter, continental rift, meteor impact, glaciation, post-ice-age greenhouse) that reopen the possibility space without erasing lineage history. The pattern comes directly from Fortune: Evolution Game's chapter structure — **Parting of the Ways** fractures the supercontinent into three, forcing divergence; **Triumph of Spring** recovers from an ice age via volcanic greenhouse effect, *"engendering an absolute explosion of life as organisms no longer struggled against the harsh conditions"* (see [[tg-evolution-games#chapter-boundaries-as-geological-soft-reboots|research doc]]).

**Why deferred:** Fortune-style chapter boundaries are structurally different from the static-slope scenarios Genesis ships with (Ice Age Survival has a linear temperature drop, not punctuated upheavals). Getting this right requires the chronicle system to be working well enough to track pre- and post-upheaval history distinctly, and scenario authoring conventions that are probably best written after playing the simpler scenario types for a while.

**Why it's here instead of forgotten:** *you said* **definitely interesting and something we should explore much later in the roadmap**. Documenting so it survives across sessions.

---

## Progression System

### Scenario Completion Rewards

| Achievement | Reward |
|-------------|--------|
| Complete scenario | Unlock next scenario |
| Complete under par time | Unlock efficiency tools |
| Complete with minimal intervention | Unlock sandbox modifiers |
| Complete all variants | Unlock scenario editor |

### Meta-Progression

**God Level** is the core progression axis. Higher god level shifts the [[#outcome-tiers|intervention outcome distribution]] toward reliable outcomes without ever fully eliminating the long tail of catastrophic failure. Illustrative distributions (exact numbers TBD during tuning):

| God Level | Catastrophic | Side effect | Partial | Clean | Amplified |
|-----------|-------------|-------------|---------|-------|-----------|
| 1 (Novice) | 20% | 30% | 30% | 15% | 5% |
| 5 (Experienced) | 10% | 20% | 25% | 35% | 10% |
| 10 (Divine) | 2% | 5% | 13% | 60% | 20% |

**Design intent:** low-level gods must work *with* the simulation — nudges, guidance, trusting evolution — because their direct-effect interventions are too unreliable to force outcomes. High-level gods *can* force outcomes, but by then the player has internalised the sim's rhythms and often chooses not to. **The arc is mastery as learning to respect the system, not overpower it.** Tutorial scenarios exist as genuine introduction to constrained play, not as baby mode.

**God XP sources:**

| Source | What it rewards |
|--------|-----------------|
| Scenario completion | Finishing a structured objective |
| Long-duration observation | Not intervening — staying out of the sim's way and letting evolution run. Creates tension: watch-and-learn vs intervene-and-risk |
| Scenario-specific milestones | *"Maintained stable population for 50 generations"*, *"Witnessed a speciation event"*, *"Detected a convergent-evolution event"* (see [[../future/infrastructure/chronicle-and-species-registry|chronicle-and-species-registry]] for the event types the system can emit) |
| Per-intervention-type practice | Casting Climate Shift improves **Climate Shift specifically** — each intervention has its own skill track, a mini skill-tree per tool. Mastery of one intervention doesn't automatically transfer to another. |

**God Powers** - Permanent unlocks that carry across scenarios:
- Enhanced observation tools (see gene details)
- Cheaper interventions
- New intervention types
- Time control options

**Achievements** - Track evolutionary milestones:
- "Speciation Event" - Split one species into two stable populations
- "Coevolution Master" - Establish 3+ mutualistic relationships
- "Survival of the Fittest" - Win Arms Race without direct intervention
- "10,000 Generations" - Run continuous simulation for extended period

---

## Divine Mythology & Champions

> [!NOTE]
> **Status: design direction.** Exploratory vision for a progression layer that turns the player-as-deity from *mechanical force* into *mythological protagonist*. **v1 scope: creation-myth choices at world-start only.** Champions, religion, and magic are all deferred to later iterations — see "v1 scope split" below.

Dwarf Fortress runs a **creation-myth generator** *before* world history — it defines the world's gods, afterlives, and magic so that downstream generators produce a coherent setting ([[../reference/historical-narrative-generation#simulate-then-filter-dwarf-fortress|reference doc]]). In DF that generator is non-interactive; Genesis could make it **player-participatory**, turning mythology-shaping into a parallel progression track alongside god level and scenario completion.

**Core framing:** the player is not just an intervention-caster with stats. They are the god of this world, and the mythology is partly their story. Effort invested in mythology shaping produces disproportionately rich chronicle material and unlocks "mythical world" runs whose chronicle books have a visibly different tier of depth than a basic playthrough.

### Components of the direction

**Creation-myth choices at world-start.** When starting a sandbox run, the player answers a short series of questions that define the world's foundational myths: *"How was this world formed? What happens to souls? Is there magic? What are its laws?"* These choices feed downstream generators — they shape the terrain generator, the starting biome palette, possibly the gene pool. They also seed the chronicle's opening chapter (*"In the beginning there was..."*). This is the pre-history creation myth as an interactive onboarding sequence, not a static world-gen parameter.

**Champion selection (deferred — lands with Ark mode).** When a sentient species emerges and the player has invested enough divine favour into that species, the player can **select a champion** — a named individual from the species whose life becomes a tracked narrative arc. Directly the Indonesian Gentleman Bord Quest pattern ([[tg-evolution-games#tier-3-narrative-protagonist-quest-bord-quest-i-dec-2013|research doc]]): zoom from species to named individual.

**Why deferred to land with Ark, not Genesis v1.** Champions are the **natural bridge mechanism to Ark mode** — your Genesis champion *becomes* your Ark playable character, walking the same world they were chosen in. A bridge only makes sense when both sides of it exist. Shipping champions in Genesis before Ark exists means building individual-level biography tracking and a selection UI for a feature with nothing downstream to feed into; shipping them paired with Ark means the feature has an immediate payoff and the data model can be designed knowing exactly what Ark needs. Champions are therefore the designated Genesis → Ark handoff and will land when Ark lands, not before.

Consequence for v1 scope: **Genesis v1 does not ship champions at all.** The engine doesn't need individual-level biography snapshots; the chronicle plan's Component 4 can stay species-only until the Ark-pair release. Genesis v1's creative-progression track is creation-myth choices, nothing more.

**Divine favour as the v1 progression narrative drive (Black & White model).** See [[../reference/belief-systems-and-god-games]] for the evidence base and honest caveats — most of what is documented about B&W's belief system is practitioner-tier, and the specific "cradle a favoured village" dynamic is folkloric rather than formally documented. Genesis's version will need empirical tuning, not citation-driven tuning. Divine favour is a **per-species scalar** measuring how much a species regards the player as their god. Favour accumulates from helpful interventions (high-tier outcomes, habitat preservation, answered crises), and drains from harmful ones (catastrophic-tier outcomes, mass-extinction interventions, environmental damage to their range). The player is not pushed to *win* a run — they are pushed to **pick a species, cradle it, nurture it toward sapience, and have it idolise them.** This is the Black & White / Black & White 2 mechanic translated to an evolutionary sim: villages in B&W generate belief through your miracles and moral example; species in Genesis generate favour through your interventions on their behalf. The player's arc in a sandbox run becomes:

1. **Creation myth** at world start sets the world's foundational lore.
2. **Early evolution** — player observes, possibly nudges, identifies an interesting species to focus on.
3. **Cradle phase** — player invests favour-earning interventions in the chosen species: fend off predators, preserve habitat during climate shifts, buffer them through disasters. Tiered intervention outcomes feed the favour stat directly, which means **every intervention has two channels of consequence: the mechanical result AND the favour impact on the target species.**
4. **Sapience threshold** — once favour and evolutionary readiness align, the species develops sapience. This is a major chronicle event and the narrative pivot of the run.
5. **Mythology** — the sapient species begins to mythologise the player. First altar, first prayer, first temple, first recorded myth — all chronicle events. The chronicle book's "mythic age" chapter opens here.
6. **Idolisation** — the species worships the player as their creator-god. The chronicle book now has a protagonist culture with its own religion, which is the accomplishment the run was working toward.

**Why this belongs in v1 despite earlier deferral.** Without divine favour, Genesis's sandbox creative-progression arc is *"make creation-myth choices, then passively watch evolution"* — which is weak. With divine favour tied to the intervention system, every action in the middle of a run has narrative stakes, not just mechanical ones. The sandbox mode now has a **clear narrative target the player works toward** without needing scenario objectives. It is also **cheap to implement** because:

- Favour is a species-level scalar, not individual-level (no champion data model needed)
- Accumulators plug directly into the already-designed tiered intervention outcomes (the `catastrophic/side effect/partial/clean/amplified` column becomes the favour delta input)
- No religion simulation required — worship is a **chronicle-layer abstraction** (the chronicle records altars, prayers, myths; the sim doesn't model every worshipper's belief state)
- No champion UI required — focus is species-level
- Negative favour is effectively free narrative richness: species that fear or hate the player develop *counter-mythology*, devil figures, and can become antagonists in the chronicle

This is what makes Black & White work as a design reference — belief was the central currency and everything the player did fed it. Genesis v1 can adopt the same structural spine (favour is the currency, every intervention is a favour transaction) without taking on B&W's creature-training or direct-village-simulation overhead.

**Chronicle events the favour system emits** (feeds Component 3 of the chronicle plan):

- Favour threshold crossings (first-time-noticed, first-prayer-possible, sapience-ready, worshipping, devoted)
- First altar / first temple / first recorded myth
- Prayer answered (player intervention in response to a species crisis while they are at prayer threshold)
- Negative favour events: first curse, first devil-figure mythologised, first apostasy
- Religious schism (deferred — see below)

**Church and religion formation (deferred — far future).** Once a species is worshipping the player, deeper religious structure — temples, priest classes, sectarian splits, competing deities — becomes possible. **This is deferred to far future.** The v1 cheap version is: the chronicle records "a temple was built", "a schism occurred", based on species state, but the sim does not model individual worshippers, priest castes, or theological content. The full religious simulation is a separate far-future project if ever.

**Magic system as player-shaped mythology.** Whether magic exists *at all* is a player choice made during the creation myth. If the player chooses "yes, magic exists", the system is shaped by further choices: *"what does magic do? who can use it? what does it cost?"* Each choice unlocks or forbids magical gene categories in the downstream simulation. Magical species traits unlock via god-species ritual interactions, not via normal evolution. This is a large commitment for the engine and is **deferred to far-future work**; the point of capturing it here is that the creation-myth system needs hooks for it when it lands.

**Effort-to-payoff for mythical worlds.** The deeper the player engages with the mythology mechanics, the richer the chronicle book. A "mythical world" run — one with champions, an established pantheon, documented religion, and possibly magic — produces a chronicle book that is visibly different from a basic playthrough. This creates a difficulty/depth axis **orthogonal to god level**:

| Axis | What it measures | Payoff |
|------|-----------------|--------|
| God level | Mechanical mastery of intervention system | Interventions become more reliable |
| Mythology depth | Creative investment in the world's myth, religion, and champions | Richer chronicle book, unlocks "mythical" tier |

A player who focuses on god level becomes a *precise manipulator*. A player who focuses on mythology becomes a *storyteller*. The two tracks are compatible but distinct — and the second one is what makes the chronicle book feel like an accomplishment rather than a log.

### Why this belongs in Genesis, not a later mode

Mythology-shaping is *creative* progression, not mechanical progression. It rewards imagination and long-form engagement. That is exactly what Genesis sandbox mode needs to not feel like a prequel to Dominion. It gives players who don't care about scenario objectives a reason to invest.

It also directly produces the narrative density that the chronicle book needs to be worth reading. Fortune's longevity relied on accumulated lore about recurring species; Genesis's chronicle payoff relies on the accumulated mythology of the world the player helped write.

### v1 scope split

| Feature | v1? | Rationale |
|---------|-----|-----------|
| Creation-myth choices at world-start | ✅ Ships in v1 | Cheap to implement (fancy world-gen UI seeding downstream generators), sets tone for entire run, gives chronicle its opening chapter, works with or without any later mythology expansion |
| Tiered intervention outcomes + god level | ✅ Ships in v1 | Already-captured mechanical-progression axis; independent of the mythology layer |
| **Divine favour (species-level, Black & White model)** | ✅ **Ships in v1** | The sandbox mode's narrative drive. Per-species scalar fed by the intervention tier outcomes; gates sapience and mythology chronicle events; no religion simulation needed; no individual tracking needed. Pushes the player to *cradle a species toward idolisation*, which is the v1 run arc. |
| Chronicle book | ✅ Ships in v1 | The accomplishment Genesis is built around |
| Champions | ❌ Deferred — lands paired with Ark mode | Champions are the Genesis → Ark bridge mechanism; build the bridge when both sides exist |
| Churches / religion (deep simulation) | ❌ Deferred — far future | Cultural simulation is hard; v1 cheap version is chronicle-layer abstraction only (altars, schisms, myths recorded as events; no individual worshipper simulation) |
| Magic system | ❌ Deferred — far future | Multi-year engine commitment; creation-myth system in v1 will have a "magic: none" choice and the non-magic path must be fully playable |
| Geological era scenarios | ❌ Deferred (already flagged above) | Depends on chronicle system being mature; post-v1 scenario category |

**v1 creative-progression track: creation-myth choices + divine favour cultivation.** The player shapes the world at world-start, picks a species to cradle and nurture toward sapience via favour-earning interventions, watches them mythologise the player, and reads the chronicle book at the end. That's the Genesis v1 sandbox arc. Scenarios provide a parallel structured-challenge track for players who want explicit objectives; both tracks feed the same chronicle.

### Concerns and open questions

- **Scope creep risk.** Addressed by the v1 scope split table above. The principle: ship creation-myth + tiered interventions + chronicle book. Defer everything else until there is a concrete downstream consumer (champions → Ark; religion → ???; magic → multi-year engine work).
- **Interaction with the species registry.** Divine favour accrues to a species, so the registry (see [[../future/infrastructure/chronicle-and-species-registry#component-1--species-registry|Component 1]]) needs a per-species favour field and per-species divine-attention event log. Champions are named individuals, which means the chronicle also needs an *individual-level* biography snapshot — this extends Component 4 (currently species-only biology snapshots) or adds a Component 5.
- **Magic system as engine commitment.** If magic is modelled, every gene, every behaviour system, and every environment interaction potentially gains a magical axis. This is a multi-year project, not a side feature. The minimum non-magic version of the creation myth system should still work.
- **Churches need cultural simulation.** Full religion simulation is hard (DF struggles with it). The cheap version: religion is a **chronicle-layer abstraction** — the chronicle records formation and schisms based on species state, but the sim doesn't literally model every worshipper.
- **"Divine favour" as a stat.** What earns favour? Non-destructive interventions? Answered prayers (species in crisis, god resolves it)? Chronicle events from the champion? Needs a concrete mechanic, not a vibes-based accumulator.
- **Player deism vs theism.** Do all Genesis runs have a god (the player is always one) or is "godless" a valid sandbox choice? A godless run still produces a chronicle book — it just doesn't have the mythology layer.

### Cross-references to update when this direction commits

**For v1 (creation-myth + divine favour):**
- [[../future/infrastructure/chronicle-and-species-registry]] — Component 1 (species registry) adds a `divine_favour` field per species. Component 3 (event log) needs new event types: creation-myth choices, favour threshold crossings (noticed / prayer-possible / worshipping / devoted / fearful / apostate), first altar, first temple, first recorded myth, prayer answered, first curse, first devil-figure. Component 4 (biology snapshots) stays species-only — no individual biographies in v1.

**For champions / Ark-pair release (deferred):**
- [[../future/infrastructure/chronicle-and-species-registry]] — Component 3 gains champion-life event types. Component 4 extends to individual-level biographies (or adds Component 5).
- [[ark-monster-collecting]] — Ark mode consumes the Genesis champion as the player's starting character; this is the handoff contract both modes must agree on.

**For far-future (deep religion simulation / magic):**
- All of the above plus whatever additional simulation layers those features require. Out of scope to plan now.

---

## UI/UX Design

### Main View

```
┌─────────────────────────────────────────────────────────────┐
│  [Scenario: Ice Age Survival]        Gen: 47    Time: 2x    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│                    WORLD VIEW                               │
│                                                             │
│     ~~~ terrain with creatures moving ~~~                   │
│                                                             │
├──────────────────────┬──────────────────────────────────────┤
│  POPULATIONS         │  SELECTED: Alpha-7                   │
│  Herbivores: 234     │  Species: Grazer                     │
│  Carnivores: 12      │  Genes: [Heat Tol: 0.3] [Speed: 0.7] │
│  Plants: 1,205       │  Health: 85%  Energy: 60%            │
│                      │  Offspring: 12  Generation: 5        │
├──────────────────────┴──────────────────────────────────────┤
│  INTERVENTIONS:  [Matchmaker] [Migrate] [Climate] [Event]   │
└─────────────────────────────────────────────────────────────┘
```

### Key Views

1. **World View** - Top-down ecosystem visualization
2. **Population Graph** - Historical population trends
3. **Gene Pool View** - Distribution of traits across population
4. **Family Tree** - Lineage visualization for selected creature
5. **Objective Tracker** - Scenario progress and hints

---

## Engine Integration

### Systems Fully Utilized

| Engine System | Game Use |
|---------------|----------|
| Genetics | Core breeding mechanics, trait visualization |
| Behavior AI | Autonomous creature actions |
| Combat | Predator-prey dynamics |
| Scent | Territory marking, mating detection |
| Coevolution | Plant-creature arms races |
| Spatial Index | Large population simulation |
| Environment | Seasonal/climate challenges |

### Additional Systems Needed

| System | Purpose | Complexity |
|--------|---------|------------|
| **Chronicle & Species Registry** | Persistent species identity, ancestry chains, notable-event log, biology snapshots — see [[../future/infrastructure/chronicle-and-species-registry]] | High — cross-cutting prerequisite |
| **Scenario Framework** | Load objectives, check completion | Medium |
| **Intervention System** | Player tools with costs/cooldowns | Medium |
| **AI Opponent** | Rival species guidance for Arms Race | High |
| **Population Analytics** | Graphs, statistics, projections | Medium |
| **Time Controls** | Pause, speed up, generation skip | Low |
| **Achievement System** | Track and reward milestones | Low |

> [!IMPORTANT]
> The **Chronicle & Species Registry** infrastructure is the most architecturally significant new system. It underpins Genesis's sandbox payoff (the history-generator framing) and is *the same infrastructure* that will later hand state to Dominion mode. The codebase currently has no persistent species identity, no ancestry chains beyond parent IDs in the Logger CSV, and no event persistence in save files — so this work must land before Genesis can ship as designed. See the detailed plan at [[../future/infrastructure/chronicle-and-species-registry]].

---

## Multiplayer Considerations

### Competitive Mode

**Parallel Ecosystems:**
- Same starting conditions, different player interventions
- Compare who achieves objective first
- Or compare final ecosystem metrics after fixed time

**Shared Ecosystem:**
- Multiple players guide different species in same world
- Natural competition without direct combat
- Diplomatic options: share territory, trade genetics?

### Asynchronous Mode

- Challenge friends with custom scenarios
- Share seed + intervention log for verification
- Leaderboards for efficiency (fewest interventions)

---

## Target Audience

**Primary:** Strategy gamers who enjoy:
- Emergent systems with surprising outcomes
- Long-term planning with delayed payoffs
- Optimization puzzles with multiple solutions
- Simulation depth with accessible interface

**Secondary:** 
- Science/biology enthusiasts
- Sandbox explorers
- Achievement hunters

**Not For:**
- Players wanting immediate action feedback
- Players who dislike waiting/observation
- Those who prefer deterministic outcomes

---

## Unique Selling Points

1. **Real Genetics** - Not fake stats. Actual inheritance, codominance, mutation.
2. **Emergent Solutions** - Players discover strategies the designers didn't anticipate.
3. **Meaningful Choices** - Interventions have cascading generational effects.
4. **Structured Freedom** - Objectives provide direction; approach is player-chosen.
5. **Living Simulation** - Ecosystem continues evolving even without intervention.
6. **The Chronicle Book** - Every run produces a readable, illuminated-manuscript-styled history book of the world the player made. The book *is* the accomplishment — sandbox mode is designed around producing it, not around scenario objectives. Long-term vision: worlds are shareable, so players can browse other peoples' chronicle books like a library of Fortune-style histories. Closest shipped precedent is Crusader Kings 2's Chronicle feature; see [[../future/infrastructure/chronicle-and-species-registry]] and [[../reference/historical-narrative-generation]].

---

## Risk Assessment

| Risk | Mitigation |
|------|------------|
| Too passive/boring | Good time controls, meaningful interventions |
| RNG frustration | Multiple valid approaches, intervention options |
| Scenario too hard | Hint system, difficulty scaling |
| Complexity overwhelming | Strong tutorial, gradual feature unlock |
| Performance at scale | Leverage spatial index, population caps |

---

## Development Priority

**High Priority:**
- Scenario framework with objective checking
- Core intervention tools
- Population analytics UI
- Tutorial scenarios

**Medium Priority:**
- AI opponent for competitive scenarios
- Achievement system
- Time controls and speed options

**Lower Priority:**
- Scenario editor
- Multiplayer modes
- Advanced interventions

---

## See Also

- [[README]] - Game concepts overview
- [[dominion-tribal]] - Alternative: character-driven tribal gameplay
- [[ark-monster-collecting]] - Alternative: collection-focused gameplay
- [[../future/README]] - Engine development roadmap
