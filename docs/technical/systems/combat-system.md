---
title: Combat System
created: 2025-12-25
updated: 2025-12-27
status: complete
audience: developer
type: system
tags: [combat, damage, weapons, defense]
---

# Combat System

**Version:** 1.0
**Date:** 2025-12-25

The combat system provides creature-to-creature combat interactions using a rock-paper-scissors damage type system with gene-derived weapons and defenses.

---

## Overview

The combat system consists of three main components:

1. **DamageTypes** - Damage and defense types with RPS effectiveness
2. **CombatAction** - Attack structures and damage distribution
3. **CombatInteraction** - Combat resolution logic

---

## Damage Types

### The Three Types

| Type | Description | Strong Against | Weak Against |
|------|-------------|----------------|--------------|
| **Piercing** | Puncture damage (sharp teeth, curved claws) | Scales | ThickHide |
| **Slashing** | Cutting damage (straight claws, horn sweeps) | FatLayer | Scales |
| **Blunt** | Impact damage (tail clubs, body mass) | ThickHide | FatLayer |

### Effectiveness Table

| Attack \ Defense | ThickHide | Scales | FatLayer |
|------------------|-----------|--------|----------|
| **Piercing** | 0.5× (resisted) | 1.5× (strong) | 1.0× |
| **Slashing** | 1.0× | 0.5× (resisted) | 1.5× (strong) |
| **Blunt** | 1.5× (strong) | 1.0× | 0.5× (resisted) |

### Biological Rationale

- **Pierce beats Scales**: Sharp points find gaps between scale edges
- **Slash beats Fat**: Cutting strokes slice through soft tissue easily
- **Blunt beats Hide**: Impact force transfers through tough leather
- **Hide resists Pierce**: Too thick and dense to puncture
- **Scales resist Slash**: Hard edges deflect cutting attacks
- **Fat resists Blunt**: Soft tissue absorbs and disperses impact

---

## Weapon Types

### Base Statistics

| Weapon | Base Damage | Cooldown | Primary Type | DPS |
|--------|-------------|----------|--------------|-----|
| **Teeth** | 10 | 3 ticks | Piercing | 3.33 |
| **Claws** | 6 | 2 ticks | Slashing | 3.00 |
| **Horns** | 12 | 4 ticks | Piercing/Blunt | 3.00 |
| **Tail** | 5 | 2 ticks | Blunt | 2.50 |
| **Body** | 8 | 3 ticks | Blunt | 2.67 |

### Weapon Availability

A weapon is available if the creature's relevant shape genes exceed the threshold (0.1):

| Weapon | Required Genes |
|--------|----------------|
| Teeth | teeth_size > 0.1 |
| Claws | claw_length > 0.1 |
| Horns | horn_length > 0.1 |
| Tail | tail_length > 0.1 |
| Body | Always available |

---

## Damage Calculation

### Core Formula

The damage formula separates **damage type distribution** (what kind of damage) from **damage magnitude** (how much damage):

```
rawDamage = distribution.total() × baseDamage × sizeFactor × specMultiplier
```

Where:
- `distribution.total()` = Sum of normalized damage distribution (always 1.0)
- `baseDamage` = Weapon's base damage value
- `sizeFactor` = Multiplier from size-related genes (magnitude)
- `specMultiplier` = Specialization bonus (1.0 to 1.5)

### Key Concepts

#### Normalized Distributions

Damage type distributions are **always normalized to sum to 1.0**. This ensures that shape genes determine the *type* of damage, not the amount.

For example, a creature with teeth that produce:
- Raw values: pierce=0.8, slash=0.2, blunt=0.4 (total=1.4)
- After normalization: pierce=0.57, slash=0.14, blunt=0.29 (total=1.0)

This means:
- 57% of damage is Piercing
- 14% of damage is Slashing
- 29% of damage is Blunt

#### Size as Magnitude Multiplier

Size genes (like `teeth_size`, `claw_length`, etc.) act as **magnitude multipliers** applied separately from the distribution:

```
sizeFactor = relevantSizeGene
```

This creates a clean separation:
- **Shape genes** → What TYPE of damage (distribution ratios)
- **Size genes** → How MUCH damage (magnitude multiplier)

---

## Shape Gene to Damage Distribution

Shape genes determine the **proportions** of each damage type. These raw values are then normalized to sum to 1.0.

