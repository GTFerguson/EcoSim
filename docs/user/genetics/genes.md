# Complete Gene Reference

## Quick Summary

| Category | Genes | Description |
|----------|:-----:|-------------|
| 🌍 Universal | 7 | Basic life traits |
| 🏃 Mobility | 5 | Movement and awareness |
| 🍖 Heterotrophy | 6 | Hunting and digestion |
| 🦴 Morphology | 9 | Physical anatomy |
| 👃 Olfactory | 7 | Scent and sensory detection |
| 🥚 Reproduction | 7 | Breeding and offspring |
| 🧠 Behavior | 8 | Behavioral patterns |
| ❤️ Health | 4 | Healing and immunity |
| ⚔️ Combat Weapons | 6 | Claws, horns, venom |
| 🛡️ Combat Defense | 6 | Shells, scales, evasion |
| 🎯 Combat Behavior | 5 | Aggression and tactics |
| 🌱 Seed Interaction | 2 | Digestive seed passage |
| 🌿 Plant-only | 16 | Photosynthesis, defense, seeds |
| **Total** | **88** | |

---

## Table of Contents

1. [The Gene System at a Glance](#the-gene-system-at-a-glance)
2. [Gene Categories](#gene-categories)
   - [Universal Genes](#-universal-genes)
   - [Mobility Genes](#-mobility-genes)
   - [Autotrophy Genes](#-autotrophy-genes)
   - [Heterotrophy Genes](#-heterotrophy-genes)
   - [Reproduction Genes](#-reproduction-genes)
   - [Plant Defense Genes](#️-plant-defense-genes-new-in-v20)
   - [Seed & Fruit Genes](#-seed--fruit-genes)
   - [Creature Interaction Genes](#-creature-interaction-genes)
3. [Coevolution](#-coevolution-the-big-picture)
4. [The Unified Gene Pool](#the-unified-gene-pool)
5. [Quick Reference Card](#quick-reference-card)
6. [Energy Budget Tip](#-energy-budget-tip)

---

Every organism in EcoSim carries genes organized into categories that determine everything from how long a creature lives to whether it can photosynthesize like a plant, defend against herbivores, or coevolve with other species!

> 🎮 **Pro Tip:** Don't worry about memorizing all of this! This reference is here when you want to understand what's happening in your ecosystem. Most players discover gene effects naturally through observation.

---

## The Gene System at a Glance

Think of genes like sliders on a character creation screen. Each gene has a value between its minimum and maximum, and that value determines how strong that trait is in the organism.

```
Low Value ◄═══════════════════════► High Value
   │                                    │
   │      Your organism is              │
   │      somewhere on this             │
   │      spectrum!                     │
   └────────────────────────────────────┘
```

### What's New in Version 2.0

🆕 **Energy Budget System** — Every gene now has a maintenance cost. More complex organisms need more energy!

🆕 **Emergent Diet Types** — Diet is no longer a fixed category. It emerges from your creature's digestion genes!

🆕 **Plant Defenses** — Plants can now fight back with toxins, thorns, and rapid regrowth!

🆕 **Coevolution** — Watch creatures and plants evolve in response to each other over generations!

🆕 **Seed Dispersal** — Six different ways plants can spread their seeds, from wind to animal transport!

---

## Gene Categories

### 🌍 Universal Genes

These **7 genes** are shared by ALL living things in EcoSim. Whether it's a towering tree or a tiny creature, these fundamental genes shape their basic existence.

| Gene | What It Does | Low Value | High Value |
|------|--------------|-----------|------------|
| **Lifespan** | How long the organism lives | Short-lived (~100 ticks) - Lives fast, dies young | Long-lived (~1,000,000 ticks) - Practically ancient |
| **Maximum Size** | How big it can grow | Tiny (0.5 units) - Easy to miss | Massive (20 units) - Dominates the landscape |
| **Metabolism Rate** | How fast it burns energy | Slow metabolism (0.1) - Needs little food, moves sluggishly | Fast metabolism (3.0) - Always hungry, very active |
| **Color Hue** | Visual appearance | Red/Orange (0°) | Through green, blue, purple, back to red (360°) |
| **Hardiness** | Resistance to damage and stress | Fragile (0.0) - Easily hurt | Tough as nails (1.0) - Shrugs off damage |
| **Cold Tolerance** | Minimum survival temperature | Needs warmth (30°C) - Only survives in tropical areas | Frost-resistant (-30°C) - Thrives in arctic conditions |
| **Heat Tolerance** | Maximum survival temperature | Heat-sensitive (10°C) - Only in cold climates | Desert-ready (60°C) - Handles scorching heat |

#### 🎯 Universal Genes in Action

**Scenario: The Long Winter**
> A cold snap hits your ecosystem. Organisms with high **Cold Tolerance** survive while those who need warmth perish. Over generations, you'll see the population shift toward cold-hardy genes!

**Scenario: The Calorie Wars**
> In a resource-scarce environment, organisms with **Low Metabolism** can survive on scraps, while high-metabolism creatures starve. But when food is plentiful, high-metabolism organisms are more active and reproduce faster!

---

### 🏃 Mobility Genes

These **5 genes** control movement and awareness. In creatures, these are fully active. In plants, they're dormant... but evolution has surprises in store!

| Gene | What It Does | Low Value | High Value |
|------|--------------|-----------|------------|
| **Locomotion** | Movement speed | Stationary (0.0) - Doesn't move at all | Swift (2.0) - Zooms across the map |
| **Sight Range** | How far it can see | Nearly blind (0) - Stumbles into things | Eagle-eyed (200) - Spots prey/predators from afar |
| **Navigation** | Pathfinding ability | Gets lost easily (0.0) - Wanders aimlessly | GPS brain (1.0) - Always finds optimal routes |
| **Flee Distance** | When to run from danger | Fearless (0) - Stands ground until too late | Paranoid (30) - Runs at first sign of threat |
| **Pursue Distance** | When to chase prey | Lazy hunter (0) - Only catches nearby food | Relentless pursuer (30) - Chases prey across the map |

#### 🎯 Mobility Genes in Action

**Scenario: The Predator-Prey Dance**
> A predator with high **Sight Range** but low **Locomotion** can spot prey easily but can't catch fast creatures. Over time, evolution might favor faster predators—or prey that's just slightly faster than average.

**Scenario: The Ambush Hunter**
> Some creatures evolve low **Pursue Distance** but high **Locomotion**. They don't chase prey—they wait for it to come close, then strike with lightning speed!

> 🌱 **Secret Feature:** Plants start with dormant mobility genes, but through rare mutations, plants CAN potentially evolve the ability to move! See the Evolution guide for more.

---

### 🌱 Autotrophy Genes

These **5 genes** enable self-feeding through photosynthesis and resource gathering. Active in plants, dormant in creatures—but who knows what evolution might bring?

| Gene | What It Does | Low Value | High Value |
|------|--------------|-----------|------------|
| **Photosynthesis** | Energy from sunlight | Can't photosynthesize (0.0) - Must find food | Solar-powered (1.0) - Makes all energy from light |
| **Root Depth** | Underground resource access | Shallow roots (0) - Only surface water | Deep roots (10) - Taps underground water reserves |
| **Water Storage** | Drought resistance | Needs constant water (0.0) - Wilts quickly | Camel-like (1.0) - Stores water for dry spells |
| **Light Requirement** | Sunlight needs | Shade-loving (0.0) - Thrives in darkness | Sun-worshipper (1.0) - Needs full sun |
| **Water Requirement** | Hydration needs | Drought-tolerant (0.0) - Survives on dew | Water-loving (1.0) - Needs constant moisture |

#### 🎯 Autotrophy Genes in Action

**Scenario: The Forest Floor**
> Under the canopy, light is scarce. Plants with low **Light Requirement** thrive, while sun-lovers struggle. Over generations, you'll see shade-adapted species dominate the understory!

**Scenario: Desert Adaptation**
> In dry regions, plants with high **Water Storage** and low **Water Requirement** survive droughts. Their **Root Depth** helps them find underground water when surface moisture is gone.

> 🦎 **Secret Feature:** Through rare mutations, creatures CAN potentially evolve photosynthesis genes! Imagine a creature that can both hunt AND make energy from sunlight...

---

### 🍖 Heterotrophy Genes

These **6 genes** control hunting, eating, and nutrition. Active in creatures, but even plants can potentially evolve carnivorous traits!

| Gene | What It Does | Low Value | High Value |
|------|--------------|-----------|------------|
| **Hunt Instinct** | Predatory drive | Pacifist (0.0) - Never attacks | Born killer (1.0) - Hunts aggressively |
| **Hunger Threshold** | When to seek food | Constantly hungry (0.0) - Always eating | Patient feeder (10.0) - Eats only when starving |
| **Thirst Threshold** | When to seek water | Always thirsty (0.0) - Drinks constantly | Desert-adapted (10.0) - Rarely needs water |
| **Diet Type** | What it can eat | Herbivore (0) - Plants only | Omnivore (1) - Everything | Scavenger (2) - Dead things | Predator (3) - Hunts prey |
| **Digestive Efficiency** | Nutrient extraction | Wasteful digestion (0.1) - Gets little from food | Perfect absorption (1.0) - Uses every calorie |
| **Nutrient Value** | Food value when eaten | Empty calories (1) - Not worth eating | Superfood (100) - One meal feeds for days |

#### 🎯 Heterotrophy Genes in Action

**Scenario: The Efficient Hunter**
> A predator with high **Hunt Instinct** and high **Digestive Efficiency** is a perfect killing machine—it hunts often AND extracts maximum energy from each meal.

**Scenario: The Walking Snack**
> An organism with high **Nutrient Value** is very appealing to predators! This creates evolutionary pressure: either become faster to escape, or become less nutritious to be ignored.

**Diet Types Explained:**

| Diet Type | Value | What It Means |
|-----------|-------|---------------|
| 🥬 **Herbivore** | 0 | Eats only plants. Peaceful but dependent on vegetation. |
| 🍽️ **Omnivore** | 1 | Eats everything. Flexible but master of nothing. |
| 💀 **Scavenger** | 2 | Eats dead things. No hunting, but limited food sources. |
| 🦁 **Predator** | 3 | Hunts prey. Powerful but needs hunting skills. |

---

### 🥚 Reproduction Genes

These **6 genes** control breeding, offspring, and physical comfort. All organisms have these, but they express differently!

| Gene | What It Does | Low Value | High Value |
|------|--------------|-----------|------------|
| **Offspring Count** | Babies per breeding | Single child (1) - Quality over quantity | Massive litter (20) - Quantity over quality |
| **Mate Threshold** | Breeding drive | Constantly ready (0.0) - Breeds whenever possible | Very selective (10.0) - Rarely breeds |
| **Spread Distance** | How far offspring travel | Stays close (0) - Offspring near parent | Far dispersal (30) - Offspring spread widely |
| **Fatigue Threshold** | Energy management | Never rests (0.0) - Active until collapse | Careful energy use (10.0) - Rests frequently |
| **Comfort Increase** | Recovery rate | Slow recovery (0.001) - Takes forever to rest | Quick recovery (0.02) - Bounces back fast |
| **Comfort Decrease** | Stress buildup | Stress-proof (0.001) - Rarely gets tired | Easily stressed (0.02) - Tires quickly |

#### 🎯 Reproduction Genes in Action

**Scenario: The Population Explosion**
> An organism with high **Offspring Count** and low **Mate Threshold** breeds constantly with large litters. Great for rapid population growth—if food is abundant!

**Scenario: The Careful Parent**
> High **Mate Threshold** means fewer but potentially healthier offspring. Combined with low **Offspring Count**, this strategy invests everything in a few well-placed children.

**Scenario: The Colonizer**
> High **Spread Distance** means offspring end up far from parents. Great for finding new resources, but risky if the new location is hostile!

---

### 🛡️ Plant Defense Genes (NEW in v2.0!)

Plants are no longer defenseless! These **3 genes** let plants fight back against herbivores.

| Gene | What It Does | Low Value | High Value |
|------|--------------|-----------|------------|
| **Toxin Production** | Poison that harms herbivores | Non-toxic (0.0) - Safe to eat | Deadly (1.0) - Damages creatures that eat it |
| **Thorn Density** | Physical defenses | Smooth (0.0) - Easy pickings | Extremely thorny (1.0) - Hurts to approach |
| **Regrowth Rate** | Recovery from grazing | Slow recovery (0.0) - Takes ages to heal | Fast recovery (1.0) - Springs back quickly |

#### 🎯 Plant Defense in Action

**Scenario: The Toxic Arms Race**
> A plant species develops high **Toxin Production**. Herbivores that eat it get sick! But over generations, some herbivores evolve **Toxin Tolerance**—and now they can eat what others can't. The plant responds by producing even MORE toxin... and so the evolutionary arms race continues!

**Scenario: The Thorny Barrier**
> Plants with high **Thorn Density** damage creatures with low **Hide Thickness** when they try to feed. But thick-skinned herbivores can push through! Watch as both plants and creatures evolve in response.

**Scenario: The Grazing Specialist**
> Grass evolves extremely high **Regrowth Rate**—it WANTS to be eaten! The quick recovery means grass spreads everywhere, dominating open areas. Plus, the seeds pass through herbivore digestive systems for dispersal!

> ⚖️ **Trade-off Alert:** Defense costs energy! Plants with high toxins or thorns need more resources to maintain these defenses. In harsh environments, some plants evolve to be MORE edible to save energy!

---

### 🍎 Seed & Fruit Genes

How do plants spread their offspring? These **8 genes** determine seed dispersal strategy!

| Gene                      | What It Does                | Low Value                              | High Value                             |
| ------------------------- | --------------------------- | -------------------------------------- | -------------------------------------- |
| **Seed Mass**             | Weight of seeds             | Tiny (0.01) - Floats on wind           | Heavy (1.0) - Falls straight down      |
| **Seed Aerodynamics**     | Wings/fluff for wind travel | Round/heavy (0.0) - No wind travel     | Winged/fluffy (1.0) - Sails on breeze  |
| **Seed Hook Strength**    | Hooks for animal fur        | Smooth (0.0) - Doesn't attach          | Burr-like (1.0) - Sticks to everything |
| **Seed Coat Durability**  | Survival through digestion  | Fragile (0.0) - Destroyed by digestion | Tough (1.0) - Survives animal gut      |
| **Fruit Appeal**          | Attractiveness to animals   | Bland (0.0) - Ignored                  | Delicious (1.0) - Animals seek it out  |
| **Fruit Production Rate** | How often fruit is made     | Rarely (0.0) - Few fruits              | Constantly (1.0) - Fruit machine       |
| **Explosive Pod Force**   | Ballistic seed launch       | Weak (0.0) - Seeds dribble out         | Powerful (1.0) - Seeds fly 15+ tiles   |
| **Runner Production**     | Vegetative clonal spread    | No runners (0.0) - Seeds only          | Extensive (1.5) - Spreads like grass   |

#### 🌬️ Six Ways Seeds Travel

Seeds don't just fall—they have evolved amazing ways to travel!

| Strategy | How It Works | Key Genes |
|----------|--------------|-----------|
| 🪨 **Gravity** | Heavy seeds fall near parent | High Seed Mass |
| 💨 **Wind** | Light seeds blow in breeze | Low Mass + High Aerodynamics |
| 🍒 **Animal Fruit** | Eaten, seeds pass through gut | High Fruit Appeal + Durable Coat |
| 🦔 **Animal Burr** | Hooks onto passing animals | High Hook Strength |
| 💥 **Explosive** | Pods burst, launching seeds | High Explosive Pod Force |
| 🌿 **Vegetative** | Runners create clones | High Runner Production |

#### 🎯 Seed Dispersal in Action

**Scenario: The Wind Colonizer**
> A dandelion-like plant evolves tiny seeds with maximum aerodynamics. One gust of wind and seeds scatter 50 tiles away! Great for colonizing new areas, but risky—most seeds land in bad spots.

**Scenario: The Delicious Deal**
> A plant makes irresistible fruit with extremely durable seed coats. Animals eat the fruit, walk far away, then... deposit the seeds with a nice pile of fertilizer! This co-dependent relationship benefits both species.

**Scenario: The Hitchhiker**
> Burr plants develop hooks that attach to creature fur. The plant doesn't need to invest in fruit—just make sticky seeds and wait for someone to brush past!

---

### 🦷 Creature Interaction Genes

These genes determine how creatures interact with plants—and how plants can fight back!

| Gene | What It Does | Low Value | High Value |
|------|--------------|-----------|------------|
| **Plant Digestion** | Efficiency at eating plants | Can't digest plants (0.0) | Perfect herbivore (1.0) |
| **Meat Digestion** | Efficiency at eating meat | Can't digest meat (0.0) | Perfect carnivore (1.0) |
| **Cellulose Breakdown** | Ability to digest fiber | Can't process fiber (0.0) | Ruminant stomach (1.0) |
| **Toxin Tolerance** | Resistance to plant toxins | Easily poisoned (0.0) | Immune to poison (1.0) |
| **Toxin Metabolism** | Detoxification speed | Slow processing (0.0) | Rapid detox (1.0) |
| **Hide Thickness** | Protection from thorns | Paper-thin skin (0.0) | Rhino hide (1.0) |
| **Seed Destruction Rate** | Grinding up seeds | Seeds pass through (0.0) | Seeds destroyed (1.0) |
| **Caching Instinct** | Tendency to bury food | Eats everything now (0.0) | Squirrel behavior (1.0) |
| **Grooming Frequency** | Removing burrs from fur | Never grooms (0.0) | Constantly grooms (1.0) |
| **Fur Density** | How easily burrs attach | Hairless (0.0) | Shaggy (1.0) |
| **Gut Transit Time** | Digestion speed | Very fast (0.5h) | Very slow (24h) |
| **Scent Detection** | Finding food by smell | No sense of smell (0.0) | Bloodhound (1.0) |
| **Color Vision** | Seeing colorful fruit | Colorblind (0.0) | Full color (1.0) |

#### 🎯 Creature Interactions in Action

**Scenario: The Toxin Arms Race**
> Plants evolve **Toxin Production**. Herbivores respond with **Toxin Tolerance**. Plants increase toxins further. Eventually you might see:
> - Herbivores that can ONLY eat toxic plants (specialists)
> - Herbivores that avoid toxic plants entirely
> - Some plants that give up on toxins to save energy

**Scenario: The Seed Disperser**
> A creature with low **Seed Destruction Rate** and optimal **Gut Transit Time** becomes the perfect seed disperser! Plants evolve MORE attractive fruit to recruit these helpful animals. Over time, specific plant-animal partnerships form.

**Scenario: The Perfect Herbivore**
> High **Cellulose Breakdown**, high **Toxin Tolerance**, and high **Hide Thickness** creates an unstoppable grazing machine. But this comes at a cost—all those specialized systems need energy to maintain!

### 🎯 Foraging Senses in Action

**Scenario: The Scent Tracker**
> A creature evolves high **Scent Detection** but low **Color Vision**. It can find fragrant berry bushes even through dense forest—but completely ignores grass patches because they have no scent!

**Scenario: The Visual Hunter**
> A creature with high **Color Vision** but low **Scent Detection** spots bright berry bushes from far away in open terrain. But in dense forests, it misses nearby food it can't see.

**Scenario: The Generalist**
> Some creatures evolve moderate levels of both senses. They're not specialists, but they can find food in more situations. This flexibility comes at an energy cost!

**Archetype Emergence:**

Over generations, you may see distinct foraging archetypes emerge:
- 🍎 **Frugivores** - High Color Vision, find colorful fruit easily
- 👃 **Scent Trackers** - High Scent Detection, follow their nose to food

---

## 🔄 Coevolution: The Big Picture

One of the most exciting features in EcoSim 2.0 is **coevolution**—when species evolve in response to each other!

### How It Works

```
🌿 Plant evolves toxins ──────────────────────────────────►
                         │
                         │ Selection pressure
                         ▼
            🐛 Herbivore evolves toxin tolerance
                         │
                         │ Selection pressure
                         ▼
🌿 Plant evolves MORE toxins ◄────────────────────────────
                         │
                         │ And the cycle continues...
                         ▼
```

### Arms Races You'll See

| Plant Gene | vs | Creature Gene | What Happens |
|------------|:--:|---------------|--------------|
| Toxin Production | ⚔️ | Toxin Tolerance | Poison escalation |
| Thorn Density | ⚔️ | Hide Thickness | Physical armor race |
| Seed Coat Durability | ⚔️ | Seed Destruction Rate | Seed survival battle |
| Fruit Appeal | 🤝 | Color Vision | Mutualistic partnership |

### Watching Coevolution Unfold

The simulation tracks these gene relationships over generations. You might see:

- **Escalating** — Both genes increasing together (arms race!)
- **Stable** — Genes reached an equilibrium
- **Oscillating** — Genes go back and forth as each side adapts
- **Declining** — One side "gave up" and evolved a different strategy

> 🔬 **Pro Tip:** Pay attention to specialist species! Creatures that focus on ONE food type can co-evolve much faster than generalists.

---

## The Unified Gene Pool

Here's the exciting part: **creatures and plants share ALL 88 genes**. They're just expressed differently!

### How Expression Works

```
┌───────────────────────────────────────────────────────────┐
│                      CREATURE                              │
├───────────────────────────────────────────────────────────┤
│ 🌍 Universal        │ ████████████  ACTIVE                │
│ 🏃 Mobility         │ ████████████  ACTIVE                │
│ 🌱 Autotrophy       │ ░░░░░░░░░░░░  dormant               │
│ 🍖 Heterotrophy     │ ████████████  ACTIVE                │
│ 🥚 Reproduction     │ ████████████  ACTIVE                │
│ 🛡️ Plant Defense   │ ░░░░░░░░░░░░  dormant               │
│ 🦷 Interactions     │ ████████████  ACTIVE                │
└───────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────┐
│                       PLANT                                │
├───────────────────────────────────────────────────────────┤
│ 🌍 Universal        │ ████████████  ACTIVE                │
│ 🏃 Mobility         │ ░░░░░░░░░░░░  dormant               │
│ 🌱 Autotrophy       │ ████████████  ACTIVE                │
│ 🍖 Heterotrophy     │ ░░░░░░░░░░░░  dormant               │
│ 🥚 Reproduction     │ ████████████  ACTIVE                │
│ 🛡️ Plant Defense   │ ████████████  ACTIVE                │
│ 🍎 Seed/Fruit       │ ████████████  ACTIVE                │
└───────────────────────────────────────────────────────────┘
```

### The Magic of Dormant Genes

Dormant genes aren't gone—they're just sleeping! Through mutation and evolution:

- 🌱 **Plants can activate mobility genes** → Moving plants!
- 🦎 **Creatures can activate photosynthesis** → Self-feeding animals!
- 🌿 **Plants can activate hunting genes** → Carnivorous plants!
- 🛡️ **Creatures can activate defense genes** → Armored herbivores!

This creates the potential for truly unique organisms that blur the line between plant and animal!

---

## Quick Reference Card

### Gene Count by Category

| Category | Gene Count | Description |
|----------|:----------:|-------------|
| 🌍 Universal | 7 | Basic life traits for all organisms |
| 🏃 Mobility | 5 | Movement and awareness |
| 🌱 Autotrophy | 5 | Self-feeding (photosynthesis) |
| 🍖 Heterotrophy (Original) | 6 | Hunting and digestion basics |
| 🍖 Heterotrophy (Coevolution) | 7 | Specialized digestion genes |
| 🦴 Morphology | 9 | Physical anatomy traits |
| 🧠 Behavior | 5 | Behavioral patterns |
| 🌱 Seed Interaction | 2 | How seeds pass through digestive systems |
| 🛡️ Plant Defense | 6 | Toxins, thorns, regrowth, fruit |
| 🥚 Reproduction | 6 | Breeding and offspring |
| 🌬️ Seed Propagation | 5 | Physical seed dispersal mechanisms |
| 👃 Olfactory | 4 | Scent-based communication |
| **TOTAL** | **67** | (Including all Phase 2 genes) |

### Key New Genes Quick Reference

| Gene | Category | Range | What It Does |
|------|----------|-------|--------------|
| Plant Digestion | 🦷 | 0 - 1.0 | Herbivore efficiency |
| Meat Digestion | 🦷 | 0 - 1.0 | Carnivore efficiency |
| Toxin Tolerance | 🦷 | 0 - 1.0 | Resist plant poisons |
| Hide Thickness | 🦷 | 0 - 1.0 | Resist thorns |
| Toxin Production | 🛡️ | 0 - 1.0 | Poison defense |
| Thorn Density | 🛡️ | 0 - 1.0 | Physical defense |
| Regrowth Rate | 🛡️ | 0 - 1.0 | Recovery speed |
| Seed Mass | 🍎 | 0.01 - 1.0 | Wind vs gravity |
| Fruit Appeal | 🍎 | 0 - 1.0 | Attract dispersers |
| Seed Coat Durability | 🍎 | 0 - 1.0 | Survive digestion |

### Original Core Genes

| Category | Gene | Range | Quick Description |
|----------|------|-------|-------------------|
| 🌍 | Lifespan | 100 - 1M | How long it lives |
| 🌍 | Max Size | 0.5 - 20 | How big it grows |
| 🌍 | Metabolism | 0.1 - 3.0 | Energy burn rate |
| 🌍 | Color Hue | 0 - 360° | Visual color |
| 🌍 | Hardiness | 0 - 1.0 | Damage resistance |
| 🌍 | Cold Tolerance | -30 - 30°C | Minimum survival temp |
| 🌍 | Heat Tolerance | 10 - 60°C | Maximum survival temp |
| 🏃 | Locomotion | 0 - 2.0 | Movement speed |
| 🏃 | Sight Range | 0 - 200 | Vision distance |
| 🏃 | Navigation | 0 - 1.0 | Pathfinding skill |
| 🏃 | Flee Distance | 0 - 30 | Danger response |
| 🏃 | Pursue Distance | 0 - 30 | Hunting range |
| 🌱 | Photosynthesis | 0 - 1.0 | Solar energy |
| 🌱 | Root Depth | 0 - 10 | Underground access |
| 🌱 | Water Storage | 0 - 1.0 | Drought resistance |
| 🌱 | Light Requirement | 0 - 1.0 | Sun needs |
| 🌱 | Water Requirement | 0 - 1.0 | Hydration needs |
| 🍖 | Hunt Instinct | 0 - 1.0 | Predatory drive |
| 🍖 | Hunger Threshold | 0 - 10 | When to eat |
| 🍖 | Thirst Threshold | 0 - 10 | When to drink |
| 🍖 | Digestive Efficiency | 0.1 - 1.0 | Nutrient absorption |
| 🍖 | Nutrient Value | 1 - 100 | Food value |
| 🥚 | Offspring Count | 1 - 20 | Babies per breeding |
| 🥚 | Mate Threshold | 0 - 10 | Breeding drive |
| 🥚 | Spread Distance | 0 - 30 | Offspring dispersal |
| 🥚 | Fatigue Threshold | 0 - 10 | Energy management |
| 🥚 | Comfort Increase | 0.001 - 0.02 | Recovery rate |
| 🥚 | Comfort Decrease | 0.001 - 0.02 | Stress rate |

---

## 💡 Energy Budget Tip

In Version 2.0, every gene has a **maintenance cost**. The more complex your organism, the more energy it needs just to survive!

```
Simple Organism:    Complex Organism:
═══════════════     ═══════════════════
Few genes active    Many genes active
Low maintenance     High maintenance
Survives on less    Needs lots of food
```

This creates natural **specialization pressure**. Organisms can't do everything well—they must choose their niche!

- 🌿 **Specialists** focus on one food type → lower maintenance, higher efficiency
- 🍽️ **Generalists** can eat anything → higher maintenance, more flexibility

Watch for this trade-off in your ecosystem!

---

## See Also

- 📖 [How Inheritance Works](inheritance.md) - Learn how genes pass to offspring
- 📖 [Evolution in Action](evolution.md) - Watch species change over time
- 📖 [Genetics Overview](README.md) - Introduction to the genetics system
