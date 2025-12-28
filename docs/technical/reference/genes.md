---
title: Gene Reference - Complete Gene Catalog
created: 2025-12-24
updated: 2025-12-25
status: complete
audience: developer
type: reference
tags: [genetics, genes, reference, catalog]
---

# Gene Reference - Complete Gene Catalog

**Version:** 3.0 (Combat & Classification Complete)
**Date:** 2025-12-25
**Total Genes:** 88 active (89 total, 1 deprecated)
**Test Coverage:** 97 tests passing

---

## Quick Reference

This document provides a single-source catalog of ALL genes in the EcoSim genetics system. Each gene is listed with its key properties for quick lookup.

**Legend:**
- **Chromosome**: Which of the 8 chromosomes contains this gene
- **Range**: [min, max] valid values for gene expression
- **Creep**: Mutation variation amount
- **Dominance**: Complete, Incomplete, Codominant, Overdominant
- **Source**: Which document introduced/defines this gene

---

## 1. UNIVERSAL GENES (7 genes)

Genes that all organisms express at moderate to high levels.

### 1.1 `lifespan`
- **Chromosome:** Lifespan
- **Range:** [100.0, 10000.0] ticks
- **Creep:** 100.0
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Maximum age before natural death
- **Effects:** Longer life = more reproduction opportunities but higher total energy cost

### 1.2 `max_size`
- **Chromosome:** Morphology
- **Range:** [0.5, 10.0] units
- **Creep:** 0.2
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Maximum body/plant size
- **Effects:** Size affects visibility, resource consumption, defense
- **Cost:** Superlinear scaling (1.5x) - larger bodies disproportionately expensive

### 1.3 `metabolism_rate`
- **Chromosome:** Metabolism
- **Range:** [0.001, 0.02] energy/tick
- **Creep:** 0.001
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Base metabolic rate
- **Effects:** Energy consumption speed, activity level

### 1.4 `color_hue`
- **Chromosome:** Morphology
- **Range:** [0.0, 360.0] degrees
- **Creep:** 10.0
- **Dominance:** Codominant
- **Source:** UniversalGenes, architecture.md
- **Description:** Visual appearance (HSV hue)
- **Effects:** Visibility to predators, mate attraction, camouflage

### 1.5 `temp_tolerance_low`
- **Chromosome:** Environmental
- **Range:** [-20.0, 20.0] °C
- **Creep:** 2.0
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Minimum survivable temperature
- **Effects:** Habitat range, cold adaptation cost

### 1.6 `temp_tolerance_high`
- **Chromosome:** Environmental
- **Range:** [20.0, 50.0] °C
- **Creep:** 2.0
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Maximum survivable temperature
- **Effects:** Habitat range, heat adaptation cost

### 1.7 `hardiness`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Overdominant
- **Source:** UniversalGenes, architecture.md
- **Description:** Overall damage resistance and stress tolerance
- **Effects:** Health damage reduction, environmental stress resistance
- **Note:** Overdominant = heterozygous advantage (hybrid vigor)

---

## 2. MOBILITY GENES (5 genes)

Genes for movement and navigation. High expression in creatures, dormant (0.1) in plants.

### 2.1 `locomotion`
- **Chromosome:** Locomotion
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Movement speed capability
- **Effects:** Tiles per tick, foraging range, escape ability
- **Expression:** Creatures=0.5-0.9, Plants=0.0-0.1
- **Cost:** High (1.5/day at full expression)

### 2.2 `sight_range`
- **Chromosome:** Sensory
- **Range:** [5.0, 200.0] tiles
- **Creep:** 5.0
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Visual perception distance
- **Effects:** Prey/predator detection, navigation
- **Cost:** High (1.2/day) - large retinas + processing

### 2.3 `navigation_ability`
- **Chromosome:** Sensory
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Pathfinding and spatial intelligence
- **Effects:** Movement efficiency, territory memory
- **Cost:** Moderate (1.0/day) - brain tissue

