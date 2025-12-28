# src/testing/CMakeLists.txt
# Test configuration for EcoSim

include(CTest)

# Set test output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# ==============================================================================
# GeneticsTest - Main test suite for genetics system
# ==============================================================================
# Note: We explicitly list files to match compile_tests.sh and exclude
# standalone test files (test_classification, test_herbivore_feeding_diagnostic,
# test_plant_copy, test_olfactory_genes, test_perception_system, test_behavior_controller,
# test_hunting_behavior) that have their own main() or other dependencies
set(GENETICS_TEST_SOURCES
    genetics/test_main.cpp
    genetics/test_core.cpp
    genetics/test_expression.cpp
    genetics/test_universal_genes.cpp
    genetics/test_energy_budget.cpp
    genetics/test_plants.cpp
    genetics/test_interactions.cpp
    genetics/test_behavior_plants.cpp
    genetics/test_behavior_feeding.cpp
    genetics/test_behavior_simulation.cpp
    genetics/test_creature_gene_api.cpp
    genetics/test_naming_system.cpp
    genetics/test_berry_bush_diagnostic.cpp
    genetics/test_combat_system.cpp
    genetics/test_combat_behavior.cpp
    genetics/test_health_healing.cpp
    genetics/test_creature_breeding.cpp
    genetics/test_creature_state_machine.cpp
    genetics/test_creature_movement.cpp
    genetics/test_health_system.cpp
    genetics/test_feeding_behavior.cpp
    genetics/test_scent_layer.cpp
    genetics/test_scent_navigation.cpp
    genetics/test_mating_behavior.cpp
    genetics/test_movement_behavior.cpp
    genetics/test_zoochory_behavior.cpp
    genetics/test_rest_behavior.cpp
    genetics/test_modulation_policy.cpp
)

add_executable(GeneticsTest
    ${GENETICS_TEST_SOURCES}
)

target_include_directories(GeneticsTest PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(GeneticsTest PRIVATE
    ecosim_genetics
    ecosim_world
    ecosim_core
)

add_compiler_warnings(GeneticsTest)

# Register the test with CTest
add_test(
    NAME GeneticsTest
    COMMAND GeneticsTest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
