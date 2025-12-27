/**
 * @file SimplexSolver.hpp
 * @brief Generic Linear Programming solver using the Two-Phase Simplex Algorithm
 * 
 * This file implements a self-contained LP solver for the Balance Analysis Tool.
 * It uses the classic two-phase Simplex method to:
 * 
 * 1. **Phase I**: Find a feasible solution (or prove infeasibility)
 *    - Adds artificial variables to each constraint
 *    - Minimizes sum of artificial variables
 *    - If minimum > 0: INFEASIBLE
 *    - If minimum = 0: Feasible starting point found
 * 
 * 2. **Phase II**: Optimize from feasible point
 *    - Uses Phase I basis as starting point
 *    - Iteratively improves objective value
 *    - Terminates at optimal or unbounded
 * 
 * ## Usage Example
 * 
 * @code
 *   SimplexSolver solver;
 *   solver.setNumVariables(2);
 *   solver.setObjective({3, 2}, true);  // maximize 3x + 2y
 *   
 *   solver.addConstraint({1, 1}, SimplexSolver::LEQ, 4);  // x + y <= 4
 *   solver.addConstraint({1, 0}, SimplexSolver::LEQ, 2);  // x <= 2
 *   solver.addConstraint({0, 1}, SimplexSolver::LEQ, 3);  // y <= 3
 *   
 *   auto result = solver.solve();
 *   if (result.status == LPStatus::OPTIMAL) {
 *       // result.solution contains optimal values
 *       // result.objectiveValue contains optimal objective
 *   }
 * @endcode
 * 
 * ## Algorithm Details
 * 
 * **Standard Form Conversion**:
 * - LEQ constraints: add slack variable (+s)
 * - GEQ constraints: subtract surplus variable (-s), add artificial (a)
 * - EQ constraints: add artificial variable (a)
 * 
 * **Pivot Selection**:
 * - Column: Most negative reduced cost (Dantzig's rule)
 * - Row: Minimum ratio test (maintains feasibility)
 * 
 * @see plans/balance-constraint-solver.md for full architecture
 */

#pragma once

#include <vector>
#include <string>
#include <limits>
#include <cmath>

namespace EcoSim {
namespace Balance {

/// Numerical tolerance for floating-point comparisons
constexpr double SIMPLEX_EPSILON = 1e-9;

/**
 * @brief Result status of LP optimization
 */
enum class LPStatus {
    OPTIMAL,        ///< Optimal solution found
    INFEASIBLE,     ///< No feasible solution exists
    UNBOUNDED,      ///< Objective can be made arbitrarily good
    MAX_ITERATIONS  ///< Hit iteration limit without convergence
};

/**
 * @brief Convert LPStatus to string representation
 * @param status The status to convert
 * @return Human-readable string
 */
inline std::string lpStatusToString(LPStatus status) {
    switch (status) {
        case LPStatus::OPTIMAL: return "OPTIMAL";
        case LPStatus::INFEASIBLE: return "INFEASIBLE";
        case LPStatus::UNBOUNDED: return "UNBOUNDED";
        case LPStatus::MAX_ITERATIONS: return "MAX_ITERATIONS";
    }
    return "UNKNOWN";
}

/**
 * @brief Result of LP optimization
 * 
 * Contains the solution status, variable values (if optimal),
 * and diagnostic information for reporting.
 */
struct LPResult {
    LPStatus status;                      ///< Optimization status
    std::vector<double> solution;         ///< Variable values if optimal
    double objectiveValue = 0.0;          ///< Optimal objective value
    std::vector<int> conflictingRows;     ///< Row indices if infeasible
    std::string explanation;              ///< Human-readable explanation
    
    /// Check if solution was found
    bool isOptimal() const { return status == LPStatus::OPTIMAL; }
    
