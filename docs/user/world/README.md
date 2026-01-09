---
title: Understanding the World
aliases: [world, world-index]
---

# Understanding the World

The world of EcoSim is where all the action happens - a procedurally generated landscape with realistic climate, 22 distinct biomes, and dynamic environmental conditions that affect every creature and plant.

---

## What Is the World?

The world is a 2D grid of **tiles**, each with:

- **Biome type** (rainforest, desert, tundra, etc.)
- **Climate** (temperature, moisture)
- **Elevation** (affecting conditions)
- **Contents** (plants, creatures, corpses)
- **Properties** (passable, water source, etc.)

---

## Quick Reference

| Element | What to Watch For |
|---------|-------------------|
| **Biomes** | Different colors = different climates |
| **Temperature** | Polar (cold) to tropical (hot) |
| **Moisture** | Desert (dry) to rainforest (wet) |
| **Plants** | Dense in favorable biomes |
| **Water** | Blue tiles creatures need to drink |
| **Creatures** | Colored by species traits |

---

## Guides

1. **[Biomes](biomes.md)** - The 22 world biomes explained
2. **[Environmental Adaptation](environmental-adaptation.md)** - How creatures survive different climates
3. **[Terrain and Biomes](terrain.md)** - Understanding the landscape
4. **[Scent and Communication](scent-communication.md)** - How creatures find each other

---

## Climate-Based World Generation

### How Worlds Are Made

Each world is **procedurally generated** using climate simulation:

1. **Elevation** - Mountains and valleys create terrain
2. **Temperature** - Latitude and elevation affect heat
3. **Moisture** - Rainfall patterns across the landscape
4. **Biome Assignment** - Climate determines the biome type

Same seed = same world! Different seed = different world.

### The Whittaker Diagram

EcoSim uses a real ecological model called the **Whittaker diagram** to determine biomes. This looks at two factors:

- **Temperature** (cold ↔ hot)
- **Moisture** (dry ↔ wet)

```
               ← Moisture →
              Dry        Wet
         ┌─────────────────────┐
    Hot  │ Desert  │ Rainforest│
         ├─────────┼───────────┤
   Temp  │ Savanna │ Monsoon   │
         ├─────────┼───────────┤
    Cold │ Tundra  │ Boreal    │
         └─────────────────────┘
```

---

## Biome Categories

EcoSim has **22 different biomes** in 6 categories:

### 🌊 Aquatic (1)
Deep water bodies - uninhabitable for land creatures

### ❄️ Cold (5)
Polar regions with snow, ice, and hardy vegetation
- Ice Cap, Tundra, Boreal Forest, Cold Desert, Glacier

### 🌲 Temperate (4)
Mild climates with seasonal changes
- Temperate Rainforest, Temperate Deciduous, Temperate Grassland, Temperate Desert

### 🌴 Warm (4)
Hot climates with abundant life
- Tropical Rainforest, Tropical Seasonal Forest, Monsoon Forest, Swamp

### 🏜️ Dry (4)
Arid regions with sparse vegetation
- Desert, Savanna, Shrubland, Semi-Arid

### 🏔️ Alpine (4)
Mountain regions with elevation-based conditions
- Alpine Tundra, Alpine Meadow, Montane Forest, Highland

---

## Environmental Effects

### Temperature Stress

Creatures and plants have **temperature tolerance** - a range they can survive in:

| Effect | What Happens |
|--------|--------------|
| **Comfortable** | Normal function |
| **Stressed** | Uses more energy |
| **Severely Stressed** | Takes damage |
| **Lethal** | Dies quickly |

A creature adapted to cold forests will struggle in a hot desert!

### Moisture Stress (Plants)

Plants need water to survive:

| Adaptation | Biome Fit |
|------------|-----------|
| **High water needs** | Rainforests, wetlands |
| **Low water needs** | Deserts, dry regions |
| **Water storage** | Can survive dry periods |

---

## Generation Parameters

| Parameter | What It Controls |
|-----------|-----------------|
| **Seed** | Unique world layout |
| **Temperature bias** | Overall climate warmth |
| **Moisture bias** | Overall wetness |
| **Elevation** | Mountain/valley frequency |

---

## Biome Recognition

| Biome | Visual Cues | Temperature | Moisture |
|-------|-------------|-------------|----------|
| **Tropical Rainforest** | Dense green | Hot | Very wet |
| **Desert** | Yellow/tan | Hot | Very dry |
| **Tundra** | Light gray | Cold | Dry |
| **Temperate Forest** | Mixed green | Mild | Moderate |
| **Savanna** | Yellow-green | Warm | Seasonal |
| **Boreal Forest** | Dark green | Cold | Moderate |

---

## Try This! 🔬

### Experiment: Biome Diversity

1. Generate a new world
2. Look for different biome colors
3. Notice how they transition at boundaries (ecotones)

### Experiment: Temperature Adaptation

1. Find a creature in a cold biome
2. Track it if it moves toward warmer areas
3. Watch for stress indicators

### Experiment: Desert Survival

1. Find a desert region
2. Watch which plants grow there
3. Notice how creatures behave near desert edges

---

## Navigation

### Camera Controls

| Key | Action |
|-----|--------|
| Arrow keys / HJKL | Pan camera |
| +/- | Zoom in/out |
| Mouse wheel | Zoom |

### UI Windows

| Key | Window |
|-----|--------|
| F1 | Statistics |
| F2 | World Info |
| F3 | Performance |
| F4 | Creature List |
| F5 | Inspector |
| F6 | Controls Help |

---

## See Also

- 📖 **[Biomes Guide](biomes.md)** - Complete biome reference
- 📖 **[Environmental Adaptation](environmental-adaptation.md)** - How creatures cope
- 📖 **[Terrain Guide](terrain.md)** - Detailed terrain information
- 📖 **[Scent System](scent-communication.md)** - How scent works
- 📖 **[Usage Guide](../usage-guide.md)** - Complete controls reference
