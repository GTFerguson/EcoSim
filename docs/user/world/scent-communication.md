# Scent and Communication

Creatures in EcoSim don't just use their eyes - they use **scent** to find mates, detect danger, and navigate the world. This invisible layer of information is one of the most powerful systems in the simulation.

---

## What Is the Scent System?

The world has an invisible **scent layer** where:

- Creatures deposit chemical signals (pheromones)
- Scents spread and decay over time
- Other creatures detect and follow scent trails
- Different scent types convey different information

---

## Scent Types

| Scent Type | Icon | Purpose | Who Produces |
|------------|------|---------|--------------|
| **Mate-seeking** | 💕 | Find breeding partners | Breeding-ready creatures |
| **Territorial** | 🏠 | Mark territory | Some creatures |
| **Alarm** | ⚠️ | Warn of danger | Fleeing creatures |
| **Food trail** | 🍖 | Mark food sources | Successful foragers |
| **Predator** | 💀 | Warn of hunters | Predators (inadvertently) |

---

## How Scent-Based Mating Works

This is the primary use of the scent system!

### The Process

```
1. Creature enters breeding state
           │
           ▼
2. Deposits MATE_SEEKING scent
           │
           ▼
3. Scent spreads outward over time
           │
           ▼
4. Other creature detects scent
           │
           ▼
5. Follows scent gradient toward source
           │
           ▼
6. Creatures meet and potentially mate!
```

### Why Scent Is Powerful

| Visual Finding | Scent Finding |
|----------------|---------------|
| Limited to sight range | Can detect beyond sight |
| Blocked by obstacles | Spreads around obstacles |
| Instantaneous | Persists over time |
| Direct line only | Follows gradients |

Scent allows creatures to find mates even when they can't see each other!

---

## Scent Genes

### Production Genes

| Gene | Effect |
|------|--------|
| `scent_production` | How strong your scent deposits are |
| Higher = stronger trail | Easier for others to find you |
| Lower = weaker trail | Harder to detect, more stealthy |

### Detection Genes

| Gene | Effect |
|------|--------|
| `olfactory_acuity` | How well you can smell |
| Higher = better detection | Find mates/food easier |
| Lower = poor detection | Must rely on sight |

---

## Genetic Signatures

Each creature has a unique **genetic signature** encoded in their scent:

```
Creature's Genes ──► Signature Calculation ──► 8-value Array
                                                   │
                                                   ▼
                                         Encodes genetic info
```

This signature is used for:

### Mate Recognition

Creatures prefer mates that are:
- **Similar enough** for compatibility
- **Different enough** for healthy offspring

The signature helps assess this without direct inspection!

### Kin Detection

Recognizing relatives helps avoid:
- Inbreeding (breeding with close relatives)
- Wasted mating attempts

---

## Scent Behavior

### Depositing Scent

When a creature is ready to breed:
1. Every tick, deposits scent at current location
2. Scent strength based on `scent_production` gene
3. Signature encodes genetic information

### Following Scent

When a creature detects scent:
1. Samples scent at current location
2. Compares nearby locations for gradient
3. Moves toward stronger scent
4. Assesses signature compatibility

### Scent Decay

Scents don't last forever:
- Intensity decreases each tick
- Eventually fades to undetectable
- Creatures must keep depositing to maintain trail

---

## What You'll See

### Mating Behavior

| Observation | What's Happening |
|-------------|------------------|
| Creature suddenly changes direction | Detected a scent trail |
| Creature heading toward nothing visible | Following scent to distant mate |
| Two creatures meeting "randomly" | Both followed scent to each other |
| Creature wandering in breeding state | Depositing scent, searching |

### Visualizing Scent (Debug Mode)

When enabled, you can see scent trails:

| Color | Scent Type |
|-------|------------|
| Pink/Magenta | Mate-seeking pheromones |
| Yellow | Territorial markers |
| Red | Alarm scent |
| Orange | Food trail |
| Dark Red | Predator mark |

---

## Try This! 🔬

### Experiment: Watch Scent-Based Mating

1. Find a creature in breeding state (check Inspector)
2. Watch for other creatures approaching
3. Note: they may come from beyond visual range!

### Experiment: Olfactory Gene Comparison

1. Select a creature with high `olfactory_acuity`
2. Watch how quickly it finds mates
3. Compare to creature with low acuity
4. Which breeds more successfully?

### Experiment: Scent vs. Sight

1. Find two breeding creatures far apart
2. Can they see each other? (Check sight range)
3. Do they still find each other?
4. Scent bridges the gap!

---

## Evolution of Scent Traits

Over generations, scent genes evolve:

### High Scent Production

**Advantages:**
- Easier to be found by mates
- Larger "attraction radius"

**Disadvantages:**
- Also easier for predators to track!
- Energy cost to produce

### High Olfactory Acuity

**Advantages:**
- Find mates faster
- Detect distant food
- Better danger awareness

**Disadvantages:**
- Energy cost to maintain

### The Trade-offs

```
      STEALTH                    ATTRACTION
          │                           │
  Low scent production        High scent production
  Harder to find              Easy to find
  Also hard to be found       Also found by predators!
```

---

## Common Questions

### Why can't this creature find a mate?

Check:
- Is `olfactory_acuity` too low?
- Are there other breeding creatures nearby?
- Are they genetically compatible?
- Is the creature too far from others?

### Why do creatures seem to know where others are?

That's the scent system! Even without visual contact, creatures can:
- Smell that others have been nearby
- Follow trails to their source
- Detect if others are breeding-ready

### Can creatures hide their scent?

Low `scent_production` means:
- Weaker scent trail
- Harder to find as a mate
- Also harder for predators to track

---

## See Also

- 📖 **[Creature Behavior](../creatures/behavior.md)** - How scent affects behavior
- 📖 **[Gene Reference](../genetics/genes.md)** - Olfactory genes
- 📖 **[Evolution](../genetics/evolution.md)** - How scent traits evolve
