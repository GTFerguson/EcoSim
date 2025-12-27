# Getting Started with EcoSim

Welcome to EcoSim, an evolutionary simulation where creatures with genetic traits compete, reproduce, and evolve over time.

---

## Quick Start

### 1. Build the Application

```bash
# For graphical mode (recommended)
./comp_sdl.sh

# For terminal mode
./comp.sh
```

### 2. Run the Simulation

```bash
# Run the graphical version
./SimDiagnostic

# Or run from the build output
./build/ecosim
```

### 3. Watch Evolution Happen

The simulation will start with a world populated by:
- **200 initial creatures** with diverse genetics
- **Plants** distributed across the terrain
- **Water sources** scattered throughout

---

## Your First 5 Minutes

### Observe the Initial Population

When the simulation starts, you'll see creatures moving around the world. They're automatically:

1. **Finding food** - Creatures seek plants or prey based on their diet
2. **Finding mates** - Breeding-ready creatures emit scent trails and follow others
3. **Fleeing predators** - Prey animals run when they detect danger
4. **Resting** - Tired creatures stop to recover energy

### Open the Statistics Window

Press **F1** to see population statistics:
- How many creatures are alive
- Birth and death counts
- Population trends over time

### Inspect a Creature

1. Press **F5** to open the Inspector
2. Click on any creature in the world
3. See its genetic makeup, energy levels, and current behavior

---

## Understanding What You See

### Creature Colors

Each creature's color comes from its `color_hue` gene:
- **Reds/Oranges**: Hue 0-60
- **Yellows/Greens**: Hue 60-180
- **Blues/Purples**: Hue 180-300
- **Pinks/Reds**: Hue 300-360

Over time, you may see color clustering as similar creatures breed together!

### Creature Size

Larger creatures have higher `max_size` genes. Size affects:
- Energy requirements (bigger = more food needed)
- Visibility (easier to spot prey/predators)
- Mating preference (some prefer similar sizes)

### Creature Behavior States

Watch for behavior patterns:
- **Wandering**: Looking for food
- **Pursuing**: Chasing food or mate
- **Fleeing**: Running from danger
- **Resting**: Recovering energy

---

## Key Concepts

### Genetics

Every creature has a **genome** with 88 genes controlling:
- Physical traits (size, speed, color)
- Digestion (what they can eat)
- Senses (sight range, scent detection)
- Behavior (aggression, exploration)
- Reproduction (offspring count, mating preferences)
- Combat (weapons, defense, tactics)

Genes are inherited from parents with small mutations, driving evolution.

### Emergent Diet Types

Creatures don't have a fixed "diet type" - their diet **emerges** from digestion genes:

| If creature has... | They become... |
|-------------------|----------------|
| High plant digestion, low meat | **Herbivore** |
| High meat digestion, low plant | **Carnivore** |
| Balanced digestion | **Omnivore** |
| High color vision, fruit preference | **Frugivore** |

### Scent-Based Mating

Creatures find mates using pheromones:
1. Breeding-ready creatures deposit **scent trails**
2. Other creatures **detect scents** based on their olfactory genes
3. They follow the **scent gradient** toward potential mates
4. **Genetic signatures** help them identify compatible partners

This allows mating even when creatures can't see each other!

---

## Common Questions

### Why are creatures dying?

Creatures die from:
- **Starvation**: Couldn't find enough food
- **Old age**: Reached their lifespan limit
- **Predation**: Eaten by a carnivore
- **Thirst**: No water access (rare)

### Why is the population declining?

If population is dropping:
- Not enough food sources (plants may need time to regrow)
- Predator-prey imbalance
- Creatures not finding mates (rare with scent system)

Try pressing **A** to add 100 more creatures.

### Why do some creatures look the same?

Genetic clustering! When similar creatures breed:
- Their offspring share traits
- They prefer genetically similar mates
- This leads to **subspeciation** over time

### How do I speed up evolution?

Evolution needs:
- **Selection pressure**: Limited resources, predation
- **Reproduction**: Successful creatures breeding
- **Time**: Run for thousands of ticks

You can add creatures (A) to increase population diversity.

---

## Next Steps

### Learn More About Genetics

- [Inheritance](genetics/inheritance.md) - How genes pass to offspring
- [Evolution](genetics/evolution.md) - How populations change over time

### Understand the Systems

- [Usage Guide](usage-guide.md) - Complete controls and features
- [Scent System](../technical/genetics/scent-system.md) - How pheromones work

### Explore the Code

- `include/genetics/defaults/UniversalGenes.hpp` - All gene definitions
- `src/objects/creature/creature.cpp` - Creature behavior
- `include/world/ScentLayer.hpp` - Scent system implementation

See the **[Usage Guide](usage-guide.md)** for complete keyboard controls and UI reference.

---

Enjoy watching evolution unfold! 🧬
