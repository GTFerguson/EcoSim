---
title: Behavior System Architecture
created: 2025-12-27
updated: 2025-12-27
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

## Core Components

### IBehavior Interface

Base interface for all behaviors in [`include/genetics/behaviors/IBehavior.hpp`](../../../include/genetics/behaviors/IBehavior.hpp):

```cpp
class IBehavior {
public:
    virtual ~IBehavior() = default;
    
    // Unique behavior identifier (e.g., "hunting", "feeding", "mating")
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

Orchestrates behavior execution using priority-based selection in [`include/genetics/behaviors/BehaviorController.hpp`](../../../include/genetics/behaviors/BehaviorController.hpp):

```cpp
class BehaviorController {
public:
    BehaviorController() = default;
    ~BehaviorController() = default;
    
    // Non-copyable, movable
    BehaviorController(const BehaviorController&) = delete;
    BehaviorController& operator=(const BehaviorController&) = delete;
    BehaviorController(BehaviorController&&) = default;
    BehaviorController& operator=(BehaviorController&&) = default;
    
    void addBehavior(std::unique_ptr<IBehavior> behavior);
    void removeBehavior(const std::string& behaviorId);
    bool hasBehavior(const std::string& behaviorId) const;
    void clearBehaviors();
    
    // Execute highest-priority applicable behavior
    BehaviorResult update(IGeneticOrganism& organism, BehaviorContext& ctx);
    
