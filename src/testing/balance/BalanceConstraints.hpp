/**
 * @file BalanceConstraints.hpp
 * @brief Core data structures for LP-based balance optimization
 * 
 * This file provides the foundational types for the Balance Constraint Solver:
 * - BalanceVariable: Decision variables with metadata
 * - BalanceConstraint: Self-documenting linear constraints
 * - ConstraintBuilder: Abstract factory for constraint categories
 * - BalanceConstraintRegistry: Central registration and management
 * 
 * ## Adding New Variables
 * 
 * To add a new optimization variable, call registerVariable():
 * @code
 *   registry.registerVariable({
 *       "NEW_PARAM",                    // id - unique identifier
 *       "Namespace::NEW_PARAM",         // displayName - full path
 *       currentValue,                   // currentValue - from game constants
 *       minBound, maxBound,             // feasible range
 *       "What this parameter controls", // description
 *       "include/path/to/file.hpp"      // sourceFile - where to change it
 *   });
 * @endcode
 * 
 * ## Adding New Constraints
 * 
 * To add a single constraint, call registerConstraint():
 * @code
 *   registry.registerConstraint({
 *       "constraint_id",                              // id
 *       "Human Readable Name",                        // displayName
 *       "category",                                   // category
 *       {{"VAR1", coef1}, {"VAR2", coef2}},          // coefficients
 *       BalanceConstraint::Relation::LEQ,            // relation
 *       rhs_value,                                    // rhs
 *       "0.5*VAR1 + 0.3*VAR2 ≤ 100",                 // humanFormula
 *       "Why this constraint exists",                 // description
 *       {"other_constraint_id"}                       // conflictHints
 *   });
 * @endcode
 * 
 * ## Adding New Constraint Categories
 * 
 * To add a new category of related constraints:
 * 1. Create a class inheriting from ConstraintBuilder
 * 2. Implement category(), description(), and build()
 * 3. Register it: registry.addBuilder(&myBuilder)
 * 
 * @see plans/balance-constraint-solver.md for full architecture details
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <limits>
#include <cmath>
#include <utility>

namespace EcoSim {
namespace Balance {

// Forward declarations
class SimplexSolver;  // Implemented in Phase 2
class BalanceConstraintRegistry;

/**
 * @brief A decision variable that can be optimized
 * 
 * Represents a game parameter that the LP solver can adjust to find
 * a balanced configuration. Includes metadata for reporting and
 * identifying where changes should be made.
 * 
 * Example:
 * @code
 *   BalanceVariable var{
 *       "NUTRITION_PER_SIZE",
 *       "Corpse::NUTRITION_PER_SIZE",
 *       100.0,
 *       1.0, std::numeric_limits<double>::infinity(),
 *       "Calories gained per unit corpse size",
 *       "include/world/Corpse.hpp"
 *   };
 * @endcode
 */
struct BalanceVariable {
    std::string id;           ///< Unique identifier: "NUTRITION_PER_SIZE"
    std::string displayName;  ///< Full path: "Corpse::NUTRITION_PER_SIZE"
    double currentValue;      ///< Current game value: 100.0
    double minBound;          ///< Minimum feasible value: 1.0
    double maxBound;          ///< Maximum feasible value: INFINITY
    std::string description;  ///< Human explanation of what this controls
    std::string sourceFile;   ///< Where to change it: "include/world/Corpse.hpp"
    
    // Computed by optimizer
    double targetValue = 0.0;   ///< Mid-range target for objective function
    double optimalValue = 0.0;  ///< Solution value after optimization
    
    /**
     * @brief Calculate midpoint target value
     * 
     * For bounded variables, returns (min + max) / 2.
     * For unbounded variables (inf upper bound), uses heuristic of 2x current.
     * 
     * @return Target value for objective function
     */
    double midpoint() const {
        if (std::isinf(maxBound)) {
            return currentValue * 2.0;  // Heuristic for unbounded variables
        }
        return (minBound + maxBound) / 2.0;
    }
};

