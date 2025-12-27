---
title: Plant Prefab System Design
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: designer
type: design
tags: [genetics, design, prefabs, plants, factory]
---

# Plant Prefab System Design

**Related Documents:**

- [[propagation-design]] - Plant dispersal strategies
- [`PlantFactory.hpp`](../../../include/genetics/organisms/PlantFactory.hpp) - Factory implementation
- [[../core/01-architecture]] - Core genetics architecture

---

## Executive Summary

This document defines the architecture for plant prefabs in the EcoSim genetics system. The design supports **species-level variation** through gene ranges, **hybrid food production** (plants produce Food items AND can be eaten directly), and **emergent defense mechanisms** that encourage evolutionary gameplay.

**Key Design Decision:** Hybrid food production model enables plants with varying strategies:
- **r-strategy plants**: Produce abundant, easily accessible fruit (Food objects)
- **K-strategy plants**: Invest in self-defense, require adaptation to consume directly
- **Mixed strategy**: Balance both approaches based on genetic traits

---

## Table of Contents

1. [Plant Species Concept](#1-plant-species-concept)
2. [Prefab Factory Pattern](#2-prefab-factory-pattern)
3. [Food Production Mechanism](#3-food-production-mechanism)
4. [World Integration](#4-world-integration)
5. [Example Prefab Definitions](#5-example-prefab-definitions)
6. [Implementation Plan](#6-implementation-plan)

---

## 1. Plant Species Concept

### 1.1 Species as Gene Range Templates

A "species" in the genetics system is defined by **constrained gene ranges** rather than fixed values. This preserves genetic variation within species while maintaining species identity.

```cpp
/**
 * @brief Species template defining gene ranges for a plant type
 * 
 * Each species specifies acceptable ranges for genes. Individual plants
 * within the species have random values within these ranges, allowing
 * for natural variation and evolution while maintaining species identity.
 */
struct PlantSpeciesTemplate {
    std::string species_name;
    std::string description;
    
    // Gene ranges - each gene has [min, max] within species
    std::unordered_map<std::string, std::pair<float, float>> gene_ranges;
    
    // Visual representation
    char display_char;
    float base_color_hue;  // HSV hue, can vary by ±20 degrees
    
    // Ecological role hints (not strict constraints)
    std::string preferred_terrain;  // "forest", "plains", "desert", etc.
    std::pair<float, float> optimal_temp_range;
};
```

**Design Rationale:**
- **Variation within species**: Two berry bushes will differ slightly in size, nutrient value, seed production
- **Evolutionary pressure**: Natural selection can shift species characteristics over generations
- **Speciation potential**: Isolated populations can diverge into subspecies
- **No hard-coded genetics**: Avoids rigidity of fixed gene values

### 1.2 Species vs Individuals

```
Species Template (Blueprint)
    ↓
Individual Plant (Instance)
    ↓
Genome (Random within species ranges)
    ↓
Phenotype (Expressed traits + environment)
    ↓
Behavior (Emergent from traits)
```

**Example:**

```cpp
// Species: Berry Bush
// - max_size: [2.0, 4.0]
// - nutrient_value: [15.0, 30.0]
// - hardiness: [0.3, 0.6]

// Individual 1: Small, nutritious, fragile
// - max_size: 2.3 (rolled within range)
// - nutrient_value: 28.0
// - hardiness: 0.35

// Individual 2: Large, less nutritious, hardy
// - max_size: 3.8
// - nutrient_value: 17.0
// - hardiness: 0.58
```

---

## 2. Prefab Factory Pattern

### 2.1 Factory Interface

```cpp
namespace EcoSim {
namespace Genetics {

/**
 * @brief Factory for creating plant instances from species templates
 * 
 * Supports both data-driven (JSON/config) and hardcoded prefabs.
 * Uses dependency injection for GeneRegistry (no singletons).
 */
class PlantFactory {
public:
    /**
     * @brief Create a plant from a species template
     * @param species_name Name of the species template to use
     * @param x World X position
     * @param y World Y position
     * @param registry Gene registry for genome creation
     * @return Unique pointer to new Plant instance
     * 
     * Creates a plant with random gene values within the species' ranges.
     */
    static std::unique_ptr<Plant> createFromSpecies(
        const std::string& species_name,
        int x, int y,
        const GeneRegistry& registry
    );
    
    /**
     * @brief Create a plant with specific genome (for breeding/loading)
     * @param genome Pre-existing genome
     * @param x World X position
     * @param y World Y position
     * @param registry Gene registry for phenotype expression
     * @return Unique pointer to new Plant instance
     */
    static std::unique_ptr<Plant> createFromGenome(
        const Genome& genome,
        int x, int y,
        const GeneRegistry& registry
    );
    
    /**
     * @brief Register a new species template
     * @param template Species definition to register
     * 
     * Allows runtime registration of species from JSON or code.
     */
    static void registerSpecies(PlantSpeciesTemplate&& species_template);
    
    /**
     * @brief Get list of all registered species names
     * @return Vector of species names
     */
    static std::vector<std::string> getAvailableSpecies();
    
    /**
     * @brief Check if a species is registered
     * @param species_name Name to check
     * @return true if species exists
     */
    static bool hasSpecies(const std::string& species_name);
    
    /**
     * @brief Get species template by name
     * @param species_name Name of species
     * @return Const reference to template
     * @throws std::runtime_error if species not found
     */
    static const PlantSpeciesTemplate& getSpecies(const std::string& species_name);

private:
    // Species registry (static storage)
    static std::unordered_map<std::string, PlantSpeciesTemplate> species_registry_;
    
    // Helper: Create genome from template
    static Genome createGenomeFromTemplate(
        const PlantSpeciesTemplate& species_template,
        const GeneRegistry& registry
    );
};

/**
 * @brief Register all default plant species
 * @param factory PlantFactory to register species into
 * 
 * Called during initialization to populate factory with default species
 * (berry bushes, fruit trees, grass, etc.)
 */
void registerDefaultPlantSpecies(PlantFactory& factory);

} // namespace Genetics
} // namespace EcoSim
```

### 2.2 Usage Examples

```cpp
// Initialize gene registry
GeneRegistry plantRegistry;
PlantGenes::registerDefaults(plantRegistry);

// Register default species
registerDefaultPlantSpecies(PlantFactory);

// Create instances from species templates
auto berry_bush = PlantFactory::createFromSpecies("berry_bush", 10, 20, plantRegistry);
auto oak_tree = PlantFactory::createFromSpecies("oak_tree", 15, 25, plantRegistry);
auto grass = PlantFactory::createFromSpecies("grass", 5, 5, plantRegistry);

// Breeding produces offspring with inherited genome
if (berry_bush->canSpreadSeeds()) {
    auto offspring = berry_bush->produceOffspring(plantRegistry);
    // Offspring genome is mutated copy of parent, not from species template
}
```

### 2.3 Data-Driven Species (Future Extension)

```json
{
  "species_name": "thorn_bush",
  "description": "Thorny bush with defensive spines",
  "display_char": "#",
  "base_color_hue": 80,
  "preferred_terrain": "savanna",
  "optimal_temp_range": [15, 35],
  "gene_ranges": {
    "plant_growth_rate": [0.3, 0.7],
    "plant_max_size": [2.0, 4.0],
    "plant_hardiness": [0.7, 0.9],
    "plant_nutrient_value": [5.0, 15.0],
    "plant_seed_production": [2.0, 5.0],
    "plant_defense_level": [0.8, 1.0]
  }
}
```

**Future Work:** Implement JSON/YAML loading for species definitions, enabling modding and dynamic content without recompilation.

---

## 3. Food Production Mechanism

### 3.1 Hybrid Model Architecture

Plants support **three consumption modes**:

```
┌────────────────────────────────────────┐
│              PLANT                     │
│                                        │
│  ┌──────────────┐   ┌──────────────┐  │
│  │  Direct      │   │  Food        │  │
│  │  Consumption │   │  Production  │  │
│  └──────┬───────┘   └──────┬───────┘  │
│         │                  │           │
└─────────┼──────────────────┼───────────┘
          │                  │
          ▼                  ▼
   ┌─────────────┐    ┌──────────┐
   │  Herbivore  │    │   Food   │
   │   takes     │    │  Object  │
   │   damage    │    │  spawns  │
   │  to plant   │    │  on tile │
   └─────────────┘    └──────────┘
```

### 3.2 New Plant Genes for Defense

Extend PlantGenes with defense-related genes:

```cpp
// Add to PlantGenes class

// Defense Morphology genes
static constexpr const char* DEFENSE_LEVEL = "plant_defense_level";
static constexpr const char* REGENERATION_RATE = "plant_regeneration_rate";
static constexpr const char* TOXICITY = "plant_toxicity";
static constexpr const char* STRUCTURAL_INTEGRITY = "plant_structural_integrity";

// Food production genes
static constexpr const char* FRUIT_PRODUCTION_RATE = "plant_fruit_production_rate";
static constexpr const char* FRUIT_DROP_AGE = "plant_fruit_drop_age";
```

**Gene Definitions:**

| Gene | Range | Effect | Evolutionary Trade-off |
|------|-------|--------|----------------------|
| `defense_level` | [0.0, 1.0] | Reduces damage from herbivores eating plant | High defense → slower growth |
| `regeneration_rate` | [0.0, 1.0] | Health recovery per tick after damage | High regen → lower seed production |
| `toxicity` | [0.0, 1.0] | Damages herbivore when eaten | High toxicity → fewer dispersers for seeds |
| `structural_integrity` | [0.0, 1.0] | Minimum health threshold before death | High integrity → higher nutrient cost |
| `fruit_production_rate` | [0.0, 10.0] | Food items produced per maturity cycle | High production → less self-defense |
| `fruit_drop_age` | [0.3, 0.9] | Normalized age when fruit drops | Early drop → less nutrition |

### 3.3 Enhanced Plant Class

```cpp
class Plant : public IPositionable, public ILifecycle, public IGeneticOrganism {
public:
    // ... existing interface ...
    
    // ========================================================================
    // Food Production (NEW)
    // ========================================================================
    
    /**
     * @brief Check if plant can produce fruit/seeds as Food items
     * @return true if plant is mature and ready to produce Food
     */
    bool canProduceFood() const;
    
    /**
     * @brief Generate a Food object from this plant
     * @return Food object with calories based on nutrient_value gene
     * 
     * Creates a Food item that drops on the tile. The Food object's
     * properties are derived from plant's genome:
     * - calories = nutrient_value * fruit_nutrition_ratio
     * - lifespan = based on fruit_drop_age and plant's decay rate
     * - appearance = color similar to plant
     */
    Food produceFood() const;
    
    /**
     * @brief Get time in ticks until next food production
     * @return Ticks remaining, or 0 if can produce now
     */
    unsigned int getTicksUntilFoodProduction() const;
    
    // ========================================================================
    // Direct Consumption (NEW)
    // ========================================================================
    
    /**
     * @brief Check if herbivore can attempt to eat this plant
     * @param herbivore_defense Herbivore's defense/resistance level
     * @return true if plant can be eaten (considering defenses)
     * 
     * Factors:
     * - Plant must be alive
     * - Plant size must be > 10% of max (too small to eat)
     * - Herbivore defense must overcome plant defense_level
     */
    bool canBeEatenBy(float herbivore_defense) const;
    
    /**
     * @brief Attempt to consume part of the plant
     * @param herbivore_defense Herbivore's resistance to plant defenses
     * @return Nutrient value obtained (may be reduced by defenses)
     * 
     * Effects:
     * - Applies damage to plant based on bite size
     * - Returns nutrient value (possibly reduced by toxicity/hardiness)
     * - May damage herbivore if plant is toxic
     * - Triggers regeneration if plant has regeneration_rate > 0
     */
    float consumePlant(float herbivore_defense);
    
    /**
     * @brief Get effective nutrient value for herbivore with given defense
     * @param herbivore_defense Herbivore's defense stat
     * @return Actual nutrition obtained after plant defenses
     */
    float getEffectiveNutrientValue(float herbivore_defense) const;
    
    /**
     * @brief Get damage dealt to herbivore when plant is eaten
     * @param herbivore_defense Herbivore's resistance
     * @return Damage value (0.0 if non-toxic)
     */
    float getCounterDamage(float herbivore_defense) const;
    
    // ========================================================================
    // Defense System (NEW)
    // ========================================================================
    
    /**
     * @brief Get defense level from genome
     * @return Defense value 0.0-1.0 (thorns, spines, tough bark)
     */
    float getDefenseLevel() const;
    
    /**
     * @brief Get toxicity level from genome
     * @return Toxicity value 0.0-1.0 (poison, irritants)
     */
    float getToxicity() const;
    
    /**
     * @brief Get regeneration rate from genome
     * @return Regeneration value 0.0-1.0 (health per tick)
     */
    float getRegenerationRate() const;
    
    /**
     * @brief Get structural integrity threshold
     * @return Minimum health ratio before death (0.0-1.0)
     */
    float getStructuralIntegrity() const;
    
    /**
     * @brief Regenerate health over time
     * 
     * Called each tick. Plants with high regeneration_rate can
     * recover from herbivore damage.
     */
    void regenerate();
    
private:
    // Food production state
    unsigned int ticks_since_last_food_ = 0;
    unsigned int food_production_cooldown_ = 0;
    
    // Defense state
    bool recently_damaged_ = false;
    unsigned int damage_recovery_timer_ = 0;
};
```

### 3.4 Creature Integration

Creatures need to decide between eating Food objects vs eating Plants directly:

```cpp
// In Creature class (future work)

/**
 * @brief Evaluate whether to eat Food object or Plant
 * @param food_option Available Food object on tile
 * @param plant_option Available Plant on tile
 * @return Chosen food source (Food or Plant)
 * 
 * Decision factors:
 * - Creature's herbivore_defense gene (can they eat defended plants?)
 * - Energy efficiency (Food vs Plant nutrition)
 * - Risk assessment (plant toxicity vs hunger urgency)
 */
FoodSource chooseFoodSource(const Food* food_option, const Plant* plant_option);

/**
 * @brief Attempt to eat a plant
 * @param plant Target plant to eat
 * @return Nutrition gained (may be less than expected if plant defends)
 * 
 * Interaction:
 * 1. Check if plant.canBeEatenBy(this->getHerbivoreDefense())
 * 2. Call plant.consumePlant(this->getHerbivoreDefense())
 * 3. Take counter-damage if plant is toxic
 * 4. Gain nutrition (or less if defenses reduce it)
 */
float eatPlant(Plant* plant);
```

### 3.5 Evolutionary Dynamics

The hybrid system creates evolutionary pressure:

```
High Food Production                    High Self-Defense
(r-strategy)                            (K-strategy)
        │                                      │
        │  ┌────────────────────┐              │
        ├──┤  Berry Bush        │              │
        │  │  - Easy to eat     │              │
        │  │  - High fruit      │              │
        │  │  - Fast spread     │              │
        │  └────────────────────┘              │
        │                                      │
        │  ┌────────────────────┐              │
        ├──┤  Grass             │              │
        │  │  - Minimal defense │              │
        │  │  - Rapid regrowth  │──────────────┤
        │  │  - High regen rate │              │
        │  └────────────────────┘              │
        │                                      │
        │  ┌────────────────────┐              │
        │  │  Oak Tree          │──────────────┤
        │  │  - Balanced        │              │
        │  │  - Medium defense  │              │
        │  │  - Acorns (Food)   │              │
        │  └────────────────────┘              │
        │                                      │
        │  ┌────────────────────┐              │
        │  │  Thorn Bush        │──────────────┤
        │  │  - High defense    │              │
        │  │  - Toxic           │              │
        │  │  - Low food        │              │
        │  └────────────────────┘              │
```

**Emergent Behaviors:**
- **Arms race**: Herbivores evolve defense, plants evolve toxicity
- **Mutualism**: Plants with good fruit attract herbivores who spread seeds
- **Niche partitioning**: Fast-reproducing grass vs slow-growing defended trees
- **Grazing tolerance**: Some plants evolve regeneration strategy (grass, bamboo)

---

## 4. World Integration

### 4.1 Storage Architecture

**Plants Stored in Tile**

```cpp
// Tile class stores plants
class Tile {
private:
    std::vector<Food> _foodVec;
    std::vector<Plant*> _plants;         // Pointers to Plants (owned elsewhere)
    
public:
    // Plant management
    bool addPlant(Plant* plant);
    void removePlant(Plant* plant);
    const std::vector<Plant*>& getPlants() const;
    
    // Update plants on this tile
    void updatePlants(const EnvironmentState& env);
    
    // Food production: Plants can add Food to _foodVec
    void produceFoodFromPlants();
};
```

**Rationale:**
- **Spatial queries**: Easy to find plants at specific coordinates
- **Rendering**: Iterate tiles, render tile contents including plants
- **Memory**: Plants are heap-allocated, Tile stores pointers

**Option B: Separate PlantContainer (Alternative)**

```cpp
// New container class
class PlantContainer {
private:
    std::vector<std::unique_ptr<Plant>> plants_;
    std::unordered_map<std::pair<int, int>, std::vector<size_t>> spatial_index_;
    
public:
    void addPlant(std::unique_ptr<Plant> plant);
    void removePlant(size_t index);
    std::vector<Plant*> getPlantsAt(int x, int y);
    void updateAll(const EnvironmentState& env);
};
```

**Comparison:**

| Aspect | Tile Storage (A) | Separate Container (B) |
|--------|------------------|------------------------|
| Spatial queries | O(1) direct access | O(1) with hash map |
| Memory | Distributed across tiles | Centralized vector |
| Cache locality | Poor (scattered) | Better (contiguous) |
| Integration | Simple, existing structure | New system |
| Rendering | Easy (tile iteration) | Requires position lookup |

**Recommendation: Option A (Tile Storage)** for simplicity and consistency with existing architecture.

### 4.2 Update Loop Integration

```cpp
// In World class update loop

void World::updateWorld(const EnvironmentState& env) {
    // 1. Update tiles (terrain, environment)
    updateTileEnvironment(env);
    
    // 2. Update plants
    for (auto& row : tiles_) {
        for (auto& tile : row) {
            // Update all plants on this tile
            tile.updatePlants(env);
            
            // Handle plant food production
            tile.produceFoodFromPlants();
        }
    }
    
    // 3. Update creatures
    updateCreatures(env);
    
    // 4. Update food decay
    updateFoodDecay();
    
    // 5. Handle plant deaths and cleanup
    cleanupDeadPlants();
}

void Tile::updatePlants(const EnvironmentState& env) {
    for (auto* plant : _plants) {
        if (plant && plant->isAlive()) {
            plant->update(env);
        }
    }
}

void Tile::produceFoodFromPlants() {
    for (auto* plant : _plants) {
        if (plant && plant->canProduceFood()) {
            Food fruit = plant->produceFood();
            addFood(fruit);  // Add to tile's food vector
        }
    }
}
```

### 4.3 Rendering Support

**Extend RenderTypes.hpp:**

```cpp
// Add to EntityType enum
enum class EntityType {
    CREATURE = 0,
    FOOD_APPLE,
    FOOD_BANANA,
    FOOD_CORPSE,
    
    // Plant types
    PLANT_BERRY_BUSH,
    PLANT_OAK_TREE,
    PLANT_GRASS,
    PLANT_THORN_BUSH,
    PLANT_GENERIC,    // Fallback for unknown species
    
    COUNT
};
```

**Rendering Strategy:**

```cpp
// In Tile rendering code

void renderTile(const Tile& tile, int x, int y) {
    // 1. Render terrain
    renderTerrain(tile.getTerrainType(), x, y);
    
    // 2. Render plants (if any)
    const auto& plants = tile.getPlants();
    if (!plants.empty()) {
        // Render top plant (most visible)
        const Plant* topPlant = plants.back();
        renderPlant(topPlant, x, y);
    }
    
    // 3. Render food items
    const auto& food = tile.getFoodVec();
    if (!food.empty()) {
        renderFood(food.back(), x, y);
    }
    
    // 4. Render creatures
    // ...
}

void renderPlant(const Plant* plant, int x, int y) {
    // Get visual properties from phenotype
    char displayChar = plant->getCurrentSize() > 5.0f ? 'T' : '#';  // Tree vs bush
    float hue = plant->getColorHue();  // HSV hue from gene
    Color color = hsvToRgb(hue, 0.8f, 0.8f);  // Convert to RGB
    
    // Render with backend
    renderer->drawChar(x, y, displayChar, color);
    
    // Optional: Show health as alpha/brightness
    if (plant->getHealth() < 0.5f) {
        color.a = static_cast<unsigned char>(plant->getHealth() * 255);
    }
}
```

**ASCII Representation Guide:**

| Plant Type | Size < 3 | Size 3-7 | Size > 7 | Color Hue |
|------------|----------|----------|----------|-----------|
| Berry Bush | `"` (small) | `#` (bush) | `#` (large bush) | 320° (red-purple) |
| Grass | `.` (tiny) | `,` (short) | `:` (tall) | 120° (green) |
| Oak Tree | `i` (sapling) | `t` (young) | `T` (mature) | 100° (yellow-green) |
| Thorn Bush | `*` (small) | `#` (thorny) | `#` (dense) | 80° (lime-green) |

---

## 5. Example Prefab Definitions

### 5.1 Berry Bush (r-strategy)

```cpp
PlantSpeciesTemplate createBerryBushTemplate() {
    PlantSpeciesTemplate berry_bush;
    
    berry_bush.species_name = "berry_bush";
    berry_bush.description = "Small shrub producing edible berries";
    berry_bush.display_char = '#';
    berry_bush.base_color_hue = 320.0f;  // Red-purple
    berry_bush.preferred_terrain = "forest";
    berry_bush.optimal_temp_range = {10.0f, 25.0f};
    
    // Gene ranges (creates variation within species)
    berry_bush.gene_ranges = {
        // Morphology - small to medium size
        {PlantGenes::MAX_SIZE, {1.5f, 3.5f}},
        {PlantGenes::COLOR_HUE, {300.0f, 340.0f}},  // Red to purple range
        {PlantGenes::HARDINESS, {0.2f, 0.5f}},       // Relatively fragile
        
        // Metabolism - moderate needs
        {PlantGenes::GROWTH_RATE, {0.7f, 1.2f}},     // Fast growing
        {PlantGenes::WATER_NEED, {0.4f, 0.7f}},
        {PlantGenes::LIGHT_NEED, {0.3f, 0.6f}},      // Shade tolerant
        {PlantGenes::NUTRIENT_VALUE, {20.0f, 40.0f}}, // Good nutrition
        
        // Reproduction - high output
        {PlantGenes::SEED_PRODUCTION, {4.0f, 8.0f}},  // Many seeds
        {PlantGenes::SPREAD_DISTANCE, {3.0f, 8.0f}},  // Medium spread
        
        // Environment - temperate
        {PlantGenes::TEMP_TOLERANCE_LOW, {0.0f, 10.0f}},
        {PlantGenes::TEMP_TOLERANCE_HIGH, {25.0f, 35.0f}},
        
        // Lifespan - relatively short-lived
        {PlantGenes::LIFESPAN, {2000.0f, 5000.0f}},
        
        // Defense - minimal (r-strategy: invest in reproduction)
        {PlantGenes::DEFENSE_LEVEL, {0.1f, 0.3f}},
        {PlantGenes::REGENERATION_RATE, {0.1f, 0.3f}},
        {PlantGenes::TOXICITY, {0.0f, 0.1f}},
        {PlantGenes::STRUCTURAL_INTEGRITY, {0.1f, 0.3f}},
        
        // Food production - high
        {PlantGenes::FRUIT_PRODUCTION_RATE, {5.0f, 10.0f}},
        {PlantGenes::FRUIT_DROP_AGE, {0.4f, 0.7f}},  // Produces fruit mid-life
    };
    
    return berry_bush;
}
```

**Ecological Role:**
- **Primary food source** for herbivores
- **Fast colonizer** of open spaces
- **Low defense** means easy to eat
- **High fruit production** ensures seed spread via herbivore dispersal
- **Evolution pressure:** May evolve mild toxicity to reduce over-predation

### 6.2 Oak Tree (K-strategy)

```cpp
PlantSpeciesTemplate createOakTreeTemplate() {
    PlantSpeciesTemplate oak_tree;
    
    oak_tree.species_name = "oak_tree";
    oak_tree.description = "Large deciduous tree with acorns";
    oak_tree.display_char = 'T';
    oak_tree.base_color_hue = 100.0f;  // Yellow-green
    oak_tree.preferred_terrain = "forest";
    oak_tree.optimal_temp_range = {5.0f, 25.0f};
    
    oak_tree.gene_ranges = {
        // Morphology - large and sturdy
        {PlantGenes::MAX_SIZE, {7.0f, 10.0f}},
        {PlantGenes::COLOR_HUE, {80.0f, 120.0f}},   // Green-brown range
        {PlantGenes::HARDINESS, {0.6f, 0.9f}},      // Very hardy
        
        // Metabolism - slow growing
        {PlantGenes::GROWTH_RATE, {0.1f, 0.4f}},    // Slow growth
        {PlantGenes::WATER_NEED, {0.5f, 0.8f}},     // Needs water
        {PlantGenes::LIGHT_NEED, {0.6f, 0.9f}},     // Sun-loving
        {PlantGenes::NUTRIENT_VALUE, {40.0f, 80.0f}}, // High nutrition (trunk)
        
        // Reproduction - low frequency, high quality
        {PlantGenes::SEED_PRODUCTION, {1.0f, 3.0f}}, // Few acorns
        {PlantGenes::SPREAD_DISTANCE, {10.0f, 20.0f}}, // Far spread (animals cache acorns)
        
        // Environment - temperate, cold-hardy
        {PlantGenes::TEMP_TOLERANCE_LOW, {-10.0f, 5.0f}},
        {PlantGenes::TEMP_TOLERANCE_HIGH, {20.0f, 30.0f}},
        
        // Lifespan - very long-lived
        {PlantGenes::LIFESPAN, {8000.0f, 10000.0f}},
        
        // Defense - moderate (tough bark)
        {PlantGenes::DEFENSE_LEVEL, {0.5f, 0.8f}},
        {PlantGenes::REGENERATION_RATE, {0.05f, 0.2f}},
        {PlantGenes::TOXICITY, {0.0f, 0.2f}},        // Acorns can be bitter
        {PlantGenes::STRUCTURAL_INTEGRITY, {0.5f, 0.8f}},
        
        // Food production - low but nutritious
        {PlantGenes::FRUIT_PRODUCTION_RATE, {1.0f, 3.0f}},
        {PlantGenes::FRUIT_DROP_AGE, {0.6f, 0.9f}},  // Late-life acorn production
    };
    
    return oak_tree;
}
```

**Ecological Role:**
- **Keystone species** providing habitat and food
- **Long-lived**, dominates mature forests
- **Moderate defense** via tough bark
- **Acorn production** attracts seed-dispersing animals
- **Evolution pressure:** May evolve deeper roots for drought resistance

### 6.3 Grass (Grazer-Adapted)

```cpp
PlantSpeciesTemplate createGrassTemplate() {
    PlantSpeciesTemplate grass;
    
    grass.species_name = "grass";
    grass.description = "Low-growing grass with rapid regeneration";
    grass.display_char = ',';
    grass.base_color_hue = 120.0f;  // Green
    grass.preferred_terrain = "plains";
    grass.optimal_temp_range = {10.0f, 30.0f};
    
    grass.gene_ranges = {
        // Morphology - very small
        {PlantGenes::MAX_SIZE, {0.5f, 1.5f}},
        {PlantGenes::COLOR_HUE, {110.0f, 140.0f}},  // Yellow-green to blue-green
        {PlantGenes::HARDINESS, {0.4f, 0.7f}},
        
        // Metabolism - fast metabolism
        {PlantGenes::GROWTH_RATE, {1.5f, 2.0f}},    // Very fast growth
        {PlantGenes::WATER_NEED, {0.3f, 0.6f}},
        {PlantGenes::LIGHT_NEED, {0.7f, 1.0f}},     // Full sun required
        {PlantGenes::NUTRIENT_VALUE, {5.0f, 15.0f}}, // Low nutrition per bite
        
        // Reproduction - clonal + sexual
        {PlantGenes::SEED_PRODUCTION, {6.0f, 10.0f}}, // Many seeds
        {PlantGenes::SPREAD_DISTANCE, {1.0f, 3.0f}},  // Spreads via runners
        
        // Environment - adaptable
        {PlantGenes::TEMP_TOLERANCE_LOW, {-5.0f, 10.0f}},
        {PlantGenes::TEMP_TOLERANCE_HIGH, {25.0f, 45.0f}},
        
        // Lifespan - short-lived
        {PlantGenes::LIFESPAN, {500.0f, 2000.0f}},
        
        // Defense - grazing tolerance strategy
        {PlantGenes::DEFENSE_LEVEL, {0.0f, 0.2f}},   // Minimal defense
        {PlantGenes::REGENERATION_RATE, {0.6f, 1.0f}}, // **HIGH REGEN**
        {PlantGenes::TOXICITY, {0.0f, 0.0f}},
        {PlantGenes::STRUCTURAL_INTEGRITY, {0.0f, 0.1f}}, // Can be grazed to ground
        
        // Food production - minimal (grass doesn't "fruit")
        {PlantGenes::FRUIT_PRODUCTION_RATE, {0.0f, 1.0f}},
        {PlantGenes::FRUIT_DROP_AGE, {0.8f, 0.9f}},
    };
    
    return grass;
}
```

**Ecological Role:**
- **Ground cover**, prevents soil erosion
- **Grazer-adapted:** Survives being eaten via rapid regeneration
- **Minimal defense**, invests in regrowth instead
- **Fast reproduction** ensures continuous coverage
- **Evolution pressure:** May evolve to produce chemicals that promote grazer digestion (mutualism)

### 6.4 Thorn Bush (Defensive Specialist)

```cpp
PlantSpeciesTemplate createThornBushTemplate() {
    PlantSpeciesTemplate thorn_bush;
    
    thorn_bush.species_name = "thorn_bush";
    thorn_bush.description = "Thorny shrub with toxic sap";
    thorn_bush.display_char = '#';
    thorn_bush.base_color_hue = 80.0f;  // Lime green
    thorn_bush.preferred_terrain = "savanna";
    thorn_bush.optimal_temp_range = {15.0f, 40.0f};
    
    thorn_bush.gene_ranges = {
        // Morphology - medium size, dense
        {PlantGenes::MAX_SIZE, {2.0f, 4.0f}},
        {PlantGenes::COLOR_HUE, {60.0f, 100.0f}},   // Yellow-green
        {PlantGenes::HARDINESS, {0.7f, 1.0f}},      // Very hardy
        
        // Metabolism - slow growing
        {PlantGenes::GROWTH_RATE, {0.2f, 0.5f}},    // Invests energy in defense
        {PlantGenes::WATER_NEED, {0.1f, 0.4f}},     // Drought resistant
        {PlantGenes::LIGHT_NEED, {0.6f, 0.9f}},
        {PlantGenes::NUTRIENT_VALUE, {10.0f, 25.0f}}, // Low nutrition (not worth eating)
        
        // Reproduction - low output
        {PlantGenes::SEED_PRODUCTION, {1.0f, 3.0f}},
        {PlantGenes::SPREAD_DISTANCE, {5.0f, 15.0f}}, // Seeds have hooks for animal transport
        
        // Environment - adapted to harsh conditions
        {PlantGenes::TEMP_TOLERANCE_LOW, {5.0f, 15.0f}},
        {PlantGenes::TEMP_TOLERANCE_HIGH, {30.0f, 50.0f}},
        
        // Lifespan - medium
        {PlantGenes::LIFESPAN, {4000.0f, 7000.0f}},
        
        // Defense - **MAXIMAL**
        {PlantGenes::DEFENSE_LEVEL, {0.8f, 1.0f}},   // Thorns/spines
        {PlantGenes::REGENERATION_RATE, {0.1f, 0.3f}},
        {PlantGenes::TOXICITY, {0.6f, 1.0f}},        // Toxic sap
        {PlantGenes::STRUCTURAL_INTEGRITY, {0.6f, 0.9f}}, // Hard to kill
        
        // Food production - minimal
        {PlantGenes::FRUIT_PRODUCTION_RATE, {0.0f, 2.0f}}, // Rare fruit
        {PlantGenes::FRUIT_DROP_AGE, {0.7f, 0.9f}},
    };
    
    return thorn_bush;
}
```

**Ecological Role:**
- **Defensive specialist:** Discourages herbivory
- **Low palatability** due to toxicity and thorns
- **Long-term survivor** in harsh environments
- **Niche:** Areas with high grazing pressure
- **Evolution pressure:** Arms race with herbivore defense evolution

### 6.5 Registration Function

```cpp
void registerDefaultPlantSpecies() {
    PlantFactory::registerSpecies(createBerryBushTemplate());
    PlantFactory::registerSpecies(createOakTreeTemplate());
    PlantFactory::registerSpecies(createGrassTemplate());
    PlantFactory::registerSpecies(createThornBushTemplate());
    
    // Future species:
    // - Pine tree (coniferous, cold-adapted)
    // - Cactus (desert, water-storing)
    // - Bamboo (ultra-fast growth, clonal)
    // - Flowering plant (attracts pollinators, high seed production)
    // - Vine (climbs other plants, parasitic tendencies)
}
```

---

## 7. Implementation Plan

### 7.1 Task Breakdown

1. **Extend Plant class with defense/food production genes** (2-3 days)
   - Add new gene constants to PlantGenes.hpp
   - Implement gene definitions in PlantGenes.cpp
   - Add getter methods to Plant class
   - Implement food production logic
   - Implement direct consumption with counter-damage

2. **Create PlantFactory and species templates** (2-3 days)
   - Implement PlantSpeciesTemplate struct
   - Implement PlantFactory class
   - Create 4 default species (berry bush, oak, grass, thorn)
   - Write unit tests for factory

3. **Integrate plants into Tile** (2 days)
   - Add `_plants` vector to Tile
   - Implement addPlant/removePlant methods
   - Update Tile::updatePlants()
   - Implement Tile::produceFoodFromPlants()

4. **Update rendering system** (2 days)
   - Add plant EntityTypes to RenderTypes.hpp
   - Implement renderPlant() function
   - Update tile rendering to show plants
   - Add health/status visualization

5. **Implement SpawnerToPlantConverter** (2 days)
   - Create converter class
   - Implement Spawner→Plant heuristics
   - Add batch conversion tool
   - Test on existing worlds

6. **Creature-Plant interaction** (3-4 days)
   - Add herbivore_defense gene to creatures (if not exists)
   - Implement Creature::eatPlant() method
   - Implement food source selection logic
   - Add UI feedback for plant consumption

7. **Testing and balancing** (3-5 days)
   - Create test scenarios with different plant species
   - Verify evolutionary dynamics (arms race, niches)
   - Balance gene ranges for gameplay
   - Performance testing with 1000+ plants

8. **Documentation and cleanup** (2 days)
   - Update user documentation
   - Add code examples
   - Create migration guide
   - Write release notes

**Total Estimate:** 18-24 days (3-4 weeks at full-time pace, 6-8 weeks part-time)

### 7.2 Testing Strategy

**Unit Tests:**

```cpp
// Test PlantFactory
TEST(PlantFactory, CreateFromSpecies) {
    GeneRegistry registry;
    PlantGenes::registerDefaults(registry);
    registerDefaultPlantSpecies();
    
    auto plant = PlantFactory::createFromSpecies("berry_bush", 0, 0, registry);
    
    ASSERT_NE(plant, nullptr);
    EXPECT_GE(plant->getMaxSize(), 1.5f);
    EXPECT_LE(plant->getMaxSize(), 3.5f);
}

// Test defense mechanics
TEST(Plant, DefenseReducesDamage) {
    Plant plant(0, 0, registry);
    // Set high defense gene
    plant.getGenomeMutable().getGene(PlantGenes::DEFENSE_LEVEL)->setValue(0.9f);
    plant.updatePhenotype();
    
    float damage = 1.0f;
    plant.takeDamage(damage);
    
    EXPECT_GT(plant.getHealth(), 0.5f);  // Defense reduced damage
}

// Test food production
TEST(Plant, ProducesFoodWhenMature) {
    Plant plant(0, 0, registry);
    
    // Age to maturity
    while (!plant.canProduceFood() && plant.isAlive()) {
        EnvironmentState env;
        plant.update(env);
    }
    
    ASSERT_TRUE(plant.canProduceFood());
    Food fruit = plant.produceFood();
    EXPECT_GT(fruit.getCalories(), 0.0f);
}
```

**Integration Tests:**

```cpp
TEST(Integration, PlantCreatureInteraction) {
    // Setup world with plant and creature
    World world(100, 100);
    Creature herbivore(...);
    Plant plant = PlantFactory::createFromSpecies("berry_bush", 50, 50, registry);
    
    // Creature eats plant
    float nutrition = herbivore.eatPlant(&plant);
    
    EXPECT_GT(nutrition, 0.0f);
    EXPECT_LT(plant.getHealth(), 1.0f);  // Plant took damage
}
```

**Performance Tests:**

```cpp
TEST(Performance, Update1000Plants) {
    std::vector<std::unique_ptr<Plant>> plants;
    for (int i = 0; i < 1000; ++i) {
        plants.push_back(PlantFactory::createFromSpecies("grass", i, i, registry));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    EnvironmentState env;
    for (auto& plant : plants) {
        plant->update(env);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 16);  // Must update in < 16ms for 60 FPS
}
```

### 7.3 Milestones and Deliverables

| Milestone | Deliverable | Acceptance Criteria |
|-----------|-------------|---------------------|
| **M1: Core Extensions** | Plant class with defense genes, food production | All unit tests pass, defense reduces damage by expected amount |
| **M2: Factory System** | PlantFactory + 4 species templates | Can create berry bush, oak, grass, thorn; gene values within ranges |
| **M3: World Integration** | Plants in Tile, rendering working | Plants visible on tiles, update loop functional |
| **M4: Creature Interaction** | Herbivores can eat plants, take counter-damage | Creature behavior adapts to plant defenses, toxicity works |
| **M5: Migration Tool** | SpawnerToPlantConverter | Can convert old saves, minimal data loss |
| **M6: Polish** | Documentation, balance, performance | Performance <16ms/frame, gameplay feels balanced |

---

## Conclusion

The plant prefab system architecture provides:

1. **Species-level identity** through gene range templates
2. **Individual variation** within species for evolution
3. **Hybrid food production** enabling diverse plant strategies
4. **Defense mechanisms** creating herbivore-plant arms races
5. **Backward compatibility** with existing Spawner system
6. **Extensibility** for future plant types and mechanics

The design follows SOLID principles, integrates seamlessly with the existing genetics system, and sets the foundation for rich emergent ecological dynamics.

**Next Steps:**
1. Review this document with stakeholders
2. Approve design decisions
3. Begin implementation with M1 (Core Extensions)
4. Iterate based on testing feedback

---

## Appendix: Gene Trade-off Matrix

| High Investment In | Benefits | Trade-offs | Example Species |
|-------------------|----------|------------|-----------------|
| **Defense** | Survive herbivory, long lifespan | Slow growth, low reproduction | Thorn bush, oak |
| **Food Production** | Attract seed dispersers, rapid spread | Vulnerable to over-grazing | Berry bush, fruit trees |
| **Regeneration** | Survive repeated grazing | High metabolism cost, shorter lifespan | Grass, bamboo |
| **Growth Rate** | Quick colonization, beat competitors | Low defense, resource-intensive | Grass, pioneer species |
| **Toxicity** | Deter herbivores | Fewer mutualist relationships | Thorn bush, nightshade |
| **Size** | Shade competitors, survive fire | Slow growth, high water need | Oak, pine |

These trade-offs create evolutionary pressure and prevent any single "optimal" plant strategy from dominating the ecosystem.

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
- [[../reference/api/organisms]] - Organism API reference

**Design:**
- [[coevolution-design]] - Creature-plant coevolutionary dynamics
- [[propagation-design]] - Seed dispersal strategies and mechanisms
- [[resource-allocation-design]] - Energy budget framework theory