    const std::string& getCurrentBehaviorId() const;
    std::size_t getBehaviorCount() const;
    std::vector<std::string> getBehaviorIds() const;
    std::string getStatusString() const;
};
```

The controller evaluates all behaviors for applicability, sorts by priority (highest first with stable sort for equal priorities), and executes the top one.

### BehaviorContext

Execution context providing world/scent layer access in [`include/genetics/behaviors/BehaviorContext.hpp`](../../../include/genetics/behaviors/BehaviorContext.hpp):

```cpp
struct BehaviorContext {
    const ScentLayer* scentLayer = nullptr;       // Scent layer for olfactory perception
    World* world = nullptr;                        // World access for entity queries
    const OrganismState* organismState = nullptr;  // Current organism state (energy, health, etc.)
    float deltaTime = 1.0f;                        // Time since last tick (normalized)
    unsigned int currentTick = 0;                  // Current simulation tick
    int worldRows = 0;                             // World height in tiles
    int worldCols = 0;                             // World width in tiles
};
```

> [!NOTE]
> Context uses raw pointers for optional dependencies. Behaviors should null-check before use.

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

| Behavior | ID | Purpose | Base Priority | Priority Range |
|----------|-----|---------|---------------|----------------|
| RestBehavior | `"rest"` | Fatigue management, sleep recovery | NORMAL (50) | 50-75 |
| HuntingBehavior | `"hunting"` | Predator hunting with extinction fixes | NORMAL (50) | 50-75 |
| FeedingBehavior | `"feeding"` | Herbivore plant feeding | NORMAL (50) | 50-75 |
| MatingBehavior | `"mating"` | Sexual reproduction | NORMAL (50) | 50-75 |
| ZoochoryBehavior | `"zoochory"` | Seed dispersal (burrs, gut passage) | IDLE (0) | 0 |
| MovementBehavior | `"movement"` | Target-based navigation | IDLE (0) | 0 |

### FeedingBehavior

Herbivore plant consumption in [`include/genetics/behaviors/FeedingBehavior.hpp`](../../../include/genetics/behaviors/FeedingBehavior.hpp):

**Applicability:**
- Organism has `plant_digestion` trait > 0.1
- Organism's hunger is below threshold (hungry)

**Priority:**
- Base: NORMAL (50)
- Scales with hunger: adds `(1 - hungerRatio) * 25` when hungry
- Maximum: 75

**Execution:**
- Finds nearest edible plant within detection range
- If adjacent (distance ≤ 1.5), eats using FeedingInteraction
- If not adjacent, signals movement is needed

**Constants:**

| Constant | Value | Purpose |
|----------|-------|---------|
| `PLANT_DIGESTION_THRESHOLD` | 0.1 | Minimum trait to eat plants |
| `DEFAULT_HUNGER_THRESHOLD` | 0.5 | Seek food below this |
| `BASE_ENERGY_COST` | 0.01 | Energy per feeding tick |
| `ADJACENT_DISTANCE` | 1.5 | Distance threshold for eating |

### HuntingBehavior

Predator hunting with extinction bug fixes in [`include/genetics/behaviors/HuntingBehavior.hpp`](../../../include/genetics/behaviors/HuntingBehavior.hpp):

**Applicability:**
- Organism has `hunt_instinct` trait > 0.4
- Organism has `locomotion_speed` > 0.3 (can chase)
- Organism is NOT satiated (energy < 80%)
- Organism is NOT on cooldown

**Priority:**
- Base: NORMAL (50)
- Scales with hunger level
- Maximum: 75

**Critical Bug Fixes:**
- **Satiation check**: No hunting when 80% full
- **Cooldown**: 30 ticks minimum between hunts
- **Energy cost**: 1.5 energy per hunt attempt
- **Escape mechanics**: Uses `flee_threshold` vs `pursue_threshold` genes

**Escape Formula:**
```cpp
escapeChance = prey_flee / (prey_flee + predator_pursue + 0.1)
```

**Constants:**

| Constant | Value | Purpose |
|----------|-------|---------|
| `HUNT_INSTINCT_THRESHOLD` | 0.4 | Minimum trait to hunt |
| `LOCOMOTION_THRESHOLD` | 0.3 | Minimum speed to chase |
| `SATIATION_THRESHOLD` | 0.8 | Too full to hunt |
| `HUNT_COST` | 1.5 | Energy per hunt attempt |
| `HUNT_COOLDOWN` | 30 | Ticks between hunts |

### MatingBehavior

Sexual reproduction in [`include/genetics/behaviors/MatingBehavior.hpp`](../../../include/genetics/behaviors/MatingBehavior.hpp):

**Applicability:**
- Organism has `mate_value` above threshold (0.7)
- Organism has sufficient resources (hunger > 5.0)
- Organism is mature (age meets maturity ratio)

**Priority:**
- Base: NORMAL (50)
- Scales with mate value
- Maximum: 75

**Execution:**
- Finds potential mate via perception
- Evaluates fitness/compatibility (prefer similar but not identical)
- Creates offspring with genetic crossover
- Deducts BREED_COST (3.0) from both parents

**Offspring Callback:**
```cpp
using OffspringCallback = std::function<void(std::unique_ptr<IGeneticOrganism>)>;
void setOffspringCallback(OffspringCallback callback);
```

**Constants:**

| Constant | Value | Purpose |
|----------|-------|---------|
| `MATE_THRESHOLD` | 0.7 | Minimum desire to mate |
| `BREED_COST` | 3.0 | Energy cost for breeding |
| `MIN_HUNGER_TO_BREED` | 5.0 | Resource requirement |
| `MATURITY_AGE_RATIO` | 0.1 | Age fraction for maturity |
| `IDEAL_SIMILARITY` | 0.8 | Preferred genetic similarity |

### RestBehavior

Fatigue management in [`include/genetics/behaviors/RestBehavior.hpp`](../../../include/genetics/behaviors/RestBehavior.hpp):

**Applicability:**
- Organism's fatigue exceeds threshold
- Organism has fatigue tracking (phenotype traits present)

**Priority:**
- Base: NORMAL (50)
- Adds `fatigueRatio * 25` when tired
- Maximum: 75

**Execution:**
- Reduces fatigue by recovery rate per tick
- Minimal energy cost (resting conserves energy)
- Completes when fatigue drops below threshold

**Constants:**

| Constant | Value | Purpose |
|----------|-------|---------|
| `DEFAULT_FATIGUE_THRESHOLD` | 3.0 | Need rest above this |
| `DEFAULT_RECOVERY_RATE` | 0.01 | Fatigue reduction per tick |
| `REST_ENERGY_COST` | 0.005 | Very low energy cost |

### MovementBehavior

Target-based navigation in [`include/genetics/behaviors/MovementBehavior.hpp`](../../../include/genetics/behaviors/MovementBehavior.hpp):

**Applicability:**
- Organism has `locomotion_speed` > 0.3
- Has a valid target set

**Priority:**
- IDLE (0) - fallback behavior

**Target Management:**
```cpp
void setTarget(int targetX, int targetY);
void clearTarget();
bool hasTarget() const;
std::pair<int, int> getTarget() const;
```

**Constants:**

| Constant | Value | Purpose |
|----------|-------|---------|
| `LOCOMOTION_THRESHOLD` | 0.3 | Minimum speed to move |
| `BASE_MOVEMENT_COST` | 0.01 | Energy per tile |
| `DIAGONAL_COST_MULTIPLIER` | 1.414 | Extra cost for diagonal |

### ZoochoryBehavior

Seed dispersal in [`include/genetics/behaviors/ZoochoryBehavior.hpp`](../../../include/genetics/behaviors/ZoochoryBehavior.hpp):

Handles animal-mediated seed dispersal:
- **Endozoochory**: Seeds consumed with fruit, pass through gut
- **Epizoochory**: Burrs/hooks that attach to animal fur

**Applicability:**
- Always applicable (passive processing)

**Priority:**
- IDLE (0) - runs when no higher priority behaviors active

**Public API:**
```cpp
// Epizoochory - burr attachment
void attachBurr(unsigned int organismId, int plantX, int plantY, int strategy);
bool hasBurrs(unsigned int organismId) const;

