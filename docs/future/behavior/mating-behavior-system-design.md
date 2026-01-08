# Sensory-Based Movement and Behavior System Design

**Version:** 2.0  
**Date:** 2025-12-23  
**Author:** Architecture Mode  
**Status:** Design Review - Revised with Sensory Foundations

---

## Executive Summary

This design introduces **general-purpose sensory systems** that enable creatures to perceive and navigate their environment through smell and sound. These foundational systems support multiple behaviors including mating, hunting, foraging, and predator avoidance.

**Key Innovation**: Rather than treating pheromones and calls as mating-specific hacks, we build reusable **olfactory** and **auditory** systems that any behavior can leverage.

### Core Systems

1. **Olfactory System (Smell)** - General scent production, detection, and gradient following
2. **Auditory System (Hearing)** - Frequency-based sound emission and detection with attenuation
3. **Signal Signatures** - Genetic-based signal composition enabling subspeciation
4. **Spatial Memory** - Track visited locations for efficient exploration

### Applications Built on These Foundations

- **Mating Communication** - Find mates via scent trails and mating calls
- **Predator-Prey Dynamics** - Predators track prey scent; prey hear approaching predators
- **Foraging** - Creatures smell food sources and remember productive areas
- **Territory** - Mark and recognize territorial boundaries

---

## 1. Problem Analysis

### Current Breeding Failure Modes

From [`breedingDiagnostic.cpp`](../src/testing/breedingDiagnostic.cpp):

```
Reason: none_in_sight_range: 98.7% of failures
Average distance to nearest potential mate: 127.3 tiles
Average creature sight range: 60-120 tiles
Map size: 500x500 tiles
```

### Root Cause: Inadequate Sensory Systems

The breeding problem is a symptom of a deeper issue: **creatures lack the sensory capabilities to perceive their environment beyond visual range**. To solve breeding, we need general sensory foundations that also enable:

- Predators tracking prey over long distances
- Prey detecting approaching threats
- Foragers locating food sources
- Territory establishment and recognition

---

## 2. System Architecture

### 2.1 Layered Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   BEHAVIOR LAYER                            │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │  Mating  │  │ Hunting  │  │ Foraging │  │Territory │  │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘  │
└───────┼─────────────┼─────────────┼─────────────┼─────────┘
        │             │             │             │
┌───────▼─────────────▼─────────────▼─────────────▼─────────┐
│              SENSORY FOUNDATION LAYER                      │
│  ┌──────────────────────┐  ┌──────────────────────┐       │
│  │  Olfactory System    │  │  Auditory System     │       │
│  │  - Scent production  │  │  - Sound emission    │       │
│  │  - Scent detection   │  │  - Frequency hearing │       │
│  │  - Gradient following│  │  - Direction finding │       │
│  └──────────┬───────────┘  └──────────┬───────────┘       │
└─────────────┼──────────────────────────┼───────────────────┘
              │                          │
┌─────────────▼──────────────────────────▼───────────────────┐
│              ENVIRONMENT LAYER                             │
│  ┌─────────────────┐  ┌─────────────────┐                 │
│  │  Scent Layer    │  │  Sound Layer    │                 │
│  │  (tile-based)   │  │  (transient)    │                 │
│  └─────────────────┘  └─────────────────┘                 │
└────────────────────────────────────────────────────────────┘
```

### 2.2 Design Principles

1. **General Purpose**: Sensory systems serve multiple behaviors, not just mating
2. **Gene-Driven**: All sensory capabilities emerge from genetic traits
3. **Signal Diversity**: Multiple signal types (mating, territory, alarm, food)
4. **Genetic Signatures**: Signals carry genetic "fingerprints" for subspeciation
5. **Layered Storage**: Separate scent and sound layers for efficient filtering

---

## 3. Olfactory System (Smell)

### 3.1 Gene Definitions

#### Core Olfactory Genes

| Gene ID | Category | Range | Creep | Default (Creature) | Description |
|---------|----------|-------|-------|-------------------|-------------|
| `scent_detection` | Heterotrophy | [0.0, 1.0] | 0.05 | 0.5 | **Existing gene** - General smell sensitivity |
| `scent_production` | Behavior | [0.0, 1.0] | 0.05 | 0.3 | Rate of general scent emission |
| `scent_signature_variance` | Reproduction | [0.0, 1.0] | 0.05 | 0.5 | How much signature varies from genotype |
| `olfactory_acuity` | Behavior | [0.0, 1.0] | 0.05 | 0.4 | Ability to distinguish scent types |
| `scent_masking` | Behavior | [0.0, 1.0] | 0.05 | 0.2 | Reduce detectability of own scent |

**Notes:**
- `scent_detection` already exists in [`UniversalGenes.hpp`](../include/genetics/defaults/UniversalGenes.hpp) line 200
- We build upon this foundation with production and signature genes
- `scent_production` controls all scent types (general system)

### 3.2 Scent Types and Purpose

Scents are **layered** - creatures can emit and detect different types:

| Scent Type | Purpose | Typical Emitters | Decay Rate |
|------------|---------|------------------|------------|
| **Mate Seeking** | Advertise breeding readiness | Breeding creatures | Fast (0.03/tick) |
| **Territorial** | Mark owned territory | Territorial creatures | Slow (0.01/tick) |
| **Alarm** | Warn of predators | Prey under threat | Very fast (0.05/tick) |
| **Food Trail** | Lead to food sources | Successful foragers | Medium (0.02/tick) |
| **Predator Mark** | Mark hunting ground | Predators | Slow (0.01/tick) |

**Design Note**: Each creature only detects scent types relevant to their behavior state and genes.

### 3.3 Scent Signature System

#### Genetic Fingerprinting

Each creature's scent carries a **genetic signature** derived from its genome:

```cpp
struct ScentSignature {
    float signature[8];  // 8-dimensional genetic fingerprint
    
    // Compute from creature's genome
    static ScentSignature computeSignature(const Genome& genome) {
        ScentSignature sig;
        // Hash key genes into signature dimensions
        sig.signature[0] = genome.getTrait("max_size") * 0.7 + 
                          genome.getTrait("metabolism_rate") * 0.3;
        sig.signature[1] = genome.getTrait("color_hue") / 360.0;
        sig.signature[2] = genome.getTrait("locomotion");
        sig.signature[3] = genome.getTrait("plant_digestion_efficiency");
        sig.signature[4] = genome.getTrait("meat_digestion_efficiency");
        sig.signature[5] = genome.getTrait("toxin_tolerance");
        sig.signature[6] = genome.getTrait("temp_tolerance_low") / 60.0 + 0.5;
        sig.signature[7] = genome.getTrait("offspring_count") / 20.0;
        
        // Add variance based on signature_variance gene
        float variance = genome.getTrait("scent_signature_variance");
        for (int i = 0; i < 8; i++) {
            sig.signature[i] += randomGaussian(0, variance * 0.1);
            sig.signature[i] = clamp(sig.signature[i], 0.0, 1.0);
        }
        
        return sig;
    }
    
