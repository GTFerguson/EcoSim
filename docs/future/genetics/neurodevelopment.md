---
title: Neurodevelopment
status: planned
tags: [genetics, cognition, neurodevelopment, brain, culture]
---

# Neurodevelopment

The genetic and architectural design for creature cognition in EcoSim. This plan consumes the research in [[../../reference/creature-cognition-and-social-scaling]] and translates it into concrete design directions for the engine.

**Core design stance.** EcoSim does not attempt to evolve sapient creatures from first principles — that is an unsolved research-frontier problem in artificial life, documented in the reference doc. Instead: EcoSim **emulates cognitive complexity and culture via authored systems with evolved cognitive prerequisites as unlock gates.** A creature's brain structure (which itself evolves across generations) determines which authored behavioural and cultural systems it can access. This is a pragmatic game-design choice, not a simulation-research ambition, and it decouples our timeline from the open research questions.

**Rollout is gradual, feature-driven.** Neurodevelopment is **not** a dedicated phase in the engine roadmap. It is a cross-cutting system that grows incrementally: **individual brain regions are implemented as specific features need them.** This means:

- There is no single "build the whole cognitive architecture" milestone. Divine favour in Genesis v1 ships with whichever regions the favour mechanic actually needs (probably Hippocampus for species memory of divine interventions, Amygdala for reactive responses). Other regions come online later as features depend on them.
- The Dennett tower tiers are not all implemented at launch. v1 probably only needs Skinnerian with a simplified threshold rather than full Popperian forward-modelling. Popperian and Gregorian land when post-v1 features (culture systems, Dominion sapience handoff) require them.
- The three-layer architecture (biochemistry / neural / genetic specification) is a *target*, not a v1 deliverable. The existing behaviour AI stays in place and is incrementally replaced region-by-region as new features demand it.
- Each brain region's first implementation is driven by the feature that first needs it. That feature's spec dictates what the region minimally has to do; later features may extend the region's capabilities further.

This means **no engine phase in `docs/future/README.md` needs a "neurodevelopment phase"** — neurodevelopment work will appear inside whatever phase or feature plan first needs a given region. The architecture described below is the *destination*, not a roadmap item. Which region is built first depends on which feature ships first.

Minimum v1 expectation: enough cognitive substrate for divine favour to reach its sapience-threshold chronicle event. This may be a simplified "proto-Skinnerian" — rudimentary Hippocampus-equivalent memory and chemical reward/punish modulation — rather than the full three-layer architecture. The full Creatures-style architecture is an aspirational destination that the gradual rollout walks toward.

---

## 1. Three-layer cognition architecture

Based on Steve Grand's Creatures architecture (Tier 2: Grand, Cliff & Malhotra, 1997, [Sussex CSRP434](https://www.sussex.ac.uk/informatics/cogslib/reports/csrp/csrp434.pdf)), the creature mind is modelled as three interacting layers rather than as a single behaviour tree or decision graph:

### Layer 1 — Biochemistry modulator

A diffuse chemical layer that sits *above* the neural substrate and modulates it via receptors. Chemicals (hormones, drives, reward/punish signals) are released by events, diffuse, bind to receptors, and bias neural activity without being hardcoded into decision logic. Drives — hunger, cold, pain, loneliness, fear, lust — live here and influence perception and action selection by modulating the neural layer, not by being decision-tree nodes.

**Why this layer exists:** without it, every drive and every emotional state has to be hand-coded into the decision logic. With it, drives are a separate simulation tier that couples into cognition uniformly via receptor genes. It also gives us a natural place to hook **divine-favour effects** from Genesis — player interventions that cause chemical release in target species.

### Layer 2 — Neural substrate

A recurrent neural network organised into functional **lobes** — dedicated sub-networks for specific computational roles. Modelled on the Norn architecture: Perception, Attention, Decision, Concept, Verb, Noun, Stimulus Source, Drive, plus any EcoSim-specific lobes that emerge from design needs. The Concept lobe is where **learned associations live** — it's the primary memory substrate. The Decision lobe implements **Winner-Takes-All** over action candidates, with each Decision neuron gated by a combination of excitatory and inhibitory connections from the Concept lobe.

