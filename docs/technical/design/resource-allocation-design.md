---
title: Universal Resource Allocation Framework
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: designer
type: design
tags: [genetics, design, energy, metabolism, evolution]
---

# Universal Resource Allocation Framework

**Related Documents:**

- [[../core/01-architecture]] - System architecture
- [[coevolution-design]] - Creature-plant coevolutionary dynamics
- [[../../future/omnivore-trade-offs-design]] - Omnivore trade-offs design

---

## Core Principle: The Universal Energy Budget

> **In nature, every adaptation has a resource cost. An organism cannot be good at everything.**

All organisms are fundamentally energy transformation systems:
- **Input:** Acquire resources (photosynthesis, eating, absorption)
- **Storage:** Maintain energy reserves (fat, starch, ATP)
- **Expenditure:** Allocate limited energy to competing needs
- **Output:** Growth, reproduction, movement, defense

This framework creates a **universal energy budget** where every gene expression consumes resources, forcing organisms into specialization through natural selection rather than arbitrary caps.

### Design Philosophy: Emergence Over Prescription

This system avoids hard-coded restrictions. Instead, it provides:
1. **Simple rules:** Each gene has a maintenance cost based on expression level
2. **Natural constraints:** Total costs cannot exceed available energy
3. **Emergent specialization:** Selection pressure favors focused strategies
4. **Flexible niches:** Different environments favor different allocations

**Result:** Diverse life strategies emerge naturally from energy economics, not designer fiat.

---

## Part 1: Universal Energy Model

### 1.1 Organism Energy State

All organisms track energy through their lifecycle:

```cpp
struct OrganismState {
    float age_normalized = 0.0f;       // 0.0 = birth, 1.0 = max lifespan
    float energy_level = 1.0f;         // Current energy: 0.0 to 1.0
    float energy_capacity = 100.0f;    // Max energy storage
    float health = 1.0f;               // Physical condition: 0.0 to 1.0
    bool is_pregnant = false;
    bool is_sleeping = false;
};
```

**Energy flow per time step:**

```
Energy(t+1) = Energy(t) + Income - (Maintenance + Activity + Growth + Reproduction)
```

Where:
- **Income:** Resources acquired (photosynthesis, eating, absorbing)
- **Maintenance:** Gene expression costs (always paid)
- **Activity:** Voluntary expenditures (movement, hunting, defense)
- **Growth:** Building biomass (size increase)
- **Reproduction:** Creating offspring

### 1.2 Gene Maintenance Cost

Every gene has a **base maintenance cost** that scales with its expression level:

```cpp
struct GeneCost {
    float base_cost;           // Energy per time unit at 100% expression
    float expression_scaling;  // How cost scales with expression (usually 1.0)
    CostCategory category;     // What this gene maintains
};

enum class CostCategory {
    Morphological,   // Physical structures (size, organs, armor)
    Metabolic,       // Chemical systems (digestion, detoxification)
    Neural,          // Sensory and cognitive systems
    Locomotor,       // Movement capabilities
    Defensive,       // Protection mechanisms
    Reproductive     // Breeding systems
};
```

**Maintenance cost formula:**

```cpp
float calculateGeneMaintenance(const Gene& gene, const GeneCost& cost) const {
    float expression = gene.getNumericValue(dominance);
    
    // Cost scales with expression level (usually linear)
    float expressionFactor = std::pow(expression, cost.expression_scaling);
    
    return cost.base_cost * expressionFactor;
}
```

**Key insight:** Even dormant genes (expression = 0.1) cost something, but active genes (expression = 0.9) cost much more.

### 1.3 Total Maintenance Budget

The organism's maintenance cost is the sum of all gene maintenance:

```cpp
float Phenotype::calculateTotalMaintenance() const {
    float totalCost = 0.0f;
    
    // Iterate through all genes in genome
    for (const auto& chromosome : genome_->getChromosomes()) {
        for (const auto& gene : chromosome.getGenes()) {
            const auto& definition = registry_->getDefinition(gene.getId());
            const GeneCost& cost = getCostForGene(gene.getId());
            
            float geneCost = calculateGeneMaintenance(gene, cost, definition.getDominance());
            totalCost += geneCost;
        }
    }
    
    // Age modulation: juveniles and elderly have different costs
    totalCost *= getAgeMaintenanceFactor();
    
    // Environmental modulation: temperature affects metabolism
    totalCost *= getEnvironmentCostFactor();
    
    return totalCost;
}
```

**Natural ceiling emerges:** If total maintenance > income, organism starves. Evolution selects for balanced allocations.

---

## Part 2: Gene-Specific Costs

### 2.1 Universal Gene Costs

Genes all organisms share:

| Gene | Base Cost | Scaling | Rationale |
|------|-----------|---------|-----------|
| **LIFESPAN** | 0.5 / day | 1.0 | Longer-lived cells need more repair mechanisms |
| **MAX_SIZE** | 1.0 / day | 1.5 | Larger bodies need proportionally more maintenance |
| **METABOLISM_RATE** | 0.3 / day | 1.0 | Higher metabolism requires more enzyme production |
| **HARDINESS** | 0.8 / day | 1.2 | Stress resistance proteins are energetically costly |
| **TEMP_TOLERANCE_LOW** | 0.4 / day | 1.0 | Cold adaptation requires antifreeze proteins |
| **TEMP_TOLERANCE_HIGH** | 0.4 / day | 1.0 | Heat shock proteins are expensive |
| **COLOR_HUE** | 0.1 / day | 1.0 | Pigment production minimal cost |

**Design note:** Size has superlinear scaling (1.5) because maintaining a 2x larger body costs more than 2x energy.

### 2.2 Creature-Specific Costs

#### Mobility Genes (High Cost - Active Systems)

| Gene | Base Cost | Scaling | Trade-off |
|------|-----------|---------|-----------|
| **LOCOMOTION** | 1.5 / day | 1.3 | Fast movement = bigger muscles = high maintenance |
| **SIGHT_RANGE** | 1.2 / day | 1.0 | Long-range vision = large retinas + brain processing |
| **NAVIGATION_ABILITY** | 1.0 / day | 1.0 | Pathfinding requires spatial memory (brain tissue) |
| **FLEE_THRESHOLD** | 0.3 / day | 1.0 | Fast reflexes require alert nervous system |
| **PURSUE_THRESHOLD** | 0.3 / day | 1.0 | Hunting behavior needs sustained attention |

**Example emergence:**
- **Speed specialist:** High LOCOMOTION (1.5 cost) → Must eat frequently → Needs abundant food
- **Ambush predator:** Low LOCOMOTION (0.3 cost) → Can survive food scarcity → Patient hunter

#### Heterotrophy Genes (Moderate Cost - Digestive Systems)

| Gene | Base Cost | Scaling | Trade-off |
|------|-----------|---------|-----------|
| **HUNT_INSTINCT** | 0.6 / day | 1.0 | Predatory behavior requires vigilance |
| **HUNGER_THRESHOLD** | 0.2 / day | 1.0 | Hunger sensing is low cost |
| **DIGESTIVE_EFFICIENCY** (plant/meat) | 0.8 / day | 1.2 | Multiple enzyme systems are expensive |
| **DIET_TYPE** | 0.4 / day | 1.0 | Dietary flexibility has moderate cost |
| **CELLULOSE_BREAKDOWN** | 1.2 / day | 1.5 | Cellulose digestion requires gut symbionts |
| **TOXIN_METABOLISM** | 1.0 / day | 1.3 | Detoxification enzymes (P450s) are very costly |

**Example emergence:**
- **Pure herbivore:** HIGH plant digestion (0.96 cost) + HIGH cellulose (1.8 cost) = 2.76/day
- **Pure carnivore:** HIGH meat digestion (0.96 cost) + LOW cellulose (0.12 cost) = 1.08/day
- **Omnivore:** MEDIUM both (0.48 + 0.48) + MEDIUM cellulose (0.6) = 1.56/day

**Natural balance:** Omnivores have intermediate costs but lower efficiency (from negative pleiotropy). They survive when food is scarce but lose to specialists when abundant.

### 2.3 Plant-Specific Costs

#### Autotrophy Genes (Investment in Infrastructure)

| Gene | Base Cost | Scaling | Trade-off |
|------|-----------|---------|-----------|
| **PHOTOSYNTHESIS** | 0.6 / day | 1.0 | Chloroplast maintenance |
| **ROOT_DEPTH** | 0.8 / day | 1.3 | Deep roots = more structural tissue |
| **WATER_STORAGE** | 0.5 / day | 1.2 | Water storage organs (succulence) |
| **LIGHT_REQUIREMENT** | 0.3 / day | 1.0 | Light sensing mechanisms |
| **WATER_REQUIREMENT** | 0.3 / day | 1.0 | Water regulation systems |

#### Plant Defense Genes (Active vs Passive Strategies)

| Gene | Base Cost | Scaling | Trade-off |
|------|-----------|---------|-----------|
| **TOXIN_PRODUCTION** | 1.5 / day | 1.4 | Alkaloids/glycosides are metabolically expensive |
| **THORN_DENSITY** | 0.9 / day | 1.2 | Physical defenses use biomass |
| **SEED_COAT_THICKNESS** | 0.4 / day | 1.0 | Tougher seeds = less total seeds |
| **FRUIT_SWEETNESS** | 0.7 / day | 1.0 | Sugar investment in dispersal |
| **GROWTH_RATE** | 1.0 / day | 1.3 | Fast growth requires more nutrients |

