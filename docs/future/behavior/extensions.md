---
title: Behavior System Future Extensions
created: 2025-12-25
updated: 2025-12-27
status: future
tags: [behavior, architecture, ai, future]
---

# Behavior System Future Extensions

The current two-layer behavior system provides a solid foundation for creature behavior. This document outlines potential future extensions that could be layered on top of the existing architecture.

## Current System Summary

The behavior system uses a **two-layer architecture**:

### Layer 1: Motivation + Action (Decision Framework)

The underlying decision framework uses Motivation to determine *what the creature wants* and Action to describe *what it's doing*:

```cpp
enum class Motivation { Hungry, Thirsty, Amorous, Tired, Content };
enum class Action { Idle, Searching, Navigating, Eating, Hunting, Fleeing, ... };
```

This provides separation of "why" (motivation) from "what" (action).

### Layer 2: BehaviorController (Modular Execution)

The BehaviorController provides the Strategy pattern for modular, testable behavior implementations:

```cpp
class IBehavior {
    virtual std::string getId() const = 0;
    virtual bool isApplicable(const IGeneticOrganism&, const BehaviorContext&) const = 0;
    virtual float getPriority(const IGeneticOrganism&) const = 0;
    virtual BehaviorResult execute(IGeneticOrganism&, BehaviorContext&) = 0;
};

class BehaviorController {
    void addBehavior(std::unique_ptr<IBehavior> behavior);
    BehaviorResult update(IGeneticOrganism& organism, BehaviorContext& ctx);
};
```

See [[../../technical/systems/behavior-system|Behavior System Architecture]] for implementation details.

---

## Extension 1: Behavior Trees

Layer behavior trees on top of the current action system for more complex decision-making.

### Concept

Instead of procedural code in behavior methods, use a composable tree:

```cpp
// Current: Procedural
void hungryBehavior() {
    if (findPrey()) { setAction(Action::Hunting); }
    else { setAction(Action::Searching); }
}

// Extension: Behavior Tree
BehaviorTree hungerTree = Selector({
    Sequence({ HasPrey(), SetAction(Hunting), ChaseTarget() }),
    Sequence({ FindPrey(), SetAction(Navigating), MoveTo() }),
    SetAction(Searching)
});
```

### Benefits

- **Modular**: Build complex behaviors from simple reusable nodes
- **Visual**: Can create tree editors for non-programmers
- **Testable**: Each node can be tested independently
- **Extensible**: New creature types = new tree configurations

### Implementation Effort

| Task | Estimate |
|------|----------|
| Node base classes | 4-6 hours |
| Common nodes (Selector, Sequence) | 4-6 hours |
| Action nodes | 8-12 hours |
| Migration of current behaviors | 12-16 hours |
| Optional: Visual editor | 20-40 hours |

### When to Consider

- Adding multiple creature archetypes with different behaviors
- Wanting designer-friendly behavior editing
- Need rapid iteration on AI without code changes

---

## Extension 2: Utility AI

Replace priority-based motivation selection with scored utility functions.

### Concept

Instead of fixed priority order, each motivation gets a dynamic score:

```cpp
// Current: Fixed priority
decideBehaviour() {
    if (thirst > threshold) setMotivation(Thirsty);
    else if (hunger > threshold) setMotivation(Hungry);
    // ... priority order
}

// Extension: Utility Scoring
float scoreThirst() {
    float base = thirst / maxThirst;
    float urgency = nearWater ? 0.8f : 1.2f;  // Boost if far from water
    float health = lowHealth ? 0.5f : 1.0f;    // Reduce if dying
    return base * urgency * health;
}
```

### Benefits

- **Nuanced**: Multiple factors influence decisions
- **Emergent**: Creatures develop "personalities" from gene-influenced weights
- **Realistic**: No abrupt priority switches, smooth behavior transitions
- **Evolvable**: Score weights could be genetic traits!

### Implementation Effort

| Task | Estimate |
|------|----------|
| Score evaluator framework | 4-6 hours |
| Score functions per motivation | 6-8 hours |
| Hysteresis/smoothing | 4-6 hours |
| Gene-influenced weights | 8-12 hours |
| Tuning and balancing | 10-20 hours |

### When to Consider

- Wanting more realistic, less "robotic" behavior
- Adding personality systems
- Need behaviors to respond to complex environmental factors

---

## Extension 3: Blackboard System

Centralized data structure for behavior modules to read/write.

### Concept

```cpp
struct CreatureBlackboard {
    // Perception
    std::vector<int> visibleCreatureIds;
    std::vector<int> visiblePlantIds;
    std::optional<int> nearestThreatId;
    std::optional<int> nearestFoodId;
    
    // Memory
    int lastWaterX, lastWaterY;
    int ticksSinceFood;
    std::vector<int> familiarCreatureIds;
    
    // Decisions (written by behavior modules)
    Motivation decidedMotivation;
    Action decidedAction;
    int decidedTargetId;
};

// Modules operate on blackboard
class PerceptionModule { void update(Blackboard& bb); };
class ThreatModule { void evaluate(Blackboard& bb); };
class ForagingModule { void evaluate(Blackboard& bb); };
```

### Benefits

