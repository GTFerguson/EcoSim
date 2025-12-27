/**
 * @file test_simplex.cpp
 * @brief Quick test for SimplexSolver implementation
 * 
 * Compile with:
 *   clang++ -std=c++17 -I../../../include src/testing/balance/test_simplex.cpp \
 *       src/testing/balance/SimplexSolver.cpp -o test_simplex
 * 
 * Or from project root:
 *   clang++ -std=c++17 -Iinclude src/testing/balance/test_simplex.cpp \
 *       src/testing/balance/SimplexSolver.cpp -o test_simplex
 */

#include "SimplexSolver.hpp"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace EcoSim::Balance;

bool approxEqual(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

// Test 1: Simple feasible problem
// max 3x + 2y subject to x + y <= 4, x <= 2, y <= 3
// Expected: OPTIMAL, x=2, y=2, objective=10
bool testFeasibleProblem() {
    std::cout << "Test 1: Simple feasible problem (maximize)" << std::endl;
    std::cout << "  max 3x + 2y" << std::endl;
    std::cout << "  s.t. x + y <= 4" << std::endl;
    std::cout << "       x <= 2" << std::endl;
    std::cout << "       y <= 3" << std::endl;
    
    SimplexSolver solver;
    solver.setNumVariables(2);
    solver.setVariableName(0, "x");
    solver.setVariableName(1, "y");
    solver.setObjective({3, 2}, true);  // maximize
    
    solver.addConstraint({1, 1}, SimplexSolver::LEQ, 4, "sum_constraint");
    solver.addConstraint({1, 0}, SimplexSolver::LEQ, 2, "x_bound");
    solver.addConstraint({0, 1}, SimplexSolver::LEQ, 3, "y_bound");
    
    auto result = solver.solve();
    
    std::cout << "  Status: " << lpStatusToString(result.status) << std::endl;
    std::cout << "  Explanation: " << result.explanation << std::endl;
    
    if (result.status == LPStatus::OPTIMAL) {
        std::cout << "  Solution: x=" << result.solution[0] 
                  << ", y=" << result.solution[1] << std::endl;
        std::cout << "  Objective: " << result.objectiveValue << std::endl;
        
        // Verify solution
        bool xOk = approxEqual(result.solution[0], 2.0);
        bool yOk = approxEqual(result.solution[1], 2.0);
        bool objOk = approxEqual(result.objectiveValue, 10.0);
        
        std::cout << "  Verification: x=" << (xOk ? "OK" : "FAIL")
                  << ", y=" << (yOk ? "OK" : "FAIL")
                  << ", obj=" << (objOk ? "OK" : "FAIL") << std::endl;
        
        return xOk && yOk && objOk;
    }
    
    return false;
}

// Test 2: Simple infeasible problem
// x <= 1, x >= 2
// Expected: INFEASIBLE
bool testInfeasibleProblem() {
    std::cout << "\nTest 2: Simple infeasible problem" << std::endl;
    std::cout << "  min x" << std::endl;
    std::cout << "  s.t. x <= 1" << std::endl;
    std::cout << "       x >= 2" << std::endl;
    
    SimplexSolver solver;
    solver.setNumVariables(1);
    solver.setVariableName(0, "x");
    solver.setObjective({1}, false);  // minimize
    
    solver.addConstraint({1}, SimplexSolver::LEQ, 1, "x_upper");
    solver.addConstraint({1}, SimplexSolver::GEQ, 2, "x_lower");
    
    auto result = solver.solve();
    
    std::cout << "  Status: " << lpStatusToString(result.status) << std::endl;
    std::cout << "  Explanation: " << result.explanation << std::endl;
    
    bool ok = (result.status == LPStatus::INFEASIBLE);
    std::cout << "  Verification: " << (ok ? "OK (correctly detected infeasible)" : "FAIL") << std::endl;
    
    return ok;
}

// Test 3: Simple minimization problem
// min x + y subject to x + y >= 2, x >= 0, y >= 0
// Expected: OPTIMAL, x=2 (or any combination summing to 2), objective=2
bool testMinimizationProblem() {
    std::cout << "\nTest 3: Simple minimization problem" << std::endl;
    std::cout << "  min x + y" << std::endl;
    std::cout << "  s.t. x + y >= 2" << std::endl;
    
    SimplexSolver solver;
    solver.setNumVariables(2);
    solver.setVariableName(0, "x");
    solver.setVariableName(1, "y");
    solver.setObjective({1, 1}, false);  // minimize
    
    solver.addConstraint({1, 1}, SimplexSolver::GEQ, 2, "sum_constraint");
    
    auto result = solver.solve();
    
    std::cout << "  Status: " << lpStatusToString(result.status) << std::endl;
    std::cout << "  Explanation: " << result.explanation << std::endl;
    
    if (result.status == LPStatus::OPTIMAL) {
        std::cout << "  Solution: x=" << result.solution[0] 
                  << ", y=" << result.solution[1] << std::endl;
        std::cout << "  Objective: " << result.objectiveValue << std::endl;
        
        // Verify objective value
        bool objOk = approxEqual(result.objectiveValue, 2.0);
        // Verify constraint is satisfied
        double sum = result.solution[0] + result.solution[1];
        bool feasible = sum >= 2.0 - 1e-6;
        
        std::cout << "  Verification: objective=" << (objOk ? "OK" : "FAIL")
                  << ", feasible=" << (feasible ? "OK" : "FAIL") << std::endl;
        
        return objOk && feasible;
    }
    
    return false;
}

// Test 4: Problem with equality constraint
// min 2x + y subject to x + y = 3, x >= 0, y >= 0
// Expected: OPTIMAL, x=0, y=3, objective=3
bool testEqualityConstraint() {
    std::cout << "\nTest 4: Problem with equality constraint" << std::endl;
    std::cout << "  min 2x + y" << std::endl;
    std::cout << "  s.t. x + y = 3" << std::endl;
    
    SimplexSolver solver;
    solver.setNumVariables(2);
    solver.setVariableName(0, "x");
    solver.setVariableName(1, "y");
    solver.setObjective({2, 1}, false);  // minimize
    
    solver.addConstraint({1, 1}, SimplexSolver::EQ, 3, "equality");
    
    auto result = solver.solve();
    
    std::cout << "  Status: " << lpStatusToString(result.status) << std::endl;
    std::cout << "  Explanation: " << result.explanation << std::endl;
    
    if (result.status == LPStatus::OPTIMAL) {
        std::cout << "  Solution: x=" << result.solution[0] 
                  << ", y=" << result.solution[1] << std::endl;
        std::cout << "  Objective: " << result.objectiveValue << std::endl;
        
        // Verify solution (minimize 2x + y with x + y = 3 gives x=0, y=3)
        bool xOk = approxEqual(result.solution[0], 0.0);
        bool yOk = approxEqual(result.solution[1], 3.0);
        bool objOk = approxEqual(result.objectiveValue, 3.0);
        
        std::cout << "  Verification: x=" << (xOk ? "OK" : "FAIL")
                  << ", y=" << (yOk ? "OK" : "FAIL")
                  << ", obj=" << (objOk ? "OK" : "FAIL") << std::endl;
        
        return xOk && yOk && objOk;
    }
    
    return false;
}

// Test 5: Empty problem
bool testEmptyProblem() {
    std::cout << "\nTest 5: Empty problem (no variables)" << std::endl;
    
    SimplexSolver solver;
    // Don't set any variables
    
    auto result = solver.solve();
    
    std::cout << "  Status: " << lpStatusToString(result.status) << std::endl;
    std::cout << "  Explanation: " << result.explanation << std::endl;
    
    bool ok = (result.status == LPStatus::OPTIMAL && 
               approxEqual(result.objectiveValue, 0.0));
    std::cout << "  Verification: " << (ok ? "OK" : "FAIL") << std::endl;
    
    return ok;
}

// Test 6: Single variable, single constraint
bool testSingleVariable() {
    std::cout << "\nTest 6: Single variable problem" << std::endl;
    std::cout << "  max x" << std::endl;
    std::cout << "  s.t. x <= 5" << std::endl;
    
    SimplexSolver solver;
    solver.setNumVariables(1);
    solver.setVariableName(0, "x");
    solver.setObjective({1}, true);  // maximize
    
    solver.addConstraint({1}, SimplexSolver::LEQ, 5, "x_bound");
    
    auto result = solver.solve();
    
    std::cout << "  Status: " << lpStatusToString(result.status) << std::endl;
    std::cout << "  Explanation: " << result.explanation << std::endl;
    
    if (result.status == LPStatus::OPTIMAL) {
        std::cout << "  Solution: x=" << result.solution[0] << std::endl;
        std::cout << "  Objective: " << result.objectiveValue << std::endl;
        
        bool xOk = approxEqual(result.solution[0], 5.0);
        bool objOk = approxEqual(result.objectiveValue, 5.0);
        
        std::cout << "  Verification: x=" << (xOk ? "OK" : "FAIL")
                  << ", obj=" << (objOk ? "OK" : "FAIL") << std::endl;
        
        return xOk && objOk;
    }
    
    return false;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "SimplexSolver Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    int passed = 0;
    int total = 6;
    
    if (testFeasibleProblem()) passed++;
    if (testInfeasibleProblem()) passed++;
    if (testMinimizationProblem()) passed++;
    if (testEqualityConstraint()) passed++;
    if (testEmptyProblem()) passed++;
    if (testSingleVariable()) passed++;
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Results: " << passed << "/" << total << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (passed == total) ? 0 : 1;
}
