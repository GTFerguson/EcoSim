# Future Sensory System Development Phases

**Status:** Planned (Not Yet Implemented)  
**Last Updated:** 2025-12-23  
**Reference:** [[behavior/mating-behavior-system-design]]

---

## Overview

This document describes planned expansions to the sensory system beyond the currently implemented olfactory (scent) system. These phases will add auditory capabilities, spatial memory, and advanced predator-prey dynamics.

> **Note:** Phase 1 (Olfactory/Scent System) and Phase 2 (Olfactory Genes) are **already implemented**. See `docs/technical/genetics/scent-system.md` for current implementation details.

---

## Phase 3: Auditory System (Hearing)

### Planned Gene Definitions

| Gene ID | Category | Range | Creep | Default | Description |
|---------|----------|-------|-------|---------|-------------|
| `hearing_acuity` | Behavior | [0.0, 1.0] | 0.05 | 0.5 | General hearing sensitivity |
| `hearing_range_low` | Behavior | [20, 300] | 10 | 50 | Minimum frequency heard (Hz) |
| `hearing_range_high` | Behavior | [300, 5000] | 100 | 2000 | Maximum frequency heard (Hz) |
| `sound_production` | Behavior | [0.0, 1.0] | 0.05 | 0.3 | Loudness of vocalizations |
| `acoustic_stealth` | Behavior | [0.0, 1.0] | 0.05 | 0.3 | Reduce sound detectability |
| `vocalization_frequency` | Reproduction | [20, 5000] | 100 | 500 | Frequency of own vocalizations (Hz) |

### Frequency-Based Hearing

Each creature will have a frequency window in which they can hear sounds:

```
Creature A: hearing_range = [50Hz, 2000Hz]
Creature B: hearing_range = [200Hz, 1500Hz]  
Creature C (bat-like): hearing_range = [1000Hz, 4000Hz]

Sound at 500Hz:
  - A can hear it (within 50-2000)
  - B can hear it (within 200-1500)
  - C cannot hear it (outside 1000-4000)
```

**Evolutionary Implications:**
- Predators and prey can evolve non-overlapping frequency ranges
- "Stealth calls" at frequencies predators can't hear
- Subspecies may use different frequency bands (like dialects)

### Sound Layer Architecture

Unlike scents, sounds will be **transient** - they exist only during emission:

```cpp
// NOT stored in tiles - processed immediately each tick
struct SoundEmission {
    int sourceX, sourceY;
    int sourceCreatureId;
    float loudness;           // 0.0-1.0 from sound_production gene
    float frequency;          // Hz, from call_frequency or other genes
    ScentSignature signature; // Genetic signature (same as scent)
    SoundType type;           // MATING_CALL, ALARM, TERRITORIAL, etc.
};

// In World class:
std::vector<SoundEmission> _activeSounds;  // Cleared each tick
```

### Sound Attenuation Model

```
Perceived Intensity = (loudness / (distance/50 + 1)) * freq_sensitivity * hearing_acuity

Examples (loudness=1.0, perfect frequency, acuity=1.0):
  distance = 0:    intensity = 1.0 (right on top)
  distance = 50:   intensity = 0.5 (half intensity)
  distance = 150:  intensity = 0.25 (quarter intensity)
  distance = 250:  intensity = 0.17 (faint)
```

---

## Phase 4: Exploration Memory

### Data Structure

```cpp
struct ExplorationMemory {
    static constexpr int MEMORY_SIZE = 100;      // Remember last 100 locations
    static constexpr int FORGET_AFTER_TICKS = 500;  // Forget after 500 ticks
    
    struct Visit {
        int x, y;
        int tickVisited;
    };
    
    Visit locations[MEMORY_SIZE];
    int nextIndex = 0;   // Circular buffer write position
    int visitCount = 0;  // How many locations stored (up to MEMORY_SIZE)
};
```

### Planned Genes

