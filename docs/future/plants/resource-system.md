---
title: Plant Resource Depletion System - Implementation Plan
created: 2025-12-26
updated: 2025-12-26
status: proposed
priority: critical
tags: [design, plants, ecosystem-balance, resources, growth-phases]
---

# Plant Resource Depletion System

## Overview

### Problem Statement

The code review identified "Unlimited Plant Food" as a **critical ecosystem balance issue**. Currently, plants provide infinite nutrition to herbivores without any resource depletion mechanism:

```cpp
// Current implementation (Plant.cpp:252-257)
float Plant::getNutrientValue() const {
    float baseValue = getGeneValueFromGenome(PlantGenes::NUTRIENT_VALUE, 25.0f);
    float sizeRatio = current_size_ / getMaxSize();
    return baseValue * sizeRatio;  // Returns value but NEVER depletes!
}
```

**Impact:** 
- Herbivore populations grow unbounded
- No selection pressure for foraging efficiency
- Ecosystem cannot self-regulate
- Plants have no evolutionary pressure for defense strategies
- Realistic food web dynamics impossible

### Solution Summary

Implement a **biologically-inspired plant resource system** where:

1. Plants store resources in different tissue types (leaves, fruit, stems, roots)
2. Each tissue type has different nutritional value and accessibility
3. Creature feeding consumes plant resources
4. Plants regenerate resources from photosynthesis at a rate influenced by genetics
5. Root investment enables survival and regrowth after heavy grazing
6. Different plant structures require different creature adaptations to access

---

## Biological Model

### Real-World Plant Resource Economics

In nature, plants operate as resource allocation systems:

```
                    PHOTOSYNTHESIS
                         │
                         ▼
           ┌─────────────────────────────┐
           │     AVAILABLE RESOURCES     │
           │   (sugars, ATP, nutrients)  │
           └─────────────────────────────┘
                    │
         ┌──────────┼──────────┬──────────┐
         ▼          ▼          ▼          ▼
    ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐
    │  GROWTH │ │ DEFENSE │ │ STORAGE │ │ REPROD. │
    │ (size)  │ │ (toxins)│ │ (roots) │ │ (seeds) │
    └─────────┘ └─────────┘ └─────────┘ └─────────┘
```

### Resource Tissue Types

Different plant tissues store different amounts of energy and require different adaptations to consume:

| Tissue Type | Energy Density | Defense Level | Regeneration | Access Requirement |
|-------------|---------------|---------------|--------------|-------------------|
| **Leaves** | Low (1.0x) | Low | Fast | None (easy to eat) |
| **Fruit** | High (2.5x) | None* | Seasonal | Color vision, sweetness preference |
| **Stems/Bark** | Medium (1.5x) | Medium | Slow | Strong teeth, patience |
| **Seeds/Nuts** | Very High (4.0x) | Hard shell | Annual | Jaw strength for nuts |
| **Roots** | Medium (1.5x) | Protected underground | Very slow | Digging behavior |

*Fruit defense is "give away" - plants want fruit eaten for seed dispersal

### Trade-offs in Plants

Plants must allocate limited resources:

| Strategy | High Investment In | Low Investment In | Ecological Niche |
|----------|-------------------|-------------------|------------------|
| **Fast Growth** | Growth rate | Defense, storage | Colonizers, disturbed areas |
| **Tank** | Defense, hardiness | Growth, reproduction | High-herbivore pressure |
| **Fruit Producer** | Reproduction, fruit | Defense | Animal-dispersed forest |
| **Survivor** | Root storage | Above-ground growth | Harsh environments |

---

## Growth Phase System

Plants progress through distinct life phases, each with different resource allocation priorities:

```
                                GROWTH PHASES
                                     
    GERMINATION ──► JUVENILE ──► MATURE ──► SENESCENT
    [Focus: Roots]   [Focus:     [Focus:    [Focus: Final
                      Growth]    Reproduce]  Seed Push]
```

### Phase Definitions

| Phase | Duration | Primary Allocation | Secondary | Reproduction |
|-------|----------|-------------------|-----------|--------------|
| **Germination** | 0-15% lifespan | Root establishment (60%) | Defense (25%) | None (0%) |
| **Juvenile** | 15-40% lifespan | Growth (50%) | Defense (30%) | Minimal (5%) |
| **Mature** | 40-85% lifespan | Reproduction (40%) | Growth (25%) | Full capacity |
| **Senescent** | 85-100% lifespan | Reproduction (60%) | Storage draw-down | Max seed output |

### Phase-Specific Behaviors

```cpp
enum class GrowthPhase {
    Germination,  ///< Establishing root system, vulnerable
    Juvenile,     ///< Rapid growth, building defenses
    Mature,       ///< Full reproduction capability
    Senescent     ///< Final reproductive push, declining health
};

/**
 * @brief Get current growth phase based on age and lifespan
 */
GrowthPhase Plant::getCurrentPhase() const {
    float ageRatio = age_ / getMaxLifespan();
    
    if (ageRatio < 0.15f) return GrowthPhase::Germination;
    if (ageRatio < 0.40f) return GrowthPhase::Juvenile;
    if (ageRatio < 0.85f) return GrowthPhase::Mature;
    return GrowthPhase::Senescent;
}

/**
 * @brief Phase-specific allocation modifiers
 *
 * These modify the base genetic allocation strategy
 */
struct PhaseAllocationModifiers {
    float growth_mod;
    float defense_mod;
    float storage_mod;
    float reproduction_mod;
};

const std::map<GrowthPhase, PhaseAllocationModifiers> PHASE_MODIFIERS = {
    {GrowthPhase::Germination, {0.3f, 0.25f, 0.60f, 0.0f}},
    {GrowthPhase::Juvenile,    {0.50f, 0.30f, 0.15f, 0.05f}},
    {GrowthPhase::Mature,      {0.25f, 0.20f, 0.15f, 0.40f}},
    {GrowthPhase::Senescent,   {0.05f, 0.05f, 0.30f, 0.60f}},
};
```

### Phase Integration with Resource System

- **Germination**: Cannot be eaten effectively (tiny), focus on root depth
- **Juvenile**: Leaves grow fast, but fruit not produced yet
- **Mature**: Full tissue availability, fruit/seeds when in season
- **Senescent**: Draws down root storage for final seed production

> [!NOTE]
> Growth phases interact with the existing `mature_` flag but provide finer granularity. A plant is "mature" (can reproduce) when in Mature or Senescent phase.

---

## Universal Gene Pool Architecture

> [!IMPORTANT]
> All genes in EcoSim come from a **universal gene pool** (`UniversalGenes`). There are no "plant-only" or "creature-only" genes - just genes that are **expressed differently** based on organism type.

### How Expression Works

Plants and creatures share the same genome structure, but differ in **expression levels**:

```cpp
// From UniversalGenes.cpp - expression levels differ, not gene availability
// Creatures: High mobility/heterotrophy, dormant autotrophy
// Plants: High autotrophy, dormant mobility/heterotrophy

// Example: PHOTOSYNTHESIS gene
// In creature genome: Allele a(0.0f, 0.1f);  // Dormant but can evolve
// In plant genome:    Allele a(0.8f, 1.0f);  // Fully active
```

### Gene Categories (from `GeneCategory` enum)

| Category | Primary Expression | Description |
|----------|-------------------|-------------|
| `Universal` | All organisms | Lifespan, size, metabolism |
| `Mobility` | Creatures | Locomotion, navigation, vision |
| `Autotrophy` | Plants | Photosynthesis, roots, water storage |
| `Heterotrophy` | Creatures | Hunting, digestion, diet |
| `Morphology` | Both | Physical structure (teeth, gut, thorns) |
| `Behavior` | Creatures | Caching, grooming, preferences |
| `PlantDefense` | Plants | Toxins, thorns, regrowth |
| `Reproduction` | Both | Offspring, spreading, seeds |

### Prefabs Set Expression Levels

Different organism types are created via prefabs that set **different expression levels** for the same genes:

```cpp
// Plant prefabs set high autotrophy, low mobility
PlantFactory::createGrass(registry);    // High photosynthesis, low locomotion
PlantFactory::createBush(registry);     // Medium storage, high fruit_appeal
PlantFactory::createTree(registry);     // High root_depth, long lifespan

// Creature prefabs set high mobility, low autotrophy
CreatureFactory::createHerbivore(registry);  // High plant_digestion, low meat
CreatureFactory::createPredator(registry);   // High meat_digestion, low plant
```

---

## Current State Analysis

### Existing Genes (Verified from UniversalGenes.hpp)

The following genes **actually exist** in the codebase and can be leveraged:

**Autotrophy Genes (Active in Plants):**

| Gene ID | Range | Current Function | Resource System Role |
|---------|-------|-----------------|---------------------|
| `photosynthesis` | [0.0, 1.0] | Energy conversion efficiency | Resource regeneration rate |
| `root_depth` | [0.0, 10.0] | Underground resource access | Storage capacity, regrowth ability |
| `water_storage` | [0.0, 1.0] | Internal water capacity | Resource buffer capacity |
| `light_requirement` | [0.0, 1.0] | Light needs | Photosynthesis efficiency |
| `water_requirement` | [0.0, 1.0] | Water needs | Growth constraints |

**Plant Defense Genes:**

| Gene ID | Range | Current Function | Resource System Role |
|---------|-------|-----------------|---------------------|
| `toxin_production` | [0.0, 1.0] | Chemical defense production | Damage to herbivores |
| `thorn_density` | [0.0, 1.0] | Physical defense density | Access difficulty |
| `regrowth_rate` | [0.0, 1.0] | Recovery after grazing | Regeneration speed |
| `hardiness` | [0.0, 1.0] | Resistance to damage/stress | Tissue durability |

**Plant Reproduction Genes:**

| Gene ID | Range | Current Function | Resource System Role |
|---------|-------|-----------------|---------------------|
| `fruit_appeal` | [0.0, 1.0] | Fruit attractiveness | Fruit tissue availability |
| `fruit_production_rate` | [0.0, 1.0] | Food object spawning | Fruit resource generation |
| `seed_coat_durability` | [0.0, 1.0] | Digestion survival | Seed tissue protection |
| `offspring_count` | [1, 20] | Seeds per cycle | Reproductive resource cost |
| `seed_mass` | [0.01, 1.0] | Seed weight | Energy per seed |

**Universal Genes (Both):**

| Gene ID | Range | Current Function | Resource System Role |
|---------|-------|-----------------|---------------------|
| `metabolism_rate` | [0.1, 3.0] | Base metabolic rate | Growth speed modifier |
| `max_size` | [0.5, 20.0] | Maximum organism size | Total resource capacity |
| `nutrient_value` | [1.0, 100.0] | How nutritious when eaten | Base nutrition output |

**Creature Digestion Genes (for feeding mechanics):**

| Gene ID | Range | Current Function | Resource System Role |
|---------|-------|-----------------|---------------------|
| `plant_digestion_efficiency` | [0.0, 1.0] | Extract calories from plants | Feeding efficiency |
| `cellulose_breakdown` | [0.0, 1.0] | Digest tough plant fibers | Tough tissue access |
| `toxin_tolerance` | [0.0, 1.0] | Resist plant toxins | Toxic plant consumption |
| `jaw_strength` | [0.0, 1.0] | Crack hard nuts/shells | Seed/bark access |
| `color_vision` | [0.0, 1.0] | Detect ripe fruit | Fruit finding |
| `sweetness_preference` | [0.0, 1.0] | Attraction to fruit | Diet preference |

### Existing Systems to Integrate

1. **FeedingInteraction** - Already calculates `plantDamage` (0.3-0.8) but doesn't apply it
2. **Plant::update()** - Growth loop exists, needs resource tracking
3. **EnergyBudget** - Framework for organism energy tracking (designed but not fully implemented)
4. **Phenotype** - Context-aware trait expression (underutilized for plants)

### Gap Analysis

| Required Feature | Current State | Work Needed |
|-----------------|---------------|-------------|
| Resource tracking | ❌ Missing | New `PlantResources` struct |
| Resource consumption | ❌ Missing | Extend `FeedingInteraction` |
| Resource regeneration | ❌ Missing | Extend `Plant::update()` |
| Tissue-type nutrition | ❌ Missing | New enum + nutrition map |
| Adaptation requirements | 🟡 Partial | Extend feeding checks |
| Regrowth from roots | ❌ Missing | New regrowth system |

---

## Proposed Design

### Resource Model

#### PlantResources Structure

```cpp
/**
 * @brief Tracks consumable resources in a plant
 * 
 * Resources are stored in different tissue types, each with different
 * nutritional values, regeneration rates, and access requirements.
 */
struct PlantResources {
    // Current resource levels (0.0 = depleted, 1.0 = full)
    float leaf_resources = 1.0f;      ///< Easy to access, fast regen
    float stem_resources = 1.0f;      ///< Harder to access, medium regen
    float fruit_resources = 0.0f;     ///< Seasonal, high value
    float seed_resources = 0.0f;      ///< Annual, very high value
    float root_resources = 1.0f;      ///< Underground, enables regrowth
    
    // Maximum capacity (scaled by genes)
    float max_leaf_capacity = 1.0f;
    float max_stem_capacity = 1.0f;
    float max_fruit_capacity = 0.5f;
    float max_seed_capacity = 0.3f;
    float max_root_capacity = 1.0f;   ///< Critical: Determines regrowth potential
    
    // Regeneration rates (per tick, modified by photosynthesis)
    float leaf_regen_rate = 0.01f;    ///< Fast: ~100 ticks to full
    float stem_regen_rate = 0.002f;   ///< Slow: ~500 ticks to full
    float fruit_regen_rate = 0.005f;  ///< Medium: ~200 ticks when mature
    float root_regen_rate = 0.001f;   ///< Very slow: ~1000 ticks to full
    
    // Calculated properties
    float getTotalResources() const {
        return leaf_resources + stem_resources + fruit_resources + 
               seed_resources + root_resources;
    }
    
    float getAvailableNutrition() const; // Accounts for accessibility
    bool canRegrow() const { return root_resources > 0.1f; }
};
```

