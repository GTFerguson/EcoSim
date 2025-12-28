---
title: Interfaces API Reference
created: 2025-12-24
updated: 2025-12-27
status: complete
audience: developer
type: reference
tags: [genetics, api, reference, interfaces]
---

# Interfaces API Reference

**Audience:** Developer
**Type:** Reference

---

## Overview

This document covers the interface definitions used by the genetics system. These interfaces define contracts that organism classes implement, enabling polymorphic behavior and modular design:

- **IPositionable** - Entities with world position (tile and float coordinates)
- **ILifecycle** - Entities with age and lifespan
- **IGeneticOrganism** - Entities with genetics (includes position and ID for behavior system)
- **IReproducible** - Entities that can reproduce

All interfaces are in the `EcoSim::Genetics` namespace.

---

## IPositionable

**Header:** [`include/genetics/interfaces/IPositionable.hpp`](include/genetics/interfaces/IPositionable.hpp:13)

Interface for entities with position in the world. Provides both tile-based (integer) and world (float) coordinate access.

```cpp
namespace EcoSim::Genetics {

class IPositionable {
public:
    virtual ~IPositionable() = default;
    
    // Tile coordinates (integer - for collision, rendering)
    virtual int getX() const = 0;
    virtual int getY() const = 0;
    
    // World coordinates (float - for smooth movement)
    virtual float getWorldX() const = 0;
    virtual float getWorldY() const = 0;
    
    // Set position using float coordinates
    virtual void setWorldPosition(float x, float y) = 0;
};

}
```

### Pure Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| [`getX()`](include/genetics/interfaces/IPositionable.hpp:21) | `int` | Tile X coordinate (floor of world X) |
| [`getY()`](include/genetics/interfaces/IPositionable.hpp:27) | `int` | Tile Y coordinate (floor of world Y) |
| [`getWorldX()`](include/genetics/interfaces/IPositionable.hpp:33) | `float` | Precise world X coordinate |
| [`getWorldY()`](include/genetics/interfaces/IPositionable.hpp:39) | `float` | Precise world Y coordinate |
| [`setWorldPosition(x, y)`](include/genetics/interfaces/IPositionable.hpp:46) | `void` | Set position using float coordinates |

### Design Notes

The float position system enables:
- **Smooth movement**: Creatures can move fractional tiles per tick
- **Visible speed differences**: Fast creatures move more per tick than slow ones
- **Accurate distance calculations**: Uses actual positions, not tile centers

Tile coordinates are derived from world coordinates via truncation (`floor`).

### Usage

Any entity that exists in the simulation world should implement this interface:

```cpp
class MyEntity : public IPositionable {
private:
    float worldX_, worldY_;
    
public:
    int getX() const override { return static_cast<int>(worldX_); }
    int getY() const override { return static_cast<int>(worldY_); }
    float getWorldX() const override { return worldX_; }
    float getWorldY() const override { return worldY_; }
    void setWorldPosition(float x, float y) override {
        worldX_ = x;
        worldY_ = y;
    }
};
```

### Implementors

- [`Plant`](include/genetics/organisms/Plant.hpp:67) - Plants in the world
- [`Creature`](include/objects/creature/creature.hpp:97) - Animals in the world

---

## ILifecycle

**Header:** [`include/genetics/interfaces/ILifecycle.hpp`](include/genetics/interfaces/ILifecycle.hpp:7)

Interface for entities with lifecycle (age and death).

```cpp
namespace EcoSim::Genetics {

class ILifecycle {
public:
    virtual ~ILifecycle() = default;
    
    virtual unsigned int getAge() const = 0;
    virtual unsigned int getMaxLifespan() const = 0;
    virtual float getAgeNormalized() const = 0;  // 0.0 to 1.0
    virtual bool isAlive() const = 0;
    virtual void age(unsigned int ticks = 1) = 0;
};

}
```

### Pure Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getAge()` | `unsigned int` | Current age in ticks |
| `getMaxLifespan()` | `unsigned int` | Maximum possible lifespan |
| `getAgeNormalized()` | `float` | Age as fraction (0.0-1.0) of max lifespan |
| `isAlive()` | `bool` | True if entity is alive |
| `age(ticks)` | `void` | Advance age by specified ticks |

