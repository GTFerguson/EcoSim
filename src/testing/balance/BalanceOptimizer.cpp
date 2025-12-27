/**
 * @file BalanceOptimizer.cpp
 * @brief Implementation of the high-level balance optimizer
 */

#include "BalanceOptimizer.hpp"

#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include <map>

namespace EcoSim {
namespace Balance {

// ============================================================================
// Constructor
// ============================================================================

BalanceOptimizer::BalanceOptimizer() = default;

// ============================================================================
// Initialization
// ============================================================================

void BalanceOptimizer::initializeDefaults() {
    // Clear any existing state
    registry_.clear();
    solver_.clear();
    
    // Register NUTRITION_PER_SIZE from Corpse.hpp
    registry_.registerVariable({
        "NUTRITION_PER_SIZE",
        "Corpse::NUTRITION_PER_SIZE",
        50.0,  // Current value (reduced from 100 to fix cannibalism exploit)
        1.0, std::numeric_limits<double>::infinity(),  // Bounds
        "Calories per unit size from corpses",
        "include/world/Corpse.hpp"
    });
    
    // Register BREED_COST from creature constants
    // BREED_COST_MULTIPLIER * 2 (for both parents) = 3.0 * 2 = 6.0
    registry_.registerVariable({
        "BREED_COST",
        "Creature::BREED_COST",
        6.0,  // Current value (3.0 per parent × 2 parents)
        1.0, 100.0,  // Widened bounds to allow LP feasibility
        "Total energy cost for both parents to breed",
        "include/objects/creature/creature.hpp"
    });
    
    // Initialize builders
    initializeBuilders();
}

void BalanceOptimizer::initializeFromGame(const TrophicAnalyzer* trophic,
                                          const ArchetypeProfiler* profiler) {
    // Store references for potential future use
    trophic_ = trophic;
    profiler_ = profiler;
    
    // For now, just use defaults
    // Future: extract actual values from analyzers
    initializeDefaults();
    
    // TODO: Override variable values with live data from analyzers
    // if (trophic) {
    //     auto* nutritionVar = registry_.getVariable("NUTRITION_PER_SIZE");
    //     if (nutritionVar) {
    //         nutritionVar->currentValue = trophic->getCurrentNutrition();
    //     }
    // }
}

void BalanceOptimizer::initializeVariables() {
    // This is called by the specific initialize methods
    // Variables are registered there
}

void BalanceOptimizer::initializeBuilders() {
    // Create constraint builders
    breedingBuilder_ = std::make_unique<BreedingConstraintBuilder>();
    trophicBuilder_ = std::make_unique<TrophicConstraintBuilder>();
    survivalBuilder_ = std::make_unique<SurvivalConstraintBuilder>();
    
    // Configure builders with appropriate parameters
    //
    // CORPSE VALUE CALCULATION CHAIN (must match game code):
    // CorpseSize = gene_size × infant_mod × health_per_size / corpse_divisor
    //            = 0.2 × 0.4 × 10.0 / 50.0 = 0.016 (minimum)
    //
    // With NUTRITION_PER_SIZE = 50:
    // Min offspring corpse nutrition = 0.016 × 50 = 0.8 calories
    // vs breeding cost = 6.0 calories
    // => Maximum ROI = 0.8 / 6.0 = 13.3% (exploit eliminated!)
    //
    constexpr float MIN_GENE_SIZE = 0.2f;
    constexpr float INFANT_EXPRESSION = 0.4f;
    constexpr float HEALTH_PER_SIZE = 10.0f;
    constexpr float CORPSE_DIVISOR = 50.0f;
    float minOffspringCorpseSize = MIN_GENE_SIZE * INFANT_EXPRESSION * HEALTH_PER_SIZE / CORPSE_DIVISOR;
    
    breedingBuilder_->setROIBounds(0.40, 0.90);  // Max 90% ROI to prevent cannibalism exploit
    breedingBuilder_->setMinOffspringSize(minOffspringCorpseSize);  // 0.016 - calculated from game constants
    breedingBuilder_->setMaxSurvivableCost(5.9);
    
    trophicBuilder_->setEfficiencyBounds(0.05, 0.50);  // Min 5% efficiency (relaxed from 20%)
    trophicBuilder_->setBaseEfficiency(0.12);
    trophicBuilder_->setBaseNutrition(50.0);  // Updated to match new NUTRITION_PER_SIZE
    
    survivalBuilder_->setMinHuntValue(50.0);
    
    // Register builders with registry
    registry_.addBuilder(breedingBuilder_.get());
    registry_.addBuilder(trophicBuilder_.get());
    registry_.addBuilder(survivalBuilder_.get());
}

// ============================================================================
// Optimization
// ============================================================================

OptimizationResult BalanceOptimizer::optimize() {
    OptimizationResult result;
    result.feasible = false;
    
    // Build all constraints from registered builders
    registry_.buildAllConstraints();
    
    // Validate registry
    std::string error;
    if (!registry_.validate(error)) {
        result.report = "Registry validation failed: " + error;
        return result;
    }
    
    // Clear and setup solver
    solver_.clear();
    
    // Populate solver from registry
    registry_.populateSolver(solver_);
    
    // Add objective function (minimize deviation from midpoints)
    addObjectiveFunction();
    
    // Solve the LP
    LPResult lpResult = solver_.solve();
    
    // Convert result based on status
    if (lpResult.status == LPStatus::OPTIMAL) {
        result.feasible = true;
        extractSolution(result, lpResult);
    } else if (lpResult.status == LPStatus::INFEASIBLE) {
        result.feasible = false;
        analyzeConflicts(result);
    } else {
        result.feasible = false;
        result.report = "Unexpected LP status: " + lpStatusToString(lpResult.status);
        if (!lpResult.explanation.empty()) {
            result.report += "\n" + lpResult.explanation;
        }
        return result;
    }
    
    // Compute category status
    computeCategoryStatus(result);
    
    // Generate human-readable report
    generateReport(result);
    
    return result;
}

void BalanceOptimizer::addObjectiveFunction() {
    // For L1 norm minimization, we need auxiliary variables
    // For simplicity, we'll minimize sum of squared deviations using a 
    // linear approximation around the midpoint targets
    
    // The solver already has variables from the registry
    // We set the objective to minimize deviation from targets
    
    const auto& vars = registry_.getVariables();
    int numVars = static_cast<int>(vars.size());
    
    // Create objective coefficients
    // For now, use a simple linear objective that tries to minimize
    // deviation from current values (will be overridden by constraints)
    std::vector<double> objective(numVars, 0.0);
    
    // Weight each variable equally in the objective
    // Since we're minimizing, positive weight means we want smaller values
    // We want values near the midpoint, so we don't set aggressive weights
    for (int i = 0; i < numVars; ++i) {
        // Small positive coefficient - the constraints will drive the solution
        objective[i] = 0.001;
    }
    
    solver_.setObjective(objective, false);  // Minimize
}

void BalanceOptimizer::extractSolution(OptimizationResult& result, 
                                       const LPResult& lpResult) {
    // Copy variables from registry and fill in optimal values
    const auto& vars = registry_.getVariables();
    result.parameters.reserve(vars.size());
    
    for (size_t i = 0; i < vars.size(); ++i) {
        BalanceVariable var = vars[i];
        if (i < lpResult.solution.size()) {
            var.optimalValue = lpResult.solution[i];
        } else {
            var.optimalValue = var.currentValue;  // Fallback
        }
        result.parameters.push_back(var);
    }
}

// ============================================================================
// Conflict Analysis
// ============================================================================

void BalanceOptimizer::analyzeConflicts(OptimizationResult& result) {
    const auto& constraints = registry_.getConstraints();
    
    // Look for constraints with conflictHints
    for (const auto& c1 : constraints) {
        for (const std::string& hint : c1.conflictHints) {
            // Find the constraint referenced by the hint
            for (const auto& c2 : constraints) {
                if (c2.id == hint) {
                    // Found a declared conflict pair
                    double gap = calculateConstraintGap(c1, c2);
                    
                    OptimizationResult::Conflict conflict;
                    conflict.constraint1 = c1.id;
                    conflict.constraint2 = c2.id;
                    conflict.gap = gap;
                    conflict.explanation = "Constraint '" + c1.displayName + 
                        "' conflicts with '" + c2.displayName + "'";
                    
                    // Add more detail based on constraint formulas
                    if (!c1.humanFormula.empty() && !c2.humanFormula.empty()) {
                        conflict.explanation += "\n  • " + c1.displayName + ": " + c1.humanFormula;
                        conflict.explanation += "\n  • " + c2.displayName + ": " + c2.humanFormula;
                    }
                    
                    result.conflicts.push_back(conflict);
                }
            }
        }
    }
    
    // If no explicit conflicts found, try to infer from LP result
    if (result.conflicts.empty()) {
        // Check for constraints on the same variable with non-overlapping bounds
        std::map<std::string, std::pair<double, double>> varBounds;  // var -> (lower, upper)
        
        for (const auto& c : constraints) {
            // Simple case: single-variable constraints
            if (c.coefficients.size() == 1) {
                const auto& [varId, coef] = c.coefficients[0];
                double bound = c.rhs / coef;
                
                if (varBounds.find(varId) == varBounds.end()) {
                    varBounds[varId] = {
                        -std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::infinity()
                    };
                }
                
                auto& [lower, upper] = varBounds[varId];
                
                if (c.relation == BalanceConstraint::Relation::LEQ && coef > 0) {
                    upper = std::min(upper, bound);
                } else if (c.relation == BalanceConstraint::Relation::GEQ && coef > 0) {
                    lower = std::max(lower, bound);
                }
            }
        }
        
        // Check for infeasible variable bounds
        for (const auto& [varId, bounds] : varBounds) {
            if (bounds.first > bounds.second) {
                OptimizationResult::Conflict conflict;
                conflict.constraint1 = varId + "_lower";
                conflict.constraint2 = varId + "_upper";
                conflict.gap = bounds.first - bounds.second;
                conflict.explanation = "Variable '" + varId + "' has infeasible bounds: " +
                    "lower=" + std::to_string(bounds.first) + 
                    ", upper=" + std::to_string(bounds.second);
                result.conflicts.push_back(conflict);
            }
        }
    }
}

double BalanceOptimizer::calculateConstraintGap(const BalanceConstraint& c1,
                                                 const BalanceConstraint& c2) const {
    // Simple heuristic: if both constraints affect the same primary variable,
    // calculate the gap between the implied bounds
    
    // Find the primary variable (first coefficient)
    if (c1.coefficients.empty() || c2.coefficients.empty()) {
        return 0.0;
    }
    
    const std::string& var1 = c1.coefficients[0].first;
    const std::string& var2 = c2.coefficients[0].first;
    
    if (var1 != var2) {
        // Different primary variables - can't easily compute gap
        return 0.0;
    }
    
    // Single-variable constraints: compute implied bound
    if (c1.coefficients.size() == 1 && c2.coefficients.size() == 1) {
        double bound1 = c1.rhs / c1.coefficients[0].second;
        double bound2 = c2.rhs / c2.coefficients[0].second;
        
        // LEQ gives upper bound, GEQ gives lower bound
        double upper = std::numeric_limits<double>::infinity();
        double lower = -std::numeric_limits<double>::infinity();
        
        if (c1.relation == BalanceConstraint::Relation::LEQ && c1.coefficients[0].second > 0) {
            upper = std::min(upper, bound1);
        } else if (c1.relation == BalanceConstraint::Relation::GEQ && c1.coefficients[0].second > 0) {
            lower = std::max(lower, bound1);
        }
        
        if (c2.relation == BalanceConstraint::Relation::LEQ && c2.coefficients[0].second > 0) {
            upper = std::min(upper, bound2);
        } else if (c2.relation == BalanceConstraint::Relation::GEQ && c2.coefficients[0].second > 0) {
            lower = std::max(lower, bound2);
        }
        
        if (lower > upper) {
            return lower - upper;  // Positive gap means infeasible
        }
    }
    
    return 0.0;
}

void BalanceOptimizer::computeCategoryStatus(OptimizationResult& result) {
    const auto& constraints = registry_.getConstraints();
    
    // Gather unique categories
    std::set<std::string> categories;
    for (const auto& c : constraints) {
        categories.insert(c.category);
    }
    
    // Compute status for each category
    for (const std::string& cat : categories) {
        OptimizationResult::CategoryStatus status;
        status.name = cat;
        status.total = 0;
        status.satisfied = 0;
        
        // Count constraints in this category
        auto catConstraints = registry_.getConstraintsByCategory(cat);
        status.total = static_cast<int>(catConstraints.size());
        
        if (result.feasible) {
            // If feasible, all constraints are satisfied
            status.satisfied = status.total;
            status.status = "✓ All satisfied";
        } else {
            // Check if this category is involved in conflicts
            bool hasConflict = false;
            for (const auto& conflict : result.conflicts) {
                for (const auto& c : catConstraints) {
                    if (c.id == conflict.constraint1 || c.id == conflict.constraint2) {
                        hasConflict = true;
                        break;
                    }
                }
                if (hasConflict) break;
            }
            
            if (hasConflict) {
                status.satisfied = 0;  // Assume none satisfied if there's a conflict
                status.status = "❌ Conflict";
            } else {
                status.satisfied = status.total;
                status.status = "✓ " + std::to_string(status.satisfied) + "/" + 
                               std::to_string(status.total) + " satisfied";
            }
        }
        
        result.categories.push_back(status);
    }
}

// ============================================================================
// Report Generation
// ============================================================================

void BalanceOptimizer::generateReport(OptimizationResult& result) {
    std::ostringstream ss;
    
    ss << "================================================================================\n";
    ss << "BALANCE OPTIMIZATION ANALYSIS\n";
    ss << "================================================================================\n\n";
    
    // Status section
    if (result.feasible) {
        ss << "STATUS: ✓ FEASIBLE - Optimal parameters found\n\n";
    } else {
        ss << "STATUS: ❌ INFEASIBLE - No valid parameter combination exists\n\n";
    }
    
    // Variables table
    ss << "REGISTERED VARIABLES (" << registry_.getVariableCount() << "):\n";
    ss << "  Variable               Current    Bounds         Source\n";
    ss << "  " << std::string(70, '-') << "\n";
    
    for (const auto& var : registry_.getVariables()) {
        std::ostringstream boundsStr;
        boundsStr << "[" << std::fixed << std::setprecision(1) << var.minBound << ", ";
        if (std::isinf(var.maxBound)) {
            boundsStr << "∞)";
        } else {
            boundsStr << var.maxBound << "]";
        }
        
        ss << "  " << std::left << std::setw(22) << var.id
           << std::right << std::setw(10) << std::fixed << std::setprecision(1) << var.currentValue
           << "  " << std::left << std::setw(14) << boundsStr.str()
           << " " << var.sourceFile << "\n";
    }
    ss << "\n";
    
    // Constraints table
    ss << "REGISTERED CONSTRAINTS (" << registry_.getConstraintCount() << "):\n";
    ss << "  Category    Constraint           Formula\n";
    ss << "  " << std::string(70, '-') << "\n";
    
    for (const auto& c : registry_.getConstraints()) {
        ss << "  " << std::left << std::setw(12) << c.category
           << std::setw(22) << c.displayName
           << c.humanFormula << "\n";
    }
    ss << "\n";
    
    if (result.feasible) {
        // Optimal solution section
        ss << "OPTIMAL SOLUTION:\n";
        ss << "  Parameter              Current    Optimal    Change\n";
        ss << "  " << std::string(60, '-') << "\n";
        
        for (const auto& var : result.parameters) {
            double change = var.optimalValue - var.currentValue;
            double pct = 0.0;
            if (std::abs(var.currentValue) > 1e-9) {
                pct = (change / var.currentValue) * 100.0;
            }
            
            ss << "  " << std::left << std::setw(22) << var.id
               << std::right << std::setw(10) << std::fixed << std::setprecision(1) << var.currentValue
               << std::setw(10) << var.optimalValue;
            
            // Show change with sign
            if (change >= 0) {
                ss << "     +" << std::fixed << std::setprecision(1) << change;
            } else {
                ss << "     " << std::fixed << std::setprecision(1) << change;
            }
            ss << " (" << std::showpos << std::fixed << std::setprecision(1) << pct << "%)" 
               << std::noshowpos << "\n";
        }
        ss << "\n";
    } else {
        // Conflict analysis section
        if (!result.conflicts.empty()) {
            ss << "CONSTRAINT CONFLICT ANALYSIS:\n\n";
            
            int conflictNum = 1;
            for (const auto& conflict : result.conflicts) {
                ss << "  CONFLICT #" << conflictNum++ << ": " 
                   << conflict.constraint1 << " vs " << conflict.constraint2 << "\n";
                ss << "  " << std::string(60, '-') << "\n";
                ss << "  " << conflict.explanation << "\n";
                ss << "  Gap: " << std::fixed << std::setprecision(2) << conflict.gap << " units\n\n";
            }
        }
    }
    
    // Category status section
    ss << "CONSTRAINT SATISFACTION:\n";
    ss << "  Category    Status              Details\n";
    ss << "  " << std::string(60, '-') << "\n";
    
    for (const auto& cat : result.categories) {
        ss << "  " << std::left << std::setw(12) << cat.name
           << std::setw(20) << cat.status;
        
        if (cat.total > 0) {
            ss << std::to_string(cat.satisfied) + "/" + std::to_string(cat.total) + " constraints";
        } else {
            ss << "No constraints registered";
        }
        ss << "\n";
    }
    ss << "\n";
    
    // Optimal values section (show regardless of feasibility)
    ss << "OPTIMAL VALUES:\n";
    ss << "  Parameter                  Current     Optimal     Change\n";
    ss << "  " << std::string(60, '-') << "\n";
    
    for (const auto& var : result.parameters) {
        double change = var.optimalValue - var.currentValue;
        
        ss << "  " << std::left << std::setw(26) << var.id
           << std::right << std::setw(10) << std::fixed << std::setprecision(2) << var.currentValue
           << std::setw(12) << std::fixed << std::setprecision(2) << var.optimalValue;
        
        if (std::abs(change) < 0.01) {
            ss << "    (no change)";
        } else if (change >= 0) {
            ss << "    +" << std::fixed << std::setprecision(2) << change;
        } else {
            ss << "    " << std::fixed << std::setprecision(2) << change;
        }
        ss << "\n";
    }
    ss << "\n";
    
    // Recommendation section
    ss << "RECOMMENDATION:\n";
    if (result.feasible) {
        ss << "  Apply optimal values to fix balance issues.\n";
        ss << "  See BalanceVariable::sourceFile for where to make changes.\n";
    } else {
        ss << "  Current game parameters CANNOT be balanced with existing mechanics.\n";
        ss << "  \n";
        ss << "  Consider:\n";
        ss << "  1. Age-based corpse value (newborns produce less nutrition)\n";
        ss << "  2. Cannibalism penalty (eating offspring reduces fitness)\n";
        ss << "  3. Size-scaled breeding cost (larger offspring cost more)\n";
    }
    ss << "\n";
    ss << "================================================================================\n";
    
    result.report = ss.str();
}

} // namespace Balance
} // namespace EcoSim