/**
 * @brief A linear constraint with full metadata for reports
 * 
 * Represents: sum(coefficients[i].second * variable[coefficients[i].first]) <relation> rhs
 * 
 * The constraint is self-documenting with human-readable formula,
 * description, and hints about what it might conflict with.
 * 
 * Example:
 * @code
 *   BalanceConstraint constraint{
 *       "breeding_roi_max",
 *       "Breeding ROI ≤ 80%",
 *       "breeding",
 *       {{"NUTRITION_PER_SIZE", 0.5}, {"BREED_COST", -3.2}},
 *       BalanceConstraint::Relation::LEQ,
 *       0.0,
 *       "0.5n - 3.2b ≤ 0",
 *       "Prevents baby cannibalism exploit",
 *       {"trophic_efficiency_min"}
 *   };
 * @endcode
 */
struct BalanceConstraint {
    std::string id;           ///< Unique identifier: "breeding_roi_max"
    std::string displayName;  ///< Report name: "Breeding ROI ≤ 80%"
    std::string category;     ///< Group: "breeding", "trophic", "survival"
    
    /// Linear coefficients: vector of {variableId, coefficient} pairs
    std::vector<std::pair<std::string, double>> coefficients;
    
    /// Constraint relation type
    enum class Relation {
        LEQ,  ///< Less than or equal (≤)
        GEQ,  ///< Greater than or equal (≥)
        EQ    ///< Equal (=)
    };
    Relation relation;
    
    double rhs;  ///< Right-hand side value
    
    // For reports
    std::string humanFormula;    ///< Human-readable: "0.5n/(2b) ≤ 0.80"
    std::string description;     ///< Why: "Prevents baby cannibalism exploit"
    
    /// IDs of constraints this might conflict with (for infeasibility analysis)
    std::vector<std::string> conflictHints;
    
    /**
     * @brief Get string representation of relation
     * @return "≤", "≥", or "="
     */
    std::string relationStr() const {
        switch (relation) {
            case Relation::LEQ: return "≤";
            case Relation::GEQ: return "≥";
            case Relation::EQ:  return "=";
        }
        return "?";
    }
};

/**
 * @brief Abstract factory for creating related constraints
 * 
 * Implement this interface to create a new category of constraints.
 * Each builder produces a set of related constraints based on the
 * current registry state (registered variables).
 * 
 * Example:
 * @code
 * class MyConstraintBuilder : public ConstraintBuilder {
 * public:
 *     std::string category() const override { return "my_category"; }
 *     std::string description() const override { 
 *         return "Description of what these constraints enforce";
 *     }
 *     
 *     std::vector<BalanceConstraint> build(
 *         const BalanceConstraintRegistry& registry) const override
 *     {
 *         std::vector<BalanceConstraint> constraints;
 *         
 *         // Only add constraint if required variables exist
 *         if (registry.getVariable("MY_VAR")) {
 *             constraints.push_back({...});
 *         }
 *         
 *         return constraints;
 *     }
 * };
 * @endcode
 */
class ConstraintBuilder {
public:
    virtual ~ConstraintBuilder() = default;
    
    /**
     * @brief Get unique category identifier
     * @return Category name used for grouping constraints
     */
    virtual std::string category() const = 0;
    
    /**
     * @brief Get human-readable description
     * @return Description of what constraints this builder creates
     */
    virtual std::string description() const = 0;
    
    /**
     * @brief Build constraints given current registry state
     * 
     * The builder should check that required variables exist before
     * creating constraints that reference them.
     * 
     * @param registry Registry containing registered variables
     * @return Vector of constraints to add
     */
    virtual std::vector<BalanceConstraint> build(
        const BalanceConstraintRegistry& registry) const = 0;
};

/**
 * @brief Central registry for variables and constraints
 * 
 * The registry is the main entry point for the constraint system:
 * - Register variables that can be optimized
 * - Register constraints manually or via builders
 * - Validate that all constraint references are valid
 * - Convert to LP form for the SimplexSolver
 * - Generate human-readable reports
 * 
 * Usage:
 * @code
 *   BalanceConstraintRegistry registry;
 *   
 *   // Register variables
 *   registry.registerVariable({...});
 *   
 *   // Register constraints manually
 *   registry.registerConstraint({...});
 *   
 *   // Or use builders
 *   BreedingConstraintBuilder breeding;
 *   registry.addBuilder(&breeding);
 *   registry.buildAllConstraints();
 *   
 *   // Validate before solving
 *   std::string error;
 *   if (!registry.validate(error)) {
 *       std::cerr << "Validation failed: " << error << std::endl;
 *   }
 *   
 *   // Populate solver (Phase 2)
 *   registry.populateSolver(solver);
 * @endcode
 */
