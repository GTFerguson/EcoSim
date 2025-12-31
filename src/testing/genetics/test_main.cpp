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

// Creature behavior test runners (new coverage tests)
extern void runHealthHealingTests();
extern void run_creature_breeding_tests();
extern void run_creature_state_machine_tests();
extern void run_creature_movement_tests();

// HealthSystem test runner (organism-agnostic)
extern void runHealthSystemTests();

// FeedingBehavior test runner (IBehavior implementation)
extern void runFeedingBehaviorTests();

// MatingBehavior test runner (IBehavior implementation)
extern void run_mating_behavior_tests();

// MovementBehavior test runner (IBehavior implementation)
extern void run_movement_behavior_tests();

// ZoochoryBehavior test runner (animal-mediated seed dispersal)
extern void runZoochoryBehaviorTests();

// RestBehavior test runner (IBehavior implementation)
extern void runRestBehaviorTests();

// Modulation Policy test runner (TraitModulationPolicy system)
extern void runModulationPolicyTests();

// Serialization test runner (JSON save/load system)
extern void runSerializationTests();

// SpatialIndex test runner (world spatial queries)
extern void runSpatialIndexTests();

// WorldGrid test runner (grid storage component)
extern void runWorldGridTests();

// WorldGenerator test runner (terrain generation component)
extern void runWorldGeneratorTests();

// CorpseManager test runner (corpse lifecycle management)
extern void runCorpseManagerTests();

// SeasonManager test runner (time/season progression)
extern void runSeasonManagerTests();

// EnvironmentSystem test runner (per-tile environmental queries)
extern void runEnvironmentSystemTests();

// PlantManager test runner (plant lifecycle management)
extern void runPlantManagerTests();

// IReproducible interface test runner
extern void runReproducibleInterfaceTests();

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
    
    // Creature Behavior Coverage Tests
    std::cout << "=== Creature Health & Healing Tests ===" << std::endl;
    runHealthHealingTests();
    std::cout << std::endl;
    
    std::cout << "=== Creature Breeding System Tests ===" << std::endl;
    run_creature_breeding_tests();
    std::cout << std::endl;
    
    std::cout << "=== Creature State Machine Tests ===" << std::endl;
    run_creature_state_machine_tests();
    std::cout << std::endl;
    
    std::cout << "=== Creature Movement System Tests ===" << std::endl;
    run_creature_movement_tests();
    std::cout << std::endl;
    
    // HealthSystem Tests (organism-agnostic)
    std::cout << "=== HealthSystem Tests (Organism-Agnostic) ===" << std::endl;
    runHealthSystemTests();
    std::cout << std::endl;
    
    // FeedingBehavior Tests (IBehavior implementation)
    std::cout << "=== FeedingBehavior Tests (IBehavior) ===" << std::endl;
    runFeedingBehaviorTests();
    std::cout << std::endl;
    
    // MatingBehavior Tests (IBehavior implementation)
    std::cout << "=== MatingBehavior Tests (IBehavior) ===" << std::endl;
    run_mating_behavior_tests();
    std::cout << std::endl;
    
    // MovementBehavior Tests (IBehavior implementation)
    std::cout << "=== MovementBehavior Tests (IBehavior) ===" << std::endl;
    run_movement_behavior_tests();
    std::cout << std::endl;
    
    // ZoochoryBehavior Tests (animal-mediated seed dispersal)
    std::cout << "=== ZoochoryBehavior Tests (Zoochory) ===" << std::endl;
    runZoochoryBehaviorTests();
    std::cout << std::endl;
    
    // RestBehavior Tests (IBehavior implementation)
    std::cout << "=== RestBehavior Tests (IBehavior) ===" << std::endl;
    runRestBehaviorTests();
    std::cout << std::endl;
    
    // Modulation Policy Tests (TraitModulationPolicy system)
    std::cout << "=== TraitModulationPolicy Tests ===" << std::endl;
    runModulationPolicyTests();
    std::cout << std::endl;
    
    // Serialization Tests (JSON save/load system)
    std::cout << "=== Serialization System Tests ===" << std::endl;
    runSerializationTests();
    std::cout << std::endl;
    
    // SpatialIndex Tests (world spatial queries)
    std::cout << "=== SpatialIndex Tests (World) ===" << std::endl;
    runSpatialIndexTests();
    std::cout << std::endl;
    
    // WorldGrid Tests (grid storage component)
    std::cout << "=== WorldGrid Tests (World) ===" << std::endl;
    runWorldGridTests();
    std::cout << std::endl;
    
    // WorldGenerator Tests (terrain generation component)
    std::cout << "=== WorldGenerator Tests (World) ===" << std::endl;
    runWorldGeneratorTests();
    std::cout << std::endl;
    
    // CorpseManager Tests (corpse lifecycle management)
    std::cout << "=== CorpseManager Tests (World) ===" << std::endl;
    runCorpseManagerTests();
    std::cout << std::endl;
    
    // SeasonManager Tests (time/season progression)
    std::cout << "=== SeasonManager Tests (World) ===" << std::endl;
    runSeasonManagerTests();
    std::cout << std::endl;
    
    // EnvironmentSystem Tests (per-tile environmental queries)
    std::cout << "=== EnvironmentSystem Tests (World) ===" << std::endl;
    runEnvironmentSystemTests();
    std::cout << std::endl;
    
    // PlantManager Tests (plant lifecycle management)
    std::cout << "=== PlantManager Tests (World) ===" << std::endl;
    runPlantManagerTests();
    std::cout << std::endl;
    
    // IReproducible Interface Tests
    std::cout << "=== IReproducible Interface Tests ===" << std::endl;
    runReproducibleInterfaceTests();
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
