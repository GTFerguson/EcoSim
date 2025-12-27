---
title: Understanding the World
aliases: [world, world-index]
---

# Understanding the World

The world of EcoSim is where all the action happens - a procedurally generated landscape where creatures and plants live, evolve, and interact.

---

## What Is the World?

The world is a 2D grid of **tiles**, each with:

- **Terrain type** (grassland, forest, water, etc.)
- **Elevation** (affecting conditions)
- **Contents** (plants, creatures, food)
- **Properties** (passable, water source, etc.)

---

## Quick Reference

| Element | What to Watch For |
|---------|-------------------|
| **Terrain** | Different colors = different types |
| **Elevation** | Affects water flow, temperature |
| **Plants** | Green areas rich in vegetation |
| **Water** | Blue tiles creatures need to drink |
| **Creatures** | Moving entities of various colors |

---

## Guides

1. **[Terrain and Biomes](terrain.md)** - Understanding the landscape
2. **[Scent and Communication](scent-communication.md)** - How creatures find each other

---

## World Generation

### How Worlds Are Made

Each world is **procedurally generated** using:

1. **Seed value** - A number that determines the exact layout
2. **Simplex noise** - Creates natural-looking terrain
3. **Elevation rules** - Assigns terrain types by height

Same seed = same world! Different seed = different world.

### Generation Parameters

| Parameter | What It Controls |
|-----------|-----------------|
| **Seed** | Unique world layout |
| **Scale** | Size of terrain features |
| **Frequency** | How often terrain varies |
| **Exponent** | Steepness of elevation changes |

---

## Terrain Types

| Terrain | Color | Passable | Water Source | Good For |
|---------|-------|----------|--------------|----------|
| **Grassland** | Green | Yes | No | Plants, herbivores |
| **Forest** | Dark green | Yes | No | Cover, some plants |
| **Water** | Blue | No | Yes | Drinking |
| **Mountain** | Gray/Brown | Varies | No | Barriers |
| **Desert** | Yellow/Tan | Yes | No | Heat-tolerant species |

---

## Try This! 🔬

### Experiment: New World Generation

1. Press **N** to generate new world
2. Notice how terrain changes
3. Try different parameters

### Experiment: Water Access

1. Find creatures far from water
2. Watch their behavior
3. Do they make water trips?

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

- 📖 **[Terrain Guide](terrain.md)** - Detailed terrain information
- 📖 **[Scent System](scent-communication.md)** - How scent works
- 📖 **[Usage Guide](../usage-guide.md)** - Complete controls reference