    // Calculate similarity between two signatures (0.0 = different, 1.0 = identical)
    float similarityTo(const ScentSignature& other) const {
        float sumSquaredDiff = 0.0f;
        for (int i = 0; i < 8; i++) {
            float diff = signature[i] - other.signature[i];
            sumSquaredDiff += diff * diff;
        }
        // Euclidean distance normalized to [0,1] similarity
        float distance = sqrt(sumSquaredDiff / 8.0);
        return 1.0 - distance;
    }
};
```

#### Attraction Based on Similarity

Creatures are **more attracted to signals from genetically similar individuals**:

```
Attraction Multiplier = base_attraction * (1.0 + similarity * attraction_bias)

Where:
  similarity ∈ [0, 1] from signature comparison
  attraction_bias = 0.5 (tune based on selection pressure desired)
  
Examples:
  similarity = 1.0 (identical) → 1.5x attraction
  similarity = 0.5 (moderate)  → 1.25x attraction  
  similarity = 0.0 (different) → 1.0x attraction (no penalty)
```

**Subspeciation Mechanism**: Over time, this creates reproductive isolation:
1. Initial population has mixed genotypes
2. Creatures preferentially mate with similar signatures
3. Gene flow between dissimilar groups decreases
4. Distinct genetic clusters emerge (subspecies)
5. Clusters may diverge in appearance, behavior, niche

### 3.4 Scent Layer Architecture

**Tile-Based Storage** (similar to current pheromone design, but generalized):

```cpp
// Add to Tile class (include/world/tile.hpp)
struct ScentData {
    ScentType type;
    float intensity;              // 0.0-1.0
    ScentSignature signature;     // Genetic fingerprint
    int ticksOld;                 // Age for decay
    int sourceCreatureId;         // Who emitted it
};

std::vector<ScentData> _scents;  // Multiple scent layers per tile
```

**World Scent Layer Methods**:

```cpp
// Add to World class (include/world/world.hpp)

/// Update all scents - decay, remove expired
void updateScentLayer(int ticksElapsed);

/// Deposit scent at location
void depositScent(int x, int y, ScentType type, 
                  float intensity, const ScentSignature& sig, 
                  int sourceId);

/// Detect scents within range, filtered by type and acuity
std::vector<ScentSource> detectScents(
    int x, int y, 
    float range, 
    float acuity,
    const std::set<ScentType>& filterTypes,
    const ScentSignature& detectorSig);  // For similarity filtering
```

### 3.5 Scent Gradient Following

#### Direction Finding Algorithm

Creatures determine direction using a **weighted gradient** of nearby scent sources:

```cpp
// In Navigator or Creature class
Direction followScentGradient(
    const Creature& creature,
    ScentType desiredType,
    const std::vector<ScentSource>& detectedScents)
{
    if (detectedScents.empty()) return Direction::none;
    
    Vector2D gradient(0, 0);
    float totalWeight = 0.0f;
    
    // Get creature's own signature for similarity comparison
    ScentSignature mySig = creature.getOwnSignature();
    
    for (const auto& source : detectedScents) {
        if (source.type != desiredType) continue;
        
        // Calculate direction to source
        Vector2D toSource(source.x - creature.getX(), 
                         source.y - creature.getY());
        float distance = toSource.length();
        
        if (distance < 1.0) continue;  // Skip if on top of source
        
        // Calculate weight based on intensity and distance
        float intensityWeight = source.intensity;
        float distanceWeight = 1.0 / (distance + 1.0);
        
        // Add genetic similarity weight
        float similarity = source.signature.similarityTo(mySig);
        float similarityWeight = 1.0 + similarity * 0.5;  // Up to 1.5x
        
        float weight = intensityWeight * distanceWeight * similarityWeight;
        
        // Add weighted direction to gradient
        toSource.normalize();
        gradient += toSource * weight;
        totalWeight += weight;
    }
    
    if (totalWeight < 0.01f) return Direction::none;
    
    // Average gradient by total weight
    gradient /= totalWeight;
    
    // Convert to 8-direction
    return vectorToDirection(gradient);
}
```

#### Gradient Visualization

```
Scent concentrations create a spatial gradient:

    [0.1] [0.2] [0.3] [0.4]
    [0.2] [0.4] [0.6] [0.5]
    [0.1] [0.3] [X]   [0.4]  X = Source creature
    [0.0] [0.1] [0.2] [0.3]

Creature at bottom-left:
  - Samples all nearby tiles within detection range
  - Weights by intensity / distance
  - Computes net direction: Northeast (toward X)
```

---

## 4. Auditory System (Hearing)

### 4.1 Gene Definitions

| Gene ID | Category | Range | Creep | Default (Creature) | Description |
|---------|----------|-------|-------|-------------------|-------------|
| `hearing_acuity` | Behavior | [0.0, 1.0] | 0.05 | 0.5 | General hearing sensitivity |
| `hearing_range_low` | Behavior | [20, 300] | 10 | 50 | Minimum frequency heard (Hz) |
| `hearing_range_high` | Behavior | [300, 5000] | 100 | 2000 | Maximum frequency heard (Hz) |
| `sound_production` | Behavior | [0.0, 1.0] | 0.05 | 0.3 | Loudness of vocalizations |
| `call_frequency` | Reproduction | [20, 5000] | 100 | 500 | Frequency of mating calls (Hz) |
| `acoustic_stealth` | Behavior | [0.0, 1.0] | 0.05 | 0.3 | Reduce sound detectability |

### 4.2 Frequency-Based Hearing

#### Hearing Range Windows

Each creature has a **frequency window** in which they can hear sounds:

```
Creature A: hearing_range = [50Hz, 2000Hz]
Creature B: hearing_range = [200Hz, 1500Hz]
Creature C (bat-like): hearing_range = [1000Hz, 4000Hz]

Sound at 500Hz:
  - A can hear it (within 50-2000)
  - B can hear it (within 200-1500)
  - C cannot hear it (outside 1000-4000)
