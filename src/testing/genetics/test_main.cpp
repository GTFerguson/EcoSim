/**
 * @file test_main.cpp
 * @brief Main test runner for the genetics system
 *
 * Runs all test suites for Phase 1, 2.1, 2.2, 2.3, 2.4, and Behavior Integration.
 */

#include <iostream>
#include <chrono>
#include "test_framework.hpp"

// Forward declarations for test runners
extern void runCoreTests();
extern void runExpressionTests();
extern void runUniversalGenesTests();
extern void runEnergyBudgetTests();
extern void runPlantTests();
extern void runInteractionTests();

// Behavior integration test runners
extern void runBehaviorPlantTests();
extern void runBehaviorFeedingTests();
extern void runBehaviorSimulationTests();

// Diagnostic test runners
extern void runCreatureGeneAPITests();
extern void runBerryBushDiagnosticTests();

// Naming system test runners
extern void runNamingSystemTests();

// Combat system test runners
extern void run_combat_system_tests();
extern void run_combat_behavior_tests();

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::cout << "================================================================" << std::endl;
    std::cout << "              EcoSim Genetics System Test Suite                 " << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << std::endl;
    
    // Phase 1: Core Genetics
    std::cout << "=== Phase 1: Core Genetics ===" << std::endl;
    runCoreTests();
    std::cout << std::endl;
    
    // Phase 1 continued: Expression
    std::cout << "=== Phase 1: Expression System ===" << std::endl;
    runExpressionTests();
    std::cout << std::endl;
    
    // Phase 2.1: Universal Genes & Diet
    std::cout << "=== Phase 2.1: Universal Genes & Emergent Diet ===" << std::endl;
    runUniversalGenesTests();
    std::cout << std::endl;
    
    // Phase 2.2: Energy Budget
    std::cout << "=== Phase 2.2: Energy Budget System ===" << std::endl;
    runEnergyBudgetTests();
    std::cout << std::endl;
    
    // Phase 2.3: Plant System
    std::cout << "=== Phase 2.3: Plant System ===" << std::endl;
    runPlantTests();
    std::cout << std::endl;
    
    // Phase 2.4: Creature-Plant Interactions
    std::cout << "=== Phase 2.4: Creature-Plant Interactions ===" << std::endl;
    runInteractionTests();
    std::cout << std::endl;
    
    // Behavior Integration Tests: Plant Behavior
    std::cout << "=== Behavior Integration: Plant Behavior ===" << std::endl;
    runBehaviorPlantTests();
    std::cout << std::endl;
    
    // Behavior Integration Tests: Feeding Behavior
    std::cout << "=== Behavior Integration: Creature-Plant Feeding ===" << std::endl;
    runBehaviorFeedingTests();
    std::cout << std::endl;
    
    // Behavior Integration Tests: World Simulation
    std::cout << "=== Behavior Integration: World Simulation ===" << std::endl;
    runBehaviorSimulationTests();
    std::cout << std::endl;
    
    // Diagnostic Tests: Creature Gene API
    std::cout << "=== Diagnostic: Creature Gene API ===" << std::endl;
    runCreatureGeneAPITests();
    std::cout << std::endl;
    
    // Naming System Tests: Dynamic Classification & Scientific Names
    std::cout << "=== Naming System: Dynamic Classification ===" << std::endl;
    runNamingSystemTests();
    std::cout << std::endl;
    
    // Berry Bush Diagnostic Tests
    std::cout << "=== Diagnostic: Berry Bush Lifecycle ===" << std::endl;
    runBerryBushDiagnosticTests();
    std::cout << std::endl;
    
    // Combat System Tests
    std::cout << "=== Combat System Tests ===" << std::endl;
    run_combat_system_tests();
    std::cout << std::endl;
    
    // Combat Behavior Tests
    std::cout << "=== Combat Behavior Tests ===" << std::endl;
    run_combat_behavior_tests();
    std::cout << std::endl;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "================================================================" << std::endl;
    std::cout << "                        TEST SUMMARY                            " << std::endl;
    std::cout << "================================================================" << std::endl;
    
    EcoSim::Testing::TestSuite::instance().printSummary();
    
    std::cout << std::endl;
    std::cout << "Total execution time: " << duration.count() << " ms" << std::endl;
    std::cout << "================================================================" << std::endl;
    
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