    /// Check if problem is infeasible
    bool isInfeasible() const { return status == LPStatus::INFEASIBLE; }
};

/**
 * @brief Linear Programming solver using the Two-Phase Simplex Algorithm
 * 
 * This class implements a complete LP solver supporting:
 * - Minimization or maximization objectives
 * - LEQ (≤), GEQ (≥), and EQ (=) constraints
 * - Variable bounds
 * - Named variables and constraints for debugging
 * 
 * The implementation uses a dense tableau representation and
 * is suitable for small to medium-sized problems (< 1000 variables).
 * 
 * ## Implementation Notes
 * 
 * The tableau layout is:
 * ```
 * [constraint coefficients | slack/artificial cols | RHS]
 * [objective coefficients  | slack/artificial cols | obj value]
 * ```
 * 
 * Basic variables are tracked in basicVars_, where basicVars_[i]
 * is the column index of the basic variable in row i.
 */
class SimplexSolver {
public:
    /// Constraint type for addConstraint()
    enum ConstraintType {
        LEQ,  ///< Less than or equal (≤)
        GEQ,  ///< Greater than or equal (≥)
        EQ    ///< Equal (=)
    };
    
    /**
     * @brief Default constructor
     * 
     * Creates an empty solver. Call setNumVariables() before adding
     * constraints or setting the objective.
     */
    SimplexSolver() = default;
    
    // =====================
    // Problem Definition
    // =====================
    
    /**
     * @brief Set the number of decision variables
     * 
     * Must be called before adding constraints or setting objective.
     * Clears any existing problem data.
     * 
     * @param n Number of decision variables (must be > 0)
     */
    void setNumVariables(int n);
    
    /**
     * @brief Set the objective function
     * 
     * Objective is: minimize/maximize sum(coeffs[i] * x[i])
     * 
     * @param coeffs Coefficient for each decision variable
     * @param maximize If true, maximize; if false, minimize (default)
     */
    void setObjective(const std::vector<double>& coeffs, bool maximize = false);
    
    /**
     * @brief Add a linear constraint
     * 
     * Constraint is: sum(coeffs[i] * x[i]) <type> rhs
     * 
     * For LEQ: adds slack variable
     * For GEQ: subtracts surplus, adds artificial
     * For EQ: adds artificial variable
     * 
     * @param coeffs Coefficient for each decision variable
     * @param type Constraint type (LEQ, GEQ, EQ)
     * @param rhs Right-hand side value
     * @param name Optional name for debugging/reporting
     */
    void addConstraint(const std::vector<double>& coeffs,
                       ConstraintType type,
                       double rhs,
                       const std::string& name = "");
    
    // =====================
    // Variable Configuration
    // =====================
    
    /**
     * @brief Set bounds for a decision variable
     * 
     * Default bounds are [0, +∞) for all variables.
     * Bounds are converted to constraints internally.
     * 
     * @param var Variable index (0-based)
     * @param lower Lower bound (use -infinity for unbounded)
     * @param upper Upper bound (use +infinity for unbounded)
     */
    void setVariableBounds(int var, double lower, double upper);
    
    /**
     * @brief Set name for a variable
     * 
     * Used in debugging and error messages.
     * 
     * @param var Variable index (0-based)
     * @param name Variable name
     */
    void setVariableName(int var, const std::string& name);
    
    // =====================
    // Solving
    // =====================
    
    /**
     * @brief Solve the LP problem
     * 
     * Runs the two-phase Simplex algorithm:
     * 1. Phase I finds a feasible starting point
     * 2. Phase II optimizes from that point
     * 
     * @param maxIterations Maximum pivots before giving up
     * @return LPResult containing status and solution
     */
    LPResult solve(int maxIterations = 1000);
    
    /**
     * @brief Clear all problem data for reuse
     * 
     * Resets the solver to initial state. Call setNumVariables()
     * again before defining a new problem.
     */
    void clear();
    
    // =====================
    // Accessors (for debugging)
    // =====================
    
    /**
     * @brief Get current number of decision variables
     * @return Number of original decision variables
     */
    int getNumVariables() const { return numVars_; }
    
    /**
     * @brief Get current number of constraints
     * @return Number of constraints (excluding bounds)
     */
    int getNumConstraints() const { return numConstraints_; }
    
    /**
     * @brief Get constraint name by index
     * @param idx Constraint index
     * @return Constraint name (may be empty)
     */
    std::string getConstraintName(int idx) const;
    
    /**
     * @brief Get variable name by index
     * @param idx Variable index
     * @return Variable name (or "x{idx}" if not set)
     */
    std::string getVariableName(int idx) const;
    
private:
    // =====================
    // Problem Data
    // =====================
    
