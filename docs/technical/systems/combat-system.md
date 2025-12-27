---
title: Combat System
created: 2025-12-25
updated: 2025-12-26
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

## Shape Gene to Damage Formulas

### Teeth Damage
- pierce = TEETH_SHARPNESS × TEETH_SIZE
- slash = TEETH_SERRATION × TEETH_SIZE × 0.5
- blunt = (1.0 - TEETH_SHARPNESS) × TEETH_SIZE

### Claws Damage
- pierce = CLAW_CURVATURE × CLAW_LENGTH × CLAW_SHARPNESS
- slash = (1.0 - CLAW_CURVATURE) × CLAW_LENGTH × CLAW_SHARPNESS
- blunt = CLAW_LENGTH × (1.0 - CLAW_SHARPNESS) × 0.3

### Horns Damage
- pierce = HORN_POINTINESS × HORN_LENGTH
- slash = HORN_SPREAD × HORN_LENGTH × 0.3
- blunt = (1.0 - HORN_POINTINESS) × HORN_LENGTH

### Tail Damage
- pierce = TAIL_SPINES × TAIL_LENGTH
- slash = (1.0 - TAIL_MASS) × TAIL_LENGTH × 0.5
- blunt = TAIL_MASS × TAIL_LENGTH

### Body Damage
- pierce = BODY_SPINES × MAX_SIZE
- slash = 0.0
- blunt = MAX_SIZE

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
- [[../design/coevolution-design]] - Creature-plant coevolution

---

**File Locations:**
- Header: include/genetics/interactions/DamageTypes.hpp
- Header: include/genetics/interactions/CombatAction.hpp
- Header: include/genetics/interactions/CombatInteraction.hpp
- Implementation: src/genetics/interactions/CombatInteraction.cpp
