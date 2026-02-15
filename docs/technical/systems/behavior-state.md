---
title: Creature Behavior State System
created: 2025-12-25
updated: 2026-02-15
status: implemented
tags: [creatures, behavior, motivation, action, architecture]
---

# Creature Behavior State System

The behavior system uses a **BehaviorController** to select and execute behaviors each tick, with **Motivation/Action** enums derived from the active behavior for UI and rendering observability.

## Overview

```
┌──────────────────────────────────────────────────────────────────┐
│                     BehaviorController                            │
│                                                                   │
│  Each tick: evaluate all behaviors → pick highest priority →     │
│  execute → derive Motivation/Action from result                  │
│                                                                   │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────────┐    │
│  │  Rest    │  │  Thirst  │  │ Hunting  │  │   Feeding    │    │
│  │ CRITICAL │  │   HIGH   │  │   HIGH   │  │   NORMAL     │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────────┘    │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐                      │
│  │  Mating  │  │ Zoochory │  │ Movement │                      │
│  │  NORMAL  │  │   LOW    │  │   IDLE   │                      │
│  └──────────┘  └──────────┘  └──────────┘                      │
│                                                                   │
│  After execution:                                                 │
│  ┌──────────────┐          ┌────────────────────────────┐       │
│  │  Motivation  │ ◄─────── │  Derived from behavior ID  │       │
│  │  (the why)   │          │  and BehaviorResult        │       │
│  └──────────────┘          └────────────────────────────┘       │
│                                                                   │
│  feeding/hunting → Hungry    thirst → Thirsty                   │
│  mating → Amorous            rest → Tired                       │
│  movement/other → Content                                        │
└──────────────────────────────────────────────────────────────────┘
```

## Architecture

### BehaviorController

The `BehaviorController` owns a list of `IBehavior` implementations. Each tick it:

1. Calls `isApplicable()` on each behavior to filter candidates
2. Calls `getPriority()` on applicable behaviors to rank them
3. Executes the highest-priority behavior via `execute()`
4. Returns a `BehaviorResult` describing what happened

```cpp
// In Creature::updateWithBehaviors()
BehaviorResult result = _behaviorController->update(*this, ctx);
```

### IBehavior Interface

All behaviors implement the `IBehavior` interface, operating on `Organism&` (not `Creature&`) for species-agnostic design:

```cpp
class IBehavior {
public:
    virtual std::string getId() const = 0;
    virtual bool isApplicable(const Organism& organism, const BehaviorContext& ctx) const = 0;
    virtual float getPriority(const Organism& organism) const = 0;
    virtual BehaviorResult execute(Organism& organism, BehaviorContext& ctx) = 0;
    virtual float getEnergyCost(const Organism& organism) const = 0;
};
```

### OrganismNeeds

Behaviors read and write creature state through `OrganismNeeds`, a shared struct on the `Organism` base class:

```cpp
struct OrganismNeeds {
    float energy = 10.0f;          // Maps to creature hunger
    float hydration = 10.0f;       // Maps to creature thirst
    float fatigue = 0.0f;          // Maps to creature fatigue
    float reproductiveUrge = 0.0f; // Maps to creature mate drive
    float maxEnergy = 10.0f;
    float maxHydration = 10.0f;
};
```

### BehaviorContext

Each behavior receives a `BehaviorContext` providing access to the world:

```cpp
struct BehaviorContext {
    World* world;
    ScentLayer* scentLayer;
    SpatialIndex* creatureIndex;
    int worldRows, worldCols;
    unsigned int currentTick;

    // Query nearby organisms via SpatialIndex
    std::vector<Organism*> queryNearbyOrganisms(float x, float y, float radius) const;
};
```

## Registered Behaviors

Behaviors are registered in priority order during `initializeBehaviorController()`:

| Behavior | ID | Priority Tier | Reads | Writes |
|----------|----|---------------|-------|--------|
| RestBehavior | `"rest"` | CRITICAL | `needs_.fatigue` | `needs_.fatigue` |
| ThirstBehavior | `"thirst"` | HIGH | `needs_.hydration` | `needs_.hydration` |
| HuntingBehavior | `"hunting"` | HIGH | `needs_.energy` | `needs_.energy` |
| FeedingBehavior | `"feeding"` | NORMAL | `needs_.energy` | `needs_.energy` |
| MatingBehavior | `"mating"` | NORMAL | `needs_.reproductiveUrge`, `needs_.energy` | `needs_.reproductiveUrge` |
| ZoochoryBehavior | `"zoochory"` | LOW | Seed readiness | Disperses seeds |
| MovementBehavior | `"movement"` | IDLE | Always applicable | Random movement |

Each behavior's `isApplicable()` gates whether it can run (e.g., FeedingBehavior checks if the organism can digest plants and is below the hunger threshold). The `getPriority()` then scales with urgency — a starving creature's FeedingBehavior has higher priority than a slightly hungry one.

## Tick Execution Flow

```
updateWithBehaviors(ctx)
    │
    ├── 1. BehaviorController selects and executes behavior
    │       └── Behaviors read/write organism.getNeeds()
    │
    ├── 2. Sync needs_ → legacy floats (_hunger, _thirst, etc.)
    │
    ├── 3. Apply behavior energy cost
    │
    ├── 4. Derive Motivation/Action from active behavior ID
    │
    ├── 5. Growth tick
    │
    ├── 6. Metabolism (drain hunger/thirst, accumulate fatigue)
    │       └── Environmental stress modifies drain rate
    │
    ├── 7. Mate drive update (increases when content, decreases when hungry/thirsty)
    │
    ├── 8. Sync legacy floats → needs_
    │
    └── 9. Age increment
```

