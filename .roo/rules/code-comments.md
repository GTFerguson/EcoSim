# Code Comments Rules

Code comments should be meaningful, self-contained, and future-proof. Anyone reading the code years from now—without access to internal planning documents, issue trackers, or project history—should understand why the code exists.

## Core Principle: WHY, Not WHAT

Comments should explain **why** code exists or **why** a non-obvious approach was taken. The code itself shows **what** it does.

**❌ Bad - Describes what the code does:**
```cpp
// Increment counter
counter++;

// Loop through all creatures
for (auto& creature : creatures) {
```

**✅ Good - Explains why:**
```cpp
// Prevent division by zero when population is extinct
if (population == 0) return defaultValue;

// Process oldest creatures first to ensure fair resource distribution
std::sort(creatures.begin(), creatures.end(), byAge);
```

## Avoid Marker Comments

Do not use comments that reference internal tracking systems, planning documents, or development phases. These become meaningless once the referenced context is gone.

**❌ Bad - Internal references:**
```cpp
// Phase 2 implementation
// C-005 fix
// Issue #123
// per phenotype-modulation-refactor.md
// Step 3 of migration
// fix: energy calculation
// DRY refactoring:
```

**✅ Good - Self-contained explanation:**
```cpp
// Energy calculation uses logarithmic scaling to prevent runaway growth
// at high metabolism values while maintaining linear behavior at low values

// Fallback to default genome when parent data is unavailable (e.g., spawned creatures)
```

## @todo Comments

`@todo` is acceptable when it describes genuine future work that the code needs. The comment should be self-explanatory.

**❌ Bad - Vague or tracking-style:**
```cpp
// @todo Phase 3
// @todo fix later
// @todo see issue #456
```

**✅ Good - Clear and actionable:**
```cpp
// @todo Add ocean biome support when water tiles are implemented
// @todo Consider caching this calculation if profiling shows it's a bottleneck
// @todo Handle edge case where both parents have identical genomes
```

## When to Comment

Add comments when:
- The code uses a non-obvious algorithm or approach
- There's important context that isn't clear from the code
- A workaround exists for a subtle bug or limitation
- Performance or correctness depends on something non-obvious

**✅ Good examples:**
```cpp
// Using weak_ptr to break circular reference between parent and offspring
std::weak_ptr<Creature> parent_;

// Must check bounds before accessing neighbors array - grid edges have fewer neighbors
if (isValidPosition(x, y)) {

// Intentionally using integer division to snap to tile boundaries
int tileX = worldX / TILE_SIZE;
```

## When NOT to Comment

Don't add comments when:
- The code is self-explanatory
- You're just restating what the code does
- You're marking sections for yourself during development

**❌ Unnecessary comments:**
```cpp
// Constructor
Creature::Creature() { }

// Getter for health
int getHealth() const { return health_; }

// Check if dead
if (health <= 0) {
```

## Summary

| Do | Don't |
|----|-------|
| Explain WHY | Describe WHAT |
| Write for future readers | Reference internal docs/issues |
| Use @todo for genuine future work | Use @todo as a tracking system |
| Comment non-obvious decisions | Comment obvious code |
| Make comments self-contained | Assume reader has project context |
