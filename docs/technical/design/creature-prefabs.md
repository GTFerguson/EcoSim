---
title: Creature Prefab System
created: 2025-12-25
updated: 2025-12-25
status: complete
tags: [creatures, archetypes, balance, combat, factory]
---

# Creature Prefab System

## Overview

The creature prefab system provides 10 balanced archetype templates for ecosystem simulation. Each archetype represents a distinct ecological niche with specific combat capabilities, dietary preferences, and behavioral patterns. The system is designed around rock-paper-scissors balance principles where each strategy has natural counters.

**Source**: [`CreatureFactory.cpp:258-748`](../../src/genetics/organisms/CreatureFactory.cpp:258)

## The 10 Archetypes

### Predators

#### Apex Predator (A) - "Carnotitan"

Large, powerful, territorial dominant predators that sit at the top of the food chain.

**Ecological Role**: Solitary apex hunters that control prey populations through raw power

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 2.3 - 2.7 | Second largest |
| Locomotion | 0.85 - 0.95 | Good speed |
| Teeth Sharpness | 0.85 - 0.95 | Primary weapon |
| Claw Sharpness | 0.75 - 0.85 | Secondary weapon |
| Hide Thickness | 0.70 - 0.80 | Moderate defense |
| Combat Aggression | 0.80 - 0.90 | Highly aggressive |
| Territorial Aggression | 0.85 - 0.95 | Very territorial |
| Retreat Threshold | 0.10 - 0.20 | Rarely retreats |

**Strengths**: Raw power, durability, intimidation
**Weaknesses**: High energy requirements, slow reproduction, solitary (no pack support)

---

#### Pack Hunter (P) - "Carnosocialis"

Small, fast, coordinated predators that hunt cooperatively.

**Ecological Role**: Group hunters that take down prey larger than any individual could manage

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 1.0 - 1.4 | Small |
| Locomotion | 1.3 - 1.5 | Fast |
| Teeth Sharpness | 0.80 - 0.90 | Sharp but small |
| Pack Coordination | 0.85 - 0.95 | Highest in system |
| Olfactory Acuity | 0.80 - 0.90 | Excellent tracking |
| Combat Aggression | 0.65 - 0.75 | Moderate |
| Retreat Threshold | 0.30 - 0.40 | Knows when to quit |

**Strengths**: Coordination, speed, scent tracking, pack tactics
**Weaknesses**: Individually weak, light armor, relies on numbers

---

#### Ambush Predator (I) - "Insidiatitan"

Patient, high burst damage predators that rely on stealth and a devastating first strike.

**Ecological Role**: Territorial ambush hunters that control specific areas

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 2.0 - 2.4 | Large |
| Locomotion | 0.50 - 0.70 | Slow sustained |
| Metabolism Rate | 0.80 - 1.0 | Low (patient) |
| Teeth Sharpness | 0.90 - 0.98 | Devastating bite |
| Teeth Size | 0.90 - 0.98 | Largest teeth |
| Scent Masking | 0.75 - 0.85 | Excellent stealth |
| Scale Coverage | 0.65 - 0.75 | Camouflaged |
| Combat Aggression | 0.55 - 0.65 | Patient |

**Strengths**: First-strike damage, stealth, durability, patience (low metabolism)
**Weaknesses**: Poor pursuit capability, vulnerable if ambush fails

---

#### Pursuit Hunter (V) - "Velocipraeda"

The fastest predator, built for chasing down prey over distance.

**Ecological Role**: Pursuit predators that specialize in catching fast prey

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 1.3 - 1.7 | Medium |
| Locomotion | 1.8 - 2.2 | Fastest creature |
| Claw Sharpness | 0.85 - 0.95 | Primary weapon |
| Claw Length | 0.80 - 0.90 | Slashing reach |
| Sight Range | 100 - 150 | Excellent vision |
| Metabolism Rate | 1.5 - 1.9 | High energy needs |
| Hide Thickness | 0.25 - 0.35 | Light for speed |
| Retreat Threshold | 0.30 - 0.40 | Calculated |

**Strengths**: Unmatched speed, excellent vision, slashing attacks
**Weaknesses**: Light armor, high energy needs, poor sustained combat

---

### Herbivores

#### Tank Herbivore (T) - "Herbotitan"

The largest creatures in the ecosystem, using bulk and horns for defense.

**Ecological Role**: Dominant grazers that can stand their ground against most predators

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 2.8 - 3.2 | Largest creature |
| Locomotion | 0.40 - 0.60 | Slow |
| Lifespan | 12000 - 18000 | Long-lived |
| Horn Length | 0.80 - 0.90 | Primary defense |
| Horn Pointiness | 0.75 - 0.85 | Dangerous |
| Hide Thickness | 0.90 - 0.98 | Maximum hide |
| Wound Tolerance | 0.70 - 0.85 | High |
| Retreat Threshold | 0.15 - 0.25 | Stands ground |