| Gene ID | Category | Range | Creep | Default | Description |
|---------|----------|-------|-------|---------|-------------|
| `exploration_drive` | Mobility | [0.0, 1.0] | 0.05 | 0.5 | Drive to explore unvisited areas |
| `path_memory` | Mobility | [0.0, 1.0] | 0.05 | 0.5 | Spatial memory of visited locations |
| `movement_persistence` | Mobility | [0.0, 1.0] | 0.05 | 0.5 | Persistence in pursuing movement goals |

### Memory-Guided Exploration

Use memory to bias movement toward unexplored areas:

```cpp
Direction exploreUnvisited(const Creature& creature, const World& world, int currentTick) {
    // Sample potential directions
    // Calculate exploration score based on recency weight
    // Higher score for unvisited areas
    // Weighted by exploration_drive gene
}
```

### Memory Capacity Trade-offs

```
Memory Effectiveness = base_memory * path_memory_gene

Where:
  base_memory = 100 locations
  path_memory_gene ∈ [0, 1]
  
Examples:
  path_memory = 0.2 → effectively remember 20 locations
  path_memory = 0.5 → remember 50 locations
  path_memory = 1.0 → full 100 locations
  
Energy Cost = 0.005 * path_memory per tick
```

---

## Phase 5: Full Mating Integration

### Priority-Based Mate Finding

Using sensory foundations, the improved `findMate()` algorithm will follow this priority order:

1. **Check for alarm signals** (predator nearby)
   - If detected: Abort mating, flee

2. **Listen for mating calls** within hearing range
   - Navigate toward most attractive call (intensity × genetic_similarity)

3. **Follow breeding pheromone trails**
   - Detect `ScentType::MATE_SEEKING` scents
   - Weight by intensity, distance, and genetic similarity
   - Follow strongest gradient

4. **Visual mate search** (existing behavior)
   - Look for visible mates within sight_range
   - Use A* pathfinding

5. **Broadcast own mating call** (if call_rate threshold met)
   - Costs energy, attracts predators

6. **Deposit breeding pheromone trail**
   - Costs energy

7. **Exploratory movement** with path memory
   - Use `exploreUnvisited()` to search new areas
   - Weighted by exploration_drive gene

8. **Fallback to random wander**

### Mating-Specific Genes

| Gene ID | Category | Range | Creep | Default | Description |
|---------|----------|-------|-------|---------|-------------|
| `call_frequency` | Reproduction | [20, 5000] | 100 | 500 | Frequency of mating calls (Hz) |
| `call_rate` | Reproduction | [0.0, 1.0] | 0.05 | 0.3 | How often to broadcast calls |
| `mate_pheromone_strength` | Reproduction | [0.0, 1.0] | 0.05 | 0.5 | Strength of breeding scent |

### Energy Costs

```
Total Mating Behavior Cost Per Tick:

Base metabolism:              0.001 * metabolism_rate
Movement (if moving):         0.015 * distance
Mating call (if active):      0.05 * sound_production² * call_rate
Breeding pheromone:           0.01 * mate_pheromone_strength
Exploration bonus:            0.015 * exploration_drive
Path memory maintenance:      0.005 * path_memory

TOTAL RANGE: 0.001 - 0.12 energy/tick
```

---

## Phase 6: Predator-Prey Sensory Applications

### Predator Hunting with Scent Tracking

**Scenario:** Carnivore hunts herbivore using scent trails

```cpp
bool Predator::huntPrey(World& world, int currentTick) {
    // 1. Check for prey scent trails
    auto scents = world.detectScents(...);
    
    if (!scents.empty()) {
        // Follow strongest prey scent
        Direction dir = followScentGradient(...);
        Navigator::moveDirection(*this, world, dir);
        return true;
    }
    
    // 2. Listen for prey sounds (rustling, calls)
    auto sounds = world.detectSounds(...);
    
    // Filter for prey sounds
    for (const auto& sound : sounds) {
        if (sound.type == SoundType::MOVEMENT || 
            sound.type == SoundType::ALARM) {
            Direction dir = moveTowardSound(*this, sound);
            Navigator::moveDirection(*this, world, dir);
            return true;
        }
    }
    
    // 3. Visual search (existing behavior)
    return findPrey(...);
}
```

