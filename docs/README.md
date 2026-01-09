---
title: EcoSim Documentation
aliases: [documentation, docs-index]
---

# EcoSim Documentation

Welcome to EcoSim - an ecosystem simulation featuring genetic evolution, climate-based world generation, and emergent creature behaviors.

---

## Documentation Sections

### 📖 [User Guide](user/README.md)
Learn how the simulation works, understand creature genetics, and discover how evolution shapes your ecosystem.

**Popular Topics:**
- [Getting Started](user/getting-started.md) - First steps
- [Understanding Genes](user/genetics/genes.md) - How genes work
- [Creature Behavior](user/creatures/behavior.md) - What creatures do
- [World Biomes](user/world/biomes.md) - The 22 biome types
- [Environmental Adaptation](user/world/environmental-adaptation.md) - How creatures survive

### 🛠️ [Technical Documentation](technical/README.md)
Developer documentation covering architecture, API reference, and system design.

**Key Sections:**
- [Architecture Overview](technical/core/01-architecture.md) - System design
- [Gene Reference](technical/reference/genes.md) - Complete gene catalog (89 genes)
- [World System](technical/systems/world-system.md) - Climate-based generation
- [Environmental Stress](technical/systems/environmental-stress.md) - Fitness gradients
- [World-Organism Integration](technical/design/world-organism-integration.md) - How environment affects organisms

---

## Quick Start

### For Users

1. **[Getting Started](user/getting-started.md)** - Run your first simulation
2. **[Usage Guide](user/usage-guide.md)** - Controls and interface
3. **[Understanding the World](user/world/README.md)** - Biomes and climate

### For Developers

1. **[Architecture](technical/core/01-architecture.md)** - System overview
2. **[Getting Started](technical/core/02-getting-started.md)** - Development setup
3. **[Extending EcoSim](technical/core/03-extending.md)** - Adding features

---

## Feature Highlights

### 🧬 Genetics System
- 89 genes controlling traits
- Chromosome-based inheritance
- Mutation and crossover during reproduction
- Emergent diet and dispersal strategies

**Learn more:** [Genetics Guide](user/genetics/README.md) | [Gene Reference](technical/reference/genes.md)

### 🌍 World System
- Climate-based procedural generation
- 22 Whittaker biomes
- Temperature and moisture simulation
- Environmental stress affecting survival

**Learn more:** [Biomes Guide](user/world/biomes.md) | [World System](technical/systems/world-system.md)

### 🧠 Behavior System
- State machine-based AI
- Needs-driven priorities
- Gene-controlled pathfinding
- Environmental awareness

**Learn more:** [Creature Behavior](user/creatures/behavior.md) | [Behavior System](technical/systems/behavior-system.md)

### 🌱 Plant System
- Complete lifecycle (seed → mature → reproduction)
- 6 emergent dispersal strategies
- Energy production via photosynthesis
- Climate adaptation

**Learn more:** [Plant Guide](user/plants/README.md)

### ⚔️ Combat System
- Gene-based attack/defense
- Predator-prey interactions
- Rock-paper-scissors damage types
- Corpse generation and scavenging

**Learn more:** [Combat Design](technical/design/combat-system.md)

---

## Quick Links

### User Documentation

| Topic | Description |
|-------|-------------|
| [Genetics Overview](user/genetics/README.md) | How inheritance works |
| [Creature Needs](user/creatures/needs.md) | What creatures require |
| [Creature Lifecycle](user/creatures/lifecycle.md) | Birth to death |
| [World Biomes](user/world/biomes.md) | 22 climate zones |
| [Environmental Adaptation](user/world/environmental-adaptation.md) | Survival strategies |
| [Scent Communication](user/world/scent-communication.md) | Chemical signals |

### Technical Documentation

| Topic | Description |
|-------|-------------|
| [Gene Reference](technical/reference/genes.md) | Complete gene catalog |
| [World System](technical/systems/world-system.md) | World architecture |
| [Environmental Stress](technical/systems/environmental-stress.md) | Fitness gradients |
| [Behavior System](technical/systems/behavior-system.md) | AI implementation |
| [World Generation](technical/design/world-generation.md) | Climate-based generation |
| [World-Organism Integration](technical/design/world-organism-integration.md) | Environment ↔ organism |

---

## Documentation Map

```
docs/
├── README.md                    ← You are here
├── user/                        ← For users
│   ├── getting-started.md
│   ├── usage-guide.md
│   ├── creatures/               ← Creature behavior & lifecycle
│   ├── genetics/                ← Gene inheritance & evolution
│   ├── plants/                  ← Plant lifecycle & reproduction
│   └── world/                   ← Biomes & environment
└── technical/                   ← For developers
    ├── core/                    ← Architecture & setup
    ├── design/                  ← Design documents
    ├── reference/               ← API & gene reference
    └── systems/                 ← System implementations
```

---

## See Also

- 📂 **[Main Repository](../README.md)** - Project overview and building
- 📂 **[Future Plans](future/README.md)** - Roadmap and improvements
