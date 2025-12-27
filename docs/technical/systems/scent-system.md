---
title: Scent System
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: system
tags: [genetics, scent, olfactory, sensory, world]
---

# Scent System Technical Documentation

**Status:** Implemented (Phase 1 Complete)  
**Version:** 1.0  
**Last Updated:** 2025-12-23

---

## Overview

The scent system provides a sparse, performance-optimized storage layer for olfactory communication between creatures. It enables creatures to find mates, mark territory, and communicate danger through pheromone-like scent trails.

### Key Features

- **Sparse storage**: Only tiles with active scents consume memory
- **Multiple scent types**: Breeding, territorial, alarm, food trail, predator marks
- **Genetic signatures**: 8-dimensional fingerprints for individual recognition
- **Performance target**: <1ms overhead for 250,000 tile maps
- **Batch decay**: Processing every 10 ticks for efficiency

---

## Architecture

### File Locations

| File | Purpose |
|------|---------|
| `include/world/ScentLayer.hpp` | Header with class definitions |
| `src/world/ScentLayer.cpp` | Implementation |
| `include/genetics/defaults/UniversalGenes.hpp` | Olfactory gene definitions |

### Class Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        ScentLayer                            │
├─────────────────────────────────────────────────────────────┤
│ - _width: int                                                │
│ - _height: int                                               │
│ - _decayInterval: unsigned int                               │
│ - _lastDecayTick: unsigned int                               │
│ - _scents: unordered_map<pair<int,int>, vector<ScentDeposit>>│
├─────────────────────────────────────────────────────────────┤
│ + deposit(x, y, ScentDeposit)                                │
│ + getScentsAt(x, y): vector<ScentDeposit>                    │
│ + getScentsOfType(x, y, type): vector<ScentDeposit>          │
│ + getStrongestScentInRadius(cx, cy, radius, type): ScentDeposit│
│ + getScentsInRadius(cx, cy, radius, type): vector<tuple>     │
│ + update(currentTick)                                        │
│ + clear()                                                    │
│ + removeScentsFromCreature(creatureId)                       │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                      ScentDeposit                            │
├─────────────────────────────────────────────────────────────┤
│ + type: ScentType                                            │
│ + creatureId: int                                            │
│ + intensity: float [0.0-1.0]                                 │
│ + signature: array<float, 8>                                 │
│ + tickDeposited: unsigned int                                │
│ + decayRate: unsigned int                                    │
├─────────────────────────────────────────────────────────────┤
│ + isDecayed(currentTick): bool                               │
│ + getDecayedIntensity(currentTick): float                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Scent Types

```cpp
enum class ScentType {
    MATE_SEEKING,      // Breeding pheromones for mate attraction
    TERRITORIAL,       // Area marking to claim territory
    ALARM,            // Danger signals to warn others
    FOOD_TRAIL,       // Path markers to food sources
    PREDATOR_MARK     // Indicates predator presence
};
```

### Decay Rates by Type

| Scent Type | Typical Decay Rate | Purpose |
|------------|-------------------|---------|
| MATE_SEEKING | 100-200 ticks | Attract potential mates |
| TERRITORIAL | 300-500 ticks | Long-lasting territory markers |
| ALARM | 50-100 ticks | Fast-fading danger warning |
| FOOD_TRAIL | 150-250 ticks | Lead others to food |
| PREDATOR_MARK | 300-500 ticks | Warn of predator territory |

---

## Olfactory Genes

The simulation includes 4 implemented olfactory genes that control scent behavior:

### Gene Definitions

| Gene ID | Range | Creep | Default | Description |
|---------|-------|-------|---------|-------------|
| `scent_production` | [0.0, 1.0] | 0.05 | 0.3 | Rate of general scent emission |
| `scent_signature_variance` | [0.0, 1.0] | 0.05 | 0.5 | How much signature varies from genotype |
| `olfactory_acuity` | [0.0, 1.0] | 0.05 | 0.4 | Ability to distinguish scent types |
| `scent_masking` | [0.0, 1.0] | 0.05 | 0.2 | Reduce detectability of own scent |

### Gene Constants

From `UniversalGenes.hpp` (lines 245-255):

