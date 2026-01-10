# Terrain and Biomes

The world of EcoSim is built from different terrain types, each creating unique conditions for life.

---

## Terrain Types

### Grassland 🌾

| Property | Value |
|----------|-------|
| **Color** | Light green |
| **Passable** | Yes |
| **Water Source** | No |
| **Plant Growth** | Excellent |

**Characteristics:**
- Open areas with good visibility
- Ideal for plant growth
- Herbivores thrive here
- Little cover from predators

### Forest 🌲

| Property | Value |
|----------|-------|
| **Color** | Dark green |
| **Passable** | Yes |
| **Water Source** | No |
| **Plant Growth** | Good |

**Characteristics:**
- Dense vegetation
- Provides cover
- Lower light at ground level
- Shade-adapted plants

### Water 💧

EcoSim features three types of water bodies, each with distinct characteristics:

#### Ocean 🌊

| Property | Value |
|----------|-------|
| **Color** | Deep navy to light coastal blue |
| **Passable** | No |
| **Water Source** | Yes |
| **Depth Variation** | Continuous gradient |

**Characteristics:**
- Connected to map edges
- Color darkens with depth (coastal shallows → deep navy abyss)
- Creates natural world boundaries
- Impassable terrain barrier

#### Lakes 🏞️

| Property | Value |
|----------|-------|
| **Color** | Light to medium blue (freshwater tones) |
| **Passable** | Yes (shallow) |
| **Water Source** | Yes |
| **Formation** | Terrain depressions with river inflow |

**Characteristics:**
- Form in natural terrain basins
- Deeper in center, shallow at edges
- Color reflects depth (darker center, lighter shores)
- Size depends on watershed and terrain shape
- Creatures can wade through shallow areas

#### Rivers 🏊

| Property | Value |
|----------|-------|
| **Color** | Light reflective blue |
| **Passable** | Yes |
| **Water Source** | Yes |
| **Formation** | Follow terrain downhill |

**Characteristics:**
- Flow from highlands to ocean or lakes
- Meander naturally through terrain
- Wider rivers appear slightly darker
- Provide water access across the map
- Creatures can cross rivers

### Mountain ⛰️

| Property | Value |
|----------|-------|
| **Color** | Gray/Brown |
| **Passable** | Varies |
| **Water Source** | No |
| **Plant Growth** | Limited |

**Characteristics:**
- High elevation
- May block movement
- Harsh conditions
- Cold temperatures at peaks

### Desert 🏜️

| Property | Value |
|----------|-------|
| **Color** | Yellow/Tan |
| **Passable** | Yes |
| **Water Source** | No |
| **Plant Growth** | Poor |

**Characteristics:**
- Hot, dry conditions
- Sparse vegetation
- Requires heat tolerance
- Water sources rare

---

## Elevation

The world has varying elevation levels that affect:

| Factor | Effect of Higher Elevation |
|--------|---------------------------|
| **Temperature** | Generally colder |
| **Water** | Flows downhill |
| **Terrain** | Changes by elevation |
| **Movement** | May be harder to traverse |

### Elevation Zones

```
HIGH    ▲ Mountains, peaks
        │ Rocky terrain
        │ Hills
        │ Plains
LOW     ▼ Water, valleys
```

---

## World Generation Controls

You can modify world generation:

| Key | Action |
|-----|--------|
| **N** | New seed (new world) |
| **Page Up/Down** | Adjust scale |
| **+/-** | Adjust frequency |
| **V/C** | Adjust exponent |
| **1-9** | Select terrain type |
| **W/Q** | Raise/lower terrain |

---

## How Terrain Affects Life

### For Plants

| Terrain | Plant Success |
|---------|---------------|
| Grassland | ★★★★★ Excellent |
| Forest | ★★★★☆ Good |
| Desert | ★★☆☆☆ Poor |
| Mountain | ★★☆☆☆ Limited |
| Water | ☆☆☆☆☆ None |

### For Creatures

| Terrain | Creature Activity |
|---------|-------------------|
| Grassland | High visibility, easy movement |
| Forest | Cover, blocked sightlines |
| Desert | Heat stress, scarce resources |
| Mountain | Movement barriers |
| Water | Must visit for drinking |

---

## Biome Patterns

### Natural Formations

The world generator creates realistic patterns:

- **Rivers** flowing from high to low
- **Lakes** in low elevation areas
- **Mountain ranges** as barriers
- **Plains** in mid-elevation

### Ecosystem Effects

Different areas support different life:

```
Mountains (cold, harsh)
    │
    ▼
Foothills (transitional)
    │
    ▼
Plains (temperate, productive)
    │
    ▼
Valleys/Rivers (water access)
```

---

## Try This! 🔬

### Experiment: Terrain Impact

1. Find creatures in different terrains
2. Compare survival rates
3. Which terrain supports most life?

### Experiment: Water Distance

1. Map creature locations vs water
2. Note travel patterns
3. Is water access affecting population distribution?

---

## See Also

- 📖 **[World Overview](README.md)** - General world information
- 📖 **[Scent System](scent-communication.md)** - Navigation via scent
- 📖 **[Creature Needs](../creatures/needs.md)** - How terrain affects needs
