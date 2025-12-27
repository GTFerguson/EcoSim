# How Inheritance Works

When two organisms reproduce, their offspring inherits genes from both parents. This creates variety—no two children are exactly alike, even from the same parents!

> 🎮 **Why This Matters:** Understanding inheritance helps you predict what offspring might look like and breed organisms with specific traits you want to see.

---

## The Basics

### You Have Two of Everything

Every organism has **two copies** of each gene—one inherited from each parent. Think of it like getting two versions of the same instruction:

```
          PARENT A                    PARENT B
         ┌────────┐                  ┌────────┐
         │ Speed: │                  │ Speed: │
         │  FAST  │                  │  SLOW  │
         └───┬────┘                  └────┬───┘
             │                            │
             └──────────┬─────────────────┘
                        │
                        ▼
                   ┌────────┐
                   │ Speed: │
                   │ FAST + │
                   │ SLOW   │
                   └────────┘
                   OFFSPRING
           (Has both versions!)
```

The offspring inherits BOTH versions, but which one actually shows up? That's where dominance comes in!

### The Coin Flip

When making a baby, each parent randomly passes on ONE of their two gene copies. It's like flipping a coin for each gene:

```
Parent A has: [Fast] [Slow]  →  Passes on: Fast (coin flip!)
Parent B has: [Slow] [Slow]  →  Passes on: Slow (only option)

Offspring gets: [Fast] [Slow]
```

This randomness means siblings can be very different from each other, even with the same parents!

---

## Inheritance Patterns

### Complete Dominance

Some gene versions are **stronger** than others. When you have one of each, the dominant version wins completely.

**Example: The Speed Gene**

```
┌─────────────────────────────────────────────────┐
│ Parent A: [Fast] [Slow]  - Appears FAST         │
│ Parent B: [Slow] [Slow]  - Appears SLOW         │
│                                                  │
│ Offspring possibilities:                         │
│ • [Fast] [Slow] = Appears FAST (dominant wins!) │
│ • [Slow] [Slow] = Appears SLOW                  │
└─────────────────────────────────────────────────┘
```

The "fast" gene dominates the "slow" gene, so any organism with at least one "fast" gene appears fast. You'd need TWO "slow" genes to actually be slow.

**What You'll See:**
- A fast parent and slow parent can have fast OR slow offspring
- Two fast parents might have slow offspring (surprise!) if both carry hidden "slow" genes

### Incomplete Dominance

Sometimes neither version wins completely—they **blend together** instead!

**Example: The Color Gene**

```
┌─────────────────────────────────────────────────┐
│ Parent A: [Red] [Red]    - Appears RED          │
│ Parent B: [Blue] [Blue]  - Appears BLUE         │
│                                                  │
│ All offspring:                                   │
│ • [Red] [Blue] = Appears PURPLE (blended!)      │
└─────────────────────────────────────────────────┘
```

In EcoSim, many genes work this way. Size, metabolism, and most continuous traits blend between the two inherited values.

**What You'll See:**
- Offspring often have traits "between" their parents
- Gradual variations rather than stark differences
- Smooth spectrum of traits across a population

### Codominance

With codominance, **both versions show up** equally and visibly!

**Example: The Pattern Gene**

```
┌─────────────────────────────────────────────────┐
│ Parent A: [Spots] [Spots]    - Has spots        │
│ Parent B: [Stripes] [Stripes] - Has stripes     │
│                                                  │
│ Offspring:                                       │
│ • [Spots] [Stripes] = Has BOTH spots AND stripes│
└─────────────────────────────────────────────────┘
```

**What You'll See:**
- Offspring that display characteristics from both parents simultaneously
- Complex trait combinations

### Overdominance (Hybrid Vigor)

Here's a twist: sometimes having **two DIFFERENT versions** is actually **better** than having two of the same!

**Example: The Immune Gene**

```
┌─────────────────────────────────────────────────┐
│ [TypeA] [TypeA] = Moderate immunity             │
│ [TypeB] [TypeB] = Moderate immunity             │
│ [TypeA] [TypeB] = STRONG immunity! ⭐           │
└─────────────────────────────────────────────────┘
```

This is called "hybrid vigor" and it's why genetic diversity is valuable!

**What You'll See:**
- Mixed-heritage organisms sometimes outperform "purebred" ones
- Populations with diverse genes are often healthier overall
- Inbreeding (same genes meeting same genes) can lead to weaker offspring

---

## The Expression System

In EcoSim, gene expression determines how strongly a trait appears. Here's how it works:

### Expression Strength

Each gene copy has an **expression strength** value:

```
Gene: Locomotion
├── Copy from Parent A: value=1.5, expression=0.8 (strong)
└── Copy from Parent B: value=0.3, expression=0.2 (weak)

Final trait = weighted average based on expression
             = (1.5 × 0.8 + 0.3 × 0.2) / (0.8 + 0.2)
             = 1.26 (closer to the strongly-expressed parent)
```

### What Affects Expression?

| Factor | Effect |
|--------|--------|
| **Gene Version** | Some variants express more strongly naturally |
| **Environment** | Stress can suppress or enhance gene expression |
| **Other Genes** | Some genes influence how others are expressed |
| **Random Chance** | Small variations occur naturally |