```cpp
/// Rate of general scent emission [0.0, 1.0], creep 0.05
static constexpr const char* SCENT_PRODUCTION = "scent_production";

/// How much genetic signature varies from genotype [0.0, 1.0], creep 0.05
static constexpr const char* SCENT_SIGNATURE_VARIANCE = "scent_signature_variance";

/// Ability to distinguish different scent types [0.0, 1.0], creep 0.05
static constexpr const char* OLFACTORY_ACUITY = "olfactory_acuity";

/// Reduce own scent detectability [0.0, 1.0], creep 0.05
static constexpr const char* SCENT_MASKING = "scent_masking";
```

### Related Gene (Pre-existing)

The `scent_detection` gene (in Heterotrophy category) was already present and works with the new olfactory genes:

```cpp
/// Smell food sources from distance [0.0, 1.0], creep 0.05
static constexpr const char* SCENT_DETECTION = "scent_detection";
```

---

## Genetic Signatures

### Purpose

Each creature has a unique 8-dimensional genetic signature derived from their genome. This enables:

- **Individual recognition**: Creatures can identify specific individuals
- **Subspeciation**: Similar signatures indicate genetic relatedness
- **Mate preference**: Creatures prefer mates with similar signatures

### Signature Computation

The signature is computed from key genes:

```cpp
std::array<float, 8> computeSignature(const Genome& genome) {
    std::array<float, 8> sig;
    
    sig[0] = genome.getTrait("max_size") * 0.7 + 
             genome.getTrait("metabolism_rate") * 0.3;
    sig[1] = genome.getTrait("color_hue") / 360.0f;
    sig[2] = genome.getTrait("locomotion");
    sig[3] = genome.getTrait("plant_digestion_efficiency");
    sig[4] = genome.getTrait("meat_digestion_efficiency");
    sig[5] = genome.getTrait("toxin_tolerance");
    sig[6] = genome.getTrait("temp_tolerance_low") / 60.0f + 0.5f;
    sig[7] = genome.getTrait("offspring_count") / 20.0f;
    
    // Add variance based on signature_variance gene
    float variance = genome.getTrait("scent_signature_variance");
    for (int i = 0; i < 8; i++) {
        sig[i] += randomGaussian(0, variance * 0.1f);
        sig[i] = clamp(sig[i], 0.0f, 1.0f);
    }
    
    return sig;
}
```

### Similarity Calculation

Signatures are compared using Euclidean distance:

```cpp
float similarityTo(const std::array<float, 8>& other) const {
    float sumSquaredDiff = 0.0f;
    for (int i = 0; i < 8; i++) {
        float diff = signature[i] - other[i];
        sumSquaredDiff += diff * diff;
    }
    // Euclidean distance normalized to [0,1] similarity
    float distance = sqrt(sumSquaredDiff / 8.0f);
    return 1.0f - distance;
}
```

---

## Sparse Storage Implementation

### Hash Function

The scent layer uses a custom hash function for coordinate pairs:

```cpp
struct TileHash {
    std::size_t operator()(const std::pair<int, int>& coords) const {
        // Cantor pairing function for coordinate hashing
        auto x = static_cast<std::size_t>(coords.first);
        auto y = static_cast<std::size_t>(coords.second);
        return ((x + y) * (x + y + 1)) / 2 + y;
    }
};

std::unordered_map<std::pair<int, int>, std::vector<ScentDeposit>, TileHash> _scents;
```

### Memory Efficiency

Only tiles with active scents are stored:

```
World size: 500x500 = 250,000 tiles
Typical scent coverage: ~5% of tiles = 12,500 tiles
Average scents per tile: 1-3

Memory usage:
  - Without sparse storage: 250,000 * sizeof(vector) = ~6 MB minimum
  - With sparse storage: 12,500 * (vector overhead + 1-3 deposits)
                       = ~1.5 MB typical
```

---

## Performance Characteristics

### Time Complexity

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Deposit | O(1) average | Hash map insertion |
| Query single tile | O(k) | k = scents on tile (typically 1-5) |
| Query radius | O(r² × k) | r = radius, k = scents per tile |
| Decay update | O(n) | n = total active scents |

### Optimization: Batch Decay

Decay is processed every 10 ticks instead of every tick:

```cpp
void ScentLayer::update(unsigned int currentTick) {
    if (currentTick - _lastDecayTick >= _decayInterval) {
        processDecay(currentTick);
        _lastDecayTick = currentTick;
    }
}
```

This reduces decay processing overhead by 90%.