### Usage

```cpp
class MyOrganism : public ILifecycle {
private:
    unsigned int m_age = 0;
    unsigned int m_maxLifespan = 1000;
    bool m_alive = true;
    
public:
    unsigned int getAge() const override { return m_age; }
    unsigned int getMaxLifespan() const override { return m_maxLifespan; }
    
    float getAgeNormalized() const override {
        return static_cast<float>(m_age) / static_cast<float>(m_maxLifespan);
    }
    
    bool isAlive() const override { return m_alive && m_age < m_maxLifespan; }
    
    void age(unsigned int ticks = 1) override {
        m_age += ticks;
        if (m_age >= m_maxLifespan) {
            m_alive = false;
        }
    }
};
```

### Implementors

- [`Plant`](include/genetics/organisms/Plant.hpp:67) - Plant lifecycle
- [`Creature`](include/objects/creature/creature.hpp:97) - Creature lifecycle

---

## IGeneticOrganism

**Header:** [`include/genetics/interfaces/IGeneticOrganism.hpp`](include/genetics/interfaces/IGeneticOrganism.hpp:18)

Interface for entities with genetics. This interface is the primary abstraction used by the behavior system, enabling organism-agnostic behavior implementations.

> [!NOTE]
> IGeneticOrganism includes position and ID methods to support the behavior system. This reduces the need for multiple interface parameters in behavior methods.

```cpp
namespace EcoSim::Genetics {

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

}
```

### Pure Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| [`getGenome()`](include/genetics/interfaces/IGeneticOrganism.hpp:23) | `const Genome&` | Const reference to genome |
| [`getGenomeMutable()`](include/genetics/interfaces/IGeneticOrganism.hpp:26) | `Genome&` | Mutable reference to genome |
| [`getPhenotype()`](include/genetics/interfaces/IGeneticOrganism.hpp:29) | `const Phenotype&` | Current expressed phenotype |
| [`updatePhenotype()`](include/genetics/interfaces/IGeneticOrganism.hpp:32) | `void` | Recalculate phenotype from genome |
| [`getX()`](include/genetics/interfaces/IGeneticOrganism.hpp:35) | `int` | Tile X coordinate |
| [`getY()`](include/genetics/interfaces/IGeneticOrganism.hpp:36) | `int` | Tile Y coordinate |
| [`getId()`](include/genetics/interfaces/IGeneticOrganism.hpp:39) | `int` | Unique identifier for tracking |

### Usage

```cpp
class MyOrganism : public IGeneticOrganism {
private:
    int id_;
    int x_, y_;
    Genome m_genome;
    Phenotype m_phenotype;
    const GeneRegistry& m_registry;
    
public:
    MyOrganism(int id, int x, int y, const GeneRegistry& registry)
        : id_(id), x_(x), y_(y), m_registry(registry),
          m_phenotype(&m_genome, &registry) {}
    
    const Genome& getGenome() const override { return m_genome; }
    Genome& getGenomeMutable() override { return m_genome; }
    const Phenotype& getPhenotype() const override { return m_phenotype; }
    
    void updatePhenotype() override {
        m_phenotype.invalidateCache();
    }
    
    int getX() const override { return x_; }
    int getY() const override { return y_; }
    int getId() const override { return id_; }
};
```

### Design Notes

- The genome is owned by the implementing class
- The phenotype maintains pointers to the genome (not ownership)
- Call `updatePhenotype()` after environmental changes
- The registry reference should remain valid for the organism's lifetime
- Position methods (`getX()`, `getY()`) allow behaviors to access organism location
- The `getId()` method enables tracking organisms across behavior ticks

### Implementors

- [`Plant`](include/genetics/organisms/Plant.hpp:67) - Plant genetics
- [`Creature`](include/objects/creature/creature.hpp:97) - Creature genetics

---

## IReproducible