**Strengths**: Massive size, thick hide, horn defense, high health
**Weaknesses**: Very slow, high food requirements, vulnerable to pack tactics

---

#### Armored Grazer (K) - "Ankylosaurus-like"

Heavily armored with scales and a tail club, prioritizes defense over all else.

**Ecological Role**: Nearly invulnerable grazers that never flee

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 2.3 - 2.7 | Large |
| Locomotion | 0.30 - 0.50 | Very slow |
| Lifespan | 14000 - 20000 | Longest lived |
| Scale Coverage | 0.90 - 0.98 | Maximum armor |
| Tail Mass | 0.85 - 0.95 | Heavy club |
| Wound Tolerance | 0.80 - 0.90 | Highest |
| Retreat Threshold | 0.05 - 0.15 | Never runs |

**Strengths**: Best armor in system, tail club counter-attack, extreme longevity
**Weaknesses**: Extremely slow, low damage output, can be surrounded

---

#### Fleet Runner (R) - "Herbocursus"

Pure speed specialists with no combat capability - survival through escape.

**Ecological Role**: Fast-breeding prey that sustains predator populations

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 0.8 - 1.2 | Small |
| Locomotion | 1.7 - 2.1 | Very fast |
| Sight Range | 100 - 140 | Excellent early warning |
| Flee Threshold | 12 - 18 | Detects threats far away |
| Combat Aggression | 0.02 - 0.10 | Pacifist |
| Retreat Threshold | 0.85 - 0.95 | Runs immediately |
| Hide Thickness | 0.15 - 0.25 | Minimal |
| Regeneration Rate | 1.0 - 1.2 | Fast recovery |

**Strengths**: Speed rivals pursuit hunters, early threat detection, fast recovery
**Weaknesses**: No combat capability, vulnerable if caught, panics easily

---

#### Spiky Defender (S) - "Spinosus"

Porcupine-like creatures that deal damage to attackers through body spines.

**Ecological Role**: Defensive herbivores that discourage predators through retaliation

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 1.1 - 1.5 | Medium |
| Locomotion | 0.70 - 0.90 | Moderate |
| Body Spines | 0.90 - 0.98 | Maximum spines |
| Tail Spines | 0.75 - 0.85 | Additional spines |
| Hide Thickness | 0.55 - 0.65 | Moderate |
| Combat Aggression | 0.25 - 0.35 | Low |
| Retreat Threshold | 0.25 - 0.35 | Moderate |

**Strengths**: Counter-damage to melee attackers, moderate mobility, pack behavior
**Weaknesses**: Moderate armor, spines don't help against ambush, moderate size

---

### Opportunists

#### Scavenger (N) - "Necrophagus"

Carrion specialists that avoid combat but excel at finding and consuming corpses.

**Ecological Role**: Ecosystem cleaners that recycle nutrients from the dead

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 1.2 - 1.6 | Medium |
| Locomotion | 1.0 - 1.2 | Moderate |
| Toxin Tolerance | 0.90 - 0.98 | Highest - eats rotten meat |
| Toxin Metabolism | 0.80 - 0.90 | Processes toxins well |
| Olfactory Acuity | 0.90 - 0.98 | Excellent corpse detection |
| Scent Detection | 0.90 - 0.98 | Best in system |
| Combat Aggression | 0.20 - 0.30 | Avoids fights |
| Retreat Threshold | 0.55 - 0.65 | Quick to retreat |

**Strengths**: Can eat what others can't, excellent scent tracking, low energy lifestyle
**Weaknesses**: Poor combat, depends on deaths, competes with predators at corpses

---

#### Omnivore Generalist (O) - "Omniflexus"

Jack-of-all-trades creatures with balanced stats in every category.

**Ecological Role**: Flexible opportunists that fill gaps in the ecosystem

**Key Characteristics**:

| Trait | Range | Notes |
|-------|-------|-------|
| Size | 1.6 - 2.0 | Medium |
| Locomotion | 0.90 - 1.1 | Average |
| Plant Digestion | 0.55 - 0.65 | Balanced |
| Meat Digestion | 0.55 - 0.65 | Balanced |
| Teeth Sharpness | 0.45 - 0.55 | Average |
| Hide Thickness | 0.45 - 0.55 | Average |
| Combat Aggression | 0.45 - 0.55 | Balanced |
| Retreat Threshold | 0.35 - 0.45 | Calculated |

**Strengths**: Flexibility, can eat anything, no critical weaknesses
**Weaknesses**: No specialization, outcompeted by specialists in their niches

---