// Endozoochory - gut seed passage
void consumeSeeds(unsigned int organismId, int plantX, int plantY,
                  int count, float viability);

// Tick processing
std::vector<DispersalEvent> processOrganismSeeds(unsigned int organismId,
                                                  int currentX, int currentY,
                                                  int ticksElapsed);

// Cleanup (on organism death)
void clearOrganismData(unsigned int organismId);
```

**Constants:**

| Constant | Value | Purpose |
|----------|-------|---------|
| `BURR_DETACH_CHANCE` | 0.05 | Probability per tick |
| `GUT_TRANSIT_TICKS` | 500 | Default gut passage time |

## Shared Systems

### OrganismServices

Dependency injection container in [`include/genetics/behaviors/OrganismServices.hpp`](../../../include/genetics/behaviors/OrganismServices.hpp):

```cpp
class OrganismServices {
public:
    OrganismServices(GeneRegistry& registry, 
                     Navigator& navigator,
                     World& world);
    ~OrganismServices();
    
    // Move-only (owns systems)
    OrganismServices(OrganismServices&&) noexcept;
    OrganismServices& operator=(OrganismServices&&) noexcept;
    
    // External dependencies (referenced, not owned)
    GeneRegistry& getGeneRegistry();
    Navigator& getNavigator();
    World& getWorld();
    
    // Owned systems (created lazily on first access)
    PerceptionSystem& getPerceptionSystem();
    HealthSystem& getHealthSystem();
    FeedingInteraction& getFeedingInteraction();
    CombatInteraction& getCombatInteraction();
    SeedDispersal& getSeedDispersal();
};
```

Services are created lazily on first access and cached for the container's lifetime.

### HealthSystem

Organism-agnostic health management shared by Creature and Plant:

Located in:
- Header: `include/genetics/systems/HealthSystem.hpp`
- Source: `src/genetics/systems/HealthSystem.cpp`

### IWorldQuery

Read-only world query interface in [`include/genetics/behaviors/IWorldQuery.hpp`](../../../include/genetics/behaviors/IWorldQuery.hpp):

```cpp
class IWorldQuery {
public:
    virtual ~IWorldQuery() = default;
    
    virtual const Tile& getTile(int x, int y) const = 0;
    virtual float getTemperature(int x, int y) const = 0;
    virtual bool isWater(int x, int y) const = 0;
    
    virtual std::vector<const Creature*> getCreaturesInRadius(
        int x, int y, float radius) const = 0;
    virtual std::vector<const Plant*> getPlantsInRadius(
        int x, int y, float radius) const = 0;
    
