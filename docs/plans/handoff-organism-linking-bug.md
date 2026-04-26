---
title: "Move Organism method implementations from creature.cpp"
status: pending
priority: high
created: 2026-04-26
owner: agent
---

## Problem

Three `Organism` instance methods have their bodies in `src/objects/creature/creature.cpp` (ecosim_core compilation unit) but are called from code inside `src/genetics/` (ecosim_genetics). The genetics library needs these symbols at link time.

**Confirmed failing build:** `cmake -B build && cmake --build build` emits:

```
undefined reference to `EcoSim::Genetics::Organism::deathCheck() const'
undefined reference to `EcoSim::Genetics::Organism::generateName[abi:cxx11]()'
undefined reference to `EcoSim::Genetics::Organism::updatePhenotypeContext(EnvironmentState const&)'
```

## Files to modify

### 1. `src/genetics/organisms/Organism.cpp` — destination

Append three method implementations before the closing namespace braces (line 444).

### 2. `src/objects/creature/creature.cpp` — source

Remove the same three implementations from these line ranges:

| Method | Approx. location |
|--------|-----------------|
| `Organism::updatePhenotypeContext(EnvironmentState)` | lines 329–350 |
| `Organism::deathCheck() const` | lines 387–405 |
| `Organism::generateName()` | lines 644–682 (end of body — see full extent below) |

---

## Implementation bodies to move

### `updatePhenotypeContext`

```cpp
void EcoSim::Genetics::Organism::updatePhenotypeContext(const EcoSim::Genetics::EnvironmentState& env) {
    EcoSim::Genetics::OrganismState orgState;
    unsigned lifespan = getLifespan();
    orgState.age_normalized = (lifespan > 0) ? static_cast<float>(age_) / static_cast<float>(lifespan) : 0.0f;
    if (heterotrophy_) {
        orgState.energy_level = std::max(0.0f, std::min(1.0f, heterotrophy_->hunger / Constants::RESOURCE_LIMIT));
    }
    if (reproduction_) {
        orgState.reproductive_urge = std::max(0.0f,
            std::min(1.0f, reproduction_->mate / Constants::RESOURCE_LIMIT));
    }
    orgState.health = 1.0f;
    phenotype_.updateContext(env, orgState);

    if (thermal_) thermal_->cacheDirty = true;

    float maxHP = getMaxHealth();
    if (health_ > maxHP) {
        health_ = maxHP;
    }
}
```

### `deathCheck`

```cpp
short EcoSim::Genetics::Organism::deathCheck () const {
  using namespace EcoSim::Genetics::Constants;

  // getMaxLifespan is virtual — Plant overrides to read PlantGenes::LIFESPAN
  // when the universal gene isn't present in the genome. getLifespan reads
  // only the universal trait, which returns the 500000 fallback for plants
  // and breaks plant old-age death. deathCheck must use the virtual form.
  if (age_ > getMaxLifespan())                                         return 1;
  if (heterotrophy_ && heterotrophy_->hunger  < STARVATION_POINT)      return 2;
  if (heterotrophy_ && heterotrophy_->thirst  < DEHYDRATION_POINT)     return 3;
  // Discomfort is a heterotroph-specific cause: reproductive drive
  // collapsing alongside starvation/dehydration. Plants also carry a
  // ReproductionComponent (for maturity / fruit-timer bookkeeping) but
  // their `mate` field has no analogous semantics — it may legally sit
  // negative without meaning the plant is dying.
  return 0;
}
```

### `generateName`

```cpp
std::string EcoSim::Genetics::Organism::generateName () {
  string name = getDietInfo(getDietType()).namePrefix;

  if (ifFlocks()) {
    unsigned  flee      = getFlee ();
    int       diffFlock = flee - getPursue();

    //  Fleeing behaviour
    if (diffFlock > 0) {
      if (flee < 10)  name += "tim";  
      else            name += "gax";

    //  Flocking behaviour
    } else {
      if      (flee < 10) name += "milia";
      else if (flee < 20) name += "micus";
      else                name += "verec";
    }
  }

  //  Movement based names
  if (ifHerds()) {
    name += "on";
  } else if (getSpeed() > 4) {
    name += "ra";
  } else if (getSpeed() > 2) {
    name += "i";
  }

  if (getDietType() == DietType::CARNIVORE) {
    name += "x";
  } else if (getDietType() == DietType::OMNIVORE) {
    name += "z";
  } else if (getDietType() == DietType::HERBIVORE) {
    name += "a";
  } else if (getDietType() == DietType::PLANT_EATER) {
    name += "phy";
  }

  if (getSpeed() < 2) {
    name += "um";
  } else if (getSpeed() >= 4) {
    name += "us";
  }

  return name;
}
```

---

## Verification

After moving, verify with:

```bash
cmake -B build && cmake --build build
```

All three undefined references must disappear.

---

## Why this happened

`Organism` methods were implemented in `creature.cpp` while the class lived in `ecosim_genetics`. Code in `src/genetics/organisms/OrganismFactory.cpp` and `src/genetics/organisms/OrganureFactory.cpp` (and potentially other genetics files) call these methods and need them at link time. When only `ecosim_genetics` is linked — or when the link order doesn't bring in `ecosim_core` early enough — the symbols are unresolved.