#### Tissue Type Enumeration

```cpp
/**
 * @brief Different plant tissues that creatures can consume
 * 
 * Each tissue type has different:
 * - Nutritional value per unit
 * - Regeneration speed
 * - Access requirements (creature adaptations needed)
 */
enum class PlantTissue {
    Leaf,       ///< Easy to eat, low nutrition, fast regen
    Stem,       ///< Requires strong teeth, medium nutrition
    Fruit,      ///< Attracts frugivores, high nutrition, seasonal
    Seed,       ///< Very high nutrition, hard shell protection
    Root,       ///< Underground, hard to access, enables regrowth
    Bark        ///< Very hard to eat, provides structural nutrients
};

/**
 * @brief Nutritional and access properties for each tissue type
 */
struct TissueProperties {
    float nutrition_multiplier;     ///< Relative to base nutrient value
    float base_accessibility;       ///< 0.0 = impossible, 1.0 = easy
    float regen_rate_multiplier;    ///< Relative to base regen
    std::string required_adaptation; ///< Gene needed for efficient access
    float adaptation_threshold;      ///< Minimum gene value for access
};

// Tissue property definitions
const std::map<PlantTissue, TissueProperties> TISSUE_PROPERTIES = {
    {PlantTissue::Leaf,  {1.0f, 0.9f, 1.0f,  "", 0.0f}},  // Always accessible
    {PlantTissue::Stem,  {1.5f, 0.4f, 0.2f,  "jaw_strength", 0.5f}},
    {PlantTissue::Fruit, {2.5f, 0.8f, 0.5f,  "color_vision", 0.3f}},
    {PlantTissue::Seed,  {4.0f, 0.2f, 0.1f,  "jaw_strength", 0.7f}},
    {PlantTissue::Root,  {1.5f, 0.1f, 0.1f,  "claw_length", 0.6f}},  // Digging
    {PlantTissue::Bark,  {0.8f, 0.2f, 0.05f, "cellulose_breakdown", 0.7f}},
};
```

### Resource Allocation

#### Allocation Gene Integration

The existing `resource_allocation_design.md` framework can be extended for plants:

```cpp
/**
 * @brief Plant-specific resource allocation priorities
 *
 * Each tick, available energy is allocated based on genetic priorities.
 * Uses existing EnergyBudget framework concepts.
 *
 * NOTE: Uses verified genes from UniversalGenes.hpp only.
 */
struct PlantAllocationStrategy {
    float growth_priority = 0.3f;        ///< Above-ground growth
    float defense_priority = 0.2f;       ///< Toxin/thorn production
    float storage_priority = 0.2f;       ///< Root investment
    float reproduction_priority = 0.3f;  ///< Seeds/fruit
    
    // Derived from VERIFIED genes (UniversalGenes.hpp)
    void calculateFromGenome(const Phenotype& phenotype, GrowthPhase phase) {
        // Base priorities from genes
        // metabolism_rate influences growth speed
        float growthBase = phenotype.getTrait("metabolism_rate") * 0.3f;
        
        // Defense from verified genes: toxin_production + thorn_density
        float defenseBase = (phenotype.getTrait("toxin_production") +
                            phenotype.getTrait("thorn_density")) * 0.25f;
        
        // Root storage from root_depth + water_storage
        float storageBase = (phenotype.getTrait("root_depth") / 10.0f +
                            phenotype.getTrait("water_storage")) * 0.25f;
        
        // Reproduction from fruit_appeal + fruit_production_rate
        float reproBase = (phenotype.getTrait("fruit_appeal") +
                          phenotype.getTrait("fruit_production_rate")) * 0.25f;
        
        // Apply phase modifiers (from PHASE_MODIFIERS table)
        const auto& mods = PHASE_MODIFIERS.at(phase);
        growth_priority = growthBase * mods.growth_mod;
        defense_priority = defenseBase * mods.defense_mod;
        storage_priority = storageBase * mods.storage_mod;
        reproduction_priority = reproBase * mods.reproduction_mod;
        
        // Normalize to sum to 1.0
        normalize();
    }
    
    void normalize() {
        float total = growth_priority + defense_priority +
                      storage_priority + reproduction_priority;
        if (total > 0.0f) {
            growth_priority /= total;
            defense_priority /= total;
            storage_priority /= total;
            reproduction_priority /= total;
        }
    }
};
```

#### Resource Generation

> [!IMPORTANT]
> **Photosynthesis requires leaves.** Roots can absorb soil nutrients but cannot photosynthesize. Plants that over-invest in roots without maintaining leaves cannot sustain themselves.

```cpp
/**
 * @brief Calculate resource income from photosynthesis
 *
 * CRITICAL: Photosynthesis requires leaves! A plant with depleted leaf
 * resources cannot photosynthesize effectively, creating a critical
 * dependency: leaves → photosynthesis → all other resources.
 *
 * @param plant The plant generating resources
 * @param environment Current environmental conditions
 * @return Energy units generated this tick
 */
float calculatePhotosynthesisIncome(const Plant& plant, const Environment& env) {
    float baseRate = plant.getPhenotype().getTrait("photosynthesis");
    
    // CRITICAL: Leaf resources directly limit photosynthesis
    // No leaves = no photosynthesis, regardless of other factors
    float leafFactor = plant.getResources().leaf_resources /
                       plant.getResources().max_leaf_capacity;
    
    if (leafFactor < 0.05f) {
        // Plant has essentially no photosynthetic capacity
        // Can only survive on root reserves until leaves regrow
        return 0.0f;
    }
    
    // Environmental factors
    float lightFactor = std::min(1.0f, env.light_level / plant.getLightNeed());
    float waterFactor = std::min(1.0f, env.water_level / plant.getWaterNeed());
    float tempFactor = plant.getTemperatureEfficiency(env.temperature);
    
    // Size affects total photosynthesis (larger plants capture more light)
    // But this is now modulated by actual leaf availability
    float sizeFactor = plant.getCurrentSize() / plant.getMaxSize();
    
    // Health affects efficiency
    float healthFactor = plant.getHealth();
    
    // Final calculation - leaf factor is the critical limiter
    return baseRate * leafFactor * lightFactor * waterFactor * tempFactor *
           sizeFactor * healthFactor * PHOTOSYNTHESIS_BASE_RATE;
}

/**
 * @brief Calculate soil nutrient income from roots
 *
 * Unlike photosynthesis, roots can gather nutrients independently.
 * This provides a secondary income that helps during leaf regrowth.
 */
float calculateRootNutrientIncome(const Plant& plant) {
    float rootDepth = plant.getPhenotype().getTrait("root_depth");
    float rootHealth = plant.getResources().root_resources /
                       plant.getResources().max_root_capacity;
    
    // Roots provide ~10-20% of what healthy leaves provide
    // This allows survival during regrowth but not thriving
    return rootDepth * rootHealth * ROOT_NUTRIENT_BASE_RATE;
}
```

