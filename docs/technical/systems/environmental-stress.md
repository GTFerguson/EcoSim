---
title: Environmental Stress System
created: 2026-01-09
updated: 2026-01-10
status: complete
audience: developer
type: reference
tags: [environment, stress, fitness, temperature, moisture, organisms]
---

# Environmental Stress System

**Audience:** Developer
**Type:** Reference

---

## Overview

The Environmental Stress System calculates fitness penalties for organisms operating outside their genetic tolerance ranges. Rather than instant death at biome boundaries, organisms experience **progressive fitness gradients** that:

- Drain energy faster (increased metabolism)
- Reduce health over time (environmental damage)
- Reduce efficiency (slower movement, reduced photosynthesis)

This creates natural selection pressure without mass extinctions at biome boundaries.

**Header:** [`include/genetics/expression/EnvironmentalStress.hpp`](../../../include/genetics/expression/EnvironmentalStress.hpp:1)

---

## Core Concepts

### Fitness Gradients

Organisms don't die instantly outside their tolerance range. Instead, stress increases progressively:

```
         Tolerance Range
              ←───────→
   ┌──────────┬─────────┬──────────┐
   │  LETHAL  │ COMFORT │  LETHAL  │
   │          │         │          │
   └──────────┴─────────┴──────────┘
   ↑          ↑         ↑          ↑
   │          │         │          │
 Severe    Mild      Mild      Severe
 Stress   Stress   Stress     Stress
```

### Stress Zones

| Zone | Distance from Tolerance | Effects |
|------|------------------------|---------|
| **Comfort** | Within tolerance | No stress, normal function |
| **Mild** | 0-5°C outside | Minor energy drain, no health damage |
| **Moderate** | 5-15°C outside | Significant energy drain, slow health loss |
| **Severe** | 15-25°C outside | Heavy energy drain, rapid health loss |
| **Lethal** | 25°C+ outside | Rapid death |

---

## Temperature Stress

### Data Structure

```cpp
namespace EcoSim {

struct TemperatureStress {
    float stressLevel;      // 0.0 = comfortable, 1.0+ = severe stress
    float energyDrainRate;  // Additional energy cost per tick
    float healthDamageRate; // Health loss per tick
    bool lethal;            // Instant death threshold exceeded
};

} // namespace EcoSim
```

### Calculation

```cpp
TemperatureStress calculateTemperatureStress(
    float currentTemp,
    float toleranceLow,
    float toleranceHigh
) {
    TemperatureStress result = {0.0f, 0.0f, 0.0f, false};
    
    // Calculate distance from tolerance range
    float distanceFromTolerance = 0.0f;
    
    if (currentTemp < toleranceLow) {
        distanceFromTolerance = toleranceLow - currentTemp;
    } else if (currentTemp > toleranceHigh) {
        distanceFromTolerance = currentTemp - toleranceHigh;
    }
    
    if (distanceFromTolerance <= 0.0f) {
        return result;  // Within tolerance, no stress
    }
    
    // Stress level: normalized 0-1+
    result.stressLevel = distanceFromTolerance / 25.0f;
    
    // Energy drain: Linear scaling (50% extra at 10°C outside)
    result.energyDrainRate = distanceFromTolerance * 0.05f;
    
    // Health damage: Only starts at moderate stress (>5°C outside)
    if (distanceFromTolerance > 5.0f) {
        result.healthDamageRate = (distanceFromTolerance - 5.0f) * 0.025f;
    }
    
    // Lethal threshold (>25°C outside tolerance)
    if (distanceFromTolerance > 25.0f) {
        result.lethal = true;
        result.healthDamageRate = 5.0f;  // Rapid death
    }
    
    return result;
}
```

### Stress Curves

| Distance Outside Tolerance | Energy Drain | Health Damage | Status |
|---------------------------|--------------|---------------|--------|
| 0°C | 0% | 0% | Comfortable |
| 5°C | +25% | 0% | Mild stress |
| 10°C | +50% | 0.125%/tick | Moderate stress |
| 15°C | +75% | 0.25%/tick | Moderate stress |
| 20°C | +100% | 0.375%/tick | Severe stress |
| 25°C+ | +125%+ | 5%/tick | Lethal |

---

## Moisture Stress (Plants)

Plants have moisture requirements that create similar fitness gradients.

### Data Structure

```cpp
struct MoistureStress {
    float stressLevel;        // 0.0 = adequate, 1.0+ = severe
    float growthModifier;     // Multiplier on growth rate
    float healthDamageRate;   // Health loss per tick
};
```

### Calculation

```cpp
float calculateMoistureStress(
    float currentMoisture,
    float waterRequirement,
    float waterStorage
) {
    // Effective need based on genes
    // High water storage reduces effective need (succulent adaptation)
    float effectiveNeed = waterRequirement * (1.0f - waterStorage * 0.5f);
    float moistureDeficit = effectiveNeed - currentMoisture;
    
    if (moistureDeficit <= 0.0f) {
        return 0.0f;  // Adequate moisture
    }
    
    // Stress increases with deficit
    return moistureDeficit * 2.0f;  // 0-2 range typical
}
```

### Plant Adaptations

| Plant Type | Water Requirement | Water Storage | Effective Need |
|------------|-------------------|---------------|----------------|
| Desert Cactus | 0.1 | 0.8 | 0.06 |
| Prairie Grass | 0.4 | 0.2 | 0.36 |
| Rainforest Fern | 0.8 | 0.1 | 0.76 |
| Temperate Oak | 0.5 | 0.3 | 0.425 |

---

## Application Points

### For Creatures

Applied in creature update loop:

