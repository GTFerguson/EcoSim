---
title: Balance Analysis System
created: 2025-12-27
updated: 2025-12-27
status: implemented
tags: [balance, testing, optimization, LP]
---

# Balance Analysis System

The Balance Analysis Tool Suite detects and fixes energy flow exploits in EcoSim. It uses Linear Programming (LP) to find optimal parameter values that satisfy all balance constraints.

## Overview

The primary motivation was the "baby cannibalism exploit" where creatures could generate infinite energy by breeding and eating offspring. The system:

1. **Analyzes** trophic energy flow between ecosystem levels
2. **Profiles** each creature archetype's energy economics
3. **Detects** balance exploits automatically
4. **Optimizes** parameters using LP to find valid configurations
5. **Reports** conflicts when no solution exists

## Architecture

```
src/testing/balance/
├── BalanceFramework.hpp      # Core types (TrophicLevel, EnergyFlow, etc.)
├── AnalysisModule.hpp        # Base class for analysis modules
├── TrophicAnalyzer.hpp/.cpp  # Trophic level energy flow analysis
├── ArchetypeProfiler.hpp/.cpp # Per-archetype energy metrics
├── ExploitDetector.hpp/.cpp  # Exploit detection rules
├── SimplexSolver.hpp/.cpp    # LP Simplex algorithm
├── BalanceConstraints.hpp/.cpp # Variable and constraint registry
├── ConstraintBuilders.hpp/.cpp # Factory pattern for constraints
├── BalanceOptimizer.hpp/.cpp # High-level optimization orchestrator
├── ReportGenerator.hpp/.cpp  # Text and CSV output
└── balance_diagnostic.cpp    # Executable entry point
```

## Core Components

### TrophicAnalyzer

Calculates energy flow between trophic levels and verifies ecological plausibility.

**Trophic Levels:**
- `PRODUCER` - Plants (primary production)
- `PRIMARY` - Herbivores (eat plants)
- `SECONDARY` - Carnivores (eat herbivores)
- `TERTIARY` - Apex predators (eat carnivores)
- `DECOMPOSER` - Scavengers (eat corpses)

**Key Metrics:**

| Metric | Formula | Expected Range |
|--------|---------|----------------|
| Transfer Efficiency | output ÷ input | 0.05 - 0.15 (realistic) |
| Trophic Position | Weighted avg of food source positions + 1 | 1.0 - 4.0 |
| Energy Pyramid Ratio | Biomass(L) ÷ Biomass(L-1) | 0.1 - 0.2 |

### ArchetypeProfiler

Generates detailed energy economics for each creature archetype.

**Per-archetype metrics:**
- Metabolism rate and average feeding gains
- Reproduction ROI (offspring value / breeding cost)
- Survival metrics and mortality causes
- Size characteristics

### ExploitDetector

Implements detection rules for balance exploits:

1. **Baby Cannibalism** - Offspring corpse value exceeds breeding cost
2. **Energy Amplification Loop** - Cycle efficiency > 1.0
3. **Corpse Value Imbalance** - Corpse worth more than lifetime consumption
4. **Size-Based Reproduction Exploit** - ROI varies dramatically by size

### SimplexSolver

A two-phase Simplex algorithm for Linear Programming:

```cpp
namespace EcoSim::Balance {

enum class LPStatus {
    OPTIMAL,
    INFEASIBLE,
    UNBOUNDED,
    MAX_ITERATIONS
};

class SimplexSolver {
public:
    void setNumVariables(int n);
    void setObjective(const std::vector<double>& coeffs, bool maximize = false);
    void addConstraint(const std::vector<double>& coeffs,
                       ConstraintType type, double rhs,
                       const std::string& name = "");
    void setVariableBounds(int var, double lower, double upper);
    LPResult solve(int maxIterations = 1000);
};

}
```

**Phase I:** Find feasible solution (or prove infeasible)
**Phase II:** Optimize from feasible point to find optimal

### BalanceConstraintRegistry

Central registry using a Factory/Builder pattern:

