---
title: Extending the Genetics System
created: 2025-12-24
updated: 2025-12-24
status: complete
audience: developer
type: guide
tags: [genetics, extension, tutorial, genes, organisms]
---

# Extending the Genetics System

This guide explains how to extend EcoSim's genetics system with new genes, organism types, and custom behaviors.

## Table of Contents

- [Adding a New Gene](#adding-a-new-gene)
- [Creating a New Gene Category](#creating-a-new-gene-category)
- [Creating a New Organism Type](#creating-a-new-organism-type)
- [Custom Dominance Patterns](#custom-dominance-patterns)
- [Custom Expression Modulation](#custom-expression-modulation)
- [Best Practices](#best-practices)
- [Testing Your Extensions](#testing-your-extensions)

---

## Adding a New Gene

### Step 1: Define the Gene

Create a [`GeneDefinition`](../../../include/genetics/core/Gene.hpp:107) with appropriate parameters:

```cpp
#include "genetics/core/Gene.hpp"
#include "genetics/core/GeneRegistry.hpp"

using namespace EcoSim::Genetics;

// Define the gene
GeneDefinition aggressionGene(
    "aggression",                       // Unique ID
    ChromosomeType::Behavior,           // Target chromosome
    GeneLimits(0.0f, 1.0f, 0.05f),      // min=0, max=1, creep=0.05
    DominanceType::Incomplete           // Blended inheritance
);

// Add pleiotropy effects (gene affects multiple traits)
aggressionGene.addEffect(EffectBinding(
    "behavior",                         // Target domain
    "attack_damage",                    // Target trait
    EffectType::Multiplicative,         // Effect type
    1.5f                                // Scale factor (50% damage boost at max)
));

aggressionGene.addEffect(EffectBinding(
    "metabolism",                       // Target domain
    "energy_cost",                      // Target trait
    EffectType::Additive,               // Effect type
    0.3f                                // Scale factor (adds to base cost)
));
```

### Step 2: Register the Gene

Add the definition to a [`GeneRegistry`](../../../include/genetics/core/GeneRegistry.hpp:25):

```cpp
GeneRegistry registry;

// Register single gene
registry.registerGene(aggressionGene);

// Or register multiple genes
registry.registerGene(GeneDefinition(
    "pack_hunter",
    ChromosomeType::Behavior,
    GeneLimits(0.0f, 1.0f, 0.05f),
    DominanceType::Complete
));
```

### Step 3: Create Genes in Genomes

When creating organisms, add the gene to their genome:

```cpp
Genome genome;

// Create gene with random value
const auto& def = registry.getDefinition("aggression");
GeneValue randomValue = def.createRandomValue();
Gene gene("aggression", randomValue);

// Add to appropriate chromosome
genome.addGene(gene, ChromosomeType::Behavior);
```

### Step 4: Access via Phenotype

Read expressed values through the [`Phenotype`](../../../include/genetics/expression/Phenotype.hpp:44):

```cpp
Phenotype phenotype(&genome, &registry);

// Update context for accurate expression
EnvironmentState env;
OrganismState state;
state.age_normalized = 0.5f;  // Adult
phenotype.updateContext(env, state);

// Get expressed trait value
float aggression = phenotype.getTrait("aggression");
```

---

## Creating a New Gene Category

To add a cohesive set of related genes, extend [`UniversalGenes`](../../../include/genetics/defaults/UniversalGenes.hpp:36):

### Step 1: Add Gene ID Constants

In `UniversalGenes.hpp`:

```cpp
class UniversalGenes {
public:
    // ... existing genes ...
    
    // ========== SOCIAL GENES (New Category) ==========
    
    /// Pack bonding strength [0.0, 1.0], creep 0.05
    static constexpr const char* PACK_BONDING = "pack_bonding";
    
    /// Territory marking frequency [0.0, 1.0], creep 0.05
    static constexpr const char* TERRITORY_MARKING = "territory_marking";
    
    /// Communication range [0.0, 50.0], creep 2.0
    static constexpr const char* COMMUNICATION_RANGE = "communication_range";
    
    /// Hierarchy awareness [0.0, 1.0], creep 0.05
    static constexpr const char* HIERARCHY_AWARENESS = "hierarchy_awareness";

private:
    // Add registration helper
    static void registerSocialGenes(GeneRegistry& registry);
};
```

### Step 2: Implement Registration

In `UniversalGenes.cpp`:

```cpp
void UniversalGenes::registerSocialGenes(GeneRegistry& registry) {
    // Pack Bonding - affects group cohesion
    GeneDefinition packBonding(
        PACK_BONDING,
        ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f),
        DominanceType::Incomplete
    );
    packBonding.addEffect(EffectBinding("behavior", "group_preference", EffectType::Direct, 1.0f));
    packBonding.addEffect(EffectBinding("reproduction", "mate_loyalty", EffectType::Multiplicative, 0.5f));
    registry.registerGene(std::move(packBonding));
    
    // Territory Marking - affects territorial behavior
    GeneDefinition territoryMarking(
        TERRITORY_MARKING,
        ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f),
        DominanceType::Complete
    );
    territoryMarking.addEffect(EffectBinding("behavior", "territorial_aggression", EffectType::Direct, 1.0f));
    registry.registerGene(std::move(territoryMarking));
    
    // Communication Range - how far signals travel
    GeneDefinition communicationRange(
        COMMUNICATION_RANGE,
        ChromosomeType::Sensory,
        GeneLimits(0.0f, 50.0f, 2.0f),
        DominanceType::Incomplete
    );
    registry.registerGene(std::move(communicationRange));
    
    // Hierarchy Awareness - dominance recognition
    GeneDefinition hierarchyAwareness(
        HIERARCHY_AWARENESS,
        ChromosomeType::Behavior,
        GeneLimits(0.0f, 1.0f, 0.05f),
        DominanceType::Incomplete
    );
    registry.registerGene(std::move(hierarchyAwareness));
}
```

### Step 3: Update Category Lookup

```cpp
void UniversalGenes::initializeCategories() {
    // ... existing categories ...
    
    // Social genes
    s_geneCategories[PACK_BONDING] = GeneCategory::Mobility;  // Or create new category
    s_geneCategories[TERRITORY_MARKING] = GeneCategory::Mobility;
    s_geneCategories[COMMUNICATION_RANGE] = GeneCategory::Mobility;
    s_geneCategories[HIERARCHY_AWARENESS] = GeneCategory::Mobility;
}
```

### Step 4: Call from registerDefaults

```cpp
void UniversalGenes::registerDefaults(GeneRegistry& registry) {
    registerUniversalGenes(registry);
    registerMobilityGenes(registry);
    registerAutotrophyGenes(registry);
    registerHeterotrophyGenes(registry);
    registerReproductionGenes(registry);
    registerSocialGenes(registry);  // Add new category
}
```

---

## Creating a New Organism Type

### Step 1: Implement Required Interfaces

Choose which interfaces your organism needs:

```cpp
#include "genetics/interfaces/IPositionable.hpp"
#include "genetics/interfaces/ILifecycle.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"
#include "genetics/interfaces/IReproducible.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"

namespace EcoSim {
namespace Genetics {

class Fungus : public IPositionable, 
               public ILifecycle, 
               public IGeneticOrganism,
               public IReproducible<std::unique_ptr<Fungus>> {
public:
    Fungus(int x, int y, const GeneRegistry& registry);
    Fungus(int x, int y, const Genome& genome, const GeneRegistry& registry);
    
    // IPositionable
    int getX() const override { return x_; }
    int getY() const override { return y_; }
    void setPosition(int x, int y) override { x_ = x; y_ = y; }
    
    // ILifecycle
    unsigned int getAge() const override { return age_; }
    unsigned int getMaxLifespan() const override;
    float getAgeNormalized() const override;
    bool isAlive() const override { return alive_; }
    void age(unsigned int ticks = 1) override;
    
    // IGeneticOrganism
    const Genome& getGenome() const override { return genome_; }
    Genome& getGenomeMutable() override { return genome_; }
    const Phenotype& getPhenotype() const override { return phenotype_; }
    void updatePhenotype() override;
    
    // IReproducible<Fungus>
    bool canReproduce() const override;
    float getReproductiveUrge() const override;
    std::unique_ptr<Fungus> reproduce(const std::unique_ptr<Fungus>& partner) override;
    
    // Fungus-specific methods
    void update(const EnvironmentState& env);
    void spreadSpores(std::vector<std::unique_ptr<Fungus>>& newFungi, 
                      const GeneRegistry& registry);
    float getDecompositionRate() const;
    
private:
    int x_, y_;
    unsigned int age_ = 0;
    bool alive_ = true;
    float biomass_ = 0.1f;
    
    Genome genome_;
    Phenotype phenotype_;
    const GeneRegistry* registry_;
};

} // namespace Genetics
} // namespace EcoSim
```

### Step 2: Create Organism-Specific Genes (Optional)

If your organism needs unique genes:

```cpp
// FungusGenes.hpp
class FungusGenes {
public:
    static void registerDefaults(GeneRegistry& registry);
    static Genome createRandomGenome(const GeneRegistry& registry);
    
    // Fungus-specific genes
    static constexpr const char* DECOMPOSITION_RATE = "decomposition_rate";
    static constexpr const char* SPORE_PRODUCTION = "spore_production";
    static constexpr const char* MYCELIUM_SPREAD = "mycelium_spread";
    static constexpr const char* TOXICITY = "toxicity";
    static constexpr const char* BIOLUMINESCENCE = "bioluminescence";
};

// FungusGenes.cpp
void FungusGenes::registerDefaults(GeneRegistry& registry) {
    // Decomposition efficiency
    registry.registerGene(GeneDefinition(
        DECOMPOSITION_RATE,
        ChromosomeType::Metabolism,
        GeneLimits(0.1f, 2.0f, 0.1f),
        DominanceType::Incomplete
    ));
    
    // Spore production per cycle
    registry.registerGene(GeneDefinition(
        SPORE_PRODUCTION,
        ChromosomeType::Reproduction,
        GeneLimits(1.0f, 100.0f, 5.0f),
        DominanceType::Incomplete
    ));
    
    // Network spread rate
    registry.registerGene(GeneDefinition(
        MYCELIUM_SPREAD,
        ChromosomeType::Morphology,
        GeneLimits(0.0f, 10.0f, 0.5f),
        DominanceType::Incomplete
    ));
    
    // Poison production
    registry.registerGene(GeneDefinition(
        TOXICITY,
        ChromosomeType::Metabolism,
        GeneLimits(0.0f, 1.0f, 0.05f),
        DominanceType::Complete
    ));
    
    // Light production
    registry.registerGene(GeneDefinition(
        BIOLUMINESCENCE,
        ChromosomeType::Morphology,
        GeneLimits(0.0f, 1.0f, 0.02f),
        DominanceType::Codominant
    ));
}
```

### Step 3: Implement the Organism

```cpp
// Fungus.cpp
Fungus::Fungus(int x, int y, const GeneRegistry& registry)
    : x_(x), y_(y), registry_(&registry) {
    genome_ = FungusGenes::createRandomGenome(registry);
    phenotype_ = Phenotype(&genome_, registry_);
}

Fungus::Fungus(int x, int y, const Genome& genome, const GeneRegistry& registry)
    : x_(x), y_(y), genome_(genome), registry_(&registry) {
    phenotype_ = Phenotype(&genome_, registry_);
}

void Fungus::updatePhenotype() {
    OrganismState state;
    state.age_normalized = getAgeNormalized();
    state.health = biomass_;
    
    EnvironmentState env;
    // Would be passed from world
    
    phenotype_.updateContext(env, state);
    phenotype_.invalidateCache();
}

float Fungus::getDecompositionRate() const {
    return phenotype_.getTrait(FungusGenes::DECOMPOSITION_RATE);
}

std::unique_ptr<Fungus> Fungus::reproduce(const std::unique_ptr<Fungus>& partner) {
    // Sexual reproduction via genome crossover
    Genome offspring_genome = Genome::crossover(genome_, partner->getGenome());
    
    // Apply mutation
    offspring_genome.mutate(0.01f, registry_->getAllDefinitions());
    
    // Random position near parent
    int offset_x = (rand() % 5) - 2;
    int offset_y = (rand() % 5) - 2;
    
    return std::make_unique<Fungus>(x_ + offset_x, y_ + offset_y, 
                                    offspring_genome, *registry_);
}
```

---

## Custom Dominance Patterns

To add new dominance behavior, extend the expression logic in [`Gene::getExpressedValue()`](../../../include/genetics/core/Gene.hpp:64):

### Step 1: Add Enum Value

```cpp
// In GeneticTypes.hpp
enum class DominanceType {
    Complete,
    Incomplete,
    Codominant,
    Overdominant,
    Epistatic,      // New: depends on other genes
    EnvironmentSensitive  // New: varies with environment
};
```

### Step 2: Update Expression Logic

```cpp
// In Gene.cpp
GeneValue Gene::getExpressedValue(DominanceType dominance) const {
    switch (dominance) {
        case DominanceType::Complete: {
            // Stronger allele wins
            return allele1_.expression_strength >= allele2_.expression_strength 
                   ? allele1_.value : allele2_.value;
        }
        case DominanceType::Incomplete: {
            // Average of both alleles
            float v1 = toNumeric(allele1_.value);
            float v2 = toNumeric(allele2_.value);
            return (v1 + v2) / 2.0f;
        }
        case DominanceType::Codominant: {
            // Both expressed - return sum
            float v1 = toNumeric(allele1_.value);
            float v2 = toNumeric(allele2_.value);
            return v1 + v2;
        }
        case DominanceType::Overdominant: {
            // Heterozygote advantage
            float v1 = toNumeric(allele1_.value);
            float v2 = toNumeric(allele2_.value);
            if (isHeterozygous()) {
                return std::max(v1, v2) * 1.1f;  // 10% boost
            }
            return (v1 + v2) / 2.0f;
        }
        // New dominance types
        case DominanceType::Epistatic: {
            // Would need additional context - simplified version
            float v1 = toNumeric(allele1_.value);
            float v2 = toNumeric(allele2_.value);
            return v1 * v2;  // Multiplicative interaction
        }
        case DominanceType::EnvironmentSensitive: {
            // Would need environment context
            // For now, use incomplete as fallback
            float v1 = toNumeric(allele1_.value);
            float v2 = toNumeric(allele2_.value);
            return (v1 + v2) / 2.0f;
        }
        default:
            return allele1_.value;
    }
}
```

---

## Custom Expression Modulation

### Adding Environment Factors

Extend [`Phenotype::applyEnvironmentModulation()`](../../../include/genetics/expression/Phenotype.hpp:173):

```cpp
float Phenotype::applyEnvironmentModulation(float value, 
                                            const std::string& trait_id,
                                            const EnvironmentState& env) const {
    float modulated = value;
    
    // Existing modulations...
    
    // New: Altitude effects (add altitude to EnvironmentState)
    if (trait_id.find("metabolism") != std::string::npos) {
        // Higher altitude = lower oxygen = slower metabolism
        float altitude_factor = 1.0f - (env.altitude / 10000.0f) * 0.2f;
        modulated *= std::max(0.5f, altitude_factor);
    }
    
    // New: Pollution effects (add pollution_level to EnvironmentState)
    if (trait_id.find("health") != std::string::npos) {
        float pollution_factor = 1.0f - env.pollution_level * 0.3f;
        modulated *= std::max(0.6f, pollution_factor);
    }
    
    // New: Seasonal effects on reproduction
    if (trait_id.find("reproduction") != std::string::npos) {
        // Spring/Summer boost
        if (env.season == 1 || env.season == 2) {
            modulated *= 1.2f;
        }
        // Winter penalty
        else if (env.season == 3) {
            modulated *= 0.5f;
        }
    }
    
    return modulated;
}
```

### Adding Organism State Factors

Extend [`Phenotype::applyOrganismStateModulation()`](../../../include/genetics/expression/Phenotype.hpp:182):

```cpp
float Phenotype::applyOrganismStateModulation(float value, 
                                               const OrganismState& org) const {
    float modulated = value;
    
    // Existing modulations...
    
    // New: Stress effects (add stress_level to OrganismState)
    if (org.stress_level > 0.5f) {
        // High stress reduces most traits
        modulated *= 1.0f - (org.stress_level - 0.5f) * 0.4f;
    }
    
    // New: Pregnancy effects
    if (org.is_pregnant) {
        // Metabolic boost during pregnancy
        modulated *= 1.15f;
    }
    
    // New: Social status effects (add is_alpha to OrganismState)
    if (org.is_alpha) {
        // Alpha individuals have enhanced expression
        modulated *= 1.1f;
    }
    
    return modulated;
}
```

---

## Best Practices

### 1. Use Meaningful Gene IDs

```cpp
// ✅ Good: Clear, descriptive IDs
static constexpr const char* COLD_RESISTANCE = "cold_resistance";
static constexpr const char* HUNTING_EFFICIENCY = "hunting_efficiency";

// ❌ Bad: Cryptic abbreviations
static constexpr const char* CR = "cr";
static constexpr const char* HE = "he";
```

### 2. Set Appropriate Limits and Creep

```cpp
// ✅ Good: Limits reflect realistic bounds, creep is ~5% of range
GeneLimits(0.0f, 1.0f, 0.05f)          // Normalized trait: 5% creep
GeneLimits(100.0f, 10000.0f, 500.0f)   // Lifespan: 5% of range

// ❌ Bad: Creep too large relative to range
GeneLimits(0.0f, 1.0f, 0.5f)    // 50% creep will cause wild swings
```

### 3. Document Pleiotropy Effects

```cpp
// ✅ Good: Clear comments on why effects exist
GeneDefinition muscleMass("muscle_mass", ChromosomeType::Morphology,
                          GeneLimits(0.1f, 2.0f, 0.1f), DominanceType::Incomplete);

// More muscle = more strength (primary effect)
muscleMass.addEffect(EffectBinding("locomotion", "movement_speed", 
                                   EffectType::Multiplicative, 0.3f));

// More muscle = higher metabolism (biological cost)
muscleMass.addEffect(EffectBinding("metabolism", "base_energy_cost", 
                                   EffectType::Additive, 0.2f));

// More muscle = slower stamina recovery (oxygen debt)
muscleMass.addEffect(EffectBinding("metabolism", "stamina_recovery", 
                                   EffectType::Multiplicative, -0.1f));
```

### 4. Use the Unified Gene System

```cpp
// ✅ Good: Add to UniversalGenes for consistent access
void UniversalGenes::registerDefaults(GeneRegistry& registry) {
    // All organisms can potentially express any gene
    registerMyNewGenes(registry);
}

// ❌ Avoid: Creating separate registries for each organism type
// (unless truly organism-specific genes are needed)
```

### 5. Test with createRandomGenome()

```cpp
// ✅ Good: Use factory functions for testing
GeneRegistry registry;
UniversalGenes::registerDefaults(registry);

// Test genome creation
Genome testGenome = UniversalGenes::createRandomGenome(registry);

// Verify all genes present
for (const auto& id : registry.getAllGeneIds()) {
    assert(testGenome.hasGene(id));
}
```

---

## Testing Your Extensions

### Unit Test Template

```cpp
#include <cassert>
#include <iostream>
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"

void testCustomGene() {
    GeneRegistry registry;
    
    // Register custom gene
    GeneDefinition customGene(
        "test_gene",
        ChromosomeType::Behavior,
        GeneLimits(0.0f, 100.0f, 5.0f),
        DominanceType::Incomplete
    );
    registry.registerGene(customGene);
    
    // Create genome with gene
    Genome genome;
    Gene gene("test_gene", GeneValue(50.0f));
    genome.addGene(gene, ChromosomeType::Behavior);
    
    // Test expression
    Phenotype phenotype(&genome, &registry);
    float value = phenotype.getTrait("test_gene");
    
    assert(value >= 0.0f && value <= 100.0f);
    std::cout << "Custom gene test passed!" << std::endl;
}

void testInheritance() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create two parent genomes
    Genome parent1 = UniversalGenes::createRandomGenome(registry);
    Genome parent2 = UniversalGenes::createRandomGenome(registry);
    
    // Create offspring
    Genome offspring = Genome::crossover(parent1, parent2, 0.5f);
    
    // Offspring should have same genes as parents
    assert(offspring.getTotalGeneCount() == parent1.getTotalGeneCount());
    
    std::cout << "Inheritance test passed!" << std::endl;
}

void testMutation() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Create genome
    Genome genome = UniversalGenes::createRandomGenome(registry);
    
    // Store original values
    std::unordered_map<std::string, float> original;
    Phenotype phenotype(&genome, &registry);
    for (const auto& id : registry.getAllGeneIds()) {
        original[id] = phenotype.getTrait(id);
    }
    
    // Apply mutation
    genome.mutate(1.0f, registry.getAllDefinitions());  // 100% mutation rate
    phenotype.invalidateCache();
    
    // Check some values changed
    int changed = 0;
    for (const auto& id : registry.getAllGeneIds()) {
        if (phenotype.getTrait(id) != original[id]) {
            changed++;
        }
    }
    
    assert(changed > 0);
    std::cout << "Mutation test passed! " << changed << " genes mutated." << std::endl;
}

int main() {
    testCustomGene();
    testInheritance();
    testMutation();
    return 0;
}
```

### Integration Test

```cpp
void testOrganismWithCustomGenes() {
    // Setup
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    FungusGenes::registerDefaults(registry);  // Custom organism genes
    
    // Create organism
    Fungus fungus(10, 20, registry);
    
    // Simulate lifecycle
    EnvironmentState env;
    env.humidity = 0.8f;  // Fungi like humidity
    env.temperature = 18.0f;
    
    for (int tick = 0; tick < 1000; tick++) {
        fungus.update(env);
        
        if (!fungus.isAlive()) {
            std::cout << "Fungus died at tick " << tick << std::endl;
            break;
        }
        
        // Check phenotype values are reasonable
        float decomp = fungus.getDecompositionRate();
        assert(decomp > 0.0f && decomp < 10.0f);
    }
    
    std::cout << "Integration test passed!" << std::endl;
}
```

### Compile and Run Tests

```bash
# Compile test file
g++ -std=c++17 -I include \
    src/genetics/core/*.cpp \
    src/genetics/defaults/*.cpp \
    src/genetics/expression/*.cpp \
    tests/test_extensions.cpp \
    -o test_extensions

# Run tests
./test_extensions
```

---

## Summary

| Task | Key Steps |
|------|-----------|
| **Add Gene** | 1. Create `GeneDefinition` 2. Register with `GeneRegistry` 3. Add to genome 4. Access via `Phenotype` |
| **Add Category** | 1. Add constants to `UniversalGenes` 2. Implement registration function 3. Update `registerDefaults()` |
| **Add Organism** | 1. Implement interfaces 2. Create organism-specific genes (optional) 3. Implement behavior |
| **Custom Dominance** | 1. Add enum value 2. Update `getExpressedValue()` |
| **Custom Modulation** | 1. Extend `EnvironmentState`/`OrganismState` 2. Update modulation functions |

## Prerequisites

Before extending the genetics system, ensure you understand:
- [[01-architecture]] - Core system architecture
- [[02-getting-started]] - Basic usage patterns
- C++17 features (templates, std::variant, constexpr)

---

## See Also

**Core Documentation:**
- [[01-architecture]] - System architecture overview
- [[02-getting-started]] - Quick start tutorial

**Reference:**
- [[../reference/quick-reference]] - Quick reference cheat sheet
- [[../reference/genes]] - Complete gene catalog
- [[../reference/api/core-classes]] - Core API reference
- [[../reference/api/expression]] - Expression API
- [[../reference/api/organisms]] - Organism API
- [[../reference/api/interfaces]] - Interface definitions

**Design:**
- [[../design/coevolution]] - Coevolution system design
- [[../design/propagation]] - Seed dispersal design
- [[../design/prefab]] - Species template design
