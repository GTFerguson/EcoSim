#!/bin/bash
# Compile the EcoSim Balance Diagnostic Tool
#
# This tool analyzes energy flow in the ecosystem and detects balance exploits.
# It generates recommendations for tuning existing game parameters only.
#
# Usage:
#   ./compile_balance.sh           # Compile only
#   ./compile_balance.sh --run     # Compile and run
#   ./compile_balance.sh --help    # Show options

set -e

# Change to project root
cd "$(dirname "$0")/../../.."

echo "Compiling EcoSim Balance Diagnostic Tool..."

# Compile command - using clang++ to match other test scripts
clang++ -g -std=c++17 -Iinclude \
    src/testing/balance/balance_diagnostic.cpp \
    src/testing/balance/TrophicAnalyzer.cpp \
    src/testing/balance/ArchetypeProfiler.cpp \
    src/testing/balance/ExploitDetector.cpp \
    src/testing/balance/ReportGenerator.cpp \
    src/testing/balance/BalanceConstraints.cpp \
    src/testing/balance/SimplexSolver.cpp \
    src/testing/balance/ConstraintBuilders.cpp \
    src/testing/balance/BalanceOptimizer.cpp \
    src/genetics/core/*.cpp \
    src/genetics/defaults/*.cpp \
    src/genetics/expression/*.cpp \
    src/genetics/organisms/Plant.cpp \
    src/genetics/organisms/PlantFactory.cpp \
    src/genetics/organisms/CreatureFactory.cpp \
    src/genetics/classification/ArchetypeIdentity.cpp \
    src/genetics/classification/CreatureTaxonomy.cpp \
    src/genetics/interactions/FeedingInteraction.cpp \
    src/genetics/interactions/SeedDispersal.cpp \
    src/genetics/interactions/CombatInteraction.cpp \
    src/logging/Logger.cpp \
    src/objects/gameObject.cpp \
    src/objects/creature/creature.cpp \
    src/objects/creature/navigator.cpp \
    src/world/tile.cpp \
    src/world/world.cpp \
    src/world/SimplexNoise.cpp \
    src/world/ScentLayer.cpp \
    src/world/Corpse.cpp \
    -o balance_diagnostic

echo "Compilation successful!"
echo ""
echo "Run with: ./balance_diagnostic"
echo ""
echo "Options:"
echo "  ./balance_diagnostic                    # Print report to stdout"
echo "  ./balance_diagnostic --output report.txt  # Save to file"
echo "  ./balance_diagnostic --csv ./output      # Export CSV files"
echo "  ./balance_diagnostic --help              # Show help"

# Optionally run immediately
if [ "$1" == "--run" ]; then
    echo ""
    echo "Running balance diagnostic..."
    echo "========================================"
    ./balance_diagnostic
fi