```

**Evolutionary Implications**:
- Predators and prey can evolve non-overlapping frequency ranges
- "Stealth calls" at frequencies predators can't hear
- Subspecies may use different frequency bands (like dialects)

#### Frequency Sensitivity

Creatures hear best in the middle of their range:

```cpp
float calculateHearingSensitivity(float soundFreq, float lowFreq, float highFreq) {
    // Outside range: can't hear at all
    if (soundFreq < lowFreq || soundFreq > highFreq) {
        return 0.0f;
    }
    
    // Calculate relative position in range [0, 1]
    float rangeWidth = highFreq - lowFreq;
    float relativePos = (soundFreq - lowFreq) / rangeWidth;
    
    // Peak sensitivity at center (0.5), falls off toward edges
    // Using bell curve: exp(-8 * (x - 0.5)^2)
    float centerDist = relativePos - 0.5f;
    float sensitivity = exp(-8.0f * centerDist * centerDist);
    
    return sensitivity;  // 1.0 at center, ~0.1 at edges
}
```

### 4.3 Sound Layer Architecture

Unlike scents, **sounds are transient** - they exist only during emission:

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

// Broadcasting a sound
void World::broadcastSound(int x, int y, int sourceId, 
                           float loudness, float frequency,
                           const ScentSignature& sig, SoundType type) {
    _activeSounds.push_back({x, y, sourceId, loudness, frequency, sig, type});
}

// Detecting sounds (called by each creature each tick)
std::vector<HeardSound> World::detectSounds(
    int listenerX, int listenerY,
    float hearingAcuity, float lowFreq, float highFreq,
    const ScentSignature& listenerSig)
{
    std::vector<HeardSound> heard;
    
    for (const auto& sound : _activeSounds) {
        // Check frequency range
        float freqSensitivity = calculateHearingSensitivity(
            sound.frequency, lowFreq, highFreq);
        if (freqSensitivity < 0.1f) continue;  // Can't hear this frequency
        
        // Calculate distance and attenuation
        float distance = calculateDistance(listenerX, listenerY, 
                                          sound.sourceX, sound.sourceY);
        
        // Sound attenuation: intensity = loudness / (distance + 1)
        // Using inverse distance law with offset to prevent infinity at distance=0
        float attenuatedIntensity = sound.loudness / (distance / 50.0 + 1.0);
        
        // Apply frequency sensitivity and hearing acuity
        float perceivedIntensity = attenuatedIntensity * 
                                   freqSensitivity * 
                                   hearingAcuity;
        
        // Threshold check
        if (perceivedIntensity < 0.1f) continue;  // Too faint to hear
        
        // Calculate genetic similarity for attraction
        float similarity = sound.signature.similarityTo(listenerSig);
        
        heard.push_back({
            sound.sourceX, sound.sourceY, sound.sourceId,
            perceivedIntensity, sound.frequency, 
            similarity, sound.type
        });
    }
    
    return heard;
}
```

### 4.4 Sound Attenuation and Directionality

#### Attenuation Model

Sound intensity decreases with distance using a modified inverse-square law:

```
Perceived Intensity = (loudness / (distance/50 + 1)) * freq_sensitivity * hearing_acuity

Where:
  loudness = sound_production gene [0-1]
  distance = tiles between source and listener
  freq_sensitivity = how well listener hears this frequency [0-1]
  hearing_acuity = general hearing ability [0-1]
  
Examples (loudness=1.0, perfect frequency, acuity=1.0):
  distance = 0:    intensity = 1.0 (right on top)
  distance = 50:   intensity = 0.5 (half intensity)
  distance = 150:  intensity = 0.25 (quarter intensity)
  distance = 250:  intensity = 0.17 (faint)
```

#### Direction Finding

Unlike scents, sounds provide **immediate directional information**:

```cpp
Direction moveTowardSound(const Creature& creature, 
                          const HeardSound& sound)
{
    int dx = sound.sourceX - creature.getX();
    int dy = sound.sourceY - creature.getY();
    
    // Convert to 8-direction
    return deltaToDirection(dx, dy);
}

// Hearing multiple sounds: use strongest or most attractive
Direction followBestSound(const Creature& creature,
                          const std::vector<HeardSound>& sounds)
{
    if (sounds.empty()) return Direction::none;
    
    float bestScore = 0.0f;
    HeardSound* bestSound = nullptr;
    
    for (const auto& sound : sounds) {
        // Score combines intensity and genetic similarity
        float score = sound.perceivedIntensity * 
                     (1.0 + sound.similarity * 0.5);
        
        if (score > bestScore) {
            bestScore = score;
            bestSound = &sound;
        }
    }
    
    return moveTowardSound(creature, *bestSound);
}
```

---

## 5. Exploration Memory System

### 5.1 Concrete Data Structure

The vague "path memory" needs a clear implementation:

```cpp
// Add to Creature class (include/objects/creature/creature.hpp)

struct ExplorationMemory {
    // Circular buffer of recently visited tiles
    static constexpr int MEMORY_SIZE = 100;  // Remember last 100 locations
    static constexpr int FORGET_AFTER_TICKS = 500;  // Forget after 500 ticks
    
    struct Visit {
        int x, y;
        int tickVisited;
    };
    
    Visit locations[MEMORY_SIZE];
    int nextIndex = 0;  // Circular buffer write position
    int visitCount = 0; // How many locations stored (up to MEMORY_SIZE)
    
    // Record a visit
    void recordVisit(int x, int y, int currentTick) {
        locations[nextIndex] = {x, y, currentTick};
        nextIndex = (nextIndex + 1) % MEMORY_SIZE;
        if (visitCount < MEMORY_SIZE) visitCount++;
    }
    
    // Check if location was visited recently
    bool wasVisitedRecently(int x, int y, int currentTick) const {
        for (int i = 0; i < visitCount; i++) {
            const Visit& v = locations[i];
            
            // Forget old visits
            if (currentTick - v.tickVisited > FORGET_AFTER_TICKS) continue;
            
            // Check if same location (with tolerance)
            int dx = abs(v.x - x);
            int dy = abs(v.y - y);
            if (dx <= 2 && dy <= 2) {  // Within 2 tiles
                return true;
            }
        }
        return false;
    }
    
    // Get visit recency weight for exploration
    float getRecencyWeight(int x, int y, int currentTick) const {
        int minTicksSinceVisit = FORGET_AFTER_TICKS + 1;
        
        for (int i = 0; i < visitCount; i++) {
            const Visit& v = locations[i];
            int ticksSince = currentTick - v.tickVisited;
            
            if (ticksSince > FORGET_AFTER_TICKS) continue;
            
            int dx = abs(v.x - x);
            int dy = abs(v.y - y);
            if (dx <= 2 && dy <= 2) {
                minTicksSinceVisit = std::min(minTicksSinceVisit, ticksSince);
            }
        }
        
        // Convert to weight: recently visited = low weight
        if (minTicksSinceVisit > FORGET_AFTER_TICKS) {
            return 1.0f;  // Never visited or forgotten
        } else {
            return (float)minTicksSinceVisit / FORGET_AFTER_TICKS;
        }
    }
};

ExplorationMemory _explorationMemory;
```

### 5.2 Memory-Guided Exploration

Use memory to bias movement toward unexplored areas:

```cpp
// In Navigator or findMate/findFood methods
Direction exploreUnvisited(const Creature& creature,
                          const World& world,
                          int currentTick)
{
    int x = creature.getX();
    int y = creature.getY();
    
    // Sample potential directions
    struct DirectionScore {
        Direction dir;
        float score;
    };
    
    DirectionScore scores[8];
    int scoreCount = 0;
    
    for (Direction dir : {Direction::N, Direction::E, Direction::S, Direction::W,
                         Direction::NE, Direction::NW, Direction::SE, Direction::SW}) {
        // Get coordinates in this direction
        int newX = x, newY = y;
        Navigator::applyDirection(newX, newY, dir);
        
        // Check if valid tile
        if (!world.inBounds(newX, newY)) continue;
        if (!world.isPassable(newX, newY)) continue;
        
        // Calculate exploration score
        float recencyWeight = creature.getExplorationMemory()
                             .getRecencyWeight(newX, newY, currentTick);
        
        // Higher score for unvisited areas
        float explorationDrive = creature.getPhenotype()
                                ->getTrait("exploration_drive");
        float score = recencyWeight * explorationDrive;
        
        scores[scoreCount++] = {dir, score};
    }
    
    if (scoreCount == 0) return Direction::none;
    
    // Weighted random selection (not always best, adds randomness)
    float totalScore = 0.0f;
    for (int i = 0; i < scoreCount; i++) {
        totalScore += scores[i].score;
    }
    
    float roll = randomFloat(0, totalScore);
    float cumulative = 0.0f;
    for (int i = 0; i < scoreCount; i++) {
        cumulative += scores[i].score;
        if (roll <= cumulative) {
            return scores[i].dir;
        }
    }
    
    return scores[0].dir;  // Fallback
}
```

### 5.3 Memory Capacity Trade-offs

The `path_memory` gene controls memory effectiveness:

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

## 6. Predator-Prey Sensory Applications

### 6.1 Predator Hunting with Scent Tracking

**Scenario**: Carnivore hunts herbivore using scent trails

```cpp
bool Predator::huntPrey(World& world, int currentTick) {
    // 1. Check for prey scent trails
    auto scents = world.detectScents(
        getX(), getY(),
        getScentDetectionRange(),
        getPhenotype()->getTrait("olfactory_acuity"),
        {ScentType::GENERAL_BODY_SCENT},  // Prey body scent
        getOwnSignature()
    );
    
    if (!scents.empty()) {
        // Follow strongest prey scent
        Direction dir = followScentGradient(*this, 
                                           ScentType::GENERAL_BODY_SCENT, 
                                           scents);
        if (dir != Direction::none) {
            Navigator::moveDirection(*this, world, dir);
            return true;
        }
    }
    
    // 2. Listen for prey sounds (rustling, calls)
    auto sounds = world.detectSounds(
        getX(), getY(),
        getPhenotype()->getTrait("hearing_acuity"),
        getPhenotype()->getTrait("hearing_range_low"),
        getPhenotype()->getTrait("hearing_range_high"),
        getOwnSignature()
    );
    
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
    return findPrey(world.getGrid(), world.rows(), world.cols(), ...);
}
```

**Evolutionary Dynamics**:
- Predators evolve higher `scent_detection` and `hearing_acuity`
- Prey evolve `scent_masking` and `acoustic_stealth`
- Arms race emerges naturally from sensory capabilities

### 6.2 Prey Predator Avoidance

**Scenario**: Herbivore detects approaching predator

```cpp
bool Prey::detectPredators(World& world) {
    // 1. Smell predator scent (predators emit more scent due to meat diet)
    auto scents = world.detectScents(
        getX(), getY(),
        getScentDetectionRange() * 0.8,  // Slightly shorter than predator
        getPhenotype()->getTrait("olfactory_acuity"),
        {ScentType::PREDATOR_MARK},
        getOwnSignature()
    );
    
    if (!scents.empty()) {
        // Flee in opposite direction of predator scent
        Direction threatDir = followScentGradient(*this, 
                                                 ScentType::PREDATOR_MARK, 
                                                 scents);
        Direction fleeDir = oppositeDirection(threatDir);
        
        setProfile(Profile::flee);
        Navigator::moveDirection(*this, world, fleeDir);
        
        // Emit alarm scent to warn others
        depositAlarmScent(world);
        return true;
    }
    
    // 2. Hear predator sounds (footsteps, breathing)
    auto sounds = world.detectSounds(
        getX(), getY(),
        getPhenotype()->getTrait("hearing_acuity"),
        getPhenotype()->getTrait("hearing_range_low"),
        getPhenotype()->getTrait("hearing_range_high"),
        getOwnSignature()
    );
    
    for (const auto& sound : sounds) {
        if (sound.type == SoundType::PREDATOR_MOVEMENT) {
            // Flee away from sound
            Direction threatDir = moveTowardSound(*this, sound);
            Direction fleeDir = oppositeDirection(threatDir);
            
            setProfile(Profile::flee);
            Navigator::moveDirection(*this, world, fleeDir);
            depositAlarmScent(world);
            return true;
        }
    }
    
    return false;
}

void Prey::depositAlarmScent(World& world) {
    float scent_prod = getPhenotype()->getTrait("scent_production");
    if (scent_prod > 0.3f) {
        world.depositScent(getX(), getY(), 
                          ScentType::ALARM,
                          scent_prod,
                          getOwnSignature(),
                          getId());
    }
}
```

### 6.3 Alarm Cascades

Prey species can create **alarm cascades** through hearing and scent:

```
Predator approaches → Prey A detects via scent
                   → Prey A emits alarm scent + alarm call
                   → Prey B hears alarm call
                   → Prey B flees and emits own alarm
                   → Prey C smells alarm scent
                   → Prey C flees and emits alarm
                   → Cascade spreads through population
```

**Implementation**:
```cpp
// In prey breedProfile or hungryProfile, check for alarms first
bool respondToAlarms(World& world) {
    // Check for alarm scents
    auto alarms = world.detectScents(
        getX(), getY(),
        getScentDetectionRange(),
        getPhenotype()->getTrait("olfactory_acuity"),
        {ScentType::ALARM},
        getOwnSignature()
    );
    
    if (!alarms.empty()) {
        // Flee from alarm source (where threat was detected)
        Direction threatDir = followScentGradient(*this, ScentType::ALARM, alarms);
        Direction fleeDir = oppositeDirection(threatDir);
        setProfile(Profile::flee);
        Navigator::moveDirection(*this, world, fleeDir);
        
        // Propagate alarm
        depositAlarmScent(world);
        if (getPhenotype()->getTrait("sound_production") > 0.3f) {
            emitAlarmCall(world);
        }
        return true;
    }
    
    // Check for alarm calls
    auto alarmSounds = world.detectSounds(...);
    // Similar logic for sound-based alarms
    
    return false;
}
```

---

## 7. Mating Behavior Integration

### 7.1 Mating-Specific Signals

Now that we have general sensory systems, **mating behaviors** are just specific uses:

| Signal Type | Sensory System | Purpose | Frequency/Scent Type |
|-------------|---------------|---------|----------------------|
| **Mating Call** | Auditory | Long-range mate attraction | `call_frequency` gene |
| **Breeding Pheromone** | Olfactory | Persistent trail to self | `ScentType::MATE_SEEKING` |
| **Sexual Selection Display** | Visual (existing) | Close-range compatibility | Color/size genes |

### 7.2 Revised Mating Genes

Integrate with sensory foundation genes:

| Gene ID | Category | Range | Creep | Default | Description |
|---------|----------|-------|-------|---------|-------------|
| `call_frequency` | Reproduction | [20, 5000] | 100 | 500 | Frequency of mating calls (Hz) |
| `call_rate` | Reproduction | [0.0, 1.0] | 0.05 | 0.3 | How often to broadcast calls |
| `mate_pheromone_strength` | Reproduction | [0.0, 1.0] | 0.05 | 0.5 | Strength of breeding scent |

**Removed genes** (now part of general sensory system):
- ~~`mating_call_strength`~~ → use `sound_production`
- ~~`mating_call_frequency`~~ → use `call_rate`  
- ~~`call_sensitivity`~~ → use `hearing_acuity` + `hearing_range_*`
- ~~`pheromone_production`~~ → use `scent_production`
- ~~`pheromone_sensitivity`~~ → use `scent_detection` + `olfactory_acuity`

### 7.3 Improved findMate() Algorithm

```
PRIORITY-BASED MATE FINDING (Using Sensory Foundations):

1. Check for alarm signals (predator nearby)
   → If detected: Abort mating, flee
   
2. Listen for mating calls within hearing range
   → If heard: Navigate toward most attractive call
                (intensity * genetic_similarity)
   
3. Follow breeding pheromone trails
   → Detect ScentType::MATE_SEEKING scents
   → Weight by intensity, distance, and genetic similarity
   → Follow strongest gradient
   
4. Visual mate search (existing behavior)
   → Look for visible mates within sight_range
   → Use A* pathfinding (existing)
   
5. Broadcast own mating call (if call_rate threshold met)
   → Frequency = call_frequency gene
   → Loudness = sound_production gene
   → Costs energy, attracts predators
   
6. Deposit breeding pheromone trail
   → Type = MATE_SEEKING
   → Intensity = mate_pheromone_strength gene
   → Signature = own genetic signature
   → Costs energy
   
7. Exploratory movement with path memory
   → Use exploreUnvisited() to search new areas
   → Weighted by exploration_drive gene
   → Avoid recently visited locations
   
8. Fallback to random wander (existing)
```

### 7.4 Mating Energy Costs

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

### 7.5 Subspeciation Through Mate Choice

The **genetic signature** system enables subspeciation:

```
Generation 0: Uniform population
  - Random genotypes
  - All signatures equally different
  - Mating is random within range

Generation 100: Weak clustering
  - Small genetic similarity bias emerges
  - Slight preference for similar mates
  - Some trait correlation begins

Generation 500: Distinct lineages
  - 2-3 genetic clusters form
  - Strong preference within clusters
  - Rare gene flow between clusters
  - Observable trait differences (color, size, behavior)

Generation 1000: Subspecies
  - 3-5 stable genetic lineages
  - Reproductive isolation (preference)
  - Divergent adaptations to micro-niches
  - Different call frequencies, scent profiles
```

**Example Subspeciation**:

```
Initial Population (500 creatures, uniform distribution):
  avg call_frequency = 500 Hz ± 200 Hz

After 500 generations:

Subspecies A "Highland Whistlers":
  avg call_frequency = 800 Hz (high-pitched)
  avg max_size = 0.8 (small)
  avg scent_signature[1] = 0.7
  Niche: Mountain regions, eat alpine plants
  
Subspecies B "Valley Rumblers":
  avg call_frequency = 250 Hz (low-pitched)  
  avg max_size = 1.5 (large)
  avg scent_signature[1] = 0.3
  Niche: Valley regions, omnivorous
  
Subspecies C "Forest Silent":
  avg call_frequency = 1200 Hz (ultrasonic to others)
  avg acoustic_stealth = 0.8 (very quiet)
  avg scent_signature[1] = 0.5
  Niche: Dense forest, stealth predator avoidance
```

---

## 8. New Gene Registry

### 8.1 Complete Gene List

Add to [`UniversalGenes.hpp`](../include/genetics/defaults/UniversalGenes.hpp):

#### Olfactory Genes (4 new)

```cpp
// ========== BEHAVIOR GENES - OLFACTORY SYSTEM (4) ==========
// Build on existing SCENT_DETECTION

/// General scent emission rate [0.0, 1.0], creep 0.05
static constexpr const char* SCENT_PRODUCTION = "scent_production";

/// How much genetic signature varies from genotype [0.0, 1.0], creep 0.05
static constexpr const char* SCENT_SIGNATURE_VARIANCE = "scent_signature_variance";

/// Ability to distinguish different scent types [0.0, 1.0], creep 0.05
static constexpr const char* OLFACTORY_ACUITY = "olfactory_acuity";

/// Reduce own scent detectability [0.0, 1.0], creep 0.05
static constexpr const char* SCENT_MASKING = "scent_masking";
```

#### Auditory Genes (6 new)

```cpp
// ========== BEHAVIOR GENES - AUDITORY SYSTEM (6) ==========

/// General hearing sensitivity [0.0, 1.0], creep 0.05
static constexpr const char* HEARING_ACUITY = "hearing_acuity";

/// Minimum audible frequency [20, 300], creep 10
static constexpr const char* HEARING_RANGE_LOW = "hearing_range_low";

/// Maximum audible frequency [300, 5000], creep 100
static constexpr const char* HEARING_RANGE_HIGH = "hearing_range_high";

/// Loudness of vocalizations [0.0, 1.0], creep 0.05
static constexpr const char* SOUND_PRODUCTION = "sound_production";

/// Reduce detectability of own sounds [0.0, 1.0], creep 0.05
static constexpr const char* ACOUSTIC_STEALTH = "acoustic_stealth";

/// Frequency of own vocalizations [20, 5000], creep 100
static constexpr const char* VOCALIZATION_FREQUENCY = "vocalization_frequency";
```

#### Exploration Genes (3 new)

```cpp
// ========== MOBILITY GENES - EXPLORATION & MEMORY (3) ==========

/// Drive to explore unvisited areas [0.0, 1.0], creep 0.05
static constexpr const char* EXPLORATION_DRIVE = "exploration_drive";

/// Spatial memory of visited locations [0.0, 1.0], creep 0.05
static constexpr const char* PATH_MEMORY = "path_memory";

/// Persistence in pursuing movement goals [0.0, 1.0], creep 0.05
static constexpr const char* MOVEMENT_PERSISTENCE = "movement_persistence";
```

