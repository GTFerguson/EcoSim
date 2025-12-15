#!/bin/bash
# EcoSim Compilation Script with SDL2 Support
# Works on: Linux, macOS, WSL
# Prerequisites: 
#   - macOS: brew install sdl2
#   - Linux: sudo apt-get install libsdl2-dev
#   - Check: sdl2-config --version

# Detect compiler (prefer clang++ on macOS, g++ elsewhere)
if [[ "$OSTYPE" == "darwin"* ]]; then
    CXX="${CXX:-clang++}"
else
    CXX="${CXX:-g++}"
fi

echo "=== EcoSim SDL2 Build ==="
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

# Get SDL2 compile and link flags
SDL2_CFLAGS=$(sdl2-config --cflags)
SDL2_LIBS=$(sdl2-config --libs)

echo "SDL2 CFLAGS: $SDL2_CFLAGS"
echo "SDL2 LIBS: $SDL2_LIBS"
echo ""

# Backend defines - enable both ncurses and SDL2
DEFINES="-DECOSIM_HAS_NCURSES=1 -DECOSIM_HAS_SDL2=1"

echo "Compiling with both NCurses and SDL2 backends..."

$CXX -g -std=c++17 \
    $SDL2_CFLAGS \
    $DEFINES \
    src/main.cpp \
    src/window.cpp \
    src/fileHandling.cpp \
    src/calendar.cpp \
    src/world/tile.cpp \
    src/world/world.cpp \
    src/world/SimplexNoise.cpp \
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
    -lncurses $SDL2_LIBS \
    -o EcoSim_SDL2 &> err_sdl

if [ -s err_sdl ]; then
    echo "❌ Compilation failed. Error output:"
    cat err_sdl
    echo ""
    echo "Opening error file in editor..."
    if command -v vim &> /dev/null; then
        vim err_sdl
    elif command -v nano &> /dev/null; then
        nano err_sdl
    else
        echo "No editor found. Error details above."
    fi
    exit 1
else
    rm -f err_sdl
    echo ""
    echo "✅ Compilation successful!"
    echo "Executable: EcoSim_SDL2"
    echo ""
    echo "Run with: ./EcoSim_SDL2"
    echo ""
    echo "Note: By default, the simulation will use SDL2 when available."
    echo "      To force NCurses mode, modify main.cpp to select BACKEND_NCURSES."
fi