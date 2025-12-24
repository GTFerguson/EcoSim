/**
 * @file timing.hpp
 * @brief Game loop timing utilities for fixed timestep simulation
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file provides timing utilities for implementing a proper "Fix Your Timestep"
 * game loop pattern. This separates simulation updates (fixed timestep) from
 * input handling and rendering (variable timestep / as fast as possible).
 * 
 * Key concepts:
 * - Frame: One iteration of the game loop (input + render)
 * - Tick: One simulation update (world, creatures, plants)
 * - TICK_DURATION_MS: Fixed time per simulation tick
 * - Frame Rate: As fast as possible, or optionally capped
 * 
 * The pattern ensures:
 * 1. Input is processed every frame (responsive controls)
 * 2. Simulation runs at a consistent rate regardless of frame rate
 * 3. Rendering is smooth and not tied to simulation speed
 */

#ifndef ECOSIM_TIMING_HPP
#define ECOSIM_TIMING_HPP

#include <chrono>

namespace EcoSim {
namespace Timing {

//==============================================================================
// Configuration Constants
//==============================================================================

/**
 * @brief Duration of one simulation tick in milliseconds
 * 
 * At 1000ms per tick, simulation runs at 1 tick/second.
 * This means 1 real second = 1 game minute (calendar advances 1 min per tick).
 * 
 * Adjusting this value:
 * - Lower = faster simulation, more CPU usage
 * - Higher = slower simulation, less CPU usage
 */
constexpr double DEFAULT_TICK_DURATION_MS = 1000.0;

/**
 * @brief Maximum frame time to prevent "spiral of death"
 * 
 * If a frame takes longer than this, we cap the accumulated time.
 * This prevents the simulation from trying to catch up with too many
 * ticks at once (which would cause more slow frames, causing more ticks, etc.)
 * 
 * At 250ms, we allow at most 5 simulation ticks per frame before capping.
 */
constexpr double MAX_FRAME_TIME_MS = 250.0;

/**
 * @brief Minimum frame time for optional frame rate cap
 * 
 * At ~16.67ms, this caps rendering at approximately 60 FPS.
 * Set to 0 for uncapped frame rate.
 */
constexpr double TARGET_FRAME_TIME_MS = 0.0;  // 0 = uncapped

//==============================================================================
// GameClock Class
//==============================================================================

/**
 * @brief High-resolution clock for game loop timing
 * 
 * Provides utilities for measuring frame time and managing the accumulator
 * for fixed timestep simulation updates.
 * 
 * Usage:
 * @code
 * GameClock clock;
 * clock.start();
 * 
 * while (running) {
 *     clock.tick();
 *     
 *     // Process input every frame
 *     handleInput();
 *     
 *     // Run simulation at fixed timestep
 *     while (clock.shouldUpdate()) {
 *         world.update();
 *         clock.consumeTick();
 *     }
 *     
 *     // Render every frame
 *     render();
 * }
 * @endcode
 */
class GameClock {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double, std::milli>;

    /**
     * @brief Constructor with configurable tick duration
     * @param tickDurationMs Duration of one simulation tick in milliseconds
     */
    explicit GameClock(double tickDurationMs = DEFAULT_TICK_DURATION_MS)
        : _tickDurationMs(tickDurationMs)
        , _accumulator(0.0)
        , _frameTime(0.0)
        , _totalTime(0.0)
        , _tickCount(0)
        , _frameCount(0) {
        _lastTime = Clock::now();
        _startTime = _lastTime;
    }

    /**
     * @brief Start or reset the clock
     */
    void start() {
        _lastTime = Clock::now();
        _startTime = _lastTime;
        _accumulator = 0.0;
        _totalTime = 0.0;
        _tickCount = 0;
        _frameCount = 0;
    }

    /**
     * @brief Update the clock at the start of each frame
     * 
     * Calculates the time since the last frame and adds it to the accumulator.
     * Also applies the spiral of death prevention cap.
     */
    void tick() {
        TimePoint currentTime = Clock::now();
        _frameTime = Duration(currentTime - _lastTime).count();
        _lastTime = currentTime;
        
        // Prevent spiral of death
        if (_frameTime > MAX_FRAME_TIME_MS) {
            _frameTime = MAX_FRAME_TIME_MS;
        }
        
        _accumulator += _frameTime;
        _totalTime += _frameTime;
        _frameCount++;
    }