#### Mating-Specific Genes (3 revised)

```cpp
// ========== REPRODUCTION GENES - MATING COMMUNICATION (3) ==========

/// Frequency of mating calls [20, 5000], creep 100
static constexpr const char* CALL_FREQUENCY = "call_frequency";

/// Rate of mating call emission [0.0, 1.0], creep 0.05
static constexpr const char* CALL_RATE = "call_rate";

/// Strength of breeding pheromone [0.0, 1.0], creep 0.05
static constexpr const char* MATE_PHEROMONE_STRENGTH = "mate_pheromone_strength";
```

### 8.2 Chromosome Assignment

Total new genes: **16** (4 olfactory + 6 auditory + 3 exploration + 3 mating)

Previous total: 58 genes  
**New total: 74 genes**

Distribution:
- Universal: 7
- Mobility: 8 (was 5, +3)
- Autotrophy: 5
- Heterotrophy: 13
- Morphology: 9
- Behavior: 16 (was 5, +11)
- Reproduction: 14 (was 11, +3)
- Plant Defense: 6

---

## 9. Implementation Architecture

### 9.1 Class Structure

```cpp
// =========== NEW FILES ===========

// include/world/ScentLayer.hpp
class ScentLayer {
public:
    void depositScent(int x, int y, const ScentData& data);
    std::vector<ScentSource> detectScents(int x, int y, float range, 
                                         float acuity, 
                                         const std::set<ScentType>& filter,
                                         const ScentSignature& detectorSig);
    void update(int ticksElapsed);  // Decay all scents
    
private:
    std::unordered_map<TileCoord, std::vector<ScentData>> _scentsByTile;
};

// include/world/SoundLayer.hpp
class SoundLayer {
public:
    void broadcastSound(const SoundEmission& emission);
    std::vector<HeardSound> detectSounds(int x, int y,
                                        float acuity,
                                        float lowFreq, float highFreq,
                                        const ScentSignature& listenerSig);
    void clearSounds();  // Called each tick
    
private:
    std::vector<SoundEmission> _activeSounds;
};

// include/genetics/ScentSignature.hpp
struct ScentSignature {
    float signature[8];
    
    static ScentSignature computeSignature(const Genome& genome);
    float similarityTo(const ScentSignature& other) const;
};

// include/objects/creature/ExplorationMemory.hpp
struct ExplorationMemory {
    void recordVisit(int x, int y, int currentTick);
    bool wasVisitedRecently(int x, int y, int currentTick) const;
    float getRecencyWeight(int x, int y, int currentTick) const;
    
private:
    struct Visit { int x, y, tickVisited; };
    static constexpr int MEMORY_SIZE = 100;
    static constexpr int FORGET_AFTER_TICKS = 500;
    Visit locations[MEMORY_SIZE];
    int nextIndex = 0;
    int visitCount = 0;
};
```

### 9.2 World Integration

```cpp
// Modify World class (include/world/world.hpp)
class World {
    // ... existing members ...
    
    ScentLayer _scentLayer;
    SoundLayer _soundLayer;
    
public:
    // Scent API
    void depositScent(int x, int y, ScentType type, float intensity,
                     const ScentSignature& sig, int sourceId) {
        _scentLayer.depositScent(x, y, {type, intensity, sig, 0, sourceId});
    }
    
    std::vector<ScentSource> detectScents(int x, int y, float range,
                                         float acuity,
                                         const std::set<ScentType>& filter,
                                         const ScentSignature& detectorSig) {
        return _scentLayer.detectScents(x, y, range, acuity, filter, detectorSig);
    }
    
    // Sound API
    void broadcastSound(int x, int y, int sourceId, float loudness,
                       float frequency, const ScentSignature& sig,
                       SoundType type) {
        _soundLayer.broadcastSound({x, y, sourceId, loudness, frequency, sig, type});
    }
    
    std::vector<HeardSound> detectSounds(int x, int y, float acuity,
                                        float lowFreq, float highFreq,
                                        const ScentSignature& listenerSig) {
        return _soundLayer.detectSounds(x, y, acuity, lowFreq, highFreq, listenerSig);
    }
    
    // Update (called each tick)
    void updateSensoryLayers(int ticksElapsed) {
        _scentLayer.update(ticksElapsed);
        _soundLayer.clearSounds();  // Sounds are transient
    }
};
```

### 9.3 Creature Integration

```cpp
// Modify Creature class (include/objects/creature/creature.hpp)
class Creature {
    // ... existing members ...
    
    ExplorationMemory _explorationMemory;
    ScentSignature _ownSignature;  // Computed from genome
    
    // Initialize signature in constructor
    void initializeSensory() {
        if (_useNewGenetics) {
            _ownSignature = ScentSignature::computeSignature(*_newGenome);
        }
    }
    
    // Sensory methods
    const ScentSignature& getOwnSignature() const { return _ownSignature; }
    ExplorationMemory& getExplorationMemory() { return _explorationMemory; }
    
    float getScentDetectionRange() const {
        return 30.0f * getPhenotype()->getTrait("scent_detection") *
                      getPhenotype()->getTrait("olfactory_acuity");
    }
    
    float getHearingRange() const {
        // Approximate max hearing range based on acuity
        return 200.0f * getPhenotype()->getTrait("hearing_acuity");
    }
};
```

---

## 10. Implementation Phases

### Phase 1: Sensory Foundation (Weeks 1-3)

**Goals:**
- Implement scent and sound layer architecture
- Add 16 new genes to UniversalGenes
- Implement ScentSignature system
- Basic scent detection and sound detection

**Deliverables:**
- `include/world/ScentLayer.hpp` + `.cpp`
- `include/world/SoundLayer.hpp` + `.cpp`
- `include/genetics/ScentSignature.hpp` + `.cpp`
- Updated `UniversalGenes.hpp/.cpp` with new genes
- Unit tests for scent/sound detection

**Testing:**
- Scent deposit, decay, detection
- Sound emission, attenuation, frequency filtering
- Signature similarity calculations

### Phase 2: Gradient Navigation (Week 4)

**Goals:**
- Implement scent gradient following algorithm
- Implement sound direction finding
- Add to Navigator class

**Deliverables:**
- `Navigator::followScentGradient()`
- `Navigator::moveTowardSound()`
- Weighted gradient calculations

**Testing:**
- Creatures follow scent trails to sources
- Creatures move toward sound sources
- Gradient direction is correct

### Phase 3: Exploration Memory (Week 5)

**Goals:**
- Implement ExplorationMemory data structure
- Memory-guided exploration movement
- Path memory gene integration

**Deliverables:**
- `include/objects/creature/ExplorationMemory.hpp`
- `Navigator::exploreUnvisited()`
- Memory capacity scaled by gene

**Testing:**
- Creatures avoid recently visited locations
- Memory decays after timeout
- Exploration drive affects movement