**Example emergence:**
- **Toxin specialist:** HIGH toxins (2.1 cost) + LOW growth (0.1 cost) = 2.2/day → Slow-growing, well-defended
- **Fast grower:** HIGH growth (1.3 cost) + LOW toxins (0.15 cost) = 1.45/day → Quick colonizer, vulnerable
- **Physical defense:** HIGH thorns (1.08 cost) + MEDIUM growth (0.65 cost) = 1.73/day → Moderate strategy

---

## Part 3: Emergent Specialization Strategies

### 3.1 How Specialists Emerge

Selection pressure naturally creates specialists through energy economics:

```
Generation 0: Random distribution
- Generalist A: 15 genes at 0.5 expression → Total cost = 12.5/day
- Specialist B: 5 genes at 0.9, 10 genes at 0.1 → Total cost = 8.9/day

Environment provides: 10 energy/day

Result:
- Generalist A: 10 - 12.5 = -2.5 (STARVES)
- Specialist B: 10 - 8.9 = +1.1 (THRIVES)

Generation 10: Specialists dominate
```

**Emergence pattern:**
1. Random genomes created with varied gene expression
2. Generalists have high total costs
3. Generalists starve, specialists survive
4. Specialists reproduce, passing focused genes
5. Population converges to specialized archetypes

**No designer intervention needed!**

### 3.2 Creature Archetypes

Different cost allocations create distinct ecological roles:

#### Fast Predator

```
HIGH: LOCOMOTION (1.95), SIGHT_RANGE (1.2), PURSUIT (0.3), MEAT_DIGESTION (0.96)
LOW: CELLULOSE (0.12), TOXIN_TOL (0.1), HARDINESS (0.08)
TOTAL: ~4.7 / day

Strategy: High-speed hunter
Income: Must kill frequently (10+ energy/day from prey)
Niche: Abundant prey environments
Weakness: Starves quickly if hunting fails
```

#### Tank Herbivore

```
HIGH: MAX_SIZE (2.25), HARDINESS (0.96), CELLULOSE (1.8), PLANT_DIGESTION (0.96)
LOW: LOCOMOTION (0.15), SIGHT_RANGE (0.12), MEAT_DIGESTION (0.08)
TOTAL: ~6.3 / day

Strategy: Large, tough, slow grazer
Income: Constant low-quality food (8 energy/day from plants)
Niche: Abundant vegetation, high predation pressure
Weakness: Can't escape predators, needs constant food
```

#### Glass Cannon (High Risk)

```
HIGH: LOCOMOTION (1.95), HUNT_INSTINCT (0.6), TOXIN_METABOLISM (1.3)
LOW: HARDINESS (0.08), MAX_SIZE (0.1)
TOTAL: ~4.0 / day

Strategy: Fast, fragile, toxin-resistant scavenger
Income: Opportunistic (variable 5-15 energy/day)
Niche: Toxic carrion, edge habitats
Weakness: Dies to any damage, inconsistent food
```

#### Generalist Survivor

```
MEDIUM: All systems at 0.4-0.5 expression
TOTAL: ~7.5 / day

Strategy: Jack of all trades
Income: Flexible (6-12 energy/day from anything)
Niche: Variable/scarce environments
Weakness: Loses to specialists in stable environments
```

### 3.3 Plant Archetypes

#### Fast Colonizer

```
HIGH: GROWTH_RATE (1.3), OFFSPRING_COUNT (1.0), SPREAD_DISTANCE (0.8)
LOW: TOXIN_PRODUCTION (0.15), HARDINESS (0.08), LIFESPAN (0.05)
TOTAL: ~3.4 / day

Strategy: Rapid expansion, short lifespan
Income: 5 energy/day (needs good conditions)
Niche: Disturbed areas, open space
Weakness: Vulnerable to herbivores, outcompeted long-term
```

#### Defensive Fortress

```
HIGH: TOXIN_PRODUCTION (2.1), THORN_DENSITY (1.08), HARDINESS (0.96)
LOW: GROWTH_RATE (0.1), OFFSPRING_COUNT (0.1)
TOTAL: ~4.3 / day

Strategy: Slow, well-defended, persistent
Income: 6 energy/day (tolerates poor conditions)
Niche: High herbivore pressure, stable sites
Weakness: Slow to colonize, low reproduction
```

#### Mutualist Spreader

