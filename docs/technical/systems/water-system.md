---
title: Water System Architecture
created: 2026-01-10
updated: 2026-01-11
status: implemented
tags: [water, hydrology, lakes, rivers, rendering, climate, creature-interaction]
---

# Water System Architecture

## Overview

The water system handles generation and rendering of all water features in EcoSim, including oceans, lakes, and rivers. It consists of two major subsystems:

1. **Hydrology Generation** - Creates realistic water bodies following terrain contours
2. **Depth-Based Rendering** - Renders water with smooth color gradients based on depth and type

**Key Source Files:**

- [`ClimateWorldGenerator.cpp`](../../../src/world/ClimateWorldGenerator.cpp:1) - Water feature generation
- [`SDL2ColorMapper.cpp`](../../../src/rendering/backends/sdl2/SDL2ColorMapper.cpp:1) - Water color mapping
- [`tile.hpp`](../../../include/world/tile.hpp:1) - Water depth storage

---

## Hydrology Generation

### Water Feature Types

The system generates three types of water features:

| Feature | Generation Method | Passable | Description |
|---------|------------------|----------|-------------|
| **Ocean** | Elevation below sea level | No | Connected to map edges |
| **Lake** | Basin-filling algorithm | Yes | Inland depressions with accumulated flow |
| **River** | Flow accumulation tracing | Yes | Channels following terrain gradient |

### River Generation

Rivers are generated using a **flow accumulation method** with meandering:

1. **Calculate flow direction** for every land cell (steepest descent with noise-based meandering)
2. **Sort cells by elevation** (highest first)
3. **Accumulate flow** from high to low elevation
4. **Mark river tiles** where flow exceeds threshold
5. **Form lakes** at local minima with high accumulated flow

```cpp
// Flow accumulation thresholds
float riverThreshold = maxFlow * 0.03f;      // Top ~3% of flow values
float majorRiverThreshold = maxFlow * 0.15f; // Wide rivers
```

**Meandering Behavior:**

On gentle slopes (gradient < 0.05), noise biases flow direction selection among valid downhill options, creating natural-looking curved paths rather than strictly following steepest descent.

### Basin-Filling Lake Formation

Lakes form using a **flood-fill algorithm** that respects natural terrain contours.

