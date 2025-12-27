# Seed Dispersal

One of the most fascinating aspects of EcoSim is watching how plants spread their seeds across the world. Each dispersal method creates unique patterns in your ecosystem.

---

## Six Ways Seeds Travel

Plants have evolved diverse strategies to spread their offspring:

| Method | Icon | Key Genes |
|--------|------|-----------|
| **Gravity** | 🪨 | High seed mass |
| **Wind** | 💨 | Low mass + high aerodynamics |
| **Animal (Fruit)** | 🍒 | High fruit appeal + durable coat |
| **Animal (Burr)** | 🦔 | High hook strength |
| **Explosive** | 💥 | High explosive pod force |
| **Vegetative** | 🌿 | High runner production |

---

## Gravity Dispersal 🪨

The simplest method - seeds fall and land near the parent plant.

### How It Works
1. Heavy seeds develop
2. Seeds drop when mature
3. Land close to parent
4. Germinate nearby

### Characteristics

| Aspect | Effect |
|--------|--------|
| **Distance** | Very short (0-2 tiles) |
| **Reliability** | Very high |
| **Energy cost** | Low |
| **Competition** | High (near parent) |

### What You'll See
- Dense clusters of plants
- Parent surrounded by offspring
- "Family groups" of similar plants

---

## Wind Dispersal 💨

Light seeds with fluffy or winged structures that catch the breeze.

### How It Works
1. Very light seeds develop
2. Seeds have aerodynamic features
3. Wind carries seeds away
4. Land at random distances

### Key Genes

| Gene | Effect |
|------|--------|
| `seed_mass` | Lower = travels further |
| `seed_aerodynamics` | Higher = catches wind better |

### Characteristics

| Aspect | Effect |
|--------|--------|
| **Distance** | Long (10-50+ tiles) |
| **Reliability** | Variable (depends on wind) |
| **Energy cost** | Moderate |
| **Competition** | Low (spread out) |

### What You'll See
- Plants appearing in unexpected locations
- Random distribution patterns
- Colonization of distant areas

---

## Animal Fruit Dispersal 🍒

Plants produce attractive fruit that animals eat, then deposit seeds elsewhere.

### How It Works
1. Plant produces appealing fruit
2. Animal eats fruit (including seeds)
3. Animal moves to new location
4. Seeds pass through digestive tract
5. Seeds deposited with "fertilizer"!

### Key Genes

| Gene | Effect |
|------|--------|
| `fruit_appeal` | Higher = more attractive to animals |
| `seed_coat_durability` | Higher = survives digestion |
| `fruit_production_rate` | How often fruit is made |

### Creature Genes That Matter

| Gene | Effect |
|------|--------|
| `seed_destruction_rate` | Lower = more seeds survive |
| `gut_transit_time` | Longer = seeds travel further |
| `color_vision` | Better = finds colorful fruit easier |

### What You'll See
- Animals clustering around fruiting plants
- Seeds appearing along animal travel routes
- Mutualistic relationships developing

### The Partnership

```
🌿 Plant makes fruit  ◄──── Both benefit! ────► 🐰 Animal gets food
         │                                              │
         ▼                                              ▼
   Seeds spread far                            Nutrition gained
```

---

## Animal Burr Dispersal 🦔

Plants develop hooks or sticky seeds that attach to passing animals.

### How It Works
1. Seeds develop hooks or sticky coating
2. Animal brushes against plant
3. Seeds attach to fur/hide
4. Animal travels
5. Seeds eventually detach at new location

### Key Genes

| Gene | Effect |
|------|--------|
| `seed_hook_strength` | Higher = attaches better |

### Creature Genes That Matter

| Gene | Effect |
|------|--------|
| `fur_density` | Higher = more burrs attach |
| `grooming_frequency` | Higher = burrs detach sooner |

### What You'll See
- Plants near animal pathways
- Spread following movement patterns
- Creatures "carrying" seeds

### The Hitchhiker Strategy
- No energy spent on fruit
- Relies on animal movement
- Works best in high-traffic areas

---

## Explosive Dispersal 💥

Seed pods that burst open, launching seeds away from the parent.

### How It Works
1. Seeds develop in pressurized pods
2. Pod reaches critical tension
3. Pod bursts open explosively
4. Seeds launch in all directions

### Key Genes

| Gene | Effect |
|------|--------|
| `explosive_pod_force` | Higher = seeds fly further |

### Characteristics

| Aspect | Effect |
|--------|--------|
| **Distance** | Medium (5-15 tiles) |
| **Reliability** | Very high |
| **Energy cost** | High |
| **Direction** | Random (all directions) |

### What You'll See
- Ring patterns around parent plants
- Even distribution in all directions
- Rapid colonization of nearby area

---

## Vegetative (Runner) Dispersal 🌿

Plants spread by sending out ground-level stems that root and create clones.

### How It Works
1. Plant sends out horizontal runners
2. Runners grow along ground
3. New roots form at intervals
4. Clone plants establish
5. Process repeats

### Key Genes

| Gene | Effect |
|------|--------|
| `runner_production` | Higher = more runners |

### Characteristics

| Aspect | Effect |
|--------|--------|
| **Distance** | Short but continuous |
| **Reliability** | Very high |
| **Energy cost** | Moderate |
| **Genetics** | Clones (identical genes) |

### What You'll See
- Connected plant networks
- Carpet-like spread patterns
- Rapid local domination

### Important Note

Runner-produced plants are **clones** - they have identical genes to the parent. This means:
- No genetic variation
- Same strengths and weaknesses
- Vulnerable to same threats

---

## Dispersal Strategies Compared

| Strategy | Distance | Reliability | Energy | Genetic Diversity |
|----------|----------|-------------|--------|-------------------|
| Gravity | Very short | High | Low | Normal |
| Wind | Long | Variable | Moderate | Normal |
| Fruit | Long | Moderate | High | Normal |
| Burr | Moderate | Variable | Low | Normal |
| Explosive | Medium | High | High | Normal |
| Vegetative | Short | Very high | Moderate | None (clones) |

---

## Try This! 🔬

### Experiment: Track a Seed Journey

1. Find a fruiting plant
2. Watch an animal eat from it
3. Follow the animal
4. See where seeds get deposited

### Experiment: Wind Patterns

1. Find wind-dispersed plants
2. Map where offspring appear
3. Can you determine wind direction?

### Experiment: Burr Mapping

1. Track a furry creature
2. Note where it brushes against plants
3. Note where burrs detach
4. Map the dispersal pattern

---

## Common Questions

### Why don't seeds travel further?

Possible reasons:
- Low aerodynamics gene
- High seed mass
- No animal dispersers nearby
- Explosive force too weak

### Why are there no plants in this area?

- Seeds can't reach there
- Environment unsuitable
- All seeds eaten/destroyed
- Too far from parent plants

### Can a plant use multiple dispersal methods?

In theory yes - genes can combine. But usually one strategy dominates based on gene values.

---

## See Also

- 📖 **[Plant Lifecycle](lifecycle.md)** - Full plant life cycle
- 📖 **[Gene Reference](../genetics/genes.md)** - Seed dispersal genes
- 📖 **[Coevolution](../genetics/evolution.md)** - Plant-animal evolution
