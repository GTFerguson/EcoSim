# Creature Needs

Every creature has four fundamental needs that drive their behavior. Understanding these needs helps you predict what creatures will do and why.

---

## The Four Needs

| Need | Icon | What It Means | Satisfied By |
|------|------|---------------|--------------|
| **Hunger** | 🍖 | Energy from food | Eating |
| **Thirst** | 💧 | Hydration | Drinking water |
| **Fatigue** | 😴 | Physical exhaustion | Resting |
| **Mate** | 💕 | Reproductive urge | Breeding |

---

## How Needs Work

### The Need Cycle

```
       ┌─────────────────────────────────────────┐
       │                                         │
       ▼                                         │
   Need GROWS ──► Reaches THRESHOLD ──► Behavior │
       │                 │                ACTIVE │
       │                 │                   │   │
       │                 ▼                   │   │
       │         Creature seeks              │   │
       │         to satisfy need             │   │
       │                 │                   │   │
       │                 ▼                   │   │
       │         Need SATISFIED ─────────────┘   │
       │              │                          │
       │              ▼                          │
       └────── Starts growing again ─────────────┘
```

### Needs vs. Thresholds

Each creature has **genes** that determine:

1. **How fast needs grow** (metabolism, activity level)
2. **When they respond** (threshold genes)

```
Example: Hunger

Creature A: Low hunger threshold
│████████░░│ ← Starts eating at 80%
           Eats frequently, small meals

Creature B: High hunger threshold  
│████░░░░░░│ ← Starts eating at 40%
           Eats rarely, big meals
```

---

## Hunger 🍖

Hunger represents the creature's need for food energy.

### How Hunger Increases

Hunger grows constantly based on:

| Factor | Effect |
|--------|--------|
| **Metabolism** | Higher = hungrier faster |
| **Movement** | Moving costs energy |
| **Size** | Larger creatures need more food |
| **Activity** | Chasing/fleeing burns energy |

### Hunger Threshold

When hunger rises above the threshold:
- Creature enters **hungry profile**
- Starts searching for food
- Other needs become secondary

### Satisfying Hunger

| Food Source | Energy Gained | Who Can Eat |
|-------------|---------------|-------------|
| Plants | Moderate | Herbivores, Omnivores |
| Prey | High | Carnivores, Omnivores |
| Carrion | Low-Moderate | Scavengers |

### What You'll See

- **Well-fed creature**: Active, breeding, exploring
- **Hungry creature**: Searching, focused on food
- **Starving creature**: Desperate, weak, may die soon

> ⚠️ **Starvation**: If hunger reaches 100% and stays there, the creature dies!

### Hunger-Related Genes

| Gene | Effect |
|------|--------|
| `hunger_threshold` | When creature starts seeking food |
| `metabolism` | How fast hunger increases |
| `digestive_efficiency` | How much energy extracted from food |
| `plant_digestion` | Efficiency eating plants |
| `meat_digestion` | Efficiency eating meat |

---

## Thirst 💧

Thirst represents the creature's hydration level.

### How Thirst Increases

Thirst grows based on:

| Factor | Effect |
|--------|--------|
| **Time** | Constant slow increase |
| **Activity** | High activity = more thirst |
| **Heat** | Hot environments increase thirst |
| **Metabolism** | Higher metabolism = more thirst |

### Thirst Threshold

When thirst rises above the threshold:
- Creature enters **thirsty profile**
- Searches for water sources
- May interrupt other activities

### Satisfying Thirst

Creatures must reach **water tiles** (blue tiles):
- Rivers
- Lakes
- Ponds
- Coastlines

### What You'll See

- **Hydrated creature**: Normal behavior
- **Thirsty creature**: Heading toward water
- **Dehydrated creature**: Sluggish, desperate

> ⚠️ **Dehydration**: Extreme thirst causes death!

### Thirst-Related Genes

| Gene | Effect |
|------|--------|
| `thirst_threshold` | When creature seeks water |
| `water_storage` | How long creature can go without water |
| `water_requirement` | How much water is needed |

---

## Fatigue 😴

Fatigue represents physical exhaustion that accumulates over time.

### How Fatigue Increases

Fatigue grows from:

| Factor | Effect |
|--------|--------|
| **Activity** | Moving, hunting, fleeing |
| **Time awake** | Gradual accumulation |
| **Energy level** | Low energy = faster fatigue |
| **Age** | Older creatures tire faster |

### Fatigue Threshold

When fatigue rises above the threshold:
- Creature enters **sleep profile**
- Stops moving to rest
- Recovers energy but is vulnerable

### Satisfying Fatigue

Rest is the only cure:
- Creature stops moving
- Fatigue slowly decreases
- Energy slowly recovers
- Must reach safe levels before resuming activity

### The Risk-Reward of Rest

```
┌─────────────────────────────────────────────────┐
│                   RESTING                        │
│                                                  │
│   ✅ Pros               ❌ Cons                  │
│   ────────────         ────────────              │
│   Recovers energy      Can't flee quickly        │
│   Reduces fatigue      Predators may spot you    │
│   Prepares for action  Missing food/mate ops     │
│                                                  │
└─────────────────────────────────────────────────┘
```

