---
title: Organisms API Reference
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: reference
tags: [genetics, api, reference, organisms, plant]
---

# Organisms API Reference

**Audience:** Developer
**Type:** Reference

---

## Overview

This document covers the organism implementation classes. These classes represent living entities in the simulation that use the genetics system:

- **Plant** - Base class for plants using the genetics system
- **PlantFactory** - Factory for creating plants from templates
- **SpeciesTemplate** - Template defining species-specific gene ranges
- **DispersalStrategy** - Seed dispersal method enumeration

All classes are in the `EcoSim::Genetics` namespace.

---

## Plant

**Header:** [`include/genetics/organisms/Plant.hpp`](include/genetics/organisms/Plant.hpp:67)

Base class for plants using the genetics system. Implements multiple interfaces for position, lifecycle, and genetics.

```cpp
namespace EcoSim::Genetics {

class Plant : public IPositionable, public ILifecycle, public IGeneticOrganism {
public:
    // Constructors
    Plant(int x, int y, const GeneRegistry& registry);
    Plant(int x, int y, const Genome& genome, const GeneRegistry& registry);
    
    // IPositionable
    int getX() const override;
    int getY() const override;
    void setPosition(int x, int y) override;
    
    // ILifecycle
    unsigned int getAge() const override;
    unsigned int getMaxLifespan() const override;
    float getAgeNormalized() const override;
    bool isAlive() const override;
    void age(unsigned int ticks = 1) override;
    
    // IGeneticOrganism
    const Genome& getGenome() const override;
    Genome& getGenomeMutable() override;
    const Phenotype& getPhenotype() const override;
    void updatePhenotype() override;
    
    // Plant-specific
    float getGrowthRate() const;
    float getMaxSize() const;
    float getNutrientValue() const;
    float getCurrentSize() const;
    bool canSpreadSeeds() const;
    void update(const EnvironmentState& env);
    void takeDamage(float amount);
    std::unique_ptr<Plant> produceOffspring(const GeneRegistry& registry) const;
    
    // Defense System
    float getToxicity() const;
    float getThornDamage() const;
    float getRegrowthRate() const;
    bool canRegenerate() const;
    void regenerate();
    
    // Fruit Production (for seed dispersal attraction)
    bool canProduceFruit() const;
    float getFruitProductionRate() const;
    float getFruitAppeal() const;
    
    // Seed Properties
    float getSeedMass() const;
    float getSeedAerodynamics() const;
    float getSeedHookStrength() const;
    float getSeedCoatDurability() const;
    DispersalStrategy getPrimaryDispersalStrategy() const;
    
    // Energy Budget Integration
    void setEnergyState(const EnergyState& state);
    EnergyState& getEnergyState();
    const EnergyState& getEnergyState() const;
    
    // Rendering
    EntityType getEntityType() const;
    void setEntityType(EntityType type);
    char getRenderCharacter() const;
    Color getRenderColor() const;
    
    // Serialization
    unsigned int getId() const;
    void setId(unsigned int id);
    std::string toString() const;
    static std::unique_ptr<Plant> fromString(const std::string& data, const GeneRegistry& registry);
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `int` | X position in world |
| `y` | `int` | Y position in world |
| `registry` | `const GeneRegistry&` | Reference to gene definitions |
| `genome` | `const Genome&` | Pre-defined genome (optional constructor) |

### Interface Methods

#### IPositionable

| Method | Returns | Description |
|--------|---------|-------------|
| `getX()` | `int` | X coordinate |
| `getY()` | `int` | Y coordinate |
| `setPosition(x, y)` | `void` | Update position |

#### ILifecycle

| Method | Returns | Description |
|--------|---------|-------------|
| `getAge()` | `unsigned int` | Current age in ticks |
| `getMaxLifespan()` | `unsigned int` | Maximum lifespan from genes |
| `getAgeNormalized()` | `float` | Age as 0.0-1.0 fraction |
| `isAlive()` | `bool` | True if not dead |
| `age(ticks)` | `void` | Advance age by ticks |

#### IGeneticOrganism

| Method | Returns | Description |
|--------|---------|-------------|
| `getGenome()` | `const Genome&` | Const genome reference |
| `getGenomeMutable()` | `Genome&` | Mutable genome reference |
| `getPhenotype()` | `const Phenotype&` | Current phenotype |
| `updatePhenotype()` | `void` | Recalculate phenotype |

### Plant-Specific Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getGrowthRate()` | `float` | Growth speed from genes |
| `getMaxSize()` | `float` | Maximum size from genes |
| `getNutrientValue()` | `float` | Food value when eaten |
| `getCurrentSize()` | `float` | Current size (0.0-maxSize) |
| `canSpreadSeeds()` | `bool` | True if mature enough |
| `update(env)` | `void` | Update for one tick |
| `takeDamage(amount)` | `void` | Apply damage from feeding |
| `produceOffspring(registry)` | `std::unique_ptr<Plant>` | Create asexual offspring |