### Teeth Distribution
- pierce = TEETH_SHARPNESS
- slash = TEETH_SERRATION × 0.5
- blunt = (1.0 - TEETH_SHARPNESS)
- **Size factor**: TEETH_SIZE

### Claws Distribution
- pierce = CLAW_CURVATURE × CLAW_SHARPNESS
- slash = (1.0 - CLAW_CURVATURE) × CLAW_SHARPNESS
- blunt = (1.0 - CLAW_SHARPNESS) × 0.3
- **Size factor**: CLAW_LENGTH

### Horns Distribution
- pierce = HORN_POINTINESS
- slash = HORN_SPREAD × 0.3
- blunt = (1.0 - HORN_POINTINESS)
- **Size factor**: HORN_LENGTH

### Tail Distribution
- pierce = TAIL_SPINES
- slash = (1.0 - TAIL_MASS) × 0.5
- blunt = TAIL_MASS
- **Size factor**: TAIL_LENGTH

### Body Distribution
- pierce = BODY_SPINES
- slash = 0.0
- blunt = 1.0
- **Size factor**: MAX_SIZE

> [!NOTE]
> All distributions are normalized after calculation. A creature with high TEETH_SHARPNESS and low TEETH_SERRATION will deal mostly Piercing damage, regardless of the raw values. The TEETH_SIZE gene then determines how much total damage is dealt.

---

## Defense Profile

Defense values come from three genes:

| Gene | Defense Type | Effect |
|------|--------------|--------|
| hide_thickness | ThickHide | Reduces Piercing |
| scale_coverage | Scales | Reduces Slashing |
| fat_layer_thickness | FatLayer | Reduces Blunt |

At maximum defense (1.0), damage is reduced by 50%.

---

## Combat Resolution

### Specialization Bonus

Creatures that focus damage in one type get a bonus:
- A pure specialist (100% in one type) gets +50% damage
- A generalist (33% in each type) gets no bonus

### Combat Initiation

Decision based on:
- COMBAT_AGGRESSION gene
- HUNT_INSTINCT gene
- Hunger level (desperate creatures fight more)

### Retreat Decision

Creature retreats when health percent falls below RETREAT_THRESHOLD gene value.

---

## Related Genes

### Combat Weapon Genes (13 genes)

**Teeth:** teeth_sharpness, teeth_serration, teeth_size
**Claws:** claw_length, claw_curvature, claw_sharpness
**Horns:** horn_length, horn_pointiness, horn_spread
**Tail:** tail_length, tail_mass, tail_spines
**Body:** body_spines

### Combat Defense Genes (2 genes + 1 shared)

- hide_thickness (also in Morphology)
- scale_coverage
- fat_layer_thickness

### Combat Behavior Genes (4 genes)

- combat_aggression - Willingness to fight
- retreat_threshold - When to flee
- territorial_aggression - Fight same-species
- pack_coordination - Group hunting (future)

### Health/Healing Genes (3 genes)

- regeneration_rate - Healing speed
- wound_tolerance - Behavior penalty reduction
- bleeding_resistance - Stop bleeding faster

---

## Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| WEAPON_USABILITY_THRESHOLD | 0.1 | Min gene value to use weapon |
| DEFAULT_STAMINA_COST | 0.1 | Energy per attack |
| COMBAT_INITIATION_THRESHOLD | 0.3 | Min score to start combat |
| BLEEDING_DAMAGE_THRESHOLD | 5.0 | Pierce damage for bleeding |
| MAX_DEFENSE_REDUCTION | 0.5 | Max damage reduction at 1.0 defense |

---

## Combat Logging

The combat system integrates with the simulation Logger to provide detailed combat event tracking with configurable verbosity levels.

### CombatLogEvent Struct

Combat events are logged using the [`CombatLogEvent`](include/logging/Logger.hpp:138) struct which captures comprehensive combat data:

#### Combatant Identification

| Field | Type | Description |
|-------|------|-------------|
| attackerId | int | Attacker creature ID |
| defenderId | int | Defender creature ID |
| attackerName | string | e.g., "ApexPredator_Alpha" |
| defenderName | string | e.g., "FleetRunner_Beta" |

#### Weapon & Attack Info

| Field | Type | Description |
|-------|------|-------------|
| weapon | WeaponType | Type of weapon used (Teeth, Claws, etc.) |
| primaryDamageType | DamageType | Main damage type dealt (Piercing, Slashing, Blunt) |

#### Damage Calculation

