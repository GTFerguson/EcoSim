---
title: Environmental Adaptation
aliases: [adaptation, environmental-stress, survival]
---

# Environmental Adaptation

Creatures and plants in EcoSim must adapt to their environment or face the consequences. This guide explains how organisms cope with different climates and what happens when they venture outside their comfort zone.

---

## The Comfort Zone

Every creature and plant has a **temperature tolerance range** - the temperatures they can comfortably survive in:

```
         Your Creature's Range
              ←───────→
   ┌──────────┬─────────┬──────────┐
   │   Too    │ Comfort │   Too    │
   │   Cold   │  Zone   │   Hot    │
   └──────────┴─────────┴──────────┘
```

- **Inside the range:** Normal function, no stress
- **Outside the range:** Environmental stress kicks in

---

## What Is Environmental Stress?

When a creature is outside its comfort zone, it experiences **stress**:

| Stress Level | Distance from Comfort | What Happens |
|--------------|----------------------|--------------|
| **None** | Within tolerance | Everything normal |
| **Mild** | 1-5°C outside | Uses extra energy |
| **Moderate** | 5-15°C outside | Extra energy + slow health loss |
| **Severe** | 15-25°C outside | High energy drain + health damage |
| **Lethal** | 25°C+ outside | Rapid death |

---

## Stress Effects

### Energy Drain

Stressed creatures burn energy faster:

| Stress Level | Energy Drain |
|--------------|--------------|
| Comfortable | Normal |
| Mild | +25% faster |
| Moderate | +50% faster |
| Severe | +100% faster |

**What this means:** A creature in a harsh environment will get hungry faster and need to eat more often!

### Health Damage

Severe stress causes direct damage:

| Stress Level | Health Effect |
|--------------|---------------|
| Comfortable | None |
| Mild | None |
| Moderate | Slow health loss |
| Severe | Faster health loss |
| Lethal | Very rapid death |

---

## Creature Adaptations

### Temperature Tolerance Genes

Creatures are born with genetic temperature tolerance:

| Gene | What It Controls |
|------|-----------------|
| **temp_tolerance_low** | How cold they can handle |
| **temp_tolerance_high** | How hot they can handle |

**Wide Range = Generalist**
- Can survive many biomes
- Not specialized for any

**Narrow Range = Specialist**
- Thrives in specific biomes
- Struggles elsewhere

### Environmental Sensitivity

The `environmental_sensitivity` gene controls how creatures **respond** to danger:

| Value | Behavior |
|-------|----------|
| **0.0** | Ignores danger, takes shortest paths |
| **1.0** | Balanced - avoids stress when convenient |
| **2.0** | Very cautious - strongly avoids stressful areas |

**High sensitivity creatures:**
- Take longer paths to avoid harsh biomes
- Survive better but travel slower

**Low sensitivity creatures:**
- Take direct routes regardless of danger
- May accidentally enter lethal zones

---

## Plant Adaptations

Plants face both temperature AND moisture stress:

### Moisture Requirements

| Adaptation | Moisture Need | Best Biomes |
|------------|---------------|-------------|
| **High water need** | 0.7-1.0 | Rainforests, wetlands |
| **Moderate need** | 0.3-0.6 | Temperate zones |
| **Low water need** | 0.1-0.3 | Grasslands, dry areas |

### Water Storage (Succulence)

Some plants can store water:

| Water Storage | Effect |
|---------------|--------|
| **High** | Survives dry periods (cacti-like) |
| **Low** | Needs consistent water |

A cactus (high storage, low need) thrives in deserts!
A fern (low storage, high need) needs rainforests!

---

## Adaptation Strategies

### For Survival

1. **Stay in your biome** - Don't wander into hostile areas
2. **Watch for stress** - Turn back if you're taking damage
3. **Find food near home** - Don't chase prey into lethal zones

### For Exploration

1. **Gradual adaptation** - Evolution can shift tolerance over generations
2. **Ecotone living** - Live on biome boundaries for flexibility
3. **Quick trips** - Can briefly enter harsh areas if necessary

### For Evolution

Over many generations, populations can adapt:

- **Cold-adapted branch:** Lower temperature tolerance evolves
- **Heat-adapted branch:** Higher temperature tolerance evolves
- **Generalist branch:** Wide tolerance range, lives everywhere

---

## Recognizing Stress in Creatures

### Visual Indicators

Watch for creatures that:
- Move erratically near biome boundaries
- Rapidly consume food (energy drain)
- Avoid certain areas (high sensitivity)
- Die near harsh biomes (low sensitivity)

### Inspector Panel

Use the creature inspector (F5) to see:
- Current temperature tolerance
- Current tile temperature
- Health status
- Energy levels

---

## Example Scenarios

### Scenario 1: Arctic Fox in Desert

**Setup:**
- Fox tolerance: -30°C to +15°C
- Desert temperature: 40°C

**What happens:**
1. Fox is 25°C outside tolerance (lethal!)
2. Energy drains at 125% rate
3. Takes rapid health damage
4. Dies within minutes if doesn't escape

**Survival strategy:** Don't go there!

### Scenario 2: Desert Lizard in Temperate Forest

**Setup:**
- Lizard tolerance: +15°C to +45°C
- Forest temperature: 12°C

**What happens:**
1. Lizard is 3°C outside tolerance (mild stress)
2. Energy drains 15% faster
3. No health damage
4. Can survive but works harder

**Survival strategy:** Stay near warmer clearings

### Scenario 3: Generalist Omnivore

**Setup:**
- Tolerance: -5°C to +35°C (wide range)
- Any moderate biome

**What happens:**
1. Comfortable in most biomes
2. Can travel freely through temperate zones
3. Only stressed in extreme deserts or arctic

**Survival strategy:** Avoid only the extremes

---

## Plant Survival Examples

### Cactus in Rainforest

| Factor | Value | Result |
|--------|-------|--------|
| Water need | 0.1 | Low |
| Water storage | 0.8 | High |
| Rainforest moisture | 0.95 | Very High |

**Result:** Survives but doesn't thrive (outcompeted)

### Fern in Desert

| Factor | Value | Result |
|--------|-------|--------|
| Water need | 0.8 | High |
| Water storage | 0.1 | Low |
| Desert moisture | 0.1 | Very Low |

**Result:** Dies from dehydration!

---

## Try This! 🔬

### Experiment: Watch Stress in Action

1. Find a creature near a harsh biome
2. Use the inspector to check its tolerance
3. Watch if it ventures in and what happens

### Experiment: Population Distribution

1. Zoom out to see the whole world
2. Notice where creatures cluster
3. Compare to biome types - do they match tolerances?

### Experiment: Evolutionary Drift

1. Let the simulation run for many generations
2. Check creature tolerances in different biomes
3. Have they specialized for their local climate?

---

## Tips for Playing

### Help Creatures Survive

- Generate worlds with diverse but connected biomes
- Mild climates allow more creatures to survive
- Extreme worlds create strong selection pressure

### Observe Evolution

- Isolated populations adapt to local conditions
- Connecting biomes allows gene flow
- Harsh climates eliminate unfit creatures quickly

---

## See Also

- 📖 **[Biomes Guide](biomes.md)** - Understanding the 22 biomes
- 📖 **[World Overview](README.md)** - Introduction to the world
- 📖 **[Creature Needs](../creatures/needs.md)** - What creatures require
- 📖 **[Genetics Overview](../genetics/README.md)** - How traits are inherited
