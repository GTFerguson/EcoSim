/**
 *  Title   : Timing System Test
 *  Author  : Gary Ferguson
 *  Date    : December 2024
 *  Purpose : Test GameClock timing functionality and Calendar integration
 *
 *  Test Isolation:
 *  - This test does NOT access file system
 *  - All test data is created in-memory
 *  - No external dependencies
 *  - Each test uses fresh instances
 *
 *  Key Findings from Code Analysis:
 *  - GameClock uses accumulator pattern for fixed timestep
 *  - DEFAULT_TICK_DURATION_MS = 50.0 (20 ticks/second)
 *  - MAX_FRAME_TIME_MS = 250.0 (spiral of death prevention)
 *  - Calendar increments 1 minute per simulation tick
 *  - 1 tick = 1 in-game minute
 *  - 20 ticks/second = 20 in-game minutes per real second
 */

#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cmath>
#include "../../include/timing.hpp"
#include "../../include/calendar.hpp"

using namespace std;
using namespace EcoSim::Timing;

// Test assertion macro with descriptive error messages
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << message << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

// Approximate equality for floating point
#define TEST_ASSERT_APPROX(a, b, tolerance, message) \
    if (std::abs((a) - (b)) > (tolerance)) { \
        std::cerr << "ASSERTION FAILED: " << message << std::endl; \
        std::cerr << "  Expected: " << (b) << ", Got: " << (a) << std::endl; \
        std::cerr << "  Tolerance: " << (tolerance) << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

//==============================================================================
// GameClock Fixed Timestep Tests
//==============================================================================

/**
 * Test: GameClock initializes with correct default settings
 */
void testGameClockInitialization() {
    cout << endl << "TEST: GameClock Initialization" << endl;
    
    // Test default constructor
    GameClock clock;
    
    TEST_ASSERT_APPROX(clock.getTickDuration(), DEFAULT_TICK_DURATION_MS, 0.001,
        "Default tick duration should be DEFAULT_TICK_DURATION_MS (50.0)");
    TEST_ASSERT_APPROX(clock.getAccumulator(), 0.0, 0.001,
        "Accumulator should start at 0");
    TEST_ASSERT(clock.getTickCount() == 0,
        "Tick count should start at 0");
    TEST_ASSERT(clock.getFrameCount() == 0,
        "Frame count should start at 0");
    TEST_ASSERT_APPROX(clock.getTotalTime(), 0.0, 0.001,
        "Total time should start at 0");
    TEST_ASSERT_APPROX(clock.getTicksPerSecond(), 20.0, 0.001,
        "Default should be 20 ticks per second");
    
    // Test custom constructor
    GameClock customClock(100.0);  // 10 ticks per second
    TEST_ASSERT_APPROX(customClock.getTickDuration(), 100.0, 0.001,
        "Custom tick duration should be set correctly");
    TEST_ASSERT_APPROX(customClock.getTicksPerSecond(), 10.0, 0.001,
        "Custom ticks per second should be 10");
    
    cout << "  Default tick duration: " << clock.getTickDuration() << "ms" << endl;
    cout << "  Default ticks per second: " << clock.getTicksPerSecond() << endl;
    cout << "  PASSED" << endl;
}

/**
 * Test: Accumulator properly tracks elapsed time
 */
void testGameClockAccumulator() {
    cout << endl << "TEST: GameClock Accumulator" << endl;
    
    GameClock clock(50.0);  // 50ms per tick
    clock.start();
    
    // Initial state
    TEST_ASSERT_APPROX(clock.getAccumulator(), 0.0, 0.001,
        "Accumulator should be 0 after start()");
    
    // Simulate a short delay and tick
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    clock.tick();
    
    double accumulator = clock.getAccumulator();
    cout << "  Accumulator after 60ms sleep: " << accumulator << "ms" << endl;
    
    // Should have accumulated approximately 60ms (with some tolerance for timing)
    TEST_ASSERT(accumulator >= 50.0, "Accumulator should have >= 50ms after 60ms sleep");
    TEST_ASSERT(accumulator <= 150.0, "Accumulator should be reasonable (< 150ms)");
    
    cout << "  Frame time: " << clock.getFrameTime() << "ms" << endl;
    cout << "  Frame count: " << clock.getFrameCount() << endl;
    TEST_ASSERT(clock.getFrameCount() == 1, "Frame count should be 1 after one tick()");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: shouldUpdate() returns true only when enough time has accumulated
 */
void testGameClockShouldUpdate() {
    cout << endl << "TEST: GameClock shouldUpdate()" << endl;
    
    GameClock clock(50.0);  // 50ms per tick
    clock.start();
    
    // Initially should not need update
    TEST_ASSERT(!clock.shouldUpdate(), 
        "shouldUpdate() should be false initially");
    
    // Sleep less than one tick
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    clock.tick();
    cout << "  After 30ms: accumulator = " << clock.getAccumulator() << "ms" << endl;
    
    // May or may not need update depending on timing
    bool needsUpdateAfter30 = clock.shouldUpdate();
    cout << "  shouldUpdate after ~30ms: " << (needsUpdateAfter30 ? "true" : "false") << endl;
    
    // Sleep enough to definitely need update
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    clock.tick();
    cout << "  After additional 60ms: accumulator = " << clock.getAccumulator() << "ms" << endl;
    
    TEST_ASSERT(clock.shouldUpdate(), 
        "shouldUpdate() should be true after sufficient time");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: consumeTick() properly subtracts tick duration from accumulator
 */
void testGameClockConsumeTick() {
    cout << endl << "TEST: GameClock consumeTick()" << endl;
    
    GameClock clock(50.0);  // 50ms per tick
    clock.start();
    
    // Accumulate time
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    clock.tick();
    
    double beforeConsume = clock.getAccumulator();
    uint64_t ticksBefore = clock.getTickCount();
    cout << "  Before consume: accumulator = " << beforeConsume << "ms, ticks = " << ticksBefore << endl;
    
    // Consume one tick
    clock.consumeTick();
    
    double afterConsume = clock.getAccumulator();
    uint64_t ticksAfter = clock.getTickCount();
    cout << "  After consume: accumulator = " << afterConsume << "ms, ticks = " << ticksAfter << endl;
    
    TEST_ASSERT_APPROX(afterConsume, beforeConsume - 50.0, 1.0,
        "Accumulator should decrease by tick duration");
    TEST_ASSERT(ticksAfter == ticksBefore + 1,
        "Tick count should increase by 1");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: Multiple ticks fire when frame takes a long time
 */
void testGameClockMultipleTicks() {
    cout << endl << "TEST: GameClock Multiple Ticks" << endl;
    
    GameClock clock(50.0);  // 50ms per tick
    clock.start();
    
    // Simulate a long frame (150ms = 3 ticks worth)
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    clock.tick();
    
    cout << "  After 150ms: accumulator = " << clock.getAccumulator() << "ms" << endl;
    
    // Count how many ticks we can consume
    int ticksConsumed = 0;
    while (clock.shouldUpdate()) {
        clock.consumeTick();
        ticksConsumed++;
    }
    
    cout << "  Ticks consumed: " << ticksConsumed << endl;
    cout << "  Remaining accumulator: " << clock.getAccumulator() << "ms" << endl;
    
    // Should have consumed 2-3 ticks (150ms / 50ms = 3, but timing variance)
    TEST_ASSERT(ticksConsumed >= 2, "Should consume at least 2 ticks for 150ms frame");
    TEST_ASSERT(ticksConsumed <= 5, "Should not consume more than 5 ticks for 150ms frame");
    TEST_ASSERT(clock.getAccumulator() >= 0.0, "Accumulator should not be negative");
    TEST_ASSERT(clock.getAccumulator() < 50.0, "Accumulator should be less than one tick");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: Spiral of death prevention - max frame time caps accumulated time
 */
void testSpiralOfDeathPrevention() {
    cout << endl << "TEST: Spiral of Death Prevention" << endl;
    
    GameClock clock(50.0);  // 50ms per tick
    clock.start();
    
    // Simulate a very long frame (500ms) - beyond MAX_FRAME_TIME_MS (250ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    clock.tick();
    
    double frameTime = clock.getFrameTime();
    double accumulator = clock.getAccumulator();
    
    cout << "  Actual sleep: ~500ms" << endl;
    cout << "  Reported frame time: " << frameTime << "ms" << endl;
    cout << "  Accumulator: " << accumulator << "ms" << endl;
    cout << "  MAX_FRAME_TIME_MS: " << MAX_FRAME_TIME_MS << "ms" << endl;
    
    // Frame time should be capped at MAX_FRAME_TIME_MS
    TEST_ASSERT(frameTime <= MAX_FRAME_TIME_MS + 1.0,
        "Frame time should be capped at MAX_FRAME_TIME_MS");
    
    // Count ticks that would fire
    int ticksToConsume = 0;
    GameClock testClock = clock;  // Copy to not affect original
    while (testClock.shouldUpdate()) {
        testClock.consumeTick();
        ticksToConsume++;
    }
    
    cout << "  Ticks that would fire: " << ticksToConsume << endl;
    
    // At 250ms cap / 50ms per tick = max 5 ticks
    TEST_ASSERT(ticksToConsume <= 6, 
        "Max ticks should be limited by MAX_FRAME_TIME_MS (expected <= 6)");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: Interpolation factor for smooth rendering
 */
void testGameClockInterpolation() {
    cout << endl << "TEST: GameClock Interpolation" << endl;
    
    GameClock clock(100.0);  // 100ms per tick for easier testing
    clock.start();
    
    // Initial interpolation should be 0
    TEST_ASSERT_APPROX(clock.getInterpolation(), 0.0, 0.01,
        "Initial interpolation should be ~0");
    
    // Accumulate about half a tick
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    clock.tick();
    
    double interp = clock.getInterpolation();
    cout << "  After ~50ms (half tick): interpolation = " << interp << endl;
    
    // Should be approximately 0.5 (with timing tolerance)
    TEST_ASSERT(interp >= 0.3 && interp <= 0.8,
        "Interpolation should be around 0.5 for half a tick");
    
    // Accumulate to full tick and consume
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    clock.tick();
    
    while (clock.shouldUpdate()) {
        clock.consumeTick();
    }
    
    interp = clock.getInterpolation();
    cout << "  After consuming ticks: interpolation = " << interp << endl;
    TEST_ASSERT(interp >= 0.0 && interp < 1.0,
        "Interpolation should be between 0 and 1");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: SimulationSpeed constants are correct
 */
void testSimulationSpeedConstants() {
    cout << endl << "TEST: SimulationSpeed Constants" << endl;
    
    // Verify speed presets
    TEST_ASSERT_APPROX(SimulationSpeed::PAUSED, 0.0, 0.001, "PAUSED should be 0");
    TEST_ASSERT_APPROX(SimulationSpeed::VERY_SLOW, 200.0, 0.001, "VERY_SLOW should be 200ms");
    TEST_ASSERT_APPROX(SimulationSpeed::SLOW, 100.0, 0.001, "SLOW should be 100ms");
    TEST_ASSERT_APPROX(SimulationSpeed::NORMAL, 50.0, 0.001, "NORMAL should be 50ms");
    TEST_ASSERT_APPROX(SimulationSpeed::FAST, 25.0, 0.001, "FAST should be 25ms");
    TEST_ASSERT_APPROX(SimulationSpeed::VERY_FAST, 10.0, 0.001, "VERY_FAST should be 10ms");
    
    // Verify conversion functions
    TEST_ASSERT_APPROX(SimulationSpeed::ticksToMs(20.0), 50.0, 0.001, 
        "20 ticks/s = 50ms/tick");
    TEST_ASSERT_APPROX(SimulationSpeed::msToTicks(50.0), 20.0, 0.001,
        "50ms/tick = 20 ticks/s");
    
    cout << "  PAUSED: " << SimulationSpeed::PAUSED << "ms (0 ticks/s)" << endl;
    cout << "  VERY_SLOW: " << SimulationSpeed::VERY_SLOW << "ms (5 ticks/s)" << endl;
    cout << "  SLOW: " << SimulationSpeed::SLOW << "ms (10 ticks/s)" << endl;
    cout << "  NORMAL: " << SimulationSpeed::NORMAL << "ms (20 ticks/s)" << endl;
    cout << "  FAST: " << SimulationSpeed::FAST << "ms (40 ticks/s)" << endl;
    cout << "  VERY_FAST: " << SimulationSpeed::VERY_FAST << "ms (100 ticks/s)" << endl;
    
    cout << "  PASSED" << endl;
}

//==============================================================================
// Calendar Integration Tests
//==============================================================================

/**
 * Test: Calendar advances once per simulation tick
 * 
 * In main.cpp, after each tick: calendar++;
 * This means 1 tick = 1 in-game minute
 */
void testCalendarAdvancesPerTick() {
    cout << endl << "TEST: Calendar Advances Per Tick" << endl;
    
    Calendar calendar;
    
    unsigned short initialMinute = calendar.getMinute();
    unsigned short initialHour = calendar.getHour();
    
    cout << "  Initial time: " << calendar.shortTime() << endl;
    
    // Simulate 5 ticks (5 minutes)
    for (int i = 0; i < 5; i++) {
        calendar++;  // Same as incrementByMinute()
    }
    
    cout << "  After 5 ticks: " << calendar.shortTime() << endl;
    
    // Calculate expected minute
    unsigned short expectedMinute = (initialMinute + 5) % 60;
    TEST_ASSERT(calendar.getMinute() == expectedMinute,
        "Calendar should advance 5 minutes after 5 ticks");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: Calendar time consistency with tick count
 * 
 * At default speed (50ms/tick = 20 ticks/second):
 * - 1 real second = 20 in-game minutes
 * - 1 real minute = 1200 in-game minutes = 20 in-game hours
 * - 1 in-game day = 1440 minutes = 1440 ticks = 72 real seconds
 */
void testCalendarTimeConsistency() {
    cout << endl << "TEST: Calendar Time Consistency" << endl;
    
    Calendar calendar;
    
    // Test minute -> hour rollover (60 ticks)
    cout << "  Testing minute -> hour rollover..." << endl;
    Calendar hourTest;
    for (int i = 0; i < 60; i++) {
        hourTest++;
    }
    TEST_ASSERT(hourTest.getHour() == 1, 
        "60 ticks should advance 1 hour");
    TEST_ASSERT(hourTest.getMinute() == 0,
        "Minute should reset to 0 after hour rollover");
    cout << "  After 60 ticks: " << hourTest.shortTime() << " - OK" << endl;
    
    // Test hour -> day rollover (24 * 60 = 1440 ticks)
    cout << "  Testing hour -> day rollover..." << endl;
    Calendar dayTest;
    unsigned short initialDay = dayTest.getDay();
    for (int i = 0; i < 1440; i++) {
        dayTest++;
    }
    TEST_ASSERT(dayTest.getDay() == initialDay + 1,
        "1440 ticks should advance 1 day");
    TEST_ASSERT(dayTest.getHour() == 0,
        "Hour should reset to 0 after day rollover");
    cout << "  After 1440 ticks: " << dayTest.longDate() << " " << dayTest.shortTime() << " - OK" << endl;
    
    // Verify tick count matches expected calendar time
    GameClock clock(50.0);
    Calendar syncTest;
    int ticksSimulated = 0;
    
    // Simulate 120 ticks
    for (int i = 0; i < 120; i++) {
        syncTest++;
        ticksSimulated++;
    }
    
    // Should be 2 hours (120 minutes)
    TEST_ASSERT(syncTest.getHour() == 2,
        "120 ticks should equal 2 hours");
    cout << "  After 120 ticks: " << syncTest.shortTime() << " (expected 02:00) - OK" << endl;
    
    cout << "  PASSED" << endl;
}

/**
 * Test: GameClock and Calendar work together correctly
 * 
 * This simulates the actual game loop pattern from main.cpp
 */
void testCalendarIntegrationWithGameClock() {
    cout << endl << "TEST: Calendar Integration With GameClock" << endl;
    
    GameClock clock(50.0);  // 50ms per tick (20 ticks/second)
    Calendar calendar;
    
    clock.start();
    
    cout << "  Simulating game loop..." << endl;
    cout << "  Initial: " << calendar.shortTime() << ", tick count: " << clock.getTickCount() << endl;
    
    // Simulate several frames of the game loop
    int simulatedMinutes = 0;
    
    for (int frame = 0; frame < 10; frame++) {
        // Simulate frame time (variable)
        std::this_thread::sleep_for(std::chrono::milliseconds(20 + (frame % 30)));
        clock.tick();
        
        // Process simulation ticks (fixed timestep)
        while (clock.shouldUpdate()) {
            calendar++;
            simulatedMinutes++;
            clock.consumeTick();
        }
    }
    
    cout << "  After simulation: " << calendar.shortTime() << endl;
    cout << "  Simulated minutes: " << simulatedMinutes << endl;
    cout << "  GameClock tick count: " << clock.getTickCount() << endl;
    
    // Tick count should match simulated minutes
    TEST_ASSERT(clock.getTickCount() == static_cast<uint64_t>(simulatedMinutes),
        "GameClock tick count should match calendar minutes advanced");
    
    // Calendar minute should match (accounting for rollover)
    unsigned short expectedMinute = simulatedMinutes % 60;
    TEST_ASSERT(calendar.getMinute() == expectedMinute,
        "Calendar minute should match ticks mod 60");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: Verify the timing relationship documented in the system
 * 
 * From timing.hpp and main.cpp:
 * - SIMULATION_TICK_MS = 50.0 (20 ticks/second)
 * - 1 tick = 1 in-game minute
 * - 1 real second = 20 in-game minutes
 */
void testTimingRelationshipDocumentation() {
    cout << endl << "TEST: Timing Relationship Documentation" << endl;
    
    // Verify constants match documentation
    TEST_ASSERT_APPROX(DEFAULT_TICK_DURATION_MS, 50.0, 0.001,
        "DEFAULT_TICK_DURATION_MS should be 50.0");
    
    double ticksPerSecond = 1000.0 / DEFAULT_TICK_DURATION_MS;
    cout << "  Ticks per second: " << ticksPerSecond << endl;
    cout << "  In-game minutes per real second: " << ticksPerSecond << endl;
    cout << "  In-game hours per real minute: " << (ticksPerSecond * 60.0 / 60.0) << endl;
    cout << "  In-game days per real hour: " << (ticksPerSecond * 3600.0 / 1440.0) << endl;
    
    // At 20 ticks/sec:
    // - 1 real second = 20 in-game minutes
    // - 1 real minute = 1200 in-game minutes = 20 in-game hours
    // - 1 real hour = 72000 in-game minutes = 1200 in-game hours = 50 in-game days
    
    double inGameMinutesPerRealSecond = ticksPerSecond;
    double inGameHoursPerRealMinute = inGameMinutesPerRealSecond * 60.0 / 60.0;
    double inGameDaysPerRealHour = inGameMinutesPerRealSecond * 3600.0 / 1440.0;
    
    TEST_ASSERT_APPROX(inGameMinutesPerRealSecond, 20.0, 0.001,
        "Should be 20 in-game minutes per real second");
    TEST_ASSERT_APPROX(inGameHoursPerRealMinute, 20.0, 0.001,
        "Should be 20 in-game hours per real minute");
    TEST_ASSERT_APPROX(inGameDaysPerRealHour, 50.0, 0.001,
        "Should be 50 in-game days per real hour");
    
    cout << "  Verified: 1 tick = 1 in-game minute" << endl;
    cout << "  PASSED" << endl;
}

/**
 * Test: Calendar boundary conditions with GameClock
 */
void testCalendarBoundaryWithGameClock() {
    cout << endl << "TEST: Calendar Boundary With GameClock" << endl;
    
    Calendar calendar;
    GameClock clock(1.0);  // Fast clock for testing (1ms per tick)
    
    clock.start();
    
    // Run enough ticks to cross day boundary
    int ticksToRunOneDay = 1440;  // 24 * 60 minutes
    
    unsigned short initialDay = calendar.getDay();
    unsigned short initialMonth = calendar.getMonth();
    
    cout << "  Initial: " << calendar.longDate() << " " << calendar.shortTime() << endl;
    
    for (int i = 0; i < ticksToRunOneDay; i++) {
        calendar++;
    }
    
    cout << "  After " << ticksToRunOneDay << " ticks: " << calendar.longDate() << " " << calendar.shortTime() << endl;
    
    TEST_ASSERT(calendar.getDay() == initialDay + 1 || 
                (calendar.getDay() == 1 && calendar.getMonth() != initialMonth),
        "Day should advance or month should roll over");
    
    cout << "  PASSED" << endl;
}

//==============================================================================
// Additional Edge Case Tests
//==============================================================================

/**
 * Test: GameClock start() resets all state
 */
void testGameClockStartReset() {
    cout << endl << "TEST: GameClock Start Reset" << endl;
    
    GameClock clock(50.0);
    clock.start();
    
    // Accumulate some state
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    clock.tick();
    while (clock.shouldUpdate()) {
        clock.consumeTick();
    }
    
    uint64_t ticksBefore = clock.getTickCount();
    double totalTimeBefore = clock.getTotalTime();
    cout << "  Before reset: ticks=" << ticksBefore << ", totalTime=" << totalTimeBefore << endl;
    
    // Reset
    clock.start();
    
    cout << "  After start(): ticks=" << clock.getTickCount() << ", totalTime=" << clock.getTotalTime() << endl;
    
    TEST_ASSERT(clock.getTickCount() == 0, "Tick count should reset to 0");
    TEST_ASSERT(clock.getFrameCount() == 0, "Frame count should reset to 0");
    TEST_ASSERT_APPROX(clock.getTotalTime(), 0.0, 0.001, "Total time should reset to 0");
    TEST_ASSERT_APPROX(clock.getAccumulator(), 0.0, 0.001, "Accumulator should reset to 0");
    
    cout << "  PASSED" << endl;
}

/**
 * Test: Setting tick duration dynamically
 */
void testGameClockDynamicTickDuration() {
    cout << endl << "TEST: GameClock Dynamic Tick Duration" << endl;
    
    GameClock clock(50.0);  // Start at normal speed
    clock.start();
    
    TEST_ASSERT_APPROX(clock.getTickDuration(), 50.0, 0.001, "Initial tick duration should be 50ms");
    
    // Change to fast speed
    clock.setTickDuration(25.0);
    TEST_ASSERT_APPROX(clock.getTickDuration(), 25.0, 0.001, "Tick duration should be 25ms");
    TEST_ASSERT_APPROX(clock.getTicksPerSecond(), 40.0, 0.001, "Should be 40 ticks/second");
    
    // Change via ticks per second
    clock.setTicksPerSecond(100.0);
    TEST_ASSERT_APPROX(clock.getTickDuration(), 10.0, 0.001, "Tick duration should be 10ms");
    TEST_ASSERT_APPROX(clock.getTicksPerSecond(), 100.0, 0.001, "Should be 100 ticks/second");
    
    cout << "  Dynamic speed changes work correctly" << endl;
    cout << "  PASSED" << endl;
}

/**
 * Test: FPS calculation accuracy
 */
void testGameClockFPSCalculation() {
    cout << endl << "TEST: GameClock FPS Calculation" << endl;
    
    GameClock clock(50.0);
    clock.start();
    
    // Simulate several frames at approximately 60 FPS (~16.67ms per frame)
    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        clock.tick();
    }
    
    double currentFPS = clock.getCurrentFPS();
    double averageFPS = clock.getAverageFPS();
    
    cout << "  Current FPS: " << currentFPS << endl;
    cout << "  Average FPS: " << averageFPS << endl;
    
    // FPS should be in a reasonable range (accounting for timing variance)
    TEST_ASSERT(currentFPS > 20.0 && currentFPS < 200.0,
        "Current FPS should be reasonable");
    TEST_ASSERT(averageFPS > 20.0 && averageFPS < 200.0,
        "Average FPS should be reasonable");
    
    cout << "  PASSED" << endl;
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main() {
    cout << "=== Timing System Test Suite ===" << endl;
    cout << "Testing GameClock fixed timestep and Calendar integration" << endl;
    
    // GameClock Fixed Timestep Tests
    cout << endl << "--- GameClock Fixed Timestep Tests ---" << endl;
    testGameClockInitialization();
    testGameClockAccumulator();
    testGameClockShouldUpdate();
    testGameClockConsumeTick();
    testGameClockMultipleTicks();
    testSpiralOfDeathPrevention();
    testGameClockInterpolation();
    testSimulationSpeedConstants();
    
    // Calendar Integration Tests
    cout << endl << "--- Calendar Integration Tests ---" << endl;
    testCalendarAdvancesPerTick();
    testCalendarTimeConsistency();
    testCalendarIntegrationWithGameClock();
    testTimingRelationshipDocumentation();
    testCalendarBoundaryWithGameClock();
    
    // Additional Edge Case Tests
    cout << endl << "--- Additional Edge Case Tests ---" << endl;
    testGameClockStartReset();
    testGameClockDynamicTickDuration();
    testGameClockFPSCalculation();
    
    cout << endl << "=== All Timing System Tests PASSED ===" << endl;
    cout << endl << "Summary:" << endl;
    cout << "  - GameClock properly implements fixed timestep pattern" << endl;
    cout << "  - Accumulator correctly tracks elapsed time" << endl;
    cout << "  - Spiral of death prevention caps frame time at " << MAX_FRAME_TIME_MS << "ms" << endl;
    cout << "  - Calendar advances 1 minute per simulation tick" << endl;
    cout << "  - At default 50ms/tick: 20 in-game minutes per real second" << endl;
    cout << "  - GameClock and Calendar integrate correctly" << endl;
    
    return 0;
}
