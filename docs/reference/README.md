---
title: EcoSim Reference Documentation
created: 2026-04-14
status: active
tags: [reference, evidence-base, citations]
---

# Reference Documentation

Evidence base for EcoSim design decisions. Reference docs capture findings from academic research and practitioner sources with full citations and tiered evidence quality. They are **inputs** to design decisions made elsewhere (plans, architecture docs, game-concept docs), not decisions themselves.

Follows the PROVEN principles (see `~/.claude/rules/proven-documentation.md`): every claim traces to a named, dated source; evidence is labeled by tier; findings are verifiable; each doc covers one topic.

## Documents

- [[historical-narrative-generation]] — How simulation games (Dwarf Fortress, Crusader Kings, RimWorld, Caves of Qud, Wildermyth, Ultima Ratio Regum) and the academic literature (Kreminski's story sifting framework, Ryan's curationism, storylets) handle the problem of distilling raw simulation events into readable world history. Informs the chronicle architecture in [[../future/infrastructure/chronicle-and-species-registry]].
- [[belief-systems-and-god-games]] — Evidence base for the player-as-deity belief/favour cultivation loop inherited from Black & White (Lionhead 2001/2005). Covers Richard Evans's Creature AI work, Molyneux's design sources, adjacent theory (Sicart, Lazzaro, Bartle), and honest gaps in the literature. Informs Genesis's divine-favour mechanic.
- [[creature-cognition-and-social-scaling]] — Evidence base for creature cognition architecture, bounded memory, social coordination at scale, sapience thresholds, and evolved neural architectures. Covers Steve Grand's Creatures (Norn brain architecture), cognitive architectures (SOAR/ACT-R/CLARION), stigmergy and pheromone-based social memory, Dennett's tower, NEAT/HyperNEAT, and Karl Sims. Informs the neurodevelopment plan.
- [[procedural-creature-appearance]] — Evidence base for gene-driven procedural creature visual generation at two tiers (cheap runtime in-world sprites, detailed inspector images). Covers Spore's metaball/retargeting pipeline, Rain World's soft-body paper-doll approach, Creatures' ATT attachment system, academic techniques (L-systems, CPPNs/Picbreeder, reaction-diffusion, Karl Sims morphology), and scalable sprite techniques (GPU instancing, modular composition). Rules out Stable Diffusion/GANs for Tier 1. Informs the pending visibility-systems plan.

## See also

- [[../game-concepts/tg-evolution-games]] — research on /tg/ collaborative evolution games, currently filed under game-concepts because it also covers game design inspiration. May eventually split into a reference-only version here.
