# Logger-Statistics Integration

## Current State
- Logger: Event stream (detailed, per-entity events)
- Statistics: Aggregates (population counts, averages)
- Currently independent systems

## Integration Options

### Option A: Observer Pattern (Logger → Statistics)
Logger notifies Statistics when events occur.

**Advantages:**
- Real-time aggregate updates
- Single source of truth for events
- Statistics always synchronized with Logger

**Disadvantages:**
- Tighter coupling between systems
- Statistics must process every event (performance overhead)
- Changes to Logger affect Statistics

### Option B: Statistics Reads Log Files
Statistics can parse log files for historical analysis.

**Advantages:**
- Systems remain decoupled
- Can analyze historical data
- No runtime overhead

**Disadvantages:**
- Delayed statistics (post-processing)
- Log file parsing adds complexity
- Duplicate data storage

### Option C: Shared Event Bus
Both systems subscribe to a central event dispatcher.

**Advantages:**
- Fully decoupled
- Other systems can subscribe too
- Clean architecture

**Disadvantages:**
- Additional complexity
- Event bus becomes single point of failure

## Recommendation
Start with independent systems. Add integration via Option A (Observer) if real-time correlation needed.
