---
title: Behavior System Architecture
created: 2025-12-27
status: implemented
tags: [architecture, behaviors, organisms]
---

# Behavior System Architecture

## Overview

The behavior system provides a modular, composable framework for organism behaviors using the Strategy pattern and dependency injection. This system was extracted from the original 2420-line Creature God Class to enable:

1. **Reusable behaviors** - Shared between creature and plant organisms
2. **Testability** - Each behavior can be tested in isolation
3. **Extensibility** - New behaviors added without modifying existing code
4. **Bug fixes** - Hunting extinction fix integrated during extraction

> [!NOTE]
> The behavior system is behind a feature flag (`USE_NEW_BEHAVIOR_SYSTEM`) for gradual rollout.

## Core Components

### IBehavior Interface

Base interface for all behaviors in `include/genetics/behaviors/IBehavior.hpp`:

```cpp
class IBehavior {
public:
    virtual ~IBehavior() = default;
    
    // Unique behavior identifier
    virtual std::string getId() const = 0;
    
    // Check if behavior can execute for organism
    virtual bool isApplicable(const IGeneticOrganism& organism,
                              const BehaviorContext& ctx) const = 0;
    
    // Dynamic priority based on organism state
    virtual float getPriority(const IGeneticOrganism& organism) const = 0;
    
    // Perform behavior and return result
    virtual BehaviorResult execute(IGeneticOrganism& organism,
                                   BehaviorContext& ctx) = 0;
    
    // Energy consumed by behavior
    virtual float getEnergyCost(const IGeneticOrganism& organism) const = 0;
};
```

### BehaviorPriority

Priority levels control behavior selection order:

```cpp
enum class BehaviorPriority : int {
    CRITICAL = 100,    // Fleeing from danger, dying
    HIGH = 75,         // Feeding when hungry, hunting
    NORMAL = 50,       // Mating, normal activities
    LOW = 25,          // Seed dispersal, grooming
    IDLE = 0           // Default wandering
};
```

### BehaviorController

Orchestrates behavior execution using priority-based selection in `include/genetics/behaviors/BehaviorController.hpp`:

```cpp
class BehaviorController {
public:
    void addBehavior(std::unique_ptr<IBehavior> behavior);
    void removeBehavior(const std::string& behaviorId);
    bool hasBehavior(const std::string& behaviorId) const;
    
    // Execute highest-priority applicable behavior
    BehaviorResult update(IGeneticOrganism& organism, BehaviorContext& ctx);
    
    const std::string& getCurrentBehavior() const;
};
```

### BehaviorContext

Execution context providing world/scent layer access:

```cpp
struct BehaviorContext {
    const IWorldQuery& world;      // Read-only world access
    const ScentLayer& scentLayer;  // Scent information
    float deltaTime;               // Time since last tick
    unsigned int currentTick;      // Current simulation tick
};
```

### BehaviorResult

Result of behavior execution:

```cpp
struct BehaviorResult {
    bool executed = false;           // Did the behavior run?
    bool completed = false;          // Is the behavior goal achieved?
    float energyCost = 0.0f;         // Energy consumed
    std::string debugInfo;           // Debug/logging information
};
```

## Implemented Behaviors

| Behavior | Purpose | Priority Range | File |
|----------|---------|----------------|------|
| RestBehavior | Fatigue management, sleep recovery | CRITICAL (100) | `RestBehavior.cpp` |
| HuntingBehavior | Predator hunting with bug fixes | HIGH (75) | `HuntingBehavior.cpp` |
| FeedingBehavior | Herbivore plant feeding | NORMAL (50) | `FeedingBehavior.cpp` |
| MatingBehavior | Sexual reproduction | NORMAL (50) | `MatingBehavior.cpp` |
| ZoochoryBehavior | Seed dispersal (burrs, gut passage) | LOW (25) | `ZoochoryBehavior.cpp` |
| MovementBehavior | Default wandering/navigation | IDLE (0) | `MovementBehavior.cpp` |

### RestBehavior

Handles fatigue and sleep:
- Triggers when fatigue exceeds threshold
- Reduces fatigue over time while resting
- Highest priority to prevent exhaustion death

### HuntingBehavior

Predator hunting with extinction bug fixes:
- **Satiation check**: No hunting when 80% full
- **Cooldown**: 30 ticks minimum between hunts
- **Energy cost**: 1.5 energy per hunt attempt
- **Escape mechanics**: Prey can escape using flee/pursue gene comparison

### FeedingBehavior

Herbivore plant consumption:
- Finds nearby edible plants
- Considers plant defenses (thorns, toxins)
- Uses digestion efficiency genes

### MatingBehavior

Sexual reproduction:
- Mate detection via scent/vision
- Fitness evaluation between potential mates
- Offspring creation with genetic crossover

### ZoochoryBehavior

Seed dispersal mechanics:
- Burr attachment/detachment
- Gut seed passage
- Dispersal event generation

### MovementBehavior

Default navigation:
- Integrates with Navigator pathfinding
- Flocking behavior support
- Terrain-aware movement costs

## Shared Systems

### HealthSystem

Organism-agnostic health management shared by Creature and Plant:

```cpp
class HealthSystem {
public:
    float applyDamage(IGeneticOrganism& organism, float amount, DamageType type);
    float heal(IGeneticOrganism& organism, float amount);
    void processNaturalHealing(IGeneticOrganism& organism);
    bool checkDeathCondition(const IGeneticOrganism& organism) const;
    WoundState getWoundState(const IGeneticOrganism& organism) const;
};
```