```cpp
// Register variables
registry.registerVariable({
    "NUTRITION_PER_SIZE",
    "Corpse::NUTRITION_PER_SIZE",
    100.0,                          // currentValue
    1.0, INFINITY,                  // bounds
    "Calories per unit corpse size",
    "include/world/Corpse.hpp"
});

// Register constraints via builders
BreedingConstraintBuilder breeding;
breeding.setROIBounds(0.4, 0.8);
registry.addBuilder(&breeding);
registry.buildAllConstraints();
```

### Constraint Builders

Modular constraint generation:

- **BreedingConstraintBuilder** - ROI bounds, survivable costs
- **TrophicConstraintBuilder** - Efficiency bounds
- **SurvivalConstraintBuilder** - Metabolism, hunt viability

## Usage

### Running the Diagnostic

```bash
cd src/testing/balance
./compile_balance.sh
./balance_diagnostic
```

### Programmatic Usage

```cpp
#include "TrophicAnalyzer.hpp"
#include "ArchetypeProfiler.hpp"
#include "ExploitDetector.hpp"
#include "BalanceOptimizer.hpp"
#include "ReportGenerator.hpp"

int main() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    
    TrophicAnalyzer trophic(registry);
    ArchetypeProfiler profiler(registry);
    ExploitDetector detector;
    
    trophic.analyze();
    profiler.analyze();
    detector.analyzeReproductionROI(profiler.getProfiles());
    
    BalanceReport report;
    trophic.contributeToReport(report);
    profiler.contributeToReport(report);
    detector.contributeToReport(report);
    
    ReportGenerator generator;
    std::cout << generator.generateTextReport(report);
    
    return report.exploits.empty() ? 0 : 1;
}
```

### Adding New Variables

```cpp
registry.registerVariable({
    "METABOLISM_RATE",
    "Creature::BASE_METABOLISM",
    Creature::BASE_METABOLISM,
    0.1, 10.0,
    "Base energy consumption per tick",
    "include/objects/creature/creature.hpp"
});
```

### Adding New Constraint Categories

```cpp
class MovementConstraintBuilder : public ConstraintBuilder {
public:
    std::string category() const override { return "movement"; }
    std::string description() const override {
        return "Movement costs and energy requirements";
    }
    
    std::vector<BalanceConstraint> build(
        const BalanceConstraintRegistry& registry) const override
    {
        std::vector<BalanceConstraint> constraints;
        // Add constraints checking registry.getVariable(...)
        return constraints;
    }
};
```

## Report Output

### Feasible Solution

```
================================================================================
BALANCE OPTIMIZATION ANALYSIS
================================================================================

STATUS: ✓ FEASIBLE - Optimal parameters found

OPTIMAL SOLUTION:
  Parameter              Current    Optimal    Change     
  ─────────────────────────────────────────────────────────
  NUTRITION_PER_SIZE     100.0      250.0     +150.0 (+150%)
  BREED_COST             3.0        3.2       +0.2 (+6.7%)

CONSTRAINT SATISFACTION:
  Category    Status    Details
  ─────────────────────────────────────────────────────────
  breeding    ✓ 3/3     All breeding constraints satisfied
  trophic     ✓ 2/2     All trophic constraints satisfied
```

### Infeasible Solution

When constraints conflict (e.g., breeding ROI max vs trophic efficiency min), the report shows:
- Conflicting constraints with their formulas
- Gap quantification between constraint bounds
- ASCII visualization of the conflict
- Minimum relaxation options required for feasibility

## Limitations

The LP solver handles **linear constraints** only. Constraints must be of the form:

```
a₁x₁ + a₂x₂ + ... ≤ b    (linear ✓)
```

Nonlinear constraints like `x × y ≤ b` (variable products) cannot be handled:

```
// This ROI constraint is nonlinear when size is a variable:
// ROI = (offspring_size × nutrition) / (2 × breed_cost)
```

**Current workaround:** Fix `offspring_size` as a parameter rather than an optimization variable.

**Future solution:** [[nlp-integration]] with NLopt library.

## See Also

- [[nlp-integration]] - Future nonlinear programming support
- [[creature-prefabs]] - Archetype definitions
- [[trophic-analysis]] - Energy flow background
