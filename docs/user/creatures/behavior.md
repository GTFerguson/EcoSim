# Creature Behavior

Creatures make decisions every moment based on their current needs and environment. Understanding these behavior patterns helps you predict what they'll do next.

---

## Motivations and Actions

Creature behavior is controlled by two systems working together:
- **Motivations** - What the creature *needs* (its internal drives)
- **Actions** - What the creature is *doing* (its current activity)

### Motivations (Drives)

At any moment, a creature has one primary motivation driving its behavior:

| Motivation | Icon | Description |
|------------|------|-------------|
| **Hungry** | 🍖 | Needs food, will search for plants or prey |
| **Thirsty** | 💧 | Needs water, will seek water sources |
| **Amorous** | 💕 | Ready to mate, seeking a compatible partner |
| **Tired** | 💤 | Needs rest to recover energy |
| **Content** | 😊 | All needs satisfied, will wander or explore |

### Actions (Activities)

The current action shows what the creature is actively doing:

| Action | Description |
|--------|-------------|
| **Idle** | Standing still, no specific goal |
| **Wandering** | Moving randomly, exploring |
| **Searching** | Looking for something (food/water/mate) |
| **Navigating** | Moving toward a known target |
| **Eating** | Consuming food (general) |
| **Grazing** | Eating plants (herbivore) |
| **Hunting** | Pursuing prey (predator) |
| **Chasing** | Actively running after target |
| **Attacking** | Engaged in combat |
| **Fleeing** | Running away from threat |
| **Drinking** | Consuming water |
| **Courting** | Looking for mate via scent |
| **Mating** | Breeding with partner |
| **Resting** | Sleeping to recover |

> 💡 **Tip:** Press **F5** to open the Inspector and see a creature's current motivation and action!

---

## How Creatures Choose Their Behavior

Every tick, creatures evaluate their needs and pick the most urgent one:

```
┌─────────────────────────────────────────────────────┐
│                   DECISION CYCLE                     │
│                                                      │
│   Check Hunger ──► Above threshold? ──► HUNGRY      │
│        │                                             │
│        ▼                                             │
│   Check Thirst ──► Above threshold? ──► THIRSTY     │
│        │                                             │
│        ▼                                             │
│   Check Fatigue ─► Above threshold? ──► SLEEP       │
│        │                                             │
│        ▼                                             │
│   Check Mate ────► Above threshold? ──► BREED       │
│        │                                             │
│        ▼                                             │
│   Default ───────────────────────────► MIGRATE      │
│                                                      │
└─────────────────────────────────────────────────────┘
```

The **thresholds** are determined by genes! This means:
- Some creatures get "hungry" earlier than others
- Some can ignore thirst longer
- Some breed more frequently

---

## Hungry Profile 🍖

When hungry, creatures search for food based on their diet:

### Herbivores
- Search for plants within sight range
- Navigate toward nearest edible plant
- Consume plants to gain energy

### Carnivores
- Search for prey within sight range
- Pursue prey (initiating a chase!)
- Hunt and consume prey for high energy

### Omnivores
- Can eat both plants and prey
- Opportunistic - takes whatever is closest
- Flexible survival strategy

### What You'll See

| Behavior | Meaning |
|----------|---------|
| Direct movement | Found food, heading toward it |
| Spiral pattern | Searching for food |
| Sudden acceleration | Prey fleeing OR predator pursuing |
| Stopping near plant | Eating |

### The Hunt

When a carnivore spots prey:

```
PREDATOR                           PREY
    │                               │
    │ ──── Spots prey ────────────► │
    │                               │
    │ ◄──────────────── Detects danger
    │                               │
    │ ◄═══ CHASE BEGINS ══════════► │
    │      (speed vs speed)         │
    │                               │
    ▼                               ▼
 Success: Feeds             Escape: Survives
 Failure: Hungry still      Caught: Death
```

**Who wins the chase?** It depends on:
- **Speed** (locomotion gene)
- **Flee distance** (how early prey detects danger)
- **Pursue distance** (how far predator will chase)
- **Navigation** (pathfinding ability)

### Finding Food: Two Senses 👀 👃

Creatures use **two senses** to locate food:

| Sense | Gene | Best For | Limitation |
|-------|------|----------|------------|
| 👀 **Sight** | Color Vision | Colorful plants (berries, fruit) | Blocked by obstacles |
| 👃 **Smell** | Scent Detection | Fragrant plants | Works through obstacles! |

#### Color Vision (Sight)

Creatures with high **Color Vision** excel at spotting brightly-colored plants:
- Berry bushes stand out with their vivid colors
- Fruit-bearing plants catch the eye from afar
- Works best in open terrain with clear line-of-sight

#### Scent Detection (Smell)

Creatures with high **Scent Detection** can smell fragrant plants:
- Sweet-smelling plants like berry bushes are easy to find
- Scent travels around obstacles—through forests, over hills
- Great for finding hidden food sources

> [!TIP]
> Watch for creatures heading toward something they can't see—they're following their nose!

#### Which Plants Smell?

Not all plants are fragrant! Here's what you'll observe:

| Plant Type | Scent Strength | Color | Found By |
|------------|----------------|-------|----------|
| 🫐 Berry Bushes | Strong, sweet | Bright | Both senses! |
| 🌵 Thorn Bushes | Moderate | Moderate | Either sense |
| 🌳 Oak Trees | Faint | Green | Mostly sight |
| 🌿 Grass | None | Green | Sight only |

