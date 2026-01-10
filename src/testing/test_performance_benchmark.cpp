/**
 * EcoSim Performance Benchmark
 * 
 * Measures simulation tick performance with varying numbers of organisms.
 * Useful for validating optimizations and identifying performance bottlenecks.
 * 
 * Usage:
 *   ./PerformanceBenchmark [warmup_ticks] [benchmark_ticks]
 * 
 * Arguments:
 *   warmup_ticks    - Number of warmup ticks to stabilize (default: 10)
 *   benchmark_ticks - Number of ticks to measure (default: 100)
 */

#include "../../include/world/world.hpp"
#include "../../include/objects/creature/creature.hpp"
#include "../../include/genetics/organisms/CreatureFactory.hpp"
#include "../../include/statistics/statistics.hpp"
#include "../../include/calendar.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

//============================================================================
// Benchmark Configuration
//============================================================================
const unsigned MAP_ROWS = 200;
const unsigned MAP_COLS = 200;
const double WORLD_DEFAULT_SCALE = 0.0035;
const double WORLD_DEFAULT_FREQUENCY = 4;
const double WORLD_DEFAULT_EXPONENT = 0.8;
const unsigned WORLD_DEFAULT_TERRACES = 64;

// Plant configuration for testing
const unsigned GRASS_MIN_ALTITUDE = 165;
const unsigned GRASS_MAX_ALTITUDE = 200;
const unsigned GRASS_SPAWN_RATE = 5;
const unsigned BERRY_MIN_ALTITUDE = 170;
const unsigned BERRY_MAX_ALTITUDE = 190;
const unsigned BERRY_SPAWN_RATE = 3;

static std::mt19937 s_gen(42);  // Fixed seed for reproducibility

//============================================================================
// Timing Utilities
//============================================================================

/**
 * Measure execution time of a function
 * @param func Function to measure
 * @param iterations Number of times to run
 * @return Average time per iteration in microseconds
 */
template<typename F>
double measureTime(F&& func, int iterations = 1) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return static_cast<double>(duration.count()) / iterations;
}

/**
 * Compute statistics for a vector of measurements
 */
struct BenchmarkStats {
    double mean;
    double median;
    double stdDev;
    double min;
    double max;
    double p95;  // 95th percentile
    
    static BenchmarkStats compute(std::vector<double>& values) {
        BenchmarkStats stats;
        if (values.empty()) return stats;
        
        std::sort(values.begin(), values.end());
        
        stats.min = values.front();
        stats.max = values.back();
        stats.median = values[values.size() / 2];
        
        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        stats.mean = sum / values.size();
        
        double sqSum = 0.0;
        for (double v : values) {
            sqSum += (v - stats.mean) * (v - stats.mean);
        }
        stats.stdDev = std::sqrt(sqSum / values.size());
        
        size_t p95Idx = static_cast<size_t>(values.size() * 0.95);
        stats.p95 = values[std::min(p95Idx, values.size() - 1)];
        
        return stats;
    }
};

//============================================================================
// World Setup
//============================================================================

World createBenchmarkWorld() {
    MapGen mg {
        42.0,  // Fixed seed for reproducibility
        WORLD_DEFAULT_SCALE,
        WORLD_DEFAULT_FREQUENCY,
        WORLD_DEFAULT_EXPONENT,
        WORLD_DEFAULT_TERRACES,
        MAP_ROWS,
        MAP_COLS,
        false
    };
    
    OctaveGen og { 2, 0.25, 0.5, 2 };
    
    return World(mg, og);
}

void addPlants(World& w) {
    w.plants().initialize();
    
    w.plants().addPlants(GRASS_MIN_ALTITUDE, GRASS_MAX_ALTITUDE,
                         GRASS_SPAWN_RATE, "grass");
    w.plants().addPlants(BERRY_MIN_ALTITUDE, BERRY_MAX_ALTITUDE,
                         BERRY_SPAWN_RATE, "berry_bush");
}

/**
 * Spawn a specified number of creatures
 */