### Prey Predator Avoidance

**Scenario:** Herbivore detects approaching predator

```cpp
bool Prey::detectPredators(World& world) {
    // 1. Smell predator scent
    auto scents = world.detectScents(..., {ScentType::PREDATOR_MARK}, ...);
    
    if (!scents.empty()) {
        // Flee in opposite direction
        Direction threatDir = followScentGradient(...);
        Direction fleeDir = oppositeDirection(threatDir);
        
        setProfile(Profile::flee);
        Navigator::moveDirection(*this, world, fleeDir);
        
        // Emit alarm scent to warn others
        depositAlarmScent(world);
        return true;
    }
    
    // 2. Hear predator sounds
    auto sounds = world.detectSounds(...);
    // Similar logic for sound-based detection
    
    return false;
}
```

### Alarm Cascades

Prey species can create alarm cascades through hearing and scent:

```
Predator approaches → Prey A detects via scent
                   → Prey A emits alarm scent + alarm call
                   → Prey B hears alarm call
                   → Prey B flees and emits own alarm
                   → Prey C smells alarm scent
                   → Prey C flees and emits alarm
                   → Cascade spreads through population
```

### Coevolutionary Dynamics

**Arms Race Emergence:**
- Predators evolve higher `scent_detection` and `hearing_acuity`
- Prey evolve `scent_masking` and `acoustic_stealth`
- Natural arms race emerges from sensory capabilities

---

## Implementation Timeline

| Phase | Description | Estimated Duration | Dependencies |
|-------|-------------|-------------------|--------------|
| Phase 3 | Auditory System | 2-3 weeks | None (parallel to scent) |
| Phase 4 | Exploration Memory | 1-2 weeks | None |
| Phase 5 | Full Mating Integration | 2 weeks | Phases 3, 4 |
| Phase 6 | Predator-Prey Applications | 2 weeks | All previous |

---

## Success Metrics

### Sensory System Metrics

| Metric | Target |
|--------|--------|
| Sound Attenuation | Realistic distance falloff curve |
| Frequency Filtering | > 90% accuracy (only hear in range) |
| Memory Recall | > 90% (correctly remember visited locations) |

### Behavior Metrics

| Metric | Current | Target |
|--------|---------|--------|
| Mating Success Rate | ~75% (with scent) | > 85% (with sound + scent) |
| Predator Hunt Success | N/A | > 40% |
| Prey Survival | N/A | > 60% |
| Alarm Cascade Range | N/A | 5-10 tiles |

### Evolutionary Metrics

| Metric | Target | Timeframe |
|--------|--------|-----------|
| Subspeciation Emergence | 2-5 clusters | 500-1000 generations |
| Arms Race | Observable predator/prey coevolution | 500+ generations |
| Strategy Diversity | 4+ distinct strategies | 300+ generations |

---

## Future Extensions (Beyond Phase 6)

### Additional Sensory Modalities

- **Electroreception** (like sharks): Detect bioelectric fields in water
- **Magnetoreception** (like birds): Navigate using magnetic fields
- **Infrared Vision** (like pit vipers): Detect heat signatures

### Complex Communication

- **Composite Signals**: Multi-modal displays (visual + acoustic + chemical)
- **Social Learning**: Cultural transmission of call "dialects"
- **Syntax and Semantics**: Sequences of calls with meaning

### Environmental Sensing

- **Weather Prediction**: Barometric pressure sensing
- **Resource Mapping**: Mental maps of food/water locations
- **Territorial Memory**: Across generations

---

## References

- **Full Design Document:** `plans/mating-behavior-system-design.md`
- **Current Scent Implementation:** `docs/technical/genetics/scent-system.md`
- **ScentLayer Source:** `include/world/ScentLayer.hpp`, `src/world/ScentLayer.cpp`
- **Universal Genes:** `include/genetics/defaults/UniversalGenes.hpp`
