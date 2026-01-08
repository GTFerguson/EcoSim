---
title: Genesis - Scenario-Based Evolution Sandbox
created: 2026-01-08
status: concept
genre: God Game / RTS Hybrid
tags: [game-design, god-game, strategy, scenarios]
---

# Genesis: Scenario-Based Evolution Sandbox

**Genre:** God Game / Real-Time Strategy Hybrid
**Tagline:** *Guide evolution. Shape ecosystems. Achieve the impossible.*

---

## Vision Statement

Genesis puts players in the role of a guiding force overseeing an evolving ecosystem. Unlike pure sandbox games, Genesis provides structured challenges that give meaning to evolutionary choices. Players observe, intervene strategically, and watch as their guidance produces emergent solutions to complex problems.

**Core Fantasy:** Being the unseen hand that shapes life across generations.

---

## Core Loop

```mermaid
graph LR
    A[Observe] --> B[Analyze]
    B --> C[Intervene]
    C --> D[Wait]
    D --> A
    D --> E[Achieve Objective]
```

1. **Observe** - Watch ecosystem dynamics unfold
2. **Analyze** - Identify genetic trends, population pressures, opportunities
3. **Intervene** - Make strategic changes (breeding, environment, events)
4. **Wait** - Allow generations to pass, see results
5. **Achieve** - Complete scenario objectives

---

## Intervention Tools

### Breeding Controls

| Tool | Description | Strategic Use |
|------|-------------|---------------|
| **Matchmaker** | Force specific individuals to mate | Combine desired traits |
| **Fertility Blessing** | Boost reproduction rate for selected creature | Spread successful genetics |
| **Isolation** | Separate population into breeding groups | Create subspecies |
| **Migration** | Move creatures to new biomes | Apply selection pressure |

### Environmental Controls

| Tool | Description | Strategic Use |
|------|-------------|---------------|
| **Climate Shift** | Adjust temperature/moisture gradually | Force adaptation |
| **Resource Placement** | Add/remove food sources | Create competition or abundance |
| **Barrier Creation** | Add terrain obstacles | Geographic isolation |
| **Disaster Trigger** | Cause flood, drought, disease | Rapid selection event |

### Divine Interventions

Higher-cost tools for dramatic changes:

| Tool | Description | Cost |
|------|-------------|------|
| **Genetic Injection** | Add specific gene variant to population | High |
| **Mass Extinction** | Remove all of one species | Very High |
| **Genesis Event** | Spawn new creature type | Extreme |
| **Time Acceleration** | Speed up generations | Medium |

---

## Scenario Types

### Tutorial Scenarios

**The First Garden**
- Objective: Maintain stable herbivore population for 20 generations
- Teaches: Basic observation, food chain awareness
- Intervention allowed: Resource placement only

**Predator Introduction**
- Objective: Introduce carnivores without extinction cascade
- Teaches: Balance, population dynamics
- Intervention allowed: Breeding controls + migration

### Challenge Scenarios

**🏔️ Ice Age Survival**
```
Starting Condition: Temperate ecosystem with diverse population
Challenge: Temperature will drop 2°C per 10 generations
Objective: At least one species survives 100 generations
```

**Genetic Solutions:**
- Evolve fur density for insulation
- Evolve hibernation behavior
- Evolve migration instincts
- Evolve fat storage metabolism

**Strategic Approaches:**
1. *Specialist Path* - Breed extreme cold tolerance in one species
2. *Diversity Path* - Maintain multiple species, see what survives
3. *Migration Path* - Guide population to warmer regions

---

**🐺 Apex Predator**
```
Starting Condition: Ecosystem dominated by fast, agile herbivore
Challenge: Herbivore has no natural predator, overgrazing occurs
Objective: Establish stable predator-prey cycle
```

**Genetic Solutions:**
- Evolve hunting speed to catch prey
- Evolve pack behavior for coordinated hunts
- Evolve ambush tactics with camouflage
- Evolve endurance for pursuit hunting

**Strategic Approaches:**
1. *Speed Match* - Breed carnivores faster than prey
2. *Intelligence* - Develop pack hunting coordination
3. *Specialization* - Target young/weak prey with specific adaptations

---

**🌸 The Pollinator Pact**
```
Starting Condition: Self-pollinating plants, omnivore creatures
Challenge: Introduce pollinator-dependent plants
Objective: Stable mutualism between creatures and flowering plants
```

**Genetic Solutions:**
- Evolve color vision in creatures
- Evolve nectar attraction in plants
- Evolve flower-friendly mouth structures
- Evolve creature-friendly pollen delivery

**Coevolutionary Victory:**
Plants with high nectar + creatures with color vision = stable mutualism

---

**⚔️ Arms Race**
```
Starting Condition: Your species vs AI-guided rival species
Challenge: Both compete for limited resources
Objective: Your species achieves 60% population dominance
```

**Strategic Dimensions:**
- Combat effectiveness vs reproductive rate
- Specialist efficiency vs generalist flexibility
- Territory control vs resource extraction
- Defensive adaptations vs offensive capabilities

---

### Sandbox Mode

No objectives. Full tool access. Pure experimentation.

**Optional Challenges:**
- How many stable species can coexist?
- Can you create obligate mutualism?
- How fast can you speciate a population?
- What's the most extreme adaptation possible?

