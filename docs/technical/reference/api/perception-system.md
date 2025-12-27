---
title: PerceptionSystem API Reference
created: 2025-12-27
updated: 2025-12-27
status: complete
audience: developer
type: api-reference
tags: [genetics, perception, scent, api]
---

# PerceptionSystem API Reference

Organism-agnostic perception system for scent emission and detection.

## Overview

The PerceptionSystem provides unified perception calculations that work for ANY organism type through the IGeneticOrganism interface.

**Key Design Principles:**
- NO type-specific code (no Plant, Creature, or DietType references)
- All organism queries use IGeneticOrganism& and phenotype traits
- Integrates with existing ScentLayer and its 8-element signature array

## File Locations

| File | Purpose |
|------|---------|
| [`include/genetics/systems/PerceptionSystem.hpp`](../../../../include/genetics/systems/PerceptionSystem.hpp) | Header |
| [`src/genetics/systems/PerceptionSystem.cpp`](../../../../src/genetics/systems/PerceptionSystem.cpp) | Implementation |

## Public Methods

### Scent Signature Generation

#### `buildScentSignature()`

```cpp
std::array<float, 8> buildScentSignature(const IGeneticOrganism& organism) const;
```

Builds 8-element scent signature from organism's phenotype traits.

**Returns:** Array with indices: [0]=nutrition, [1]=fruit_appeal, [2]=toxicity, [3]=hardiness, [4]=scent_production, [5]=color_hue, [6]=size_gene, [7]=reserved

### Scent Emission

#### `depositScent()`

```cpp
void depositScent(
    const IGeneticOrganism& organism,
    int posX, int posY,
    int sourceId,
    ScentLayer& scentLayer,
    ScentType type,
    unsigned int currentTick) const;
```

Deposits scent at position if organism's scent_production ≥ 0.05.

**Parameters:**
- `organism` - The organism emitting scent
- `posX, posY` - World coordinates for scent deposit
- `sourceId` - Unique identifier for the organism
- `scentLayer` - Target ScentLayer to deposit into
- `type` - ScentType (FOOD_TRAIL for plants)
- `currentTick` - Current simulation tick for decay calculations

### Scent Detection

#### `detectFoodDirection()`

```cpp
std::optional<std::pair<int, int>> detectFoodDirection(
    const IGeneticOrganism& seeker,
    int seekerX, int seekerY,
    const ScentLayer& scentLayer) const;
```

Finds direction to strongest edible FOOD_TRAIL scent.

**Returns:** Optional (x, y) coordinates of strongest scent source, or nullopt if none found

#### `isEdibleScent()`

```cpp
bool isEdibleScent(
    const std::array<float, 8>& signature,
    const IGeneticOrganism& eater) const;
```

Checks diet compatibility using phenotype traits.

**Checks:**
1. `plant_digestion >= 0.1f` (can digest plants)
2. `toxicity <= toxin_resistance × 1.5f` (not too toxic)
3. `hardiness <= eater_hardiness + 0.5f` (not too tough)
4. `nutrition >= 0.05f` (has nutritional value)

### Range Calculations

#### `calculateVisualRange()`

```cpp
float calculateVisualRange(
    const IGeneticOrganism& seeker,
    float targetColorfulness) const;
```

**Formula:** `sightRange + (colorVision × targetColorfulness × 100)`

#### `calculateScentRange()`

```cpp
float calculateScentRange(const IGeneticOrganism& seeker) const;
```

**Formula:** `scent_detection × 100`

#### `calculateEffectiveRange()`

```cpp
float calculateEffectiveRange(
    const IGeneticOrganism& seeker,
    float targetColorfulness) const;
```

Returns maximum of visual and scent ranges.

## Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `MIN_SCENT_PRODUCTION` | 0.05f | Minimum to emit scent |
| `DEFAULT_DECAY_RATE` | 50 | Ticks until scent decays |
| `SCENT_RANGE_MULTIPLIER` | 100.0f | Converts scent_detection to tiles |
| `COLOR_VISION_RANGE_MULTIPLIER` | 100.0f | Converts color_vision bonus to tiles |

## See Also

- [[../../systems/scent-system]] - Scent layer architecture
- [[../../reference/genes#9-olfactory-genes]] - Olfactory gene definitions
- [[interfaces]] - IGeneticOrganism interface