**Header:** [`include/genetics/interfaces/IReproducible.hpp`](include/genetics/interfaces/IReproducible.hpp:8)

Template interface for entities that can reproduce.

```cpp
namespace EcoSim::Genetics {

template<typename T>
class IReproducible {
public:
    virtual ~IReproducible() = default;
    
    virtual bool canReproduce() const = 0;
    virtual float getReproductiveUrge() const = 0;
    virtual T reproduce(const T& partner) = 0;
};

}
```

### Template Parameter

| Parameter | Description |
|-----------|-------------|
| `T` | The concrete type being reproduced (e.g., `Plant`, `Creature`) |

### Pure Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `canReproduce()` | `bool` | True if entity can reproduce now |
| `getReproductiveUrge()` | `float` | Current desire to reproduce (0.0-1.0) |
| `reproduce(partner)` | `T` | Create offspring with partner |

### Usage

```cpp
class MyOrganism : public IReproducible<MyOrganism> {
private:
    float m_energy;
    float m_maturity;
    Genome m_genome;
    
public:
    bool canReproduce() const override {
        return m_energy > 50.0f && m_maturity >= 1.0f;
    }
    
    float getReproductiveUrge() const override {
        if (!canReproduce()) return 0.0f;
        return std::min(1.0f, (m_energy - 50.0f) / 50.0f);
    }
    
    MyOrganism reproduce(const MyOrganism& partner) override {
        // Create offspring genome through crossover
        Genome offspringGenome = Genome::crossover(m_genome, partner.m_genome);
        
        // Deduct reproduction cost
        m_energy -= 25.0f;
        
        return MyOrganism(offspringGenome);
    }
};
```

### Design Notes

- The template parameter allows type-safe reproduction
- `canReproduce()` should check all requirements (energy, maturity, etc.)
- `getReproductiveUrge()` drives AI behavior for mate-seeking
- `reproduce()` should handle genome crossover and resource deduction

---

## Interface Composition

The genetics system uses interface composition to build complete organisms. A typical organism implements multiple interfaces:

```cpp
class Plant : public IPositionable, 
              public ILifecycle, 
              public IGeneticOrganism {
    // Implements all three interfaces
    // IReproducible is handled separately via PlantFactory
};
```

### Interface Hierarchy Diagram

```
IPositionable
├── getX()              // Tile X (integer)
├── getY()              // Tile Y (integer)
├── getWorldX()         // World X (float)
├── getWorldY()         // World Y (float)
└── setWorldPosition()  // Set float position

ILifecycle
├── getAge()
├── getMaxLifespan()
├── getAgeNormalized()
├── isAlive()
└── age()

IGeneticOrganism        // Primary behavior system interface
├── getGenome()
├── getGenomeMutable()
├── getPhenotype()
├── updatePhenotype()
├── getX()              // Position for behaviors
├── getY()
└── getId()             // Tracking identifier

IReproducible<T>
├── canReproduce()
├── getReproductiveUrge()
└── reproduce()
```

---

## Best Practices

### 1. Virtual Destructor

All interfaces have a virtual destructor to ensure proper cleanup:

```cpp
virtual ~IPositionable() = default;
```

### 2. Const Correctness

Accessor methods are marked `const`:

```cpp
virtual int getX() const = 0;  // Const - doesn't modify state
virtual void setPosition(int x, int y) = 0;  // Non-const - modifies state
```

### 3. Reference Returns

Return references for large objects to avoid copies:

```cpp
virtual const Genome& getGenome() const = 0;  // Returns reference, not copy
```

### 4. Pure Virtual

All methods are pure virtual (`= 0`) to ensure implementors provide all functionality.

---

## See Also

**Core Documentation:**
- [[../../core/01-architecture]] - System architecture overview
- [[../../core/03-extending]] - Extension guide

**Reference:**
- [[../quick-reference]] - Quick reference cheat sheet
- [[../genes]] - Complete gene catalog
- [[core-classes]] - Core genetics classes (Genome, Gene)
- [[expression]] - Phenotype and state management
- [[organisms]] - Organism implementations (Plant)
- [[interactions]] - System interactions