---

## Mutation

Sometimes genes **change** during reproduction. These mutations are the engine of evolution!

### Types of Mutations

#### Small Changes (Creep Mutations)

Most mutations are tiny adjustments to existing values:

```
Parent's gene: Speed = 1.5
                 ↓
          Small mutation!
                 ↓  
Offspring's gene: Speed = 1.52 (+0.02)
```

These happen frequently and allow gradual adaptation.

**What You'll See:**
- Offspring slightly different from parents
- Population traits slowly shifting over generations
- Smooth, continuous evolution

#### Major Mutations (Jumps)

Rarely, a gene changes dramatically:

```
Parent's gene: Speed = 1.5
                 ↓
          Big mutation!
                 ↓  
Offspring's gene: Speed = 0.3 (completely different!)
```

These are uncommon but can create interesting new variants.

**What You'll See:**
- Occasional "weird" offspring very different from parents
- Potential for new strategies to emerge
- Sometimes harmful, sometimes beneficial, often interesting!

#### New Gene Activation

The rarest and most exciting mutation: a **dormant gene wakes up**!

```
Plant genome:
├── Photosynthesis: ACTIVE
├── Root Depth: ACTIVE
└── Locomotion: dormant (value exists but expression = 0.0)
                 ↓
          Rare mutation activates locomotion!
                 ↓  
└── Locomotion: ACTIVE (expression = 0.1 now!)
```

This is how plants might eventually learn to move, or creatures might develop photosynthesis!

---

## Inheritance in Action

### The Punnett Square

A classic tool for predicting offspring! Let's say we're tracking a simple gene with two versions: [A] and [B].

```
                    Parent B
                 ┌─────┬─────┐
                 │ [A] │ [B] │
           ┌─────┼─────┼─────┤
Parent A   │ [A] │ AA  │ AB  │
           ├─────┼─────┼─────┤
           │ [B] │ AB  │ BB  │
           └─────┴─────┴─────┘

Offspring possibilities:
• 25% chance: AA
• 50% chance: AB  
• 25% chance: BB
```

### Multi-Gene Inheritance

Real organisms have **88 genes**, and each is inherited independently! This creates enormous variety:

```
Parent A: Fast, Red, Large, Aggressive, Long-lived...
Parent B: Slow, Blue, Small, Docile, Short-lived...

Possible offspring combinations: 2⁸⁸ ≈ 10²⁶ different possibilities!
```

Even with the same parents, you'll rarely see identical siblings.

---

## What You'll Observe in EcoSim

### Generational Patterns

| Generation | What You'll See |
|------------|-----------------|
| **1st** | Offspring look somewhat like parents |
| **2nd** | More variation appears |
| **3rd+** | Hidden traits resurface, new combinations emerge |

### The "Grandparent Effect"

Recessive genes can **skip generations**:

```
Grandparent A: [Fast] [slow]  →  Parent: [Fast] [slow]  →  Child: [slow] [slow]
                                     ↑                          ↑
                               Still appears fast!         Surprise! Slow!
```

That "slow" gene was hiding for a whole generation before appearing again!

### Population Drift

Over time, you'll see gene frequencies shift:

```
Generation 1:  [████████░░░░] 70% Fast genes
Generation 10: [██████░░░░░░] 55% Fast genes  
Generation 25: [████░░░░░░░░] 35% Fast genes
```

This happens due to random chance and natural selection (see the Evolution guide!).

---

## Tips for Breeding

### If You Want Consistent Traits

1. **Find two organisms with the same trait** - they're more likely to pass it on
2. **Watch for several generations** - hidden genes might appear
3. **Breed siblings carefully** - good for concentrating traits, but risks inbreeding weakness

### If You Want Variety

1. **Breed very different organisms** - maximizes genetic diversity
2. **Introduce new individuals** - brings fresh genes into the population
3. **Wait for mutations** - they add new possibilities

### Signs of Genetic Health

✅ **Healthy population:**
- Variety of traits visible
- Some hybrid vigor (mixed individuals thriving)
- Occasional mutations appearing

⚠️ **Concerning signs:**
- Everyone looks identical
- Offspring weaker than parents
- No new traits appearing

---

## Common Questions

### "Why doesn't my offspring have ANY of my selected parent's traits?"

Bad luck with the coin flip! Each gene is inherited randomly. With 88 genes, it's possible (though unlikely) for offspring to inherit mostly from one parent.

### "I bred two fast creatures but got a slow offspring?"

Both parents probably carried a hidden "slow" gene: [Fast] [slow]. When both passed on their "slow" copy, you got a [slow] [slow] offspring!

### "Can I 'breed out' a trait I don't want?"

Yes, but it takes time. Stop breeding organisms with that trait, and over generations it will become rarer. It may never disappear completely if it keeps hiding in recessive genes though!

### "Why are my offspring getting weirder over time?"

Mutations accumulate! Each generation adds small changes. Over many generations, these add up to significant differences from the original population.

---

## See Also

- 📖 [Complete Gene Reference](genes.md) - What each gene does
- 📖 [Evolution in Action](evolution.md) - How inheritance drives species change
- 📖 [Genetics Overview](README.md) - Introduction to the genetics system