## Motivation and Action Derivation

After the BehaviorController executes, Motivation and Action are derived from the active behavior:

```cpp
std::string behaviorId = _behaviorController->getCurrentBehaviorId();

if (behaviorId == "feeding")       { _motivation = Motivation::Hungry;   _action = Action::Grazing;   }
else if (behaviorId == "hunting")  { _motivation = Motivation::Hungry;   _action = Action::Hunting;   }
else if (behaviorId == "thirst")   { _motivation = Motivation::Thirsty;  _action = result.completed ? Action::Drinking : Action::Searching; }
else if (behaviorId == "mating")   { _motivation = Motivation::Amorous;  _action = Action::Courting;  }
else if (behaviorId == "rest")     { _motivation = Motivation::Tired;    _action = Action::Resting;   }
else if (behaviorId == "movement") { _motivation = Motivation::Content;  _action = Action::Wandering; }
else                               { _motivation = Motivation::Content;  _action = Action::Idle;      }
```

This means Motivation and Action are **observability** — they reflect what the BehaviorController decided, not what drove the decision.

## Core Types

### Motivation Enum

Motivations represent what a creature *wants* — derived from the active behavior after each tick.

```cpp
enum class Motivation {
    Hungry,     // Feeding or hunting behavior active
    Thirsty,    // Thirst behavior active
    Amorous,    // Mating behavior active
    Tired,      // Rest behavior active
    Content     // Movement or no urgent needs
};
```

### Action Enum

Actions represent what a creature is *doing* — the observable activity.

```cpp
enum class Action {
    Idle,           // No current action
    Searching,      // Looking for target (food/water/mate)
    Navigating,     // Moving toward known target
    Eating,         // Consuming food (corpse or meat)
    Drinking,       // Consuming water
    Hunting,        // Pursuing prey (predator behavior)
    Chasing,        // Active pursuit of target
    Attacking,      // In combat, dealing damage
    Fleeing,        // Running from threat
    Resting,        // Sleep/recovery
    Courting,       // Breeding behavior - seeking mate
    Mating,         // Reproduction in progress
    Wandering,      // No goal, random movement
    Grazing         // Herbivore eating plants
};
```

## Key Design Decisions

### ID-Based Targeting

Targets are stored as creature IDs rather than pointers to prevent dangling pointer issues when creatures die or vectors resize.

```cpp
creature.setTargetId(target.getId());

if (creature.hasTarget()) {
    // Resolve ID to creature when needed
}

creature.clearTarget();
```

### Combat as Actions

Combat states are represented as Actions rather than separate boolean flags:

| Legacy | Current |
|--------|---------|
| `_inCombat = true` | `setAction(Action::Chasing)` or `Action::Attacking` |
| `_isFleeing = true` | `setAction(Action::Fleeing)` |
| `_combatTarget = ptr` | `setTargetId(target.getId())` |

Convenience methods provide backward-compatible checks:

```cpp
bool isInCombat() const {
    return _behaviorState.action == Action::Attacking ||
           _behaviorState.action == Action::Chasing;
}

bool isFleeing() const {
    return _behaviorState.action == Action::Fleeing;
}
```

## Serialization

### JSON Format

Both Motivation and Action are serialized:

```json
{
    "behavior": {
        "motivation": "HUNGRY",
        "action": "GRAZING"
    }
}
```

### String Format

Motivation is included in the comma-separated string serialization:

```cpp
ss << motivationToString(creature.getMotivation()) << ",";
```

## Usage in Rendering

### Color Mapping

Renderers use Motivation for creature color:

```cpp
switch (creature.getMotivation()) {
    case Motivation::Hungry:   return RED;
    case Motivation::Thirsty:  return BLUE;
    case Motivation::Amorous:  return PINK;
    case Motivation::Tired:    return GRAY;
    case Motivation::Content:  return GREEN;
}
```

### Inspector Display

The ImGui overlay displays both Motivation and Action:

```cpp
ImGui::Text("Motivation: %s", motivationToString(creature.getMotivation()).c_str());
ImGui::Text("Action: %s", actionToString(creature.getAction()).c_str());
if (creature.hasTarget()) {
    ImGui::Text("Target ID: %d", creature.getTargetId());
}
```

## Files

| File | Purpose |
|------|---------|
| `include/genetics/behaviors/IBehavior.hpp` | Behavior interface |
| `include/genetics/behaviors/BehaviorContext.hpp` | Context struct with world access |
| `include/genetics/behaviors/BehaviorController.hpp` | Controller that selects/executes behaviors |
| `src/genetics/behaviors/*.cpp` | Individual behavior implementations |
| `include/genetics/organisms/OrganismNeeds.hpp` | Shared needs struct |
| `src/objects/creature/BehaviorContextImpl.cpp` | Context builder with SpatialIndex queries |
| `include/objects/creature/creature.hpp` | Motivation/Action enums, BehaviorState struct |
| `src/objects/creature/creature.cpp` | `updateWithBehaviors()` — tick execution and M/A derivation |

## See Also

- [[../reference/api/core-classes|Core Classes API]] - Creature class reference
- [[../../user/creatures/behavior|User Guide: Creature Behavior]] - User-facing behavior documentation
- [[scent-system|Scent System]] - How creatures detect mates via scent
