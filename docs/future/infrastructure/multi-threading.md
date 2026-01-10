---
title: Multi-Threading for Creature Updates
created: 2026-01-10
updated: 2026-01-10
status: deferred
priority: low
tags: [performance, threading, optimization, infrastructure]
---

# Multi-Threading for Creature Updates

**Status:** Deferred
**Priority:** Low (not needed until 1000+ creatures)
**Complexity:** High

---

## Overview

This document outlines a future multi-threading architecture for creature updates in EcoSim. Profiling has shown that creature updates become the primary bottleneck at high population counts (1000+ creatures). Multi-threading could provide 2-4x speedup on multi-core systems.

> [!NOTE]
> This optimization is **deferred** because:
> - Current single-threaded performance is adequate for typical simulations (100-500 creatures)
> - The complexity of thread-safe design is significant
> - Determinism requirements make implementation non-trivial

---

## Proposed Architecture

### Two-Phase Update Model

The key insight is separating creature updates into two phases:

```
┌─────────────────────────────────────────────────────────────────┐
│                      PHASE 1: DECISIONS                         │
│                    (Parallel, Read-Only)                        │
├─────────────────────────────────────────────────────────────────┤
│  Thread 1       │  Thread 2       │  Thread 3       │  Thread 4 │
│  ┌──────────┐   │  ┌──────────┐   │  ┌──────────┐   │  ┌──────────┐
│  │Creature 0│   │  │Creature 1│   │  │Creature 2│   │  │Creature 3│
│  │ - sense  │   │  │ - sense  │   │  │ - sense  │   │  │ - sense  │
│  │ - decide │   │  │ - decide │   │  │ - decide │   │  │ - decide │
│  │ - plan   │   │  │ - plan   │   │  │ - plan   │   │  │ - plan   │
│  └──────────┘   │  └──────────┘   │  └──────────┘   │  └──────────┘
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      PHASE 2: APPLY                             │
│                  (Sequential, Mutating)                         │
├─────────────────────────────────────────────────────────────────┤
│  For each creature (in deterministic order):                    │
│    - Apply movement                                             │
│    - Resolve conflicts (combat, mating, feeding)                │
│    - Process births (deferred from Phase 1)                     │
│    - Update spatial index                                       │
└─────────────────────────────────────────────────────────────────┘
```

### Phase 1: Decision Phase (Parallel)

Each creature independently:
1. **Senses environment** - Reads world state, queries spatial index
2. **Makes decisions** - Behavior selection, target finding
3. **Plans actions** - Movement direction, action type

**Thread safety:** All operations are read-only against shared state. Each creature writes only to its own pending action buffer.

### Phase 2: Apply Phase (Sequential)

After all decisions are made:
1. **Apply movements** - Update positions in deterministic order
2. **Resolve conflicts** - Handle simultaneous combat/mating
3. **Process births** - Add new creatures from mating
4. **Update indices** - Rebuild spatial index for next tick

**Why sequential:** Mutations to shared state (creature list, spatial index, world) must be ordered deterministically.

---

## Requirements

### 1. Deferred Birth Handling

Currently, births can occur during creature iteration, modifying the creature list. Multi-threading requires:

```cpp
struct DeferredBirth {
    Genome childGenome;
    float x, y;
    // ... other spawn parameters
};

// Phase 1: Record birth intent
std::vector<DeferredBirth> pendingBirths;  // Thread-local or per-creature

// Phase 2: Process all births after iteration complete
for (auto& birth : pendingBirths) {
    creatures.emplace_back(birth.childGenome, birth.x, birth.y);
}
```

### 2. Thread-Safe SpatialIndex Access

The spatial index is read-heavy during Phase 1. Options:

| Approach | Pros | Cons |
|----------|------|------|
| **Read-write lock** | Simple, allows concurrent reads | Lock contention overhead |
| **Immutable snapshot** | Zero contention, cache-friendly | Memory overhead for copy |
| **Per-thread index** | No synchronization | High memory, rebuild cost |

**Recommended:** Immutable snapshot created before Phase 1, used by all threads.

### 3. Determinism Preservation

For reproducible simulations:
- **Fixed thread count** - Same number of threads regardless of hardware
- **Deterministic partitioning** - Creatures assigned to threads by index, not dynamically
- **Ordered conflict resolution** - Lower creature ID wins ties
- **Seeded per-creature RNG** - Each creature has deterministic random stream

```cpp
// Deterministic creature partitioning
void partitionCreatures(std::vector<Creature>& creatures, int numThreads) {
    // Creature i goes to thread (i % numThreads)
    // Order within thread is preserved
}
```

### 4. Combat/Mating Conflict Resolution

When multiple creatures attempt simultaneous interactions:

```cpp
struct PendingInteraction {
    size_t initiatorId;
    size_t targetId;
    InteractionType type;  // COMBAT, MATE, FEED
    int priority;          // Deterministic tiebreaker
};

// Resolve conflicts: lower ID wins simultaneous actions on same target
std::vector<PendingInteraction> resolveConflicts(
    std::vector<PendingInteraction>& all
) {
    std::sort(all.begin(), all.end(), byTargetThenInitiatorId);
    // First interaction per target wins
}
```

---

## Implementation Roadmap

### Phase A: Preparation (No Threading Yet)

1. **Separate decision from mutation** - Refactor creature update to output action intent
2. **Implement deferred births** - Test with single thread first
3. **Add conflict resolution** - Handle simultaneous interactions deterministically

### Phase B: Basic Threading

4. **Add thread pool** - Use `std::thread` or `std::async` with fixed thread count
5. **Parallelize decision phase** - Read-only creature updates
6. **Verify determinism** - Compare multi-threaded output to single-threaded

### Phase C: Optimization

7. **Profile and tune** - Find optimal thread count, batch size
8. **Spatial index optimization** - Consider immutable snapshot approach
9. **Cache optimization** - Ensure creature data is cache-friendly

---

## Estimated Performance Impact

Based on profiling analysis:

| Population | Single-Thread | 4 Threads (Est.) | Speedup |
|------------|---------------|------------------|---------|
| 100 | 2ms | 1.5ms | 1.3x |
| 500 | 10ms | 4ms | 2.5x |
| 1000 | 22ms | 7ms | 3.1x |
| 2000 | 48ms | 14ms | 3.4x |

> [!WARNING]
> Estimates assume ~70% of work is parallelizable (decision phase).
> Actual speedup depends on conflict resolution overhead.

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Non-determinism bugs | High | Extensive comparison testing |
| Race conditions | High | Strict read-only Phase 1 |
| Overhead exceeds gains | Medium | Profile before committing |
| Complexity for contributors | Medium | Clear documentation, abstractions |

---

## When to Implement

Consider implementing when:
- [ ] Target population regularly exceeds 1000 creatures
- [ ] Single-threaded frame time exceeds 30ms
- [ ] User feedback indicates performance issues
- [ ] Development resources available for complexity

---

## References

- Performance analysis from optimization work (2026-01)
- [[../../technical/systems/spatial-index|Spatial Index System]] - Current implementation
- [[../../technical/systems/behavior-system|Behavior System]] - Creature decision logic

---

## See Also

**Infrastructure:**
- [[logger-statistics]] - Logging system improvements
- [[nlp-integration]] - Balance optimization tools

**Technical:**
- [[../../technical/systems/spatial-index]] - Spatial indexing for neighbor queries
- [[../../technical/core/architecture]] - Overall system architecture