### Leaf-Photosynthesis Dependency

This creates a critical biological trade-off:

```
          ┌─────────────────────────────────────────┐
          │           RESOURCE FLOW                 │
          └─────────────────────────────────────────┘
          
    LEAVES ────────► PHOTOSYNTHESIS ────────► ALL RESOURCES
       ▲                                          │
       │                                          │
       └──────────────── regrowth ◄───────────────┘
       
    ROOTS ────────► SOIL NUTRIENTS ────────► SURVIVAL ONLY
                    (small amount)            (not growth)
```

**Evolutionary Implications:**
- Plants cannot just invest everything in roots
- Leaf defense becomes critical (thorns, toxins)
- Fast leaf regrowth (`regrowth_rate` gene) is valuable
- Seasonal deciduous strategy: lose leaves when grazing pressure high

### Feeding Mechanics

#### Extended FeedingResult

```cpp
struct FeedingResult {
    // Existing fields
    bool success = false;
    float nutritionGained = 0.0f;
    float damageReceived = 0.0f;
    float plantDamage = 0.0f;
    bool seedsConsumed = false;
    bool seedsDestroyed = false;
    float seedsToDisperse = 0;
    std::string description;
    
    // NEW: Resource depletion tracking
    PlantTissue tissueConsumed = PlantTissue::Leaf;
    float resourcesConsumed = 0.0f;        ///< Amount actually taken from plant
    bool plantKilled = false;               ///< Did feeding kill the plant?
    bool plantCanRegrow = true;             ///< Can plant recover?
};
```

#### Tissue Selection Architecture

> [!IMPORTANT]
> **Tissue selection is a creature behavior decision**, not a FeedingInteraction responsibility. The creature's action selection system chooses *what* to eat; FeedingInteraction executes *how* to eat it.

```
    ARCHITECTURE SEPARATION
    
    ┌─────────────────────────────────────────────────────────┐
    │               CREATURE BEHAVIOR SYSTEM                  │
    │  (Action Selection - decides WHAT to do)                │
    │                                                         │
    │  selectFeedingAction() {                                │
    │      // 1. Find nearby food sources                     │
    │      // 2. Evaluate accessibility vs nutrition          │
    │      // 3. Select target plant AND target tissue        │
    │      return FeedingAction{plant, tissue, priority};     │
    │  }                                                      │
    └─────────────────────────────────────────────────────────┘
                              │
                              ▼ (passes selected tissue)
    ┌─────────────────────────────────────────────────────────┐
    │               FEEDING INTERACTION                       │
    │  (Execution - HOW to eat selected tissue)               │
    │                                                         │
    │  attemptToEatPlant(creature, plant, tissue) {           │
    │      // 1. Validate creature can access this tissue     │
    │      // 2. Calculate efficiency penalties               │
    │      // 3. Execute consumption, deplete resources       │
    │      // 4. Return nutrition gained                      │
    │  }                                                      │
    └─────────────────────────────────────────────────────────┘
```

#### Creature Behavior: Tissue Preference Calculation

This logic belongs in the creature's behavior/decision-making system:

```cpp
// In creature behavior/action selection system
// File: include/objects/creature/creature.hpp or similar

/**
 * @brief Evaluate all accessible tissues and return preferred target
 *
 * This is a BEHAVIOR decision, not a feeding mechanic.
 * Called during action selection when creature decides to forage.
 */
struct TissuePreference {
    PlantTissue tissue;
    float desirability;     ///< How much creature wants this tissue
    float accessibility;    ///< How easily creature can get it
};

std::vector<TissuePreference> Creature::evaluatePlantTissues(
    const Plant& plant) const
{
    std::vector<TissuePreference> preferences;
    const Phenotype& phenotype = getPhenotype();
    
    for (const auto& [tissue, props] : TISSUE_PROPERTIES) {
        // Check if plant has this tissue available
        float available = plant.getResources().getTissueLevel(tissue);
        if (available <= 0.0f) continue;
        
        // Calculate creature's ability to access this tissue
        float accessAbility = 1.0f;
        if (!props.required_adaptation.empty()) {
            accessAbility = phenotype.getTrait(props.required_adaptation);
        }
        
        // Calculate desirability based on nutrition and preferences
        float desirability = props.nutrition_multiplier * available;
        
        // Frugivores strongly prefer fruit
        if (tissue == PlantTissue::Fruit) {
            desirability *= 1.0f + phenotype.getTrait("sweetness_preference");
        }
        
        // Desperate creatures consider inaccessible tissues
        bool canAccess = accessAbility >= props.adaptation_threshold;
        if (!canAccess && getHunger() < 0.8f) {
            continue;  // Skip inaccessible unless desperate
        }
        
        preferences.push_back({
            tissue,
            desirability,
            canAccess ? accessAbility : accessAbility * 0.3f
        });
    }
    
    return preferences;
}

/**
 * @brief Select best tissue to target for feeding
 */
PlantTissue Creature::selectTargetTissue(const Plant& plant) const {
    auto preferences = evaluatePlantTissues(plant);
    
    if (preferences.empty()) {
        return PlantTissue::Leaf;  // Default fallback
    }
    
    // Score = desirability * accessibility
    auto best = std::max_element(preferences.begin(), preferences.end(),
        [](const auto& a, const auto& b) {
            return (a.desirability * a.accessibility) <
                   (b.desirability * b.accessibility);
        });
    
    return best->tissue;
}
```

#### FeedingInteraction: Execute Selected Consumption

FeedingInteraction receives the already-selected tissue as a parameter:

```cpp
// FeedingInteraction receives tissue choice, doesn't make it
FeedingResult FeedingInteraction::attemptToEatPlant(
    const Phenotype& creaturePhenotype,
    Plant& plant,
    PlantTissue targetTissue,  // NEW: Passed in, not calculated here
    float creatureHunger)
{
    // Validate and execute consumption of specified tissue
    // (implementation in Consumption section below)
}
```

#### Consumption Implementation