**Algorithm Overview:**

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Basin-Filling Lake Algorithm                      │
├─────────────────────────────────────────────────────────────────────┤
│  1. Identify local minimum with significant accumulated flow         │
│  2. Calculate water surface elevation based on inflow amount         │
│  3. Flood-fill from center using priority queue (lowest elev first)  │
│  4. Mark all tiles below water surface as lake                       │
│  5. Stop at shore (terrain >= water surface) or size limit           │
└─────────────────────────────────────────────────────────────────────┘
```

**Implementation Details:**

```cpp
void ClimateWorldGenerator::formLake(int centerX, int centerY, 
                                       float inflow, float maxFlow) {
    float centerElev = _elevationMap[centerX][centerY];
    
    // Water surface rises with more inflow
    float depthFactor = clamp(inflow / (maxFlow * 0.5f), 0.0f, 1.0f);
    float lakeWaterSurface = centerElev + 0.02f + depthFactor * 0.08f;
    
    // Priority queue processes lowest elevation first (simulates filling)
    std::priority_queue<PQEntry, vector<PQEntry>, greater<PQEntry>> frontier;
    
    // ... flood fill logic
}
```

**Key Parameters:**

| Parameter | Value | Purpose |
|-----------|-------|---------|
| `MAX_LAKE_TILES` | 500 | Prevents runaway expansion |
| Base water rise | 0.02 | Minimum water surface above terrain |
| Max water rise | 0.10 | Maximum water surface (0.02 + 0.08) |
| Depth factor divisor | `maxFlow * 0.5f` | Scales inflow to depth |

**Depth Calculation:**

For each tile in the lake basin:

```cpp
float depth = lakeWaterSurface - terrainElevation;
```

This gives realistic depth variation where:
- Center (lowest elevation) = deepest
- Edges (higher elevation) = shallower
- Shore (at water surface) = zero depth

### Integration with River Generation

Lakes form naturally where rivers terminate at terrain depressions:

1. Flow accumulation identifies cells with no downhill neighbor
2. If accumulated flow exceeds `riverThreshold * 2.0f`, a lake forms
3. Lake depth correlates with accumulated upstream flow
4. Larger watersheds produce bigger, deeper lakes

---

## Depth-Based Water Coloring

### WaterType Enum

Water rendering uses distinct color palettes for each water type:

```cpp
enum class WaterType {
    OCEAN,  // Full range from coastal to deep abyss
    LAKE,   // Freshwater, slightly warmer tones
    RIVER   // Flowing water, lighter and more reflective
};
```

**Header:** [`SDL2ColorMapper.hpp`](../../../include/rendering/backends/sdl2/SDL2ColorMapper.hpp:24)

### Color Palettes

Each water type has shallow and deep endpoint colors:

| Type | Shallow RGB | Deep RGB | Character |
|------|-------------|----------|-----------|
| **Ocean** | (75, 130, 180) | (5, 15, 55) | Light coastal blue → Dark navy |
| **Lake** | (55, 115, 165) | (30, 70, 120) | Lake blue → Deeper blue |
| **River** | (70, 135, 185) | (50, 100, 155) | Light river → Medium river |

> [!NOTE]
> Ocean has the widest color range to represent everything from tropical shallows to abyssal depths. Lake and river colors are warmer/greener to distinguish freshwater from saltwater.

### Color Blending Function

Water color is computed by linear interpolation between shallow and deep colors:

```cpp
SDL_Color SDL2ColorMapper::waterToColor(float depth, WaterType type) {
    // Clamp depth to [0.0, 1.0]
    depth = clamp(depth, 0.0f, 1.0f);
    
    SDL_Color shallow, deep;
    // Select palette based on type...
    
    return blendColors(shallow, deep, depth);
}
```

Where `blendColors` performs per-channel linear interpolation:

```cpp
SDL_Color blendColors(SDL_Color c1, SDL_Color c2, float t) {
    float invT = 1.0f - t;
    return {
        static_cast<Uint8>(c1.r * invT + c2.r * t),
        static_cast<Uint8>(c1.g * invT + c2.g * t),
        static_cast<Uint8>(c1.b * invT + c2.b * t),
        static_cast<Uint8>(c1.a * invT + c2.a * t)
    };
}
```

### Depth Storage

Water depth is stored per-tile for rendering:

```cpp
class Tile {
private:
    float _waterDepth;  // 0.0 = shallow, 1.0 = deep
    
public:
    float getWaterDepth() const;
    void setWaterDepth(float depth);
};
```

**Header:** [`tile.hpp`](../../../include/world/tile.hpp:46)

### Depth Calculation by Water Type

**Ocean Depth:**

Calculated from elevation relative to sea level:

```cpp
// In applyToGrid()
float oceanDepth = seaLevel - elevation;
waterDepth = clamp(oceanDepth / seaLevel, 0.0f, 1.0f);
```

This gives:
- At sea level surface: depth ≈ 0.0 (shallow color)
- At elevation 0: depth = 1.0 (deepest color)

**Lake Depth:**

Uses the stored `waterLevel` from basin-filling:

```cpp
waterDepth = clamp(climate.waterLevel, 0.0f, 1.0f);
```

The `waterLevel` field stores actual depth calculated during [`formLake()`](../../../src/world/ClimateWorldGenerator.cpp:1430).

**River Depth:**

Uses flow magnitude as a proxy for depth:

```cpp
waterDepth = clamp(climate.waterLevel, 0.0f, 1.0f);
```

Larger rivers (higher accumulated flow) appear darker/deeper.

---

## Visual Results

### Before vs After

The depth-based system eliminates hard color transitions:

| Aspect | Old System | New System |
|--------|------------|------------|
| Ocean | 3 discrete colors | Continuous gradient |
| Lakes | Single flat color | Center-to-edge gradient |
| Rivers | Uniform color | Width-based depth variation |
| Transitions | Visible banding | Smooth blending |

### Color Gradient Examples

```
Ocean (cross-section from shore to deep):
Shore ─────────────────────────────────────────── Deep
(75,130,180) ──gradient── (40,72,117) ──gradient── (5,15,55)
   ↑                          ↑                        ↑
