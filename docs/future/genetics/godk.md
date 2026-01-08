# GODk - Genetic Organism Development Kit

*Future modding API for EcoSim*

> **Status:** 🔮 Planned — Not yet implemented
> **Target Phase:** Phase 2 (after core system stabilization)
> **Last Updated:** December 2024

## Vision

**"Become the god of your own genetic creations"**

GODk (Genetic Organism Development Kit) will be a comprehensive modding API that empowers players, researchers, and creators to design and simulate their own genetic systems without writing C++ code.

### Core Philosophy

1. **Accessibility** — Define complex genetics through simple JSON
2. **Discoverability** — Visual tools to explore gene interactions
3. **Shareability** — Community marketplace for gene packs
4. **Scientific Accuracy** — Support for real-world genetic concepts
5. **Gameplay Integration** — Custom genes affect simulation meaningfully

---

## Planned Features

### 1. JSON Gene Definitions

Define genes without touching C++ code:

```json
{
  "version": "1.0",
  "gene_pack": {
    "id": "mythical_creatures",
    "name": "Mythical Creatures Gene Pack",
    "author": "community_creator",
    "description": "Adds genes for dragons, phoenixes, and other mythical creatures"
  },
  "genes": [
    {
      "id": "fire_breath",
      "chromosome": "Metabolism",
      "limits": {
        "min": 0.0,
        "max": 1.0,
        "creep": 0.02
      },
      "dominance": "complete",
      "description": "Ability to produce fire from specialized glands",
      "effects": [
        {
          "domain": "behavior",
          "trait": "ranged_attack_damage",
          "type": "multiplicative",
          "scale": 2.0
        },
        {
          "domain": "metabolism",
          "trait": "energy_cost",
          "type": "additive",
          "scale": 0.5
        }
      ],
      "requirements": {
        "min_genes": {
          "heat_resistance": 0.5
        }
      }
    },
    {
      "id": "regeneration",
      "chromosome": "Lifespan",
      "limits": {
        "min": 0.0,
        "max": 1.0,
        "creep": 0.01
      },
      "dominance": "incomplete",
      "description": "Rapid cellular regeneration",
      "effects": [
        {
          "domain": "lifespan",
          "trait": "health_recovery",
          "type": "multiplicative",
          "scale": 3.0
        },
        {
          "domain": "reproduction",
          "trait": "offspring_count",
          "type": "additive",
          "scale": -0.3
        }
      ]
    },
    {
      "id": "flight_capability",
      "chromosome": "Locomotion",
      "limits": {
        "min": 0.0,
        "max": 1.0,
        "creep": 0.03
      },
      "dominance": "incomplete",
      "description": "Wing development and flight muscles",
      "effects": [
        {
          "domain": "locomotion",
          "trait": "movement_speed",
          "type": "multiplicative",
          "scale": 1.5,
          "condition": "terrain != water"
        },
        {
          "domain": "locomotion",
          "trait": "terrain_ignore",
          "type": "threshold",
          "threshold": 0.7,
          "scale": 1.0
        }
      ]
    }
  ],
  "organisms": [
    {
      "id": "dragon",
      "name": "Dragon",
      "base_genome": "creature",
      "gene_expression": {
        "fire_breath": { "min": 0.8, "max": 1.0 },
        "flight_capability": { "min": 0.9, "max": 1.0 },
        "heat_resistance": { "min": 0.7, "max": 1.0 },
        "locomotion": { "min": 0.3, "max": 0.5 }
      },
      "visual": {
        "symbol": "D",
        "color_gene": "color_hue",
        "size_gene": "max_size",
        "size_multiplier": 2.0
      }
    }
  ]
}
```

### 2. Gene Validator

Automated validation catches issues before they break your simulation:

```
$ godk validate my_gene_pack.json

✓ JSON syntax valid
✓ All gene IDs unique
✓ Chromosome assignments valid
✓ Limit ranges reasonable
✓ Effect bindings reference valid traits
✓ No circular gene dependencies

⚠ Warnings:
  - fire_breath: High energy cost (0.5) may cause rapid starvation
  - regeneration: Negative offspring_count effect may prevent reproduction
  
ℹ Suggestions:
  - Consider adding a "fire_gland" gene as prerequisite for fire_breath
  - Balance: fire_breath damage (2.0x) vs energy cost (0.5)
  
Gene Pack Score: 87/100 (Good)
```

**Validation checks:**
- Syntax and schema compliance
- Unique IDs across all loaded packs
- Valid chromosome and effect type references
- Balanced effect scales (warns on extremes)
- Circular dependency detection
- Conflict detection with other loaded packs

