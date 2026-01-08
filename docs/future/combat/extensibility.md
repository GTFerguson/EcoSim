---
title: Combat System Extensibility
created: 2025-12-29
updated: 2025-12-29
status: future-consideration
tags: [combat, extensibility, architecture]
---

# Combat System Extensibility

## Overview

This document outlines architectural limitations discovered during code review that affect the extensibility of the combat system, particularly around damage types and defense types.

## Current Architecture

### Type Effectiveness System

The combat system uses a rock-paper-scissors effectiveness matrix defined in [`DamageTypes.hpp`](../../../include/genetics/interactions/DamageTypes.hpp):

```cpp
enum class CombatDamageType { Blunt, Pierce, Slash };
enum class DefenseType { ThickHide, Scales, FatLayer };

static constexpr float EFFECTIVENESS_TABLE[3][3] = {
    //              ThickHide  Scales  FatLayer
    /* Blunt  */  {    1.0f,    1.5f,    0.5f   },
    /* Pierce */  {    1.5f,    0.5f,    1.0f   },
    /* Slash  */  {    0.5f,    1.0f,    1.5f   }
};
```

### Defense Defaults Balancing

The defense gene defaults in [`UniversalGenes.cpp`](../../../src/genetics/defaults/UniversalGenes.cpp) are intentionally asymmetric:

| Defense Type | Default Value | Reason |
|--------------|--------------|--------|
| ThickHide | 0.3f | Counters Pierce (~31% of damage) |
| Scales | 0.3f | Counters Slash (~22% of damage) |
| FatLayer | 0.4f | Counters Blunt (~48% of damage) |

This balancing accounts for the weapon damage type distribution derived from the `WEAPON_BASE_STATS[]` array:
- **Blunt**: ~48% (tails, horns with low pointiness)
- **Pierce**: ~31% (teeth, claws with high curvature)
- **Slash**: ~22% (claws, serrated teeth)

> [!IMPORTANT]
> These defaults are intentionally balanced. Do not "equalize" them without understanding the damage type prevalence analysis.

## Extensibility Limitations

### 1. Quadratic Scaling Problem

**Issue**: Adding new damage or defense types requires O(N²) manual entries.

The effectiveness table grows quadratically with each new type:
- 3×3 = 9 effectiveness values (current)
- 4×4 = 16 values (+7 new entries)
- 5×5 = 25 values (+9 new entries)
- 6×6 = 36 values (+11 new entries)

Each new type requires:
1. New enum value in `CombatDamageType` and/or `DefenseType`
2. N new effectiveness values (one per existing type)
3. Updated default values in `UniversalGenes.cpp`
4. Re-balancing of all existing defaults

### 2. Rigid 1:1 Mapping

The current system assumes each damage type has exactly one "strong against" and one "weak against" defense type. This prevents:
- Damage types that are strong/weak against multiple defenses
- Defense types with varying degrees of resistance
- Conditional effectiveness (e.g., size-dependent)

### 3. Manual Default Balancing

New damage types require manual analysis of:
- How common the new damage type will be (weapon formulas)
- Which existing defenses should counter it
- Re-balancing all defense defaults to maintain equilibrium

## Proposed Solutions

### Option A: Data-Driven Configuration

Move the effectiveness table to a configuration file:

```json
{
  "damageTypes": ["Blunt", "Pierce", "Slash", "Fire", "Poison"],
  "defenseTypes": ["ThickHide", "Scales", "FatLayer", "FireResist", "Immunity"],
  "effectiveness": {
    "Blunt": { "ThickHide": 1.0, "Scales": 1.5, "FatLayer": 0.5, ... },
    ...
  }
}
```

**Pros**: Easy to modify, no recompilation  
**Cons**: Still O(N²), runtime overhead

### Option B: Category-Based Grouping

Group damage types into categories with shared interactions:

```cpp
enum class DamageCategory { Physical, Elemental, Chemical };
enum class DefenseCategory { Armor, Resistance, Immunity };

// Only need category-level effectiveness
static constexpr float CATEGORY_EFFECTIVENESS[3][3] = { ... };
```

**Pros**: O(1) scaling for types within categories  
**Cons**: Less granular control

### Option C: Formula-Based Calculation

Replace the table with a formula:

```cpp
float calculateEffectiveness(CombatDamageType dmg, DefenseType def) {
    // Based on damage properties vs defense properties
    float piercing = getDamagePiercing(dmg);
    float hardness = getDefenseHardness(def);
    return std::clamp(piercing - hardness + 1.0f, 0.5f, 1.5f);
}
```

**Pros**: Infinitely scalable, emergent interactions  
**Cons**: Harder to balance, less predictable

## Recommendations

For near-term development:
1. Document any new damage/defense types thoroughly
2. Include damage prevalence analysis with additions
3. Create automated balance tests for new types

For long-term refactoring:
1. Consider Option B (categories) for major expansions
2. Use Option A (config) for rapid iteration/testing
3. Option C suitable if pursuing more emergent simulation

## Related Documents

- [[combat-system]] - Combat system documentation
- [[balance-system]] - Game balance framework
- [[genes]] - Gene reference including combat genes

## See Also

- [`include/genetics/interactions/DamageTypes.hpp`](../../../include/genetics/interactions/DamageTypes.hpp)
- [`src/genetics/interactions/CombatInteraction.cpp`](../../../src/genetics/interactions/CombatInteraction.cpp)
- [`src/genetics/defaults/UniversalGenes.cpp`](../../../src/genetics/defaults/UniversalGenes.cpp)
