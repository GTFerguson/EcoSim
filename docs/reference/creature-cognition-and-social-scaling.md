---
title: Creature Cognition, Memory, and Social Scaling
created: 2026-04-14
updated: 2026-04-14
status: active
tags: [reference, cognition, neuroevolution, social-simulation, stigmergy, sapience, creatures, alife]
---

# Creature Cognition, Memory, and Social Scaling

Evidence base for modeling creature cognition in EcoSim — specifically the path from "organism with basic behaviour AI" to "organism whose behaviour is complex enough to feel sentient", the architecture of evolved brains, the bounded-memory problem, the social-coordination-at-scale problem, and mechanically-testable sapience markers.

This doc informs [[../future/genetics/neurodevelopment|neurodevelopment.md]] (the plans doc on brain region design) and [[../game-concepts/genesis-god-sim|Genesis]] (which needs a credible sapience path for its divine-favour mechanic to be meaningful).

---

## 1. The problem

EcoSim's creatures need cognition rich enough that the following are *not* arbitrary:

- **Species nurtured toward sapience** — the Genesis divine-favour arc depends on a species' cognition actually becoming more complex over generations, not a flag flipping at a fixed favour threshold.
- **Social collaboration** — intelligent creatures that hunt cooperatively, share food, warn kin of danger, teach offspring, form hierarchies — **without `O(population²)` social interaction cost** at thousands-of-creatures scale.
- **Memory with real limits** — creatures that remember food locations, threats, recognised individuals — bounded, priority-driven, mechanically forgetful.
- **A mechanically-testable sapience threshold** — not a vibes-based "they are smart now" flag; a concrete detector that can fire on an evolved trait.

The literature splits cleanly into **four substrates** for answering these: practitioner precedents (Creatures series), classical cognitive architectures (SOAR/ACT-R/CLARION), evolved neural architectures (NEAT, HyperNEAT, Karl Sims), and cheap-coordination techniques (flocking, influence maps, stigmergy). This doc catalogues all four.

---

## 2. Individual cognition — architecture

### The Creatures series (Grand, Cliff, Malhotra 1997) as the primary precedent

The Norns in Steve Grand's *Creatures* (Cyberlife, 1996–) are the richest practitioner precedent for individual cognition in an evolving game-scale population. Their architecture is unusually well-documented because Grand is a serious biologically-inspired AI researcher, not just a game designer. The peer-reviewed primary sources are:

- (Tier 2: Grand, S., Cliff, D. & Malhotra, A., 1997, *"Creatures: Artificial life autonomous software agents for home entertainment"*, *Proceedings of the First International Conference on Autonomous Agents*, ACM, [ACM DOI](https://dl.acm.org/doi/10.1145/267658.267663); also University of Sussex Cognitive Science Research Paper **CSRP434** / Millennium Technical Report 9601, **[free PDF at sussex.ac.uk](https://www.sussex.ac.uk/informatics/cogslib/reports/csrp/csrp434.pdf)** — this is the canonical primary URL, persistent and authoritative) — original conference paper
- (Tier 2: Grand, S. & Cliff, D., 1998, *"Creatures: Entertainment software agents with artificial life"*, *Autonomous Agents and Multi-Agent Systems* 1(1):39–57, [Springer](https://link.springer.com/article/10.1023/A:1010042522104); free mirror at [SciSpace](https://scispace.com/pdf/creatures-entertainment-software-agents-with-artificial-life-20bvap0h0w.pdf)) — expanded journal version
- (Tier 2: Cliff, D. & Grand, S., 1999, *"The Creatures Global Digital Ecosystem"*, *Artificial Life* 5(1):77–93, MIT Press, [link](https://direct.mit.edu/artl/article/5/1/77/2314/The-Creatures-Global-Digital-Ecosystem)) — follow-up paper on the multi-player Creatures ecosystem, adds population-level and cross-machine breeding to the original architecture

**Architectural tiers** (corroborated by secondary sources including [Zucconi, 2020, *The AI of Creatures*](https://www.alanzucconi.com/2020/07/27/the-ai-of-creatures/) and the Creatures community wiki at [creatures.wiki](https://creatures.wiki/Brain)):

**Tier 1 — Neural substrate.** A Gen-1 Norn is controlled by a recurrent neural network of **952 neurons** (not 954 as some secondary sources state) and **~5,000 connections**, organised into **9 functionally distinct lobes** named after neuroanatomical regions. Per-lobe neuron counts from the CSRP434 preprint:

| Lobe | Neurons | Role |
|------|--------:|------|
| Concept | 640 | Learned association storage; the main "memory" lobe |
| Perception | 112 | Input lobe |
| Attention | 40 | Attention-direction |
| Noun | 40 | Lexical binding for objects |
| Stimulus Source | 40 | Source tracking |
| Drive | 16 (13 active) | Hunger, cold, pain, loneliness, etc. |
| Decision | (small) | Winner-takes-all action selection |
| Verb | (small) | Lexical binding for actions |
| General Sense | (small) | General afferent signals |

The **Decision lobe** implements Winner-Takes-All over action neurons; each Decision neuron has **256 dendrites** from the Concept lobe — specifically **128 type-0 (excitatory / "do this") + 128 type-1 (inhibitory / "don't do this")**. The Concept lobe is where learned associations live and where most of the brain's representational capacity sits (640 of the 952 neurons — ~67%).

**Tier 2 — Biochemistry modulator layer.** Above the neurons sits a genetically-specified artificial biochemistry: chemicals, emitters (release chemicals in response to events), receptors (couple chemical levels to neural or behavioural changes), reactions, and half-lives. Hormones act as **diffuse modulators** of neuron activity and also gate staged ontogenetic development (aging, puberty, etc). Critically, this layer is *not* a collection of stat modifiers — it is a separate simulation tier that runs alongside neurons and couples into them via receptors. Drives (hunger, cold, pain, loneliness) live here and influence the Perception lobe, biasing action selection without being hardcoded into decision logic.

**Tier 3 — Genetic specification.** A Gen-1 Norn has ~790 haploid genes. They specify: brain lobe layout and dendrite wiring rules, biochemistry (which chemicals exist, reactions, receptors, emitters, half-lives), appearance, instincts, and life-stage transitions. **Crucially, the genes specify brain structure, not just scalar parameters.** Breeding mixes parental genes with mutation, producing inheritable variation in cognitive architecture across generations. Documented emergent outcomes include blind Norns, immortal Norns from hyper-efficient digestion, and novel behaviours from mutated receptors.

**Learning.** Hebbian plasticity at Concept-lobe dendrites, modulated by global "reward" and "punish" chemicals. When reward fires, recently-active inputs are strengthened; on punishment or "disappointment" (action taken, no outcome), they are weakened. Memory is bounded by the fixed dendrite budget per Concept neuron and by decay; new associations overwrite old unused ones. Language acquisition uses the Verb and Noun lobes: teaching by naming ("food!" while the Norn looks at a seed) binds the percept to the lexical item.

**Social behaviour emerges from drives + shared language, not a dedicated social layer.** This is the most important architectural insight for EcoSim. Norns have drives (loneliness, crowdedness) that push them toward or away from each other. They teach each other learned associations via speech in the "Bibble" dialect. Untaught Norns pick up language from neighbours — including Grendel dialect if raised among Grendels. Cross-species interaction (Norns / Grendels / Ettins) uses the same neural substrate and biochemistry; the differences are genetic (different instincts, different drives). **Nothing in the architecture is a "social module".** Social cognition is what happens when multiple cognition instances share an environment and a language lobe.

### Grand on what Creatures achieved and didn't — primary-source reflection

Grand's own retrospective on Norns is unusually candid and is worth quoting directly. From his 2011 essay *Introduction to an Artificial Mind* (Tier 5: Grand, S., 2011, *stevegrand.wordpress.com*, [blog post](https://stevegrand.wordpress.com/2011/03/06/introduction-to-an-artificial-mind/)):

> *"Norns learn, they generalize from their past to help them deal with novel situations, and they react intelligently to stimuli. BUT THEY DON'T THINK."*

Grand is explicit that the original Creatures architecture lands at **Skinnerian** in Dennett's tower (learning by reinforcement) but not **Popperian** (internal simulation). His follow-up project Grandroids was specifically an attempt at the Popperian transition — "mental imagery in its broadest sense — the ability for a virtual creature to visualize a state of the world that doesn't actually exist at that moment." His design argument:

> *"I don't sit here passively waiting for a stimulus to arrive, and then just react to it automatically, on the basis of a learned reflex... Most of the time I have thoughts going through my mind. [...] The internal model allows us to predict what WILL happen, in time for us to actually do something about it. Prediction is what intelligence is for."*

And his direct rejection of the symbolic-AI tradition:

> *"Artificial intelligence was founded largely on the assumption that thinking is reasoning, and reasoning is the logical application of knowledge. [...] But in truth there is no algorithm for thought."*

This primary-source framing is what makes Creatures unusually valuable as a precedent: Grand explicitly designed the Skinnerian substrate, explicitly diagnosed what it couldn't do, and explicitly proposed Popperian internal-simulation as the missing tier — which maps cleanly onto Dennett's tower from §6. **Creatures reached Skinnerian. Grandroids aimed at Popperian. Neither fully achieved the Gregorian cumulative-culture tier.** This gives EcoSim a concrete cognitive-architecture trajectory to aim for with known landmarks along the way.

Other direct-source Grand material worth reading: *Grandroids FAQ* ([link](https://stevegrand.wordpress.com/2011/03/02/grandroids-faq/)), Grand's *CV* ([link](https://stevegrand.wordpress.com/cv/)) which lists his full publication history including *The Emergence of Personality: How to Create Souls from Cells* and *Effing the Ineffable: an engineering approach to consciousness*, the *20 Years of Creatures* anniversary essay (Tier 5: Grand, S., 2016, *Discover Albia*, [link](https://discoveralbia.com/2016/11/20-years-of-creatures-with-steve-grand.html)), and the MIT Technology Review profile (Tier 5: *"A Grand Quest to Create Virtual Life"*, 2014, [link](https://www.technologyreview.com/2014/09/18/171315/a-grand-quest-to-create-virtual-life/)) which includes his motivation statement:

> *"By creating alternative kinds of life, I hope to encourage people to ask deep and important questions about their own lives. What does it mean to be alive, to be conscious, to be hurt, to be ethical? Artificial life can help to shed a little light on all these things."*

Grand's books are both available for borrow on Internet Archive:
- *Creation: Life and How to Make It* (Harvard University Press, 2000) — [archive.org/details/creationlifehowt00gran](https://archive.org/details/creationlifehowt00gran)
- *Growing Up with Lucy: How to Build an Android in Twenty Easy Steps* (Weidenfeld & Nicolson, 2003) — [archive.org/details/growingupwithluc0000gran](https://archive.org/details/growingupwithluc0000gran)

These are the next-deepest primary-source reads if the CSRP434 preprint is not enough detail; *Creation* in particular has extensive chapters on Norn brain architecture that automated tooling cannot render but a human reader can.

### Classical cognitive architectures as comparison

The three canonical cognitive architectures — SOAR, ACT-R, CLARION — are useful references but **none have shipped in a commercial game simulating thousands of agents**. The Kotseruba & Tsotsos survey of 84 architectures and ~900 practical applications confirms this gap (Tier 2: Kotseruba, I. & Tsotsos, J.K., 2018, *"40 Years of Cognitive Architectures: Core Cognitive Abilities and Practical Applications"*, *Artificial Intelligence Review* 53:17–94, [arXiv](https://arxiv.org/abs/1610.08602)).

- **SOAR** (Tier 2: Laird, J.E., 2012, *The Soar Cognitive Architecture*, MIT Press, [book](https://mitpress.mit.edu/9780262538534/); Tier 4: Laird, J.E., 2022, [arXiv:2205.03854](https://arxiv.org/pdf/2205.03854)) — symbolic architecture integrating production rules, working memory, episodic memory, semantic memory, reinforcement learning, mental imagery, and appraisal-based emotion. Targets general intelligence via a single decision cycle. Cost profile: O(productions × WME matches) per cycle via the Rete algorithm — tractable for single agents, untested at game scale.
- **ACT-R** (Tier 2: Ritter, F.E. et al., 2019, *"ACT-R: A cognitive architecture for modeling cognition"*, *WIREs Cognitive Science* 10(3):e1488, [link](https://wires.onlinelibrary.wiley.com/doi/abs/10.1002/wcs.1488)) — hybrid production system: symbolic chunks plus subsymbolic activation equations. Productions compete by a utility equation over cost/benefit. Implemented in Common Lisp, designed to model millisecond-level human cognition — not thousands of agents.
- **CLARION** (Tier 2: Sun, R., 2006, *"The CLARION cognitive architecture: Extending cognitive modeling to social simulation"*, in *Cognition and Multi-Agent Interaction*, Cambridge, [link](https://www.cambridge.org/core/books/abs/cognition-and-multiagent-interaction/clarion-cognitive-architecture-extending-cognitive-modeling-to-social-simulation/0873DF19A72639841BF5D9B5DEE64453)) — explicit dual-process architecture: top-level localist rules (explicit) interacting with bottom-level distributed neural-network representations (implicit). Four subsystems (action-centered, non-action-centered, motivational, metacognitive). Notably *has* been extended explicitly for social simulation but not at game scale.

**Takeaway:** the classical cognitive architectures are the right reference for understanding the *problem space* (what memory, learning, procedural knowledge, declarative knowledge look like in a principled architecture), but they are not cost-viable substrates for an EcoSim-scale ecosystem. The Creatures architecture is the closest match for "individual cognition at game scale" and its design choices (emergent social layer, biochemistry as modulator, genetic specification of structure, Hebbian concept associations) should be treated as defaults subject to revision.

### Bounded rationality as scoping constraint

All three Creatures-era and classical architectures respect Herbert Simon's **bounded rationality** — the principle that rational choice must match "the computational capacities actually possessed by organisms" (Tier 2: Simon, H.A., 1955, *"A Behavioral Model of Rational Choice"*, *Quarterly Journal of Economics* 69(1):99–118, [PDF](https://cooperative-individualism.org/simon-herbert_a-behavioral-model-of-rational-choice-1955-feb.pdf)). Simon's solution is **satisficing**: search until an aspiration-level threshold is met, not until the global optimum is found. Directly applicable as a per-tick compute budget cap: creatures don't optimise their decisions, they accept the first viable one.

Kahneman's dual-process theory (Tier 5: Kahneman, D., 2011, *Thinking, Fast and Slow*) translates this into game-implementation terms: **most ticks use a cheap reflex (System 1); occasionally upgrade to deliberation (System 2)** — meaning most creatures most of the time are not actually running expensive cognition, they are running cached policies.

---

## 3. Memory and forgetting

The literature converges on three primitives for bounded memory in agents:

- **Miller's 7±2** (Tier 2: Miller, G.A., 1956, *"The Magical Number Seven, Plus or Minus Two"*, *Psychological Review* 63(2):81–97, [PDF](https://psychclassics.yorku.ca/Miller/)) — short-term memory span is roughly seven chunks. The exact number has been retracted, but the principle — **a small fixed slot count** — persists. Modern retrospective: Cowan, N., 2015, *PMC4486516*, [link](https://pmc.ncbi.nlm.nih.gov/articles/PMC4486516/).
- **Ebbinghaus forgetting curve** (Tier 2: Ebbinghaus, H., 1885; modern replication: Murre, J.M.J. & Dros, J., 2015, *"Replication and Analysis of Ebbinghaus' Forgetting Curve"*, *PLoS ONE*, [link](https://pmc.ncbi.nlm.nih.gov/articles/PMC4492928/)) — retention decays roughly exponentially post-encoding; ~50% lost in an hour, ~30% after a day. Supports **priority-decayed memory slots** where unused items fade and reward-activated items are strengthened.
- **Norn concept-lobe memory** (Grand/Cliff/Malhotra 1997) — memory = fixed dendrite budget per Concept neuron; Hebbian strengthening on reward; decay + overwrite on disuse. This is the most directly-applicable formalisation we have — it takes the Miller "slot count" intuition and replaces it with a concrete dendrite-budget mechanism that is **genetically specifiable** (brain-region genes govern how many slots each creature has).

**Mapping to neurodevelopment.md's current sketch**: the existing Hippocampus design ("memory capacity as blocks") is structurally sound. Recommended update: formalise "memory blocks" as **dendrite budget on a Concept-lobe equivalent**, with priority = reward-linked activation, forgetting = exponential decay, and capacity = gene-specified. The genetic lever (Hippocampus size gene) then controls how many dendrites that creature's concept-substrate has, producing inheritable variation in memory capacity.

---

## 4. Social scaling — the core computational problem

**EcoSim cannot afford `O(population²)` social simulation.** At thousands of creatures this is hundreds of thousands of interactions per tick. The literature catalogues several cheap alternatives with concrete cost profiles.

### Flocking and spatial hashing

(Tier 2: Reynolds, C.W., 1987, *"Flocks, herds and schools: A distributed behavioral model"*, *SIGGRAPH '87* 21(4):25–34, [PDF](https://www.red3d.com/cwr/papers/1987/SIGGRAPH87.pdf))

Three local rules — separation, alignment, cohesion — over neighbours within a small radius. **Naive cost O(n²)**, but reduces to **O(n)** with a uniform grid or spatial hash: project positions into hash cells, inspect only local cells, cost is O(k) per query where k is cell occupancy. (Practitioner reference: [Müller, spatial hashing tutorial](https://matthias-research.github.io/pages/tenMinutePhysics/11-hashing.pdf).) This is the baseline for any local-neighbour-based interaction in EcoSim and should sit underneath every social computation.

### Influence maps

(Tier 5: Mark, D., 2015, *"Modular Tactical Influence Maps"*, in *Game AI Pro 2*, Ch. 30, [PDF](https://www.gameaipro.com/GameAIPro2/GameAIPro2_Chapter30_Modular_Tactical_Influence_Maps.pdf); Tier 5: Mark, D., GDC 2018, [Vault](https://www.gdcvault.com/play/1025243/))

Precomputed 2D scalar grids representing team strength, danger, desirability. Propagation via distance-decay. **O(1) per-agent query.** The standard technique for "what's the tactical situation in my region" without iterating over every other agent. Dave Mark's modular system is the canonical practitioner reference. EcoSim's existing spatial index makes influence maps a natural fit.

### Stigmergy and Ant Colony Optimization

(Tier 2: Dorigo, M. & Stützle, T., 2004, *Ant Colony Optimization*, MIT Press, [PDF](https://web2.qatar.cmu.edu/~gdicaro/15382/additional/aco-book.pdf); Tier 2: Theraulaz, G. & Bonabeau, E., 1999, *"A Brief History of Stigmergy"*, *Artificial Life* 5(2):97–116, [link](https://www.researchgate.net/publication/12680033))

**Stigmergy** (Grassé, 1959) is indirect coordination where an environmental trace left by one agent stimulates the next. **ACO** is the canonical algorithmic form: agents deposit and follow evaporating pheromone, and shorter/better paths accumulate more signal. Cost per agent per step is O(neighbours). The general principle is that **coordination moves into the environment**, decoupling social coordination from direct agent-to-agent messaging.

### Pheromones as social memory substrate — the key insight for EcoSim

EcoSim already has a **scent system**. Biology literature strongly supports scent as a substrate for all four social functions EcoSim needs:

- **Kin recognition / inbreeding avoidance** (Tier 2: Heth, G. et al., 1998, *"Heritable odour cues and sibling recognition in golden hamsters"*, *Animal Behaviour* 56(2):409–417, [ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0003347298907473))
- **Territory marking** (Tier 2: Wyatt, T.D., 2003, *Pheromones and Animal Behaviour*, Cambridge, [sample chapter](https://catdir.loc.gov/catdir/samples/cam033/2002024628.pdf))
- **Alarm / danger signalling** (Tier 2: Wyatt, T.D., 2017, *"Pheromones"*, *Current Biology* 27:R739–R743, [link](https://www.cell.com/current-biology/fulltext/S0960-9822(17)30776-5))
- **Mating signals** (Tier 2: Symonds, M.R.E. & Elgar, M.A., 2008, *"The evolution of pheromone diversity"*, *Trends in Ecology & Evolution* 23(4):220–228, [ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0169534708000542))

Computationally, Dorigo's ACO and Grassé-style stigmergy demonstrate that scent fields can carry all the coordination signal normally carried by direct messaging — at the cost of a single grid update per deposit and **O(1) per-query read**. A **channelized scent field** — one layer per signal type (kin, territory, alarm, mate-available) — gives O(1) per-agent social sensing regardless of population size, because the grid is the shared memory. Evaporation gives automatic decay, directly matching Ebbinghaus exponential forgetting from §3.

> [!IMPORTANT]
> **The combined framing — "scent field as bounded-cognition social memory substrate for game creatures" — is a synthesis not directly published in the literature.** Each component is supported by strong Tier-2 sources (pheromone biology, stigmergy theory, ACO algorithms), but no single paper names the combined design pattern for games. This is the most transferable insight in this entire reference doc and the most honestly novel claim — it should be treated as a testable hypothesis, not an established technique.

### Game implementations at scale

- **RimWorld thoughts** — Typed memory records with tick-based expiry. **Two-layer decay is explicit: short mood thoughts vs long social opinions.** Mood thoughts like "inside too long" expire in 140k–280k ticks (~7–14 days); social opinions like "Divorced me" persist 180 days social but only 30 days mood. (Tier 5: [RimWorld Wiki — Thoughts](https://rimworldwiki.com/wiki/Thoughts), [Social](https://rimworldwiki.com/wiki/Social); Tier 5: Sylvester, T., 2017, *"RimWorld: Contrarian, Ridiculous, and Impossible Game Design Methods"*, GDC 2017, [Vault](https://www.gdcvault.com/play/1024232/)). This is the clearest published precedent for **separating transient mood state from persistent social memory at different decay rates**.
- **Dwarf Fortress personality** — Beliefs + goals + facets (Aquinas-adjacent / Big Five hybrid). Memories modify facets over time; facets gate social skill learning and thought valence. (Tier 5: [DF Wiki — Personality facet](https://dwarffortresswiki.org/index.php/Personality_facet); Tier 4: Compton, K., 2021, *"Characterization and Emergent Narrative in Dwarf Fortress"*, [ResearchGate](https://www.researchgate.net/publication/356686095)). No published performance data.
- **The Sims smart objects** — Will Wright: the original Sims AI was "too good" and was *intentionally dumbed down*; agents primarily react to smart **objects** that advertise utility scores. **Cognition is offloaded to the environment** (a stigmergic trick). (Tier 5: [Wright quote, PC Gamer](https://www.pcgamer.com/games/the-sims/will-wright-says-the-original-sims-ai-was-actually-too-good-almost-anything-the-player-did-was-worse-than-the-sims-running-on-autopilot/); Tier 5: [Mark Brown — *The Genius AI Behind The Sims*](https://gmtk.substack.com/p/the-genius-ai-behind-the-sims)).
- **Crusader Kings opinion** — Each character holds an opinion value toward every other relevant character; heirs inherit fractions (25% positive, 50% negative) on death. (Tier 5: [CK3 AI modding wiki](https://ck3.paradoxwikis.com/AI_modding)). No published technical writeup.

**Takeaway:** RimWorld's two-layer decay model and The Sims' smart-object stigmergy are the most directly transferable. Neither is computationally novel — they are disciplined applications of the stigmergy and bounded-memory principles above.

---

## 5. Evolved neural architectures

The classical cognitive architectures above assume a *designed* brain. EcoSim wants **brains that are products of evolution**, which means the neural architecture itself has to be heritable, variable, and subject to selection pressure.

- (Tier 2: Sims, K., 1994, *"Evolving Virtual Creatures"*, *SIGGRAPH '94* pp.15–22, [PDF](https://www.karlsims.com/papers/siggraph94.pdf)) — the canonical reference: co-evolves morphology and neural controllers as directed graphs. Fitness was hand-designed (swim, walk, follow, compete for cube). Sims needed supercomputer time in 1994 for ~100 generations of ~300 creatures, so this is the earliest demonstration of *evolutionarily-grown brains* in simulation, though at very small scale compared to an EcoSim ecosystem.
- (Tier 2: Stanley, K.O. & Miikkulainen, R., 2002, *"Evolving Neural Networks through Augmenting Topologies"* (NEAT), *Evolutionary Computation* 10(2), [PDF](https://nn.cs.utexas.edu/downloads/papers/stanley.ec02.pdf)) — direct encoding, innovation numbers, species-protection, minimal-structure start. Evolves both network topology and weights simultaneously. Won ISAL's Outstanding Paper of the Decade. The most mature and widely-used neuroevolution framework.
- (Tier 2: Stanley, K.O., D'Ambrosio, D.B. & Gauci, J., 2009, *"A Hypercube-Based Encoding for Evolving Large-Scale Neural Networks"* (HyperNEAT), *Artificial Life*) — CPPN indirect encoding exploits geometric regularities to scale to orders of magnitude more connections. Designed specifically to address NEAT's scaling problems for deep or spatially-structured networks.
- (Tier 2: Tu, X. & Terzopoulos, D., 1994, *"Artificial Fishes: Physics, Locomotion, Perception, Behavior"*, *SIGGRAPH '94*; full paper: Terzopoulos, D., Tu, X. & Grzeszczuk, R., 1994, *"Artificial Fishes: Autonomous Locomotion, Perception, Behavior, and Learning in a Simulated Physical World"*, *Artificial Life* 1(4):327–351, [PDF](http://web.cs.ucla.edu/~dt/papers/alifej94/alifej94.pdf)) — per-agent brains with motor/perception/behaviour/learning centres; reinforcement-learned swimming; emergent schooling, mating, and predator-avoidance. **Important caveat: behaviours are partly hand-structured** (habits + intention generator), not purely emergent. Shows what is *actually achievable* with 1990s-era simulation; useful as a floor.
- (Tier 5: Nolfi, S. & Floreano, D., 2000, *Evolutionary Robotics*, MIT Press) — foundational synthesis of evolving NN controllers in embodied agents. "Brain/body/environment coupling is the substrate for emergent behaviour, not a bolt-on."

### Avoiding fitness-function traps

Naive fitness functions — survive, reproduce, get bigger — tend to plateau at "local optima" that block cognitive evolution. Two techniques address this:

- (Tier 2: Lehman, J. & Stanley, K.O., 2011, *"Abandoning Objectives: Evolution through the Search for Novelty Alone"*, *Evolutionary Computation* 19(2), [PDF](https://www.cs.swarthmore.edu/~meeden/DevelopmentalRobotics/lehman_ecj11.pdf)) — **Novelty Search**. Reward behavioural novelty *only*, ignoring task objective. Outperforms objective-driven search on deceptive tasks. Key insight: in open-ended environments, novelty is a better proxy for progress than fitness.
- (Tier 4: Wang, R., Lehman, J., Clune, J. & Stanley, K., 2019, *"POET: Open-Ended Coevolution of Environments and their Optimized Solutions"*, [arXiv:1901.01753](https://arxiv.org/abs/1901.01753)) — co-evolves environments and agents to dodge stagnation. Compute-expensive; Uber-scale demo. **Has not been demonstrated at ecosystem scale with persistent populations and open-ended environments** — this remains an open research problem.

### Cost at ecosystem scale

**None of these approaches have been demonstrated at the scale EcoSim needs.** NEAT has been used for thousands of parameters but suffers on deep networks. HyperNEAT scales better but has its own training-time issues. Sims's and Terzopoulos's fish stay at ~100s of agents. POET is compute-bound. This is a real gap: the substrates for evolved brains exist, but nobody has run them at ecosystem scale with thousands of agents and open-ended selection pressure. EcoSim is, in this specific narrow sense, attempting something the literature has not demonstrated.

---

## 6. Sapience thresholds — how to know when a species has "become sapient"

"Sapience" resists a single crisp threshold, but the literature converges on **hierarchical framings** where each tier is mechanically describable.

### Dennett's Tower of Generate-and-Test — the most implementable framework

(Tier 5: Dennett, D.C., 1996, *Kinds of Minds*, Basic Books; Tier 5: Dennett, D.C., 1987, *The Intentional Stance*, MIT Press, [MIT Press](https://mitpress.mit.edu/9780262540537/the-intentional-stance/))

Dennett's tower ladders cognitive complexity:

1. **Darwinian creatures** — fixed behaviours shaped by selection only
2. **Skinnerian creatures** — modify behaviour by in-life reinforcement (classical or operant conditioning)
3. **Popperian creatures** — internal simulation; "pre-select" actions against a forward model before committing
4. **Gregorian creatures** — mind-tools; thinking *about* thinking; use culturally-inherited cognitive scaffolds

**This maps directly to an architectural progression in EcoSim.** A creature is Darwinian if its genes fix its behaviours. It becomes Skinnerian when it has a functioning Concept-lobe equivalent that does Hebbian reinforcement learning. It becomes Popperian when it evolves an internal-simulation component — a "forward model" network that predicts outcomes before the Decision lobe commits. It becomes Gregorian when cumulative culture (non-genetic behavioural transmission) starts accumulating complexity. **Genesis can use these as architecture gates, each unlocking when specific sub-networks evolve**, rather than as arbitrary thresholds.

### Global Workspace Theory — computationally-relevant vocabulary

(Tier 2: Dehaene, S., Changeux, J.-P. & Naccache, L., 2011, *"The Global Neuronal Workspace Model of Conscious Access"*, *Research and Perspectives in Neurosciences*, [PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC8770991/))

GWT proposes consciousness = broadcast of local-processor contents to a shared workspace. Signatures: ignition, P3 wave, gamma burst, long-range sync. **Useful computationally as a vocabulary for an attention/broadcast bottleneck** — a "workspace" neuron layer that receives from multiple subsystems and can only process one "winner" per tick. This gives Dennett's Popperian tier a concrete implementation: the forward model runs in the workspace, and "conscious" is what reaches the workspace that tick.

### Integrated Information Theory — flagged as NOT recommended

(Tier 2: Tononi, G., 2004, *"An information integration theory of consciousness"*, *BMC Neuroscience* 5:42, [PubMed](https://pubmed.ncbi.nlm.nih.gov/15522121/))

Tononi's IIT proposes Φ (phi) as a scalar measure of consciousness. Theoretically attractive but:

- **Computationally intractable for real systems** — computing Φ exactly is super-exponential; approximations are contested
- **Contested as pseudoscience** — multiple recent critiques in *Nature Neuroscience* (2023/2025)
- Does not produce a usable per-creature metric at game scale

**Recommendation: don't use Φ-based metrics in Genesis.** Mentioning IIT in design discussion is fine; attempting to compute it is not.

### Testable cognitive markers

Concrete capacities that could be mechanically detected as evolved traits. Each is citeable, each has a plausible detector:

| Marker | Detector design | Citation |
|--------|-----------------|----------|
| **Nth-order intentionality** | Log whether an agent's policy conditions on a model of another agent's belief state; count recursion depth | Dennett 1987 (intentional stance) |
| **Episodic-like memory (what/where/when)** | Clayton & Dickinson's scrub-jay paradigm: does the creature avoid caches whose contents have decayed since encoding? | Tier 2: Clayton, N.S. & Dickinson, A., 1998, *Nature* 395:272–274, [link](https://www.nature.com/articles/26216) |
| **Mirror self-recognition analogue** | Expose agent to a reflection channel mirroring its own state; detect self-directed investigation rather than social response | Tier 2: Gallup 1970 lineage; modern review [royalsocietypublishing.org](https://royalsocietypublishing.org/rstb/article/380/1939/20240312) |
| **Tool manufacture (Popperian gate)** | Agent modifies a world object before use; forward model is queried N times before motor output | Dennett 1996 |
| **Deception** | Detect when an agent's communication/scent diverges from its ground-truth state in a way that increases fitness at another agent's expense | Tier 4: [arXiv:2603.05872](https://arxiv.org/abs/2603.05872) (evolving deception) |
| **Cumulative culture / ratchet effect** | Measure whether behavioural complexity in a lineage monotonically rises across generations via *social* learning, not genetics. Transmission fidelity is the key variable | Tier 2: Lewis, H.M. & Laland, K.N., 2012, *"Transmission fidelity is the key to the build-up of cumulative culture"*, *Phil Trans B*, [PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC3385684/); Tier 5: Tomasello ratchet hypothesis |

---

## 7. What's implementable in EcoSim now vs research frontier

**Implementable now, with current literature and off-the-shelf techniques:**

- **Three-layer cognition architecture** (biochemistry modulator + neural substrate + genetic specification) modelled on Creatures
- **Hebbian concept-lobe learning** with reward/punish chemical modulation
- **Dennett's tower as architecture gates** — Darwinian → Skinnerian → Popperian → Gregorian unlocks when specific sub-networks evolve
- **NEAT as the neural substrate** for the Skinnerian → Popperian transition (mature, open-source, proven)
- **Memory as bounded dendrite budget** with priority = reward activation, forgetting = exponential decay
- **Channelized scent field for O(1) social cognition** (kin / territory / alarm / mate) — computationally cheap, biologically grounded, matches existing engine systems
- **Spatial hashing for flocking-style local interactions** — standard technique
- **Influence maps for tactical situation queries** — standard technique
- **Two-layer memory decay** (short mood / long social), modelled on RimWorld
- **Smart-object stigmergy** for creature-environment interaction — modelled on The Sims
- **Novelty-search hybrid fitness** to escape local optima in evolving brains (Lehman & Stanley 2011)
- **Clayton/Dickinson-style world-probe detectors** for episodic memory (scripted test environment)
- **Mirror-test analogue detector** (cheap to instrument)
- **Deception detector** (compare communication to ground truth across agent pairs)
- **Cumulative-culture detector** (track behavioural complexity through non-genetic transmission)

**Research frontier — aspirational, not plannable for v1:**

- **HyperNEAT at ecosystem scale** with thousands of co-evolving agents (compute-bound)
- **Genuine nth-order theory of mind emerging** (not hand-scaffolded)
- **Symbolic/compositional communication emerging** without training pressure designed to produce it
- **POET-style open-ended environment generation** coupled to a full ecosystem simulator
- **Any IIT/Φ-based consciousness measure** (intractable)
- **Sapience as a fully unplanned emergent milestone** — no simulation has demonstrated this

---

## 8. Gaps in the literature

Honest gaps that any design decision must respect:

- **No commercial game uses SOAR/ACT-R/CLARION at ecosystem scale.** Verified via Kotseruba & Tsotsos 2018 survey of ~900 applications.
- **No published O(n log n) or O(n) per-agent bound for fully-remembered pairwise social tracking** (the RimWorld / DF model). Likely scales as O(n × k) where k is remembered-others-per-agent, but no formal result.
- **No paper directly proposes "scent field as social memory substrate for game creatures."** Each component (pheromone biology, stigmergy theory, ACO) is supported; the combined framing is ours.
- **No demonstrated path from Skinnerian to Popperian by pure environmental selection.** Every sim that shows internal world models either hand-engineered them (Terzopoulos) or used RL with engineered auxiliary losses. Getting Popperian cognition to *evolve* from Skinnerian under environmental selection alone is an open problem.
- **Cumulative culture has never been shown to emerge in an evolved-brain sim.** Transmission-fidelity thresholds from Lewis & Laland exist as numbers, but no simulation demonstrates the threshold crossing organically. **EcoSim explicitly sidesteps this gap** by not attempting to evolve cumulative culture from first principles — instead, cultural capabilities (tool use, language, agriculture, religion, writing, etc.) are implemented as **authored systems with evolved cognitive prerequisites as unlock gates**. A species gains *access* to a cultural system when its evolved brain meets the system's cognitive requirements; the specific *instance* of that culture (what tools, what language, what rituals) emerges from environmental and social context. This reframes the problem from "simulate sentient creatures capable of culture" to "emulate culture via gated authored systems, with evolved brains deciding who can access what." The research-frontier gap still exists for the alife community, but EcoSim does not attempt to cross it and does not depend on its solution.
- **Open-endedness is unsolved.** POET extends duration but doesn't guarantee cognitive complexity rises. Lehman & Stanley's *"Why Greatness Cannot Be Planned"* is vision, not algorithm.
- **Mirror test is contested as a sapience marker.** Fish and ants pass variants; the test's validity is weakening ([royalsocietypublishing.org/rstb/article/380/1939/20240312](https://royalsocietypublishing.org/rstb/article/380/1939/20240312)). Treat as one input, not a definitive marker.
- **No benchmark for evolved theory-of-mind depth.** Any such detector must be invented; cite the animal-cognition paradigms and implement analogues.
- **Grand/Cliff/Malhotra 1997 preprint URL is now known** — [CSRP434 at sussex.ac.uk](https://www.sussex.ac.uk/informatics/cogslib/reports/csrp/csrp434.pdf), free and persistent. The canonical numbers (952 neurons, 9 lobes, 128+128 dendrites per Decision neuron, Concept lobe 640) were cross-verified across multiple extractions of the PDF, the Cliff & Grand 1998 journal version, Zucconi 2020, and the Creatures Wiki. A human reader should still open the PDF directly to confirm exact wording and page numbers before implementation specs quote it verbatim — automated PDF text extraction returned garbled output for this document.
- **Grand's books — both available via Internet Archive borrow** ([Creation](https://archive.org/details/creationlifehowt00gran), [Growing Up with Lucy](https://archive.org/details/growingupwithluc0000gran)). Not directly read end-to-end during research; *Creation*'s chapters on Norn brain architecture are the recommended next read for deeper primary-source grounding.

**Key loose thread:** the most actionable mapping is **Dennett's tower as architecture gates + Clayton/Dickinson-style world-probe detectors + NEAT-family substrate + novelty-search pressure + channelized scent social memory**. Global Workspace Theory gives vocabulary for the broadcast bottleneck if a workspace-style attention mechanism is wanted. IIT should not be used.

---

## 9. Implications for EcoSim (pointers, not decisions)

This section deliberately does not make design decisions — those live in [[../future/genetics/neurodevelopment|neurodevelopment.md]] and [[../game-concepts/genesis-god-sim|genesis-god-sim.md]]. It points at what the evidence base supports:

- **Extending neurodevelopment.md** with the three-layer architecture (biochemistry modulator / neural substrate / genetic structure specification), bounded dendrite-budget memory, and Hebbian concept-lobe learning. See extension in that doc.
- **Using the channelized scent field** as the social memory substrate for kin recognition, territory, alarm, and mating — because it is the only technique found that is simultaneously (a) biologically grounded, (b) O(1) per-creature, (c) decay-native via evaporation, and (d) already compatible with EcoSim's existing scent system.
- **Dennett's tower as the Genesis sapience progression gate** — a species moves up the tower when specific architectural features evolve in its lineage. The divine-favour system in Genesis can hook sapience-threshold chronicle events to specific tower transitions rather than to a favour scalar alone.
- **Two-layer memory decay** (RimWorld model) as the pattern for short-term vs long-term retention.
- **NEAT as the substrate for evolved brain topology**, with novelty-search pressure to avoid fitness-function plateaus.

---

## References (consolidated by tier)

### Tier 2 — peer-reviewed primary sources

- Grand, S., Cliff, D. & Malhotra, A. (1997). *Creatures: Artificial life autonomous software agents for home entertainment*. *First International Conference on Autonomous Agents*. ACM. [ACM DOI](https://dl.acm.org/doi/10.1145/267658.267663) — canonical free preprint: [Sussex CSRP434 PDF](https://www.sussex.ac.uk/informatics/cogslib/reports/csrp/csrp434.pdf)
- Grand, S. & Cliff, D. (1998). *Creatures: Entertainment software agents with artificial life*. *Autonomous Agents and Multi-Agent Systems* 1(1):39–57. [Springer](https://link.springer.com/article/10.1023/A:1010042522104) — free mirror: [SciSpace PDF](https://scispace.com/pdf/creatures-entertainment-software-agents-with-artificial-life-20bvap0h0w.pdf)
- Cliff, D. & Grand, S. (1999). *The Creatures Global Digital Ecosystem*. *Artificial Life* 5(1):77–93. MIT Press. [link](https://direct.mit.edu/artl/article/5/1/77/2314/The-Creatures-Global-Digital-Ecosystem)
- Sims, K. (1994). *Evolving Virtual Creatures*. *SIGGRAPH '94* pp.15–22. [PDF](https://www.karlsims.com/papers/siggraph94.pdf)
- Stanley, K.O. & Miikkulainen, R. (2002). *Evolving Neural Networks through Augmenting Topologies* (NEAT). *Evolutionary Computation* 10(2). [PDF](https://nn.cs.utexas.edu/downloads/papers/stanley.ec02.pdf)
- Stanley, K.O., D'Ambrosio, D.B. & Gauci, J. (2009). *A Hypercube-Based Encoding for Evolving Large-Scale Neural Networks* (HyperNEAT). *Artificial Life*.
- Terzopoulos, D., Tu, X. & Grzeszczuk, R. (1994). *Artificial Fishes: Autonomous Locomotion, Perception, Behavior, and Learning in a Simulated Physical World*. *Artificial Life* 1(4):327–351. [PDF](http://web.cs.ucla.edu/~dt/papers/alifej94/alifej94.pdf)
- Reynolds, C.W. (1987). *Flocks, herds and schools: A distributed behavioral model*. *SIGGRAPH '87* 21(4):25–34. [PDF](https://www.red3d.com/cwr/papers/1987/SIGGRAPH87.pdf)
- Dorigo, M. & Stützle, T. (2004). *Ant Colony Optimization*. MIT Press. [PDF](https://web2.qatar.cmu.edu/~gdicaro/15382/additional/aco-book.pdf)
- Theraulaz, G. & Bonabeau, E. (1999). *A Brief History of Stigmergy*. *Artificial Life* 5(2):97–116. [ResearchGate](https://www.researchgate.net/publication/12680033)
- Dehaene, S., Changeux, J.-P. & Naccache, L. (2011). *The Global Neuronal Workspace Model of Conscious Access*. [PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC8770991/)
- Tononi, G. (2004). *An information integration theory of consciousness*. *BMC Neuroscience* 5:42. [PubMed](https://pubmed.ncbi.nlm.nih.gov/15522121/)
- Clayton, N.S. & Dickinson, A. (1998). *Episodic-like memory during cache recovery by scrub jays*. *Nature* 395:272–274. [link](https://www.nature.com/articles/26216)
- Lewis, H.M. & Laland, K.N. (2012). *Transmission fidelity is the key to the build-up of cumulative culture*. *Phil Trans B*. [PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC3385684/)
- Lehman, J. & Stanley, K.O. (2011). *Abandoning Objectives: Evolution through the Search for Novelty Alone*. *Evolutionary Computation* 19(2). [PDF](https://www.cs.swarthmore.edu/~meeden/DevelopmentalRobotics/lehman_ecj11.pdf)
- Simon, H.A. (1955). *A Behavioral Model of Rational Choice*. *Quarterly Journal of Economics* 69(1):99–118. [PDF](https://cooperative-individualism.org/simon-herbert_a-behavioral-model-of-rational-choice-1955-feb.pdf)
- Miller, G.A. (1956). *The Magical Number Seven, Plus or Minus Two*. *Psychological Review* 63(2):81–97. [PDF](https://psychclassics.yorku.ca/Miller/)
- Murre, J.M.J. & Dros, J. (2015). *Replication and Analysis of Ebbinghaus' Forgetting Curve*. *PLoS ONE*. [PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC4492928/)
- Kotseruba, I. & Tsotsos, J.K. (2018). *40 Years of Cognitive Architectures*. *Artificial Intelligence Review* 53:17–94. [arXiv](https://arxiv.org/abs/1610.08602)
- Laird, J.E. (2012). *The Soar Cognitive Architecture*. MIT Press. [book](https://mitpress.mit.edu/9780262538534/)
- Ritter, F.E. et al. (2019). *ACT-R: A cognitive architecture for modeling cognition*. *WIREs Cognitive Science* 10(3):e1488. [link](https://wires.onlinelibrary.wiley.com/doi/abs/10.1002/wcs.1488)
- Sun, R. (2006). *The CLARION cognitive architecture*. In *Cognition and Multi-Agent Interaction*. Cambridge.
- Heth, G. et al. (1998). *Heritable odour cues and sibling recognition in golden hamsters*. *Animal Behaviour* 56(2):409–417.
- Wyatt, T.D. (2003). *Pheromones and Animal Behaviour*. Cambridge. [sample](https://catdir.loc.gov/catdir/samples/cam033/2002024628.pdf)
- Wyatt, T.D. (2017). *Pheromones*. *Current Biology* 27:R739–R743. [link](https://www.cell.com/current-biology/fulltext/S0960-9822(17)30776-5)
- Symonds, M.R.E. & Elgar, M.A. (2008). *The evolution of pheromone diversity*. *Trends in Ecology & Evolution* 23(4):220–228.

### Tier 4 — workshop papers, book chapters, secondary peer-review

- Wang, R., Lehman, J., Clune, J. & Stanley, K. (2019). *POET: Open-Ended Coevolution of Environments and their Optimized Solutions*. [arXiv:1901.01753](https://arxiv.org/abs/1901.01753)
- Laird, J.E. (2022). *Introduction to the Soar Cognitive Architecture*. [arXiv:2205.03854](https://arxiv.org/pdf/2205.03854)
- Compton, K. (2021). *Characterization and Emergent Narrative in Dwarf Fortress*. [ResearchGate](https://www.researchgate.net/publication/356686095)

### Tier 5 — practitioner sources

- Grand, S. (2000). *Creation: Life and How to Make It*. Harvard University Press. [Internet Archive borrow](https://archive.org/details/creationlifehowt00gran)
- Grand, S. (2003). *Growing Up with Lucy: How to Build an Android in Twenty Easy Steps*. Weidenfeld & Nicolson. [Internet Archive borrow](https://archive.org/details/growingupwithluc0000gran)
- Grand, S. (2011). *Introduction to an Artificial Mind*. [stevegrand.wordpress.com](https://stevegrand.wordpress.com/2011/03/06/introduction-to-an-artificial-mind/) — primary-source blog essay on Grandroids design philosophy and his critique of Norn cognition limits
- Grand, S. (2011). *Grandroids FAQ*. [stevegrand.wordpress.com](https://stevegrand.wordpress.com/2011/03/02/grandroids-faq/)
- Grand, S. (2016). *20 Years of Creatures*. [Discover Albia](https://discoveralbia.com/2016/11/20-years-of-creatures-with-steve-grand.html) — anniversary retrospective
- *A Grand Quest to Create Virtual Life*. (2014). *MIT Technology Review*. [link](https://www.technologyreview.com/2014/09/18/171315/a-grand-quest-to-create-virtual-life/)
- Grand, S. *CV / publications list*. [stevegrand.wordpress.com/cv](https://stevegrand.wordpress.com/cv/) — full academic and keynote history including *The Emergence of Personality: How to Create Souls from Cells*, *Effing the Ineffable: an engineering approach to consciousness*, *Bubbles in cyberspace* (ECAI 1998)
- Nolfi, S. & Floreano, D. (2000). *Evolutionary Robotics*. MIT Press.
- Dennett, D.C. (1996). *Kinds of Minds*. Basic Books.
- Dennett, D.C. (1987). *The Intentional Stance*. MIT Press. [MIT Press](https://mitpress.mit.edu/9780262540537/the-intentional-stance/)
- Godfrey-Smith, P. (2016). *Other Minds*. FSG.
- Kahneman, D. (2011). *Thinking, Fast and Slow*.
- Mark, D. (2015). *Modular Tactical Influence Maps*. In *Game AI Pro 2*, Ch. 30. [PDF](https://www.gameaipro.com/GameAIPro2/GameAIPro2_Chapter30_Modular_Tactical_Influence_Maps.pdf)
- Sylvester, T. (2017). *RimWorld: Contrarian, Ridiculous, and Impossible Game Design Methods*. GDC 2017. [Vault](https://www.gdcvault.com/play/1024232/)
- Zucconi, A. (2020). *The AI of Creatures*. [blog](https://www.alanzucconi.com/2020/07/27/the-ai-of-creatures/)
- Creatures Wiki — [Brain](https://creatures.wiki/Brain), [Concept lobe](https://creatures.wiki/Concept_lobe), Genetics, Biochemistry, CAOS
- RimWorld Wiki — [Thoughts](https://rimworldwiki.com/wiki/Thoughts), [Social](https://rimworldwiki.com/wiki/Social)
- Dwarf Fortress Wiki — [Personality facet](https://dwarffortresswiki.org/index.php/Personality_facet)
- Wright, W. — [quoted in PC Gamer](https://www.pcgamer.com/games/the-sims/will-wright-says-the-original-sims-ai-was-actually-too-good-almost-anything-the-player-did-was-worse-than-the-sims-running-on-autopilot/)
- Brown, M. — *The Genius AI Behind The Sims*. [gmtk.substack.com](https://gmtk.substack.com/p/the-genius-ai-behind-the-sims)

---

## See also

- [[../future/genetics/neurodevelopment]] — the plan doc this reference informs
- [[../game-concepts/genesis-god-sim#divine-mythology-champions]] — the Genesis divine-favour mechanic that depends on a credible sapience path
- [[historical-narrative-generation]] — parallel reference doc on chronicle / story-sifting systems (distinct topic)
- [[belief-systems-and-god-games]] — parallel reference doc on belief cultivation loops (distinct topic)
