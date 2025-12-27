---
title: Understanding Creatures
aliases: [creatures, creatures-index]
---

# Understanding Creatures

Creatures are the heart of EcoSim - autonomous life forms that eat, breed, flee, and evolve. This guide explains what you'll observe as you watch them.

---

## What Are Creatures?

Each creature is an independent agent with:

- **Genes** that determine its traits (size, speed, diet, senses)
- **Needs** that drive behavior (hunger, thirst, fatigue, mating urge)
- **A Lifespan** from birth to eventual death
- **The Ability to Breed** and pass on traits to offspring

Creatures don't follow scripts - they respond to their environment moment-to-moment, creating emergent behavior patterns.

---

## Quick Reference

| Aspect | What to Watch For |
|--------|-------------------|
| **Behavior** | Profile states (hungry, thirsty, breeding, fleeing) |
| **Movement** | Speed, direction changes, pursuit and evasion |
| **Diet** | What they eat (plants, other creatures, or both) |
| **Reproduction** | Scent trails, mate-finding, offspring |
| **Death** | Starvation, predation, old age |

---

## Guides

1. **[Creature Behavior](behavior.md)** - Understanding behavior states and decisions
2. **[Creature Needs](needs.md)** - Hunger, thirst, fatigue, and mating urge
3. **[Creature Lifecycle](lifecycle.md)** - From birth to death

---

## Try This! 🔬

### Experiment: Track a Single Creature

1. Press **F5** to open the Inspector
2. Click on a creature to select it
3. Watch for 5 minutes and note:
   - How often does it change behavior?
   - Does it succeed in finding food?
   - Does it encounter other creatures?
   - How does its energy change over time?

This single-creature focus reveals the complexity of their decision-making!

### Experiment: Predator-Prey Dynamics

1. Find a carnivore (check diet in inspector)
2. Watch it hunt - does it succeed?
3. Find what it was chasing
4. Compare their speed genes

The outcome of each chase is determined by genetics!

---

## Visual Identification

### By Color

Creature color comes from the `color_hue` gene:

| Color Range | Hue Values |
|-------------|------------|
| 🔴 Red/Orange | 0° - 60° |
| 🟡 Yellow/Green | 60° - 180° |
| 🔵 Blue/Purple | 180° - 300° |
| 🟣 Pink/Red | 300° - 360° |

**Watch for color clustering!** When similar creatures breed together, their offspring tend to have similar colors. Over time, you might see distinct color "tribes" emerge.

### By Size

Size comes from the `max_size` gene. Larger creatures:
- Need more food to survive
- Are easier to spot
- May have different predator-prey dynamics

### By Movement

Watch how creatures move:
- **Fast, direct** = high locomotion, good navigation
- **Slow, wandering** = low locomotion or poor navigation
- **Sudden direction changes** = fleeing or pursuing

---

## The Inspector Window (F5)

When you select a creature, you'll see:

| Section | What It Shows |
|---------|---------------|
| **Basic Info** | Age, generation, position |
| **Current State** | Active behavior profile |
| **Energy** | Current energy level |
| **Needs Bars** | Hunger, thirst, fatigue, mate urge |
| **Physical** | Size, speed, sight range |
| **Diet** | What it can eat |
| **Genes** | Full genetic information |

---

## Common Questions

### Why is this creature just standing still?

It's probably **resting**. When fatigue gets high or all needs are satisfied, creatures rest to recover energy. Check the fatigue bar in the inspector.

### Why did two creatures meet but not breed?

Several possibilities:
- One or both weren't in breeding state
- Their genetic compatibility was too low
- One fled because it saw the other as a threat
- They didn't have enough energy

### Why do some creatures seem to wander aimlessly?

This is usually:
- **Low navigation gene** - they can't plan efficient paths
- **Low sight range** - they can't see food/mates until very close
- **Migration behavior** - exploring new territory

### Can I control creatures directly?

No! Creatures are autonomous. You can only:
- Add more creatures (press **A**)
- Modify the environment
- Watch and learn

This hands-off approach is what makes evolution emerge naturally.

---

## See Also

- 📖 **[Behavior States](behavior.md)** - Deep dive into creature decision-making
- 📖 **[Creature Needs](needs.md)** - Understanding what drives behavior
- 📖 **[Creature Lifecycle](lifecycle.md)** - Birth, life, and death
- 📖 **[Genetics: Gene Reference](../genetics/genes.md)** - All genes that affect creatures