```cpp
/**
 * @brief Consume resources from plant, depleting its stores
 * 
 * @param plant Target plant (modified)
 * @param tissue Which tissue to consume
 * @param amount How much to try to consume
 * @param efficiency Creature's digestion efficiency
 * @return Actual nutrition gained (may be less if plant depleted)
 */
float consumePlantResources(Plant& plant, PlantTissue tissue, 
                            float amount, float efficiency) {
    PlantResources& resources = plant.getResourcesMutable();
    
    // Get current tissue level
    float available = resources.getTissueLevel(tissue);
    float consumed = std::min(amount, available);
    
    // Deplete the tissue
    resources.setTissueLevel(tissue, available - consumed);
    
    // Calculate nutrition based on tissue type
    const auto& props = TISSUE_PROPERTIES.at(tissue);
    float nutrition = consumed * props.nutrition_multiplier * 
                      plant.getNutrientValue() * efficiency;
    
    // Heavy grazing on non-root tissue damages above-ground parts
    if (tissue != PlantTissue::Root && consumed > 0.3f) {
        float structuralDamage = (consumed - 0.3f) * 0.5f;
        plant.takeDamage(structuralDamage);
    }
    
    // Eating roots damages regrowth potential
    if (tissue == PlantTissue::Root) {
        if (resources.root_resources < 0.1f) {
            plant.markForDeath("Root system destroyed");
        }
    }
    
    return nutrition;
}
```

### Regrowth Mechanics

#### Regrowth Algorithm

```cpp
/**
 * @brief Process plant resource regeneration and regrowth
 * 
 * Called each tick during Plant::update().
 * Plants regenerate resources from photosynthesis income,
 * with priority given based on allocation strategy.
 */
void Plant::regenerateResources(float deltaTime) {
    // Calculate income
    float income = calculatePhotosynthesisIncome(*this, current_environment_);
    
    // Get allocation strategy from genes
    PlantAllocationStrategy allocation;
    allocation.calculateFromGenome(phenotype_);
    
    // Distribute income to different pools
    float growth_budget = income * allocation.growth_priority;
    float defense_budget = income * allocation.defense_priority;
    float storage_budget = income * allocation.storage_priority;
    float reprod_budget = income * allocation.reproduction_priority;
    
    // Root regeneration (critical for survival)
    if (resources_.root_resources < resources_.max_root_capacity) {
        float rootRegen = std::min(storage_budget, 
                                    resources_.root_regen_rate * deltaTime);
        resources_.root_resources += rootRegen;
        storage_budget -= rootRegen;
    }
    
    // Leaf regeneration (fast, for continued photosynthesis)
    if (resources_.leaf_resources < resources_.max_leaf_capacity) {
        float leafRegen = std::min(growth_budget * 0.5f,
                                    resources_.leaf_regen_rate * deltaTime);
        resources_.leaf_resources += leafRegen;
    }
    
    // Stem regeneration (slow)
    if (resources_.stem_resources < resources_.max_stem_capacity) {
        float stemRegen = std::min(growth_budget * 0.3f,
                                    resources_.stem_regen_rate * deltaTime);
        resources_.stem_resources += stemRegen;
    }
    
    // Fruit production (only when mature, seasonal)
    if (mature_ && isInFruitingSeason()) {
        if (resources_.fruit_resources < resources_.max_fruit_capacity) {
            float fruitRegen = std::min(reprod_budget,
                                        resources_.fruit_regen_rate * deltaTime);
            resources_.fruit_resources += fruitRegen;
        }
    }
    
    // Defense maintenance costs energy
    float defenseMaintenanceCost = (toxicity_ + thorn_density_) * 
                                    DEFENSE_MAINTENANCE_RATE * deltaTime;
    // Deduct from defense budget (if insufficient, defenses weaken)
}
```

#### Root Investment Benefits

```cpp
/**
 * @brief Root system determines:
 * - Resource storage capacity
 * - Regrowth speed after grazing
 * - Survival after heavy damage
 */
struct RootSystemBenefits {
    /**
     * @brief Calculate root storage capacity from genes
     * 
     * root_depth gene directly increases storage capacity
     */
    static float calculateStorageCapacity(const Phenotype& phenotype) {
        float rootDepth = phenotype.getTrait("root_depth");
        float waterStorage = phenotype.getTrait("water_storage");
        
        // Deep roots = more storage (up to 3x base)
        return 1.0f + (rootDepth / 10.0f) * 2.0f + waterStorage * 0.5f;
    }
    
    /**
     * @brief Calculate regrowth rate multiplier
     * 
     * Plants with strong root systems regrow faster
     */
    static float calculateRegrowthMultiplier(const Plant& plant) {
        float rootHealth = plant.getResources().root_resources / 
                           plant.getResources().max_root_capacity;
        float rootDepth = plant.getPhenotype().getTrait("root_depth");
        
        // Need at least 10% roots to regrow
        if (rootHealth < 0.1f) return 0.0f;
        
        // Deeper roots = faster recovery
        return rootHealth * (1.0f + rootDepth * 0.2f);
    }
    
    /**
     * @brief Check if plant can survive total above-ground destruction
     * 
     * Plants with >30% root resources can regrow from rootstock
     */
    static bool canSurviveDefoliation(const Plant& plant) {
        return plant.getResources().root_resources > 0.3f;
    }
};
```

---

## Gene Integration

### Genes Already Available (Verified from UniversalGenes.hpp)

> [!NOTE]
> Most genes needed for this system **already exist** in the universal gene pool. We extend their usage rather than adding new genes.

**Plant Resource Genes (already implemented):**

| Gene ID | Current Range | Current Use | Extended Use |
|---------|--------------|-------------|--------------|
| `photosynthesis` | [0.0, 1.0] | Energy conversion | Resource regeneration rate |
| `root_depth` | [0.0, 10.0] | Drought resistance | Storage capacity, regrowth ability |
| `water_storage` | [0.0, 1.0] | Internal water capacity | Resource buffer capacity |
| `regrowth_rate` | [0.0, 1.0] | Recovery after grazing | Tissue regeneration multiplier |
| `hardiness` | [0.0, 1.0] | Damage resistance | Tissue durability |

**Plant Defense Genes (already implemented):**

| Gene ID | Current Range | Current Use | Extended Use |
|---------|--------------|-------------|--------------|
| `toxin_production` | [0.0, 1.0] | Chemical defense | Damage to herbivores on consumption |
| `thorn_density` | [0.0, 1.0] | Physical defense | Access difficulty modifier |

**Reproduction Genes (already implemented):**

| Gene ID | Current Range | Current Use | Extended Use |
|---------|--------------|-------------|--------------|
| `fruit_appeal` | [0.0, 1.0] | Fruit attractiveness | Fruit tissue availability |
| `fruit_production_rate` | [0.0, 1.0] | Food spawning | Fruit resource generation |
| `seed_coat_durability` | [0.0, 1.0] | Digestion survival | Seed tissue protection |
| `offspring_count` | [1, 20] | Seeds per cycle | Reproductive resource cost |
| `seed_mass` | [0.01, 1.0] | Seed weight | Energy per seed |