```cpp
void Creature::update(const EnvironmentState& env) {
    // Get tolerance genes
    float toleranceLow = getPhenotype().getTrait("temp_tolerance_low");
    float toleranceHigh = getPhenotype().getTrait("temp_tolerance_high");
    
    // Calculate stress
    auto stress = calculateTemperatureStress(
        env.temperature, toleranceLow, toleranceHigh
    );
    
    // Apply energy drain (increased metabolism)
    float baseMetabolism = getBaseMetabolismCost();
    float adjustedMetabolism = baseMetabolism * (1.0f + stress.energyDrainRate);
    drainEnergy(adjustedMetabolism);
    
    // Apply health damage
    if (stress.healthDamageRate > 0.0f) {
        damage(stress.healthDamageRate);
    }
    
    // Lethal check
    if (stress.lethal) {
        die(DeathCause::ENVIRONMENTAL);
    }
}
```

### For Plants

Applied in plant update loop:

```cpp
void Plant::update(const EnvironmentState& env) {
    // Temperature stress
    float toleranceLow = getPhenotype().getTrait("temp_tolerance_low");
    float toleranceHigh = getPhenotype().getTrait("temp_tolerance_high");
    auto tempStress = calculateTemperatureStress(
        env.temperature, toleranceLow, toleranceHigh
    );
    
    // Moisture stress
    float waterReq = getPhenotype().getTrait("water_requirement");
    float waterStorage = getPhenotype().getTrait("water_storage");
    float moistureStress = calculateMoistureStress(
        env.moisture, waterReq, waterStorage
    );
    
    // Combined stress affects growth rate
    float combinedStress = tempStress.stressLevel + moistureStress;
    float growthModifier = 1.0f / (1.0f + combinedStress);
    
    // Apply modified growth
    grow(growthModifier);
    
    // Apply damage from severe stress
    if (tempStress.healthDamageRate > 0.0f) {
        takeDamage(tempStress.healthDamageRate);
    }
    if (moistureStress > 1.0f) {
        takeDamage((moistureStress - 1.0f) * 0.1f);  // Drought damage
    }
}
```

---

## Related Genes

### Temperature Tolerance

| Gene | Range | Description |
|------|-------|-------------|
| `temp_tolerance_low` | -20°C to +20°C | Minimum survivable temperature |
| `temp_tolerance_high` | +20°C to +50°C | Maximum survivable temperature |

### Moisture (Plants)

| Gene | Range | Description |
|------|-------|-------------|
| `water_requirement` | 0.0 to 1.0 | Base water needs |
| `water_storage` | 0.0 to 1.0 | Succulent adaptation |

### Environmental Sensitivity (Creatures)

| Gene | Range | Description |
|------|-------|-------------|
| `environmental_sensitivity` | 0.0 to 2.0 | Weighs danger in pathfinding |

---

## Example Scenarios

### Arctic Fox in Desert

**Setup:**
- Tolerance: -30°C to +15°C
- Desert temperature: 40°C
- Distance outside tolerance: 25°C

**Effects:**
```
Stress Level: 1.0 (severe)
Energy Drain: +125% metabolism
Health Damage: 0.5% per tick
Status: Near lethal threshold

Creature will:
- Burn through energy quickly
- Lose health steadily  
- Die within ~200 ticks if not escaping
```

### Desert Cactus in Rainforest

**Setup:**
- Temp tolerance: +5°C to +50°C
- Water requirement: 0.1
- Water storage: 0.8
- Rainforest: 28°C, 95% moisture

**Effects:**
```
Temperature Stress: 0.0 (within tolerance)
Moisture Stress: 0.0 (excess water doesn't harm)
Growth Modifier: 1.0

However:
- Outcompeted by fast-growing rainforest plants
- Limited space due to dense vegetation
- May survive but won't thrive
```

---

## Performance Considerations

### Calculation Frequency

- Temperature stress: Calculated per tick per organism
- Moisture stress: Calculated per tick per plant
- Caching: Thermal adaptation values are cached (see below)

### Thermal Adaptation Cache

Creature thermal adaptations and tolerance ranges are now cached to reduce per-tick phenotype lookups:

```cpp
struct CachedThermalAdaptation {
    float toleranceLow;
    float toleranceHigh;
    float coldAdaptation;
    float heatAdaptation;
};

// Cached in creature, invalidated when phenotype context changes
CachedThermalAdaptation _cachedThermal;
```

**Cache invalidation:** The cache is invalidated when [`updatePhenotypeContext()`](../../../src/objects/creature/creature.cpp:1) is called (typically when environment changes significantly or creature moves to new biome).

**Performance impact:** Reduces per-tick phenotype lookups from 8+ trait queries to 0 for creatures that haven't moved biomes. This is significant because `getPhenotype().getTrait()` involves string lookups.

### Optimization Opportunities

1. **Spatial caching** - Cache stress values per tile, update when environment changes
2. **Batch calculation** - Process all organisms in same tile together
3. **Skip comfortable** - Skip detailed calculation for organisms well within tolerance

---

## Testing

Tests in [`test_environmental_stress.cpp`](../../../src/testing/genetics/test_environmental_stress.cpp:1):

```bash
./genetics_tests "[environmental_stress]"
```

Key test cases:
- Within tolerance returns zero stress
- Stress increases linearly with distance
- Lethal threshold triggers correctly
- Moisture stress considers water storage
- Combined stress affects growth correctly

---

## See Also

**Systems:**
- [[world-system]] - World architecture and environment queries
- [[behavior-system]] - How stress affects creature behavior

**Design:**
- [[../design/world-organism-integration]] - Full integration design
- [[../design/world-generation]] - Climate-based biome generation

**Reference:**
- [[../reference/genes]] - Gene catalog including tolerance genes