### 2.4 `flee_threshold`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Complete
- **Source:** UniversalGenes, architecture.md
- **Description:** Danger response sensitivity
- **Effects:** Predator avoidance, survivability
- **Cost:** Low (0.3/day) - reflexive system

### 2.5 `pursue_threshold`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Complete
- **Source:** UniversalGenes, architecture.md
- **Description:** Prey/mate seeking aggressiveness
- **Effects:** Hunting success, mate finding
- **Cost:** Low (0.3/day)

---

## 3. AUTOTROPHY GENES (5 genes)

Genes for photosynthesis and nutrient absorption. High expression in plants, dormant in creatures.

### 3.1 `photosynthesis`
- **Chromosome:** Metabolism
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Light to energy conversion efficiency
- **Effects:** Energy income from sunlight
- **Expression:** Plants=0.7-0.9, Creatures=0.0-0.1
- **Cost:** Moderate (0.6/day) - chloroplast maintenance

### 3.2 `root_depth`
- **Chromosome:** Morphology
- **Range:** [0.0, 10.0] units
- **Creep:** 0.5
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Root system depth for nutrient/water absorption
- **Effects:** Drought resistance, nutrient access, stability
- **Cost:** High (0.8/day) - structural tissue

### 3.3 `water_storage`
- **Chromosome:** Metabolism
- **Range:** [0.0, 100.0] units
- **Creep:** 5.0
- **Dominance:** Overdominant
- **Source:** UniversalGenes, architecture.md
- **Description:** Water storage capacity (succulence)
- **Effects:** Drought survival time
- **Cost:** Moderate (0.5/day)

### 3.4 `light_requirement`
- **Chromosome:** Environmental
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Minimum light level for survival
- **Effects:** Shade tolerance, habitat range
- **Cost:** Low (0.3/day)

### 3.5 `water_requirement`
- **Chromosome:** Environmental
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Minimum water level for survival
- **Effects:** Drought tolerance, habitat range
- **Cost:** Low (0.3/day)

---

## 4. HETEROTROPHY GENES - ORIGINAL (5 genes, 1 deprecated)

Original genes for consuming other organisms.

### 4.1 `hunt_instinct`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Complete
- **Source:** UniversalGenes, architecture.md
- **Description:** Predatory behavior drive
- **Effects:** Prey pursuit, attack probability
- **Cost:** Moderate (0.6/day)

### 4.2 `hunger_threshold`
- **Chromosome:** Metabolism
- **Range:** [0.0, 10.0] energy units
- **Creep:** 0.1
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Energy level that triggers foraging
- **Effects:** Foraging frequency, risk-taking

### 4.3 ~~`diet_type`~~ **DEPRECATED**
- **Chromosome:** ~~Behavior~~
- **Range:** ~~[0.0, 3.0]~~ (herbivore=0, omnivore=1, scavenger=2, predator=3)
- **Status:** ⚠️ **DEPRECATED** - Use `getEmergentDiet()` calculation
- **Source:** UniversalGenes (to be removed)
- **Replacement:** Calculate from plant_digestion_efficiency + meat_digestion_efficiency
- **Migration:** Provide adapter for backward compatibility

