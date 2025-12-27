# Building EcoSim

## Quick Start

### macOS/Linux
```bash
# Install dependencies
brew install cmake sdl2          # macOS
sudo apt-get install cmake libsdl2-dev  # Debian/Ubuntu

# Build with CMake
cmake -B build
cmake --build build
./build/EcoSim
```

### Windows (WSL)
```bash
# Install WSL2 with Ubuntu
wsl --install

# Inside WSL
sudo apt-get update
sudo apt-get install build-essential cmake libsdl2-dev
cmake -B build
cmake --build build
./build/EcoSim
```

## CMake Build

CMake provides a modern, flexible build system with out-of-source builds and configurable options.

### Default Build (SDL2 + ImGui)
```bash
cmake -B build
cmake --build build
./build/EcoSim
```
Features: Hardware-accelerated rendering, ImGui debug panels, graphical UI.

### NCurses-Only Build
```bash
# Install ncurses if needed
brew install ncurses           # macOS
sudo apt-get install libncurses5-dev  # Linux

cmake -B build -DECOSIM_USE_SDL2=OFF
cmake --build build
./build/EcoSim
```
Features: Runs in terminal, minimal dependencies, good for SSH sessions.

### Building Tests
```bash
cmake -B build -DECOSIM_BUILD_TESTS=ON
cmake --build build
```

### Running Tests
```bash
cd build && ctest           # Run all tests
ctest -V                    # Verbose output
ctest -R genetics           # Run tests matching "genetics"
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `ECOSIM_USE_SDL2` | ON | Build with SDL2 + ImGui support |
| `ECOSIM_USE_NCURSES` | ON | Build with NCurses support |
| `ECOSIM_BUILD_TESTS` | ON | Build test suite |

### Example Configurations

**Minimal build (NCurses only, no tests):**
```bash
cmake -B build -DECOSIM_USE_SDL2=OFF -DECOSIM_BUILD_TESTS=OFF
cmake --build build
```

**Full build with tests:**
```bash
cmake -B build -DECOSIM_BUILD_TESTS=ON
cmake --build build
cd build && ctest
```

**Release build:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Build Options

### Compiler Selection
- **macOS**: Uses `clang++` by default
- **Linux**: Uses `g++` by default
- **Custom compiler**: `cmake -B build -DCMAKE_CXX_COMPILER=g++-11`

### Compiler Flags
- `-g`: Debug symbols
- `-std=c++17`: C++17 standard
- `-lncurses`: Link ncurses library
- SDL2 flags via `find_package(SDL2)`

## Build Artifacts

All build artifacts are placed in the `build/` directory:
- `build/EcoSim`: Main executable
- `build/tests/GeneticsTest`: Test executable (if tests enabled)
- `build/libecosim_*.a`: Library files

## Troubleshooting

### "SDL2 not found"
**macOS**: `brew install sdl2`
**Linux**: `sudo apt-get install libsdl2-dev`

### "ncurses.h not found" (for terminal build)
**macOS**: `brew install ncurses`
**Linux**: `sudo apt-get install libncurses5-dev`

### "clang: error: linker command failed"
Check libraries are installed:
- SDL2: `sdl2-config --version`
- ncurses: `pkg-config --libs ncurses`

### CMake configuration fails
Ensure CMake is installed and up-to-date:
```bash
brew install cmake           # macOS
sudo apt-get install cmake   # Linux
cmake --version              # Should be 3.16+
```

### Terminal colors don't work
Use a modern terminal emulator:
- **macOS**: iTerm2 or Terminal.app
- **Linux**: gnome-terminal, konsole, or alacritty
- Set `TERM=xterm-256color` if needed

## Controls

Once running:
- **Arrow keys** or **HJKL**: Navigate map
- **Spacebar**: Pause/resume
- **F**: Toggle HUD
- **A**: Add 100 creatures
- **S**: Save world state
- **Escape**: Quit

## Project Structure

```
EcoSim/
├── CMakeLists.txt           # CMake build configuration
├── cmake/                   # CMake helper modules
├── build/                   # Build output (git-ignored)
├── include/                 # Header files
│   ├── genetics/            # Genetics system
│   ├── rendering/           # Rendering backends
│   └── statistics/          # Data collection
├── src/                     # Implementation files
│   └── testing/             # Test suite
├── saves/                   # Saved simulations
└── additional_documents/    # Research papers
```

## Next Steps

After building successfully:
1. Run `./build/EcoSim`
2. Select "New World" (option 1)
3. Press Enter to skip world editor
4. Watch the ecosystem evolve!

## Advanced: Custom Builds

### Optimized Release Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Debug Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Parallel Build
```bash
cmake --build build -j8    # Use 8 cores
```

### Static Analysis
```bash
clang-tidy src/*.cpp -- -std=c++17 -Iinclude
```

## Known Issues

1. **Window resizing**: May cause display issues - restart if needed
2. **Very small terminals**: Minimum 80x24 recommended
3. **Slow on large maps**: 500x500 default - performance depends on population

## Getting Help

- **Documentation**: See `additional_documents/Dissertation_Paper.pdf`
- **Issues**: Check existing saves in `saves/` for examples
- **Code questions**: All headers are well-documented
