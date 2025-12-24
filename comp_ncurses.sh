#!/bin/bash
# EcoSim Compilation Script
# Works on: Linux, macOS, WSL

# Detect compiler (prefer clang++ on macOS, g++ elsewhere)
if [[ "$OSTYPE" == "darwin"* ]]; then
    CXX="${CXX:-clang++}"
else
    CXX="${CXX:-g++}"
fi

echo "Using compiler: $CXX"

$CXX -g -std=c++17 -Iinclude src/main.cpp src/window.cpp src/fileHandling.cpp \
src/calendar.cpp \
src/world/tile.cpp src/world/world.cpp src/world/SimplexNoise.cpp src/world/ScentLayer.cpp \
src/objects/food.cpp src/objects/gameObject.cpp src/objects/spawner.cpp \
src/statistics/statistics.cpp \
src/objects/creature/creature.cpp src/objects/creature/genome.cpp \
src/objects/creature/navigator.cpp \
src/rendering/RenderSystem.cpp \
src/rendering/backends/ncurses/NCursesColorMapper.cpp \
src/rendering/backends/ncurses/NCursesRenderer.cpp \
src/rendering/backends/ncurses/NCursesInputHandler.cpp \
src/genetics/core/Chromosome.cpp \
src/genetics/core/Gene.cpp \
src/genetics/core/GeneRegistry.cpp \
src/genetics/core/GeneticTypes.cpp \
src/genetics/core/Genome.cpp \
src/genetics/defaults/DefaultGenes.cpp \
src/genetics/defaults/PlantGenes.cpp \
src/genetics/defaults/UniversalGenes.cpp \
src/genetics/expression/EnvironmentState.cpp \
src/genetics/expression/EnergyBudget.cpp \
src/genetics/expression/OrganismState.cpp \
src/genetics/expression/Phenotype.cpp \
src/genetics/expression/PhenotypeCache.cpp \
src/genetics/migration/LegacyGenomeAdapter.cpp \
src/genetics/organisms/Plant.cpp \
src/genetics/organisms/PlantFactory.cpp \
src/genetics/organisms/CreatureFactory.cpp \
src/genetics/interactions/FeedingInteraction.cpp \
src/genetics/interactions/SeedDispersal.cpp \
src/genetics/interactions/CoevolutionTracker.cpp \
src/logging/Logger.cpp \
-lncurses -o EcoSim 2> err

# Check if there are actual errors (not just warnings) by looking at exit code
COMPILE_STATUS=$?

if [ $COMPILE_STATUS -ne 0 ]; then
    echo "❌ Compilation failed. Error output:"
    cat err
    echo ""
    exit 1
elif [ -s err ]; then
    # Only warnings - show but don't fail
    echo "⚠️  Compiled with warnings:"
    cat err
    echo ""
    rm -f err
    echo "✅ Compilation successful!"
    echo "Run with: ./EcoSim"
else
    rm -f err
    echo "✅ Compilation successful!"
    echo "Run with: ./EcoSim"
fi
