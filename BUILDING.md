# Building EcoSim

## Quick Start

### macOS
```bash
# Install SDL2 (required for default build)
brew install sdl2

# Clone and build
cd EcoSim
bash comp.sh
./EcoSim
```

### Linux
```bash
# Install SDL2 (required for default build)
sudo apt-get install libsdl2-dev                        # Debian/Ubuntu
sudo yum install SDL2-devel                             # RHEL/CentOS
sudo pacman -S sdl2                                     # Arch

# Build
bash comp.sh
./EcoSim
```

### Windows (WSL)
```bash
# Install WSL2 with Ubuntu
wsl --install

# Inside WSL, same as Linux
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev
bash comp.sh
./EcoSim
```

## Build Scripts

| Script | Description | Output |
|--------|-------------|--------|
| `comp.sh` | **Default** - SDL2 + Dear ImGui (recommended) | `EcoSim` |
| `comp_ncurses.sh` | Terminal-only version (ncurses) | `EcoSim` |

### Default Build (SDL2 + ImGui)
```bash
bash comp.sh
./EcoSim
```
Features: Hardware-accelerated rendering, ImGui debug panels, graphical UI.

### Terminal Build (ncurses)
```bash
# Install ncurses if needed
brew install ncurses           # macOS
sudo apt-get install libncurses5-dev  # Linux

bash comp_ncurses.sh
./EcoSim
```
Features: Runs in terminal, minimal dependencies, good for SSH sessions.

## Build Options

The compilation scripts automatically detect your system:
- **macOS**: Uses `clang++` by default
- **Linux**: Uses `g++` by default
- **Custom compiler**: `CXX=g++-11 bash comp.sh`

## Compilation Details

### Compiler Flags
- `-g`: Debug symbols
- `-std=c++17`: C++17 standard
- `-lncurses`: Link ncurses library (both builds)
- SDL2 flags via `sdl2-config` (default build)

### Build Artifacts
- `EcoSim`: Main executable
- `err`: Temporary error log (deleted on success)

## Troubleshooting

### "SDL2 not found" or "sdl2-config: command not found"
**macOS**: `brew install sdl2`
**Linux**: `sudo apt-get install libsdl2-dev`

### "ncurses.h not found" (for terminal build)
**macOS**: `brew install ncurses`
**Linux**: `sudo apt-get install libncurses5-dev`

### "clang: error: linker command failed"
Check libraries are installed:
- SDL2: `sdl2-config --version`
- ncurses: `pkg-config --libs ncurses`

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
├── comp.sh              # Build script (THIS)
├── include/             # Header files
│   ├── objects/         # Game objects
│   ├── world/           # World generation
│   └── statistics/      # Data collection
├── src/                 # Implementation files
├── saves/               # Saved simulations
└── additional_documents/# Research papers
```

## Next Steps

After building successfully:
1. Run `./EcoSim`
2. Select "New World" (option 1)
3. Press Enter to skip world editor
4. Watch the ecosystem evolve!

## Advanced: Custom Builds

### Optimized Build
```bash
CXX=clang++ CXXFLAGS="-O3 -march=native" bash comp.sh
```

### Debug Build (default)
```bash
bash comp.sh  # Already includes -g flag
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