Located in:
- Header: `include/genetics/systems/HealthSystem.hpp`
- Source: `src/genetics/systems/HealthSystem.cpp`
- Tests: `src/testing/genetics/test_health_system.cpp` (29 tests)

### OrganismServices

Dependency injection container for shared services:

```cpp
class OrganismServices {
public:
    OrganismServices(std::shared_ptr<GeneRegistry> registry);
    
    IFeedingInteraction& getFeedingInteraction();
    ICombatInteraction& getCombatInteraction();
    ISeedDispersal& getSeedDispersal();
    Navigator& getNavigator();
    PerceptionSystem& getPerceptionSystem();
    HealthSystem& getHealthSystem();
    
    std::unique_ptr<BehaviorController> createBehaviorController();
};
```

## Usage

### Feature Flag

The new behavior system is behind a feature flag in `creature.hpp`:

```cpp
#define USE_NEW_BEHAVIOR_SYSTEM 0  // Set to 1 to enable
```

### Integration Points

In `Creature` class:

```cpp
// Initialize behavior controller with DI
void Creature::initializeBehaviorController() {
    behaviorController_ = services_.createBehaviorController();
    
    // Add behaviors in priority order
    behaviorController_->addBehavior(std::make_unique<RestBehavior>());
    behaviorController_->addBehavior(std::make_unique<HuntingBehavior>(
        services_.getCombatInteraction()));
    behaviorController_->addBehavior(std::make_unique<FeedingBehavior>(
        services_.getFeedingInteraction()));
    behaviorController_->addBehavior(std::make_unique<MatingBehavior>(
        services_.getPerceptionSystem()));
    behaviorController_->addBehavior(std::make_unique<ZoochoryBehavior>(
        services_.getSeedDispersal()));
    behaviorController_->addBehavior(std::make_unique<MovementBehavior>(
        services_.getNavigator()));
}

// Execute behavior system each tick
void Creature::updateWithBehaviors() {
    BehaviorContext ctx = buildBehaviorContext();
    BehaviorResult result = behaviorController_->update(*this, ctx);
    // Handle result...
}

// Create execution context
BehaviorContext Creature::buildBehaviorContext() {
    return BehaviorContext{
        .world = worldQuery_,
        .scentLayer = scentLayer_,
        .deltaTime = 1.0f,
        .currentTick = currentTick_
    };
}
```

### Adding New Behaviors

To add a new behavior:

1. Create header in `include/genetics/behaviors/`:

```cpp
class SwimmingBehavior : public IBehavior {
public:
    std::string getId() const override { return "swimming"; }
    
    bool isApplicable(const IGeneticOrganism& organism,
                      const BehaviorContext& ctx) const override {
        // Check if organism has swimming capability
        float swimmingAbility = organism.getPhenotype().getTrait("swimming_ability");
        return swimmingAbility > 0.3f && ctx.world.isWater(organism.getX(), organism.getY());
    }
    
    float getPriority(const IGeneticOrganism& organism) const override {
        return static_cast<float>(BehaviorPriority::NORMAL);
    }
    
    BehaviorResult execute(IGeneticOrganism& organism, BehaviorContext& ctx) override;
    float getEnergyCost(const IGeneticOrganism& organism) const override;
};
```

2. Implement in `src/genetics/behaviors/`
3. Add to `BehaviorController` in organism initialization
4. Write unit tests

## Design Principles

### Organism-Agnostic

All behaviors use `IGeneticOrganism&` interface, not concrete types:

```cpp
// CORRECT - uses interface
bool isApplicable(const IGeneticOrganism& organism, ...) const {
    float trait = organism.getPhenotype().getTrait("hunt_instinct");
    return trait > 0.4f;
}

// WRONG - type casting
bool isApplicable(const IGeneticOrganism& organism, ...) const {
    const Creature* c = dynamic_cast<const Creature*>(&organism);  // ❌
    return c != nullptr && c->isHungry();
}
```

### Phenotype-Based Checks

Capability checks use phenotype traits, enabling evolutionary emergence:

```cpp
// Gene expression determines behavior availability
float huntInstinct = organism.getPhenotype().getTrait("hunt_instinct");
float locomotion = organism.getPhenotype().getTrait("locomotion_speed");

// Thresholds for behavior activation
if (huntInstinct > 0.4f && locomotion > 0.3f) {
    // Hunting behavior applicable
}
```

### Dependency Injection

No static singletons - all dependencies injected via constructor:

```cpp
HuntingBehavior::HuntingBehavior(ICombatInteraction& combat)
    : combat_(combat) {}
```

## Testing

### Unit Tests

Each behavior has isolated unit tests:

| Test File | Test Count |
|-----------|------------|
| test_behavior_controller.cpp | 13 |
| test_feeding_behavior.cpp | 10 |
| test_hunting_behavior.cpp | 14 |
| test_mating_behavior.cpp | 15 |
| test_movement_behavior.cpp | 12 |
| test_rest_behavior.cpp | 8 |
| test_zoochory_behavior.cpp | 10 |
| test_health_system.cpp | 29 |

### Integration Tests

`test_creature_behavior_integration.cpp` validates:
- Behavior controller with all behaviors
- Priority-based selection
- Energy budget constraints
- Old vs new system equivalence

## See Also

- [[../../../plans/creature-decomposition-progress|Decomposition Progress]] - Implementation progress
- [[../../code-review/recommendations/creature-decomposition-plan|Decomposition Plan]] - Original plan
- [[../../future/genetics/unified-organism-genome|Unified Organism Vision]] - Future unified organism design
- [[../core/01-architecture|Core Architecture]] - Overall system architecture