### Defense System Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getToxicity()` | `float` | Toxin production level |
| `getThornDamage()` | `float` | Damage dealt to feeders |
| `getRegrowthRate()` | `float` | Recovery speed after damage |
| `canRegenerate()` | `bool` | True if can regrow |
| `regenerate()` | `void` | Perform regeneration |

### Food Production Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `canProduceFruit()` | `bool` | True if mature and has energy |
| `produceFruit()` | `Food` | Create fruit food object |
| `getFruitProductionRate()` | `float` | Fruits per time unit |
| `getFruitAppeal()` | `float` | Attractiveness to creatures |

### Seed Property Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `getSeedMass()` | `float` | Seed weight (affects distance) |
| `getSeedAerodynamics()` | `float` | Wind dispersal efficiency |
| `getSeedHookStrength()` | `float` | Burr attachment strength |
| `getSeedCoatDurability()` | `float` | Digestive survival rate |
| `getPrimaryDispersalStrategy()` | `DispersalStrategy` | Primary dispersal method |

---

## DispersalStrategy

**Header:** [`include/genetics/organisms/Plant.hpp`](include/genetics/organisms/Plant.hpp:25)

Enumeration of seed dispersal methods.

```cpp
enum class DispersalStrategy {
    GRAVITY,        // Heavy seeds fall near parent
    WIND,           // Light seeds travel on wind
    ANIMAL_FRUIT,   // Seeds pass through digestive system
    ANIMAL_BURR,    // Hooks attach to animal fur
    EXPLOSIVE,      // Pod tension launches seeds
    VEGETATIVE      // Runners/stolons create clonal spread
};
```

| Strategy | Mechanism | Distance | Requirements |
|----------|-----------|----------|--------------|
| `GRAVITY` | Heavy seeds fall | Short | High seed mass |
| `WIND` | Wind carries seeds | Variable | High aerodynamics, low mass |
| `ANIMAL_FRUIT` | Digestive passage | Medium-Long | High fruit appeal, durable coat |
| `ANIMAL_BURR` | Fur attachment | Medium-Long | High hook strength |
| `EXPLOSIVE` | Pod ejection | Medium | Specialized pod structure |
| `VEGETATIVE` | Clonal spread | Short | Regrowth ability |

---

## PlantFactory

**Header:** [`include/genetics/organisms/PlantFactory.hpp`](include/genetics/organisms/PlantFactory.hpp:98)

Factory for creating plants from species templates.

```cpp
namespace EcoSim::Genetics {

class PlantFactory {
public:
    explicit PlantFactory(std::shared_ptr<GeneRegistry> registry);
    
    // Plant creation
    Plant createFromTemplate(const std::string& templateName, int x, int y) const;
    Plant createOffspring(const Plant& parent1, const Plant& parent2, int x, int y) const;
    Plant createRandom(int x, int y) const;
    
    // Template management
    void registerTemplate(const SpeciesTemplate& tmpl);
    const SpeciesTemplate* getTemplate(const std::string& name) const;
    std::vector<std::string> getTemplateNames() const;
    bool hasTemplate(const std::string& name) const;
    void registerDefaultTemplates();
    
    // Pre-built templates
    static SpeciesTemplate createBerryBushTemplate();
    static SpeciesTemplate createOakTreeTemplate();
    static SpeciesTemplate createGrassTemplate();
    static SpeciesTemplate createThornBushTemplate();
};

}
```

### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `registry` | `std::shared_ptr<GeneRegistry>` | Shared pointer to gene registry |

### Plant Creation Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `createFromTemplate(name, x, y)` | `Plant` | Create plant from named template |
| `createOffspring(p1, p2, x, y)` | `Plant` | Create offspring from two parents |
| `createRandom(x, y)` | `Plant` | Create plant with random genes |

### Template Management Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `registerTemplate(tmpl)` | `void` | Add a species template |
| `getTemplate(name)` | `const SpeciesTemplate*` | Get template by name (nullptr if not found) |
| `getTemplateNames()` | `std::vector<std::string>` | List all registered template names |
| `hasTemplate(name)` | `bool` | Check if template exists |
| `registerDefaultTemplates()` | `void` | Register all built-in templates |

### Static Template Creators

| Method | Returns | Description |
|--------|---------|-------------|
| `createBerryBushTemplate()` | `SpeciesTemplate` | r-strategy fruit producer |
| `createOakTreeTemplate()` | `SpeciesTemplate` | K-strategy long-lived tree |
| `createGrassTemplate()` | `SpeciesTemplate` | Fast-growing grazing target |
| `createThornBushTemplate()` | `SpeciesTemplate` | Defensive thorny plant |