    int numVars_ = 0;              ///< Number of original decision variables
    int numConstraints_ = 0;       ///< Number of constraints
    bool maximize_ = false;        ///< True if maximizing
    
    std::vector<double> objective_;       ///< Objective coefficients
    std::vector<std::string> varNames_;   ///< Variable names
    std::vector<std::string> constraintNames_;  ///< Constraint names
    
    // Constraint storage (before conversion to standard form)
    struct Constraint {
        std::vector<double> coeffs;
        ConstraintType type;
        double rhs;
        std::string name;
    };
    std::vector<Constraint> constraints_;
    
    // Variable bounds
    struct VarBounds {
        double lower = 0.0;
        double upper = std::numeric_limits<double>::infinity();
    };
    std::vector<VarBounds> varBounds_;
    
    // =====================
    // Tableau Data (built during solve)
    // =====================
    
    /// Tableau: [rows=constraints+1][cols=vars+slack+artificial+1]
    /// Last row is objective, last column is RHS
    std::vector<std::vector<double>> tableau_;
    
    /// basicVars_[i] = column index of basic variable in row i
    std::vector<int> basicVars_;
    
    /// Number of slack variables added
    int numSlack_ = 0;
    
    /// Number of artificial variables added
    int numArtificial_ = 0;
    
    /// Starting column of artificial variables
    int artificialStart_ = 0;
    
    /// Total number of columns (excluding RHS)
    int totalCols_ = 0;
    
    // =====================
    // Simplex Operations
    // =====================
    
    /**
     * @brief Convert to standard form and build initial tableau
     * 
     * Standard form is: min c^T x, s.t. Ax = b, x >= 0
     * 
     * Adds slack, surplus, and artificial variables as needed.
     */
    void convertToStandardForm();
    
    /**
     * @brief Select pivot column (entering variable)
     * 
     * Uses Dantzig's rule: most negative reduced cost.
     * 
     * @return Column index to enter, or -1 if optimal
     */
    int selectPivotColumn() const;
    
    /**
     * @brief Select pivot row (leaving variable)
     * 
     * Uses minimum ratio test to maintain feasibility.
     * 
     * @param col Pivot column
     * @return Row index to leave, or -1 if unbounded
     */
    int selectPivotRow(int col) const;
    
    /**
     * @brief Perform pivot operation
     * 
     * Makes the specified element the pivot and performs
     * row operations to maintain row echelon form.
     * 
     * @param row Pivot row
     * @param col Pivot column
     */
    void pivot(int row, int col);
    
    /**
     * @brief Check if current solution is optimal
     * @return true if all reduced costs are non-negative
     */
    bool isOptimal() const;
    
    /**
     * @brief Check if problem is unbounded in given direction
     * @param col Column with negative reduced cost
     * @return true if unbounded
     */
    bool isUnbounded(int col) const;
    
    // =====================
    // Two-Phase Method
    // =====================
    
    /**
     * @brief Phase I: Find initial feasible solution
     * 
     * Minimizes sum of artificial variables. If minimum is 0,
     * a feasible basic solution has been found.
     * 
     * @param maxIterations Maximum iterations
     * @return Result with OPTIMAL (feasible) or INFEASIBLE
     */
    LPResult phaseOne(int maxIterations);
    
    /**
     * @brief Phase II: Optimize from feasible point
     * 
     * Uses the basis from Phase I and optimizes the original
     * objective function.
     * 
     * @param maxIterations Maximum iterations
     * @return Optimization result
     */
    LPResult phaseTwo(int maxIterations);
    
    /**
     * @brief Extract solution values for original variables
     * @return Vector of decision variable values
     */
    std::vector<double> extractSolution() const;
    
    /**
     * @brief Get current objective value from tableau
     * @return Objective value (negated if maximizing)
     */
    double getObjectiveValue() const;
    
    /**
     * @brief Identify conflicting constraints for infeasibility
     * @return Vector of constraint indices
     */
    std::vector<int> identifyConflicts() const;
    
    /**
     * @brief Get the RHS column index
     * @return Index of RHS in tableau
     */
    int rhsCol() const { return totalCols_; }
    
    /**
     * @brief Get the objective row index
     * @return Index of objective row in tableau
     */
    int objRow() const { return numConstraints_; }
};

} // namespace Balance
} // namespace EcoSim