This trade-off creates selection pressure for:
- High fatigue tolerance (rest less)
- Fast recovery (short rest periods)
- Alert genes (wake up when threatened)

### What You'll See

- **Rested creature**: Active, fast responses
- **Tired creature**: Slower movement
- **Exhausted creature**: Stopping to rest, vulnerable

### Fatigue-Related Genes

| Gene | Effect |
|------|--------|
| `fatigue_threshold` | When creature must rest |
| `comfort_increase` | How fast creature recovers |
| `comfort_decrease` | How fast fatigue builds |

---

## Mating Urge 💕

The mating urge represents the drive to reproduce.

### How Mating Urge Increases

Mating urge grows based on:

| Factor | Effect |
|--------|--------|
| **Time** | Steady increase when well-fed |
| **Energy** | Higher energy = faster urge growth |
| **Age** | Mature creatures develop urge |
| **Recent mating** | Reset after successful breeding |

### Mate Threshold

When mating urge rises above the threshold:
- Creature enters **breed profile**
- Deposits scent trails
- Searches for compatible mates
- May travel long distances

### Satisfying Mating Urge

Successful breeding requires:
1. Finding a mate (via scent or sight)
2. Both creatures in breeding state
3. Sufficient energy in both
4. Genetic compatibility check
5. Actual mating encounter

### What You'll See

- **Low mating urge**: Focus on survival needs
- **High mating urge**: Following scent trails, seeking others
- **Breeding encounter**: Two creatures meeting briefly

### Mating-Related Genes

| Gene | Effect |
|------|--------|
| `mate_threshold` | When creature starts seeking mates |
| `offspring_count` | Babies per successful mating |
| `spread_distance` | How far offspring spawn |
| `scent_production` | Strength of pheromone trail |
| `olfactory_acuity` | Ability to detect mate scents |

---

## Need Priority

When multiple needs are high, creatures prioritize:

```
PRIORITY (highest first):
────────────────────────
1. 💀 CRITICAL DANGER
   (Flee from predators)
   
2. 💧 THIRST
   (Dehydration kills quickly)
   
3. 🍖 HUNGER  
   (Starvation is slower but deadly)
   
4. 😴 FATIGUE
   (Exhaustion is debilitating)
   
5. 💕 MATING
   (Important but not survival-critical)
   
6. 🧭 MIGRATION
   (Default when needs are satisfied)
```

### Competing Needs

Sometimes needs conflict:

**Hungry but tired?**
- If fatigue is critical → rest first
- If hunger is critical → eat first
- Borderline → depends on threshold genes

**Thirsty but being hunted?**
- Flee always wins over thirst
- After escaping, seek water

**Want to breed but hungry?**
- Usually: eat first
- Some creatures: breed anyway (risky strategy!)

---

## Watching Needs Evolve

Over generations, need-related genes evolve:

### In Resource-Rich Environments
- Higher thresholds (can be picky)
- More time for breeding
- Higher offspring counts

### In Resource-Scarce Environments
- Lower thresholds (respond earlier)
- More efficient metabolism
- Better foraging genes

### Under Predation Pressure
- Better flee responses
- Lower fatigue buildup
- Shorter rest periods

---

## Try This! 🔬

### Experiment: Need Comparison

1. Select a young creature (Inspector: F5)
2. Note its threshold genes
3. Watch which needs it responds to first
4. Compare with an old creature

Older creatures may have different strategies!

### Experiment: Starvation Watch

1. Find a creature in a food-poor area
2. Track its hunger over time
3. Does it migrate to find food?
4. Does it survive?

This shows natural selection in action!

### Experiment: The Thirsty Desert

1. Find creatures far from water
2. Watch their behavior patterns
3. Do they make regular water trips?
4. How does distance affect survival?

Water availability shapes evolution!

---

## Common Questions

### Why isn't this creature eating even though it's near food?

Check:
- Is hunger below threshold? (not hungry enough yet)
- Is it the right food type? (diet compatibility)
- Is another need more urgent?
- Is it fleeing from something?

### Why did this creature die when there was food nearby?

Possibilities:
- **Low sight range** - couldn't see the food
- **Poor navigation** - couldn't reach it
- **Wrong diet** - couldn't eat that food type
- **Too slow** - another creature got it first
- **Competing need** - thirst or fatigue was also critical

### Can creatures eat while fleeing?

No! Fleeing is the highest priority. A creature must escape danger before it can eat. This means predators indirectly cause starvation by preventing feeding.

---

## See Also

- 📖 **[Creature Behavior](behavior.md)** - How needs drive behavior
- 📖 **[Creature Lifecycle](lifecycle.md)** - How needs change with age
- 📖 **[Gene Reference](../genetics/genes.md)** - All need-related genes
- 📖 **[Watching Evolution](../genetics/evolution.md)** - How needs evolve
