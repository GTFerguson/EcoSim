---
title: Headless Simulation Runner
created: 2026-01-11
updated: 2026-01-11
status: implemented
tags: [testing, debugging, headless, simulation, tools]
---

# Headless Simulation Runner

## Overview

HeadlessSimulation is a GUI-free simulation runner designed for rapid debugging of creature movement issues, segfaults, and behavioral bugs. It runs the full EcoSim simulation without SDL2 rendering, allowing maximum execution speed and detailed diagnostic output.

**Source File:** [`src/testing/headlessSimulation.cpp`](../../../src/testing/headlessSimulation.cpp:1)

**Build Output:** `./build/tests/HeadlessSimulation`

---

## Features

- **No GUI** - Runs without SDL2, maximum execution speed
- **Signal Handler** - Catches SIGSEGV/SIGABRT with stack trace output
- **Configurable Debug Logging** - Navigator and behavior debug flags
- **Biome-Based Population** - Spawns creatures appropriate to each biome
- **Periodic Status Reports** - Tracks population, deaths, births

---

## Usage

```bash
./build/tests/HeadlessSimulation [options]
```

### Command Line Options

| Option | Short | Description | Default |
|--------|-------|-------------|---------|
| `--ticks N` | `-t` | Number of simulation ticks to run | 1000 |
| `--population N` | `-p` | Initial creature population | 100 |
| `--seed N` | `-s` | World generation seed | random |
| `--width N` | `-w` | Map width | 200 |
| `--height N` | `-h` | Map height | 200 |
| `--interval N` | `-i` | Status report interval (ticks) | 100 |
| `--verbose` | `-v` | Enable verbose debug output | off |
| `--nav-debug` | | Enable navigator debug logging | off |
| `--behavior-debug` | | Enable creature behavior debug logging | off |
| `--help` | | Show help message | |

### Example Commands

**Basic run (1000 ticks, 100 creatures):**

```bash
./build/tests/HeadlessSimulation
```

**Extended run with verbose output:**

```bash
./build/tests/HeadlessSimulation -t 10000 -p 200 -v
```

**Debug movement issues:**

```bash
./build/tests/HeadlessSimulation --nav-debug --behavior-debug -t 500
```

**Reproducible test with specific seed:**

```bash
./build/tests/HeadlessSimulation -s 12345 -t 5000
```

---

## Debug Logging

### Navigator Debug (`--nav-debug`)

Enables `NAVIGATOR_DEBUG_LOG` in [`navigator.cpp`](../../../src/objects/creature/navigator.cpp:1), which outputs:
- Path calculations and targets
- Movement attempts and results
- Stuck creature detection
- Pathfinding failures

### Behavior Debug (`--behavior-debug`)

Enables `CREATURE_BEHAVIOR_DEBUG_LOG` in [`creature.cpp`](../../../src/objects/creature/creature.cpp:1), which outputs:
- Motivation changes
- Action state transitions
- Target acquisition and loss
- Behavior execution details

### Enabling in Source

Both debug modes are controlled by preprocessor macros (default: off):

```cpp
// In navigator.cpp
#define NAVIGATOR_DEBUG_LOG 0  // Set to 1 to enable

// In creature.cpp  
#define CREATURE_BEHAVIOR_DEBUG_LOG 0  // Set to 1 to enable
```

> [!NOTE]
> Command-line flags `--nav-debug` and `--behavior-debug` configure runtime logging via the Logger system, not these compile-time macros. For maximum debug output, enable both.

---

## Signal Handler

HeadlessSimulation installs signal handlers for crash diagnosis:

```cpp
signal(SIGSEGV, signalHandler);  // Segmentation fault
signal(SIGABRT, signalHandler);  // Abort
signal(SIGFPE, signalHandler);   // Floating point exception
```

### Crash Output

When a crash occurs, the signal handler reports:

```
========================================
  SIGNAL CAUGHT: 11 (SIGSEGV - Segmentation Fault)
========================================
State at crash:
  Current tick: 523
  Creature count: 87
  Last action: executing behavior for creature 42 motivation=1
========================================
Stack trace:
[... backtrace symbols ...]
========================================
```

This helps identify:
- Which tick the crash occurred on
- How many creatures were alive
- What action was being performed
- Full stack trace (on macOS/Linux)

---

## Status Output

Every `--interval` ticks (default: 100), the simulation prints:

```
[Tick   500] Pop:  142 | Deaths: OA=12 ST=5 DH=3 DC=0 PR=8 | Births: 45
```

With `--verbose`, additional motivation breakdown:

```
         Motivations: Content=67 Hungry=34 Thirsty=12 Amorous=21 Tired=8
```

### Death Codes

| Code | Meaning |
|------|---------|
| OA | Old Age |
| ST | Starvation |
| DH | Dehydration |
| DC | Discomfort (environmental stress) |
| PR | Predator (combat death) |

---

## World Initialization

HeadlessSimulation uses the full ClimateWorldGenerator for realistic biome-based worlds:

1. **World Generation** - Climate-based terrain, biomes, water features
2. **Plant Seeding** - Biome-appropriate plants via PlantFactory
3. **Plant Warm-up** - 50 ticks of plant growth before creatures spawn
4. **Biome Population** - Creatures spawned in appropriate biomes:

| Biome Category | Herbivore | Carnivore |
|----------------|-----------|-----------|
| Tundra/Arctic | Woolly Mammoth | Arctic Wolf |
| Desert | Desert Camel | Fennec Fox |
| Tropical | Jungle Elephant | Jaguar |
| Temperate | Standard Mix | Standard Mix |

---

## Use Cases

### 1. Debugging Segfaults

```bash
./build/tests/HeadlessSimulation -t 100000 -p 500
```

Run for many ticks with large population to reproduce rare crashes. Signal handler provides state at crash.

### 2. Movement Issue Diagnosis

```bash
./build/tests/HeadlessSimulation --nav-debug -t 1000 -v
```

Track navigator decisions, identify stuck creatures, analyze pathfinding.

### 3. Behavioral Testing

```bash
./build/tests/HeadlessSimulation --behavior-debug -t 500 -p 50 -v
```

Monitor creature decision-making, motivation changes, action transitions.

### 4. Performance Profiling

```bash
time ./build/tests/HeadlessSimulation -t 10000 -p 200
```

No rendering overhead—measures pure simulation performance. Final report includes ticks/second.

### 5. Reproducible Bug Reports

```bash
./build/tests/HeadlessSimulation -s 98765 -t 2000 -p 100
```

Fixed seed ensures reproducible world generation and creature spawning.

---

## Final Report

At completion, HeadlessSimulation outputs:

```
────────────────────────────────────────────────────────────────
[Headless] Simulation complete!
  Duration:       4523 ms
  Ticks/second:   221.1
  Final pop:      156
  Total deaths:
    Old age:      23
    Starvation:   15
    Dehydration:  8
    Discomfort:   2
    Predator:     31
  Total births:   112
────────────────────────────────────────────────────────────────
```

---

## Building

HeadlessSimulation is built as part of the test suite:

```bash
mkdir -p build && cd build
cmake ..
make HeadlessSimulation
```

Or build all tests:

```bash
make tests
```

---

## See Also

- [[behavior-state]] - Creature behavior state system
- [[../core/01-architecture]] - System architecture overview
- [[world-system]] - World generation and environment
