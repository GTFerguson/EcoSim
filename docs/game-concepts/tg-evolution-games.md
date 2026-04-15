---
title: /tg/ Evolution Games — Collaborative Evolution as Inspiration
created: 2026-04-14
updated: 2026-04-14
status: reference
tags: [game-design, inspiration, collaborative, emergent-narrative, community]
---

# /tg/ Evolution Games

A reference doc on the forum-based evolution and civilization quests that have run on 4chan's `/tg/` (Traditional Games) board since early 2011. Included here as a source of design inspiration for EcoSim — they are a decade-long natural experiment in what makes collaborative evolution engaging.

> [!NOTE]
> This doc draws on (a) the academic literature on emergent narrative and forum-based collaborative storytelling, and (b) primary reading of four specific games — Primordial Central Civ I, Bord Quest, Fortune: Evolution Game, and Cyto — to extract concrete design mechanisms rather than framework-level theory.

---

## What They Are

An "Evo Game" is a forum-hosted collaborative game. The thread-runner (OP) seeds a world with a handful of baseline organisms and an environment. Players reply suggesting mutations or behavioural changes. The OP arbitrates — interpreting votes, rolling dice for environmental events, deciding which suggestions pan out — and posts an update showing the new state of the world. Repeat across many threads.

The format is a specialisation of the broader "quest" genre: *"a piece of interactive fiction, largely played on imageboards and forums, where one person tells a story whose course is decided by the audience"* — *"sort of halfway points between long-form roleplaying and traditional serialized fiction"* (Tier 5: [Fanlore — Quest (writing)](https://fanlore.org/wiki/Quest_(writing))).

Some games stop at the pre-sapient stage. Others push their creatures into sapience and transition into civilization threads, where the same collaborative voting shapes culture, technology, and history. A third mode — the *narrative protagonist quest* — zooms further into one named individual whose choices scale up into civilization-level stakes.

---

## Subgenres

The Fanlore wiki formalises a taxonomy (Tier 5: [Fanlore — Evolution Game](https://fanlore.org/wiki/Evolution_Game)). Primary reading confirms the distinctions matter mechanically, not just in flavour.

### 1. Branching Evolution Game (Sagan-like)

Many parallel lineages, many players working on different species simultaneously. Usually community-drawn: a player posts an edited image of a creature, and that submission becomes canon if the OP accepts it.

**Mechanic signature:** image-editing, one change per post, no back-to-back self-edits, low coordination overhead.

**Examples on /tg/:** Primordial Evolution Game, Species Saga, Fortune: Evolution Game, Snowcone, Swamp Evo.

### 2. Protagonist Lineage (Cyto-like)

One protagonist organism. Players suggest mutations or actions; the OP alone produces all canon art and text. Structured like a serialised MS Paint Adventures story.

**Mechanic signature:** GM-as-sole-illustrator bottleneck → editorial coherence; voting-on-input-only → low contribution barrier; evolution becomes a character arc rather than a tree.

**Origin:** despite the "MSPA-like" descriptor, Cyto (the namesake) originated on the **Sporum (Spore forums)**, not MSPA. Fanlore groups it with NESLife and Sagan 4 as Spore fangames. MSPA-style is a *format* comparison.

### 3. Civilization Quest

After evolution produces sapient species, players adopt tribes and play out their civilization using a structured ruleset (typically a `d20` per action). Inherited biology from the evo phase constrains what tribes can do.

**Mechanic signature:** species "biology cards" as character sheets, per-tribe actions with dice-tiered outcomes, GM-arbitrated consequences.

### 4. Narrative Protagonist Quest ("drawquest")

Narrative zoom from civilization into a single *named individual* inside an evolved species. Players submit joint `d100 + action/name` posts; OP draws hand-rendered panels and arbitrates on best-of-first-N-rolls.

**Mechanic signature:** individual-inside-a-species framing, rapid voting windows that merge joke-votes with serious-votes, player-suggested names locking into canon.

---

## Case Study: Indonesian Gentleman's Three-Tier Pipeline

The single strongest observation from primary reading: **one OP, "Indonesian Gentleman", ran the same world across all three subgenres 2 → 3 → 4**, creating a natural pipeline from sandbox evolution to civilization to individual narrative. This is the subgenre-crossing pattern most worth studying.

### Tier 1 — Branching evolution: Primordial Evolution Game (2011–2015+)

The longest-running /tg/ evo game; 30+ threads across four continents (Tier 5: [1d6chan wiki](https://1d6chan.miraheze.org/wiki/Primordial_Evolution_Game)). Sandbox rules, community-drawn species. Over years of play, the community evolved a stable cast of sapient species: Pusmal, Suzumou, Gorrog, Koburroga, Tentouz, Kwagoa, Hastun, Stusval, etc.

### Tier 2 — Civilization quest: Primordial Central Civilization I (Jan 2013)

Thread [#22486926](https://suptg.thisisnotatrueending.com/archive/22486926/), started 2013-01-09 by Indonesian Gentleman. The OP post:

> *"It's the continuation of the Primordial evo games, where people play out the civilizations that rise from the creatures that were evolved in the evo games. [...] all you need are five things: a tribe/civilization name, a species your tribe is made of, a logo or emblem of your tribe, where your civilization would be located, and a short backstory."*

Voting is strictly structured (`d20` via 4chan's dice namefield):

> *"1 means crit fail, 2 to 5 is failure, 6 to 10 is minor failure, 11 to 15 is minor success, 16 to 19 is success, 20 is the awesome roll. For improvements or inventing something, if you have tried three different times and failed, you will get what you tried to have. It's supposed to be your tribe learning from their mistakes."*

**The evo→civ handoff is a character sheet import.** OP front-loads the thread with ~12 species "biology cards" (posts 22487015, 22487029, 22487053, 22487083, 22487108, 22487130, 22487163...). Each card describes inherited traits that constrain civ actions:

> *"The pusmal are bioengineering hive creatures who evolved underground. The Pusmal queens have unstable DNA, which allows them to control traits in its offspring. [...] The pusmals are unaffected by difficult terrain."* — post 22487015, "1) Pusmal"

> *"They can alter their venom's acidity, usually only releasing the acidic substance while they bore through the shell. The other part of the venom is a neuro-toxin, which causes temporary paralysis, and semi comatose state in the victims. As the venom fades from the victim's body, it will wake up and lash out at those around them, though this makes them hated by their community."* — post 22487029, "2) Suzumou"

Players then adopt tribes by filling the 5-field intake, cite the evo-game wiki as their canonical lore source (e.g., post 22487354: *"Look any time here, everything is wrote down: http://1d4chan.org/wiki/Primordial_Evolution_Game#Hastun"*), and begin rolling `d20`s per action.

Real consequences follow real dice. Example from the thread:

> **Basic tools, molden (Hastun), Rolled 1, post 22487890:** *"They first try to use cutting objects but their strenght lacks to work it as they would. They then find out that they can shape it slowly but precisely by pouring acid concentrated mucus on it. [...] Rolling for woodshaping"*
>
> **OP (Indonesian Gentleman), post 22487973:** *"An accident at the workshop happened! A Tentouz poured too much acidic mucus onto the wood, and it burned a hole through the workshop. Some wooden tools were also liquified in the process."*

A critical-fail roll destroys the workshop and the existing tools — a tangible step backwards. Contrast the tone of the next high-roll player check-in:

> **Cawkasaurus (Stusval), Rolled 19, post 22488534:** *"Without the ability for true shelter she turns to her aching stomach. She sends out the fast stusval along the beach to scavenge for food. In the meanwhile she directs her dumb workers to bring her some of the smaller plants for a snack... &lt;Did I do ok?&gt;"*

Art: hand-drawn continent map (901×1261, "central continent annotated.png") with hand-written region labels; cartoonish 600×400 PNG portraits per species card; shared-sketchbook aesthetic — confident linework, not Paint doodles, but not digital polish either.

### Tier 3 — Narrative protagonist quest: Bord Quest I (Dec 2013)

Thread [#29100009](https://archive.4plebs.org/tg/thread/29100009/), started 2013-12-27 by the same Indonesian Gentleman (not "Switchman" as the 1d6chan wiki tagging suggested). Format shifts again. The OP post is terse:

> *"Hey! Hope you're in the mood for some drawquesting. If you do, sit back and enjoy this one."*

Rules emerge mid-thread. At post 29100092:

> *"One of these sleeper pods open... this is where you come in. >Who is this? I'll need a 1d100, and a name! Taking best of first five rolls!"*

Voting window: best of first five `d100` rolls + suggested name. Ties broken by OP coin flip. Much faster cadence than Central Civ's `d20`-per-action civ model.

**The engagement engine is the naming arbitration.** Three players tie at 98 — posts 29100135 ("Bigga Dickerson"), 29100156 ("Hugo Packageberg"), 29100159 ("Tiny Dingler"). OP rolls d2, gets 2, picks Hugo Packageberg. Player reaction: *"HUGO PACKAGEBERG LIIIIIIVVVVEEEEESSSSS"* (29100209); *"Damn you, Packageberg. The Dickinson clan will not forget this outrage."* (29100223). Then at post 29100679, the ship AI VIRA earnestly greets the player as *"Good morning, Engineer Hugo Packageberg! Have a good bord'n shift!"* — the joke name has locked into canon, voiced straight.

Art: hand-inked "BORD QUEST" title banner with a horizontal starship silhouette over dotted stars; rough hand-drawn/MS-Paint panels for the story beats (cryotubes, VIRA greeting, the lounge area). Pacing starts fast (posts at 02:26, 02:29, 02:31, 03:00, 03:04) then slows as panels require drawing time — OP explicitly posts *"Drawing."* placeholders (29101798) before dropping full panels (next at 04:19).

---

## What Makes These Games Engaging

The academic literature on emergent narrative provides the framework; primary reading provides the concrete mechanisms.

### The five-property framework (framework-level)

Kreminski & Wardrip-Fruin identify five fundamental characteristics of emergent narrative: **coherence, agency, possibility space, uncertainty, and co-authoring** (Tier 3: Kreminski, M. & Wardrip-Fruin, N., 2021, *A Coauthorship-Centric History of Interactive Emergent Narrative*, ICIDS 2021, [PDF](https://mkremins.github.io/publications/IENHistory_ICIDS2021.pdf)). All four games observed satisfy all five, with different mechanisms per subgenre.

Zalka's dissertation on forum-based roleplaying (Tier 3: Zalka, C.V., 2012, *Collaborative Storytelling 2.0*, PhD dissertation, Bowling Green State University, [full text](https://scholarworks.bgsu.edu/acs_diss/93/)) confirms the core draw is **creative collaborative writing**, not winning or simulation fidelity. This is consistent with every game sampled — none has win conditions; Fortune's explicit goal is *"to create an interesting setting"* (Tier 5: [1d6chan Fortune wiki](https://1d6chan.miraheze.org/wiki/Fortune:_Evolution_Game)).

### Concrete mechanisms (observed in threads)

The following are grounded in primary reading of specific threads, not theory. Each is cited.

#### 1. Inherited biology as binding constraint

In Central Civ I, species biology cards act as *character sheets that constrain civ actions*. The Hastun have "superior vision at night", the Pusmal "are unaffected by difficult terrain", the Suzumou's venom "causes temporary paralysis and semi-comatose state" (posts 22487015+). These aren't flavour text — they're binding inputs to OP arbitration. A player attempting a civ action plays *as* their biology, not separately from it.

**Why it works:** players cannot choose to be "strong" or "fast" — they inherit those facts from thousands of posts of prior evolution. The setting resists wish-fulfilment, which makes earned outcomes feel genuinely earned.

#### 2. Dice tiers that fail-narrate, not just fail-block

Indonesian Gentleman's `d20` table gives a failure a *story*: rolled 1 → the workshop burns through because a Tentouz mishandles acidic mucus (post 22487973). The failure advances the narrative instead of stopping it. The "three strikes → you get it anyway" rule (OP post 22486926) turns repeated failure into earned learning.

**Why it works:** failure is never silent dead-air. Every roll is a story beat, win or lose. The table makes the OP's arbitration predictable without removing the OP's creative latitude.

#### 3. Joint joke+serious vote funnel

Bord Quest's "best of first five rolls" window (post 29100092) means jokesters and serious players are competing in the *same* narrow arbitration slot. Hugo Packageberg wins the name not because the thread decides so but because three tied 98s survive the arbitration random. VIRA voices the name earnestly (post 29100679), completing the alchemy.

**Why it works:** communities on imageboards self-select for absurdity; any pure-voting system defaults to lowest-common-denominator chaos. Forcing joke and serious votes through the same constrained window — rewarding *early* arrivers and tie-breaking randomly — means absurdity that *survives* the filter becomes genuine canon. The user in Central Civ asking "&lt;Did I do ok?&gt;" (post 22488534) is the same community — but calmed by the `d20` structure.

#### 4. Chapter boundaries as geological soft-reboots

Fortune: Evolution Game's longevity (6+ years, 2nd-longest-running) relies on OP-triggered geological events that reset niches without erasing lore (Tier 5: [1d6chan — Parting of the Ways](https://1d6chan.miraheze.org/wiki/Parting_of_the_Ways_(Fortune:_Evolution_Game))): the Parting of the Ways fractures the supercontinent into three, forcing divergence; Triumph of Spring recovers from an ice age via volcanic greenhouse effect, *"engendering an absolute explosion of life as organisms no longer struggled against the harsh conditions."*

**Why it works:** sustained worlds need to reset *constraints* without resetting *canon*. Extinction events preserve the setting's history while reopening the possibility space. Fortune's rule that "tool use and sentience are disallowed" during evo phases then *unlocks* via chapter-boundary OP fiat — a simple but effective progression gate.

#### 5. Cross-referenced species as long-horizon payoff

Fortune's wiki fluff explicitly names prey-predator-parasite relationships: the Gardenback is *"like its cousin the Rainbow Blimpie"*; Forest Ticks have *"Grey Wretches and Gardenbacks are their most common hosts"*. Tradeoffs are explicit: the Webspinner *"can rather easily go from a bipedal to a quadrupedal mode [...] However, their jaw power, bone density, and overall takedown ability has suffered."*

**Why it works:** the payoff of reading thread 20 is *recognising callbacks from thread 4*. Sustained worlds reward sustained attention. Evo games where every species is isolated die fast; ones where species reference each other accumulate reader investment.

#### 6. The evo → civ → individual zoom

Indonesian Gentleman's three-tier pipeline is the strongest structural observation. Each tier shrinks the scope and increases the narrative density:

| Tier | Scope | Player action | Art load |
|------|-------|---------------|----------|
| Evolution (Primordial) | World-level, many species | Image-edit mutations | Community |
| Civilization (Central Civ) | Tribe-level, one species each | `d20 + action` text | GM maps + species cards |
| Narrative quest (Bord Quest) | Individual-level, one named character | `d100 + action/name` text | GM hand-drawn panels |

**Why it works:** each tier uses the *output* of the previous tier as its constraint layer. You can't start at tier 3; Hugo Packageberg is only meaningful because the Bord evolved in Primordial and built a civilization in (unseen) Bord civ threads. The whole pipeline is playable because each transition is a structural zoom, not a reset.

**The Dwarf Fortress precedent.** The /tg/ pipeline independently arrived at the same structure Dwarf Fortress has shipped as its canonical design: world-gen simulates deep history, then **Fortress mode** plays a civilization on the generated world, and **Adventurer mode** plays a single named character on the *same persistent world state* — your abandoned fortress becomes explorable ruins for your adventurer; the historical figures from world-gen become NPCs the adventurer can seek out. DF validates the pattern outside the forum-quest context: the pipeline is not a community-game quirk, it is the canonical way to make deep-time simulation matter for moment-to-moment play. The two precedents differ in how tier transitions are initiated — DF uses explicit mode switches the player triggers, Indonesian Gentleman uses OP-launched new threads — but the structural spine is identical: one persistent world, multiple zoom levels, shared state.

### What Cyto adds (framework-only, no primary reading)

Cyto could not be read directly — Sporum shut down 2019, the community mirror at [forum.spore.kylenanakdewa.com](http://forum.spore.kylenanakdewa.com/jforum/forums/list.html) exists but wasn't traversed in this research pass. From Fanlore's taxonomy:

- The GM-as-sole-illustrator bottleneck forces editorial coherence across the entire game — one style, one voice.
- Single-protagonist framing turns evolution into a character arc, not a taxonomic tree.
- Voting-on-mutations-only lowers the contribution barrier compared to games where players draft full species pages.

These are plausible but uncited design observations and should be verified by primary reading before being load-bearing.

---

## Implications for EcoSim

Each item below maps a *concretely observed* mechanism to a feature EcoSim's engine could plausibly support. These are directions, not commitments. Items marked (observed) are grounded in primary thread reading; (framework) items are inferred from the literature.

| Mechanism (source) | EcoSim translation |
|-------------------|--------------------|
| Inherited biology as binding constraint *(observed: Central Civ I #22486926)* | "Species sheet" UI — when a lineage reaches sapience, freeze current genotype as a civ character sheet that constrains civ-level actions |
| Dice tiers that fail-narrate *(observed: Central Civ I #22487973)* | Tiered outcome system for player-initiated events — every intervention succeeds, partially succeeds, fails-safely, or fails-catastrophically with a story beat |
| Joint joke+serious vote funnel *(observed: Bord Quest #29100092)* | Spectator mode where viewers submit `d100 + action` suggestions; engine picks best-of-first-N on a timer; joke names that survive arbitration lock into the lineage's canon |
| Chapter boundaries as geological soft-reboots *(observed: Fortune wiki)* | Scripted world-reset events — volcanic eruptions, glaciations, asteroid impacts — that reopen the possibility space without erasing lineage history |
| Cross-referenced species long-horizon payoff *(observed: Fortune wiki)* | Auto-chronicle view that surfaces prey/predator/symbiosis callbacks between lineages across long runs |
| Evo → civ → individual zoom *(observed: Indonesian Gentleman pipeline)* | Three gameplay modes sharing one world: sandbox sim → tribal civ mode when sapience emerges → character-narrative mode for named individuals |
| Character-sheet import at tier transitions *(observed: Central Civ I)* | When a tribe transitions from sandbox → civ mode, the genotype becomes a locked "species biology card" exposing inherited traits as action modifiers |
| Possibility space from under-constrained starts *(framework: Kreminski 2021)* | Scenario seeds with deliberately minimal starting species and rich environments (cf. [[genesis-god-sim]]) |
| Editorial coherence via single illustrator *(framework: Cyto/Fanlore)* | Consistent engine-rendered visuals across all events — no user-drawn assets mixed with engine output |
| Co-authoring as primary draw *(framework: Kreminski 2021, Zalka 2012)* | Multiplayer/spectator as first-class — every played world should be shareable and extensible by others |

**The strongest single recommendation:** build the **evo → civ → individual zoom** as three modes sharing one persistent world, with tier transitions that import the lower tier's output as the higher tier's constraint layer. This is the pattern Indonesian Gentleman's pipeline exhibits across Primordial, Central Civ, and Bord Quest, and it is the same structural spine Dwarf Fortress has shipped as its canonical design — Fortress mode plays a civilization on a generated world while Adventurer mode plays a single character on the same persistent world state, with the player's abandoned fortresses becoming explorable ruins for their adventurers. EcoSim has explicit DF philosophical influence, so this pattern is already aligned with the project's direction. It is the structural spine that lets deep-time evolution matter for moment-to-moment narrative play, rather than being a separate upstream activity.

---

## Notable Games

### Primordial Evolution Game (2011–ongoing)

Longest-running /tg/ evo game. 30+ threads across four continents, plus civ quests (Central Civ I–IV, Eastern Civ I–V, Southern Civ I–V) and side quests (Bord Quest, Puruu Quest). Run by Indonesian Gentleman. Canonical source of the three-tier pipeline.

- Wiki: [1d6chan — Primordial Evolution Game](https://1d6chan.miraheze.org/wiki/Primordial_Evolution_Game)
- Central Civ I: [suptg #22486926](https://suptg.thisisnotatrueending.com/archive/22486926/)
- Bord Quest I: [4plebs #29100009](https://archive.4plebs.org/tg/thread/29100009/)

### Fortune: Evolution Game (2012–2018+)

2nd-longest-running evo game. FortuneHost was originally a Primordial player (running the Mohu'awane species) who spun his own game off. Rules-light branching format with regional sub-threads. Chapter-based geological upheavals drive progression.

- Wiki: [1d6chan — Fortune: Evolution Game](https://1d6chan.miraheze.org/wiki/Fortune:_Evolution_Game)
- [Parting of the Ways](https://1d6chan.miraheze.org/wiki/Parting_of_the_Ways_(Fortune:_Evolution_Game)) — chapter 6 continental split
- [Triumph of Spring](https://1d6chan.miraheze.org/wiki/Triumph_of_Spring_(Fortune:_Evolution_Game)) — post-ice-age recovery

### Species Saga (concluded)

A more 'realistic' Evo Game run by GM *ecologyfag*. Seeded with the amphibious geckolike Gilim, the insectlike Deka, and the fishlike Feesh; later joined by the Shreemp (which migrated into the Primordial series). Branching sandbox format — community members edit a sprite and repost.

- Thread 1: [4plebs #18318543](https://archive.4plebs.org/tg/thread/18318543/) — *"The saga of a species. (without silly lightning effects)"*
- Thread 2: [4plebs #18332312](https://archive.4plebs.org/tg/thread/18332312/) — *"Evolving onwards and feeshwards"*
- 4plebs is the only mirror still serving images for these 2012-era threads.

### Cyto (Sporum, date unknown)

The canonical Cyto-like protagonist-lineage game. Despite frequent "MSPA-like" descriptions, Cyto originated on the Sporum (Spore forums), not MSPA Forums. Primary source not retrieved in this research pass. Only framework-level discussion available.

- Taxonomy: [Fanlore — Evolution Game](https://fanlore.org/wiki/Evolution_Game) (see "Cyto-Like" section)
- Archive (not traversed): [forum.spore.kylenanakdewa.com](http://forum.spore.kylenanakdewa.com/jforum/forums/list.html)

### Threadjacker Evolution Game, Mollusk Quest, and others

Surveyed but not deep-read. Mollusk Quest (by *fromAether*) is sprite-edit format — players edit an image of a slug and submit mutations. Extinction-event threads are a notable variant. See the [1d6chan Evolution Games index](https://1d6chan.miraheze.org/wiki/Evolution_Games) for the full catalogue.

---

## Sources

### Tier 3 — Peer-reviewed research / dissertations

- Kreminski, M. & Wardrip-Fruin, N. (2021). *A Coauthorship-Centric History of Interactive Emergent Narrative*. ICIDS 2021. [PDF](https://mkremins.github.io/publications/IENHistory_ICIDS2021.pdf) — source for the five-property framework.
- Zalka, C.V. (2012). *Collaborative Storytelling 2.0: A framework for studying forum-based roleplaying games*. PhD dissertation, Bowling Green State University. [Full text](https://scholarworks.bgsu.edu/acs_diss/93/) — characterises forum-based collaborative fiction as creative writing practice.

### Tier 4 — Academic framework / review articles

- *World-Sharing Rhetoric: A Framework for Inviting Participation in Symbolic Worlds* (2025). *Topoi*, Springer. [Link](https://link.springer.com/article/10.1007/s11245-025-10327-9).
- Ryan, J. et al. *Open Design Challenges for Interactive Emergent Narrative*. UCSC Expressive Intelligence Studio. [PDF](https://eis.ucsc.edu/papers/ryanEtAl_OpenDesignChallengesForInteractiveEmergentNarrative.pdf).

### Tier 5 — Community wikis, archives, and primary threads

- [Evolution Games — 2d4chan wiki](https://2d4chan.org/wiki/Evolution_Games)
- [Evolution Games — 1d6chan wiki (successor)](https://1d6chan.miraheze.org/wiki/Evolution_Games) — full catalogue
- [Fanlore — Evolution Game](https://fanlore.org/wiki/Evolution_Game) — subgenre taxonomy
- [Fanlore — Quest (writing)](https://fanlore.org/wiki/Quest_(writing))
- [1d6chan — Primordial Evolution Game](https://1d6chan.miraheze.org/wiki/Primordial_Evolution_Game)
- [1d6chan — Fortune: Evolution Game](https://1d6chan.miraheze.org/wiki/Fortune:_Evolution_Game)
- [4plebs /tg/ archive](https://archive.4plebs.org/tg/) — image mirror for 2012+ threads
- [suptg /tg/ archive](https://suptg.thisisnotatrueending.com/) — text-first mirror, some 403s on old threads
- [desuarchive.org /tg/](https://desuarchive.org/tg/) — alternative text mirror

**Primary threads read in depth for this doc:**

- [Primordial Central Civilization I #22486926](https://suptg.thisisnotatrueending.com/archive/22486926/) — Indonesian Gentleman, 2013-01-09
- [Bord Quest I #29100009 — "The Bord Awaken"](https://archive.4plebs.org/tg/thread/29100009/) — Indonesian Gentleman, 2013-12-27

> [!WARNING]
> Evidence tiers are uneven. Kreminski & Wardrip-Fruin (Tier 3) is load-bearing for the framework claims. The concrete mechanism observations are grounded in primary reading of two threads (Central Civ I and Bord Quest I) plus secondary-source reading of Fortune and Cyto. The "Implications for EcoSim" table distinguishes *(observed)* from *(framework)* items accordingly. Cyto specifically is framework-only and any design decisions drawing from it should verify by primary reading first.

---

## See Also

- [[README]] — game concepts index
- [[genesis-god-sim]] — closest existing concept; the *possibility space from under-constrained starts* mechanism is a direct fit
- [[dominion-tribal]] — the *character-sheet import* mechanism is the civ-mode handoff for this concept
- [[ark-monster-collecting]] — the *joint joke+serious vote funnel* could inform spectator/multiplayer features
