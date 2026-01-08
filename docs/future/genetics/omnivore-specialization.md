---
title: Specialization Trade-offs for Omnivore Balance
created: 2025-12-26
updated: 2025-12-27
status: partially-implemented
priority: medium
tags: [genetics, balance, omnivore, trade-offs]
---

# Specialization Trade-offs: Preventing Omnivore Dominance

## Overview

Design for realistic constraints that prevent omnivores from dominating while maintaining emergent behavior. Some gene cross-effects are now implemented; additional mechanisms remain for future work.

## Problem Statement

Creatures with high values for multiple digestion types could become "super-omnivores" that outcompete specialists. This doesn't match natural ecosystems where specialists often dominate their niches.

## Implemented Mechanisms

### Negative Pleiotropy (Partial)

Gene cross-effects that create trade-offs are implemented in `src/genetics/defaults/UniversalGenes.cpp`:

- `PLANT_DIGESTION_EFFICIENCY` reduces `meat_digestion_efficiency` by 50%
- `MEAT_DIGESTION_EFFICIENCY` reduces `plant_digestion_efficiency` by 50%
- `GUT_LENGTH` adds +35% to `plant_digestion_efficiency`
- `STOMACH_ACIDITY` adds +35% to `meat_digestion_efficiency`
- `TOOTH_SHARPNESS` adds +15% to `meat_digestion_efficiency`
- `TOOTH_GRINDING` adds +15% to `plant_digestion_efficiency`

## Future Work

### 1. Metabolic Overhead System

Maintaining multiple enzyme systems should cost base metabolic rate. Super-generalists should burn 21.8% more energy just existing.

### 2. Specialist Efficiency Bonus

Specialists should extract up to 30% more nutrition from preferred food based on their focus score.

### 3. Additional Anatomical Cross-Effects

- Gut length affects movement (heavy gut = slower)
- Long gut causes meat to rot before digestion
- Sharp teeth reduce plant grinding ability
- Grinding teeth reduce bite damage

## Omnivore Ecological Niche

**When Omnivores Should Thrive:**
- Scarce resources - flexibility allows survival when specialists starve
- Variable environments - seasonal fluctuations favor generalists
- Edge habitats - transition zones with diverse food sources

**When Omnivores Should Struggle:**
- Resource abundance - specialists extract more from preferred food
- Stable environments - specialists evolve optimal adaptations
- High competition - always second-best at everything

## See Also

- [[balance-system]] - Balance analysis tool
- [[creature-plant-coevolution]] - Plant-creature co-evolution
- [[gene-reference]] - Gene definitions
