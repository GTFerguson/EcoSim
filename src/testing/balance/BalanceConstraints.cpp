/**
 * @file BalanceConstraints.cpp
 * @brief Implementation of BalanceConstraintRegistry
 * 
 * Provides the core functionality for registering and managing
 * optimization variables and constraints for the Balance LP Solver.
 */

#include "BalanceConstraints.hpp"
#include "SimplexSolver.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>

namespace EcoSim {
namespace Balance {

// =====================
// Variable Management
// =====================

void BalanceConstraintRegistry::registerVariable(const BalanceVariable& var) {
    // Check if variable already exists
    auto it = variableIndex_.find(var.id);
    if (it != variableIndex_.end()) {
        // Update existing variable
        variables_[it->second] = var;
    } else {
        // Add new variable
        variableIndex_[var.id] = static_cast<int>(variables_.size());
        variables_.push_back(var);
    }
}

const BalanceVariable* BalanceConstraintRegistry::getVariable(const std::string& id) const {
    auto it = variableIndex_.find(id);
    if (it == variableIndex_.end()) {
        return nullptr;
    }
    return &variables_[it->second];
}

// =====================
// Constraint Management
// =====================

void BalanceConstraintRegistry::registerConstraint(const BalanceConstraint& constraint) {
    constraints_.push_back(constraint);
}

std::vector<BalanceConstraint> BalanceConstraintRegistry::getConstraintsByCategory(
    const std::string& category) const {
    std::vector<BalanceConstraint> result;
    for (const auto& c : constraints_) {
        if (c.category == category) {
            result.push_back(c);
        }
    }
    return result;
}

// =====================
// Builder Pattern
// =====================

void BalanceConstraintRegistry::addBuilder(ConstraintBuilder* builder) {
    if (builder) {
        builders_.push_back(builder);
    }
}

void BalanceConstraintRegistry::buildAllConstraints() {
    for (auto* builder : builders_) {
        if (builder) {
            auto constraints = builder->build(*this);
            for (auto& c : constraints) {
                registerConstraint(c);
            }
        }
    }
}

// =====================
// LP Conversion
// =====================

void BalanceConstraintRegistry::populateSolver(SimplexSolver& solver) const {
    // Setup solver with variable count
    int numVars = static_cast<int>(variables_.size());
    solver.setNumVariables(numVars);
    
    // Set variable names and bounds
    for (int i = 0; i < numVars; ++i) {
        const auto& var = variables_[i];
        solver.setVariableName(i, var.id);
        solver.setVariableBounds(i, var.minBound, var.maxBound);
    }
    
    // Convert each constraint to dense form and add to solver
    for (const auto& constraint : constraints_) {
        // Build dense coefficient vector
        std::vector<double> coeffs(numVars, 0.0);
        
        for (const auto& coefPair : constraint.coefficients) {
            const std::string& varId = coefPair.first;
            double coef = coefPair.second;
            
            auto it = variableIndex_.find(varId);
            if (it != variableIndex_.end()) {
                coeffs[it->second] = coef;
            }
        }
        
        // Convert relation to SimplexSolver constraint type
        SimplexSolver::ConstraintType type;
        switch (constraint.relation) {
            case BalanceConstraint::Relation::LEQ:
                type = SimplexSolver::LEQ;
                break;
            case BalanceConstraint::Relation::GEQ:
                type = SimplexSolver::GEQ;
                break;
            case BalanceConstraint::Relation::EQ:
            default:
                type = SimplexSolver::EQ;
                break;
        }
        
        // Add constraint to solver
        solver.addConstraint(coeffs, type, constraint.rhs, constraint.id);
    }
}

int BalanceConstraintRegistry::getVariableIndex(const std::string& id) const {
    auto it = variableIndex_.find(id);
    if (it == variableIndex_.end()) {
        return -1;
    }
    return it->second;
}

// =====================
// Validation
// =====================

bool BalanceConstraintRegistry::validate(std::string& error) const {
    // Track constraint IDs to detect duplicates
    std::set<std::string> seenConstraintIds;
    
    // Check variable bounds
    for (const auto& var : variables_) {
        if (var.minBound > var.maxBound) {
            error = "Variable '" + var.id + "' has invalid bounds: min (" +
                    std::to_string(var.minBound) + ") > max (" +
                    std::to_string(var.maxBound) + ")";
            return false;
        }
    }
    
    // Check all constraints
    for (const auto& constraint : constraints_) {
        // Check for duplicate constraint IDs
        if (seenConstraintIds.count(constraint.id) > 0) {
            error = "Duplicate constraint ID: '" + constraint.id + "'";
            return false;
        }
        seenConstraintIds.insert(constraint.id);
        
        // Check that all coefficient variable references exist
        for (const auto& coef : constraint.coefficients) {
            const std::string& varId = coef.first;
            if (variableIndex_.find(varId) == variableIndex_.end()) {
                error = "Constraint '" + constraint.id + 
                        "' references undefined variable: '" + varId + "'";
                return false;
            }
        }
        
        // Check for empty constraint (no coefficients)
        if (constraint.coefficients.empty()) {
            error = "Constraint '" + constraint.id + "' has no coefficients";
            return false;
        }
    }
    
    return true;
}

// =====================
// Reporting
// =====================

std::string BalanceConstraintRegistry::generateVariableTable() const {
    std::ostringstream oss;
    
    // Header
    oss << "REGISTERED VARIABLES (" << variables_.size() << "):\n";
    oss << "  " << std::left 
        << std::setw(25) << "Variable"
        << std::setw(12) << "Current"
        << std::setw(18) << "Bounds"
        << "Source\n";
    oss << "  " << std::string(75, '-') << "\n";
    
    // Variables
    for (const auto& var : variables_) {
        // Format bounds
        std::ostringstream boundsStr;
        boundsStr << "[" << var.minBound << ", ";
        if (std::isinf(var.maxBound)) {
            boundsStr << "∞";
        } else {
            boundsStr << var.maxBound;
        }
        boundsStr << "]";
        
        oss << "  " << std::left 
            << std::setw(25) << var.displayName
            << std::setw(12) << var.currentValue
            << std::setw(18) << boundsStr.str()
            << var.sourceFile << "\n";
    }
    
    return oss.str();
}

std::string BalanceConstraintRegistry::generateConstraintTable() const {
    std::ostringstream oss;
    
    // Collect unique categories
    std::vector<std::string> categories;
    std::set<std::string> seenCategories;
    for (const auto& c : constraints_) {
        if (seenCategories.count(c.category) == 0) {
            categories.push_back(c.category);
            seenCategories.insert(c.category);
        }
    }
    
    // Header
    oss << "REGISTERED CONSTRAINTS (" << constraints_.size() << "):\n";
    oss << "  " << std::left 
        << std::setw(12) << "Category"
        << std::setw(28) << "Constraint"
        << "Formula\n";
    oss << "  " << std::string(75, '-') << "\n";
    
    // Group by category
    for (const auto& category : categories) {
        for (const auto& c : constraints_) {
            if (c.category == category) {
                oss << "  " << std::left 
                    << std::setw(12) << c.category
                    << std::setw(28) << c.displayName
                    << c.humanFormula << "\n";
            }
        }
    }
    
    return oss.str();
}

// =====================
// Utility
// =====================

void BalanceConstraintRegistry::clear() {
    variables_.clear();
    constraints_.clear();
    builders_.clear();
    variableIndex_.clear();
}

} // namespace Balance
} // namespace EcoSim
