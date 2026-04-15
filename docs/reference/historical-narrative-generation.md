---
title: Historical Narrative Generation in Simulation Games
created: 2026-04-14
updated: 2026-04-14
status: active
tags: [reference, narrative-generation, story-sifting, chronicle, simulation, emergent-narrative]
---

# Historical Narrative Generation in Simulation Games

Evidence base for the problem of **distilling millions of raw simulation events into a readable historical narrative** — a "chronicle" or "history book" that a player can engage with as a coherent story rather than a stream of data. Covers academic research on *story sifting* and *curationism*, plus the architectural approaches taken by six simulation games that have actually shipped chronicle-like systems.

This doc informs the chronicle architecture proposed in [[../future/infrastructure/chronicle-and-species-registry]].

---

## The core problem

A rich simulation produces tens of thousands to millions of events per run. Almost none of them are individually interesting. A player who wants to read the world's history wants tens to hundreds of events — the ones that form a coherent narrative shape, surface tradeoffs, or connect to other events in meaningful ways. The gap between what the simulation knows and what a reader wants is the **story recognition problem** (Tier 2: Kreminski, Dickinson, Wardrip-Fruin & Mateas, 2019, *Felt: A Simple Story Sifter*, ICIDS 2019, [PDF](https://mkremins.github.io/publications/Felt_SimpleStorySifter.pdf)).

Four architectural approaches have been shipped to production, and the academic literature formalises one of them (simulate-then-filter). The rest of this doc surveys them.

---

## Architectural approaches (game practice)

Game practice converges on four distinct strategies. Naming is informal but the distinctions are clean.

### 1. Simulate-then-filter — Dwarf Fortress

Log everything the simulation produces as typed event records with stable entity IDs. Filter at view time. The simulation *is* the log.

**Mechanism.** Dwarf Fortress's world-gen runs phases in a fixed order (elevation → temperature → rivers → lakes → vegetation → verify → wildlife → "Recounting legends"), with the legends pass being the historical-events generator. Events are strongly-typed records referencing historical figures, sites, entities, and artifacts by numeric ID. The XML dump format exposes 14 top-level collections (historical_figures, historical_events, historical_event_collections, entities, sites, artifacts, written_content, etc.). Each event is a structured record like `hf_died { hfid, slayer_hfid, site_id }` or `hf_entity_link_added { hfid, entity_id, link_type }`. Raw events are rendered to English at view time by Legends mode, not at emission time (Tier 5: Dwarf Fortress Wiki, *XML dump*, [link](https://dwarffortresswiki.org/index.php/DF2014:XML_dump)).

