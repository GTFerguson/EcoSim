---
title: Creature Behavior State System
created: 2025-12-25
updated: 2025-12-25
status: implemented
tags: [creatures, behavior, motivation, action, architecture]
---

# Creature Behavior State System

The behavior state system manages what creatures want (motivation) and what they're doing (action). This hierarchical two-level architecture enables complex behaviors while maintaining clean separation between goals and actions.

## Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     BehaviorState                           │
│  ┌──────────────┐          ┌────────────────────────────┐  │
│  │  Motivation  │ ────────►│         Action             │  │
│  │  (the why)   │          │        (the what)          │  │
│  └──────────────┘          └────────────────────────────┘  │
│                                                             │
│  Hungry ───────► Searching → Hunting → Chasing → Eating   │
│  Thirsty ──────► Searching → Navigating → Drinking        │
│  Amorous ──────► Searching → Courting → Mating            │
│  Tired ────────► Resting                                   │
│  Content ──────► Wandering                                 │
└─────────────────────────────────────────────────────────────┘
```

## Core Types

### Motivation Enum

Motivations represent what a creature *wants* - their high-level drive or goal.

```cpp
enum class Motivation {
    Hungry,     // Needs food
    Thirsty,    // Needs water
    Amorous,    // Has urge to find mate
    Tired,      // Needs rest
    Content     // No pressing needs
};
```

| Motivation | Trigger | Behavior Method |
|------------|---------|-----------------|
| `Hungry` | Food need > threshold | [`hungryBehavior()`](src/objects/creature/creature.cpp:1086) |
| `Thirsty` | Water need > threshold | [`thirstyBehavior()`](src/objects/creature/creature.cpp:1184) |
| `Amorous` | Breeding urge > threshold | [`amorousBehavior()`](src/objects/creature/creature.cpp:1189) |
| `Tired` | Fatigue > threshold | [`tiredBehavior()`](src/objects/creature/creature.cpp:1253) |
| `Content` | No urgent needs | [`contentBehavior()`](src/objects/creature/creature.cpp:1060) |

### Action Enum

Actions represent what a creature is *doing* - their observable behavior.

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

### BehaviorState Struct

The combined state structure used for tracking and display.

```cpp
struct BehaviorState {
    Motivation motivation = Motivation::Content;
    Action action = Action::Idle;
    int targetId = -1;              // ID-based targeting (not pointer)
    int ticksInAction = 0;          // Duration tracking
    Action previousAction = Action::Idle;  // For transition detection
};
```

## Key Design Decisions

### ID-Based Targeting

Targets are stored as creature IDs rather than pointers to prevent dangling pointer issues when creatures die or vectors resize.

```cpp
// Setting a target
creature.setTargetId(target.getId());

// Checking for target
if (creature.hasTarget()) {
    // Resolve ID to creature when needed
}

// Clearing target
creature.clearTarget();
```

### Action Transition Tracking

The [`setAction()`](src/objects/creature/creature.cpp:93) method automatically tracks transitions:

```cpp
void Creature::setAction(Action a) {
    if (_behaviorState.action != a) {
        _behaviorState.previousAction = _behaviorState.action;
        _behaviorState.ticksInAction = 0;
    }
    _behaviorState.action = a;
}
```

### Combat as Actions

Combat states are now represented as Actions rather than separate boolean flags:

| Old System | New System |
|------------|------------|
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

## State Transitions

### Motivation Selection

[`decideBehaviour()`](src/objects/creature/creature.cpp:1338) selects motivation based on need priorities:

```cpp
// Priority order (highest to lowest)
enum Priority { water, food, rest, mate, explore };

// Maps to Motivation
Motivation priorities[] = {
    Motivation::Thirsty,  // water
    Motivation::Hungry,   // food
    Motivation::Tired,    // rest
    Motivation::Amorous,  // mate
    Motivation::Content   // explore (default)
};
```

### Action Flow Example: Hungry Carnivore

```
decideBehaviour()
    │
    ▼
setMotivation(Motivation::Hungry)
    │
    ▼
hungryBehavior()
    │
    ├──► No prey visible ──► setAction(Action::Searching)
    │
    ├──► Prey detected ──► setAction(Action::Hunting)
    │                         │
    │                         ▼
    │                      enterCombat(prey)
    │                         │
    │                         ├──► setAction(Action::Chasing)
    │                         │
    │                         └──► Target reached ──► setAction(Action::Attacking)
    │                                                    │
    │                                                    ▼
    │                                                 Kill prey
    │                                                    │
    │                                                    ▼
    │                                              setAction(Action::Eating)
    │
    └──► Corpse found ──► setAction(Action::Eating)
```

## API Reference

### Accessor Methods

```cpp
// Get entire state
const BehaviorState& getBehaviorState() const;

// Individual accessors
Motivation getMotivation() const;
Action getAction() const;
int getTargetId() const;
int getTicksInAction() const;
```

### Mutator Methods

```cpp
// Set motivation (high-level goal)
void setMotivation(Motivation m);

// Set action (current activity) - tracks transitions
void setAction(Action a);

// Target management
void setTargetId(int id);
void clearTarget();
```

### Utility Methods

```cpp
// Convenience checks
bool isInCombat() const;
bool isFleeing() const;
bool hasTarget() const;

// String conversion (for serialization/display)
static Motivation stringToMotivation(const std::string& str);
std::string motivationToString() const;
std::string actionToString() const;
```

## Serialization

### Save Format

Only `Motivation` is persisted (Action is transient):

```cpp
// In Creature::toString()
ss << motivationToString() << ":";  // Field position unchanged from Profile
```

### Backward Compatibility

[`stringToMotivation()`](src/objects/creature/creature.cpp:2726) handles legacy Profile names:

```cpp
// New names
"hungry"  → Motivation::Hungry
"thirsty" → Motivation::Thirsty
"amorous" → Motivation::Amorous
"tired"   → Motivation::Tired
"content" → Motivation::Content

// Legacy names (backward compatible)
"breed"   → Motivation::Amorous
"sleep"   → Motivation::Tired
"migrate" → Motivation::Content
```

## Usage in Rendering

### Color Mapping

Renderers use Motivation for creature color:

```cpp
switch(creature.getMotivation()) {
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
ImGui::Text("Motivation: %s", creature.motivationToString().c_str());
ImGui::Text("Action: %s", creature.actionToString().c_str());
if (creature.hasTarget()) {
    ImGui::Text("Target ID: %d", creature.getTargetId());
}
```

## Files

| File | Purpose |
|------|---------|
| [`include/objects/creature/creature.hpp`](include/objects/creature/creature.hpp) | Enum/struct definitions, method declarations |
| [`src/objects/creature/creature.cpp`](src/objects/creature/creature.cpp) | Behavior method implementations |
| [`src/main.cpp`](src/main.cpp) | Main loop motivation dispatch |

## See Also

- [[../reference/api/core-classes|Core Classes API]] - Creature class reference
- [[../../user/creatures/behavior|User Guide: Creature Behavior]] - User-facing behavior documentation
- [[scent-system|Scent System]] - How creatures detect mates via scent
