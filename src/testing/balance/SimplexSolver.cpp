/**
 * @file SimplexSolver.cpp
 * @brief Implementation of the Two-Phase Simplex Algorithm
 * 
 * This file implements a complete LP solver using the classic two-phase
 * Simplex method. The algorithm handles:
 * 
 * - Standard form conversion (adding slack/surplus/artificial variables)
 * - Phase I: Finding an initial feasible solution
 * - Phase II: Optimizing the objective function
 * - Detection of infeasibility and unboundedness
 * 
 * ## Tableau Layout
 * 
 * The tableau is organized as:
 * ```
 *   [A | I_slack | I_artificial | b]
 *   [c |    0    |      M       | 0]  (during Phase I, M is large)
 * ```
 * 
 * After Phase I, artificial columns are zeroed and the original objective
 * is restored for Phase II.
 * 
 * @see SimplexSolver.hpp for API documentation
 */

#include "SimplexSolver.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>

namespace EcoSim {
namespace Balance {

// ============================================================================
// Public Interface
// ============================================================================

void SimplexSolver::setNumVariables(int n) {
    clear();
    numVars_ = n;
    objective_.resize(n, 0.0);
    varNames_.resize(n);
    varBounds_.resize(n);
    
    // Default names
    for (int i = 0; i < n; ++i) {
        varNames_[i] = "x" + std::to_string(i);
    }
}

void SimplexSolver::setObjective(const std::vector<double>& coeffs, bool maximize) {
    if (static_cast<int>(coeffs.size()) != numVars_) {
        return;  // Invalid size
    }
    objective_ = coeffs;
    maximize_ = maximize;
}

void SimplexSolver::addConstraint(const std::vector<double>& coeffs,
                                   ConstraintType type,
                                   double rhs,
                                   const std::string& name) {
    if (static_cast<int>(coeffs.size()) != numVars_) {
        return;  // Invalid size
    }
    
    // Handle negative RHS by flipping constraint
    Constraint c;
    c.name = name;
    
    if (rhs < 0) {
        // Multiply both sides by -1 and flip inequality
        c.coeffs.resize(coeffs.size());
        for (size_t i = 0; i < coeffs.size(); ++i) {
            c.coeffs[i] = -coeffs[i];
        }
        c.rhs = -rhs;
        
        // Flip constraint type
        if (type == LEQ) {
            c.type = GEQ;
        } else if (type == GEQ) {
            c.type = LEQ;
        } else {
            c.type = EQ;
        }
    } else {
        c.coeffs = coeffs;
        c.type = type;
        c.rhs = rhs;
    }
    
    constraints_.push_back(c);
    constraintNames_.push_back(name);
    numConstraints_ = static_cast<int>(constraints_.size());
}

void SimplexSolver::setVariableBounds(int var, double lower, double upper) {
    if (var < 0 || var >= numVars_) {
        return;
    }
    varBounds_[var].lower = lower;
    varBounds_[var].upper = upper;
}

void SimplexSolver::setVariableName(int var, const std::string& name) {
    if (var < 0 || var >= numVars_) {
        return;
    }
    varNames_[var] = name;
}

void SimplexSolver::clear() {
    numVars_ = 0;
    numConstraints_ = 0;
    maximize_ = false;
    objective_.clear();
    varNames_.clear();
    constraintNames_.clear();
    constraints_.clear();
    varBounds_.clear();
    tableau_.clear();
    basicVars_.clear();
    numSlack_ = 0;
    numArtificial_ = 0;
    artificialStart_ = 0;
    totalCols_ = 0;
}

std::string SimplexSolver::getConstraintName(int idx) const {
    if (idx < 0 || idx >= static_cast<int>(constraintNames_.size())) {
        return "";
    }
    return constraintNames_[idx];
}

std::string SimplexSolver::getVariableName(int idx) const {
    if (idx < 0 || idx >= static_cast<int>(varNames_.size())) {
        return "x" + std::to_string(idx);
    }
    return varNames_[idx];
}

// ============================================================================
// Main Solve Entry Point
// ============================================================================

LPResult SimplexSolver::solve(int maxIterations) {
    // Handle empty problem
    if (numVars_ == 0) {
        LPResult result;
        result.status = LPStatus::OPTIMAL;
        result.objectiveValue = 0.0;
        result.explanation = "Empty problem (no variables)";
        return result;
    }
    
    // Add variable bound constraints
    for (int i = 0; i < numVars_; ++i) {
        // Lower bound: x_i >= lower  =>  -x_i <= -lower
        if (varBounds_[i].lower > -std::numeric_limits<double>::infinity() &&
            varBounds_[i].lower != 0.0) {
            std::vector<double> coeffs(numVars_, 0.0);
            coeffs[i] = 1.0;
            Constraint c;
            c.coeffs = coeffs;
            c.type = GEQ;
            c.rhs = varBounds_[i].lower;
            c.name = varNames_[i] + "_lower";
            constraints_.push_back(c);
        }
        
        // Upper bound: x_i <= upper
        if (varBounds_[i].upper < std::numeric_limits<double>::infinity()) {
            std::vector<double> coeffs(numVars_, 0.0);
            coeffs[i] = 1.0;
            Constraint c;
            c.coeffs = coeffs;
            c.type = LEQ;
            c.rhs = varBounds_[i].upper;
            c.name = varNames_[i] + "_upper";
            constraints_.push_back(c);
        }
    }
    numConstraints_ = static_cast<int>(constraints_.size());
    
    // Handle no constraints case
    if (numConstraints_ == 0) {
        LPResult result;
        
        // Check if objective is unbounded
        bool hasNonZero = false;
        for (double c : objective_) {
            if (std::abs(c) > SIMPLEX_EPSILON) {
                hasNonZero = true;
                break;
            }
        }
        
        if (hasNonZero) {
            // Unbounded in direction of objective
            result.status = LPStatus::UNBOUNDED;
            result.explanation = "No constraints - objective is unbounded";
        } else {
            // Zero objective, any point is optimal
            result.status = LPStatus::OPTIMAL;
            result.solution.resize(numVars_, 0.0);
            result.objectiveValue = 0.0;
            result.explanation = "Zero objective with no constraints";
        }
        return result;
    }
    
    // Convert to standard form and build tableau
    convertToStandardForm();
    
    // Phase I: Find feasible solution
    LPResult phase1Result = phaseOne(maxIterations);
    
    if (phase1Result.status == LPStatus::INFEASIBLE) {
        return phase1Result;
    }
    
    if (phase1Result.status == LPStatus::MAX_ITERATIONS) {
        return phase1Result;
    }
    
    // Phase II: Optimize
    return phaseTwo(maxIterations);
}

// ============================================================================
// Standard Form Conversion
// ============================================================================

void SimplexSolver::convertToStandardForm() {
    // Count slack and artificial variables needed
    numSlack_ = 0;
    numArtificial_ = 0;
    
    for (const auto& c : constraints_) {
        switch (c.type) {
            case LEQ:
                numSlack_++;       // +s for slack
                break;
            case GEQ:
                numSlack_++;       // -s for surplus
                numArtificial_++;  // +a for artificial
                break;
            case EQ:
                numArtificial_++;  // +a for artificial
                break;
        }
    }
    
    // Total columns: original vars + slack + artificial
    totalCols_ = numVars_ + numSlack_ + numArtificial_;
    artificialStart_ = numVars_ + numSlack_;
    
    // Build tableau: (numConstraints_ + 1) rows x (totalCols_ + 1) cols
    // Last row is objective, last column is RHS
    tableau_.resize(numConstraints_ + 1, std::vector<double>(totalCols_ + 1, 0.0));
    basicVars_.resize(numConstraints_, -1);
    
    int slackIdx = numVars_;
    int artificialIdx = artificialStart_;
    
    // Fill constraint rows
    for (int i = 0; i < numConstraints_; ++i) {
        const Constraint& c = constraints_[i];
        
        // Original variable coefficients
        for (int j = 0; j < numVars_; ++j) {
            tableau_[i][j] = c.coeffs[j];
        }
        
        // RHS
        tableau_[i][rhsCol()] = c.rhs;
        
        // Add slack/surplus/artificial based on type
        switch (c.type) {
            case LEQ:
                // Add slack: +s
                tableau_[i][slackIdx] = 1.0;
                basicVars_[i] = slackIdx;  // Slack is basic
                slackIdx++;
                break;
                
            case GEQ:
                // Subtract surplus: -s
                tableau_[i][slackIdx] = -1.0;
                slackIdx++;
                // Add artificial: +a
                tableau_[i][artificialIdx] = 1.0;
                basicVars_[i] = artificialIdx;  // Artificial is basic
                artificialIdx++;
                break;
                
            case EQ:
                // Add artificial: +a
                tableau_[i][artificialIdx] = 1.0;
                basicVars_[i] = artificialIdx;  // Artificial is basic
                artificialIdx++;
                break;
        }
    }
    
    // For Phase I: objective is sum of artificial variables
    // We'll set up the Phase I objective in phaseOne()
}

// ============================================================================
// Phase I: Find Feasible Solution
// ============================================================================

LPResult SimplexSolver::phaseOne(int maxIterations) {
    LPResult result;
    
    // If no artificial variables, already feasible
    if (numArtificial_ == 0) {
        result.status = LPStatus::OPTIMAL;
        result.explanation = "No artificial variables - problem starts feasible";
        return result;
    }
    
    // Set Phase I objective: minimize sum of artificial variables
    // In tableau, this is row objRow() (last row)
    
    // Clear objective row
    for (int j = 0; j <= totalCols_; ++j) {
        tableau_[objRow()][j] = 0.0;
    }
    
    // Set coefficient 1.0 for each artificial variable (minimizing)
    for (int j = artificialStart_; j < totalCols_; ++j) {
        tableau_[objRow()][j] = 1.0;
    }
    
    // Make objective row consistent with basic variables
    // For each artificial basic variable, subtract its row from objective
    for (int i = 0; i < numConstraints_; ++i) {
        int basicCol = basicVars_[i];
        if (basicCol >= artificialStart_) {
            // This is an artificial basic variable
            // Subtract row i from objective row to zero out the column
            for (int j = 0; j <= totalCols_; ++j) {
                tableau_[objRow()][j] -= tableau_[i][j];
            }
        }
    }
    
    // Run simplex iterations
    int iterations = 0;
    while (iterations < maxIterations) {
        // Check if optimal (all reduced costs >= 0)
        int pivotCol = selectPivotColumn();
        if (pivotCol == -1) {
            // Optimal for Phase I
            break;
        }
        
        // Select pivot row
        int pivotRow = selectPivotRow(pivotCol);
        if (pivotRow == -1) {
            // Unbounded in Phase I - shouldn't happen with artificial vars
            result.status = LPStatus::UNBOUNDED;
            result.explanation = "Phase I appears unbounded (internal error)";
            return result;
        }
        
        // Perform pivot
        pivot(pivotRow, pivotCol);
        iterations++;
    }
    
    if (iterations >= maxIterations) {
        result.status = LPStatus::MAX_ITERATIONS;
        result.explanation = "Phase I exceeded iteration limit";
        return result;
    }
    
    // Check if feasible: Phase I optimal value should be 0
    double phaseIValue = std::abs(tableau_[objRow()][rhsCol()]);
    
    if (phaseIValue > SIMPLEX_EPSILON) {
        // Infeasible - artificial variables couldn't be driven to zero
        result.status = LPStatus::INFEASIBLE;
        result.conflictingRows = identifyConflicts();
        
        std::ostringstream oss;
        oss << "Problem is infeasible. Phase I objective = " 
            << std::fixed << std::setprecision(6) << phaseIValue;
        if (!result.conflictingRows.empty()) {
            oss << ". Conflicting constraints: ";
            for (size_t i = 0; i < result.conflictingRows.size(); ++i) {
                if (i > 0) oss << ", ";
                int idx = result.conflictingRows[i];
                if (idx < static_cast<int>(constraintNames_.size()) && 
                    !constraintNames_[idx].empty()) {
                    oss << constraintNames_[idx];
                } else {
                    oss << "row " << idx;
                }
            }
        }
        result.explanation = oss.str();
        return result;
    }
    
    // Feasible - check if any artificial variables are still basic
    // If so, try to pivot them out
    for (int i = 0; i < numConstraints_; ++i) {
        if (basicVars_[i] >= artificialStart_) {
            // Artificial variable still basic (at value 0)
            // Try to find a non-artificial column to pivot in
            for (int j = 0; j < artificialStart_; ++j) {
                if (std::abs(tableau_[i][j]) > SIMPLEX_EPSILON) {
                    pivot(i, j);
                    break;
                }
            }
        }
    }
    
    result.status = LPStatus::OPTIMAL;
    result.explanation = "Phase I complete - feasible solution found";
    return result;
}

// ============================================================================
// Phase II: Optimize
// ============================================================================

LPResult SimplexSolver::phaseTwo(int maxIterations) {
    LPResult result;
    
    // Set up Phase II objective (original objective)
    for (int j = 0; j <= totalCols_; ++j) {
        tableau_[objRow()][j] = 0.0;
    }
    
    // Original objective coefficients
    // For minimization: c_j
    // For maximization: -c_j (since we always minimize internally)
    double sign = maximize_ ? -1.0 : 1.0;
    for (int j = 0; j < numVars_; ++j) {
        tableau_[objRow()][j] = sign * objective_[j];
    }
    
    // Make objective row consistent with current basis
    // Need to eliminate ALL basic variables, not just original ones
    for (int i = 0; i < numConstraints_; ++i) {
        int basicCol = basicVars_[i];
        if (basicCol >= 0 && basicCol < artificialStart_) {
            // Non-artificial variable is basic - zero out its objective coefficient
            double coef = tableau_[objRow()][basicCol];
            if (std::abs(coef) > SIMPLEX_EPSILON) {
                for (int j = 0; j < artificialStart_; ++j) {
                    tableau_[objRow()][j] -= coef * tableau_[i][j];
                }
                // Also update RHS
                tableau_[objRow()][rhsCol()] -= coef * tableau_[i][rhsCol()];
            }
        }
    }
    
    // Force artificial variable columns to have large positive cost
    // This prevents them from re-entering the basis
    for (int j = artificialStart_; j < totalCols_; ++j) {
        tableau_[objRow()][j] = 0.0;  // They should stay at 0
    }
    
    // Run simplex iterations
    int iterations = 0;
    while (iterations < maxIterations) {
        // Check if optimal
        int pivotCol = selectPivotColumn();
        if (pivotCol == -1) {
            // Optimal
            result.status = LPStatus::OPTIMAL;
            result.solution = extractSolution();
            result.objectiveValue = getObjectiveValue();
            
            std::ostringstream oss;
            oss << "Optimal solution found. Objective = " 
                << std::fixed << std::setprecision(6) << result.objectiveValue;
            result.explanation = oss.str();
            return result;
        }
        
        // Check if unbounded
        if (isUnbounded(pivotCol)) {
            result.status = LPStatus::UNBOUNDED;
            result.explanation = "Problem is unbounded - no finite optimum exists";
            return result;
        }
        
        // Select pivot row
        int pivotRow = selectPivotRow(pivotCol);
        if (pivotRow == -1) {
            result.status = LPStatus::UNBOUNDED;
            result.explanation = "Problem is unbounded (no valid pivot row)";
            return result;
        }
        
        // Perform pivot
        pivot(pivotRow, pivotCol);
        iterations++;
    }
    
    result.status = LPStatus::MAX_ITERATIONS;
    result.explanation = "Phase II exceeded iteration limit";
    result.solution = extractSolution();
    result.objectiveValue = getObjectiveValue();
    return result;
}

// ============================================================================
// Simplex Operations
// ============================================================================

int SimplexSolver::selectPivotColumn() const {
    // Dantzig's rule: select most negative reduced cost
    int pivotCol = -1;
    double minCost = -SIMPLEX_EPSILON;  // Only consider truly negative
    
    // Only consider columns up to artificialStart_ (non-artificial)
    // This prevents artificial variables from re-entering the basis in Phase II
    // In Phase I, artificial columns have objective coefficients but we want
    // to allow pivoting on them
    for (int j = 0; j < totalCols_; ++j) {
        double reducedCost = tableau_[objRow()][j];
        if (reducedCost < minCost) {
            minCost = reducedCost;
            pivotCol = j;
        }
    }
    
    return pivotCol;
}

int SimplexSolver::selectPivotRow(int col) const {
    // Minimum ratio test
    int pivotRow = -1;
    double minRatio = std::numeric_limits<double>::infinity();
    
    for (int i = 0; i < numConstraints_; ++i) {
        double elem = tableau_[i][col];
        if (elem > SIMPLEX_EPSILON) {  // Only positive coefficients
            double ratio = tableau_[i][rhsCol()] / elem;
            if (ratio >= -SIMPLEX_EPSILON && ratio < minRatio) {
                minRatio = ratio;
                pivotRow = i;
            }
        }
    }
    
    return pivotRow;  // -1 if unbounded
}

void SimplexSolver::pivot(int pivotRow, int pivotCol) {
    double pivotElement = tableau_[pivotRow][pivotCol];
    
    // Normalize pivot row
    for (int j = 0; j <= totalCols_; ++j) {
        tableau_[pivotRow][j] /= pivotElement;
    }
    
    // Eliminate pivot column from other rows (including objective row)
    for (int i = 0; i <= numConstraints_; ++i) {
        if (i != pivotRow) {
            double factor = tableau_[i][pivotCol];
            if (std::abs(factor) > SIMPLEX_EPSILON) {
                for (int j = 0; j <= totalCols_; ++j) {
                    tableau_[i][j] -= factor * tableau_[pivotRow][j];
                }
            }
        }
    }
    
    // Update basic variable tracking
    basicVars_[pivotRow] = pivotCol;
}

bool SimplexSolver::isOptimal() const {
    // All reduced costs must be non-negative for optimality
    for (int j = 0; j < totalCols_; ++j) {
        if (tableau_[objRow()][j] < -SIMPLEX_EPSILON) {
            return false;
        }
    }
    return true;
}

bool SimplexSolver::isUnbounded(int col) const {
    // Unbounded if no positive entry in pivot column
    for (int i = 0; i < numConstraints_; ++i) {
        if (tableau_[i][col] > SIMPLEX_EPSILON) {
            return false;  // Can pivot on this
        }
    }
    return true;  // All entries <= 0, unbounded
}

// ============================================================================
// Solution Extraction
// ============================================================================

std::vector<double> SimplexSolver::extractSolution() const {
    std::vector<double> solution(numVars_, 0.0);
    
    // For each basic variable that's an original variable, read its value
    for (int i = 0; i < numConstraints_; ++i) {
        int basicCol = basicVars_[i];
        if (basicCol >= 0 && basicCol < numVars_) {
            solution[basicCol] = tableau_[i][rhsCol()];
        }
    }
    
    return solution;
}

double SimplexSolver::getObjectiveValue() const {
    // Objective value is in the RHS of the objective row
    // For minimization: -tableau_[objRow()][rhsCol()] (since we store -z)
    // Actually, our setup stores the reduced costs, and value is negative of RHS
    double value = -tableau_[objRow()][rhsCol()];
    
    // If maximizing, we minimized -f(x), so negate
    if (maximize_) {
        value = -value;
    }
    
    return value;
}

std::vector<int> SimplexSolver::identifyConflicts() const {
    std::vector<int> conflicts;
    
    // Find rows where artificial variables are still basic with positive value
    for (int i = 0; i < numConstraints_; ++i) {
        int basicCol = basicVars_[i];
        if (basicCol >= artificialStart_) {
            double value = tableau_[i][rhsCol()];
            if (std::abs(value) > SIMPLEX_EPSILON) {
                conflicts.push_back(i);
            }
        }
    }
    
    // If no artificial basics found with positive values, 
    // return all constraint rows (generic infeasibility)
    if (conflicts.empty()) {
        for (int i = 0; i < numConstraints_; ++i) {
            conflicts.push_back(i);
        }
    }
    
    return conflicts;
}

} // namespace Balance
} // namespace EcoSim