## Combat Gene Configuration

### Weapon Distribution

| Archetype | Primary Weapon | Secondary | Notes |
|-----------|---------------|-----------|-------|
| Apex Predator | Teeth (0.85-0.95) | Claws (0.75-0.85) | Balanced offense |
| Pack Hunter | Teeth (0.80-0.90) | Claws (0.70-0.80) | Lower but coordinated |
| Ambush Predator | Teeth (0.90-0.98) | Claws (0.60-0.70) | Devastating bite |
| Pursuit Hunter | Claws (0.85-0.95) | Teeth (0.65-0.75) | Slashing focus |
| Tank Herbivore | Horns (0.80-0.90) | - | Defensive only |
| Armored Grazer | Tail Club (0.85-0.95) | - | Counter-attack |
| Fleet Runner | None | - | Pure escape |
| Spiky Defender | Body Spines (0.90-0.98) | Tail Spines (0.75-0.85) | Passive damage |
| Scavenger | Teeth (0.45-0.55) | - | Minimal |
| Omnivore | Teeth (0.45-0.55) | Claws (0.45-0.55) | Balanced minimal |

### Defense Distribution

| Archetype | Hide | Scales | Fat | Total Defense |
|-----------|------|--------|-----|---------------|
| Apex Predator | 0.70-0.80 | 0.10-0.20 | 0.30-0.40 | Moderate |
| Pack Hunter | 0.30-0.40 | 0.05-0.15 | 0.15-0.25 | Light |
| Ambush Predator | 0.80-0.90 | 0.65-0.75 | 0.35-0.45 | Heavy |
| Pursuit Hunter | 0.25-0.35 | 0.05-0.15 | 0.10-0.20 | Minimal |
| Tank Herbivore | 0.90-0.98 | 0.30-0.40 | 0.65-0.75 | Maximum |
| Armored Grazer | 0.75-0.85 | 0.90-0.98 | 0.75-0.85 | Maximum |
| Fleet Runner | 0.15-0.25 | 0.05-0.15 | 0.10-0.20 | Minimal |
| Spiky Defender | 0.55-0.65 | 0.20-0.30 | 0.30-0.40 | Moderate + Spines |
| Scavenger | 0.40-0.50 | 0.10-0.20 | 0.35-0.45 | Moderate |
| Omnivore | 0.45-0.55 | 0.15-0.25 | 0.35-0.45 | Moderate |

---

## Balance Philosophy

### Rock-Paper-Scissors Dynamics

The system creates circular counter relationships:

```
Apex Predator → Tank Herbivore (size advantage)
Tank Herbivore → Pack Hunter (too big to coordinate on)
Pack Hunter → Apex Predator (overwhelm with numbers)

Pursuit Hunter → Fleet Runner (can catch them)
Fleet Runner → Ambush Predator (too fast to ambush)
Ambush Predator → Pursuit Hunter (one-shot fragile hunters)

Spiky Defender → Pack Hunter (counter-damage punishes many hits)
Armored Grazer → Pursuit Hunter (can't penetrate armor)
```

### Spawn Ratios

The [`createEcosystemMix()`](../../src/genetics/organisms/CreatureFactory.cpp:168) function uses these ratios:

| Category | Percentage | Archetypes |
|----------|------------|------------|
| Herbivores | 60% | Tank, Armored, Fleet, Spiky |
| Predators | 25% | Apex, Pack, Ambush, Pursuit |
| Opportunists | 15% | Scavenger, Omnivore |

This creates a sustainable ecosystem where prey outnumber predators approximately 2.4:1.

### Design Principles

1. **No Perfect Counter**: Every archetype can be defeated or can succeed
2. **Multiple Viable Strategies**: Speed, armor, damage, stealth are all valid
3. **Ecological Realism**: Ratios mirror real ecosystem trophic levels
4. **Emergent Behavior**: Complex outcomes from simple rules

---

## Legacy Templates

Four legacy templates remain for backwards compatibility:

| Template | Character | Maps To |
|----------|-----------|---------|
| Grazer | Q | TankHerbivore category |
| Browser | 0 | FleetRunner category |
| Hunter | M | ApexPredator category |
| Forager | 0 | OmnivoreGeneralist category |

These are registered via [`registerDefaultTemplates()`](../../src/genetics/organisms/CreatureFactory.cpp:234) but new code should use the 10 modern archetypes.

> [!NOTE]
> Legacy templates use the old diet-niche design philosophy and have different gene ranges than modern archetypes. They're maintained for save file compatibility.

---

## See Also

- [[classification]] - Creature taxonomy and classification system
- [[gene-reference]] - Complete gene documentation
- [[plant-prefabs]] - Plant archetype system
- [[coevolution]] - Creature-plant interactions