void spawnCreatures(World& w, std::vector<Creature>& creatures, unsigned count) {
    std::uniform_int_distribution<int> colDis(0, MAP_COLS - 1);
    std::uniform_int_distribution<int> rowDis(0, MAP_ROWS - 1);
    const unsigned MAX_ATTEMPTS = 10000;
    
    auto registry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
    EcoSim::Genetics::CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::string> templates = {"grazer", "browser", "hunter", "forager"};
    
    for (unsigned i = 0; i < count; i++) {
        int x, y;
        unsigned attempts = 0;
        do {
            x = colDis(s_gen);
            y = rowDis(s_gen);
            if (++attempts > MAX_ATTEMPTS) {
                std::cerr << "[Benchmark] Warning: Could not find passable tile" << std::endl;
                return;
            }
        } while (!w.getGrid().at(x).at(y).isPassable());
        
        std::string templateName = templates[i % templates.size()];
        Creature newC = factory.createFromTemplate(templateName, x, y);
        creatures.push_back(std::move(newC));
    }
}

//============================================================================
// Simulation Tick
//============================================================================

/**
 * Execute one simulation tick (simplified for benchmarking)
 */
void executeTick(World& w, std::vector<Creature>& creatures, GeneralStats& gs) {
    // Update world objects
    w.updateAllObjects();
    
    // Process creatures in reverse order (for safe removal)
    for (int i = static_cast<int>(creatures.size()) - 1; i >= 0; i--) {
        Creature* activeC = &creatures[i];
        
        // Death check
        short dc = activeC->deathCheck();
        if (dc != 0) {
            creatures.erase(creatures.begin() + i);
            continue;
        }
        
        // Update creature
        activeC->update();
        
        // Update phenotype context
        auto localEnv = w.environment().getEnvironmentStateAt(
            static_cast<int>(activeC->getWorldX()),
            static_cast<int>(activeC->getWorldY()));
        activeC->updatePhenotypeContext(localEnv);
        
        // Execute behavior based on motivation
        switch(activeC->getMotivation()) {
            case Motivation::Content:   activeC->contentBehavior(w, creatures, i); break;
            case Motivation::Hungry:    activeC->hungryBehavior(w, creatures, i, gs); break;
            case Motivation::Thirsty:   activeC->thirstyBehavior(w, creatures, i); break;
            case Motivation::Amorous:   activeC->amorousBehavior(w, creatures, i, gs); break;
            case Motivation::Tired:     break;
        }
    }
    
    gs.population = creatures.size();
}

//============================================================================
// Benchmark Runner
//============================================================================

struct BenchmarkResult {
    unsigned creatureCount;
    unsigned plantCount;
    BenchmarkStats tickStats;
    double ticksPerSecond;
    unsigned finalCreatures;
};

BenchmarkResult runBenchmark(unsigned creatureCount, int warmupTicks, int benchmarkTicks) {
    BenchmarkResult result;
    result.creatureCount = creatureCount;
    
    // Initialize genetics system
    Creature::initializeGeneRegistry();
    
    // Create world
    World w = createBenchmarkWorld();
    
    // Add plants
    addPlants(w);
    
    // Warm up plants
    const int PLANT_WARMUP = 50;
    for (int i = 0; i < PLANT_WARMUP; i++) {
        w.plants().tick(static_cast<unsigned>(i));
    }
    
    // Plants are stored in tiles; approximate count is not easily accessible
    // Skip counting plants for this benchmark
    result.plantCount = 0;
    
    // Spawn creatures
    std::vector<Creature> creatures;
    spawnCreatures(w, creatures, creatureCount);
    
    // Create stats
    Calendar calendar;
    GeneralStats gs = { calendar, 0, 0, 0, 0 };
    
    // Warmup phase (not measured)
    for (int i = 0; i < warmupTicks; i++) {
        executeTick(w, creatures, gs);
    }
    
    // Benchmark phase
    std::vector<double> tickTimes;
    tickTimes.reserve(benchmarkTicks);
    
    for (int i = 0; i < benchmarkTicks; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        executeTick(w, creatures, gs);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        tickTimes.push_back(static_cast<double>(duration.count()));
    }
    
    // Compute statistics
    result.tickStats = BenchmarkStats::compute(tickTimes);
    result.ticksPerSecond = 1000000.0 / result.tickStats.mean;
    result.finalCreatures = creatures.size();
    
    return result;
}