```
HIGH: FRUIT_SWEETNESS (0.7), NUTRIENT_VALUE (0.9), SEED_COAT (0.4)
MEDIUM: GROWTH_RATE (0.5), TOXIN_PRODUCTION (0.3)
TOTAL: ~2.8 / day

Strategy: Attract dispersers, protect seeds
Income: 4 energy/day (efficient photosynthesis)
Niche: Forested areas, creature-dispersed
Weakness: Dependent on animal populations
```

---

## Part 4: Integration with Existing Systems

### 4.1 Negative Pleiotropy (Already Designed)

**Existing mechanism:** Genes have inverse effects on opposing traits

```cpp
// Plant digestion reduces meat digestion
GeneDefinition plantDig(PLANT_DIGESTION_EFFICIENCY, ...);
plantDig.addEffect(EffectBinding("metabolism", "meat_nutrition", EffectType::Inverse, 0.5f));

// Meat digestion reduces plant digestion
GeneDefinition meatDig(MEAT_DIGESTION_EFFICIENCY, ...);
meatDig.addEffect(EffectBinding("metabolism", "plant_nutrition", EffectType::Inverse, 0.5f));
```

**Integration with energy costs:**
1. Negative pleiotropy reduces effectiveness of opposing systems
2. Energy costs punish maintaining both systems
3. **Double penalty:** Generalists pay costs for systems that don't work well together

**Result:** Synergistic constraints create strong specialization pressure.

### 4.2 Age Modulation

Energy costs vary across lifespan:

```cpp
float Phenotype::getAgeMaintenanceFactor() const {
    float age = organism_state_.age_normalized;
    
    if (age < 0.1f) {
        // Juvenile: Growing rapidly, high metabolism
        return 0.6f + (age / 0.1f) * 0.8f; // 0.6 → 1.4
    } else if (age < 0.8f) {
        // Adult: Peak efficiency
        return 1.0f;
    } else {
        // Elderly: Declining repair mechanisms
        float elderly_progress = (age - 0.8f) / 0.2f;
        return 1.0f + (elderly_progress * 0.5f); // 1.0 → 1.5
    }
}
```

**Life stage costs:**
- **Juvenile (0-10% lifespan):** 60-140% normal cost (growth expensive)
- **Adult (10-80% lifespan):** 100% cost (peak efficiency)
- **Elderly (80-100% lifespan):** 100-150% cost (wear and tear)

**Emergence:** Species evolve appropriate lifespans for their strategies:
- Fast predators: Short lifespan (avoid elderly penalty)
- Long-lived plants: High HARDINESS gene (mitigate elderly costs)

### 4.3 Environmental Modulation

Temperature affects metabolic costs:

```cpp
float Phenotype::getEnvironmentCostFactor() const {
    const float temp = environment_.temperature;
    const float optimal_temp = (getTrait(TEMP_TOLERANCE_LOW) + 
                                getTrait(TEMP_TOLERANCE_HIGH)) / 2.0f;
    
    float temp_deviation = std::abs(temp - optimal_temp);
    
    // For every 10°C away from optimum, +20% cost
    float temp_penalty = temp_deviation / 10.0f * 0.2f;
    
    return 1.0f + temp_penalty;
}
```

**Example:**
- Creature optimal: 20°C
- Current temp: 30°C
- Deviation: 10°C → +20% energy cost
- If maintenance = 5/day, actual cost = 6/day

**Emergence:** Species evolve temperature tolerances matching their habitats. Warm-adapted and cold-adapted species naturally separate.

---

## Part 5: Implementation Architecture

### 5.1 Energy Budget Tracking

Add to organism implementations:

```cpp
class IEnergeticOrganism {
public:
    virtual ~IEnergeticOrganism() = default;
    
    // Energy state
    virtual float getEnergyLevel() const = 0;
    virtual float getEnergyCapacity() const = 0;
    virtual float getEnergyRatio() const = 0; // level / capacity
    
    // Energy flows
    virtual float calculateIncome(float deltaTime) const = 0;
    virtual float calculateMaintenance(float deltaTime) const = 0;
    virtual float calculateActivityCost(ActivityType activity) const = 0;
    
    // Energy transactions
    virtual bool spendEnergy(float amount, SpendingCategory category) = 0;
    virtual void gainEnergy(float amount) = 0;
    
    // Budget queries
    virtual float getAvailableEnergy() const = 0; // After maintenance
    virtual bool canAfford(ActivityType activity) const = 0;
};
```

### 5.2 Gene Cost Registry

Extend [`GeneDefinition`](../../../include/genetics/core/Gene.hpp:107) to include costs:

```cpp
class GeneDefinition {
private:
    std::string id_;
    ChromosomeType chromosome_;
    GeneLimits limits_;
    DominanceType dominance_;
    std::vector<EffectBinding> effects_;
    
    // NEW: Energy cost data
    GeneCost energy_cost_;
    
public:
    // Existing methods...
    
    // NEW: Cost accessors
    const GeneCost& getEnergyCost() const { return energy_cost_; }
    void setEnergyCost(const GeneCost& cost) { energy_cost_ = cost; }
    
    // NEW: Calculate maintenance for specific expression level
    float calculateMaintenance(float expression_level) const {
        float factor = std::pow(expression_level, energy_cost_.expression_scaling);
        return energy_cost_.base_cost * factor;
    }
};
```

### 5.3 Phenotype Cost Calculation

Extend [`Phenotype`](../../../include/genetics/expression/Phenotype.hpp:44) class:

```cpp
class Phenotype {
private:
    const Genome* genome_;
    const GeneRegistry* registry_;
    mutable PhenotypeCache cache_;
    EnvironmentState environment_;
    OrganismState organism_state_;
    
    // NEW: Energy tracking
    mutable float cached_maintenance_cost_ = -1.0f;
    mutable bool maintenance_cache_valid_ = false;

public:
    // Existing methods...
    
    // NEW: Energy cost methods
    float calculateTotalMaintenance() const;
    float calculateGeneCategory Cost(CostCategory category) const;
    float getAgeMaintenanceFactor() const;
    float getEnvironmentCostFactor() const;
    
    // NEW: Budget queries
    bool canExpressGene(const std::string& gene_id, float target_expression) const;
    float getAffordableExpression(const std::string& gene_id, float energy_budget) const;
    
    // NEW: Cache invalidation
    void invalidateMaintenanceCache() {
        maintenance_cache_valid_ = false;
        cached_maintenance_cost_ = -1.0f;
    }
};
```

### 5.4 Organism Update Loop

Modify organism update cycle to include energy budgeting:

```cpp
void Creature::update(float deltaTime) {
    // 1. Calculate income
    float energy_income = calculateEnergyIncome(deltaTime);
    energy_ += energy_income;
    
    // 2. Pay mandatory maintenance
    float maintenance = phenotype_->calculateTotalMaintenance() * deltaTime;
    if (energy_ < maintenance) {
        // Starvation! Health declines
        health_ -= (maintenance - energy_) * 0.1f;
        energy_ = 0.0f;
    } else {
        energy_ -= maintenance;
    }
    
    // 3. Decide activities based on available energy
    float available = energy_ - (maintenance * 2.0f); // Keep 2x maintenance as reserve
    
    if (available > 0) {
        // Can afford activities
        if (canReproduce() && available > reproductionCost()) {
            reproduce();
        } else if (needsFood() && available > movementCost()) {
            moveTowardsFood();
        }
        // etc.
    } else {
        // Must rest to recover energy
        rest();
    }
    
    // 4. Age progression
    age_ += deltaTime;
    organism_state_.age_normalized = age_ / maxAge_;
    
    // 5. Update phenotype cache if needed
    if (significantStateChange()) {
        phenotype_->invalidateMaintenanceCache();
    }
}
```

---

## Part 6: Tuning Parameters

### 6.1 Global Energy Scales

Control simulation-wide energy availability:

```cpp
struct EnergyConfiguration {
    float base_photosynthesis_rate = 5.0f;    // Energy/day for plants
    float base_herbivore_extraction = 0.6f;   // Fraction of plant energy
    float base_carnivore_extraction = 0.7f;   // Fraction of prey energy
    float starvation_health_loss = 0.1f;      // Health loss per energy deficit
    float movement_cost_multiplier = 1.0f;    // Scale all movement costs
    float global_cost_multiplier = 1.0f;      // Scale ALL maintenance costs
};
```

**Tuning guidelines:**
- If all organisms starving: Increase income OR decrease `global_cost_multiplier`
- If no selection pressure: Decrease income OR increase costs
- If specialists too dominant: Decrease specialist efficiency bonuses
- If generalists too dominant: Increase maintenance costs or pleiotropy strength

### 6.2 Per-Gene Cost Tuning

Each gene's cost can be individually adjusted:

```cpp
// If mobility too dominant
GeneCost locomotion_cost;
locomotion_cost.base_cost = 2.0f;  // Increase from 1.5f
locomotion_cost.expression_scaling = 1.5f; // Superlinear penalty

// If toxin defense too weak
GeneCost toxin_cost;
toxin_cost.base_cost = 1.0f; // Decrease from 1.5f to make affordable
```

### 6.3 Testing Specialization Pressure

Simulation should show:

```
Generation 0 (random):
- 100 organisms, random expression levels
- Average gene diversity: 0.8 (high variation)
- Average maintenance cost: 12.5 / day
- Survival rate: 20%

Generation 50 (selection):
- 200 organisms, specialized lineages
- Average gene diversity: 0.3 (low variation - specialists!)
- Average maintenance cost: 6.8 / day
- Survival rate: 85%
- Archetypes: 30% fast predators, 40% herbivores, 20% omnivores, 10% other
```

**Success criteria:**
1. Specialist archetypes emerge naturally
2. Population stabilizes below carrying capacity
3. Multiple viable strategies coexist
4. No single archetype dominates all environments

---

## Part 7: Emergent Dynamics Examples

### 7.1 Energy-Rich Environment

**Setup:** Abundant food (15 energy/day available)

**Generation 0:**
- Random organisms with 5-15 energy/day maintenance costs
- High-cost generalists (10+ cost) can survive initially

**Generation 25:**
- Selection favors organisms investing in competitive advantages
- HIGH LOCOMOTION predators emerge (can afford 4.7/day cost)
- HIGH TOXIN plants emerge (can afford 4.3/day cost)
- Arms race begins

**Generation 100:**
- Specialized predators with HIGH SIGHT + HIGH SPEED dominate
- Defensive plants with HIGH TOXINS + HIGH THORNS dominate
- Simple generalists extinct (outcompeted in every niche)
- Remaining generalists are sophisticated (behavioral switching)

**Insight:** Resource abundance allows "expensive" strategies that would starve in scarcity.

### 7.2 Energy-Scarce Environment

**Setup:** Limited food (6 energy/day available)

**Generation 0:**
- High-cost organisms (>6/day) starve immediately
- Only low-cost organisms (3-6/day) survive

**Generation 25:**
- Selection favors efficiency over capabilities
- LOW LOCOMOTION (sit-and-wait) predators emerge
- LOW GROWTH (slow but tough) plants emerge
- MEDIUM EVERYTHING generalists thrive (flexibility valuable)

**Generation 100:**
- Population consists of energy-efficient specialists
- Generalists maintain 30-40% of population (scarcity favors flexibility)
- Few high-cost organisms (only if extremely efficient at their specialty)

**Insight:** Scarcity favors energy efficiency and flexibility over raw power.

### 7.3 Seasonal Variation

**Setup:** Alternating 10 energy/day (summer) and 4 energy/day (winter)

**Generation 0:**
- Summer: High-cost organisms thrive
- Winter: Mass die-off of high-cost organisms

**Generation 50:**
- Two strategies emerge:
  1. **Migrants:** HIGH LOCOMOTION, leave during winter
  2. **Hibernators:** Can reduce maintenance (LOW active genes)

**Generation 100:**
- Stable coexistence of three groups:
  - Summer specialists (40%): Reproduce rapidly in summer, die in winter
  - Year-round generalists (40%): Moderate costs, survive both seasons
  - Hibernators (20%): Enter low-energy state in winter

**Insight:** Environmental variation maintains generalist niche permanently.

---

## Part 8: Mathematical Validation

### 8.1 Specialization Equilibrium

At equilibrium, specialist advantage equals generalist flexibility:

```
Specialist fitness = Efficiency_specialist × P(resource_available) - Cost_specialist
Generalist fitness = Efficiency_generalist × P(any_resource) - Cost_generalist

Equilibrium when:
Efficiency_specialist × P_specialist - Cost_specialist = 
Efficiency_generalist × P_generalist - Cost_generalist
```

**Example calculation:**

Environment: 60% plant abundant, 40% prey abundant

Herbivore specialist:
- Efficiency: 1.2 (specialist bonus)
- Resource probability: 0.6
- Cost: 6.3/day
- Fitness: 1.2 × 0.6 × 10 - 6.3 = 7.2 - 6.3 = **0.9**

Carnivore specialist:
- Efficiency: 1.2
- Resource probability: 0.4
- Cost: 4.7/day
- Fitness: 1.2 × 0.4 × 15 - 4.7 = 7.2 - 4.7 = **2.5** ✓ (Dominates!)

Omnivore generalist:
- Efficiency: 0.6 (pleiotropy penalty)
- Resource probability: 1.0 (eats anything)
- Cost: 7.5/day
- Fitness: 0.6 × 1.0 × 12.5 - 7.5 = 7.5 - 7.5 = **0.0** (Barely survives)

**Prediction:** Carnivores dominate, herbivores survive, omnivores struggle.

**Validation:** Run simulation and measure population ratios. Adjust costs if predictions don't match outcomes.

### 8.2 Trait Correlation Analysis

Measure which traits correlate in evolved populations:

```cpp
struct TraitCorrelation {
    std::string trait1;
    std::string trait2;
    float correlation; // -1.0 to 1.0
};

// Example expected correlations:
// HIGH LOCOMOTION ↔ LOW MAX_SIZE: -0.7 (speed vs bulk)
// HIGH PLANT_DIGESTION ↔ LOW MEAT_DIGESTION: -0.8 (dietary exclusion)
// HIGH TOXIN_PRODUCTION ↔ LOW GROWTH_RATE: -0.6 (defense vs growth)
// HIGH SIGHT_RANGE ↔ HIGH LOCOMOTION: +0.7 (sensory-motor syndrome)
```

**Success metric:** Strong correlations emerge matching biological predictions.

---

## Part 9: Code Structure

### 9.1 New Files Required

```
include/genetics/
├── core/
│   └── GeneCost.hpp          // NEW: Cost definitions
├── expression/
│   ├── EnergyBudget.hpp      // NEW: Energy tracking
│   └── ResourceAllocator.hpp // NEW: Cost calculations
└── interfaces/
    └── IEnergeticOrganism.hpp // NEW: Energy interface

src/genetics/
├── core/
│   └── GeneCost.cpp
├── expression/
│   ├── EnergyBudget.cpp
│   └── ResourceAllocator.cpp
```

### 9.2 Modified Files

```
include/genetics/core/Gene.hpp
- Add GeneCost member to GeneDefinition
- Add calculateMaintenance() method

include/genetics/expression/Phenotype.hpp
- Add calculateTotalMaintenance()
- Add maintenance caching
- Add budget query methods

include/genetics/expression/OrganismState.hpp
- Add energy_level and energy_capacity fields

include/genetics/defaults/UniversalGenes.hpp
- Add cost specifications for all 29 genes
- Document cost rationales
```

### 9.3 Implementation Priority

**Phase 1:** Core infrastructure
1. Create `GeneCost` class
2. Extend `GeneDefinition` with costs
3. Add energy tracking to `OrganismState`

**Phase 2:** Cost calculations
1. Implement `Phenotype::calculateTotalMaintenance()`
2. Add age and environment modulation
3. Add caching for performance

**Phase 3:** Organism integration
1. Update `Creature::update()` with energy budget
2. Update `Plant::update()` with energy budget
3. Add energy-based decision making

**Phase 4:** Gene cost definitions
1. Define costs for all 29 universal genes
2. Add costs to gene registration
3. Document cost rationales

**Phase 5:** Testing and tuning
1. Unit tests for cost calculations
2. Integration tests for energy budgeting
3. Evolution simulations to verify specialization
4. Tune costs based on emergent behaviors

---

## Part 10: Expected Emergent Behaviors

### 10.1 Short-term Emergence (1-10 generations)

**Energy-based decision making:**
- Organisms prioritize high-return, low-cost activities
- Rest when energy low (automatic behavior, not scripted)
- Avoid costly unsuccessful activities

**Example:** Predator with low energy stops chasing fast prey (movement cost too high), waits for easier opportunity.

### 10.2 Medium-term Emergence (10-50 generations)

**Niche specialization:**
- Fast reproducers in resource-rich zones
- Efficient survivors in resource-poor zones
- Different strategies in different regions of same map

**Example:** River valleys have fast-growing plants (water abundant = can afford high growth cost). Deserts have water-storing succulents (scarcity = need efficiency).

### 10.3 Long-term Emergence (50-500 generations)

**Ecosystem structure:**
- Three-tier food webs form naturally
- Energy flows match ecological models
- Predator-prey cycles stabilize
- Multiple stable states in different biomes

**Example:** Forest biome develops:
- Canopy: Fast-growing plants (HIGH GROWTH, LOW DEFENSE)
- Understory: Shade-tolerant plants (LOW LIGHT_REQUIREMENT, LOW GROWTH)
- Herbivores: Specialist browsers (HIGH TOXIN_TOLERANCE)
- Predators: Ambush hunters (LOW LOCOMOTION, HIGH PATIENCE)

### 10.4 Very Long-term Emergence (500+ generations)

**Coevolutionary dynamics:**
- Arms races stabilize at cost equilibria
- Mutualistic relationships emerge (plant-disperser)
- Migration patterns optimize energy acquisition
- Seasonal adaptations to environmental cycles

**Example:** Plants evolve HIGH FRUIT_SWEETNESS exactly when herbivores develop HIGH FRUIT_PREFERENCE, creating mutualistic dispersal system naturally from energy economics.

---

## Part 11: Troubleshooting Guide

### Problem: All organisms starve

**Diagnosis:** Energy costs too high or income too low