### 3. Testing Sandbox

Preview and test custom genes in isolation:

```
┌─────────────────────────────────────────────────────────┐
│  GODk Testing Sandbox                           [×]    │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Gene: fire_breath                                      │
│  ┌─────────────────────────────────────────────────┐   │
│  │ Allele 1: [====●=====] 0.7                      │   │
│  │ Allele 2: [========●=] 0.9                      │   │
│  │ Dominance: Complete                              │   │
│  │ Expressed: 0.9                                   │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  Effects Preview:                                       │
│  ├─ ranged_attack_damage: 1.0 → 2.8 (+180%)           │
│  └─ energy_cost: 1.0 → 1.5 (+50%)                     │
│                                                         │
│  Age Curve:            Environment:                     │
│  ──●────────────       Temp: [======●===] 25°C         │
│   └─ Juvenile          Season: Summer                   │
│                                                         │
│  [Simulate 100 Generations]  [Export Results]           │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**Sandbox features:**
- Real-time expression preview
- Age modulation visualization
- Environment effect simulation
- Multi-generation evolution preview
- Population statistics graphs
- Export to simulation or data file

### 4. Visual Gene Designer

Node-based visual editor for complex gene interactions:

```
┌─────────────────────────────────────────────────────────┐
│  Gene Designer                                  [×]    │
├─────────────────────────────────────────────────────────┤
│                                                         │
│   ┌──────────┐      ┌──────────┐      ┌──────────┐    │
│   │fire_gland│─────►│fire_breath│────►│  Output  │    │
│   └──────────┘      └──────────┘      │ Traits   │    │
│                           │           └──────────┘    │
│   ┌──────────┐           │                            │
│   │heat_res. │───────────┘                            │
│   └──────────┘                                         │
│                                                         │
│   Gene Properties:                                      │
│   ┌─────────────────────────────────────────────────┐ │
│   │ ID: fire_breath                                  │ │
│   │ Chromosome: [Metabolism ▼]                       │ │
│   │ Dominance: [Complete ▼]                          │ │
│   │ Limits: 0.0 ────────── 1.0  Creep: 0.02         │ │
│   └─────────────────────────────────────────────────┘ │
│                                                         │
│   [+ Add Effect]  [+ Add Requirement]  [Save Gene]     │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 5. Community Marketplace

Share and discover gene packs:

```
┌─────────────────────────────────────────────────────────┐
│  GODk Marketplace                        🔍 Search     │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Featured Packs                                         │
│  ──────────────                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ 🏆 Realistic Mammal Genetics          ★★★★★     │   │
│  │ by: bio_researcher_42                            │   │
│  │ 45 genes | 12 organisms | 5,234 downloads        │   │
│  │ "Scientifically accurate mammal genetics..."     │   │
│  │ [Download] [Preview] [Fork]                      │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ 🎮 Fantasy Creatures                  ★★★★☆     │   │
│  │ by: dragon_master                                │   │
│  │ 28 genes | 8 organisms | 3,891 downloads         │   │
│  │ "Dragons, phoenixes, unicorns, and more..."      │   │
│  │ [Download] [Preview] [Fork]                      │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  Categories: [All] [Realistic] [Fantasy] [Sci-Fi]      │
│  Sort by: [Downloads ▼]                                 │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**Marketplace features:**
- Upload and share gene packs
- Version control and update notifications
- Dependency management (pack requires other packs)
- Ratings and reviews
- Fork and modify existing packs
- Compatibility checking with EcoSim versions

### 6. Behavior Scripting

Attach custom behaviors triggered by gene expression:

```json
{
  "behaviors": [
    {
      "id": "fire_attack",
      "trigger": {
        "gene": "fire_breath",
        "threshold": 0.5,
        "condition": "energy > 0.3 AND target_in_range"
      },
      "action": "ranged_attack",
      "parameters": {
        "damage_gene": "fire_breath",
        "range": 5,
        "energy_cost": 0.1,
        "cooldown_ticks": 50,
        "visual_effect": "fire_projectile"
      }
    },
    {
      "id": "phoenix_rebirth",
      "trigger": {
        "gene": "regeneration",
        "threshold": 0.9,
        "condition": "health <= 0"
      },
      "action": "resurrect",
      "parameters": {
        "health_restored": 0.5,
        "cooldown_ticks": 10000,
        "visual_effect": "fire_explosion"
      }
    }
  ]
}
```

### 7. Research Mode

Export simulation data for scientific analysis:

```json
{
  "research_config": {
    "track_genes": ["fire_breath", "flight_capability", "regeneration"],
    "sample_rate": 100,
    "metrics": [
      "population_size",
      "gene_frequency",
      "allele_diversity",
      "fitness_correlation"
    ],
    "export_format": "csv",
    "output_path": "./research_data/"
  }
}
```

**Research exports:**
- Gene frequency over time
- Allele distribution histograms
- Fitness landscape analysis
- Phylogenetic trees
- Statistical summaries
- Raw genotype data

---

## Technical Architecture

### JSON Schema

GODk will use JSON Schema for validation:

```json
{
  "$schema": "https://ecosim.github.io/godk/schema/v1.0.json",
  "type": "object",
  "required": ["version", "gene_pack", "genes"],
  "properties": {
    "version": { "type": "string", "pattern": "^\\d+\\.\\d+$" },
    "gene_pack": { "$ref": "#/definitions/GenePack" },
    "genes": {
      "type": "array",
      "items": { "$ref": "#/definitions/GeneDefinition" }
    }
  }
}
```

### Runtime Loading

Genes will be loaded at simulation startup:

```cpp
// Future API preview
class GODkLoader {
public:
    // Load gene pack from JSON file
    static bool loadGenePack(const std::string& path, GeneRegistry& registry);
    