### 4.4 `digestive_efficiency`
- **Chromosome:** Metabolism
- **Range:** [0.2, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Generic food processing efficiency
- **Effects:** Energy extraction from food
- **Note:** Superseded by specific plant/meat digestion genes

### 4.5 `scent_detection`
- **Chromosome:** Sensory
- **Range:** [0.0, 50.0] tiles
- **Creep:** 2.0
- **Dominance:** Incomplete
- **Source:** UniversalGenes, creature-plant-coevolution.md
- **Description:** Olfactory detection range for food/mates
- **Effects:** Food finding beyond visual range
- **Cost:** Moderate (0.8/day)

### 4.6 `social_tendency`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Gregarious vs solitary preference
- **Effects:** Group formation, cooperative hunting

---

## 5. HETEROTROPHY GENES - COEVOLUTION (7 genes)

New genes for plant-creature interactions.

### 5.1 `plant_digestion_efficiency`
- **Chromosome:** Metabolism
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Plant cellulose/fiber digestion capability
- **Effects:** Nutrition from plants (+), meat digestion (−0.5 inverse)
- **Trade-off:** Negative pleiotropy with meat_digestion
- **Cost:** High (0.8/day at 1.0, scales 1.2x)

### 5.2 `meat_digestion_efficiency`
- **Chromosome:** Metabolism
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Animal protein/fat digestion capability
- **Effects:** Nutrition from meat (+), plant digestion (−0.5 inverse)
- **Trade-off:** Negative pleiotropy with plant_digestion
- **Cost:** High (0.8/day at 1.0, scales 1.2x)

### 5.3 `cellulose_breakdown`
- **Chromosome:** Metabolism
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Fermentation/gut bacteria for tough plants
- **Effects:** Grass/bark digestion (+), digestion time (+)
- **Cost:** Very high (1.2/day at 1.0, scales 1.5x) - gut symbionts expensive

### 5.4 `color_vision`
- **Chromosome:** Sensory
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Color acuity for detecting ripe fruit
- **Effects:** Fruit detection range (+), energy cost (+)
- **Trade-off:** Vision specialization reduces motion detection

### 5.5 `sweetness_preference`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Attraction to sweet/ripe fruit
- **Effects:** Fruit foraging priority, mutualism with plants

### 5.6 `toxin_tolerance`
- **Chromosome:** Metabolism
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Passive resistance to plant toxins
- **Effects:** Toxin damage reduction
- **Trade-off:** Baseline resistance without enzyme cost

### 5.7 `toxin_metabolism`
- **Chromosome:** Metabolism
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Active detoxification enzyme system (P450)
- **Effects:** Toxin damage reduction, energy cost (+0.2)
- **Cost:** Very high (1.0/day at 1.0, scales 1.3x) - enzymes expensive

---

## 6. MORPHOLOGY GENES - DEFENSE (5 genes)

Physical structures for defense/offense.

### 6.1 `fur_density`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Coat texture and density
- **Effects:** Seed attachment (+), insulation (+), parasites (+)
- **Trade-off:** Burr dispersal vs hygiene

### 6.2 `hide_thickness`
- **Chromosome:** Morphology
- **Range:** [0.0, 5.0] mm
- **Creep:** 0.2
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Skin/hide toughness
- **Effects:** Thorn damage (−), tactile sensitivity (−), heat dissipation (−)
- **Trade-off:** Protection vs sensory/cooling

### 6.3 `jaw_strength`
- **Chromosome:** Morphology
- **Range:** [0.0, 10.0] force units
- **Creep:** 0.5
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Bite force for cracking nuts/seeds
- **Effects:** Nut access (+), combat damage (+), seed destruction (+)
- **Pleiotropy:** Affects feeding AND fighting

### 6.4 `mucus_protection`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Stomach/mouth protective lining
- **Effects:** Irritant damage reduction, acid digestion support

### 6.5 `pain_sensitivity`
- **Chromosome:** Sensory
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Pain detection threshold
- **Effects:** Pain threshold (inverse), danger avoidance (+)
- **Trade-off:** Low = eats thorny plants, High = avoids injury

---

## 7. BEHAVIOR GENES - SEED INTERACTION (5 genes)

Behavioral genes affecting seed dispersal.

### 7.1 `gut_transit_time`
- **Chromosome:** Metabolism
- **Range:** [0.5, 24.0] hours
- **Creep:** 1.0
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Time for food to pass through digestive system
- **Effects:** Seed passage time, nutrition extraction (inverse)
- **Optimal:** 4-12 hours for seed scarification

### 7.2 `seed_destruction_rate`
- **Chromosome:** Metabolism
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Seed crushing/digestion in gut
- **Effects:** Seed nutrition (+), dispersal effectiveness (−)
- **Trade-off:** Extract calories vs benefit plant mutualism

### 7.3 `caching_instinct`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Food hoarding/burying behavior
- **Effects:** Food storage (+), foraging time (−)
- **Enables:** Nut dispersal via forgotten caches

### 7.4 `spatial_memory`
- **Chromosome:** Sensory
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Remember food/cache locations
- **Effects:** Return to fruiting trees, cache recovery
- **Trade-off:** Energy cost for memory maintenance

### 7.5 `grooming_frequency`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** creature-plant-coevolution.md
- **Description:** Burr/parasite removal rate
- **Effects:** Burr removal (+), parasite load (−), seed dispersal (−)
- **Trade-off:** Hygiene vs plant dispersal service

---

## 8. PLANT REPRODUCTION GENES (9 genes)

Physical seed properties that create emergent dispersal strategies.

### 8.1 `offspring_count`
- **Chromosome:** Reproduction
- **Range:** [1.0, 20.0] seeds/pups
- **Creep:** 1.0
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Number of offspring per reproductive cycle
- **Effects:** Reproductive output, energy cost per offspring (inverse)
- **Trade-off:** r-strategy (many) vs K-strategy (few)

### 8.2 `spread_distance`
- **Chromosome:** Reproduction
- **Range:** [1.0, 50.0] tiles
- **Creep:** 2.0
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Base dispersal range
- **Effects:** Colonization range, genetic mixing
- **Note:** Modified by emergent dispersal calculation

### 8.3 `mate_selectivity`
- **Chromosome:** Reproduction
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes, architecture.md
- **Description:** Partner fitness requirement threshold
- **Effects:** Offspring quality vs reproduction frequency

### 8.4 `seed_mass`
- **Chromosome:** Reproduction
- **Range:** [1.0, 100.0] mg
- **Creep:** 2.0
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Individual seed weight
- **Effects:** Dispersal distance (inverse), germination success (+), seedling vigor (+)
- **Emergent:** Low mass = wind dispersal, high mass = gravity/nut

### 8.5 `seed_aerodynamics`
- **Chromosome:** Reproduction
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Surface-to-mass ratio (wings/parachutes)
- **Effects:** Wind dispersal efficiency (+), construction cost (+)
- **Emergent:** High value + low mass = wind strategy

### 8.6 `seed_hook_strength`
- **Chromosome:** Reproduction
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Barbs/hooks for fur attachment
- **Effects:** Attachment probability (+), seed count (−0.2)
- **Emergent:** High value = burr dispersal strategy

### 8.7 `fruit_appeal`
- **Chromosome:** Reproduction
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Attractiveness to frugivores (color/sugar/aroma)
- **Effects:** Animal attraction (+), energy cost (+0.5), seed count (−0.4)
- **Emergent:** High appeal + durable coat = animal fruit dispersal

### 8.8 `seed_coat_durability`
- **Chromosome:** Reproduction
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Seed coat toughness to survive digestion
- **Effects:** Digestion survival (+), germination delay (+0.3)
- **Emergent:** Required for animal fruit dispersal (>0.5)

### 8.9 `explosive_pod_force`
- **Chromosome:** Reproduction
- **Range:** [0.0, 15.0] Newtons
- **Creep:** 0.5
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Mechanical tension force in seed pods
- **Effects:** Launch distance (+), pod construction cost (+0.4)
- **Emergent:** High force (>5.0) = explosive dispersal

### 8.10 `runner_production`
- **Chromosome:** Reproduction
- **Range:** [0.0, 2.0] runners/season
- **Creep:** 0.1
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Vegetative clonal spreading rate
- **Effects:** Clonal spread (+), sexual seed count (−0.5)
- **Emergent:** High value (>0.7) = vegetative strategy

### 8.11 `seed_energy_reserve`
- **Chromosome:** Reproduction
- **Range:** [5.0, 100.0] energy units
- **Creep:** 3.0
- **Dominance:** Incomplete
- **Source:** plant-propagation-strategies.md
- **Description:** Energy invested per seed
- **Effects:** Germination success (+), seedling survival (+), seed count (−0.7)
- **Trade-off:** r-strategy (low) vs K-strategy (high)

---

## 9. OLFACTORY GENES (4 genes)

Genes controlling scent production and detection.

### 9.1 `scent_production`

- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Rate of general scent emission
- **Effects:** Territory marking, mate attraction, predator detection
- **Plant-Specific:** Controls FOOD_TRAIL emission intensity

#### Plant SCENT_PRODUCTION Values

| Plant Type | Default Range | Effect |
|------------|---------------|--------|
| Berry Bush | 0.3 - 0.5 | Strong food scent attracts frugivores |
| Thorn Bush | 0.1 - 0.2 | Moderate scent despite defenses |
| Oak Tree | 0.02 - 0.08 | Minimal scent |
| Grass | 0.0 - 0.01 | No scent (below 0.01 emission threshold) |

> [!NOTE]
> Plants with `scent_production < 0.01` do not emit FOOD_TRAIL scents.
> Creatures must use visual detection (color_vision) to find them.

### 9.2 `scent_signature_variance`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** How much genetic signature varies from genotype
- **Effects:** Individual recognition, kin detection

### 9.3 `olfactory_acuity`
- **Chromosome:** Sensory
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Ability to distinguish different scent types
- **Effects:** Scent detection range, type discrimination

### 9.4 `scent_masking`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reduce own scent detectability
- **Effects:** Predator avoidance, hunting stealth

---

## 10. HEALTH & HEALING GENES (3 genes)

Genes controlling wound recovery and injury tolerance.

### 10.1 `regeneration_rate`
- **Chromosome:** Metabolism
- **Range:** [0.0, 2.0]
- **Creep:** 0.1
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Multiplier on base healing rate
- **Effects:** Higher values = faster wound recovery, but costs more energy

### 10.2 `wound_tolerance`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reduces behavioral penalty from wounds
- **Effects:** High tolerance = less speed/behavior penalty when injured

### 10.3 `bleeding_resistance`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reduces bleed damage over time
- **Effects:** High resistance = wounds stop bleeding faster

---

## 11. COMBAT WEAPON GENES (13 genes)

Physical weapon morphology affecting combat damage types.

### 11.1 `teeth_sharpness`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Sharp→pierce, dull→blunt for teeth
- **Effects:** Determines pierce vs blunt damage ratio for bite attacks

### 11.2 `teeth_serration`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Adds slash damage component to teeth
- **Effects:** Higher serration = more slashing damage on bites

### 11.3 `teeth_size`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Scales base bite damage
- **Effects:** Larger teeth = more total bite damage

### 11.4 `claw_length`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reach & damage scaling for claws
- **Effects:** Longer claws = more damage potential

### 11.5 `claw_curvature`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Curved→pierce, straight→slash
- **Effects:** High curvature = piercing, low = slashing

### 11.6 `claw_sharpness`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Overall cutting ability of claws
- **Effects:** Sharper claws = more damage

### 11.7 `horn_length`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reach & charge damage for horns
- **Effects:** Longer horns = more damage on charges

### 11.8 `horn_pointiness`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Pointed→pierce, broad→blunt
- **Effects:** Determines pierce vs blunt ratio for horn attacks

### 11.9 `horn_spread`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Narrow→gore, wide→sweep
- **Effects:** Wide spread = area attacks, narrow = focused

### 11.10 `tail_length`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reach & whip damage for tail
- **Effects:** Longer tail = more reach and damage

### 11.11 `tail_mass`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Heavy→blunt (club), light→slash (whip)
- **Effects:** Determines damage type for tail attacks

### 11.12 `tail_spines`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Adds pierce component to tail
- **Effects:** Spiny tails deal piercing damage

### 11.13 `body_spines`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Counter-damage pierce (porcupine-like)
- **Effects:** Attackers take pierce damage when attacking

---

## 12. COMBAT DEFENSE GENES (2 genes)

Additional defensive traits.

### 12.1 `scale_coverage`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reduces slashing damage
- **Effects:** Scales deflect and blunt cutting attacks
- **RPS:** Counters Slashing damage type

### 12.2 `fat_layer_thickness`
- **Chromosome:** Morphology
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Reduces blunt damage
- **Effects:** Fat absorbs and disperses impact energy
- **RPS:** Counters Blunt damage type

---

## 13. COMBAT BEHAVIOR GENES (4 genes)

Combat behavioral traits.

### 13.1 `combat_aggression`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Complete
- **Source:** UniversalGenes
- **Description:** Willingness to initiate/continue combat
- **Effects:** Higher aggression = more likely to attack

### 13.2 `retreat_threshold`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Complete
- **Source:** UniversalGenes
- **Description:** Health % at which creature flees
- **Effects:** Higher threshold = flees sooner

### 13.3 `territorial_aggression`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Aggression toward same-species
- **Effects:** High = fights conspecifics for territory

### 13.4 `pack_coordination`
- **Chromosome:** Behavior
- **Range:** [0.0, 1.0]
- **Creep:** 0.05
- **Dominance:** Incomplete
- **Source:** UniversalGenes
- **Description:** Future: pack hunting behavior
- **Effects:** Enables coordinated group attacks

---

## 14. Gene Categories Summary

| Category | Count | Primary Users | Expression in Others |
|----------|-------|---------------|---------------------|
| **Universal** | 7 | All | High (0.5-0.9) |
| **Mobility** | 5 | Creatures | Dormant in plants (0.0-0.1) |
| **Autotrophy** | 5 | Plants | Dormant in creatures (0.0-0.1) |
| **Heterotrophy (original)** | 6 | Creatures | Dormant in plants |
| **Heterotrophy (coevolution)** | 7 | Creatures | Some in carnivorous plants |
| **Morphology (digestive)** | 9 | Creatures | Low in plants |
| **Olfactory** | 4 | All | Varies |
| **Behavior (interaction)** | 5 | Creatures | Not in plants |
| **Health/Healing** | 3 | Creatures | Low in plants |
| **Combat Weapons** | 13 | Creatures | Not in plants |
| **Combat Defense** | 2 | Creatures | Not in plants |
| **Combat Behavior** | 4 | Creatures | Not in plants |
| **Seed Interaction** | 2 | Creatures | Not in plants |
| **Plant Defense** | 6 | Plants | Low in creatures |
| **Reproduction** | 6 | All | Varies by strategy |
| **Seed Propagation** | 5 | Plants | Low in creatures |
| **TOTAL** | **89** | - | - |
| **Active** | 88 | - | - |
| **Deprecated** | 1 (`diet_type`) | - | - |

---

## 15. Emergent Properties Reference

These are NOT genes but computed properties from gene combinations.

### 15.1 Dispersal Strategy (Plants)

**Calculated from:** seed_mass, seed_aerodynamics, seed_hook_strength, fruit_appeal, seed_coat_durability, explosive_pod_force, runner_production

**Possible Values:**
- `GRAVITY`: Heavy seeds, no special adaptations
- `WIND`: Light seeds + high aerodynamics
- `ANIMAL_FRUIT`: High appeal + durable coat
- `ANIMAL_BURR`: High hook strength
- `EXPLOSIVE`: High pod force
- `VEGETATIVE`: High runner production
- `MIXED`: Multiple strategies active

### 15.2 Diet Type (Creatures)

**Calculated from:** plant_digestion_efficiency, meat_digestion_efficiency, cellulose_breakdown, sweetness_preference

**Possible Values:**
- `HERBIVORE`: High plant digestion + cellulose
- `FRUGIVORE`: High plant + sweetness, low cellulose
- `CARNIVORE`: High meat, low plant
- `OMNIVORE`: Balanced capabilities
- `SCAVENGER`: Low capabilities overall

**Formula:**
```cpp
if (plantDig > 0.6 && sweetness > 0.7 && cellulose < 0.3) return FRUGIVORE;
if (plantDig > 0.7 && cellulose > 0.5) return HERBIVORE;
if (meatDig > 0.7 && plantDig < 0.3) return CARNIVORE;
if (plantDig > 0.4 && meatDig > 0.4) return OMNIVORE;
return SCAVENGER;
```

---

## 16. Quick Lookup by Function

### To Create Herbivore:
High: `plant_digestion_efficiency`, `cellulose_breakdown`, `tooth_grinding`, `gut_length`  
Low: `meat_digestion_efficiency`, `hunt_instinct`, `tooth_sharpness`

### To Create Carnivore:
High: `meat_digestion_efficiency`, `hunt_instinct`, `tooth_sharpness`, `jaw_speed`  
Low: `plant_digestion_efficiency`, `cellulose_breakdown`, `tooth_grinding`

### To Create Frugivore:
High: `plant_digestion_efficiency`, `color_vision`, `sweetness_preference`  
Low: `seed_destruction_rate`, `cellulose_breakdown`, `meat_digestion_efficiency`

### To Create Wind-Dispersed Plant:
High: `seed_aerodynamics`, `offspring_count`  
Low: `seed_mass` (<10.0), `fruit_appeal`, `seed_hook_strength`

### To Create Fruit-Dispersed Plant:
High: `fruit_appeal`, `seed_coat_durability`, `seed_energy_reserve`  
Low: `seed_hook_strength`, `explosive_pod_force`

### To Create Defended Plant:
High: `defense_level`, `toxicity`, `structural_integrity`, `hardiness`  
Low: `fruit_production_rate`, `growth_rate`

### To Create Fast-Growing Colonizer:
High: `plant_growth_rate`, `offspring_count`, `spread_distance`  
Low: `defense_level`, `toxicity`, `lifespan`

---

## 17. Maintenance Cost Reference

Estimated daily maintenance costs at full expression (1.0):

### High Cost (>1.0 energy/day)
- `locomotion`: 1.5
- `toxin_metabolism`: 1.0
- `sight_range`: 1.2
- `navigation_ability`: 1.0
- `cellulose_breakdown`: 1.2
- `plant_growth_rate`: 1.0
- `toxicity`: 1.5

### Moderate Cost (0.5-1.0 energy/day)
- `max_size`: 1.0 (scales 1.5x with size)
- `hardiness`: 0.8
- `root_depth`: 0.8
- `plant_digestion_efficiency`: 0.8
- `meat_digestion_efficiency`: 0.8
- `hunt_instinct`: 0.6
- `photosynthesis`: 0.6

### Low Cost (<0.5 energy/day)
- `color_hue`: 0.1
- `temp_tolerance_low`: 0.4
- `temp_tolerance_high`: 0.4
- `flee_threshold`: 0.3
- `pursue_threshold`: 0.3
- `metabolism_rate`: 0.3
- `hunger_threshold`: 0.2

**Note:** These are baseline values. Actual costs modified by expression level, age, and environment.

---

## See Also

**Core Documentation:**
- [[../core/01-architecture]] - System architecture overview
- [[../core/02-getting-started]] - Quick start tutorial
- [[../core/03-extending]] - Extension guide

**Reference:**
- [[quick-reference]] - Quick reference cheat sheet
- [[api/core-classes]] - Core API reference
- [[api/expression]] - Expression API
- [[api/organisms]] - Organism API

**Systems:**
- [[../systems/scent-system]] - Olfactory communication system

**Design:**
- [[../design/coevolution]] - Coevolution design rationale
- [[../design/propagation]] - Seed dispersal design
- [[../design/resource-allocation]] - Energy budget design

---

**Document Purpose:** Single source of truth for all gene data. Use this for quick lookups during implementation, testing, and debugging.

**Maintenance:** Update this document whenever genes are added, modified, or deprecated. Keep synchronized with code.
