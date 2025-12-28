---
title: Organism System Design
created: 2025-12-27
updated: 2025-12-27
status: implemented
tags: [design, organisms, interfaces, behaviors]
---

# Organism System Design

## Overview

The organism system provides a unified interface for genetic entities in the simulation. Both `Creature` and `Plant` implement common interfaces, enabling the behavior system to operate organism-agnostically.

## Core Interface: IGeneticOrganism

The `IGeneticOrganism` interface is the primary abstraction for behavior system integration:

```cpp
class IGeneticOrganism {
public:
    virtual ~IGeneticOrganism() = default;
    
    // Genetics
    virtual const Genome& getGenome() const = 0;
    virtual Genome& getGenomeMutable() = 0;
    virtual const Phenotype& getPhenotype() const = 0;
    virtual void updatePhenotype() = 0;
    
    // Position (for behavior system)
    virtual int getX() const = 0;
    virtual int getY() const = 0;
    
    // Identification (for tracking)
    virtual int getId() const = 0;
};
```

### Why Position and ID in IGeneticOrganism?

The interface includes position and ID methods that might seem redundant with `IPositionable`. This design decision enables:

1. **Single Interface Parameter**: Behaviors take `IGeneticOrganism&` instead of multiple interfaces
2. **Simpler API**: No need for `dynamic_cast` or multiple inheritance checks
3. **Behavior System Compatibility**: All behavior methods can work with one interface type

### Interface Relationships

```
┌─────────────────┐
│  IPositionable  │──── Float position (getWorldX/Y, setWorldPosition)
└────────┬────────┘     Integer position (getX/Y for tiles)
         │
         │ implements
         ▼
┌─────────────────┐     ┌─────────────────┐
│      Plant      │     │    Creature     │
└────────┬────────┘     └────────┬────────┘
         │                       │
         │ implements            │ implements
         ▼                       ▼
┌─────────────────────────────────────────┐
│           IGeneticOrganism              │◄── Primary behavior interface
│  - getGenome()/getGenomeMutable()       │
│  - getPhenotype()/updatePhenotype()     │
│  - getX()/getY() (tile coords)          │
│  - getId() (tracking)                   │
└─────────────────────────────────────────┘
         │                       │
         │ implements            │ implements
         ▼                       ▼
┌─────────────────┐     ┌─────────────────┐
│    ILifecycle   │     │    ILifecycle   │
└─────────────────┘     └─────────────────┘
```

## Implementors

### Plant

Plants implement all three interfaces:

```cpp
class Plant : public IPositionable, 
              public ILifecycle, 
              public IGeneticOrganism { ... };
```

**Key Characteristics:**
- Stationary (position set at creation)
- World position returns tile center (x + 0.5, y + 0.5)
- Reproduction via asexual offspring or seed dispersal
- Behaviors: ZoochoryBehavior (seed dispersal)

### Creature

Creatures implement lifecycle and genetic interfaces:

```cpp
class Creature : public ILifecycle,
                 public IGeneticOrganism { ... };
```

**Key Characteristics:**
- Mobile (float position for smooth movement)
- Complex behavior system (feeding, hunting, mating, rest)
- Behavior controller manages priority-based behavior selection
- Sexual reproduction with genetic crossover

## Behavior System Integration

The behavior system operates on `IGeneticOrganism&` references, enabling organism-agnostic behavior implementations:

### BehaviorController

Each creature owns a `BehaviorController` that manages registered behaviors:

```cpp
class BehaviorController {
public:
    void addBehavior(std::unique_ptr<IBehavior> behavior);
    BehaviorResult update(IGeneticOrganism& organism, BehaviorContext& ctx);
    // ...
};
```

### Behavior Interface

Behaviors check applicability using phenotype traits, not type casting:

```cpp
class IBehavior {
public:
    // Uses interface, not concrete type
    virtual bool isApplicable(const IGeneticOrganism& organism,
                              const BehaviorContext& ctx) const = 0;
    
    virtual BehaviorResult execute(IGeneticOrganism& organism,
                                   BehaviorContext& ctx) = 0;
};
```

### Phenotype-Based Capability Checks

Behaviors determine applicability through trait queries:

```cpp
// Example: HuntingBehavior applicability check
bool HuntingBehavior::isApplicable(const IGeneticOrganism& organism,
                                    const BehaviorContext& ctx) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Check if organism has hunting capability
    float huntInstinct = phenotype.getTrait("hunt_instinct");
    float locomotion = phenotype.getTrait("locomotion_speed");
    
    return huntInstinct > 0.4f && locomotion > 0.3f;
}
```

This approach enables:
- **Emergent Behaviors**: A plant that evolves sufficient traits could theoretically hunt
- **No Type Coupling**: Behaviors don't depend on Creature or Plant classes
- **Evolution-Driven**: Natural selection determines which organisms exhibit behaviors

## OrganismServices Dependency Injection

The `OrganismServices` class provides shared systems to behaviors:

```cpp
class OrganismServices {
public:
    OrganismServices(GeneRegistry& registry, 
                     Navigator& navigator,
                     World& world);
    
    // External dependencies (referenced)
    GeneRegistry& getGeneRegistry();
    Navigator& getNavigator();
    World& getWorld();
    
    // Owned systems (created lazily)
    PerceptionSystem& getPerceptionSystem();
    HealthSystem& getHealthSystem();
    FeedingInteraction& getFeedingInteraction();
    CombatInteraction& getCombatInteraction();
    SeedDispersal& getSeedDispersal();
};
```

### Service Ownership

| Service | Ownership | Creation |
|---------|-----------|----------|
| GeneRegistry | External (referenced) | Application startup |
| Navigator | External (referenced) | World creation |
| World | External (referenced) | Application startup |
| PerceptionSystem | Owned | Lazy (first access) |
| HealthSystem | Owned | Lazy (first access) |
| FeedingInteraction | Owned | Lazy (first access) |
| CombatInteraction | Owned | Lazy (first access) |
| SeedDispersal | Owned | Lazy (first access) |

## Position System

Both organisms use a dual-coordinate system:

### Tile Coordinates (Integer)
- Used for: Collision detection, tile-based queries, NCurses rendering
- Methods: `getX()`, `getY()`

### World Coordinates (Float)
- Used for: Smooth movement, precise distance calculations
- Methods: `getWorldX()`, `getWorldY()`, `setWorldPosition()`

### Plant Position Implementation

Plants are fixed to tile centers:

```cpp
float Plant::getWorldX() const { return static_cast<float>(x_) + 0.5f; }
float Plant::getWorldY() const { return static_cast<float>(y_) + 0.5f; }

void Plant::setWorldPosition(float x, float y) {
    x_ = static_cast<int>(x);  // Truncates to tile
    y_ = static_cast<int>(y);
}
```

### Creature Position Implementation

Creatures maintain precise float positions:

```cpp
float Creature::getWorldX() const { return _worldX; }
float Creature::getWorldY() const { return _worldY; }

void Creature::setWorldPosition(float x, float y) {
    _worldX = x;
    _worldY = y;
}

// Tile coordinates derived from world position
int Creature::getX() const { return static_cast<int>(_worldX); }
int Creature::getY() const { return static_cast<int>(_worldY); }
```

## ID System

Both organisms provide unique IDs for tracking:

| Organism | ID Type | Notes |
|----------|---------|-------|
| Plant | `unsigned int` → `int` | Stored as unsigned, returned as int for interface |
| Creature | `int` | Native int storage |

The interface uses `int` to simplify common use cases (comparison with -1 for "none").

## Design Principles

### 1. Interface Segregation
Each interface serves a specific purpose:
- `IPositionable`: Spatial existence
- `ILifecycle`: Age and mortality
- `IGeneticOrganism`: Genetics and behavior compatibility

### 2. Dependency Inversion
Behaviors depend on `IGeneticOrganism` interface, not concrete types. This enables:
- Testing with mock organisms
- Future organism types
- Behavior reuse across organism types

### 3. Composition Over Inheritance
Organisms compose interfaces rather than inheriting from a base organism class. This allows flexible combinations of capabilities.

### 4. Phenotype-Driven Capability
Organism capabilities emerge from expressed genes, not class membership. A plant with high locomotion genes could theoretically move (future feature).

## See Also

- [[../reference/api/interfaces|Interfaces API Reference]] - Full interface documentation
- [[../reference/api/organisms|Organisms API Reference]] - Plant and Creature APIs
- [[../systems/behavior-system|Behavior System Architecture]] - Behavior implementation details
- [[../core/01-architecture|Core Architecture]] - Overall system design
