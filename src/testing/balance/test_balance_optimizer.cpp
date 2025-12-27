/**
 * @file test_balance_optimizer.cpp
 * @brief Test the BalanceOptimizer component (Phase 4 of LP Solver)
 * 
 * This test verifies:
 * 1. BalanceOptimizer compiles and links correctly
 * 2. initializeDefaults() registers expected variables
 * 3. optimize() runs and returns a result
 * 4. Report generation produces readable output
 * 
 * Compile with:
 *   clang++ -std=c++17 -Iinclude \
 *     src/testing/balance/test_balance_optimizer.cpp \
 *     src/testing/balance/BalanceOptimizer.cpp \
 *     src/testing/balance/BalanceConstraints.cpp \
 *     src/testing/balance/SimplexSolver.cpp \
 *     src/testing/balance/ConstraintBuilders.cpp \
 *     -o test_optimizer
 */

#include "BalanceOptimizer.hpp"

#include <iostream>
#include <cassert>

using namespace EcoSim::Balance;

// Simple test framework
static int testsRun = 0;
static int testsPassed = 0;

#define ASSERT_TRUE(cond) do { \
    testsRun++; \
    if (!(cond)) { \
        std::cerr << "  FAILED: " #cond " at line " << __LINE__ << std::endl; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    testsRun++; \
    if ((a) != (b)) { \
        std::cerr << "  FAILED: " #a " != " #b " at line " << __LINE__ << std::endl; \
    } else { \
        testsPassed++; \
    } \
} while(0)

void test_default_initialization() {
    std::cout << "Test 1: Default initialization" << std::endl;
    
    BalanceOptimizer optimizer;
    optimizer.initializeDefaults();
    
    const auto& registry = optimizer.getRegistry();
    
    // Should have 2 variables registered
    ASSERT_EQ(registry.getVariableCount(), 2u);
    
    // Check NUTRITION_PER_SIZE exists
    const auto* nutrition = registry.getVariable("NUTRITION_PER_SIZE");
    ASSERT_TRUE(nutrition != nullptr);
    if (nutrition) {
        ASSERT_EQ(nutrition->currentValue, 100.0);
        ASSERT_EQ(nutrition->minBound, 1.0);
    }
    
    // Check BREED_COST exists
    const auto* breedCost = registry.getVariable("BREED_COST");
    ASSERT_TRUE(breedCost != nullptr);
    if (breedCost) {
        ASSERT_EQ(breedCost->currentValue, 6.0);
    }
    
    std::cout << "  Variables registered correctly" << std::endl;
}

void test_constraint_building() {
    std::cout << "\nTest 2: Constraint building" << std::endl;
    
    BalanceOptimizer optimizer;
    optimizer.initializeDefaults();
    
    // Manually trigger constraint building
    optimizer.getRegistry().buildAllConstraints();
    
    const auto& registry = optimizer.getRegistry();
    
    // Should have constraints from all builders
    ASSERT_TRUE(registry.getConstraintCount() > 0);
    
    // Check for breeding constraints
    auto breeding = registry.getConstraintsByCategory("breeding");
    ASSERT_TRUE(!breeding.empty());
    std::cout << "  Breeding constraints: " << breeding.size() << std::endl;
    
    // Check for trophic constraints
    auto trophic = registry.getConstraintsByCategory("trophic");
    ASSERT_TRUE(!trophic.empty());
    std::cout << "  Trophic constraints: " << trophic.size() << std::endl;
    
    // Check for survival constraints
    auto survival = registry.getConstraintsByCategory("survival");
    std::cout << "  Survival constraints: " << survival.size() << std::endl;
}

void test_optimization_runs() {
    std::cout << "\nTest 3: Optimization runs" << std::endl;
    
    BalanceOptimizer optimizer;
    optimizer.initializeDefaults();
    
    OptimizationResult result = optimizer.optimize();
    
    // Result should have a report
    ASSERT_TRUE(!result.report.empty());
    
    // Should have category status
    ASSERT_TRUE(!result.categories.empty());
    
    std::cout << "  Feasible: " << (result.feasible ? "YES" : "NO") << std::endl;
    std::cout << "  Categories analyzed: " << result.categories.size() << std::endl;
    std::cout << "  Conflicts found: " << result.conflicts.size() << std::endl;
}

void test_conflict_analysis() {
    std::cout << "\nTest 4: Conflict analysis" << std::endl;
    
    BalanceOptimizer optimizer;
    optimizer.initializeDefaults();
    
    OptimizationResult result = optimizer.optimize();
    
    // With default values, we expect infeasibility due to breeding vs trophic conflict
    if (!result.feasible) {
        std::cout << "  System is infeasible as expected" << std::endl;
        
        // Should have identified conflicts
        if (!result.conflicts.empty()) {
            std::cout << "  Conflicts identified:" << std::endl;
            for (const auto& conflict : result.conflicts) {
                std::cout << "    - " << conflict.constraint1 
                          << " vs " << conflict.constraint2 << std::endl;
                std::cout << "      Gap: " << conflict.gap << std::endl;
            }
        }
    } else {
        std::cout << "  System is feasible (unexpected with default values)" << std::endl;
        
        // If feasible, we should have optimal parameters
        ASSERT_TRUE(!result.parameters.empty());
    }
}

void test_report_generation() {
    std::cout << "\nTest 5: Report generation" << std::endl;
    
    BalanceOptimizer optimizer;
    optimizer.initializeDefaults();
    
    OptimizationResult result = optimizer.optimize();
    
    // Report should contain expected sections
    ASSERT_TRUE(result.report.find("BALANCE OPTIMIZATION") != std::string::npos);
    ASSERT_TRUE(result.report.find("STATUS:") != std::string::npos);
    ASSERT_TRUE(result.report.find("REGISTERED VARIABLES") != std::string::npos);
    ASSERT_TRUE(result.report.find("REGISTERED CONSTRAINTS") != std::string::npos);
    ASSERT_TRUE(result.report.find("RECOMMENDATION") != std::string::npos);
    
    std::cout << "  Report contains all expected sections" << std::endl;
}

void test_registry_validation() {
    std::cout << "\nTest 6: Registry validation" << std::endl;
    
    BalanceOptimizer optimizer;
    optimizer.initializeDefaults();
    optimizer.getRegistry().buildAllConstraints();
    
    std::string error;
    bool valid = optimizer.getRegistry().validate(error);
    
    ASSERT_TRUE(valid);
    if (!valid) {
        std::cout << "  Validation failed: " << error << std::endl;
    } else {
        std::cout << "  Registry validation passed" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "BalanceOptimizer Tests (Phase 4)" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    test_default_initialization();
    test_constraint_building();
    test_optimization_runs();
    test_conflict_analysis();
    test_report_generation();
    test_registry_validation();
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Print the full report for inspection
    std::cout << std::endl;
    std::cout << "Full Optimization Report:" << std::endl;
    std::cout << "========================================" << std::endl;
    
    BalanceOptimizer optimizer;
    optimizer.initializeDefaults();
    OptimizationResult result = optimizer.optimize();
    std::cout << result.report;
    
    return (testsPassed == testsRun) ? 0 : 1;
}
