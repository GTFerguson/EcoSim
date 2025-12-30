# EcoSim Usage Guide

A comprehensive guide to using the EcoSim evolutionary simulation.

---

## Table of Contents

1. [Building the Application](#building-the-application)
2. [Controls](#controls)
3. [UI Elements](#ui-elements)
4. [Simulation Speed](#simulation-speed)
5. [Understanding the Display](#understanding-the-display)

---

## Building the Application

For detailed build instructions, see [`BUILDING.md`](../../BUILDING.md) in the project root.

### Quick Build (macOS/Linux)

```bash
# Clone the repository
git clone <repository-url>
cd EcoSim

# Build with SDL2 (graphical mode)
./comp_sdl.sh

# Or build with ncurses (terminal mode)
./comp.sh
```

### Dependencies

- **macOS/Linux**: ncurses (typically pre-installed)
- **SDL2 Build**: SDL2 library
- **C++17 compatible compiler**

---

## Controls

### Navigation Controls

| Key | Action | Description |
|-----|--------|-------------|
| `↑` / `K` | Move Up | Pan camera/viewport up |
| `↓` / `J` | Move Down | Pan camera/viewport down |
| `←` / `H` | Move Left | Pan camera/viewport left |
| `→` / `L` | Move Right | Pan camera/viewport right |

> **Note:** Vim-style HJKL navigation is fully supported.

### Simulation Controls

| Key      | Action        | Description                                                    |
| -------- | ------------- | -------------------------------------------------------------- |
| `Space`  | Pause/Resume  | Toggle simulation pause state                                  |
| `A`      | Add Creatures | Spawn 100 new creatures into the world                         |
| `S`      | Quick Save    | Quick save to legacy format                                    |
| `Escape` | Pause Menu    | Opens the pause menu with Save, Load, Resume, and Quit options |

### UI Controls

| Key | Action | Description |
|-----|--------|-------------|
| `F` | Toggle HUD | Show/hide the heads-up display |
| `F1` | Statistics Window | Population counts, births, deaths, charts |
| `F2` | World Info | Dimensions, generation parameters |
| `F3` | Performance | FPS, frame times, performance metrics |
| `F4` | Creature List | Filterable, sortable list of all creatures |
| `F5` | Creature Inspector | Detailed view of selected creature |
| `F6` | Controls Help | On-screen keyboard shortcuts reference |

### World Editor Controls

| Key | Action | Description |
|-----|--------|-------------|
| `Page Up` | Increase Scale | Larger terrain features |
| `Page Down` | Decrease Scale | Smaller terrain features |
| `N` | New Seed | Generate new world with random seed |
| `=` / `+` | Increase Frequency | Higher terrain frequency |
| `-` | Decrease Frequency | Lower terrain frequency |
| `V` | Increase Exponent | Steeper terrain falloff |
| `C` | Decrease Exponent | Gentler terrain falloff |
| `1-9` | Select Terrain | Choose terrain type (1-9) |
| `W` | Increase Terrain Level | Raise selected terrain |
| `Q` | Decrease Terrain Level | Lower selected terrain |

### Zoom Controls

| Key | Action |
|-----|--------|
| `+` / `=` | Zoom In |
| `-` | Zoom Out |
| `Mouse Wheel` | Zoom In/Out |

### Mouse Controls (SDL2 Mode)

| Button | Action |
|--------|--------|
| Left Click | Select creature/tile |
| Right Click | Context menu |
| Middle Click | Center view on position |
| Mouse Wheel | Zoom in/out |

---

## UI Elements

### Statistics Window (F1)

The statistics window provides real-time information about the simulation:

- **Population Counts**: Current number of creatures and plants
- **Birth/Death Rates**: Creatures born and died per tick
- **Population Charts**: Graphs showing population over time
- **Gene Statistics**: Distribution of gene values across population

### World Info Window (F2)

Displays world generation parameters:

- **Dimensions**: World width and height in tiles
- **Generation Seed**: Random seed used for world generation
- **Terrain Parameters**: Scale, frequency, exponent values
- **Biome Distribution**: Percentage of each biome type

### Performance Window (F3)

Technical performance metrics:

- **FPS**: Frames per second
- **Frame Time**: Milliseconds per frame
- **Tick Time**: Time to process simulation logic
- **Render Time**: Time to draw the world

### Creature List (F4)

Sortable and filterable list of all creatures:

- **Sort by**: ID, Age, Energy, Generation
- **Filter by**: Diet type, Profile state, Health status
- Click on a creature to select and track it

### Creature Inspector (F5)

A comprehensive tabbed interface for viewing detailed creature information. The inspector opens when a creature is selected and provides deep insights into all aspects of the creature's biology and behavior.

#### Header (Always Visible)

The header appears above all tabs and shows:

- **Common Name**: Dynamically generated name based on behavior and traits (e.g., "Striped Runner")
- **Scientific Name**: Genus species format (e.g., *Carnisocialis parvus velocis*)
- **ID, Age, and Lifespan**: Basic identification and life stage information

#### Tab 1: Overview

At-a-glance summary of the most important information:

- **Current Behavior**: What the creature is doing and why
- **Diet Classification**: Emergent diet type based on genes (Herbivore/Carnivore/Omnivore/Frugivore)
- **Size & Position**: Current location in the world
- **Key Stats Summary**: Health and hunger bars
- **Notable Traits**: Highlighted extreme or distinctive gene expressions

#### Tab 2: State

Current health, needs, and condition:

- **Vitals**: Health bar with wound state indicator
- **Needs**: Four progress bars showing:
  - Hunger (0-10 scale)
  - Thirst (0-10 scale)
  - Fatigue (0-10 scale)
  - Mate drive (0-10 scale)
- **Energy Budget**: Available energy and metabolism information
- **Stress & Condition**: Hardiness gene value

#### Tab 3: Combat

Fighting capabilities and current combat status using the **Rock-Paper-Scissors damage system**:

- **Combat Status**: Whether in combat, target info, flee status
- **Combat Behavior Genes**: Aggression, retreat threshold, territorial instinct, pack coordination
- **Weapons Breakdown**: Teeth, claws, horns, antlers, tail weapons
  - Each weapon deals one of three damage types: **Piercing**, **Slashing**, or **Blunt**
- **Defense Breakdown**:
  - **Thick Hide**: Resists Piercing (0.5x), weak to Blunt (1.5x)
  - **Scales**: Resists Slashing (0.5x), weak to Piercing (1.5x)
  - **Fat Layer**: Resists Blunt (0.5x), weak to Slashing (1.5x)

**Type Effectiveness Table:**

| Attack Type | vs Hide | vs Scales | vs Fat |
|-------------|---------|-----------|--------|
| **Piercing** | 0.5x ❌ | 1.5x ✅ | 1.0x |
| **Slashing** | 1.0x | 0.5x ❌ | 1.5x ✅ |
| **Blunt** | 1.5x ✅ | 1.0x | 0.5x ❌ |

#### Tab 4: Physical

Body characteristics, senses, and mobility:

- **Morphology**: Body coverage (hide, fur, scales, fat), jaw & teeth, digestive system
- **Mobility**: Locomotion type, stamina, stride length, limb strength, turn speed
- **Sensory Abilities**: Vision (sight range, color vision), olfaction (scent detection, sensitivity, trail following)
- **Appearance**: Color genes (hue, saturation, value)

#### Tab 5: Genetics

Complete gene listing with 16 sub-tabs organizing all 88 genes:

| Sub-Tab | Genes | Description |
|---------|-------|-------------|
| Universal | 7 | Core traits: size, lifespan, metabolism, color, hardiness |
| Mobility | 5 | Movement: locomotion, stamina, stride, limb strength, turn speed |
| Heterotrophy | 13 | Feeding: digestion, hunting, foraging, toxin tolerance |
| Morphology | 9 | Body structure: hide, fur, scales, teeth, gut, jaw, tail |
| Seed | 2 | Plant coevolution: seed preference, viability bonus |
| Reproduction | 6 | Breeding: offspring count/size, maturity, mating display |
| Olfactory | 4 | Scent: sensitivity, trail following, production, specificity |
| Behavior | 5 | Cognitive: sight range, aggression, territorial, group affinity |
| Health | 3 | Healing: regeneration rate, wound tolerance, bleeding resistance |
| Cbt:Weapons | 13 | Offensive: teeth, claws, horns, antlers, tail, venom |
| Cbt:Defense | 3 | Protective: hide thickness, scale hardness, fat layer depth |
| Cbt:Behavior | 4 | Fighting tactics: combat aggression, retreat, territorial, pack |

Each gene shows its current value (0.00-1.00 normalized scale).

### Controls Window (F6)

On-screen reference of all keyboard shortcuts.

### Pause Menu (ESC)

The pause menu provides access to save/load functionality and game control:

- **Resume**: Close the menu and continue the simulation
- **Save Game**: Save current game state to `saves/SAVE_02/quicksave.json`
- **Load Game**: Load a previously saved game state
- **Quit**: Exit the simulation

The menu appears centered on screen with a semi-transparent overlay that darkens the simulation view. The simulation is automatically paused while the menu is open.

> **Note:** Save files use JSON format and include complete creature and plant state, including all genetic information.

---

## Saving and Loading

### Save File Location

Save files are stored in:
```
saves/<slot_name>/quicksave.json
```

The default save slot is `SAVE_02`.

### What Gets Saved

A complete save file includes:
- **Creatures**: Position, health, needs, genetics, behavior state
- **Plants**: Position, growth stage, energy, seed count, genetics
- **Calendar**: Current in-game time and date
- **World**: Current simulation tick

### Save File Format

Save files use human-readable JSON format, making them easy to inspect and modify:

```json
{
  "version": 1,
  "savedAt": "2025-12-28T12:00:00Z",
  "world": { "tick": 5000, "mapWidth": 500, "mapHeight": 500 },
  "calendar": { "minute": 30, "hour": 14, "day": 5, "month": 3, "year": 1 },
  "creatures": [ ... ],
  "plants": [ ... ]
}
```

---

## Simulation Speed

### Timing System

The simulation uses a **fixed timestep** system:

```
1 tick = 1000ms (1 real second)
1 tick = 1 in-game minute
60 ticks = 1 in-game hour
1440 ticks = 1 in-game day
```

### Time Passage

- The simulation runs at a fixed rate of 1 tick per second by default
- Each tick processes:
  - Creature movement and behavior
  - Plant growth and reproduction
  - Resource consumption
  - Scent decay and propagation
  - World updates

### Calendar System

The simulation includes a calendar that tracks:
- Current day
- Current season
- Environmental conditions that vary by season

---

## Understanding the Display

### Creature Representation

Creatures are displayed with colors indicating their characteristics:

| Visual | Meaning |
|--------|---------|
| **Size** | Physical size from `max_size` gene |
| **Color** | Determined by `color_hue` gene (HSV hue value) |
| **Movement** | Animation shows creature facing/moving direction |

### Creature Profiles (Behavior States)

Creatures cycle through different behavioral states:

| Profile | Description |
|---------|-------------|
| **Hungry** | Actively seeking food |
| **Thirsty** | Seeking water |
| **Breed** | Looking for a mate |
| **Flee** | Running from predator |
| **Idle** | Resting, no urgent needs |

### Plant Types

Plants are displayed based on their characteristics:

| Visual | Type | Description |
|--------|------|-------------|
| Green | Standard | Basic photosynthesizing plant |
| Dark Green | Hardy | Resistant, slower growing |
| Light Green | Fast | Quick growth, less durable |
| Fruiting | With fruit | Currently producing seeds/fruit |

### Terrain Types

| Terrain | Description |
|---------|-------------|
| **Grassland** | Open area, good for grazers |
| **Forest** | Dense vegetation, shelter |
| **Water** | Lakes and rivers |
| **Mountain** | High elevation, harsh conditions |
| **Desert** | Arid, sparse resources |

### Scent Visualization (Debug Mode)

When enabled, scent trails are displayed as:

| Color | Scent Type |
|-------|------------|
| Pink/Magenta | Mate-seeking pheromones |
| Yellow | Territorial markers |
| Red | Alarm scent |
| Orange | Food trail |
| Dark Red | Predator mark |

---

## Tips for New Users

1. **Start with Observation**: Let the simulation run for a few minutes without intervention to see natural dynamics

2. **Use the Inspector**: Press F5 and click on creatures to understand their behavior and genetics

3. **Watch Population Dynamics**: The F1 statistics window shows population trends

4. **Experiment with Spawning**: Press A to add creatures and observe how they integrate

5. **Try Different World Seeds**: Press N to generate new worlds with different terrain

6. **Save Interesting States**: Press ESC and use "Save Game" to preserve interesting evolutionary developments

---

## See Also

- [Getting Started Guide](getting-started.md) - Quick start for first-time users
- [Genetics: Inheritance](genetics/inheritance.md) - How genes are passed to offspring
- [Genetics: Evolution](genetics/evolution.md) - How populations change over time
- [Technical: Scent System](../technical/genetics/scent-system.md) - How scent-based mating works
