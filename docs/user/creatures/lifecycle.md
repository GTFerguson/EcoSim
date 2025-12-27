# Creature Lifecycle

From birth to death, each creature goes through a complete life journey. Understanding this lifecycle helps you appreciate the drama unfolding in your ecosystem.

---

## The Life Stages

```
    BIRTH ──► YOUTH ──► MATURITY ──► OLD AGE ──► DEATH
      │         │          │            │          │
   Spawn     Growing    Breeding      Slowing    End
   small     rapidly    actively      down      of life
```

---

## Birth

### How Creatures Are Born

Creatures enter the world in two ways:

**1. Spawning (Simulation Start)**
- Initial population created
- Random genetics with some variation
- Distributed across the world

**2. Breeding (During Simulation)**
- Two parent creatures mate
- Offspring inherits genes from both
- Born near parents with some spread

### What Newborns Look Like

| Trait | Newborn State |
|-------|---------------|
| **Size** | Smaller than adult size |
| **Energy** | Moderate starting energy |
| **Needs** | Low hunger/thirst initially |
| **Behavior** | Immediately autonomous |

> 💡 **Note**: There's no "baby" or "juvenile" phase where creatures are cared for. Newborns are immediately independent!

### The First Moments

A newborn creature immediately:
1. Assesses its environment
2. Checks its needs
3. Begins normal behavior

The genes it inherited determine:
- How fast it will move
- What it can eat
- How good its senses are
- Its survival chances

---

## Growth and Maturity

### Physical Growth

Creatures grow toward their genetic maximum size:

```
Birth        ─────►        Adult
│▪│                        │████│
Small                      Full size
```

**Size affects:**
- Energy requirements (bigger = needs more food)
- Visibility (easier to spot/be spotted)

### Reaching Breeding Age

At a certain age, creatures become **breeding-capable**:
- Mating urge begins accumulating
- Can enter breeding profile
- Can produce offspring

This happens relatively quickly - creatures don't have an extended "childhood."

### What You'll See

- Young creatures focusing on survival (eating, avoiding predators)
- Mature creatures adding breeding behavior
- Size gradually increasing over time

---

## Prime of Life

### The Active Period

During their prime, creatures are:
- **Most active**: High energy, fast movement
- **Most fertile**: Frequent breeding attempts
- **Most competitive**: Best at hunting/foraging

### Behavior Priorities

| Activity | Prime Creature |
|----------|----------------|
| Foraging | Efficient, covers ground |
| Hunting | Fast, successful |
| Fleeing | Quick escape response |
| Breeding | Active mate-seeking |

### This Is When Evolution Happens

Creatures in their prime:
- Have the most offspring
- Pass on their genes most effectively
- Face selection pressure intensely

**The creatures that survive to their prime and breed successfully shape the next generation!**

---

## Aging

### Signs of Aging

As creatures approach their lifespan limit:

| Aspect | Change |
|--------|--------|
| **Speed** | May slow slightly |
| **Energy** | Recovers more slowly |
| **Fatigue** | Builds faster |
| **Breeding** | Continues until death |

### The Lifespan Gene

Every creature has a `lifespan` gene that determines maximum age:

```
Short lifespan (100 ticks)     Long lifespan (1,000,000 ticks)
├──────────────────────────────────────────────────────────┤
│                                                          │
│  Fast generations         Slow generations               │
│  Quick evolution          Stable populations             │
│  Many opportunities       Accumulated experience         │
│                                                          │
```

### What You'll See

- Old creatures (high age in Inspector) still active
- Eventually, creatures simply disappear (death from old age)
- Population turnover as generations replace each other

---

## Death

### Causes of Death

Creatures die from several causes:

| Cause | How It Happens |
|-------|----------------|
| **Old Age** | Reached lifespan limit |
| **Starvation** | Hunger reached critical level |
| **Dehydration** | Thirst reached critical level |
| **Predation** | Eaten by a carnivore |

### What Happens at Death

1. Creature is removed from simulation
2. Statistics updated (death count)
3. Tile space freed for other creatures
4. (Carnivores may have eaten it for energy)

### Death as Selection

Death is the **mechanism of natural selection**:

```
Population with varied genes
            │
            ▼
    ┌───────────────┐
    │ ENVIRONMENT   │
    │ Challenges:   │
    │ - Find food   │
    │ - Avoid death │
    │ - Find mates  │
    └───────────────┘
            │
            ▼
   ┌────────┴────────┐
   │                 │
   ▼                 ▼
SURVIVORS        DEATHS
(Reproduce)      (Removed)
   │
   ▼
Next Generation
(Has survivor genes)
```

---

## Watching the Cycle

### Generation Counter

Each creature has a **generation number**:
- Generation 0: Initial spawned creatures
- Generation 1: Their offspring
- Generation 2: Grandchildren
- And so on...

Higher generation numbers mean:
- More time for evolution to act
- Potentially more adapted genes
- Descended from successful ancestors

### Population Turnover

Watch the statistics window (F1) for:
- **Birth rate**: New creatures per tick
- **Death rate**: Creatures dying per tick
- **Population**: Total alive

A healthy ecosystem shows:
- Roughly balanced births and deaths
- Gradual population fluctuations
- Generational advancement

### Evolutionary Timescales

| Timeframe | What Happens |
|-----------|--------------|
| Minutes | Individual lives and deaths |
| Hours | Population shifts, trait frequency changes |
| Days | Noticeable evolutionary adaptation |
| Extended | Major species changes, new strategies emerge |

---

## Try This! 🔬

### Experiment: Track a Bloodline

1. Select a creature and note its ID
2. Watch it breed and note offspring IDs
3. Track the offspring - do they survive?
4. Follow the family tree across generations

You're watching genetic legacy unfold!

### Experiment: Lifespan Comparison

1. Open Statistics (F1)
2. Note average lifespan
3. Run for many generations
4. Is average lifespan changing?

Evolution may favor longer or shorter lives!

### Experiment: Cause of Death Analysis

1. Watch the population over time
2. Try to observe deaths happening
3. Categorize: predation vs. starvation vs. old age
4. Which is most common?

This reveals the selection pressure in your ecosystem!

### Experiment: The Founder Effect

1. Start a new simulation
2. Note the traits of initial creatures
3. Let it run for 50+ generations
4. Compare final population to founders

How much has changed? What survived?

---

## The Circle of Life

### Every Death Enables Life

```
┌─────────────────────────────────────────────────────┐
│                                                     │
│   🌱 Plants photosynthesize                        │
│          │                                          │
│          ▼                                          │
│   🐰 Herbivores eat plants                         │
│          │                                          │
│          ▼                                          │
│   🦊 Carnivores eat herbivores                     │
│          │                                          │
│          ▼                                          │
│   💀 All creatures eventually die                  │
│          │                                          │
│          ▼                                          │
│   🌱 Space and resources freed for new life        │
│                                                     │
└─────────────────────────────────────────────────────┘
```

### Population Balance

A stable ecosystem has:
- Enough births to replace deaths
- Enough deaths to prevent overpopulation
- Resources cycling through the food chain
- Multiple generations coexisting

---

## Common Questions

### How long do creatures live?

It depends on their `lifespan` gene! Can range from:
- Short: ~100 ticks
- Long: ~1,000,000 ticks

Most creatures die from other causes before reaching maximum lifespan.

### Do creatures get "better" over generations?

"Better" is relative to the environment! Creatures evolve to be better adapted to their specific challenges. What's "better" in one environment might be useless in another.

### Why do some creatures live longer than others?

Several factors:
- **Lifespan gene** sets maximum
- **Luck** in avoiding predators
- **Food availability** in their area
- **Genetics** for survival (speed, senses)

### Can I see a creature's family tree?

Not directly in the UI, but you can:
- Note parent/offspring pairs manually
- Track generation numbers
- Watch breeding events closely

---

## See Also

- 📖 **[Creature Behavior](behavior.md)** - What creatures do during their lives
- 📖 **[Creature Needs](needs.md)** - What keeps them alive
- 📖 **[Evolution in Action](../genetics/evolution.md)** - How generations change over time
- 📖 **[Inheritance](../genetics/inheritance.md)** - How genes pass to offspring