**Creature Digestion Genes (already implemented):**

| Gene ID | Current Range | Current Use | Extended Use |
|---------|--------------|-------------|--------------|
| `plant_digestion_efficiency` | [0.0, 1.0] | Plant calorie extraction | Feeding efficiency |
| `cellulose_breakdown` | [0.0, 1.0] | Tough fiber digestion | Bark/stem access |
| `toxin_tolerance` | [0.0, 1.0] | Resist plant toxins | Toxic plant consumption |
| `jaw_strength` | [0.0, 1.0] | Bite force | Nut/bark access |
| `color_vision` | [0.0, 1.0] | Ripe fruit detection | Fruit finding |
| `sweetness_preference` | [0.0, 1.0] | Fruit attraction | Diet preference weighting |
| `claw_length` | [0.0, 1.0] | Digging ability | Root access |

### New Genes Required

The following genes fill gaps not covered by existing genes:

| Gene ID | Chromosome | Range | Purpose | Trade-offs Enabled |
|---------|-----------|-------|---------|-------------------|
| `bark_thickness` | Morphology | [0.0, 1.0] | Stem/bark structural protection | Fire resistance, woody vs herbaceous, stem consumption difficulty |
| `root_storage_efficiency` | Autotrophy | [0.0, 1.0] | How efficiently roots store energy | Deep/inefficient (drought-resistant) vs shallow/efficient (grazing-resistant) |
| `seed_energy_reserve` | Reproduction | [0.0, 1.0] | Energy density within seeds | High energy (better survival) vs low energy (lighter, better dispersal) |
| `fruiting_season_length` | Reproduction | [0.0, 1.0] | Duration of fruit production | Short/intense (many dispersers) vs long/steady (reliable dispersal) |

#### Gene Details

**`bark_thickness`** [0.0 - 1.0]
- Different from `hardiness` (general damage resistance)
- Different from `thorn_density` (deters before consumption)
- Affects:
  - Stem tissue accessibility (higher = harder to consume)
  - Fire resistance (savanna/grassland adaptation)
  - Woody vs herbaceous plant distinction
- High bark + low thorns = "tough but approachable" (like tree bark)
- Low bark + high thorns = "soft but defended" (like cacti)

**`root_storage_efficiency`** [0.0 - 1.0]
- Distinct from `root_depth` (how deep) and `water_storage` (water capacity)
- Affects how efficiently energy is stored/retrieved from roots
- Creates strategies:
  - **Deep + Low Efficiency**: Drought resistant, slow regrowth after grazing
  - **Shallow + High Efficiency**: Fast regrowth from grazing, drought vulnerable
  - **Deep + High Efficiency**: Best of both but high energy cost to build

**`seed_energy_reserve`** [0.0 - 1.0]
- Distinct from `seed_mass` (physical weight)
- Real-world examples:
  - Coconut: Large mass, low energy density (mostly husk/water)
  - Acorn: Moderate mass, very high energy density
  - Dandelion: Tiny mass, minimal energy reserve
- Trade-offs:
  - High reserve → better seedling survival, worse dispersal (heavy)
  - Low reserve → lighter seeds disperse farther, seedlings more vulnerable

---

### Architecture Discussion: Fruit/Seed Gene Relationship

> [!IMPORTANT]
> **Design Question**: Should fruits and seeds share the same genes, or be distinct? Biologically, a fruit IS a seed distribution mechanism—the fleshy part exists solely to attract dispersers.

#### Current Gene Separation

Currently, we have separate genes for fruit and seeds:

| Fruit-Related Genes | Seed-Related Genes |
|--------------------|--------------------|
| `fruit_appeal` | `seed_mass` |
| `fruit_production_rate` | `seed_coat_durability` |
| (proposed) `fruiting_season_length` | `offspring_count` |
| | (proposed) `seed_energy_reserve` |

#### The Biological Reality

```
                    REPRODUCTIVE STRUCTURE
                           │
           ┌───────────────┴───────────────┐
           │                               │
      SEED (embryo)                   FRUIT (optional wrapper)
      - The actual offspring         - Attracts dispersers
      - Energy reserve               - Protects seed during transport
      - Protective coat              - Digestible or not
```

**Key Insight**: All plants produce seeds, but only SOME wrap them in fruit.

#### Three Possible Architectures

**Option A: Keep Separate (Current Design)**
```
PlantTissue::Fruit  → uses fruit_appeal, fruit_production_rate
PlantTissue::Seed   → uses seed_mass, seed_energy_reserve, seed_coat_durability
```

Pros:
- Simple to implement
- Clear separation of "wrapper" vs "embryo"

Cons:
- Doesn't model that fruit CONTAINS seeds
- Eating fruit should disperse seeds; eating seeds destroys them

---

**Option B: Hierarchical Model (Fruit Contains Seeds)**
```cpp
struct ReproductiveStructure {
    bool has_fruit;           // Does this plant make fruit?
    float fruit_appeal;       // Attractiveness of fruit wrapper
    float fruit_production;   // Rate of fruit development
    
    // Seeds are INSIDE fruit (if fruit exists) or standalone
    SeedProperties seeds;     // mass, energy, coat durability, count
};
```

Pros:
- More biologically accurate
- Naturally models: eat fruit → seeds pass through → dispersal
- Eat seed directly → no dispersal

Cons:
- More complex implementation
- Need to track fruit vs naked-seed plants separately

---

**Option C: Unified Propagule Gene Set**
```cpp
// "Propagule" = generic term for dispersal unit (seed OR fruit)
struct PropaguleGenes {
    float wrapper_investment;     // 0 = naked seed, 1 = fleshy fruit
    float wrapper_appeal;         // Attractiveness (only matters if wrapped)
    float core_energy_reserve;    // Seed energy
    float core_protection;        // Seed coat durability
    float dispersal_timing;       // Season length
};
```

Pros:
- Single unified system
- Naturally creates spectrum: naked seeds ↔ nuts ↔ berries ↔ fleshy fruits

Cons:
- Requires rethinking existing fruit/seed gene structure
- May be over-engineered for current needs

---

#### Recommended Design: Option C - Unified Propagule System

> [!NOTE]
> **Future Enhancement**: The detailed PropaguleGenes design is documented separately in [[propagule-genes|Unified Propagule Gene System]].

The PropaguleGenes concept elegantly models the full spectrum of plant reproductive strategies using a single unified gene set with a key innovation: `wrapper_investment` [0.0-1.0] creates a continuous spectrum:

- **0.0** = naked seeds (grass, dandelion, pine)
- **0.3** = nuts (oak, walnut)
- **0.6** = drupes/stone fruits (cherry, plum)
- **0.9** = berries (tomato, grape)

