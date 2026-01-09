# EcoSim

An evolutionary ecosystem simulation featuring genetic inheritance, autonomous creature behavior, climate-based world generation, and plant-creature coevolution.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20WSL-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Overview

EcoSim is a real-time ecosystem simulation where autonomous creatures with genetic traits compete for resources, find mates through scent-based communication, reproduce with realistic inheritance patterns, and evolve over generations. Watch as natural selection shapes populations and emergent behaviors arise from simple genetic rules in a climate-driven world with 22 distinct biomes.

---

## 🌟 Features

### 🧬 Genetics System
| Feature | Description |
|---------|-------------|
| **88 unique genes** | Control metabolism, size, diet, perception, combat, reproduction, and more |
| **Diploid inheritance** | Dominant/recessive alleles, codominance, and incomplete dominance |
| **Phenotype expression** | Genes combine to produce observable traits with trait modulation policies |
| **Mutation & crossover** | Configurable mutation rates during reproduction |
| **Unified genome** | Shared genetic architecture between creatures and plants |

### 🧠 Behavior System
| Feature | Description |
|---------|-------------|
| **State machine AI** | Creatures transition between hunting, feeding, mating, resting states |
| **Need-driven priorities** | Hunger, thirst, fatigue, and mating urge drive decision-making |
| **Pathfinding** | A* navigation with terrain awareness and environmental sensitivity |
| **Gene-controlled behavior** | Environmental sensitivity gene affects route planning |
| **Emergent speciation** | Populations diverge based on environmental pressures |

### ⚔️ Combat System
| Feature | Description |
|---------|-------------|
| **Attack/defense calculations** | Based on morphology genes (claws, teeth, horns, hide) |
| **Damage types** | Pierce, slash, and blunt with rock-paper-scissors counters |
| **Predator-prey interactions** | Hunting behavior with pursuit and flee thresholds |
| **Health & healing** | Regeneration rate, wound tolerance, and bleeding resistance genes |
| **Corpse generation** | Dead creatures become scavengeable resources |

### 👃 Scent System
| Feature | Description |
|---------|-------------|
| **Multi-channel communication** | Territory, mating, food, and alarm scent types |
| **Pheromone trails** | Creatures follow scent gradients to find mates |
| **Territory marking** | Scent-based territory boundaries |
| **Prey detection** | Predators track prey via scent trails |
| **Olfactory genes** | Scent production, detection, masking, and acuity |

### 🌍 World System (Climate-Based)
| Feature | Description |
|---------|-------------|
| **Procedural generation** | Simplex noise-based continent and elevation generation |
| **22 Whittaker biomes** | From tropical rainforest to ice sheet, based on temperature × moisture |
| **Climate simulation** | Temperature varies by latitude and elevation; moisture by coast distance and rain shadow |
| **Environmental stress** | Organisms suffer fitness penalties outside their tolerance ranges |
| **Biome-specific adaptation** | Creatures and plants spawn with traits matching their biome |
| **Rivers & lakes** | Downhill water flow simulation with lake formation |

**Biome Categories:**
- **Cold:** Ice Sheet, Tundra, Taiga, Boreal Forest, Glacier
- **Temperate:** Temperate Forest, Temperate Rainforest, Temperate Grassland
- **Warm:** Tropical Rainforest, Tropical Seasonal Forest, Savanna
- **Dry:** Hot Desert, Cold Desert, Steppe, Shrubland
- **Alpine:** Alpine Meadow, Alpine Tundra, Mountain Bare
- **Aquatic:** Ocean Deep, Ocean Shallow, Ocean Coast, Freshwater

### 🌱 Plant System
| Feature | Description |
|---------|-------------|
| **Complete lifecycle** | Seed → sprout → mature → seed production → death |
| **Six dispersal strategies** | Wind, animal-fruit, animal-burr, explosive, gravity, vegetative |
| **Photosynthesis** | Energy production based on light level and genes |
| **Berry/fruit production** | Food source for herbivores and frugivores |
| **Defense mechanisms** | Thorns, toxins, and structural defenses |
| **Biome adaptation** | Plant templates matched to biome climate conditions |

### 📊 Statistics & Visualization
| Feature | Description |
|---------|-------------|
| **Population tracking** | Real-time graphs of creature and plant populations |
| **Genome statistics** | Track gene distribution across populations |
| **SDL2 rendering** | Hardware-accelerated 2D rendering |
| **ImGui overlay** | Interactive UI panels for inspection and control |
| **Creature inspector** | Click to view individual creature stats and genome |
| **World info panel** | Biome distribution, temperature, and moisture maps |

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
- [Gene Reference](docs/technical/reference/genes.md) - All 88 genes

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