---

## Usage Examples

### Depositing a Scent

```cpp
// Creature deposits breeding pheromone
void Creature::depositBreedingScent(ScentLayer& scentLayer, unsigned int tick) {
    float production = getPhenotype()->getTrait("scent_production");
    
    if (production > 0.1f) {
        ScentDeposit deposit(
            ScentType::MATE_SEEKING,
            getId(),
            production,          // intensity based on gene
            getGeneticSignature(),
            tick,
            200                  // decay over 200 ticks
        );
        
        scentLayer.deposit(getX(), getY(), deposit);
    }
}
```

### Detecting Scents

```cpp
// Creature looks for mate scents nearby
std::optional<Direction> Creature::detectMateDirection(
    const ScentLayer& scentLayer, 
    unsigned int currentTick) 
{
    float acuity = getPhenotype()->getTrait("olfactory_acuity");
    int detectionRange = static_cast<int>(30.0f * acuity);
    
    int scentX, scentY;
    ScentDeposit strongest = scentLayer.getStrongestScentInRadius(
        getX(), getY(), detectionRange,
        ScentType::MATE_SEEKING,
        scentX, scentY
    );
    
    if (strongest.intensity > 0.0f && strongest.creatureId != getId()) {
        // Calculate direction to scent source
        int dx = scentX - getX();
        int dy = scentY - getY();
        return vectorToDirection(dx, dy);
    }
    
    return std::nullopt;
}
```

---

## Integration with World

The ScentLayer is integrated with the World class:

```cpp
// In World class (world.hpp)
class World {
private:
    ScentLayer _scentLayer;
    
public:
    void initialize(int width, int height) {
        // ... terrain initialization ...
        _scentLayer.initialize(width, height);
    }
    
    void update(unsigned int currentTick) {
        // ... other updates ...
        _scentLayer.update(currentTick);
    }
    
    ScentLayer& getScentLayer() { return _scentLayer; }
    const ScentLayer& getScentLayer() const { return _scentLayer; }
};
```

---

## Debugging and Visualization

### Getting Statistics

```cpp
// Check scent layer activity
size_t activeTiles = scentLayer.getActiveTileCount();
size_t totalScents = scentLayer.getTotalScentCount();

std::cout << "Active tiles: " << activeTiles << std::endl;
std::cout << "Total scents: " << totalScents << std::endl;
std::cout << "Avg scents/tile: " << (float)totalScents / activeTiles << std::endl;
```

### Clearing Scents

```cpp
// Clear all scents (e.g., for debugging)
scentLayer.clear();

// Remove scents from a specific creature (e.g., when they die)
scentLayer.removeScentsFromCreature(creature.getId());
```

---

## Future Extensions

See `docs/future/sensory-system-phases.md` for planned expansions including:

- **Phase 3**: Auditory system (mating calls, predator detection)
- **Phase 4**: Exploration memory (avoid backtracking)
- **Phase 5**: Full mating integration using all sensory systems
- **Phase 6**: Advanced predator-prey scent tracking

---

## References

- **Design Document**: `plans/mating-behavior-system-design.md`
- **Source Files**: `include/world/ScentLayer.hpp`, `src/world/ScentLayer.cpp`
- **Gene Definitions**: `include/genetics/defaults/UniversalGenes.hpp`
- **Tests**: `src/testing/genetics/test_scent_layer.cpp`, `test_scent_navigation.cpp`

## See Also

**Core Documentation:**
- [[../core/01-architecture]] - System architecture overview
- [[../core/02-getting-started]] - Quick start tutorial
- [[../core/03-extending]] - Extension guide

**Reference:**
- [[../reference/quick-reference]] - Quick reference cheat sheet
- [[../reference/genes]] - Complete gene catalog (olfactory genes section)
- [[../reference/api/core-classes]] - Core API reference
- [[../reference/api/interactions]] - Interaction APIs
- [[../reference/api/perception-system]] - PerceptionSystem API reference

**Design:**
- [[../design/coevolution-design]] - Coevolution design (scent-related interactions)

---

## PerceptionSystem Integration

The `PerceptionSystem` class provides organism-agnostic perception utilities that work through the `IGeneticOrganism` interface.

### File Locations