**Solutions:**
1. Reduce `global_cost_multiplier` from 1.0 to 0.7
2. Increase `base_photosynthesis_rate` by 20%
3. Increase `base_herbivore_extraction` from 0.6 to 0.7
4. Check for bugs in cost calculation (summing costs twice?)

### Problem: No specialization occurs

**Diagnosis:** Selection pressure too weak

**Solutions:**
1. Decrease energy income (make resources scarcer)
2. Increase variation in gene costs (make trade-offs sharper)
3. Increase negative pleiotropy strength (make generalists worse)
4. Verify mutation rate isn't too high (preventing selection)

### Problem: Only one archetype survives

**Diagnosis:** One strategy too dominant

**Solutions:**
1. Increase cost of dominant archetype's key genes
2. Add environmental variation (favor different strategies over time)
3. Increase specialist efficiency bonus (help other niches compete)
4. Check for niche overlap (maybe specialists aren't differentiated enough)

### Problem: Costs seem arbitrary

**Response:** This is a feature, not a bug!

**Explanation:**
The exact cost values are tuning parameters. What matters is:
1. **Relative costs:** Mobility > Defense > Sensing (matches biology)
2. **Trade-off strength:** High enough to prevent "super organisms"
3. **Emergent specialization:** Multiple viable strategies coexist

Start with biologically-inspired ratios, then adjust based on simulation outcomes. The goal is interesting dynamics, not perfect realism.

---

## Part 12: Future Extensions

### 12.1 Developmental Costs

Add one-time costs for growing structures:

```cpp
struct DevelopmentalCost {
    float growth_cost_per_unit;  // Energy to build each unit of size
    float metamorphosis_cost;     // Energy for life stage transitions
};
```

**Example:** Growing from size 1.0 to 2.0 costs 50 energy (not maintenance, but capital investment).

### 12.2 Opportunity Costs

Track what organism COULD have done with energy:

```cpp
struct ActivityOpportunityCost {
    float hunting_vs_mating;  // Hunting time can't be used for mating
    float growth_vs_repair;   // Energy for growth can't repair damage
};
```

**Example:** Young organism prioritizes growth (future payoff), old organism prioritizes repair (immediate survival).

### 12.3 Social Costs

Extend to group-living organisms:

```cpp
struct SocialCost {
    float group_coordination;   // Maintaining social bonds
    float dominance_displays;   // Status competition
    float offspring_care;       // Parenting investment
};
```

**Example:** Social species have higher costs but gain group hunting efficiency, creating new emergent trade-offs.

### 12.4 Seasonal Acclimation

Allow temporary cost reduction through adaptation:

```cpp
struct AcclimationState {
    float cold_acclimated;  // Reduces cold temperature penalty
    float heat_acclimated;  // Reduces heat temperature penalty
    float acclimation_rate; // How quickly adaptation occurs
};
```

**Example:** Organism in cold environment gradually reduces temperature maintenance cost over days/weeks, but loses acclimation if moves to warm area.

---

## Conclusion

This Universal Resource Allocation Framework transforms the genetics system into an **energy economics simulator** where:

1. **Every gene has a price:** Expression costs energy
2. **Budgets are finite:** Total costs cannot exceed income
3. **Trade-offs are mandatory:** Cannot invest in everything
4. **Specialization emerges:** Selection favors focused strategies
5. **Niches arise naturally:** Different environments favor different allocations
6. **Diversity persists:** Multiple strategies remain viable

**Key innovation:** This is not a constraint system with arbitrary caps, but an **economic system with natural equilibria**. Organisms evolve appropriate specializations based on their energy budgets, environmental conditions, and competitive pressures.

**Implementation philosophy:** Start simple, tune costs through simulation, let complex behaviors emerge from simple energy rules. Trust the evolution algorithm to find optimal allocations—you just provide the economic constraints.

**Next steps:**
1. Review this design with team/users
2. Implement core infrastructure (Phase 1-2)
3. Run small-scale evolution tests
4. Tune costs based on observed specialization
5. Document emergent archetypes
6. Expand to full simulation

The biological world runs on energy economics. This framework brings that fundamental principle into EcoSim.

---

## See Also

**Core Documentation:**
- [[../core/01-architecture]] - System architecture overview
- [[../core/02-getting-started]] - Quick start tutorial
- [[../core/03-extending]] - Extension guide

**Reference:**
- [[../reference/quick-reference]] - Quick reference cheat sheet
- [[../reference/genes]] - Complete gene catalog
- [[../reference/api/core-classes]] - Core API reference
- [[../reference/api/expression]] - Expression system API reference

**Design:**
- [[coevolution-design]] - Creature-plant coevolutionary dynamics
- [[propagation-design]] - Seed dispersal strategies and mechanisms
- [[prefab-design]] - Plant factory architecture and species templates
