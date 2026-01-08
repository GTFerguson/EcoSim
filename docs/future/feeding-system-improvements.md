---
title: Feeding System Improvements
created: 2025-12-26
updated: 2025-12-26
status: needs-review
tags: [feeding, herbivore, balance]
---

# Feeding System Improvements

> [!WARNING]
> The feeding interaction system has accumulated significant complexity and may need a comprehensive review or simplification in the future.

## Overview

The creature feeding system, particularly herbivore plant-eating behavior, has multiple interconnected checks that create a complex decision tree. This document catalogs known issues and potential improvements.

## Current System Flow

When a hungry creature tries to eat a plant, the following checks occur:

```
hungryBehavior()
└── hungryProfile()
    └── findGeneticsPlants()
        └── canEatPlant()
            └── FeedingInteraction::attemptToEatPlant()
                ├── Phase 1: Detection Check
                │   └── detectionScore vs DETECTION_THRESHOLD
                ├── Phase 2: Attraction Check  
                │   └── attraction vs ATTRACTION_THRESHOLD
                ├── Phase 3: Defense Check
                │   └── canOvercomeDefenses()
                └── Phase 4: Nutrition Calculation
                    └── calculateNutrition()
```

## Known Issues

### Issue 1: Detection Formula Complexity

**Status**: Partially addressed (2025-12-26)

The detection formula in [`FeedingInteraction::attemptToEatPlant()`](../../src/genetics/interactions/FeedingInteraction.cpp:29) relies on:
- `colorVision` × `fruitAppeal`
- `scentDetection` × 0.5
- `plantDigestion` × 0.3 (added as fix)

**Problem**: Plants like grass have very low `fruit_appeal` (0.02-0.08) because they don't produce attractive fruit. The original formula didn't account for herbivores instinctively recognizing vegetation as food.

**Current Fix**: Added `plantDigestion * 0.3f` bonus to detection score and `plantDigestion * 0.2f` to attraction score.

**Concern**: This adds more terms to an already complex formula. A simpler approach might be:
- Herbivores automatically detect plants within their sight range
- Visual/scent appeal only affects attraction priority, not detectability

### Issue 2: Template Values Not Applied to Creatures

**Status**: Unresolved

The [`CreatureFactory`](../../src/genetics/organisms/CreatureFactory.cpp) templates define correct herbivore values:

| Template | Expected Plant Digestion | Actual Value |
|----------|--------------------------|--------------|
| tank_herbivore | 0.85-0.95 | ~0.24 |
| armored_grazer | 0.85-0.95 | ~0.23 |
| fleet_runner | 0.75-0.85 | ~0.20 |

This suggests either:
1. Template values not being applied during creature creation
2. Pleiotropy effects reducing values after application
3. Phenotype expression issues

**Impact**: Limited detection range (2.7-6.2 tiles instead of expected 15-20+)

### Issue 3: Multiple Overlapping Threshold Systems

The system has multiple threshold checks that can independently block feeding:

1. **Detection Threshold** (`DETECTION_THRESHOLD = 0.1f`)
2. **Attraction Threshold** (`ATTRACTION_THRESHOLD = 0.05f`)
3. **Defense Check** (`totalDamage < expectedNutrition * 2.0f`)
4. **Distance Check** in `canDetectPlant()`
5. **Diet Type Check** in `canEatPlant()`

**Concern**: Difficult to predict or balance behavior with so many independent checks.

### Issue 4: Hardcoded Magic Numbers

The system contains many hardcoded values:

```cpp
// FeedingInteraction.cpp
DETECTION_THRESHOLD = 0.1f
ATTRACTION_THRESHOLD = 0.05f
defense multiplier = 2.0f
forage bonus = 0.3f
forage attraction = 0.2f
```

These should potentially be:
- Gene-based traits
- Configuration-driven values
- Better documented with biological rationale

## Diagnostic Results

From `test_herbivore_feeding_diagnostic.cpp` (2025-12-26):

### Close Range (d=2) Results

| Creature | Berry | Grass | Oak | Thorn |
|----------|-------|-------|-----|-------|
| Tank Herbivore | ✓ | ✓ | ✗ | ✓ |
| Armored Grazer | ✓ | ✓ | ✗ | ✓ |
| Fleet Runner | ✓ | ✓ | ✗ | ✓ |
| Spiky Defender | ✓ | ✓ | ✗ | ✓ |
| Canopy Forager | ✓ | ✓ | ✗ | ✓ |
| Omnivore Gen. | ✓ | ✓ | ✗ | ✓ |

Oak fails because at 1000 ticks it only reaches 0.56/14.13 size with 0.98 nutrition (vs 0.21 toxicity).

### Medium/Far Range Results

All creatures show "OUT OF RANGE" at 10+ tiles due to Issue 2 (template values not applied).

## Potential Future Approaches

### Option A: Simplify Detection

Replace complex formula with simpler checks:
```cpp
bool canDetect = (distance <= sightRange) && 
                 ((isDietType(HERBIVORE) && plant.isEdible()) ||
                  (plant.fruitAppeal > 0.5f));
```

### Option B: Move Complexity to Genes

Make all thresholds gene-driven:
- `FOOD_DETECTION_SENSITIVITY` gene
- `FORAGING_INSTINCT` gene
- Let evolution optimize rather than hardcoding

### Option C: Unified Food System

Create a single `FoodInteraction` class that handles:
- Plant eating
- Meat eating
- Corpse scavenging

With consistent rules and thresholds.

## Files Involved

- [`src/genetics/interactions/FeedingInteraction.cpp`](../../src/genetics/interactions/FeedingInteraction.cpp) - Main feeding logic
- [`src/genetics/interactions/FeedingInteraction.hpp`](../../include/genetics/interactions/FeedingInteraction.hpp) - Threshold constants
- [`src/objects/creature/creature.cpp`](../../src/objects/creature/creature.cpp) - `hungryBehavior()`, `findGeneticsPlants()`, `canEatPlant()`
- [`src/genetics/organisms/CreatureFactory.cpp`](../../src/genetics/organisms/CreatureFactory.cpp) - Template definitions

## Diagnostic Test

The diagnostic test is available at:
[`src/testing/genetics/test_herbivore_feeding_diagnostic.cpp`](../../src/testing/genetics/test_herbivore_feeding_diagnostic.cpp)

Compile with:
```bash
g++ -std=c++17 -I include -o herbivore_diag_v2 \
  src/testing/genetics/test_herbivore_feeding_diagnostic.cpp \
  src/genetics/core/*.cpp \
  src/genetics/defaults/*.cpp \
  src/genetics/expression/*.cpp \
  src/genetics/interactions/*.cpp \
  src/genetics/organisms/*.cpp \
  src/genetics/classification/*.cpp \
  src/objects/creature/creature.cpp \
  src/objects/creature/navigator.cpp \
  src/objects/gameObject.cpp \
  src/world/*.cpp \
  src/calendar.cpp \
  src/statistics/*.cpp \
  src/logging/Logger.cpp \
  -lncurses
```

## See Also

- [[creature-plant-coevolution]] - Coevolution design
- [[gene-reference]] - Gene definitions
- [[blocking-issues]] - Critical issues list
