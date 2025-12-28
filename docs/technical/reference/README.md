---
title: Reference Documentation
aliases: [reference, reference-index]
---

# Reference Documentation

API documentation and gene catalogs for quick lookup.

## Contents

| Document | Description |
|----------|-------------|
| [[quick-reference]] | Cheat sheet for common patterns |
| [[genes]] | Complete gene catalog (92 genes with modulation policies) |
| [[api/]] | Detailed API documentation by category |

## API Reference by Category

| Category | Description |
|----------|-------------|
| [[api/core-classes]] | Gene, Chromosome, Genome, Registry |
| [[api/expression]] | Phenotype, caching, state classes, **trait modulation** |
| [[api/organisms]] | Plant, PlantFactory |
| [[api/interactions]] | Feeding, dispersal, coevolution |
| [[api/interfaces]] | Interface definitions |

## Key Concepts

### Trait Modulation Policies

The system uses policy-based trait modulation to ensure biological correctness. Each gene is assigned a modulation policy that determines how organism state affects trait expression:

| Policy | Description |
|--------|-------------|
| `NEVER` | Physical structure traits - never modulated by state |
| `HEALTH_ONLY` | Metabolic efficiency - affected only by health |
| `ENERGY_GATED` | Production traits - require energy threshold |
| `CONSUMER_APPLIED` | Performance traits - modulated at use-time |

See [[genes#15-trait-modulation-policies]] for complete gene categorization.

## See Also

- [[../core/02-getting-started]] - How to use these APIs
- [[../core/03-extending]] - How to add new features