void printHeader() {
    std::cout << "========================================" << std::endl;
    std::cout << "  EcoSim Performance Benchmark" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "World size: " << MAP_ROWS << "x" << MAP_COLS << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

void printResultsTable(const std::vector<BenchmarkResult>& results) {
    std::cout << std::endl;
    std::cout << "=== Benchmark Results ===" << std::endl;
    std::cout << std::endl;
    
    // Print header
    std::cout << std::left 
              << std::setw(12) << "Creatures"
              << std::setw(10) << "Plants"
              << std::setw(12) << "Mean (µs)"
              << std::setw(12) << "Median"
              << std::setw(12) << "StdDev"
              << std::setw(10) << "Min"
              << std::setw(10) << "Max"
              << std::setw(10) << "P95"
              << std::setw(10) << "TPS"
              << std::setw(10) << "Final"
              << std::endl;
    
    std::cout << std::string(108, '-') << std::endl;
    
    // Print results
    std::cout << std::fixed << std::setprecision(1);
    for (const auto& r : results) {
        std::cout << std::left
                  << std::setw(12) << r.creatureCount
                  << std::setw(10) << r.plantCount
                  << std::setw(12) << r.tickStats.mean
                  << std::setw(12) << r.tickStats.median
                  << std::setw(12) << r.tickStats.stdDev
                  << std::setw(10) << r.tickStats.min
                  << std::setw(10) << r.tickStats.max
                  << std::setw(10) << r.tickStats.p95
                  << std::setw(10) << r.ticksPerSecond
                  << std::setw(10) << r.finalCreatures
                  << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Legend:" << std::endl;
    std::cout << "  Mean/Median/Min/Max/P95 - Tick times in microseconds" << std::endl;
    std::cout << "  StdDev - Standard deviation of tick times" << std::endl;
    std::cout << "  TPS - Ticks per second achievable" << std::endl;
    std::cout << "  Final - Final creature count after benchmark" << std::endl;
}

void printScalingSummary(const std::vector<BenchmarkResult>& results) {
    std::cout << std::endl;
    std::cout << "=== Scaling Analysis ===" << std::endl;
    std::cout << std::endl;
    
    if (results.size() < 2) return;
    
    // Analyze how tick time scales with creature count
    std::cout << "Time scaling (compared to " << results[0].creatureCount << " creatures):" << std::endl;
    
    double baseTime = results[0].tickStats.mean;
    unsigned baseCount = results[0].creatureCount;
    
    std::cout << std::fixed << std::setprecision(2);
    for (size_t i = 1; i < results.size(); i++) {
        double timeRatio = results[i].tickStats.mean / baseTime;
        double countRatio = static_cast<double>(results[i].creatureCount) / baseCount;
        double scalingFactor = timeRatio / countRatio;  // 1.0 = linear, <1.0 = sub-linear, >1.0 = super-linear
        
        std::string scalingDesc;
        if (scalingFactor < 0.9) scalingDesc = "(sub-linear - GOOD)";
        else if (scalingFactor <= 1.1) scalingDesc = "(linear)";
        else if (scalingFactor <= 1.5) scalingDesc = "(mildly super-linear)";
        else scalingDesc = "(super-linear - needs optimization)";
        
        std::cout << "  " << results[i].creatureCount << " creatures: "
                  << timeRatio << "x time for " << countRatio << "x creatures "
                  << scalingDesc << std::endl;
    }
}

//============================================================================
// Main
//============================================================================

int main(int argc, char* argv[]) {
    // Parse arguments
    int warmupTicks = 10;
    int benchmarkTicks = 100;
    
    if (argc >= 2) {
        warmupTicks = std::atoi(argv[1]);
        if (warmupTicks < 0) warmupTicks = 10;
    }
    
    if (argc >= 3) {
        benchmarkTicks = std::atoi(argv[2]);
        if (benchmarkTicks < 1) benchmarkTicks = 100;
    }
    
    printHeader();
    std::cout << "Warmup ticks: " << warmupTicks << std::endl;
    std::cout << "Benchmark ticks: " << benchmarkTicks << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Test with varying creature counts
    std::vector<unsigned> creatureCounts = {25, 50, 100, 200, 400};
    std::vector<BenchmarkResult> results;
    
    for (unsigned count : creatureCounts) {
        std::cout << "Running benchmark with " << count << " creatures..." << std::flush;
        
        BenchmarkResult result = runBenchmark(count, warmupTicks, benchmarkTicks);
        results.push_back(result);
        
        std::cout << " done (" << std::fixed << std::setprecision(1) 
                  << result.tickStats.mean << " µs/tick, "
                  << result.ticksPerSecond << " TPS)" << std::endl;
    }
    
    // Print results
    printResultsTable(results);
    printScalingSummary(results);
    
    std::cout << "========================================" << std::endl;
    std::cout << "Benchmark complete!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
