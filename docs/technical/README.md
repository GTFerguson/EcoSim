---
title: Technical Documentation
aliases: [technical, technical-index]
created: 2025-12-24
updated: 2026-01-11
status: complete
tags: [documentation, index, navigation]
---

# Technical Documentation

Welcome to EcoSim's technical documentation. This guide is organized for different use cases.

## 🚀 Quick Start

**New to the genetics system?** Start here:
1. [[core/01-architecture]] - System overview
2. [[core/02-getting-started]] - Your first gene
3. [[core/03-extending]] - Add custom genes/organisms

## 📚 Documentation Structure

### Core (Essential Reading)

Core developer documentation for understanding and using the genetics system.

| Document | Description | Status |
|----------|-------------|--------|
| [[core/01-architecture]] | System design and SOLID principles | ✅ Available |
| [[core/02-getting-started]] | Quick start tutorial | ✅ Available |
| [[core/03-extending]] | Extension guide | ✅ Available |

### Reference (Quick Lookups)

API documentation and gene catalogs for quick reference during development.

| Document | Description | Status |
|----------|-------------|--------|
| [[reference/api/core-classes]] | Core classes API | ✅ Available |
| [[reference/api/expression]] | Expression system API | ✅ Available |
| [[reference/api/organisms]] | Organisms API | ✅ Available |
| [[reference/api/interactions]] | Interactions API | ✅ Available |
| [[reference/api/interfaces]] | Interfaces API | ✅ Available |
| [[reference/genes]] | Complete gene catalog | ✅ Available |
| [[reference/quick-reference]] | Common patterns cheat sheet | ✅ Available |

### Systems (Deep Dives)

Detailed documentation for individual subsystems.

| Document | Description | Status |
|----------|-------------|--------|
| [[systems/scent-system]] | Olfactory communication | ✅ Available |
| [[systems/world-system]] | World environment and climate | ✅ Available |
| [[systems/water-system]] | Water features and creature drinking | ✅ Available |
| [[systems/environmental-stress]] | Fitness gradients and survival | ✅ Available |
| [[systems/behavior-system]] | State machine AI | ✅ Available |
| [[systems/behavior-state]] | Creature motivation/action states | ✅ Available |
| [[systems/spatial-index]] | O(1) creature queries | ✅ Available |
| [[systems/headless-simulation]] | GUI-free debugging tool | ✅ Available |
| [[design/resource-allocation]] | Energy budget framework | ✅ Available |
| [[systems/energy-budget]] | Energy system details | 🚧 Coming |
| [[systems/dispersal]] | Seed dispersal mechanics | 🚧 Coming |

### Design (Understanding Why)

Design rationale explaining the philosophy behind implementation decisions.

| Document | Description | Status |
|----------|-------------|--------|
| [[design/world-generation]] | Climate-based world generation | ✅ Available |
| [[design/world-organism-integration]] | Environment ↔ organism interaction | ✅ Available |
| [[design/coevolution]] | Creature-plant interactions | ✅ Available |
| [[design/organism]] | Organism interface architecture | ✅ Available |
| [[design/prefab]] | Plant factory architecture | ✅ Available |
| [[design/propagation]] | Seed dispersal philosophy | ✅ Available |
| [[design/resource-allocation]] | Energy budget design | ✅ Available |
| [[design/emergent-traits]] | Emergent vs categorical design | 🚧 Coming |

## 🎯 Find What You Need

**I want to...**

| Goal | Go To |
|------|-------|
| Add a new gene | [[core/03-extending#adding-a-new-gene]] |
| Create a new organism type | [[core/03-extending#creating-a-new-organism-type]] |
| Look up a class API | [[reference/api/core-classes]] |
| Find gene properties | [[reference/genes]] |
| Understand energy costs | [[design/resource-allocation]] |
| Understand world generation | [[design/world-generation]] |
| Learn about environmental stress | [[systems/environmental-stress]] |
| See how environment affects organisms | [[design/world-organism-integration]] |
| Debug creature behavior | [[systems/headless-simulation]] |
| See future roadmap | [[../future/genetics-phase3-roadmap]] |
| Understand design choices | [[design/coevolution]] |

## 📖 Learning Paths

### Path 1: Quick Implementation

For developers who want to use the system ASAP:
1. [[core/01-architecture]] (30 min) - Understand the architecture
2. [[core/03-extending]] (30 min) - Learn extension patterns
3. Start coding!

### Path 2: Deep Understanding

For developers who want to understand the architecture:
1. [[core/01-architecture]] (30 min) - System design
2. [[design/resource-allocation]] (20 min) - Energy framework
3. [[design/coevolution]] (40 min) - Design philosophy
4. [[core/03-extending]] (30 min) - Extension guide

### Path 3: System Integration

For developers integrating genetics into the simulation:
1. [[core/01-architecture]] (30 min) - Overview
2. [[reference/api/core-classes]] (browse) - Find relevant APIs
3. [[../future/genetics-phase3-roadmap]] (10 min) - Future roadmap

### Path 4: World Systems

For developers working with world generation and environment:
1. [[design/world-generation]] (20 min) - Climate-based generation design
2. [[systems/world-system]] (30 min) - World system implementation
3. [[systems/environmental-stress]] (20 min) - Fitness gradients
4. [[design/world-organism-integration]] (30 min) - Environment ↔ organism

## 🔧 Source Code Reference

```
include/genetics/
├── core/           # Gene, Chromosome, Genome, Registry
├── expression/     # Phenotype, caching, state
├── interfaces/     # Interface definitions
├── defaults/       # Gene definitions
├── organisms/      # Plant, factories
└── interactions/   # Feeding, dispersal, coevolution

include/world/
├── BiomeTypes.hpp          # 22 biome definitions
├── ClimateWorldGenerator.hpp # Climate-based generation
├── EnvironmentSystem.hpp   # Environment queries
├── WorldGrid.hpp           # Tile management
├── SeasonManager.hpp       # Seasonal cycles
└── SpatialIndex.hpp        # O(1) entity queries

src/genetics/       # Implementations mirror include/
src/world/          # World system implementations
src/testing/        # Test tools and diagnostics
└── headlessSimulation.cpp  # GUI-free debugging runner
```

## ℹ️ Document Status Legend

| Status | Meaning |
|--------|---------|
| ✅ Available | Fully written and current |
| 🚧 Coming | Being written |
| 📝 Planned | Designed but not written |
| ⚠️ Outdated | Needs update |

---

**Last Updated:** 2026-01-11
**Documentation Version:** 4.1 (Movement System Fixes & Debug Tools)
