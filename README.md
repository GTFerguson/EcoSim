# EcoSim

An evolutionary ecosystem simulation featuring genetic inheritance, creature behavior, and plant-creature coevolution.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20WSL-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Overview

EcoSim is a real-time ecosystem simulation where autonomous creatures with genetic traits compete for resources, find mates through scent-based communication, reproduce with realistic inheritance patterns, and evolve over generations. Watch as natural selection shapes populations and emergent behaviors arise from simple genetic rules.

## ✨ Key Features

### Genetics System
- **88 unique genes** controlling behavior, physiology, and reproduction
- **Diploid inheritance** with dominant/recessive alleles and codominance
- **Unified genome** shared between creatures and plants
- **Realistic mutation** with configurable rates

### Creature Behavior
- **Scent-based mating** - creatures emit and follow pheromone trails
- **Need-driven AI** - hunger, thirst, fatigue, and mating urge drive decisions
- **Emergent speciation** - populations diverge based on environmental pressures

### Plant System
- **Six dispersal strategies** - wind, animal-carried, explosive, water, gravity, ant-dispersed
- **Creature-plant coevolution** - fruit appeal vs seed toxicity trade-offs
- **Resource allocation** - energy budgets determine growth vs reproduction

### World
- **Procedurally generated terrain** with multiple biomes
- **Scent layer system** for creature communication
- **Day/night cycle** and seasonal changes (planned)

## 🚀 Quick Start

### Prerequisites
- C++17 compatible compiler (clang++ or g++)
- SDL2 library
- ncurses library (optional, for terminal-only version)

### Build and Run

```bash
# Clone the repository
git clone https://github.com/yourusername/EcoSim.git
cd EcoSim

# Build (SDL2 + ImGui - recommended)
bash comp.sh
./EcoSim

# Alternative: Terminal-only version (ncurses)
bash comp_ncurses.sh
./EcoSim
```

**For detailed build instructions** including platform-specific setup, see **[BUILDING.md](BUILDING.md)**.

### Basic Controls

| Key | Action |
|-----|--------|
| **Arrow keys / HJKL** | Pan camera |
| **Space** | Pause/Resume |
| **A** | Add 100 creatures |
| **F** | Toggle HUD |
| **S** | Save world |
| **Esc** | Quit |

## 📚 Documentation

### For Players

**[User Documentation](docs/user/README.md)** - Learn how the simulation works:
- [Getting Started](docs/user/getting-started.md) - First-time setup guide
- [Understanding Creatures](docs/user/creatures/README.md) - Behavior, needs, lifecycle
- [Understanding Plants](docs/user/plants/README.md) - Growth, reproduction, dispersal
- [Understanding Genetics](docs/user/genetics/README.md) - Inheritance and evolution

### For Developers

**[Technical Documentation](docs/technical/README.md)** - Architecture and API reference:
- [Architecture Overview](docs/technical/core/01-architecture.md) - System design
- [Extending the System](docs/technical/core/03-extending.md) - Add genes/organisms
- [API Reference](docs/technical/reference/api/core-classes.md) - Class documentation
- [Gene Reference](docs/technical/reference/genes.md) - All 67 genes

### Roadmap

**[Future Development](docs/future/README.md)** - Planned features and improvements:
- Environment systems (temperature, wind, light)
- Aquatic life support
- Scripted genetics system (GODk) for modding
- Dynamic world events

## 🧬 Technical Highlights

### Gene Architecture

```
Genome
├── Chromosome (maternal)
│   └── Gene (allele)
└── Chromosome (paternal)
    └── Gene (allele)
        ↓
    Phenotype (expressed trait)
```

### Inheritance Patterns

- **Complete Dominance** - One allele masks another
- **Codominance** - Both alleles express equally  
- **Incomplete Dominance** - Blended expression

### Energy Budget System

Organisms allocate limited energy across competing needs:
- Growth vs Reproduction
- Speed vs Endurance
- Defense vs Attractiveness

## 📁 Project Structure

```
EcoSim/
├── include/genetics/     # Genetics system headers
│   ├── core/             # Gene, Chromosome, Genome
│   ├── expression/       # Phenotype, energy budget
│   ├── organisms/        # Plant, factories
│   └── interactions/     # Feeding, dispersal
├── src/                  # Implementation files
├── docs/                 # Documentation
│   ├── user/             # Player guides
│   ├── technical/        # Developer docs
│   └── future/           # Roadmap
└── additional_documents/ # Research papers
```

## 📄 Research Background

EcoSim was developed as part of academic research into evolutionary simulation. Related publications can be found in `additional_documents/`:
- [Dissertation Paper](additional_documents/Dissertation_Paper.pdf)
- [CGVC Short Paper](additional_documents/CGVC-Short_Paper.pdf)

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.