class BalanceConstraintRegistry {
public:
    // =====================
    // Variable Management
    // =====================
    
    /**
     * @brief Register a new optimization variable
     * 
     * Variables must be registered before constraints that reference them.
     * Duplicate IDs will overwrite existing variables.
     * 
     * @param var Variable to register
     */
    void registerVariable(const BalanceVariable& var);
    
    /**
     * @brief Look up a variable by ID
     * @param id Variable identifier
     * @return Pointer to variable, or nullptr if not found
     */
    const BalanceVariable* getVariable(const std::string& id) const;
    
    /**
     * @brief Get mutable access to all variables
     * @return Reference to variable vector
     */
    std::vector<BalanceVariable>& getVariables() { return variables_; }
    
    /**
     * @brief Get read-only access to all variables
     * @return Const reference to variable vector
     */
    const std::vector<BalanceVariable>& getVariables() const { return variables_; }
    
    // =====================
    // Constraint Management
    // =====================
    
    /**
     * @brief Register a single constraint
     * 
     * Constraints can reference variables that will be registered later,
     * but validate() will fail if they don't exist at validation time.
     * 
     * @param constraint Constraint to register
     */
    void registerConstraint(const BalanceConstraint& constraint);
    
    /**
     * @brief Get all registered constraints
     * @return Const reference to constraint vector
     */
    const std::vector<BalanceConstraint>& getConstraints() const { return constraints_; }
    
    /**
     * @brief Get constraints in a specific category
     * @param category Category to filter by
     * @return Vector of matching constraints
     */
    std::vector<BalanceConstraint> getConstraintsByCategory(const std::string& category) const;
    
    // =====================
    // Builder Pattern
    // =====================
    
    /**
     * @brief Add a constraint builder
     * 
     * Builders are not invoked immediately - call buildAllConstraints()
     * after all variables and builders are registered.
     * 
     * @param builder Pointer to builder (registry does not take ownership)
     */
    void addBuilder(ConstraintBuilder* builder);
    
    /**
     * @brief Invoke all registered builders
     * 
     * Calls build() on each builder and registers the returned constraints.
     * Should be called after all variables are registered.
     */
    void buildAllConstraints();
    
    // =====================
    // LP Conversion
    // =====================
    
    /**
     * @brief Populate a SimplexSolver with registry contents
     * 
     * Converts variables and constraints to LP standard form and
     * adds them to the solver. Call validate() first to ensure
     * the registry is in a valid state.
     * 
     * @param solver SimplexSolver to populate
     */
    void populateSolver(SimplexSolver& solver) const;
    
    /**
     * @brief Get variable index for LP tableau
     * @param id Variable identifier
     * @return 0-based index, or -1 if not found
     */
    int getVariableIndex(const std::string& id) const;
    
    // =====================
    // Validation
    // =====================
    
    /**
     * @brief Validate registry state
     * 
     * Checks that:
     * - All constraint coefficient variable references exist
     * - Variable bounds are sensible (min ≤ max)
     * - No duplicate constraint IDs
     * 
     * @param error Output: error message if validation fails
     * @return true if valid, false otherwise
     */
    bool validate(std::string& error) const;
    
    // =====================
    // Reporting
    // =====================
    
    /**
     * @brief Generate formatted variable table
     * @return ASCII table of all registered variables
     */
    std::string generateVariableTable() const;
    
    /**
     * @brief Generate formatted constraint table
     * @return ASCII table of all registered constraints
     */
    std::string generateConstraintTable() const;
    
    /**
     * @brief Get count of registered variables
     * @return Number of variables
     */
    size_t getVariableCount() const { return variables_.size(); }
    
    /**
     * @brief Get count of registered constraints
     * @return Number of constraints
     */
    size_t getConstraintCount() const { return constraints_.size(); }
    
    /**
     * @brief Clear all variables, constraints, and builders
     */
    void clear();
    
private:
    std::vector<BalanceVariable> variables_;
    std::vector<BalanceConstraint> constraints_;
    std::vector<ConstraintBuilder*> builders_;
    std::unordered_map<std::string, int> variableIndex_;  ///< Maps variable ID to index
};

} // namespace Balance
} // namespace EcoSim
