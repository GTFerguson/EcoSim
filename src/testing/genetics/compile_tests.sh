#!/bin/bash
# Compile the modular genetics test suite

set -e

# Change to project root
cd "$(dirname "$0")/../../.."

echo "Compiling EcoSim Genetics Test Suite..."

# Note: We explicitly list files to exclude those with legacy Creature/Genome dependencies
# (LegacyGenomeAdapter.cpp, CoevolutionTracker.cpp)

# Compile command
clang++ -g -std=c++17 -Iinclude \
    src/testing/genetics/test_main.cpp \
    src/testing/genetics/test_core.cpp \
    src/testing/genetics/test_expression.cpp \
    src/testing/genetics/test_universal_genes.cpp \
    src/testing/genetics/test_energy_budget.cpp \
    src/testing/genetics/test_plants.cpp \
    src/testing/genetics/test_interactions.cpp \
    src/testing/genetics/test_behavior_plants.cpp \
    src/testing/genetics/test_behavior_feeding.cpp \
    src/testing/genetics/test_behavior_simulation.cpp \
    src/testing/genetics/test_creature_gene_api.cpp \
    src/genetics/core/*.cpp \
    src/genetics/defaults/*.cpp \
    src/genetics/expression/*.cpp \
    src/genetics/organisms/Plant.cpp \
    src/genetics/organisms/PlantFactory.cpp \
    src/genetics/interactions/FeedingInteraction.cpp \
    src/genetics/interactions/SeedDispersal.cpp \
    src/logging/Logger.cpp \
    src/objects/food.cpp \
    src/objects/gameObject.cpp \
    -o GeneticsTest

echo "Compilation successful!"
echo ""
echo "Run tests with: ./GeneticsTest"

# Optionally compile standalone behavior tests
if [ "$1" == "--behavior" ]; then
    echo ""
    echo "Compiling standalone Behavior Tests..."
    
    # Behavior Plants Test
    clang++ -g -std=c++17 -Iinclude \
        -DTEST_BEHAVIOR_PLANTS_STANDALONE \
        src/testing/genetics/test_behavior_plants.cpp \
        src/genetics/core/*.cpp \
        src/genetics/defaults/*.cpp \
        src/genetics/expression/*.cpp \
        src/genetics/organisms/Plant.cpp \
        src/genetics/organisms/PlantFactory.cpp \
        src/genetics/interactions/FeedingInteraction.cpp \
        src/genetics/interactions/SeedDispersal.cpp \
        src/logging/Logger.cpp \
        src/objects/food.cpp \
        src/objects/gameObject.cpp \
        -o BehaviorPlantsTest
    
    # Behavior Feeding Test
    clang++ -g -std=c++17 -Iinclude \
        -DTEST_BEHAVIOR_FEEDING_STANDALONE \
        src/testing/genetics/test_behavior_feeding.cpp \
        src/genetics/core/*.cpp \
        src/genetics/defaults/*.cpp \
        src/genetics/expression/*.cpp \
        src/genetics/organisms/Plant.cpp \
        src/genetics/organisms/PlantFactory.cpp \
        src/genetics/interactions/FeedingInteraction.cpp \
        src/genetics/interactions/SeedDispersal.cpp \
        src/logging/Logger.cpp \
        src/objects/food.cpp \
        src/objects/gameObject.cpp \
        -o BehaviorFeedingTest
    
    # Behavior Simulation Test
    clang++ -g -std=c++17 -Iinclude \
        -DTEST_BEHAVIOR_SIMULATION_STANDALONE \
        src/testing/genetics/test_behavior_simulation.cpp \
        src/genetics/core/*.cpp \
        src/genetics/defaults/*.cpp \
        src/genetics/expression/*.cpp \
        src/genetics/organisms/Plant.cpp \
        src/genetics/organisms/PlantFactory.cpp \
        src/genetics/interactions/FeedingInteraction.cpp \
        src/genetics/interactions/SeedDispersal.cpp \
        src/logging/Logger.cpp \
        src/objects/food.cpp \
        src/objects/gameObject.cpp \
        -o BehaviorSimulationTest
    
    echo ""
    echo "Standalone behavior tests compiled!"
    echo "Run with:"
    echo "  ./BehaviorPlantsTest"
    echo "  ./BehaviorFeedingTest"
    echo "  ./BehaviorSimulationTest"
fi