    virtual const ScentLayer& getScentLayer() const = 0;
    virtual unsigned int getCurrentTick() const = 0;
    virtual int getRows() const = 0;
    virtual int getCols() const = 0;
};
```

## Usage

### Integration in Creature

In `Creature` class:

```cpp
// Initialize behavior controller
void Creature::initializeBehaviors(OrganismServices& services) {
    behaviorController_ = std::make_unique<BehaviorController>();
    
    // Add behaviors - controller selects highest priority applicable
    behaviorController_->addBehavior(std::make_unique<RestBehavior>());
    behaviorController_->addBehavior(std::make_unique<HuntingBehavior>(
        services.getCombatInteraction(), services.getPerceptionSystem()));
    behaviorController_->addBehavior(std::make_unique<FeedingBehavior>(
        services.getFeedingInteraction(), services.getPerceptionSystem()));
    behaviorController_->addBehavior(std::make_unique<MatingBehavior>(
        services.getPerceptionSystem(), services.getGeneRegistry()));
    behaviorController_->addBehavior(std::make_unique<ZoochoryBehavior>(
        services.getSeedDispersal()));
    behaviorController_->addBehavior(std::make_unique<MovementBehavior>());
}

// Execute each tick
void Creature::update() {
    BehaviorContext ctx{
        .scentLayer = &scentLayer_,
        .world = &world_,
        .organismState = &state_,
        .deltaTime = 1.0f,
        .currentTick = currentTick_,
        .worldRows = world_.getRows(),
        .worldCols = world_.getCols()
    };
    
    BehaviorResult result = behaviorController_->update(*this, ctx);
    // Handle result...
}
```

### Adding New Behaviors

To add a new behavior:

1. Create header in `include/genetics/behaviors/`:

```cpp
class SwimmingBehavior : public IBehavior {
public:
    SwimmingBehavior() = default;
    ~SwimmingBehavior() override = default;
    
    std::string getId() const override { return "swimming"; }
    
    bool isApplicable(const IGeneticOrganism& organism,
                      const BehaviorContext& ctx) const override {
        // Check if organism has swimming capability
        const Phenotype& phenotype = organism.getPhenotype();
        float swimmingAbility = phenotype.getTrait("swimming_ability");
        
        if (ctx.world == nullptr) return false;
        return swimmingAbility > 0.3f && 
               ctx.world->isWater(organism.getX(), organism.getY());
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
    const Phenotype& phenotype = organism.getPhenotype();
    float trait = phenotype.getTrait("hunt_instinct");
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
const Phenotype& phenotype = organism.getPhenotype();
float huntInstinct = phenotype.getTrait("hunt_instinct");
float locomotion = phenotype.getTrait("locomotion_speed");

// Thresholds for behavior activation
if (huntInstinct > 0.4f && locomotion > 0.3f) {
    // Hunting behavior applicable
}
```

### Dependency Injection

No static singletons - all dependencies injected via constructor:

```cpp
HuntingBehavior::HuntingBehavior(CombatInteraction& combat, 
                                  PerceptionSystem& perception)
    : combat_(combat), perception_(perception) {}

FeedingBehavior::FeedingBehavior(FeedingInteraction& feeding,
                                  PerceptionSystem& perception)
    : feeding_(feeding), perception_(perception) {}
```

## Testing

### Unit Tests

Each behavior has isolated unit tests:

| Test File | Tests |
|-----------|-------|
| `test_behavior_controller.cpp` | Controller selection logic |
| `test_feeding_behavior.cpp` | Herbivore feeding |
| `test_hunting_behavior.cpp` | Predator hunting + extinction fixes |
| `test_mating_behavior.cpp` | Reproduction |
| `test_movement_behavior.cpp` | Navigation |
| `test_rest_behavior.cpp` | Fatigue recovery |
| `test_zoochory_behavior.cpp` | Seed dispersal |
| `test_health_system.cpp` | Health management |

### Integration Tests

`test_creature_behavior_integration.cpp` validates:
- Behavior controller with all behaviors
- Priority-based selection
- Energy budget constraints
- Behavior state transitions

## See Also

- [[../../future/behavior/extensions|Behavior Extensions]] - Future AI extensions (behavior trees, GOAP)
- [[../../future/genetics/unified-organism-genome|Unified Organism Vision]] - Future unified organism design
- [[../core/01-architecture|Core Architecture]] - Overall system architecture