| File | Purpose |
|------|---------|
| [`include/genetics/systems/PerceptionSystem.hpp`](../../../include/genetics/systems/PerceptionSystem.hpp) | Header with class definitions |
| [`src/genetics/systems/PerceptionSystem.cpp`](../../../src/genetics/systems/PerceptionSystem.cpp) | Implementation |

### Key Methods

| Method | Purpose |
|--------|---------|
| `buildScentSignature()` | Creates 8-element signature from phenotype traits |
| `depositScent()` | Unified scent emission for any organism |
| `detectFoodDirection()` | Scent-based food finding |
| `isEdibleScent()` | Phenotype-based diet compatibility checking |
| `calculateVisualRange()` | Visual detection range from traits |
| `calculateScentRange()` | Scent detection range (scent_detection × 100 tiles) |

### 8-Element Scent Signature Array

| Index | Trait | Normalization |
|-------|-------|---------------|
| [0] | nutrient_value | ÷ 100 |
| [1] | fruit_appeal | direct |
| [2] | toxicity | direct |
| [3] | hardiness | direct |
| [4] | scent_production | direct |
| [5] | color_hue | direct |
| [6] | size_gene | direct |
| [7] | reserved | 0.0f |

### Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `MIN_SCENT_PRODUCTION` | 0.05f | Minimum to emit scent |
| `DEFAULT_DECAY_RATE` | 50 | Ticks until scent decays |
| `SCENT_RANGE_MULTIPLIER` | 100.0f | scent_detection to tiles |

---

## Plant Scent Emission

Plants emit FOOD_TRAIL scents when they have sufficient scent production capability.

### Emission Logic

From [`src/world/world.cpp:410-434`](../../../src/world/world.cpp:410):

Plants emit scent during `World::updateGeneticsPlants()`:
- Checks if `getScentProductionRate() > 0.01f` (emission threshold)
- Intensity scales with maturity: `scentRate × (currentSize / maxSize)`
- Uses `ScentType::FOOD_TRAIL` with 50-tick decay rate

### Plant SCENT_PRODUCTION Values

| Plant Type | SCENT_PRODUCTION Range | Emits Scent? |
|------------|------------------------|--------------|
| Berry Bush | 0.3 - 0.5 | ✓ Yes (fragrant fruit) |
| Oak Tree | 0.02 - 0.08 | ✓ Minimal |
| Grass | 0.0 - 0.01 | ✗ No (below threshold) |
| Thorn Bush | 0.1 - 0.2 | ✓ Yes (moderate) |

> [!NOTE]
> Plants with `scent_production < 0.01` do not emit FOOD_TRAIL scents.
> Creatures must use visual detection (color_vision) to find them.

### Scent Intensity Formula

```
intensity = scent_production × (current_size / max_size)
```

Young plants emit weaker scents; mature plants emit stronger scents.

---

## Creature Scent-Based Food Detection

Creatures use a dual-pathway detection system:

1. **Visual search first** - Spiral search using `forEachTileInRange()`
2. **Scent fallback** - If visual fails and creature has scent ability

### Scent Fallback Implementation

From [`src/objects/creature/creature.cpp:968-1063`](../../../src/objects/creature/creature.cpp:968):

- `hasScentDetection()` - Checks if `scent_detection > 0.1f`
- `findFoodScent()` - Queries ScentLayer for strongest FOOD_TRAIL in range
- Detection range: `100 + (scentDetection × 100)` tiles

### Detection Range Formulas

From [`src/genetics/interactions/FeedingInteraction.cpp:189-210`](../../../src/genetics/interactions/FeedingInteraction.cpp:189):

**Visual Detection:**
```
visualBonus = colorVision × fruitAppeal × 100
effectiveRange = sightRange + max(visualBonus, scentBonus)
```

**Scent Detection:**
```
scentBonus = scentDetection × 100
```

### Example Scenarios

| Creature Traits | Berry Bush (appeal=0.8) | Grass (appeal=0.1) |
|-----------------|-------------------------|-------------------|
| colorVision=0.9, sight=50 | 50 + (0.9 × 0.8 × 100) = 122 tiles | 50 + (0.9 × 0.1 × 100) = 59 tiles |
| scentDetection=0.8 | 80 tiles (fallback) | N/A (grass doesn't emit scent) |

### See Also

- [[../reference/genes#9-olfactory-genes]] - Olfactory gene definitions
- [[../reference/api/perception-system]] - PerceptionSystem API reference
