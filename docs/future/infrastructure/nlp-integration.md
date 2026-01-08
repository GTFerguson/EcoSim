---
title: NLP Integration for Balance Optimization
created: 2025-12-27
updated: 2025-12-27
status: planned
priority: medium
tags: [balance, optimization, NLP, future]
---

# Nonlinear Programming Integration

## Overview

The current [[balance-system]] uses Linear Programming (LP) via a Simplex solver. However, some balance constraints are inherently nonlinear and cannot be expressed in LP form. This document outlines the planned integration of Nonlinear Programming (NLP) to handle these cases.

## The Problem

### Linear vs Nonlinear Constraints

A constraint is **linear** when it's a sum of variables multiplied by constants:

```
3x + 2y ≤ 10    ← Linear ✓
x - 5y + z = 7  ← Linear ✓
```

A constraint is **nonlinear** when variables multiply each other:

```
x × y ≤ 10      ← Nonlinear ✗ (variable product)
x² + y ≤ 5      ← Nonlinear ✗ (variable squared)
```

### Current Limitation

The ROI formula for breeding creates nonlinear constraints when offspring size is a variable:

```
ROI = (offspring_size × nutrition) / (2 × breed_cost)
```

**With 2 variables (nutrition `n` and breed_cost `b`):**
```
ROI = (1.0 × n) / (2 × b) ≤ 0.90
→ n - 1.8b ≤ 0  ← Linear ✓ (size is fixed constant 1.0)
```

**With 3 variables (size `s`, nutrition `n`, and breed_cost `b`):**
```
ROI = (s × n) / (2 × b) ≤ 0.90
→ s × n - 1.8b ≤ 0  ← Nonlinear ✗ (s × n is variable product)
```

### Why LP Can't Handle This

LP relies on the feasible region forming a **convex polytope** with flat faces. The Simplex algorithm walks corners of this shape, guaranteeing the optimal solution is at a vertex.

With curved boundaries from nonlinear constraints, the optimal might be anywhere on the curve—Simplex cannot find it.

## Recommended Solution: NLopt

### Why NLopt?

| Library | Type | License | Dependencies | Ease of Use |
|---------|------|---------|--------------|-------------|
| **NLopt** | General NLP | LGPL | **None (header-only)** | ⭐⭐⭐⭐⭐ |
| IPOPT | Large-scale NLP | EPL | BLAS, LAPACK, HSL/MUMPS | ⭐⭐ |
| Ceres | Least squares | BSD | Eigen, glog, gflags | ⭐⭐⭐ |
| dlib | General | Boost | None | ⭐⭐⭐⭐ |

**NLopt wins for EcoSim because:**

1. **Zero Dependencies** - Just include the header
2. **Multiple Algorithms** - ~40 algorithms; if one fails, try another
3. **Simple API** - Matches our existing solver interface style
4. **Handles Our Exact Problem** - Variable products work natively

### NLopt Algorithms

For our gradient-free optimization needs:

- `LN_COBYLA` - Constrained Optimization BY Linear Approximations
- `LN_BOBYQA` - Bound Optimization BY Quadratic Approximation
- `GN_DIRECT` - Global optimization (slower but finds global optimum)

## Implementation Plan

### Phase 1: NLopt Integration

1. Add NLopt header to `external/nlopt/` directory
2. Create `NLoptSolver` class with same interface as [`SimplexSolver`](src/testing/balance/SimplexSolver.hpp)
3. Update [`BalanceOptimizer`](src/testing/balance/BalanceOptimizer.hpp) to select solver based on constraint type

### Phase 2: Nonlinear Constraint Support

1. Extend `BalanceConstraint` to mark constraints as linear/nonlinear
2. Add `offspring_size` as a proper optimization variable
3. Update `BreedingConstraintBuilder` to generate nonlinear ROI constraints
4. Test with the cannibalism exploit scenario

### Phase 3: Hybrid Solver

1. Use LP (Simplex) for purely linear problems—faster
2. Automatically switch to NLP when nonlinear constraints detected
3. Provide solver selection override for testing

## API Design

### NLoptSolver Interface

```cpp
namespace EcoSim::Balance {

class NLoptSolver {
public:
    void setNumVariables(int n);
    void setObjective(ObjectiveFunc func, void* data);
    void addConstraint(ConstraintFunc func, void* data, double tolerance);
    void setVariableBounds(int var, double lower, double upper);
    
    // Algorithm selection
    void setAlgorithm(nlopt::algorithm alg);
    
    NLPResult solve(const std::vector<double>& initialGuess,
                    int maxIterations = 1000);
};

}
```

### Constraint Function Signature

```cpp
// Objective: minimize this value
double my_objective(const std::vector<double>& x, 
                    std::vector<double>& grad,  // empty if gradient-free
                    void* data);

// Constraint: must return ≤ 0 when satisfied
double roi_constraint(const std::vector<double>& x,
                      std::vector<double>& grad,
                      void* data) {
    double nutrition = x[0];
    double breed_cost = x[1];
    double offspring_size = x[2];
    
    // Nonlinear constraint: s × n - 1.8 × b ≤ 0
    return offspring_size * nutrition - 1.8 * breed_cost;
}
```

## Potential Issues

### Local vs Global Optima

NLP's main weakness is getting stuck in local optima:

```
    Value
      ↑
      │ ╭──╮
      │╱    ╲      ╭─╮
      │      ╲ A  ╱   ╲
      │       ╲__╱     ╲__B__  ← B is global optimum
      │                        but solver might stop at A
      └──────────────────────→
                             x
```

**Mitigations:**

1. **Multi-start:** Run from many random starting points, take best
2. **Global algorithms:** Use `GN_DIRECT` or `GN_ISRES` (slower)
3. **Accept "good enough":** For balance, we need *valid* parameters, not necessarily optimal

For EcoSim balance, local optima are acceptable—we just need parameters that satisfy all constraints.

## Alternative Approaches

If NLopt proves insufficient, consider:

### Constraint Programming (OR-Tools CP-SAT)
- Good for "find valid parameters" rather than optimal
- Handles integer constraints naturally (gene values)
- Different mental model than LP/NLP

### Evolutionary Algorithms
- Genetic algorithms, simulated annealing
- Handle ANY constraint formula
- Slower but very flexible
- Good for complex multi-objective balance

## Success Criteria

NLP integration is complete when:

- [ ] `NLoptSolver` passes same test cases as `SimplexSolver`
- [ ] Nonlinear ROI constraint works with 3 variables
- [ ] Solver automatically selects LP vs NLP based on problem
- [ ] Performance acceptable for interactive balance tuning

## See Also

- [[balance-system]] - Current LP implementation
- [[creature-prefabs]] - Archetype definitions affected by balance
