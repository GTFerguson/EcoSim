/**
 * @file BalanceOptimizer.hpp
 * @brief High-level orchestrator for LP-based balance optimization
 * 
 * This file ties together all components of the Balance Constraint Solver:
 * - BalanceConstraintRegistry: Variable and constraint management
 * - SimplexSolver: LP algorithm implementation
 * - ConstraintBuilders: Factory pattern for constraint generation
 * 
 * The BalanceOptimizer provides a simple API for running balance analysis:
 * 
 * @code
 *   BalanceOptimizer optimizer;
 *   optimizer.initializeDefaults();  // Use hardcoded game constants
 *   // OR
 *   optimizer.initializeFromGame(trophic, profiler);  // Use live data
 *   
 *   OptimizationResult result = optimizer.optimize();
 *   
 *   if (result.feasible) {
 *       // Apply optimal parameter values
 *   } else {
 *       // Analyze conflicts
 *       for (const auto& conflict : result.conflicts) {
 *           std::cout << conflict.explanation << "\n";
 *       }
 *   }
 *   
 *   std::cout << result.report;  // Human-readable summary
 * @endcode
 * 
 * @see plans/balance-constraint-solver.md for full architecture details
 */

#pragma once

#include "BalanceConstraints.hpp"
#include "SimplexSolver.hpp"
#include "ConstraintBuilders.hpp"

#include <memory>
#include <string>
#include <vector>
#include <sstream>

namespace EcoSim {
namespace Balance {

// Forward declarations for optional game state integration
class TrophicAnalyzer;
class ArchetypeProfiler;

/**
 * @brief Result of balance optimization analysis
 * 
 * Contains the optimization status, optimal parameter values (if feasible),
 * conflict analysis (if infeasible), and a human-readable report.
 */
struct OptimizationResult {
    bool feasible = false;  ///< True if a valid parameter combination exists
    
    /// If feasible: optimal parameter values with optimalValue filled in
    std::vector<BalanceVariable> parameters;
    
    /**
     * @brief Describes a conflict between two constraints
     * 
     * When the LP is infeasible, conflicts identify which constraints
     * cannot be satisfied simultaneously.
     */
    struct Conflict {
        std::string constraint1;    ///< ID of first conflicting constraint
        std::string constraint2;    ///< ID of second conflicting constraint
        double gap;                 ///< How far apart the constraints are
        std::string explanation;    ///< Human-readable explanation
    };
    std::vector<Conflict> conflicts;  ///< List of identified conflicts
    
    /**
     * @brief Status of a constraint category
     * 
     * Summarizes how many constraints in a category are satisfied.
     */
    struct CategoryStatus {
        std::string name;      ///< Category name (e.g., "breeding", "trophic")
        int total = 0;         ///< Total constraints in category
        int satisfied = 0;     ///< Number satisfied with current/optimal values
        std::string status;    ///< "✓ All satisfied", "⚠ 2/3 satisfied", "❌ Conflict"
    };
    std::vector<CategoryStatus> categories;  ///< Status for each category
    
    std::string report;  ///< Human-readable summary of analysis
};

/**
 * @brief High-level orchestrator for balance optimization
 * 
 * BalanceOptimizer coordinates all components of the constraint solver:
 * 1. Initializes variables from game constants or live data
 * 2. Creates and configures constraint builders
 * 3. Runs LP optimization via SimplexSolver
 * 4. Analyzes results and generates reports
 * 
 * ## Usage Modes
 * 
 * **Standalone Testing** (no game dependencies):
 * @code
 *   BalanceOptimizer optimizer;
 *   optimizer.initializeDefaults();
 *   auto result = optimizer.optimize();
 * @endcode
 * 
 * **Integrated Analysis** (with game state):
 * @code
 *   BalanceOptimizer optimizer;
 *   optimizer.initializeFromGame(trophicAnalyzer, archetypeProfiler);
 *   auto result = optimizer.optimize();
 * @endcode
 * 
 * ## Extensibility
 * 
 * Access the registry to add custom variables or constraints:
 * @code
 *   optimizer.getRegistry().registerVariable({...});
 *   optimizer.getRegistry().registerConstraint({...});
 * @endcode
 */
class BalanceOptimizer {
public:
    /**
     * @brief Default constructor
     * 
     * Creates an empty optimizer. Call initializeDefaults() or
     * initializeFromGame() before calling optimize().
     */
    BalanceOptimizer();
    
    /**
     * @brief Destructor
     */
    ~BalanceOptimizer() = default;
    
    // =====================
    // Initialization
    // =====================
    