    /**
     * @brief Check if a simulation tick should be performed
     * @return true if enough time has accumulated for a tick
     */
    bool shouldUpdate() const {
        return _accumulator >= _tickDurationMs;
    }

    /**
     * @brief Consume one tick's worth of time from the accumulator
     * 
     * Call this after each simulation update.
     */
    void consumeTick() {
        _accumulator -= _tickDurationMs;
        _tickCount++;
    }

    /**
     * @brief Get the time since the last frame in milliseconds
     */
    double getFrameTime() const { return _frameTime; }

    /**
     * @brief Get the current accumulator value
     */
    double getAccumulator() const { return _accumulator; }

    /**
     * @brief Get the tick duration in milliseconds
     */
    double getTickDuration() const { return _tickDurationMs; }

    /**
     * @brief Set the tick duration in milliseconds
     */
    void setTickDuration(double ms) { _tickDurationMs = ms; }

    /**
     * @brief Get ticks per second (simulation speed)
     */
    double getTicksPerSecond() const { return 1000.0 / _tickDurationMs; }

    /**
     * @brief Set ticks per second (simulation speed)
     */
    void setTicksPerSecond(double tps) { 
        if (tps > 0) {
            _tickDurationMs = 1000.0 / tps; 
        }
    }

    /**
     * @brief Get total time elapsed since start in milliseconds
     */
    double getTotalTime() const { return _totalTime; }

    /**
     * @brief Get total number of simulation ticks performed
     */
    uint64_t getTickCount() const { return _tickCount; }

    /**
     * @brief Get total number of frames rendered
     */
    uint64_t getFrameCount() const { return _frameCount; }

    /**
     * @brief Get interpolation factor for smooth rendering
     * 
     * Returns a value between 0 and 1 representing how far we are
     * between the last simulation tick and the next one. Can be used
     * for interpolating visual positions for smoother rendering.
     */
    double getInterpolation() const {
        return _accumulator / _tickDurationMs;
    }

    /**
     * @brief Get the average frames per second
     */
    double getAverageFPS() const {
        if (_totalTime > 0) {
            return (_frameCount * 1000.0) / _totalTime;
        }
        return 0.0;
    }

    /**
     * @brief Get the current instantaneous frames per second
     */
    double getCurrentFPS() const {
        if (_frameTime > 0) {
            return 1000.0 / _frameTime;
        }
        return 0.0;
    }

private:
    double _tickDurationMs;     ///< Duration of one simulation tick
    double _accumulator;        ///< Accumulated time for simulation
    double _frameTime;          ///< Time of last frame
    double _totalTime;          ///< Total time since start
    TimePoint _lastTime;        ///< Time of last tick() call
    TimePoint _startTime;       ///< Time when start() was called
    uint64_t _tickCount;        ///< Total simulation ticks
    uint64_t _frameCount;       ///< Total frames rendered
};

//==============================================================================
// SimulationSpeed Helper
//==============================================================================

/**
 * @brief Predefined simulation speed settings
 * 
 * These provide user-friendly speed options with corresponding
 * tick durations in milliseconds.
 */
struct SimulationSpeed {
    static constexpr double PAUSED     = 0.0;      // No simulation
    static constexpr double VERY_SLOW  = 4000.0;   // 0.25 ticks/second (4 sec/game-min)
    static constexpr double SLOW       = 2000.0;   // 0.5 ticks/second (2 sec/game-min)
    static constexpr double NORMAL     = 1000.0;   // 1 tick/second (1 sec = 1 game-min)
    static constexpr double FAST       = 500.0;    // 2 ticks/second (0.5 sec/game-min)
    static constexpr double VERY_FAST  = 100.0;    // 10 ticks/second (0.1 sec/game-min)
    static constexpr double REALTIME   = 10.0;     // 100 ticks/second (for testing)
    
    /**
     * @brief Convert ticks per second to milliseconds per tick
     */
    static constexpr double ticksToMs(double ticksPerSecond) {
        return ticksPerSecond > 0 ? 1000.0 / ticksPerSecond : 0.0;
    }
    
    /**
     * @brief Convert milliseconds per tick to ticks per second
     */
    static constexpr double msToTicks(double msPerTick) {
        return msPerTick > 0 ? 1000.0 / msPerTick : 0.0;
    }
};

} // namespace Timing
} // namespace EcoSim

#endif // ECOSIM_TIMING_HPP
