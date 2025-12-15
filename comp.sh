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

$CXX -g -std=c++17 src/main.cpp src/window.cpp src/fileHandling.cpp \
src/calendar.cpp \
src/world/tile.cpp src/world/world.cpp src/world/SimplexNoise.cpp \
src/objects/food.cpp src/objects/gameObject.cpp src/objects/spawner.cpp \
src/statistics/statistics.cpp \
src/objects/creature/creature.cpp src/objects/creature/genome.cpp \
src/objects/creature/navigator.cpp \
src/rendering/RenderSystem.cpp \
src/rendering/backends/ncurses/NCursesColorMapper.cpp \
src/rendering/backends/ncurses/NCursesRenderer.cpp \
src/rendering/backends/ncurses/NCursesInputHandler.cpp \
-lncurses -o EcoSim &> err

if [ -s err ]; then
    echo "❌ Compilation failed. Error output:"
    cat err
    echo ""
    echo "Opening error file in editor..."
    if command -v vim &> /dev/null; then
        vim err
    elif command -v nano &> /dev/null; then
        nano err
    else
        echo "No editor found. Error details above."
    fi
    exit 1
else
    rm -f err
    echo "✅ Compilation successful!"
    echo "Run with: ./EcoSim"
fi