    // Load all packs from directory
    static int loadAllPacks(const std::string& directory, GeneRegistry& registry);
    
    // Validate pack without loading
    static ValidationResult validate(const std::string& path);
    
    // Get loaded pack info
    static std::vector<GenePackInfo> getLoadedPacks();
};
```

### Compatibility Layers

GODk will maintain backward compatibility:

| Version | Features | Breaking Changes |
|---------|----------|------------------|
| 1.0 | Basic genes, effects | Initial release |
| 1.1 | Conditional effects | None |
| 1.2 | Behaviors | None |
| 2.0 | Custom chromosomes | Schema v2 |

---

## Development Timeline

### Phase 2.1: Core JSON Support
- [ ] JSON schema definition
- [ ] Basic gene loading from JSON
- [ ] CLI validator tool
- [ ] Documentation

### Phase 2.2: Visual Tools
- [ ] Testing sandbox UI
- [ ] Gene designer (basic)
- [ ] Expression previewer

### Phase 2.3: Advanced Features
- [ ] Behavior scripting
- [ ] Conditional effects
- [ ] Research mode exports

### Phase 2.4: Community Features
- [ ] Marketplace backend
- [ ] User accounts
- [ ] Pack versioning
- [ ] Rating system

---

## API Preview

### Loading Gene Packs

```cpp
#include "godk/GODkLoader.hpp"

int main() {
    GeneRegistry registry;
    
    // Load built-in genes first
    UniversalGenes::registerDefaults(registry);
    
    // Load custom gene packs
    GODkLoader::loadAllPacks("mods/gene_packs/", registry);
    
    // Validate after loading
    auto issues = registry.validateConsistency();
    for (const auto& issue : issues) {
        std::cerr << "Warning: " << issue << std::endl;
    }
    
    // Create simulation with extended genetics
    Simulation sim(registry);
    sim.run();
}
```

### Creating Custom Organisms

```cpp
#include "godk/OrganismFactory.hpp"

// Register custom organism template
OrganismFactory::registerTemplate("dragon", {
    .base = "creature",
    .required_genes = {"fire_breath", "flight_capability"},
    .expression_ranges = {
        {"fire_breath", {0.8f, 1.0f}},
        {"flight_capability", {0.9f, 1.0f}}
    }
});

// Create dragon instance
auto dragon = OrganismFactory::create("dragon", registry, position);
```

---

## FAQ

### When will GODk be available?
GODk is planned for Phase 2 of development, after the core genetics system is stable and well-tested.

### Will GODk break my existing simulations?
No. GODk is an extension layer on top of the existing C++ genetics system. All current functionality will continue to work.

### Can I use GODk for research?
Yes! The research mode is specifically designed for scientific use, with data export capabilities and statistical analysis tools.

### How do I report bugs or request features?
Open an issue on GitHub with the `[GODk]` tag in the title.

---

## See Also

- [[../../technical/genetics/architecture|Genetics System Architecture]] - How the genetics system is designed
- [[../../technical/genetics/gene-reference|Gene Reference]] - Complete gene catalog
- [[../../technical/genetics/extending|Extending the Genetics System]] - How to extend genetics in C++
