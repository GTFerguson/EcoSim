---
title: Interfaces API Reference
created: 2025-12-24
updated: 2025-12-24
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

- **IPositionable** - Entities with world position
- **ILifecycle** - Entities with age and lifespan
- **IGeneticOrganism** - Entities with genetics
- **IReproducible** - Entities that can reproduce

All interfaces are in the `EcoSim::Genetics` namespace.

---

## IPositionable

**Header:** [`include/genetics/interfaces/IPositionable.hpp`](include/genetics/interfaces/IPositionable.hpp:7)

Interface for entities with world position.

```cpp
namespace EcoSim::Genetics {

class IPositionable {
public:
    virtual ~IPositionable() = default;
    
    virtual int getX() const = 0;
    virtual int getY() const = 0;
    virtual void setPosition(int x, int y) = 0;
};

}
```

### Pure Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getX()` | `int` | Get X coordinate in world |
| `getY()` | `int` | Get Y coordinate in world |
| `setPosition(x, y)` | `void` | Set new position |

### Usage

Any entity that exists in the simulation world should implement this interface:

```cpp
class MyEntity : public IPositionable {
private:
    int m_x, m_y;
    
public:
    int getX() const override { return m_x; }
    int getY() const override { return m_y; }
    void setPosition(int x, int y) override {
        m_x = x;
        m_y = y;
    }
};
```

### Implementors

- [`Plant`](include/genetics/organisms/Plant.hpp) - Plants in the world
- `Creature` (future) - Animals in the world

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

- [`Plant`](include/genetics/organisms/Plant.hpp) - Plant lifecycle

---

## IGeneticOrganism

**Header:** [`include/genetics/interfaces/IGeneticOrganism.hpp`](include/genetics/interfaces/IGeneticOrganism.hpp:11)

Interface for entities with genetics.

```cpp
namespace EcoSim::Genetics {

class IGeneticOrganism {
public:
    virtual ~IGeneticOrganism() = default;
    
    virtual const Genome& getGenome() const = 0;
    virtual Genome& getGenomeMutable() = 0;
    virtual const Phenotype& getPhenotype() const = 0;
    virtual void updatePhenotype() = 0;
};

}
```

### Pure Virtual Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getGenome()` | `const Genome&` | Const reference to genome |
| `getGenomeMutable()` | `Genome&` | Mutable reference to genome |
| `getPhenotype()` | `const Phenotype&` | Current expressed phenotype |
| `updatePhenotype()` | `void` | Recalculate phenotype from genome |

### Usage

```cpp
class MyOrganism : public IGeneticOrganism {
private:
    Genome m_genome;
    Phenotype m_phenotype;
    const GeneRegistry& m_registry;
    
public:
    MyOrganism(const GeneRegistry& registry) 
        : m_registry(registry), m_phenotype(&m_genome, &registry) {}
    
    const Genome& getGenome() const override { return m_genome; }
    Genome& getGenomeMutable() override { return m_genome; }
    const Phenotype& getPhenotype() const override { return m_phenotype; }
    
    void updatePhenotype() override {
        m_phenotype.invalidateCache();
    }
};
```

### Design Notes

- The genome is owned by the implementing class
- The phenotype maintains pointers to the genome (not ownership)
- Call `updatePhenotype()` after environmental changes
- The registry reference should remain valid for the organism's lifetime

### Implementors

- [`Plant`](include/genetics/organisms/Plant.hpp) - Plant genetics

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
├── getX()
├── getY()
└── setPosition()

ILifecycle
├── getAge()
├── getMaxLifespan()
├── getAgeNormalized()
├── isAlive()
└── age()

IGeneticOrganism
├── getGenome()
├── getGenomeMutable()
├── getPhenotype()
└── updatePhenotype()

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