### Phase 4: Mating Integration (Weeks 6-7)

**Goals:**
- Refactor `findMate()` to use sensory foundations
- Mating calls via sound system
- Breeding pheromones via scent system
- Genetic similarity attraction

**Deliverables:**
- Updated `creature.cpp::findMate()`
- Mating call emission
- Breeding pheromone trails
- Similarity-weighted attraction

**Testing:**
- Mating success rate > 70%
- Genetic similarity affects pairing
- Energy costs apply correctly
- Run breeding diagnostic

### Phase 5: Predator-Prey (Weeks 8-9)

**Goals:**
- Predator scent tracking
- Prey predator detection
- Alarm cascades
- Coevolutionary dynamics

**Deliverables:**
- Predator `huntPrey()` using scent/sound
- Prey `detectPredators()` system
- Alarm scent and call propagation
- Stealth vs detection arms race

**Testing:**
- Predators successfully track prey
- Prey successfully avoid predators
- Alarm cascades spread through groups
- Long-term evolution shows arms race

### Phase 6: Subspeciation (Week 10+)

**Goals:**
- Long-term evolution simulation
- Observe subspeciation emergence
- Document genetic lineages
- Frequency-dependent selection

**Deliverables:**
- Subspeciation tracking tools
- Genetic cluster analysis
- Evolution report with lineages
- Demonstration of reproductive isolation

**Testing:**
- 10,000+ tick simulations
- Multiple stable genetic clusters emerge
- Different call frequencies between subspecies
- Preferential mating within subspecies

---

## 11. Performance Considerations

### 11.1 Scent Layer Optimization

**Concern**: Checking all tiles for scents is expensive

**Solution**: Spatial partitioning with sparse storage

```cpp
// Only store scents in tiles that have them
std::unordered_map<TileCoord, std::vector<ScentData>> _scentsByTile;

// When detecting:
for (int dx = -range; dx <= range; dx++) {
    for (int dy = -range; dy <= range; dy++) {
        TileCoord coord = {x + dx, y + dy};
        
        // Early exit if no scents at this tile
        auto it = _scentsByTile.find(coord);
        if (it == _scentsByTile.end()) continue;
        
        // Check distance
        if (dx*dx + dy*dy > range*range) continue;
        
        // Process scents at this tile
        for (const auto& scent : it->second) {
            // ... detection logic ...
        }
    }
}
```

**Expected Cost**: O(range² * avg_scents_per_tile)  
For range=30, ~900 tiles checked, but only ~5% have scents = ~45 actual checks

### 11.2 Sound Layer Optimization

**Concern**: All creatures checking all sounds every tick is O(n²)

**Solution**: Only process sounds within max hearing range

```cpp
// When detecting sounds
for (const auto& sound : _activeSounds) {
    // Quick distance check before expensive calculations
    int dx = abs(sound.sourceX - listenerX);
    int dy = abs(sound.sourceY - listenerY);
    
    // Max possible hearing range is ~200 tiles
    if (dx > 200 || dy > 200) continue;
    
    float distance = sqrt(dx*dx + dy*dy);
    if (distance > 200) continue;
    
    // Now do expensive frequency and attenuation checks
    // ...
}
```

**Expected Cost**: O(creatures_in_range * sounds_in_range)  
Much better than O(all_creatures * all_sounds)

### 11.3 Memory Usage

```
ScentLayer: ~10 scents per occupied tile * 5% occupancy * 500x500
  = 10 * 0.05 * 250,000 * sizeof(ScentData)
  = 125,000 * 64 bytes = ~8 MB

SoundLayer: max 200 active sounds * sizeof(SoundEmission)
  = 200 * 48 bytes = ~10 KB (negligible)

ExplorationMemory per creature: 100 visits * 12 bytes = 1.2 KB
  * 200 creatures = 240 KB

Total: ~8.25 MB additional memory (acceptable)
```

---

## 12. Success Criteria

### 12.1 Sensory System Metrics

| Metric | Target | Validation |
|--------|--------|------------|
| **Scent Detection Accuracy** | > 95% | Creatures detect all scents in range |
| **Sound Attenuation** | Realistic curve | Intensity drops with distance as expected |
| **Frequency Filtering** | > 90% correct | Creatures only hear in their range |
| **Gradient Direction** | < 15° error | Creatures move toward scent sources |
| **Memory Recall** | > 90% | Correctly remember visited locations |

### 12.2 Behavior Metrics

| Metric | Current | Target | Notes |
|--------|---------|--------|-------|
| **Mating Success Rate** | 1.3% | > 75% | Primary goal |
| **Predator Hunt Success** | N/A | > 40% | When using scent tracking |
| **Prey Survival** | N/A | > 60% | When using alarm systems |
| **Alarm Cascade Range** | N/A | 5-10 tiles | How far alarms spread |
| **Exploration Efficiency** | N/A | < 300 tiles | To find mate/food |

### 12.3 Evolutionary Metrics

| Metric | Target | Timeframe |
|--------|--------|-----------|
| **Subspeciation Emergence** | 2-5 clusters | 500-1000 generations |
| **Frequency-Dependent Selection** | Observable | 200+ generations |
| **Arms Race** | Predator/prey coevolution | 500+ generations |
| **Strategy Diversity** | 4+ distinct strategies | 300+ generations |
| **Population Stability** | Self-sustaining | Long-term |

---

## 13. Future Extensions

### 13.1 Additional Sensory Modalities

**Electroreception** (like sharks):
```cpp
static constexpr const char* ELECTRORECEPTION = "electroreception";
// Detect bioelectric fields from prey in water
```

**Magnetoreception** (like birds):
```cpp
static constexpr const char* MAGNETIC_SENSE = "magnetic_sense";
// Navigate using Earth's magnetic field
```

**Infrared Vision** (like pit vipers):
```cpp
static constexpr const char* INFRARED_VISION = "infrared_vision";
// Detect heat signatures of prey
```

### 13.2 Complex Communication

**Composite Signals**:
- Multi-modal displays (visual + acoustic + chemical)
- Honest signaling costs
- Deceptive mimicry

**Social Learning**:
- Younger creatures learn signal meanings from parents
- Cultural transmission of call "dialects"
- Local vs migrant signal differences

**Syntax and Semantics**:
- Sequences of calls with meaning
- Different call patterns for different threats
- Referential signaling (point to specific objects)

### 13.3 Environmental Sensing

**Weather Prediction**:
- Barometric pressure sensing
- Migration triggered by seasonal cues
- Shelter-seeking before storms

**Resource Mapping**:
- Mental maps of food/water locations
- Shared knowledge through social learning
- Territorial memory across generations

---

## 14. Documentation Requirements

### 14.1 User Documentation