Reference numbers from the Creatures architecture (for scale — EcoSim's own numbers will differ):

| Lobe | Norn count | Role |
|------|----------:|------|
| Concept | 640 | Learned associations, the main memory store |
| Perception | 112 | Sensory input |
| Attention | 40 | What's being attended to |
| Noun | 40 | Object lexicon |
| Stimulus Source | 40 | Who/what caused it |
| Drive | 16 (13 active) | Internal state drivers |
| Decision, Verb, General Sense | smaller | Action selection, action lexicon, general afferent |
| **Total** | **952** | |

EcoSim's creatures will vary in brain size by genetics, so there is no fixed total; the Norn counts are reference scales.

### Layer 3 — Genetic specification

Genes specify **brain structure, not just scalar parameters.** Individual genes control: lobe sizes, dendrite wiring rules per lobe, biochemistry (which chemicals exist, reactions, receptors, emitters, half-lives), instinct circuits, and life-stage transitions that gate ontogenetic development. Breeding mixes parental specifications with mutation, producing inheritable variation in *cognitive architecture* across generations. This is why an EcoSim lineage can evolve toward more complex cognition — the brain itself is a genetically-specified structure under selection, not a fixed template with tuned knobs.

Concrete capabilities that can evolve structurally (not just numerically):

- Lobe sizes (measured in neurons or dendrite budget)
- Connection density between specific lobe pairs
- Existence of entirely new lobes (emergent via duplication + divergence mutations)
- New chemical receptors or emitters
- New instinct circuits (hardwired behaviour templates triggered by gene presence)

---

## 2. Brain regions as gates for capability

The existing sketch mapped brain regions (PFC, Hippocampus, Amygdala, Basal Ganglia, Cerebellum, Temporal Lobes, Parietal Lobes) to behavioural capabilities. This is still the right framing for player-facing mechanics; it maps onto Layer 2 above.

Updated mapping with research grounding from the reference doc:

| Brain Region          | Affects                              | Small Region                        | Large Region                         | Key Co-Interactions                             | Unlocks (see §4) |
| --------------------- | ------------------------------------ | ----------------------------------- | ------------------------------------ | ----------------------------------------------- | ---------------- |
| **Prefrontal Cortex** | Executive control, forward modelling, planning depth | Impulsive, reactive, poor foresight | Strategic, controlled, slower to act | Modulates **Amygdala**, **Basal Ganglia**, hosts **Popperian** internal simulation | Tool manufacture, trap-setting, storage/caching, long-range planning |
| **Hippocampus**       | Learning & memory throughput; episodic memory | Slow learner, forgetful             | Fast learner, good navigation        | Boosts **Basal Ganglia** skill learning; key for Clayton-Dickinson what/where/when memory | Spatial memory, food caching, seasonal migration, resource sharing |
| **Amygdala**          | Threat sensitivity, emotional modulation | Under-reacts, fearless to fault     | Hyper-reactive, stress-prone         | Split by **Prefrontal Cortex** strength; bound to biochemistry (fear chemicals) | Alarm behaviour, territorial defence |
| **Basal Ganglia**     | Habit formation & skill automation   | Clumsy, inconsistent                | Efficient, repetitive, rigid         | Feeds from **Hippocampus**, governed by **PFC** | Skilled tool use, complex motor sequences, ritual behaviour |
| **Cerebellum**        | Motor precision & timing             | Sloppy movement                     | High dexterity, fine control         | Enhances **Basal Ganglia** execution; governs scent-deposition and precise manipulation | Tool manufacture (co-prerequisite with PFC) |
| **Temporal Lobes**    | Pattern & signal interpretation, language | Poor communication                  | Symbol recognition, language         | Works with **Hippocampus** for binding | Communication, scent-dialect, teaching offspring, naming |
| **Parietal Lobes**    | Sensory integration, sensory filtering | Disoriented                         | High awareness, fine discrimination  | Feeds **PFC** decisions; governs sensory bandwidth | Aesthetic sensibility, fine-grained kin recognition, tracking |

**Development costs.** More developed brains lead to higher caloric cost and longer developmental time. A **Capacity gene** increases the overall neurodevelopment budget available but at a cost to maintenance budget and developmental time. Each regional gene controls what percentage of the overall budget it gets, so they all compete.

**Memory as dendrite budget, not slot count.** The Hippocampus's effect on "memory capacity" is best modelled as a **dendrite budget per Concept-lobe neuron**, with priority = reward-linked activation and forgetting = exponential decay per Ebbinghaus (Tier 2: Murre & Dros, 2015). This replaces the earlier "memory blocks" framing with a mechanism that matches the Creatures architecture and has grounded decay curves from memory research. See [[../../reference/creature-cognition-and-social-scaling#memory-and-forgetting|reference doc §3]].

---

## 3. Dennett's tower as the cognitive progression gate

Rather than a vibes-based "this species is now sapient" flag, cognitive complexity in EcoSim is measured by where a species sits on **Dennett's Tower of Generate-and-Test** (Tier 5: Dennett, *Kinds of Minds*, 1996). Each tier is detectable by which sub-networks have evolved in the lineage's brain.

| Tier | Meaning | EcoSim detection | Unlocked culture systems (§4) |
|------|---------|------------------|-------------------------------|
| **1. Darwinian** | Fixed behaviours shaped by selection only | Default state — genes fix the behaviour repertoire, no in-life learning | None — no transmission |
| **2. Skinnerian** | In-life reinforcement learning | Functioning Concept-lobe equivalent with reward/punish chemical modulation | **Simple learning-transmission** — parent teaches offspring via imitation + reinforcement |
| **3. Popperian** | Internal simulation, forward-model planning | A simulation sub-network evolved in the PFC — creatures "try an action in their head" before committing | **Planning-based** — tool manufacture, caching, trap-setting, multi-step strategy |
| **4. Gregorian** | Mind-tools; cumulative culture; thinking about thinking | Detectable cumulative culture — behaviours transmitted non-genetically between generations with increasing complexity | **Symbolic / institutional** — language, writing, religion, institutions |

**Transitions as chronicle events.** When a species crosses a tier, the [[../infrastructure/chronicle-and-species-registry|chronicle system]] emits a major historical event (`TierTransitionEvent { from, to, tick, species_id }`). Genesis's divine-favour mechanic can use these transitions as narrative pivots — the sapience threshold in the divine-favour arc is specifically the **Skinnerian → Popperian** transition, where a species first becomes capable of meaningful planning and self-imagery.

**Grand's primary-source framing** for why the Skinnerian → Popperian transition matters, from his 2011 essay *Introduction to an Artificial Mind*:

> *"Norns learn, they generalize from their past to help them deal with novel situations, and they react intelligently to stimuli. BUT THEY DON'T THINK."* — Grand, stevegrand.wordpress.com 2011-03-06

Creatures (2nd tier) is the floor we know is achievable. Grandroids aimed at the Popperian transition (not fully achieved even by Grand). EcoSim's goal is to ship the Skinnerian floor reliably and make the Popperian transition an evolved, chronicle-worthy milestone — not to solve the open research problem of making it happen organically from pure selection.

---

## 4. Culture as gated authored systems

Cultural capabilities — tool use, language, agriculture, religion, writing, art, ritual — are implemented as **authored engine systems**, not as emergent phenomena. Each cultural system has **prerequisites** that a species must meet to unlock it. Prerequisites can be one or more of:

- **Brain region thresholds** (e.g., language requires Temporal Lobes + Hippocampus above threshold)
- **Dennett tower tier** (e.g., religion requires Popperian or above — you can't worship something you can't imagine)
- **Biochemistry features** (e.g., music requires specific reward-chemical circuits for rhythm perception)
- **Environmental / discovery triggers** (e.g., fire-mastery requires exposure to a lightning-strike event while the species has sufficient PFC for causal association)
- **Social preconditions** (e.g., agriculture requires a species already sedentary enough to return to a food source)

**The culture system is the blueprint; the species's evolved brain is the key.** When a species meets the prerequisites, the system is **accessible** to that species. Instances of the culture (specific tools, specific languages, specific rituals) then emerge from environmental and social context, giving variety across worlds and species without requiring each culture to be hand-authored end-to-end.

### Candidate culture systems and prerequisites (starting point)

This is a starting list; each system needs its own design pass.

| Culture system | Tier | Brain prerequisites | Other prereqs |
|----------------|------|---------------------|---------------|
| **Simple tool use** (pick up stick, break nut with rock) | Skinnerian | PFC moderate, Cerebellum moderate | — |
| **Tool manufacture** (shape a stick into a point) | Popperian | PFC high, Cerebellum high | Discovery: observed-manipulation of material |
| **Fire use** | Popperian | PFC high, Amygdala regulated | Discovery: survived lightning-strike exposure |
| **Scent-dialect / proto-language** | Skinnerian | Temporal Lobes moderate, Social drives | Scent system already in EcoSim |
| **Symbolic language** | Popperian | Temporal Lobes high, Hippocampus high, Concept-lobe capacity | — |
| **Food caching** | Skinnerian | Hippocampus (spatial memory) | — |
| **Cooperative hunting** | Skinnerian | Temporal Lobes (communication), Social drives, kin-recognition via scent | — |
| **Teaching offspring** | Skinnerian | Temporal Lobes, social drives, PFC (suppress self-interest) | — |
| **Seasonal migration** | Skinnerian | Hippocampus (episodic memory — what/where/when) | Environmental: seasonal resource gradient |
| **Agriculture / food cultivation** | Popperian | PFC (long-horizon planning), Hippocampus (spatial+temporal memory) | Social: sedentary behaviour already established |
| **Rituals / proto-religion** | Popperian | PFC (counterfactual reasoning), language unlocked, Amygdala (emotional weight) | Discovery: unexplained significant event |
| **Writing / symbolic recording** | Gregorian | Language unlocked, Cumulative culture detected, Cerebellum (fine motor) | Discovery or invention |
| **Institutions / law / hierarchy** | Gregorian | Language, writing or persistent memory, social stratification emerged | — |

The list is open-ended. New systems can be authored as engine features at any time — each one just needs its prerequisite graph defined.

### Emergent variety on top of authored systems

Once a culture system is unlocked for a species, the **specific instance** is emergent from:

- **Environment** — what objects exist to make tools from, what foods exist to cultivate, what sounds the scent-palette supports
- **Social structure** — how many individuals transmit to how many, whether transmission is parent→offspring only or wider
- **Genetic predispositions** — some species might favour scent-based language over sound; some might favour individual over cooperative strategies
- **Random drift / initial conditions** — which first instance of the system appeared and what it was shaped by
- **Inter-species contact** — two cultures that meet can borrow elements from each other

This emergence is cheap because it operates on *already-authored systems*. Two player worlds will have different cultures not because the culture systems are simulated from scratch, but because the *instances* of the same systems are shaped by different contexts.

### Why this sidesteps the research frontier

Real alife research is still trying to demonstrate that cumulative culture can emerge from evolved brains under pure environmental selection pressure. That is an open problem (see [[../../reference/creature-cognition-and-social-scaling#gaps-in-the-literature|reference doc §8]]). EcoSim does not attempt to solve it. What EcoSim does:

1. **Models cognitive architecture honestly** via evolved brains with genetically-specified structure (the reference doc's grounding)
2. **Gates authored cultural systems behind cognitive prerequisites** so that cultural unlocks feel earned and reflect the evolved state of the species
3. **Emerges culture instances, not culture systems** — the systems are designed by us; the specific cultures are produced by the interaction of evolved brains with environments

This is a pragmatic game-design choice. It gives players the *feeling* of watching cultures emerge from evolution without requiring us to solve the alife research frontier. A species's cultural capabilities will look different in every playthrough because the genetic and environmental context is different — even though the underlying culture systems are authored and constant.

---

## 5. Open design questions

- **How many brain region genes?** Each brain region needs at least one gene controlling its development budget; may need separate genes for size, density, and specific connection rules. The existing `unified-organism-genome` plan should be updated.
- **Dendrite budget numbers for EcoSim.** The Norn reference numbers (952 total, 640 Concept) are reference scales; EcoSim's scale depends on performance budget. Needs empirical measurement once the neural substrate is implemented.
- **Tier transition detectors.** What exactly flags a Skinnerian → Popperian transition? A forward-model sub-network appearing in PFC is the target; how do we detect "a sub-network exists" in a NEAT-evolved topology? This is a non-trivial implementation question.
- **Culture system authoring workflow.** Culture systems are data-driven (each system is a record with prerequisites + instance generator). The authoring workflow needs to be defined — is this modder-editable like GODk plans to be?
- **Prerequisite graph evaluation.** When a species's brain crosses a threshold, the engine needs to efficiently check *which culture systems just became accessible*. This is a dependency-graph problem; solvable but needs specification.
- **Discovery events.** Some prerequisites are discovery-based (fire, flintknapping). How are discovery events triggered — probabilistically per tick, by specific environmental events, by player intervention? Needs design pass.
- **Cross-species transmission.** If species A unlocks language and teaches species B (with sufficient cognitive substrate), does B unlock language even without reaching the normal prerequisite? This is the "cultural transmission across speciation" question and the answer probably shapes a lot of narrative possibility.
- **Forgetting of cultural capabilities.** If a species loses the cognitive substrate (e.g. a lineage branch evolves smaller brains), does it lose access to previously-unlocked cultural systems? Probably yes, but transitions need definition.

---

## See also

- [[../../reference/creature-cognition-and-social-scaling]] — the research reference doc this plan consumes
- [[../../reference/belief-systems-and-god-games]] — reference doc for the Genesis divine-favour mechanic that hooks into the Dennett tower progression
- [[../../game-concepts/genesis-god-sim#divine-mythology-champions]] — Genesis's divine-favour mechanic, which uses Skinnerian → Popperian transition as its sapience threshold
- [[../infrastructure/chronicle-and-species-registry]] — the chronicle system that emits tier-transition and culture-unlock events
- [[unified-organism-genome]] — the genome plan that will need to grow to include neurodevelopment genes
- [[../behavior/sensory-phases]] — sensory system phases that interact with brain region development