This creates different foraging strategies! Some creatures evolve to "see" their food, others to "smell" it.

---

## Thirsty Profile 💧

When thirsty, creatures search for water:

1. Scan for water tiles within sight range
2. Navigate toward nearest water
3. Drink to restore hydration

> 🌊 **Water tiles** are blue tiles in the world, usually near low elevations.

### Drinking from Shore

Creatures don't need to stand IN the water to drink—they can drink when **adjacent** to water, just like real animals approaching a river bank or lake shore!

| Position | Can Drink? |
|----------|------------|
| Standing on water tile | ✅ Yes |
| Adjacent to water (on shore) | ✅ Yes |
| More than 1 tile away | ❌ No |

This means you'll see creatures approach the water's edge, pause briefly to drink, and then continue on their way without having to wade into the water.

### What You'll See
- Creatures heading toward blue areas
- Brief pause at water's edge (drinking)
- Return to other activities afterward

---

## Breed Profile 💕

The breeding system is one of EcoSim's most complex behaviors!

### Finding a Mate

Breeding-ready creatures use **scent trails**:

```
                    SCENT TRAIL
     💕 ─────────────────────────────────
         │ │ │ │ │ │ │ │ │ │ │ │
         stronger ─────────► weaker

Other creature detects scent and follows gradient toward source
```

See **[[scent-communication.md|Scent and Communication]]** for the full scent system!

### Mating Requirements

Both creatures must:
- ✅ Be in breeding profile
- ✅ Have enough energy
- ✅ Be genetically compatible (but not too similar)
- ✅ Meet in the same location

### Genetic Compatibility

Creatures prefer mates that are:
- **Similar enough** to be compatible
- **Different enough** for healthy offspring

This creates a "Goldilocks zone" of mate preference!

### What You'll See

| Behavior | What's Happening |
|----------|------------------|
| Wandering in breeding state | Depositing scent, looking for scent |
| Sudden direction change | Detected a scent trail |
| Following another creature | Found a potential mate |
| Brief pause with another | Mating occurring |
| New small creature appears | Offspring born! |

---

## Sleep Profile 💤

When fatigue gets too high, creatures must rest:

### What Happens During Rest

- Creature stops moving
- Energy slowly recovers
- Fatigue decreases
- Vulnerable to predators!

### The Risk of Resting

Resting creatures can't flee quickly. Predators may:
- Spot resting prey more easily
- Catch them before they can escape

This creates selection pressure for:
- High fatigue tolerance (rest less often)
- Quick recovery (short rest periods)
- Safe resting spots (near allies, away from predators)

### What You'll See
- Creature stops moving
- No profile changes for several ticks
- Gradual energy bar recovery

---

## Migrate Profile 🧭

When no urgent needs exist, creatures explore:

### Migration Behavior
- Move in generally consistent direction
- Occasional random direction changes
- Explores new territory
- May find new food sources or mates

### Why Migration Matters

Migration helps creatures:
- Escape overpopulated areas
- Find unexploited resources
- Spread genes across the world
- Avoid local predators

---

## Watching Behavior Evolution

Over generations, you may notice behavior changes:

### Herbivore Adaptations
- **Sight range** increases (spot plants earlier)
- **Flee distance** increases (escape predators sooner)
- **Navigation** improves (efficient food-finding)

### Carnivore Adaptations
- **Pursue distance** increases (longer chases)
- **Speed** increases (catch prey)
- **Hunger threshold** optimizes (hunt at right times)

### General Adaptations
- **Mate threshold** shifts based on food availability
- **Fatigue tolerance** increases (less vulnerable resting)
- **Metabolism** optimizes for environment

---

## Try This! 🔬

### Experiment: Behavior Frequency

1. Select a creature with Inspector (F5)
2. Note its starting profile
3. Count how many times each profile appears over 5 minutes
4. Compare with another creature of different generation

You might see evolved creatures spending more time in productive profiles!

### Experiment: Predator Stalking

1. Find a carnivore
2. Watch its hunting attempts
3. Track: Attempts vs. Successes
4. Compare with the genes of prey it catches vs. prey that escapes

This reveals the selection pressure driving evolution!

### Experiment: Breeding Patterns

1. Watch a high-population area
2. Note breeding pairs that form
3. Use Inspector to compare their colors
4. Are similar colors breeding together?

This shows **assortative mating** in action!

---

## Common Questions

### Why does this creature keep changing direction?

Possible reasons:
- **Low navigation gene** - poor pathfinding
- **Multiple targets** - switching between detected food/mates
- **Conflicting needs** - thirst and hunger both high
- **Fleeing** - detecting threats from multiple directions

### Why won't two creatures in breeding state mate?

Check:
- Are they close enough? (must be on same/adjacent tile)
- Do they have enough energy?
- Is genetic compatibility in the "good" range?
- Is one fleeing from the other? (predator/prey)

### Why do some creatures never seem to rest?

They might have:
- **High fatigue threshold** - can go longer without rest
- **Low metabolism** - doesn't tire quickly
- **Constantly finding food** - replenishing energy

---

## See Also

- 📖 **[Creature Needs](needs.md)** - What drives each behavior
- 📖 **[Creature Lifecycle](lifecycle.md)** - How behavior changes with age
- 📖 **[Scent and Communication](../world/scent-communication.md)** - How creatures find each other
- 📖 **[Watching Evolution](../genetics/evolution.md)** - How behavior evolves over time
