# Building EcoSim

## Quick Start

### macOS
```bash
# Install ncurses (usually pre-installed)
brew install ncurses

# Clone and build
cd EcoSim
bash comp.sh
./EcoSim
```

### Linux
```bash
# Install ncurses
sudo apt-get install libncurses5-dev libncursesw5-dev  # Debian/Ubuntu
sudo yum install ncurses-devel                          # RHEL/CentOS
sudo pacman -S ncurses                                  # Arch

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
sudo apt-get install build-essential libncurses5-dev
bash comp.sh
./EcoSim
```

## Build Options

The compilation script automatically detects your system:
- **macOS**: Uses `clang++` by default
- **Linux**: Uses `g++` by default
- **Custom compiler**: `CXX=g++-11 bash comp.sh`

## Compilation Details

### Compiler Flags
- `-g`: Debug symbols
- `-std=c++17`: C++17 standard
- `-lncurses`: Link ncurses library

### Build Artifacts
- `EcoSim`: Main executable
- `err`: Temporary error log (deleted on success)

## Troubleshooting

### "ncurses.h not found"
**macOS**: `brew install ncurses`
**Linux**: `sudo apt-get install libncurses5-dev`

### "clang: error: linker command failed"
Check ncurses is installed: `pkg-config --libs ncurses`

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