**Key behaviors:**
- Eating wrapper (fruit) → seeds may survive digestion → dispersal
- Eating seeds directly → seeds destroyed → no dispersal, high nutrition

This system should be implemented **after** the core resource depletion system is working. See [[propagule-genes]] for full implementation details.

---

### Creature Adaptation Requirements

| Plant Tissue | Required Gene | Threshold | Effect if Below Threshold |
|--------------|--------------|-----------|--------------------------|
| Stem/Bark | `jaw_strength` | 0.5 | 70% reduced efficiency |
| Seeds/Nuts | `jaw_strength` | 0.7 | Cannot crack shells |
| Roots | `claw_length` | 0.6 | Cannot dig |
| Fruit | `color_vision` | 0.3 | 50% detection range |
| Tough leaves | `cellulose_breakdown` | 0.5 | 50% nutrition extraction |

---

## Implementation Plan

### Phase 1: Core Resource Tracking (8 hours)

**Goal:** Add resource storage to Plant class without changing behavior

**Files Modified:**
- `include/genetics/organisms/Plant.hpp` - Add `PlantResources` struct
- `src/genetics/organisms/Plant.cpp` - Initialize resources, add getters

**Tasks:**
1. Create `PlantResources` structure (2 hours)
2. Add `resources_` member to Plant class (1 hour)
3. Initialize resources based on size/genes (2 hours)
4. Add resource query methods (1 hour)
5. Unit tests for resource tracking (2 hours)

**Acceptance Criteria:**
- [ ] Plants track leaf, stem, fruit, seed, root resources
- [ ] Resources initialized correctly on plant creation
- [ ] Resources scale with plant size and genes
- [ ] All existing tests still pass

### Phase 2: Feeding Integration (12 hours)

**Goal:** FeedingInteraction depletes plant resources

**Files Modified:**
- `include/genetics/interactions/FeedingInteraction.hpp` - Tissue selection
- `src/genetics/interactions/FeedingInteraction.cpp` - Consumption logic
- `include/genetics/organisms/Plant.hpp` - Mutable resource access

**Tasks:**
1. Add `PlantTissue` enum and properties (2 hours)
2. Implement `selectTargetTissue()` (3 hours)
3. Implement `consumePlantResources()` (3 hours)
4. Update `attemptToEatPlant()` to use new system (2 hours)
5. Integration tests for feeding depletion (2 hours)

**Acceptance Criteria:**
- [ ] Feeding reduces plant resources
- [ ] Different tissues provide different nutrition
- [ ] Creature adaptations affect tissue access
- [ ] Heavy grazing can kill plants (if roots depleted)

### Phase 3: Regrowth System (10 hours)

**Goal:** Plants regenerate resources over time

**Files Modified:**
- `src/genetics/organisms/Plant.cpp` - Regeneration in `update()`
- `include/genetics/organisms/Plant.hpp` - Allocation strategy

**Tasks:**
1. Implement `PlantAllocationStrategy` (2 hours)
2. Add `regenerateResources()` method (3 hours)
3. Implement root-based regrowth (2 hours)
4. Add seasonal fruiting logic (2 hours)
5. Balance testing and tuning (1 hour)

**Acceptance Criteria:**
- [ ] Depleted resources regenerate over time
- [ ] Regeneration rate affected by photosynthesis gene
- [ ] Root investment enables faster regrowth
- [ ] Fruiting is seasonal for fruit plants

### Phase 4: Creature Adaptation Requirements (8 hours)

**Goal:** Creatures need proper adaptations to access plant tissues

**Files Modified:**
- `src/genetics/interactions/FeedingInteraction.cpp` - Adaptation checks
- `include/genetics/defaults/UniversalGenes.hpp` - Document requirements

**Tasks:**
1. Implement adaptation threshold checks (3 hours)
2. Add efficiency penalties for missing adaptations (2 hours)
3. Update diet emergence calculation (2 hours)
4. Documentation and test coverage (1 hour)

**Acceptance Criteria:**
- [ ] Creatures without adaptations can only eat leaves
- [ ] Jaw strength required for nuts/bark
- [ ] Color vision helps find fruit
- [ ] Cellulose breakdown for tough vegetation

### Total Estimated Effort: 38 hours

---

## Code Changes Required

### Plant.hpp Additions

```cpp
// In class Plant:

// Resource Management
PlantResources resources_;
PlantAllocationStrategy allocation_;

// New public methods
const PlantResources& getResources() const { return resources_; }
PlantResources& getResourcesMutable() { return resources_; }
void regenerateResources(float deltaTime);
float consumeResource(PlantTissue tissue, float amount);
bool canRegrow() const { return resources_.canRegrow(); }

// Resource capacity getters
float getRootStorageCapacity() const;
float getRegrowthMultiplier() const;
```

### FeedingInteraction.cpp Modifications

```cpp
// In attemptToEatPlant():

// After all validation passes, CONSUME resources
PlantTissue targetTissue = selectTargetTissue(creaturePhenotype, plant, creatureHunger);
float consumeAmount = 0.5f * getDigestionEfficiency(creaturePhenotype, plant);

// NEW: Actually deplete plant resources
result.resourcesConsumed = plant.consumeResource(targetTissue, consumeAmount);
result.tissueConsumed = targetTissue;

// Recalculate nutrition based on what was actually available
result.nutritionGained = result.resourcesConsumed * 
                          TISSUE_PROPERTIES.at(targetTissue).nutrition_multiplier *
                          digestionEfficiency;

// Check if plant died
result.plantKilled = !plant.isAlive();
result.plantCanRegrow = plant.canRegrow();
```

### Plant.cpp Modifications

```cpp
// In Plant::update():

void Plant::update() {
    // Existing age/death checks...
    
    // NEW: Regenerate resources
    regenerateResources(1.0f);  // deltaTime = 1 tick
    
    // NEW: Check for regrowth from root system
    if (health_ <= 0.0f && canRegrow()) {
        // Plant was "killed" but has root reserves
        initiateRegrowth();
    }
    
    // Existing growth code...
}

void Plant::initiateRegrowth() {
    // Reset above-ground size based on root reserves
    float regrowthPotential = resources_.root_resources / resources_.max_root_capacity;
    current_size_ = getMaxSize() * 0.1f * regrowthPotential;
    health_ = 0.3f;  // Weakened but alive
    mature_ = false;  // Needs to grow again
    
    // Consume root reserves for regrowth
    resources_.root_resources *= 0.5f;
}
```

---

## Testing Strategy

### Unit Tests