Light blue              Mid blue               Dark navy

Lake (center to shore):
Shore ─────────────────────────────────────────── Center  
(55,115,165) ──gradient── (42,92,142) ──gradient── (30,70,120)
   ↑                          ↑                        ↑
Light lake              Mid depth              Deep center
```

---

## Configuration

### Generator Configuration

Water generation parameters in [`ClimateGeneratorConfig`](../../../include/world/ClimateWorldGenerator.hpp:191):

```cpp
struct ClimateGeneratorConfig {
    // Water features
    bool generateRivers = true;
    int maxRivers = 20;
    float riverSourceElevation = 0.55f;
    float riverSourceMoisture = 0.4f;
    float riverSpawnChance = 0.02f;
    bool generateLakes = true;
    
    // Sea level affects ocean extent
    float seaLevel = 0.30f;
};
```

### Rendering Integration

Water type is determined at render time based on tile properties:

```cpp
// Pseudocode for determining water type
if (tile.isOcean()) {
    type = WaterType::OCEAN;
} else if (tile.feature == TerrainFeature::LAKE) {
    type = WaterType::LAKE;
} else if (tile.feature == TerrainFeature::RIVER) {
    type = WaterType::RIVER;
}

color = waterToColor(tile.getWaterDepth(), type);
```

---

## Performance Considerations

### Generation Performance

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Flow direction | O(n) | Single pass over all cells |
| Flow accumulation | O(n log n) | Sort by elevation + linear pass |
| Lake formation | O(k) per lake | k = lake size, capped at 500 |
| Total rivers/lakes | ~O(n log n) | Dominated by sort |

### Rendering Performance

- Depth lookup: O(1) per tile (cached in `Tile::_waterDepth`)
- Color calculation: O(1) per tile (simple linear interpolation)
- No additional overhead vs old discrete color system

---

## Creature Water Interaction

### Adjacent Water Drinking

**Updated (2026-01-11):** Creatures can now drink water when adjacent to water tiles, not just when standing on water tiles.

**Distance Threshold:**

```cpp
const float WATER_ADJACENT_THRESHOLD = 0.1f;  // Within 0.1 units of water tile edge
```

A creature is considered "at water" if:
1. Standing directly on a water tile, OR
2. Within `WATER_ADJACENT_THRESHOLD` (0.1 units) of a water tile edge

**Why Adjacent Drinking?**

| Old Behavior | Problem |
|--------------|---------|
| Creatures must stand ON water | Land creatures couldn't drink from lakes/rivers |
| Had to step into water | Unrealistic—animals drink from shores |

| New Behavior | Benefit |
|--------------|---------|
| Can drink when adjacent | Creatures approach water's edge naturally |
| Shore drinking | Realistic animal behavior |

**Implementation:**

```cpp
bool Creature::isAdjacentToWater(const World& world) const {
    // Check if any of the 8 neighboring tiles is water
    int tileX = static_cast<int>(getWorldX());
    int tileY = static_cast<int>(getWorldY());
    
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            if (world.isWater(tileX + dx, tileY + dy)) {
                // Check if creature is close enough to tile edge
                float distToEdge = calculateDistanceToTileEdge(dx, dy);
                if (distToEdge <= WATER_ADJACENT_THRESHOLD) {
                    return true;
                }
            }
        }
    }
    return false;
}
```

**Affected Behaviors:**

- `Motivation::Thirsty` - Creatures search for and navigate to water
- `Action::Drinking` - Triggered when at water (on tile OR adjacent)

---

## See Also

- [[world-generation]] - Overall climate-based generation system
- [[world-system]] - World architecture overview
- [[../design/world-generation]] - Design rationale for climate system
- [[behavior-state]] - Creature behavior state system (thirsty motivation)

---

**Last Updated:** 2026-01-11
