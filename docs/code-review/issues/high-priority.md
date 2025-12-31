---
title: High Priority Issues
created: 2025-12-26
updated: 2025-12-31
status: complete
tags: [code-review, issues, high-priority]
---

# High Priority Issues

> [!NOTE]
> All high priority issues have been resolved as of 2025-12-31.

## Overview

High priority issues are those that:
- Cause incorrect behavior in important features
- Significantly impact performance
- Create maintenance burden
- Have workarounds but are frequently encountered
- Could escalate to critical if left unaddressed

**Total High Priority Issues**: 10 (10 Resolved, 0 Open)

## Issue Registry

| ID | Component | File | Description | Impact | Status |
|----|-----------|------|-------------|--------|--------|
| H-001 | Genetics Core | [`Gene.hpp`](../../../include/genetics/core/Gene.hpp) | Two classes in one file | Maintainability | ✅ Resolved |
| H-002 | Genetics Core | Multiple | DRY violation: `getRandomEngine()` pattern | Code duplication | ✅ Resolved |
| H-003 | Expression System | [`Phenotype.cpp:85-250`](../../../src/genetics/expression/Phenotype.cpp:85) | DRY violation: Effect processing switch | Maintenance burden | ✅ Resolved |
| H-004 | Creature System | [`creature.cpp:1165`](../../../src/objects/creature/creature.cpp:1165) | Iterator invalidation risk | Potential crashes | ✅ Resolved |
| H-005 | Combat & Interactions | [`PhenotypeUtils.hpp`](../../../include/genetics/expression/PhenotypeUtils.hpp) | DRY violation: `getTraitSafe()` duplicated | Code duplication | ✅ Resolved |
| H-006 | World System | [`SpatialIndex.hpp`](../../../include/world/SpatialIndex.hpp) | No spatial indexing for creatures | O(n²) performance | ✅ Resolved |
| H-007 | Combat & Interactions | Multiple files | Asymmetric defense defaults | Balance issue | ✅ Resolved |
| H-008 | Plant System | [`Plant.cpp:708-762`](../../../src/genetics/organisms/Plant.cpp:708) | Incomplete serialization | Data loss on load | ✅ Resolved |
| H-009 | Plant System | [`Plant.hpp:66`](../../../include/genetics/organisms/Plant.hpp:66) | Missing IReproducible interface | API inconsistency | ✅ Resolved |
| H-010 | Support Systems | [`Logger.hpp`](../../../include/logging/Logger.hpp) | Unbounded history growth | Memory exhaustion | ✅ Resolved |

---

## Detailed Issues

### H-001: Two Classes in Gene.hpp

**Component**: Genetics Core
**File**: [`Gene.hpp`](../../../include/genetics/core/Gene.hpp)
**Severity**: High
**Status**: ✅ Resolved

#### Description

The `Gene.hpp` file contains both `Gene` and `GeneDefinition` classes. While related, these serve different purposes and should be in separate files.

#### Impact

- Longer compilation times
- Confusion about class responsibilities
- Cannot include one without the other

#### Resolution

`GeneDefinition` extracted to separate files: [`GeneDefinition.hpp`](../../../include/genetics/core/GeneDefinition.hpp) and [`GeneDefinition.cpp`](../../../src/genetics/core/GeneDefinition.cpp).

---

### H-002: DRY Violation - getRandomEngine()

**Component**: Genetics Core
**Files**: [`Gene.cpp:143-146`](../../../src/genetics/core/Gene.cpp:143), [`Chromosome.cpp:120-123`](../../../src/genetics/core/Chromosome.cpp:120)
**Severity**: High
**Status**: ✅ Resolved

#### Description

Thread-local RNG pattern duplicated in 3 locations with identical code.

#### Resolution

Created `RandomEngine` utility providing shared thread-local RNG implementation across genetics core components.

---

### H-003: DRY Violation - Effect Processing

**Component**: Expression System
**File**: [`Phenotype.cpp:85-250`](../../../src/genetics/expression/Phenotype.cpp:85)
**Severity**: High
**Status**: ✅ Resolved

#### Description

`EffectType` switch duplicated in `computeTrait()` and `computeTraitRaw()` (~60 lines each).

#### Resolution

