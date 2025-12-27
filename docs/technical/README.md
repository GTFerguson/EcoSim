---
title: Technical Documentation
aliases: [technical, technical-index]
created: 2025-12-24
updated: 2025-12-24
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
| [[design/resource-allocation-design]] | Energy budget framework | ✅ Available |
| [[systems/energy-budget]] | Energy system details | 🚧 Coming |
| [[systems/dispersal]] | Seed dispersal mechanics | 🚧 Coming |

### Design (Understanding Why)

Design rationale explaining the philosophy behind implementation decisions.

| Document | Description | Status |
|----------|-------------|--------|
| [[design/coevolution-design]] | Creature-plant interactions | ✅ Available |
| [[design/propagation-design]] | Seed dispersal philosophy | ✅ Available |
| [[design/prefab-design]] | Plant factory architecture | ✅ Available |
| [[design/resource-allocation-design]] | Energy budget design | ✅ Available |
| [[design/emergent-traits]] | Emergent vs categorical design | 🚧 Coming |

## 🎯 Find What You Need

**I want to...**

| Goal | Go To |
|------|-------|
| Add a new gene | [[core/03-extending#adding-a-new-gene]] |
| Create a new organism type | [[core/03-extending#creating-a-new-organism-type]] |
| Look up a class API | [[reference/api/core-classes]] |
| Find gene properties | [[reference/genes]] |
| Understand energy costs | [[design/resource-allocation-design]] |
| See future roadmap | [[../future/genetics-phase3-roadmap]] |
| Understand design choices | [[design/coevolution-design]] |

## 📖 Learning Paths

### Path 1: Quick Implementation

For developers who want to use the system ASAP:
1. [[core/01-architecture]] (30 min) - Understand the architecture
2. [[core/03-extending]] (30 min) - Learn extension patterns
3. Start coding!

### Path 2: Deep Understanding

For developers who want to understand the architecture:
1. [[core/01-architecture]] (30 min) - System design
2. [[design/resource-allocation-design]] (20 min) - Energy framework
3. [[design/coevolution-design]] (40 min) - Design philosophy
4. [[core/03-extending]] (30 min) - Extension guide

### Path 3: System Integration

For developers integrating genetics into the simulation:
1. [[core/01-architecture]] (30 min) - Overview
2. [[reference/api/core-classes]] (browse) - Find relevant APIs
3. [[../future/genetics-phase3-roadmap]] (10 min) - Future roadmap

## 🔧 Source Code Reference

```
include/genetics/
├── core/           # Gene, Chromosome, Genome, Registry
├── expression/     # Phenotype, caching, state
├── interfaces/     # Interface definitions
├── defaults/       # Gene definitions
├── organisms/      # Plant, factories
└── interactions/   # Feeding, dispersal, coevolution

src/genetics/       # Implementations mirror include/
```

## ℹ️ Document Status Legend

| Status | Meaning |
|--------|---------|
| ✅ Available | Fully written and current |
| 🚧 Coming | Being written |
| 📝 Planned | Designed but not written |
| ⚠️ Outdated | Needs update |

---

**Last Updated:** 2025-12-24
**Documentation Version:** 3.0 (Complete)