**Add to `docs/user/behaviors/`:**
- `sensory-systems.md` - Overview of smell and hearing
- `subspeciation-guide.md` - How genetic clustering emerges
- `signal-strategies.md` - Different communication approaches
- `predator-prey-dynamics.md` - Hunting and avoidance behaviors

### 14.2 Technical Documentation

**Add to `docs/technical/sensory/`:**
- `scent-layer-architecture.md` - Implementation details
- `sound-propagation.md` - Attenuation and frequency mechanics
- `gradient-algorithms.md` - Direction finding mathematics
- `genetic-signatures.md` - Signature computation and similarity

### 14.3 API Documentation

All new classes and methods need complete Doxygen comments:
- Parameter descriptions
- Return value meanings
- Complexity analysis (O notation)
- Usage examples

---

## 15. Conclusion

This revised design establishes **general-purpose sensory foundations** that enable:

1. **Mating Communication** - Creatures find mates via scent trails and calls with genetic signatures
2. **Hunting Behaviors** - Predators track prey using smell and sound
3. **Predator Avoidance** - Prey detect threats and coordinate alarm responses
4. **Subspeciation** - Genetic similarity preferences lead to reproductive isolation
5. **Emergent Strategies** - Multiple viable approaches to sensing and signaling

### Key Improvements Over v1.0

- **General Purpose**: Sensory systems serve all behaviors, not just mating
- **Genetic Signatures**: Enable subspeciation through similarity-based attraction
- **Frequency-Based Hearing**: Allows stealth via frequency selection
- **Concrete Memory**: ExplorationMemory has clear data structure and algorithm
- **Layered Architecture**: Separate scent and sound layers for efficiency
- **Predator-Prey Applications**: Explicit hunting and avoidance behaviors
- **Arms Race Dynamics**: Natural coevolution emerges from sensory capabilities

### Expected Outcomes

- **Mating success rate**: 1.3% → 75%+ (primary goal achieved)
- **Behavioral diversity**: Multiple sensory strategies coexist
- **Subspeciation**: 2-5 genetic lineages emerge naturally
- **Predator-prey dynamics**: Realistic hunting and evasion
- **Evolutionary stability**: Self-sustaining populations with balanced selection

---

## Appendices

### A. Scent Signature Computation Example

```cpp
// Example creature genome
Genome genome;
genome.setTrait("max_size", 1.2);
genome.setTrait("metabolism_rate", 0.8);
genome.setTrait("color_hue", 180.0);  // Cyan
genome.setTrait("locomotion", 0.9);
// ... etc ...

ScentSignature sig = ScentSignature::computeSignature(genome);
// sig.signature = [0.74, 0.50, 0.90, 0.60, 0.30, 0.45, 0.50, 0.35]

// Compare to another creature
ScentSignature other = {0.72, 0.48, 0.88, 0.58, 0.32, 0.47, 0.52, 0.33};
float similarity = sig.similarityTo(other);
// similarity ≈ 0.95 (very similar, likely same subspecies)
```

### B. Sound Attenuation Graph

```
Intensity vs Distance (loudness = 1.0, perfect frequency, acuity = 1.0)

1.0 |■
0.9 |■■
0.8 |■■
0.7 |■■■
0.6 |■■■
0.5 |■■■■
0.4 |■■■■■
0.3 |■■■■■■
0.2 |■■■■■■■■
0.1 |■■■■■■■■■■
0.0 |■■■■■■■■■■■■■■■■
    └─────────────────────────────
    0   50  100  150  200  250  (tiles)
```

### C. Exploration Memory Example

```
Creature path over 20 ticks:
Tick 0:  (10, 10) - start
Tick 5:  (15, 12) - moved NE
Tick 10: (18, 15) - moved NE  
Tick 15: (20, 18) - moved NE
Tick 20: (17, 20) - moved NW

Memory state at tick 20:
locations[0] = {10, 10, 0}
locations[1] = {15, 12, 5}
locations[2] = {18, 15, 10}
locations[3] = {20, 18, 15}
locations[4] = {17, 20, 20}
nextIndex = 5
visitCount = 5

Query: Is location (16, 13) recently visited?
Check: distance to (15, 12) = sqrt(1 + 1) ≈ 1.4 tiles
Result: YES (within 2-tile tolerance, visited 15 ticks ago)

Query: What's the recency weight for (16, 13)?
TicksSinceVisit = 20 - 5 = 15 ticks
Weight = 15 / 500 = 0.03 (very recently visited, low exploration weight)
```

### D. Gene Interaction Examples

#### Example 1: Stealth Predator

```
Genotype:
  scent_masking = 0.9 (high)
  acoustic_stealth = 0.8 (high)
  scent_detection = 0.7 (good)
  hearing_acuity = 0.6 (moderate)
  
Strategy: Silent stalker
  - Emits minimal scent (hard for prey to detect)
  - Very quiet movement
  - Good sensory detection of prey
  - Relies on surprise rather than speed
```

#### Example 2: Alarm Specialist

```
Genotype:
  scent_production = 0.8 (high)
  sound_production = 0.7 (high)
  scent_detection = 0.9 (excellent)
  hearing_acuity = 0.8 (excellent)
  
Strategy: Community sentinel
  - Excellent early warning detection
  - Produces strong alarm signals
  - Helps protect group at personal risk
  - High predator attraction but benefits kin
```

#### Example 3: Frequency Specialist

```
Genotype:
  call_frequency = 3500 Hz (ultrasonic)
  hearing_range_low = 2000 Hz
  hearing_range_high = 4500 Hz
  
Strategy: Private channel
  - Mating calls at 3500 Hz
  - Only similar creatures can hear (subspecies isolation)
  - Common predators hear 50-2000 Hz (can't detect calls)
  - Creates communication privacy
```

### E. References

**Existing Systems**:
- Breeding Diagnostic: [`src/testing/breedingDiagnostic.cpp`](../src/testing/breedingDiagnostic.cpp)
- Universal Genes: [`include/genetics/defaults/UniversalGenes.hpp`](../include/genetics/defaults/UniversalGenes.hpp)
- Creature Class: [`include/objects/creature/creature.hpp`](../include/objects/creature/creature.hpp)
- Navigator Class: [`include/objects/creature/navigator.hpp`](../include/objects/creature/navigator.hpp)
- Tile Class: [`include/world/tile.hpp`](../include/world/tile.hpp)
- World Class: [`include/world/world.hpp`](../include/world/world.hpp)
- Energy Budget: [`include/genetics/expression/EnergyBudget.hpp`](../include/genetics/expression/EnergyBudget.hpp)

**New Components** (to be implemented):
- `include/world/ScentLayer.hpp` - Scent storage and detection
- `include/world/SoundLayer.hpp` - Sound emission and hearing
- `include/genetics/ScentSignature.hpp` - Genetic fingerprinting
- `include/objects/creature/ExplorationMemory.hpp` - Path memory system

---

**End of Design Document**