    /**
     * @brief Initialize from live game state
     * 
     * Extracts current parameter values from the TrophicAnalyzer and
     * ArchetypeProfiler to build variables and configure constraints.
     * 
     * @param trophic Trophic analyzer for efficiency data (optional)
     * @param profiler Archetype profiler for creature data (optional)
     */
    void initializeFromGame(const TrophicAnalyzer* trophic,
                            const ArchetypeProfiler* profiler);
    
    /**
     * @brief Initialize with default game constants
     * 
     * Uses hardcoded values matching the game's default configuration.
     * Suitable for standalone testing without game dependencies.
     * 
     * Registers:
     * - NUTRITION_PER_SIZE: 100.0 (from Corpse.hpp)
     * - BREED_COST: 6.0 (from creature.hpp)
     */
    void initializeDefaults();
    
    // =====================
    // Optimization
    // =====================
    
    /**
     * @brief Run optimization and return result
     * 
     * Workflow:
     * 1. Build all constraints from registered builders
     * 2. Validate registry state
     * 3. Populate SimplexSolver from registry
     * 4. Add objective function (minimize deviation from midpoints)
     * 5. Solve LP
     * 6. Extract solution or analyze conflicts
     * 7. Generate human-readable report
     * 
     * @return OptimizationResult containing status and analysis
     */
    OptimizationResult optimize();
    
    // =====================
    // Registry Access
    // =====================
    
    /**
     * @brief Get mutable access to the constraint registry
     * 
     * Use this to add custom variables or constraints before optimization.
     * 
     * @return Reference to internal registry
     */
    BalanceConstraintRegistry& getRegistry() { return registry_; }
    
    /**
     * @brief Get read-only access to the constraint registry
     * @return Const reference to internal registry
     */
    const BalanceConstraintRegistry& getRegistry() const { return registry_; }
    
private:
    // =====================
    // Initialization Helpers
    // =====================
    
    /**
     * @brief Initialize optimization variables
     * 
     * Called by initializeFromGame() and initializeDefaults().
     * Registers standard balance variables with the registry.
     */
    void initializeVariables();
    
    /**
     * @brief Initialize constraint builders
     * 
     * Creates and registers all standard constraint builders.
     * Called after variables are registered.
     */
    void initializeBuilders();
    
    // =====================
    // Optimization Helpers
    // =====================
    
    /**
     * @brief Add L1 norm objective function to solver
     * 
     * Minimizes sum of |x_i - target_i| using auxiliary variables:
     * x_i = target_i + d⁺_i - d⁻_i
     * Objective: minimize Σ(d⁺_i + d⁻_i)
     */
    void addObjectiveFunction();
    
    /**
     * @brief Extract solution from LP result
     * @param result Output optimization result to populate
     * @param lpResult LP solver result
     */
    void extractSolution(OptimizationResult& result, const LPResult& lpResult);
    
    /**
     * @brief Analyze conflicts when LP is infeasible
     * 
     * Uses conflictHints from constraints to identify and explain
     * which constraints conflict with each other.
     * 
     * @param result Output optimization result to populate
     */
    void analyzeConflicts(OptimizationResult& result);
    
    /**
     * @brief Calculate the "gap" between two constraints
     * 
     * For bounds on the same variable, returns the distance between
     * the upper bound of one and lower bound of the other.
     * 
     * @param c1 First constraint
     * @param c2 Second constraint
     * @return Gap value (positive if constraints don't overlap)
     */
    double calculateConstraintGap(const BalanceConstraint& c1, 
                                  const BalanceConstraint& c2) const;
    
    /**
     * @brief Generate human-readable report
     * 
     * Creates a formatted summary including:
     * - Status (feasible/infeasible)
     * - Variable table with current and optimal values
     * - Constraint satisfaction status by category
     * - Conflict analysis (if infeasible)
     * - Recommendations
     * 
     * @param result Output optimization result to populate report field
     */
    void generateReport(OptimizationResult& result);
    
    /**
     * @brief Compute category status summaries
     * @param result Output optimization result to populate categories
     */
    void computeCategoryStatus(OptimizationResult& result);
    
    // =====================
    // Member Data
    // =====================
    
    BalanceConstraintRegistry registry_;  ///< Central variable/constraint registry
    SimplexSolver solver_;                ///< LP solver instance
    
    /// Owned constraint builders
    std::unique_ptr<BreedingConstraintBuilder> breedingBuilder_;
    std::unique_ptr<TrophicConstraintBuilder> trophicBuilder_;
    std::unique_ptr<SurvivalConstraintBuilder> survivalBuilder_;
    
    /// External references for game integration (optional, may be null)
    const TrophicAnalyzer* trophic_ = nullptr;
    const ArchetypeProfiler* profiler_ = nullptr;
};

} // namespace Balance
} // namespace EcoSim