---

## Progression System

### Scenario Completion Rewards

| Achievement | Reward |
|-------------|--------|
| Complete scenario | Unlock next scenario |
| Complete under par time | Unlock efficiency tools |
| Complete with minimal intervention | Unlock sandbox modifiers |
| Complete all variants | Unlock scenario editor |

### Meta-Progression

**God Powers** - Permanent unlocks that carry across scenarios:
- Enhanced observation tools (see gene details)
- Cheaper interventions
- New intervention types
- Time control options

**Achievements** - Track evolutionary milestones:
- "Speciation Event" - Split one species into two stable populations
- "Coevolution Master" - Establish 3+ mutualistic relationships
- "Survival of the Fittest" - Win Arms Race without direct intervention
- "10,000 Generations" - Run continuous simulation for extended period

---

## UI/UX Design

### Main View

```
┌─────────────────────────────────────────────────────────────┐
│  [Scenario: Ice Age Survival]        Gen: 47    Time: 2x    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│                    WORLD VIEW                               │
│                                                             │
│     ~~~ terrain with creatures moving ~~~                   │
│                                                             │
├──────────────────────┬──────────────────────────────────────┤
│  POPULATIONS         │  SELECTED: Alpha-7                   │
│  Herbivores: 234     │  Species: Grazer                     │
│  Carnivores: 12      │  Genes: [Heat Tol: 0.3] [Speed: 0.7] │
│  Plants: 1,205       │  Health: 85%  Energy: 60%            │
│                      │  Offspring: 12  Generation: 5        │
├──────────────────────┴──────────────────────────────────────┤
│  INTERVENTIONS:  [Matchmaker] [Migrate] [Climate] [Event]   │
└─────────────────────────────────────────────────────────────┘
```

### Key Views

1. **World View** - Top-down ecosystem visualization
2. **Population Graph** - Historical population trends
3. **Gene Pool View** - Distribution of traits across population
4. **Family Tree** - Lineage visualization for selected creature
5. **Objective Tracker** - Scenario progress and hints

---

## Engine Integration

### Systems Fully Utilized

| Engine System | Game Use |
|---------------|----------|
| Genetics | Core breeding mechanics, trait visualization |
| Behavior AI | Autonomous creature actions |
| Combat | Predator-prey dynamics |
| Scent | Territory marking, mating detection |
| Coevolution | Plant-creature arms races |
| Spatial Index | Large population simulation |
| Environment | Seasonal/climate challenges |

### Additional Systems Needed

| System | Purpose | Complexity |
|--------|---------|------------|
| **Scenario Framework** | Load objectives, check completion | Medium |
| **Intervention System** | Player tools with costs/cooldowns | Medium |
| **AI Opponent** | Rival species guidance for Arms Race | High |
| **Population Analytics** | Graphs, statistics, projections | Medium |
| **Time Controls** | Pause, speed up, generation skip | Low |
| **Achievement System** | Track and reward milestones | Low |

---

## Multiplayer Considerations

### Competitive Mode

**Parallel Ecosystems:**
- Same starting conditions, different player interventions
- Compare who achieves objective first
- Or compare final ecosystem metrics after fixed time

**Shared Ecosystem:**
- Multiple players guide different species in same world
- Natural competition without direct combat
- Diplomatic options: share territory, trade genetics?

### Asynchronous Mode

- Challenge friends with custom scenarios
- Share seed + intervention log for verification
- Leaderboards for efficiency (fewest interventions)

---

## Target Audience

**Primary:** Strategy gamers who enjoy:
- Emergent systems with surprising outcomes
- Long-term planning with delayed payoffs
- Optimization puzzles with multiple solutions
- Simulation depth with accessible interface

**Secondary:** 
- Science/biology enthusiasts
- Sandbox explorers
- Achievement hunters

**Not For:**
- Players wanting immediate action feedback
- Players who dislike waiting/observation
- Those who prefer deterministic outcomes

---

## Unique Selling Points

1. **Real Genetics** - Not fake stats. Actual inheritance, codominance, mutation.
2. **Emergent Solutions** - Players discover strategies the designers didn't anticipate.
3. **Meaningful Choices** - Interventions have cascading generational effects.
4. **Structured Freedom** - Objectives provide direction; approach is player-chosen.
5. **Living Simulation** - Ecosystem continues evolving even without intervention.

---

## Risk Assessment

| Risk | Mitigation |
|------|------------|
| Too passive/boring | Good time controls, meaningful interventions |
| RNG frustration | Multiple valid approaches, intervention options |
| Scenario too hard | Hint system, difficulty scaling |
| Complexity overwhelming | Strong tutorial, gradual feature unlock |
| Performance at scale | Leverage spatial index, population caps |

---

## Development Priority

**High Priority:**
- Scenario framework with objective checking
- Core intervention tools
- Population analytics UI
- Tutorial scenarios

**Medium Priority:**
- AI opponent for competitive scenarios
- Achievement system
- Time controls and speed options

**Lower Priority:**
- Scenario editor
- Multiplayer modes
- Advanced interventions

---

## See Also

- [[README]] - Game concepts overview
- [[dominion-tribal]] - Alternative: character-driven tribal gameplay
- [[ark-monster-collecting]] - Alternative: collection-focused gameplay
- [[../future/README]] - Engine development roadmap
