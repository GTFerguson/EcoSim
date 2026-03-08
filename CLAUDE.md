# Agent Onboarding Guide

Quick orientation for AI agents working on EcoSim. This is a signpost document - detailed information lives in the referenced docs.

## Project Identity

EcoSim is a C++17 evolutionary ecosystem simulation. Creatures with genetic traits compete for resources, reproduce with inheritance, and evolve over generations in a climate-driven world with 22 biomes.

## Codebase Orientation

```
EcoSim/
├── include/                 # Headers (public API)
│   ├── genetics/            # Core system: genes, phenotypes, organisms
│   │   ├── core/            # Gene, Chromosome, Genome, GeneRegistry
│   │   ├── expression/      # Phenotype, caching, environment state
│   │   ├── organisms/       # Plant, factories
│   │   ├── behaviors/       # State machine behaviors
│   │   └── interactions/    # Combat, feeding, dispersal
│   ├── objects/creature/    # Creature class and subsystems
│   ├── world/               # World, tiles, biomes, environment
│   └── rendering/           # Abstract renderer + backends
├── src/                     # Implementations mirror include/ structure
│   └── main.cpp             # Entry point and game loop
├── docs/                    # Maintained documentation (approval required)
├── plans/                   # Active development notes (no approval needed)
├── external/                # Third-party libs (imgui)
└── build/                   # Build output (cmake)
```

## Documentation Navigation

| Need to understand... | Read |
|-----------------------|------|
| System architecture | `docs/technical/core/01-architecture.md` |
| How to extend/add features | `docs/technical/core/03-extending.md` |
| Specific gene details | `docs/technical/reference/genes.md` |
| A particular system | `docs/technical/systems/<system>.md` |
| Design rationale | `docs/technical/design/<topic>.md` |
| Future plans/roadmap | `docs/future/README.md` |
| User-facing explanations | `docs/user/` |

**API Reference:** `docs/technical/reference/api/` has class-by-class documentation.

## Conventions

Rules are defined in `.roo/rules/` - read these files for full details:

- **Code comments:** Explain WHY, not WHAT. No marker comments (issue numbers, phase references). Self-contained `@todo` only.
- **Markdown:** Blank line before tables (Obsidian compatibility). Use fenced code blocks with language identifiers.
- **Documentation:** YAML frontmatter, Obsidian callouts (`> [!NOTE]`), wiki-links for cross-references.

## Development Workflow

### `plans/` - Working documents
- Create, modify, delete freely during development
- For active planning and work-in-progress
- Delete or migrate when complete

### `docs/` - Curated documentation
- **Requires user approval** for content changes
- Minor fixes (typos, broken links) allowed without approval
- Prompt user at milestones: "Would you like me to update the documentation?"

### Lifecycle
```
plans/ (active) → docs/technical/ (complete)
                → docs/future/ (deferred)
```

## Build Commands

```bash
cmake -B build && cmake --build build    # Build
./build/EcoSim                           # Run
cmake -B build -DECOSIM_BUILD_TESTS=ON   # With tests
```

## Quick Reference

| Task | Location |
|------|----------|
| Add a new gene | `include/genetics/defaults/UniversalGenes.hpp` |
| Modify creature behavior | `include/genetics/behaviors/` + `src/genetics/behaviors/` |
| Change world generation | `include/world/ClimateWorldGenerator.hpp` |
| Add rendering feature | `include/rendering/backends/` |
| Game loop logic | `src/main.cpp` |
| Creature turn logic | `src/main.cpp:advanceSimulation()` |