**Significance filtering.** DF runs an explicit **culling pass** after history generation: a historical figure is culled if it died early, had no offspring, and never killed a named creature. Culled figures drop out of Legends mode and appear as "an unknown creature" to surviving characters. Culling aggressiveness is player-configurable as a CPU/memory tradeoff (Tier 5: Dwarf Fortress Wiki, *Advanced world generation*, [link](https://dwarffortresswiki.org/index.php/Advanced_world_generation)). Importance is emergent: a figure matters because it interacted with already-important entities, not because an authored score flagged it.

**Design philosophy.** Tarn Adams' 2016 GDC talk with Tanya Short ("Practices in Procedural Generation") described a **creation-myth generator** that runs *before* history — it defines gods, afterlives, and magic, so downstream generators produce a coherent setting. Tarn's stated preference is simulation-first: events happen because rules fire, not because a narrative scheduler wants drama. Origin story: Tarn and Zach wrote different event "chapters" they wanted to see in Armok, then implemented the underlying simulation that would produce them (Tier 5: Adams & Short, 2016, *Practices in Procedural Generation*, GDC Vault, [link](https://www.gdcvault.com/play/1023372/Practices-in-Procedural)).

**Player surface.** Legends mode — a browsable index of maps, civilizations, sites, historical figures, artifacts, written works, and a chronological event log. Third-party tools (LegendsViewer by Kromtec) parse `legends.xml` into cross-linked web views (Tier 5: Kromtec, *LegendsViewer*, [GitHub](https://github.com/Kromtec/LegendsViewer)).

### 2. Author-then-fire — Crusader Kings, RimWorld, Wildermyth

Hand-author event templates. Schedule them via triggers, mean-time-to-happen, or live scheduler. The significance filter is the author.

**Crusader Kings II Chronicle.** CK2 had an explicit **Chronicle** feature — an Anglo-Saxon-Chronicle-styled per-dynasty history book populated by **chronicle events** hand-scripted in Paradox's data-driven event system. Each chronicle event is explicit script: `if X fires, append this localized sentence keyed by character/title/date`. Events declared in `.txt` files with triggers, conditions, mean-time-to-happen, or on-action hooks. Significance is purely authored — a designer decides which gameplay event types deserve a chronicle line. The inverse of DF's "simulate-first, filter-later". CK3 dropped the Chronicle feature, which the community filled with mods and save-scrapers (Tier 5: CK2 Wiki, *Chronicle events*, [link](https://ck2.paradoxwikis.com/Chronicle_events); Tier 5: Paradox, 2020, *CK3 Dev Diary #30 — Event Scripting*, [forum](https://forum.paradoxplaza.com/forum/threads/crusader-kings-3-dev-diary-30-event-scripting.1397140/)).

**RimWorld AI Storyteller.** RimWorld calls itself a *story generator*, not a history generator — there is no prior world history. Narrative is produced live by the AI Storyteller (Cassandra Classic / Phoebe Chillax / Randy Random). Storytellers tick every ~1000 game ticks and roll whether to fire an `IncidentWorker_*` event. Cassandra and Phoebe split incidents into categories (ThreatBig, ThreatSmall, Misc) and pick a category first, then weighted-pick an incident by `baseChance`. Randy skips categories entirely — one pool, weighted random, plus raid-points variance. A `PopulationIntent` stat biases colonist-adding incidents up when population is low (Tier 5: RimWorld Wiki, *AI Storytellers*, [link](https://rimworldwiki.com/wiki/AI_Storytellers)).

**The key RimWorld design decision — hard split between sim noise and story.** Tynan Sylvester's 2017 GDC talk and his 2009 blog post "Mutual Storytellers" (predates RimWorld) argue that the designer's job is to build mechanics that reliably yield dramatic moments, not to write them. Only **incidents** go through the Storyteller. Pawn moods, chores, and ambient sim never become "narrative" — the player narrativises the feed themselves. This is the opposite of DF, which logs everything and filters at view time (Tier 5: Sylvester, 2017, *RimWorld: Contrarian, Ridiculous, and Impossible Game Design Methods*, GDC Vault, [link](https://www.gdcvault.com/play/1024232/-RimWorld-Contrarian-Ridiculous-and); Tier 5: Sylvester, 2009, *Mutual Storytellers*, [blog](https://tynansylvester.com/2009/01/open-discussion-mutual-storytellers/)).

**Wildermyth hooks as a constraint solver.** Wildermyth layers procedural content on a fixed five-chapter campaign skeleton. Heroes carry 3 **hooks** (personality aspects). Every event declares targeting criteria: required personality stats, required hooks, and inter-hero relationship requirements. At each opportunity point the engine scans the event stable, filters to events whose cast requirements the current party can fill, then casts heroes into roles. Hook quests are optional side arcs gated on a specific hook + helper hero; resolving one clears the hook, grants a promotion, extends retirement. Designer commentary: they *tried* procedural plots and abandoned them — a cohesive story needs to build to one destination — so they fixed the macro arc and proc-gen only the micro (Tier 5: Wildermyth Wiki, *Hook*, [link](https://wildermyth.com/wiki/Hook); Tier 5: Austin et al., *AIAS Game Maker's Notebook*, *Balancing Procedural and Intimate Storytelling in Wildermyth*, [podcast](https://gamemakersnotebook.libsyn.com/balancing-procedural-and-intimate-storytelling-in-wildermyth)).

### 3. Symptoms-not-simulation — Caves of Qud

Don't simulate the past. Generate the *artifacts* of the past — books, engravings, ruins, rumours, secrets — and let the player reconstruct causation from them. Explicitly rejects the DF approach.

**Mechanism.** Qud's **sultan history generation** template-drives each of the first five sultans: start with an origin event, draw ~8 core life events from a pool of 17 types, require the events to touch the 2–3 historic regions added during world-gen for that sultanate period, then finish with a death event. The sixth sultan (Resheph) is hand-authored. Events aren't raw sim output — they are procedural *stories* generated about entities that were never simulated living lives. Procedural text rendering draws from a ~40k-word corpus with grammar rules to preserve voice (Tier 5: Grinblat & Bucklew, 2017, *Subverting Historical Cause & Effect: Generation of Mythic Biographies in Caves of Qud*, PCG Workshop, [PDF](https://www.freeholdgames.com/papers/Generation_of_mythic_biographies_in_Cavesofqud.pdf)).

**Design philosophy.** Grinblat's chapter "Generating Histories" in *Procedural Storytelling in Game Design* (Routledge, 2019) argues that simulating a full history is usually wasteful and lossy — the sim outputs data, but players need *stories*, and the translation layer is expensive. His alternative: generate the symptoms (documents, ruins, rumours) directly, and let players reconstruct the cause. The Water Ritual is the social mechanic that unblocks lore exchange with faction NPCs; it is both gameplay and chronicle interface (Tier 5: Grinblat, 2019, *Generating Histories*, ch. 18 in *Procedural Storytelling in Game Design*, [Routledge](https://www.taylorfrancis.com/chapters/edit/10.1201/9780429488337-18/generating-histories-jason-grinblat); Tier 5: Grinblat, 2017, Roguelike Celebration talk, [video page](https://www.gamedeveloper.com/design/video-procedurally-generating-history-in-i-caves-of-qud-i-)).

**Player surface.** Lore is **hidden and discovered**, not presented up front. Sultan-history events are "secrets" unlocked by examining shrines, engraved objects, books, or by trading secrets during the Water Ritual with NPCs. This inverts the usual chronicle model: there is no Legends viewer showing the world's history; there is a player who has found 47 of 213 secrets (Tier 5: Caves of Qud Wiki, *Sultan histories*, [link](https://wiki.cavesofqud.com/wiki/Sultan_histories)).

### 4. Dependency-chain qualitative procgen — Ultima Ratio Regum

Generate facts in layered dependency order, where each later layer references the previous. Significance emerges from reachability via the reference graph — a fact is significant if it is anchored by something the player can encounter.

**Mechanism.** URR is built around "qualitative procedural generation" with "chains of meaning": later generators consume the output of earlier ones, so nothing exists in isolation. Generation starts macro (solar systems → planets → continents → 250×250 overworld → ~2.5M sub-tiles), with history, geography, and political ideology generated first and feeding dialects, remedies, religious practices, riddles, and book text. The player's primary interface to history in v0.11 is procedurally-generated books, riddles, journals, and engraved clues — not a timeline view (Tier 5: Johnson, 2021, *The 10-Year Journey of Ultima Ratio Regum*, Game Developer, [link](https://www.gamedeveloper.com/design/the-10-year-journey-of-ultima-ratio-regum-the-culture-generating-roguelike)).

**Significance as reachability.** A fact is significant iff it is referenced by a downstream generator (a riddle, a book, an engraving) that the player can encounter. No runtime salience scoring; salience = "is this reachable via an investigation chain?" Johnson's retrospective describes an explicit rejection of "stamp-collecting" procgen (lots of unrelated random items) in favour of relational procgen where every generated object is anchored to at least one other.

**Design philosophy.** The player role is explicitly a researcher/investigator. Johnson is an academic — PhD in science-and-technology studies, Senior Lecturer in game studies at U. of Sydney — and his practitioner writing is unusually explicit about underlying theory (Tier 5: Johnson, URR dev blog *Riddle Generation* series, [site](https://www.markrjohnsongames.com/games/ultima-ratio-regum/)).

---

## Academic framework: story sifting

The academic literature — largely from Max Kreminski and collaborators at UCSC Expressive Intelligence Studio — formalises the **simulate-then-filter** approach as a discipline called **story sifting**. Kreminski's group is the single most relevant academic reference for the chronicle problem.

### Core concepts

**Story recognition** is the formal framing: given the "vast chronicles of events generated by interactive emergent narrative simulations," identify the event sequences "of particular narrative interest or significance." Key terms:

- **Story sifter** — the system that performs sifting
- **Chronicle** — the full raw event log
- **Sifting pattern** — a logic-pattern query (in Felt's case, Datalog-style logic variables over a DataScript event database) that matches an ordered subset of events, allowing unmatched events to be interspersed
- **Microstory** — a recognised narratively-potent subsequence extracted from the chronicle
- **Narrative affordance / storyful situation** — a pattern a reader could plausibly narrativise

(Tier 2: Kreminski et al., 2019, *Felt: A Simple Story Sifter*, ICIDS 2019, [PDF](https://mkremins.github.io/publications/Felt_SimpleStorySifter.pdf))

### Kreminski's body of work

- **Felt** (Tier 2: Kreminski, Dickinson, Wardrip-Fruin & Mateas, 2019, ICIDS 2019, [PDF](https://mkremins.github.io/publications/Felt_SimpleStorySifter.pdf)) — the original sifter: a Datalog-flavoured DSL for logic-pattern queries over a simulation event database. [GitHub](https://github.com/mkremins/felt).
- **Toward Example-Driven Program Synthesis of Story Sifting Patterns** (Tier 4: Kreminski, Wardrip-Fruin & Mateas, 2020, INT @ AIIDE 2020, [PDF](https://ceur-ws.org/Vol-2862/paper18.pdf)) — inductive logic programming to synthesise sifting patterns from a few user-supplied example event sequences.
- **Winnow: A Domain-Specific Language for Incremental Story Sifting** (Tier 2: Kreminski, Dickinson & Mateas, 2021, AIIDE 2021, [PDF](https://cdn.aaai.org/ojs/18903/18903-52-22669-1-2-20211004.pdf)) — addresses Felt's retrospective-only limit by tracking partial matches as events stream in, so in-progress stories can be flagged before they complete. [GitHub](https://github.com/mkremins/winnow).
- **Authoring for Story Sifters** (Tier 4: Kreminski, Wardrip-Fruin & Mateas, 2022, in Hargood et al. eds. *The Authoring Problem*, Springer HCI, [PDF](https://mkremins.github.io/publications/AuthoringSifters_TAP.pdf)) — introduces **sifting heuristics**: high-level encodings of "what makes a compelling story" (reversal, symmetry, character arc completion) layered on top of low-level patterns.
- **Select the Unexpected: A Statistical Heuristic for Story Sifting** (Tier 2: Kreminski, Dickinson, Wardrip-Fruin & Mateas, 2022, ICIDS 2022, [PDF](https://mkremins.github.io/publications/StU_ICIDS2022.pdf)) — scores candidate microstories by **statistical rarity**: unusual event configurations are proxies for narrative significance. This is the clearest candidate for an automated significance metric.
- **Why Are We Like This? (WAWLT)** (Tier 2: Kreminski, Dickinson, Mateas & Wardrip-Fruin, 2020, ELO 2020 and FDG 2020) — a co-creative storytelling game that operationalises Felt over a live social simulation.
- **Gossamer: Toward Better Gossip Simulation in Emergent Narrative Systems** (Tier 2: Kreminski et al., 2023, IEEE CoG 2023, [PDF](https://mkremins.github.io/publications/Gossamer_CoG2023.pdf)) — characters use Felt-style queries over their own memory store to retrieve retellable events.
- **A Coauthorship-Centric History of Interactive Emergent Narrative** (Tier 2: Kreminski & Mateas, 2021, ICIDS 2021, [PDF](https://mkremins.github.io/publications/IENHistory_ICIDS2021.pdf)) — the historiographical overview; traces story sifting back through Aylett, Louchart, and Ryan's curationism.
- **Sketching a Map of the Storylets Design Space** (Tier 2: Kreminski & Wardrip-Fruin, 2018, ICIDS 2018, [PDF](https://mkremins.github.io/publications/Storylets_SketchingAMap.pdf)) — academic survey of storylet architectures (content selection, preconditions, repeatability).

Kreminski's publications index lives at [mkremins.github.io/publications](https://mkremins.github.io/publications/).

### Related academic work

- **Curationist emergent narrative** (Tier 3: Ryan, 2018, PhD diss. UCSC, *Curating Simulated Storyworlds*, [eScholarship](https://escholarship.org/uc/item/1340j5h2)) — the foundational argument that simulation output is *raw material* and a separate **curation** pass constructs the narrative artifact. Ryan's thesis is "recounting, not invention": the sim produces a history; humans (or algorithms) curate it into a story. Engines described: World, Talk of the Town, Hennepin. Games: Bad News, Sheldon County.
- **Storylets / Quality-Based Narrative** (Tier 5: Short, 2016, *Beyond Branching: Quality-Based and Salience-Based Narrative Structures*, [blog](https://emshort.blog/2016/04/12/beyond-branching-quality-based-and-salience-based-narrative-structures/); Tier 5: Short, 2019, *Storylets: You Want Them*, [blog](https://emshort.blog/2019/11/29/storylets-you-want-them/)) — Failbetter Games coined the term; narrative content is authored as discrete chunks gated by numeric qualities and selected by salience. Practitioner-dominant but cited throughout the academic literature.
- **Interactive Emergent Narrative / FearNot!** (Tier 2: Aylett, Louchart, Dias, Paiva & Vala, 2005, *FearNot! – An Experiment in Emergent Narrative*, TIDSE 2005, LNCS 3814, [Springer](https://link.springer.com/chapter/10.1007/11550617_26)) — the prior generation: narrative emerges from affective-agent interaction without explicit recognition.
- **Ceptre** (Tier 2: Martens, 2015, *Ceptre: A Language for Modeling Generative Interactive Systems*, AIIDE 2015, [PDF](https://www.cs.cmu.edu/~cmartens/ceptre.pdf)) — linear-logic forward-chaining DSL for simulation where the proof trace *is* the narrative causal graph. Natural fit for extracting causally-connected event subgraphs.
- **Ensemble / Comme il Faut** (Tier 2: McCoy et al., 2011, *Prom Week: Social Physics as Gameplay*, FDG 2011, [PDF](http://www.ben-samuel.com/wp-content/uploads/2015/09/FDG-2011-Prom-Week-Social-Physics-as-Gameplay.pdf); Tier 2: Samuel et al., 2015, *The Ensemble Engine*, FDG 2015) — rules-based social-state simulation whose event log is a natural substrate for sifting.
- **Awash: Prospective Story Sifting Intervention** (Tier 2: Clothier & Millard, 2023, ICIDS 2023, [PDF](https://eprints.soton.ac.uk/482864/1/Awash.pdf)) — uses an incremental sifter as input to a drama manager that nudges the simulation toward completing in-progress stories. Builds directly on Winnow.
- **Crowdsourced plot graphs** (Tier 2: Li, Lee-Urban, Johnston & Riedl, 2013, *Story Generation with Crowdsourced Plot Graphs*, AAAI 2013, [PDF](https://faculty.cc.gatech.edu/~riedl/pubs/aaai13.pdf)) — less directly applicable but cited as a comparator: authored/learned plot graphs decide whether an action sequence is narratively coherent.

---

## Techniques catalogue

Concrete technical terms harvested from the literature, usable as building blocks:

| Technique | Source | Description |
|-----------|--------|-------------|
| **Typed event records with stable entity IDs** | DF `legends.xml`, CK event scripts | Structured records referencing entities by numeric ID, not free-text logs. Universal prerequisite for querying a chronicle years later. |
| **Post-hoc culling pass** | DF | After history generation, drop entities that never interacted with anything significant. Cheap bulk filter, applied before any rendering. |
| **Sifting patterns over an event database** | Felt, Winnow | Datalog/DataScript logic-variable queries with gap-allowed ordered matching. Patterns match event subsequences, not consecutive runs. |
| **Incremental pattern matching** | Winnow | Track partial matches as events stream in so in-progress stories can be flagged before they complete. |
| **Inductive pattern synthesis from examples** | Kreminski 2020 | Learn sifting patterns from labelled microstories rather than authoring them by hand. Reduces authoring cost. |
| **Statistical rarity / unexpectedness scoring** | Select the Unexpected | Model the marginal probability of an event configuration in the simulation's run distribution; surface microstories whose configuration is improbable. The clearest automated significance metric. |
| **Sifting heuristics** | Authoring for Story Sifters | Higher-level aesthetic filters (reversal, symmetry, character arc completion) layered atop low-level patterns. |
| **Reader curation / authorial framing** | Ryan 2018 | A human editorial step that selects and frames matched microstories into a media artifact. Central to curationism; still largely manual in production systems. |
| **Memory-scoped sifting** | Gossamer | Each character has their own event memory store; Felt-style queries run per-character, turning gossip and recall into sifting problems. |
| **Causal-chain extraction from linear-logic proofs** | Ceptre | The resource-consumption trace implicitly contains causally necessary subsequences; extract subgraphs directly from the proof. |
| **Hard split: ambient sim vs story events** | RimWorld | Refuse to record boring events at all. Only "incidents" go through the storyteller; everything else is ambient and unnarrativised. |
| **Authored chronicle event hooks** | CK2 | Hand-written script: `if gameplay event X fires, append this localized chronicle line`. High coverage limit (bounded by authoring budget) but zero ambiguity. |
| **Template-driven biography generation** | Caves of Qud | Don't simulate the past; generate structured biography templates filled with world-gen facts, rendered via a grammar corpus. |
| **Dependency-chain generation** | URR | Generate facts in layered dependency order. Significance = reachability via downstream references. |
| **Hooks as constraint solver targets** | Wildermyth | Events declare cast requirements (personality, hooks, relationships); the runtime casts current characters into roles satisfying constraints. |
| **Chronicle UI decoupled from storage format** | DF Legends, CK2 Chronicle book, Qud secrets | The player view is a rendering of structured data, not the data itself. Decoupling storage from rendering is universal across all shipped systems. |

---

## Gaps and open problems

Explicit pain points named in the academic literature:

- **Authoring cost is the largest named bottleneck.** Sifting patterns are hand-authored per-simulation and don't port between worlds (Tier 4: Kreminski et al., 2022, *Authoring for Story Sifters*). Every new simulation needs its own pattern library.
- **Retrospective-only matching.** Winnow addresses it partially, but real-time sifting over large simulations remains expensive.
- **No domain-general significance metric.** "Select the Unexpected" proposes rarity as a proxy, but the paper frames it as a heuristic, not a solution. Narrative significance is not reducible to statistical surprise.
- **Scalability of the event store.** All named academic systems assume tens of thousands to low millions of events. Dwarf Fortress-scale chronicles (Ryan's dissertation discusses this) are still treated as an open engineering problem.
- **Authorship/participation ambiguity** (Tier 2: Kreminski & Mateas, 2021). The literature hasn't cleanly separated "sifting to tell the player a story the sim generated" from "sifting to give the player authorial affordances". The first framing — which is what a history-book chronicle needs — is *less* well-studied than the second.
- **Curation as a separate media step** (Tier 3: Ryan, 2018) remains largely manual. Bad News and Sheldon County both rely on a human performer/editor. Fully automatic chronicle-to-history pipelines are basically unsolved.

---

## Cross-approach comparison

Four answers to the same problem. Each has an authoring-cost / emergent-coverage tradeoff.

| Approach | Authoring cost | Coverage of emergent events | Significance filter | Example |
|----------|----------------|---------------------------|---------------------|---------|
| Simulate-then-filter | Low (filters are generic) | Complete (everything is logged) | Cheap culling + optional story sifters | Dwarf Fortress |
| Author-then-fire | High (per-event scripting) | Bounded by authoring budget | The author is the filter | CK2 Chronicle, RimWorld, Wildermyth |
| Symptoms-not-simulation | Medium (biography templates + grammar) | N/A — history is generated, not logged | The template is the filter | Caves of Qud |
| Dependency-chain | Medium (generators + reference rules) | Implicit (only generated facts exist) | Reachability via reference graph | Ultima Ratio Regum |

Kreminski's story-sifting framework is specifically a formalisation of approach 1 (simulate-then-filter). The other three approaches are well-documented in practitioner talks but less formalised academically.

---

## Provenance and caveats

- Academic paper claims (Kreminski et al., Ryan, Aylett et al., Martens, McCoy et al., Clothier & Millard, Li et al.) are sourced via WebSearch snippets and Kreminski's publications page (scraped via scout-browse). Full PDFs were not read end-to-end; summaries are based on abstracts and secondary descriptions. For any claim that becomes load-bearing in an implementation decision, the primary PDF should be read first.
- Game practice claims (DF, CK, RimWorld, Qud, Wildermyth, URR) are sourced from developer talks, wikis, blog posts, and third-party reverse-engineering projects. GDC talks and author blog posts are authoritative; wikis are good but can lag. CK3 Dev Diary #30 and the Grinblat PCG Workshop PDF were cited via search snippets, not fetched directly.
- Evidence tiers are uneven: Kreminski et al. papers are Tier 2 (peer-reviewed conference), Ryan 2018 is Tier 3 (dissertation), game practice sources are Tier 5 (practitioner, no peer review).

---

## See also

- [[../future/infrastructure/chronicle-and-species-registry]] — consumes this research; the plan doc translating the findings into EcoSim-specific infrastructure
- [[../game-concepts/tg-evolution-games]] — parallel community-game research (Primordial, Central Civ, Bord Quest, Fortune) that independently validates the "readable history is the payoff" framing
- [[../game-concepts/genesis-god-sim]] — the primary consumer mode of the chronicle system
