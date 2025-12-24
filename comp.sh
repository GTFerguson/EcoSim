#!/bin/bash
# EcoSim Compilation Script (Default: SDL2 + Dear ImGui)
# Works on: Linux, macOS, WSL
# Prerequisites:
#   - macOS: brew install sdl2
#   - Linux: sudo apt-get install libsdl2-dev
#   - Check: sdl2-config --version
#
# For ncurses-only build (terminal version), use: bash comp_ncurses.sh

# Detect compiler (prefer clang++ on macOS, g++ elsewhere)
if [[ "$OSTYPE" == "darwin"* ]]; then
    CXX="${CXX:-clang++}"
else
    CXX="${CXX:-g++}"
fi

echo "=== EcoSim Build (SDL2 + Dear ImGui) ==="
echo "Using compiler: $CXX"

# Check for SDL2
if ! command -v sdl2-config &> /dev/null; then
    echo "❌ SDL2 not found. Please install:"
    echo "   macOS:  brew install sdl2"
    echo "   Linux:  sudo apt-get install libsdl2-dev"
    exit 1
fi

SDL2_VERSION=$(sdl2-config --version)
echo "SDL2 version: $SDL2_VERSION"

# Check for ImGui files
if [ ! -f "external/imgui/imgui.cpp" ]; then
    echo "❌ Dear ImGui files not found in external/imgui/"
    echo "   Please run the download script or manually download ImGui"
    exit 1
fi
echo "Dear ImGui: Found in external/imgui/"

# Get SDL2 compile and link flags
SDL2_CFLAGS=$(sdl2-config --cflags)
SDL2_LIBS=$(sdl2-config --libs)

echo "SDL2 CFLAGS: $SDL2_CFLAGS"
echo "SDL2 LIBS: $SDL2_LIBS"
echo ""

# Backend defines - enable ncurses, SDL2, and ImGui
DEFINES="-DECOSIM_HAS_NCURSES=1 -DECOSIM_HAS_SDL2=1 -DECOSIM_HAS_IMGUI=1"

# Include paths
INCLUDES="-Iinclude -Iexternal/imgui -Iexternal/imgui/backends"

echo "Compiling with NCurses, SDL2, and Dear ImGui..."
echo ""

$CXX -g -std=c++17 \
    $SDL2_CFLAGS \
    $DEFINES \
    $INCLUDES \
    src/main.cpp \
    src/window.cpp \
    src/fileHandling.cpp \
    src/calendar.cpp \
    src/world/tile.cpp \
    src/world/world.cpp \
    src/world/SimplexNoise.cpp \
    src/world/ScentLayer.cpp \
    src/objects/food.cpp \
    src/objects/gameObject.cpp \
    src/objects/spawner.cpp \
    src/statistics/statistics.cpp \
    src/statistics/genomeStats.cpp \
    src/objects/creature/creature.cpp \
    src/objects/creature/genome.cpp \
    src/objects/creature/navigator.cpp \
    src/rendering/RenderSystem.cpp \
    src/rendering/backends/ncurses/NCursesColorMapper.cpp \
    src/rendering/backends/ncurses/NCursesRenderer.cpp \
    src/rendering/backends/ncurses/NCursesInputHandler.cpp \
    src/rendering/backends/sdl2/SDL2ColorMapper.cpp \
    src/rendering/backends/sdl2/SDL2Renderer.cpp \
    src/rendering/backends/sdl2/SDL2InputHandler.cpp \
    src/rendering/backends/sdl2/ImGuiOverlay.cpp \
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
    external/imgui/imgui.cpp \
    external/imgui/imgui_draw.cpp \
    external/imgui/imgui_tables.cpp \
    external/imgui/imgui_widgets.cpp \
    external/imgui/imgui_demo.cpp \
    external/imgui/backends/imgui_impl_sdl2.cpp \
    external/imgui/backends/imgui_impl_sdlrenderer2.cpp \
    -lncurses $SDL2_LIBS \
    -o EcoSim 2>&1 | tee err

if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo ""
    echo "❌ Compilation failed. See errors above."
    echo "Error log saved to: err"
    exit 1
else
    rm -f err
    echo ""
    echo "✅ Compilation successful!"
    echo ""
    echo "Run with: ./EcoSim"
    echo ""
    echo "Features:"
    echo "  - SDL2 hardware-accelerated rendering"
    echo "  - Dear ImGui debug panels"
    echo "  - Windows menu bar to toggle panels"
    echo "  - Statistics, World Info, and Performance panels"
    echo ""
    echo "Alternative: bash comp_ncurses.sh for terminal-only version"
fi