| Field | Type | Description |
|-------|------|-------------|
| rawDamage | float | Damage before defense applied |
| finalDamage | float | Damage after defense reduction |
| effectivenessMultiplier | float | Type effectiveness (0.5-1.5) |

#### Defense Info

| Field | Type | Description |
|-------|------|-------------|
| defenseUsed | DefenseType | Defender's primary defense type |
| defenseValue | float | Defense strength applied |

#### Health Readouts

| Field | Type | Description |
|-------|------|-------------|
| attackerHealthBefore | float | Attacker HP before attack |
| attackerHealthAfter | float | Attacker HP after attack |
| attackerMaxHealth | float | Attacker maximum HP |
| defenderHealthBefore | float | Defender HP before attack |
| defenderHealthAfter | float | Defender HP after attack |
| defenderMaxHealth | float | Defender maximum HP |

#### Effects & Outcomes

| Field | Type | Description |
|-------|------|-------------|
| hit | bool | Did the attack connect? |
| causedBleeding | bool | Did attack cause bleeding effect? |
| defenderDied | bool | Was this a killing blow? |
| critical | bool | Critical hit (future use) |

#### Stamina/Energy

| Field | Type | Description |
|-------|------|-------------|
| attackerStaminaBefore | float | Attacker stamina before attack |
| attackerStaminaAfter | float | Attacker stamina after attack |

### CombatLogDetail Verbosity Levels

Combat log verbosity is controlled via [`CombatLogDetail`](include/logging/Logger.hpp:48):

| Level | Description | Example Output |
|-------|-------------|----------------|
| MINIMAL | Just damage amount | `#1→#2: 15.8 damage` |
| STANDARD | + weapon, type, health | `Teeth 15.8 Pierce \| HP:50→34` |
| DETAILED | + effectiveness, defense | `Teeth 20.0→15.8 Pierce (×0.79 vs Scales)` |
| DEBUG | Full multi-line output | See Debug Format below |

#### Debug Format Example

```
[T:101] [DEBUG] === COMBAT EVENT ===
  Attacker: ApexPredator_Alpha (#1)
    Health: 45.0/50.0 → 45.0/50.0 | Stamina: 80.0 → 75.0
  Defender: FleetRunner_Beta (#2)
    Health: 50.0/50.0 → 34.2/50.0 | Taking 15.8 damage
  Attack: Teeth | Type: Piercing | Raw: 20.0
  Defense: Scales (0.35) | Effectiveness: ×0.79
  Effects: [BLEEDING]
  Outcome: Hit, Defender Alive
```

### Configuration

Set combat log verbosity via the Logger singleton:

```cpp
#include "logging/Logger.hpp"

// Set verbosity level
logging::Logger::getInstance().setCombatLogDetail(logging::CombatLogDetail::DETAILED);

// Get current level
logging::CombatLogDetail current = logging::Logger::getInstance().getCombatLogDetail();
```

Or configure via [`LoggerConfig`](include/logging/Logger.hpp:58):

```cpp
logging::LoggerConfig config;
config.combatDetail = logging::CombatLogDetail::STANDARD;  // Default
logging::Logger::getInstance().configure(config);
```

### Logging Combat Events

Log combat events via [`combatEvent()`](include/logging/Logger.hpp:216):

```cpp
logging::CombatLogEvent event;
event.attackerId = attacker.getId();
event.attackerName = attacker.getArchetypeLabel();
event.defenderId = defender.getId();
event.defenderName = defender.getArchetypeLabel();
event.weapon = action.weapon;
event.primaryDamageType = result.primaryType;
event.rawDamage = result.rawDamage;
event.finalDamage = result.finalDamage;
event.effectivenessMultiplier = result.effectivenessMultiplier;
event.defenderHealthBefore = defenderHealthBefore;
event.defenderHealthAfter = defender.getHealth();
event.defenderMaxHealth = defender.getMaxHealth();
event.causedBleeding = result.causedBleeding;
event.defenderDied = defender.getHealth() <= 0;

logging::Logger::getInstance().combatEvent(event);
```

---

## See Also

- [[genes]] - Combat gene reference
- [[classification]] - Archetype classification (affects combat behavior)
- [[../design/coevolution]] - Creature-plant coevolution

---

**File Locations:**
- Header: include/genetics/interactions/DamageTypes.hpp
- Header: include/genetics/interactions/CombatAction.hpp
- Header: include/genetics/interactions/CombatInteraction.hpp
- Implementation: src/genetics/interactions/CombatInteraction.cpp