- **Decoupled**: Modules don't know about each other
- **Observable**: Entire creature state in one place
- **Memory**: Can track historical information
- **Debuggable**: Inspect blackboard to understand decisions

### Implementation Effort

| Task | Estimate |
|------|----------|
| Blackboard structure | 2-4 hours |
| Perception module | 6-8 hours |
| Behavior modules | 10-15 hours |
| Memory systems | 8-12 hours |
| Integration with current system | 8-10 hours |

### When to Consider

- Adding creature memory systems
- Need complex perception processing
- Want modular AI that's easy to extend

---

## Extension 4: Goal-Oriented Action Planning (GOAP)

AI that plans sequences of actions to achieve goals.

### Concept

```cpp
// Goals
Goal("Not hungry", achieved = hunger < 0.2);
Goal("Not thirsty", achieved = thirst < 0.2);

// Actions with preconditions and effects
Action("Find Food", 
    precondition = !hasTarget,
    effect = hasTarget = true
);
Action("Navigate", 
    precondition = hasTarget && !atTarget,
    effect = atTarget = true
);
Action("Eat", 
    precondition = atTarget,
    effect = hunger -= 0.3
);

// Planner chains: Find Food → Navigate → Eat → hunger satisfied!
```

### Benefits

- **Adaptive**: AI replans when world changes
- **Emergent**: Complex behavior sequences emerge from simple actions
- **Flexible**: Same actions can achieve different goals
- **Debuggable**: Can inspect current plan

### Implementation Effort

| Task | Estimate |
|------|----------|
| Goal/Action definitions | 6-8 hours |
| A* planner implementation | 15-20 hours |
| World state representation | 8-10 hours |
| Integration | 10-15 hours |
| Optimization | 10-15 hours |

### When to Consider

- Creatures need to solve multi-step problems
- Behaviors should adapt to unexpected situations
- Want truly emergent AI behavior

---

## Extension 5: Hierarchical Task Network (HTN)

Decompose high-level goals into subtasks recursively.

### Concept

```cpp
// High-level task
Task("SatisfyHunger") {
    Methods: [
        Method("Hunt") {
            Precondition: isPredator
            Subtasks: [FindPrey, Chase, Attack, Eat]
        },
        Method("Graze") {
            Precondition: isHerbivore
            Subtasks: [FindPlant, Navigate, Consume]
        },
        Method("Scavenge") {
            Precondition: true  // Always possible
            Subtasks: [FindCorpse, Navigate, Consume]
        }
    ]
};
```

### Benefits

- **Hierarchical**: Natural way to organize complex behaviors
- **Readable**: High-level tasks map to player understanding
- **Efficient**: Only decompose needed subtasks
- **Flexible**: Multiple methods per task

### Implementation Effort

| Task | Estimate |
|------|----------|
| Task/Method structures | 6-8 hours |
| Decomposition engine | 12-16 hours |
| Primitive task execution | 8-10 hours |
| Integration | 10-15 hours |

---

## Hybrid Approach Recommendation

Rather than replacing the current system, extensions should **layer on top**:

```
┌─────────────────────────────────────────────────┐
│            High-Level AI (Future)                │
│   ┌─────────────┐  ┌──────────────────────────┐ │
│   │ Behavior    │  │ Utility AI / GOAP / HTN  │ │
│   │ Trees       │  │                          │ │
│   └──────┬──────┘  └────────────┬─────────────┘ │
├──────────┼──────────────────────┼───────────────┤
│          │                      │               │
│          ▼                      ▼               │
│   ┌─────────────────────────────────────────┐   │
│   │      Motivation + Action System         │   │
│   │      (Current Implementation)           │   │
│   └─────────────────────────────────────────┘   │
│                      │                          │
│                      ▼                          │
│   ┌─────────────────────────────────────────┐   │
│   │        Creature Class / Methods          │   │
│   │        (Movement, Combat, etc.)          │   │
│   └─────────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
```

### Incremental Path

1. **Current**: Motivation + Action (implemented ✅)
2. **Near-term**: Add Blackboard for perception/memory
3. **Medium-term**: Utility AI for motivation selection
4. **Long-term**: Behavior Trees OR GOAP for action selection

---

## Comparison Matrix

| Extension | Complexity | Performance | Debuggability | When to Use |
|-----------|------------|-------------|---------------|-------------|
| **Behavior Trees** | ★★★☆☆ | ★★★★☆ | ★★★☆☆ | Multiple creature archetypes |
| **Utility AI** | ★★★☆☆ | ★★★☆☆ | ★★☆☆☆ | Personality/nuanced behavior |
| **Blackboard** | ★★☆☆☆ | ★★★★☆ | ★★★★★ | Memory/perception systems |
| **GOAP** | ★★★★★ | ★★☆☆☆ | ★★★☆☆ | Complex problem-solving |
| **HTN** | ★★★★☆ | ★★★☆☆ | ★★★★☆ | Hierarchical task breakdown |

---

## See Also

- [[../technical/systems/behavior-state|Behavior State System]] - Current implementation
- [[genetics/improvements|Genetic Improvements]] - Evolution of behavior genes
- [[environment/ecosystem-improvements|Ecosystem Improvements]] - Environmental factors