```cpp
// test_plant_resources.cpp

TEST(PlantResources, InitializesCorrectly) {
    Plant plant(0, 0, registry);
    EXPECT_NEAR(plant.getResources().leaf_resources, 1.0f, 0.1f);
    EXPECT_NEAR(plant.getResources().root_resources, 1.0f, 0.1f);
}

TEST(PlantResources, DepletesWhenConsumed) {
    Plant plant(0, 0, registry);
    float initial = plant.getResources().leaf_resources;
    
    plant.consumeResource(PlantTissue::Leaf, 0.5f);
    
    EXPECT_LT(plant.getResources().leaf_resources, initial);
}

TEST(PlantResources, RegeneratesOverTime) {
    Plant plant(0, 0, registry);
    plant.consumeResource(PlantTissue::Leaf, 0.8f);
    float depleted = plant.getResources().leaf_resources;
    
    for (int i = 0; i < 100; i++) {
        plant.update();
    }
    
    EXPECT_GT(plant.getResources().leaf_resources, depleted);
}

TEST(PlantResources, DiesWhenRootsDepleted) {
    Plant plant(0, 0, registry);
    plant.consumeResource(PlantTissue::Root, 0.95f);
    
    EXPECT_FALSE(plant.isAlive());
}

TEST(PlantResources, RegrowsFromRoots) {
    Plant plant(0, 0, registry);
    // Severely damage but leave roots intact
    plant.consumeResource(PlantTissue::Leaf, 1.0f);
    plant.consumeResource(PlantTissue::Stem, 1.0f);
    plant.takeDamage(0.9f);
    
    EXPECT_TRUE(plant.canRegrow());
    
    for (int i = 0; i < 500; i++) {
        plant.update();
    }
    
    EXPECT_TRUE(plant.isAlive());
    EXPECT_GT(plant.getCurrentSize(), 0.0f);
}
```

### Integration Tests

```cpp
// test_feeding_depletion.cpp

TEST(FeedingDepletion, HerbivoreDepletesPlant) {
    Plant plant(0, 0, registry);
    Creature herbivore = CreatureFactory::createHerbivore(registry);
    FeedingInteraction feeding;
    
    float initialResources = plant.getResources().getTotalResources();
    
    // Feed multiple times
    for (int i = 0; i < 10; i++) {
        feeding.attemptToEatPlant(herbivore.getPhenotype(), plant, 0.8f);
    }
    
    EXPECT_LT(plant.getResources().getTotalResources(), initialResources);
}

TEST(FeedingDepletion, PlantRecoversBetweenFeedings) {
    Plant plant(0, 0, registry);
    Creature herbivore = CreatureFactory::createHerbivore(registry);
    FeedingInteraction feeding;
    
    feeding.attemptToEatPlant(herbivore.getPhenotype(), plant, 0.8f);
    float afterFeeding = plant.getResources().leaf_resources;
    
    // Let plant recover
    for (int i = 0; i < 50; i++) {
        plant.update();
    }
    
    EXPECT_GT(plant.getResources().leaf_resources, afterFeeding);
}
```

### Ecosystem Balance Tests

```cpp
// test_ecosystem_balance.cpp

TEST(EcosystemBalance, HerbivorePopulationStabilizes) {
    World world(100, 100);
    world.addPlants(50);
    world.addCreatures(CreatureFactory::createHerbivore, 10);
    
    std::vector<int> populationHistory;
    for (int day = 0; day < 100; day++) {
        world.simulateDay();
        populationHistory.push_back(world.getHerbivoreCount());
    }
    
    // Population should stabilize, not grow unbounded
    int finalPop = populationHistory.back();
    int midPop = populationHistory[50];
    
    EXPECT_LT(finalPop, 200);  // Shouldn't explode
    EXPECT_GT(finalPop, 0);    // Shouldn't crash to 0
}
```

---

## Balance Considerations

### Parameters Requiring Tuning

| Parameter | Initial Value | Expected Range | Tuning Goal |
|-----------|--------------|----------------|-------------|
| `PHOTOSYNTHESIS_BASE_RATE` | 0.1 energy/tick | 0.05 - 0.2 | Plants grow but not instantly |
| `LEAF_REGEN_RATE` | 0.01/tick | 0.005 - 0.02 | Full recovery in ~100-200 ticks |
| `ROOT_REGEN_RATE` | 0.001/tick | 0.0005 - 0.002 | Full recovery in ~500-2000 ticks |
| `DEFENSE_MAINTENANCE_RATE` | 0.02/tick | 0.01 - 0.05 | Defense has meaningful cost |
| `ROOT_SURVIVAL_THRESHOLD` | 0.1 (10%) | 0.05 - 0.2 | Determines regrowth possibility |

### Expected Emergent Behaviors

1. **Herbivore-Plant Cycles**: Herbivore populations should fluctuate with plant availability
2. **Overgrazing Collapse**: Too many herbivores in one area kill plants, causing herbivore die-off
3. **Plant Defense Evolution**: Under heavy grazing, toxic/thorny plants should increase
4. **Niche Differentiation**: Different herbivore types specialize on different tissues
5. **Root Investment Value**: Plants in high-grazing areas evolve deeper roots

### Warning Signs (Imbalanced System)

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| All plants die quickly | Regeneration too slow | Increase `*_REGEN_RATE` |
| Herbivores always starving | Nutrition extraction too low | Increase tissue nutrition multipliers |
| No plant diversity | One strategy dominates | Adjust trade-off costs |
| Herbivore explosion | Regeneration too fast | Decrease `*_REGEN_RATE` |
| No regrowth happens | Root threshold too high | Lower `ROOT_SURVIVAL_THRESHOLD` |

---

## Estimated Effort Summary

| Phase | Hours | Dependencies |
|-------|-------|--------------|
| Phase 1: Core Resource Tracking | 8 | None |
| Phase 2: Feeding Integration | 12 | Phase 1 |
| Phase 3: Regrowth System | 10 | Phase 1 |
| Phase 4: Creature Adaptation Requirements | 8 | Phase 2 |
| **Total** | **38 hours** | Sequential |

### Suggested Implementation Order

```
Week 1: Phase 1 (Core Resources)
        └── Foundation for everything else
        
Week 2: Phase 2 (Feeding Integration)
        └── The critical fix for ecosystem balance
        
Week 3: Phase 3 (Regrowth System)
        └── Plants can recover, sustaining ecosystem
        
Week 4: Phase 4 (Adaptations)
        └── Creature specialization pressure
```

---

## See Also

**Design Documents:**
- [[resource-allocation]] - Energy budget framework
- [[../technical/design/coevolution]] - Creature-plant dynamics
- [[../technical/design/propagation]] - Seed dispersal strategies

**Future Enhancements:**
- [[propagule-genes]] - Unified fruit/seed gene system

**Technical Reference:**
- [[../technical/reference/genes]] - Complete gene catalog
- [[../technical/reference/api/organisms]] - Organism API

---

**Document Purpose:** Implementation plan for fixing the critical "Unlimited Plant Food" issue through a biologically-inspired resource depletion and regeneration system.

**Next Steps:**
1. Review this design with team/users
2. Begin Phase 1 implementation
3. Iterate on balance parameters through testing