Refactored so `computeTrait()` delegates to `computeTraitRaw()`, eliminating the code duplication.

---

### H-004: Iterator Invalidation Risk

**Component**: Creature System
**File**: [`creature.cpp:1165`](../../../src/objects/creature/creature.cpp:1165)
**Severity**: High
**Status**: ✅ Resolved

#### Description

`creatures.erase(closestPrey)` during iteration can invalidate iterators.

#### Resolution

Implemented mark-and-sweep pattern - creatures are now marked for death during iteration and removed afterward, preventing iterator invalidation.

---

### H-005: DRY Violation - getTraitSafe()

**Component**: Combat & Interactions
**Files**: `CombatInteraction.cpp`, `FeedingInteraction.cpp`, `SeedDispersal.cpp`
**Severity**: High
**Status**: ✅ Resolved (2025-12-31)

#### Description

`getTraitSafe()` helper duplicated across multiple interaction files.

#### Resolution

Consolidated all `getTraitSafe()` implementations into [`PhenotypeUtils.hpp`](../../../include/genetics/expression/PhenotypeUtils.hpp). All 8 files now use the shared utility, eliminating code duplication. Additionally, all 53 fallback values changed from non-zero defaults to 0.0f to prevent "phantom capabilities" for organisms without those genes.

---

### H-006: No Spatial Indexing

**Component**: World System
**File**: [`SpatialIndex.hpp`](../../../include/world/SpatialIndex.hpp)
**Severity**: High
**Status**: ✅ Resolved

#### Description

Creatures stored in flat vector - neighbor queries are O(n), making interactions O(n²).

#### Resolution

Implemented [`SpatialIndex`](../../../include/world/SpatialIndex.hpp) with grid-based spatial hashing for O(1) cell lookups. See [`spatial-index.md`](../../technical/systems/spatial-index.md) for documentation.

---

### H-007: Asymmetric Defense Defaults

**Component**: Combat & Interactions
**File**: Multiple files using `getTraitSafe()`
**Severity**: High
**Status**: ✅ Resolved (2025-12-31)

#### Description

Non-zero fallback values in `getTraitSafe()` calls were granting "phantom capabilities" to organisms without those genes. For example, defense traits defaulting to 0.3-0.5 meant creatures without defense genes still had inherent resistance.

#### Resolution

All 53 `getTraitSafe()` fallback values across 8 files changed to 0.0f. Organisms without specific genes now correctly have zero capability for those traits. This affects defense traits, weapon traits, perception, metabolism, and all other phenotype lookups.

---

### H-008: Incomplete Plant Serialization

**Component**: Plant System
**File**: [`Plant.cpp:708-762`](../../../src/genetics/organisms/Plant.cpp:708)
**Severity**: High
**Status**: ✅ Resolved

#### Description

`fromString()` creates random genome instead of deserializing original.

#### Resolution

Plant `toString()` now delegates to JSON serialization, ensuring complete genome data is preserved and restored correctly.

---

### H-009: Missing IReproducible Interface

**Component**: Plant System
**File**: [`Plant.hpp:66`](../../../include/genetics/organisms/Plant.hpp:66)
**Severity**: High
**Status**: ✅ Resolved

#### Description

Plant has reproduction methods but doesn't implement `IReproducible<Plant>`.

#### Resolution

Implemented unified `IReproducible` interface for Plant class, providing consistent reproduction API across all organisms.

---

### H-010: Unbounded Logger History

**Component**: Support Systems
**File**: [`Logger.hpp`](../../../include/logging/Logger.hpp)
**Severity**: High
**Status**: ✅ Resolved (2025-12-31)

#### Description

`m_populationHistory` and `m_breedingHistory` grow without bounds, leading to potential memory exhaustion in long-running simulations.

#### Resolution

Implemented bounded history using `std::deque` with `MAX_HISTORY_SIZE` limit (10,000 entries). When the limit is reached, oldest entries are removed via `pop_front()` before adding new ones. This provides O(1) insertion/removal while maintaining a sliding window of recent history.

---

## Navigation

- [[../README|Back to Code Review Overview]]
- [[../00-executive-summary|Executive Summary]]
- [[critical|Critical Issues]]
- [[medium-priority|Medium Priority Issues]]
- [[low-priority|Low Priority Issues]]