---

## SpeciesTemplate

**Header:** [`include/genetics/organisms/PlantFactory.hpp`](include/genetics/organisms/PlantFactory.hpp:35)

Template defining species-specific gene ranges and properties.

```cpp
namespace EcoSim::Genetics {

struct SpeciesTemplate {
    std::string name;                    // e.g., "berry_bush"
    std::string displayName;             // e.g., "Berry Bush"
    std::unordered_map<std::string, std::pair<float, float>> geneRanges;
    float expressionStrengthCreature = 0.1f;
    float expressionStrengthPlant = 1.0f;
    EntityType entityType = EntityType::PLANT_GENERIC;
    char renderCharacter = 'p';
};

}
```

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `name` | `std::string` | Internal identifier (e.g., "berry_bush") |
| `displayName` | `std::string` | Human-readable name (e.g., "Berry Bush") |
| `geneRanges` | `std::unordered_map<...>` | Map of gene ID to (min, max) value pairs |
| `expressionStrengthCreature` | `float` | How visible to creatures (0.0-1.0) |
| `expressionStrengthPlant` | `float` | Plant gene expression strength |
| `entityType` | `EntityType` | Rendering entity type |
| `renderCharacter` | `char` | ASCII representation |

---

## Pre-built Templates

### Berry Bush

| Trait | Value | Strategy |
|-------|-------|----------|
| Growth Rate | High | Fast maturation |
| Fruit Production | High | Maximizes fruit output |
| Lifespan | Short | r-strategy |
| Defenses | Low | No thorns, low toxins |
| Dispersal | Animal Fruit | Relies on creature dispersal |

### Oak Tree

| Trait | Value | Strategy |
|-------|-------|----------|
| Growth Rate | Low | Slow maturation |
| Hardiness | High | Damage resistant |
| Lifespan | Long | K-strategy |
| Max Size | Large | Dominates canopy |
| Dispersal | Gravity | Heavy seeds, local spread |

### Grass

| Trait | Value | Strategy |
|-------|-------|----------|
| Growth Rate | Very High | Rapid recovery |
| Regrowth | Maximum | Grazing-adapted |
| Lifespan | Medium | Perennial |
| Defenses | None | Tolerates grazing |
| Dispersal | Vegetative | Clonal spread via runners |

### Thorn Bush

| Trait | Value | Strategy |
|-------|-------|----------|
| Thorn Density | High | Physical defense |
| Toxin Production | High | Chemical defense |
| Lifespan | Medium | Moderate investment |
| Fruit Appeal | Low | Deters browsing |
| Dispersal | Animal Burr | Hooks to fur |

---

## Usage Example

```cpp
#include "genetics/organisms/PlantFactory.hpp"

using namespace EcoSim::Genetics;

// Create factory with registry
auto registry = std::make_shared<GeneRegistry>();
// ... register genes ...

PlantFactory factory(registry);
factory.registerDefaultTemplates();

// Create plants from templates
Plant berryBush = factory.createFromTemplate("berry_bush", 10, 20);
Plant oakTree = factory.createFromTemplate("oak_tree", 15, 25);

// Create offspring from two parents
Plant offspring = factory.createOffspring(berryBush, anotherBush, 12, 22);

// Custom template
SpeciesTemplate cactus;
cactus.name = "cactus";
cactus.displayName = "Desert Cactus";
cactus.geneRanges["WATER_STORAGE"] = {0.8f, 1.0f};
cactus.geneRanges["THORN_DENSITY"] = {0.7f, 0.9f};
cactus.geneRanges["GROWTH_RATE"] = {0.1f, 0.3f};
cactus.entityType = EntityType::PLANT_CACTUS;
cactus.renderCharacter = '*';

factory.registerTemplate(cactus);
Plant desertPlant = factory.createFromTemplate("cactus", 50, 50);
```

---

## See Also

**Core Documentation:**
- [[../../core/01-architecture]] - System architecture overview
- [[../../core/02-getting-started]] - Quick start tutorial
- [[../../core/03-extending]] - Extension guide

**Reference:**
- [[../quick-reference]] - Quick reference cheat sheet
- [[../genes]] - Complete gene catalog
- [[core-classes]] - Core genetics classes
- [[expression]] - Phenotype and state management
- [[interactions]] - Feeding and seed dispersal systems
- [[interfaces]] - Interface definitions

**Design:**
- [[../../design/prefab-design]] - Species template design
- [[../../design/propagation-design]] - Seed dispersal design
