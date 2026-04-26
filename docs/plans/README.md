# Plans

Active development notes. No approval required to create/modify/delete here.

---

## 2026-04-26 — Sweep Findings

### Linking Bug — Resolved ✅
- `docs/plans/handoff-organism-linking-bug.md` — Organism method implementations stranded in `creature.cpp` instead of `Organism.cpp`. Fixed.

### Code Quality — Low Priority

| Item | Location | Effort |
|------|----------|--------|
| `dynamic_cast` violations in MovementBehavior | `src/genetics/behaviors/MovementBehavior.cpp:45,187` | Medium |
| `const_cast` lazy-init in OrganismServices | `src/genetics/organisms/OrganismServices.cpp:84-112` | Medium |
| Mixed include guards (`#pragma once` vs `#define`) | ~70% / 30% split across codebase | Low |

### Docs Gaps — Unresolved

| Gap | Where |
|-----|-------|
| OrganismServices `const_cast` pattern not documented | `docs/technical/design/organism.md` only says "lazy", not *how* |
| Remaining 5 `dynamic_cast` violations not listed as debt | `docs/technical/systems/behavior-system.md` documents the rule but not the exceptions |
| Include guard convention not standardized | No convention doc |

---

## Lifecycle

```
plans/ (active) → docs/technical/ (shipped)
                → docs/future/ (deferred)
```

Delete plan docs once their scope ships. Do not leave stale plans.