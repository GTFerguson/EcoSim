/**
 * EcoSim Detailed Profiler
 * 
 * Profiles individual components of the simulation tick to identify
 * remaining CPU hotspots after caching optimizations.
 * 
 * Usage:
 *   ./ProfileHotspots [creature_count] [benchmark_ticks]
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

const unsigned GRASS_MIN_ALTITUDE = 165;
const unsigned GRASS_MAX_ALTITUDE = 200;
const unsigned GRASS_SPAWN_RATE = 5;
const unsigned BERRY_MIN_ALTITUDE = 170;
const unsigned BERRY_MAX_ALTITUDE = 190;
const unsigned BERRY_SPAWN_RATE = 3;

static std::mt19937 s_gen(42);

//============================================================================
// Phase Timing Data Structure
//============================================================================

struct PhaseTimings {
    // World-level phases
    double tickCacheUpdate;      // EnvironmentSystem::updateTickCache
    double spatialIndexRebuild;  // World::rebuildCreatureIndex
    double worldObjectsUpdate;   // World::updateAllObjects (plants, seasons, etc.)
    double scentLayerUpdate;     // World::updateScentLayer
    double corpseUpdate;         // World::tickCorpses
    double scentDeposit;         // Creature scent pre-pass
    
    // Creature-level breakdown (summed across all creatures)
    double creatureDeathCheck;
    double creatureUpdate;
    double creaturePhenotypeContext;
    double creatureBehavior;
    
    // Behavior breakdown
    double behaviorHungry;
    double behaviorThirsty;
    double behaviorAmorous;
    double behaviorContent;
    double behaviorTired;
    
    // Creature removal
    double creatureRemoval;
    
    // Total tick
    double totalTick;
    
    // Counts
    unsigned creatureCount;
    unsigned hungryCount;
    unsigned thirstyCount;
    unsigned amorousCount;
    unsigned contentCount;
    unsigned tiredCount;
    unsigned deathCount;
    
    void reset() {
        tickCacheUpdate = 0;
        spatialIndexRebuild = 0;
        worldObjectsUpdate = 0;
        scentLayerUpdate = 0;
        corpseUpdate = 0;
        scentDeposit = 0;
        creatureDeathCheck = 0;
        creatureUpdate = 0;
        creaturePhenotypeContext = 0;
        creatureBehavior = 0;
        behaviorHungry = 0;
        behaviorThirsty = 0;
        behaviorAmorous = 0;
        behaviorContent = 0;
        behaviorTired = 0;
        creatureRemoval = 0;
        totalTick = 0;
        creatureCount = 0;
        hungryCount = 0;
        thirstyCount = 0;
        amorousCount = 0;
        contentCount = 0;
        tiredCount = 0;
        deathCount = 0;
    }
    
    void accumulate(const PhaseTimings& other) {
        tickCacheUpdate += other.tickCacheUpdate;
        spatialIndexRebuild += other.spatialIndexRebuild;
        worldObjectsUpdate += other.worldObjectsUpdate;
        scentLayerUpdate += other.scentLayerUpdate;
        corpseUpdate += other.corpseUpdate;
        scentDeposit += other.scentDeposit;
        creatureDeathCheck += other.creatureDeathCheck;
        creatureUpdate += other.creatureUpdate;
        creaturePhenotypeContext += other.creaturePhenotypeContext;
        creatureBehavior += other.creatureBehavior;
        behaviorHungry += other.behaviorHungry;
        behaviorThirsty += other.behaviorThirsty;
        behaviorAmorous += other.behaviorAmorous;
        behaviorContent += other.behaviorContent;
        behaviorTired += other.behaviorTired;
        creatureRemoval += other.creatureRemoval;
        totalTick += other.totalTick;
        creatureCount += other.creatureCount;
        hungryCount += other.hungryCount;
        thirstyCount += other.thirstyCount;
        amorousCount += other.amorousCount;
        contentCount += other.contentCount;
        tiredCount += other.tiredCount;
        deathCount += other.deathCount;
    }
    
    PhaseTimings average(int count) const {
        PhaseTimings avg = *this;
        if (count > 0) {
            double n = static_cast<double>(count);
            avg.tickCacheUpdate /= n;
            avg.spatialIndexRebuild /= n;
            avg.worldObjectsUpdate /= n;
            avg.scentLayerUpdate /= n;
            avg.corpseUpdate /= n;
            avg.scentDeposit /= n;
            avg.creatureDeathCheck /= n;
            avg.creatureUpdate /= n;
            avg.creaturePhenotypeContext /= n;
            avg.creatureBehavior /= n;
            avg.behaviorHungry /= n;
            avg.behaviorThirsty /= n;
            avg.behaviorAmorous /= n;
            avg.behaviorContent /= n;
            avg.behaviorTired /= n;
            avg.creatureRemoval /= n;
            avg.totalTick /= n;
            avg.creatureCount = creatureCount / count;
            avg.hungryCount = hungryCount / count;
            avg.thirstyCount = thirstyCount / count;
            avg.amorousCount = amorousCount / count;
            avg.contentCount = contentCount / count;
            avg.tiredCount = tiredCount / count;
            avg.deathCount = deathCount / count;
        }
        return avg;
    }
};

//============================================================================
// High-Resolution Timer Helper
//============================================================================

class ScopedTimer {
public:
    ScopedTimer(double& target) : m_target(target) {
        m_start = std::chrono::high_resolution_clock::now();
    }
    
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        m_target += std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count() / 1000.0;
    }
    
private:
    double& m_target;
    std::chrono::high_resolution_clock::time_point m_start;
};

#define TIME_PHASE(target) ScopedTimer _timer##__LINE__(target)

//============================================================================
// World Setup
//============================================================================

World createBenchmarkWorld() {
    MapGen mg {
        42.0,
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
                std::cerr << "[Profiler] Warning: Could not find passable tile" << std::endl;
                return;
            }
        } while (!w.getGrid().at(x).at(y).isPassable());
        
        std::string templateName = templates[i % templates.size()];
        Creature newC = factory.createFromTemplate(templateName, x, y);
        creatures.push_back(std::move(newC));
    }
}

//============================================================================
// Profiled Simulation Tick
//============================================================================

PhaseTimings executeProfiledTick(World& w, std::vector<Creature>& creatures, 
                                 GeneralStats& gs, unsigned tick) {
    PhaseTimings timings;
    timings.reset();
    
    auto totalStart = std::chrono::high_resolution_clock::now();
    
    // Phase 1: Update tick cache
    {
        TIME_PHASE(timings.tickCacheUpdate);
        w.environment().updateTickCache(static_cast<int>(tick));
    }
    
    // Phase 2: Rebuild spatial index
    {
        TIME_PHASE(timings.spatialIndexRebuild);
        w.rebuildCreatureIndex(creatures);
    }
    
    // Phase 3: Update world objects (plants, etc.)
    {
        TIME_PHASE(timings.worldObjectsUpdate);
        w.updateAllObjects();
    }
    
    // Phase 4: Update scent layer
    {
        TIME_PHASE(timings.scentLayerUpdate);
        w.updateScentLayer();
    }
    
    // Phase 5: Tick corpses
    {
        TIME_PHASE(timings.corpseUpdate);
        w.tickCorpses();
    }
    
    // Phase 6: Scent deposit pre-pass
    {
        TIME_PHASE(timings.scentDeposit);
        for (auto& creature : creatures) {
            if (creature.getMotivation() == Motivation::Amorous) {
                creature.depositBreedingScent(w.getScentLayer(), tick);
            }
        }
    }
    
    // Phase 7: Process all creatures
    timings.creatureCount = creatures.size();
    
    for (size_t i = 0; i < creatures.size(); ++i) {
        Creature* activeC = &creatures[i];
        
        if (!activeC->isAlive()) continue;
        
        // Death check
        short dc;
        {
            TIME_PHASE(timings.creatureDeathCheck);
            dc = activeC->deathCheck();
        }
        
        if (dc != 0) {
            timings.deathCount++;
            activeC->setHealth(-1.0f);
            continue;
        }
        
        // Creature update (aging, needs, etc.)
        {
            TIME_PHASE(timings.creatureUpdate);
            activeC->update();
        }
        
        // Phenotype context update
        {
            TIME_PHASE(timings.creaturePhenotypeContext);
            auto localEnv = w.environment().getEnvironmentStateAt(
                static_cast<int>(activeC->getWorldX()),
                static_cast<int>(activeC->getWorldY()));
            activeC->updatePhenotypeContext(localEnv);
        }
        
        // Behavior execution
        Motivation motivation = activeC->getMotivation();
        {
            TIME_PHASE(timings.creatureBehavior);
            
            switch(motivation) {
                case Motivation::Hungry:
                    {
                        TIME_PHASE(timings.behaviorHungry);
                        activeC->hungryBehavior(w, creatures, static_cast<unsigned>(i), gs);
                        timings.hungryCount++;
                    }
                    break;
                case Motivation::Thirsty:
                    {
                        TIME_PHASE(timings.behaviorThirsty);
                        activeC->thirstyBehavior(w, creatures, static_cast<unsigned>(i));
                        timings.thirstyCount++;
                    }
                    break;
                case Motivation::Amorous:
                    {
                        TIME_PHASE(timings.behaviorAmorous);
                        activeC->amorousBehavior(w, creatures, static_cast<unsigned>(i), gs);
                        timings.amorousCount++;
                    }
                    break;
                case Motivation::Content:
                    {
                        TIME_PHASE(timings.behaviorContent);
                        activeC->contentBehavior(w, creatures, static_cast<unsigned>(i));
                        timings.contentCount++;
                    }
                    break;
                case Motivation::Tired:
                    {
                        TIME_PHASE(timings.behaviorTired);
                        activeC->tiredBehavior(w, creatures, static_cast<unsigned>(i));
                        timings.tiredCount++;
                    }
                    break;
            }
        }
    }
    
    // Phase 8: Remove dead creatures
    {
        TIME_PHASE(timings.creatureRemoval);
        creatures.erase(
            std::remove_if(creatures.begin(), creatures.end(),
                [](const Creature& c) { return !c.isAlive(); }),
            creatures.end()
        );
    }
    
    auto totalEnd = std::chrono::high_resolution_clock::now();
    timings.totalTick = std::chrono::duration_cast<std::chrono::nanoseconds>(totalEnd - totalStart).count() / 1000.0;
    
    gs.population = creatures.size();
    
    return timings;
}

//============================================================================
// Reporting
//============================================================================

void printTimingReport(const PhaseTimings& avg, int tickCount) {
    std::cout << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "  DETAILED PROFILING RESULTS" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Ticks measured: " << tickCount << std::endl;
    std::cout << "Avg creatures per tick: " << avg.creatureCount << std::endl;
    std::cout << std::endl;
    
    std::cout << "--- PHASE BREAKDOWN (avg µs per tick) ---" << std::endl;
    std::cout << std::fixed << std::setprecision(1);
    
    double total = avg.totalTick;
    auto printPhase = [total](const std::string& name, double time) {
        double pct = (total > 0) ? (time / total * 100.0) : 0.0;
        std::cout << std::left << std::setw(30) << name 
                  << std::right << std::setw(10) << time << " µs"
                  << std::setw(10) << pct << "%" << std::endl;
    };
    
    std::cout << std::endl;
    std::cout << "WORLD PHASES:" << std::endl;
    printPhase("  Tick cache update", avg.tickCacheUpdate);
    printPhase("  Spatial index rebuild", avg.spatialIndexRebuild);
    printPhase("  World objects update", avg.worldObjectsUpdate);
    printPhase("  Scent layer update", avg.scentLayerUpdate);
    printPhase("  Corpse update", avg.corpseUpdate);
    printPhase("  Scent deposit pre-pass", avg.scentDeposit);
    
    double worldTotal = avg.tickCacheUpdate + avg.spatialIndexRebuild + 
                        avg.worldObjectsUpdate + avg.scentLayerUpdate +
                        avg.corpseUpdate + avg.scentDeposit;
    printPhase("  [World Subtotal]", worldTotal);
    
    std::cout << std::endl;
    std::cout << "CREATURE PHASES (all creatures combined):" << std::endl;
    printPhase("  Death checks", avg.creatureDeathCheck);
    printPhase("  Creature update", avg.creatureUpdate);
    printPhase("  Phenotype context", avg.creaturePhenotypeContext);
    printPhase("  Behavior execution", avg.creatureBehavior);
    printPhase("  Creature removal", avg.creatureRemoval);
    
    double creatureTotal = avg.creatureDeathCheck + avg.creatureUpdate +
                           avg.creaturePhenotypeContext + avg.creatureBehavior +
                           avg.creatureRemoval;
    printPhase("  [Creature Subtotal]", creatureTotal);
    
    std::cout << std::endl;
    std::cout << "BEHAVIOR BREAKDOWN:" << std::endl;
    printPhase("  Hungry behavior", avg.behaviorHungry);
    printPhase("  Thirsty behavior", avg.behaviorThirsty);
    printPhase("  Amorous behavior", avg.behaviorAmorous);
    printPhase("  Content behavior", avg.behaviorContent);
    printPhase("  Tired behavior", avg.behaviorTired);
    
    std::cout << std::endl;
    std::cout << "MOTIVATION DISTRIBUTION (avg per tick):" << std::endl;
    std::cout << "  Hungry:  " << avg.hungryCount << " creatures" << std::endl;
    std::cout << "  Thirsty: " << avg.thirstyCount << " creatures" << std::endl;
    std::cout << "  Amorous: " << avg.amorousCount << " creatures" << std::endl;
    std::cout << "  Content: " << avg.contentCount << " creatures" << std::endl;
    std::cout << "  Tired:   " << avg.tiredCount << " creatures" << std::endl;
    std::cout << "  Deaths:  " << avg.deathCount << " creatures" << std::endl;
    
    std::cout << std::endl;
    std::cout << "--- SUMMARY ---" << std::endl;
    printPhase("TOTAL TICK TIME", avg.totalTick);
    
    double tps = (avg.totalTick > 0) ? (1000000.0 / avg.totalTick) : 0.0;
    std::cout << "Ticks per second: " << std::setprecision(1) << tps << std::endl;
    
    // Per-creature metrics
    if (avg.creatureCount > 0) {
        std::cout << std::endl;
        std::cout << "--- PER-CREATURE METRICS ---" << std::endl;
        std::cout << "Time per creature: " << std::setprecision(2)
                  << (creatureTotal / avg.creatureCount) << " µs" << std::endl;
        std::cout << "Death check per creature: " 
                  << (avg.creatureDeathCheck / avg.creatureCount) << " µs" << std::endl;
        std::cout << "Update per creature: "
                  << (avg.creatureUpdate / avg.creatureCount) << " µs" << std::endl;
        std::cout << "Phenotype context per creature: "
                  << (avg.creaturePhenotypeContext / avg.creatureCount) << " µs" << std::endl;
        std::cout << "Behavior per creature: "
                  << (avg.creatureBehavior / avg.creatureCount) << " µs" << std::endl;
    }
    
    // Identify hotspots
    std::cout << std::endl;
    std::cout << "--- TOP HOTSPOTS ---" << std::endl;
    
    struct Hotspot {
        std::string name;
        double time;
    };
    
    std::vector<Hotspot> hotspots = {
        {"Tick cache update", avg.tickCacheUpdate},
        {"Spatial index rebuild", avg.spatialIndexRebuild},
        {"World objects update", avg.worldObjectsUpdate},
        {"Scent layer update", avg.scentLayerUpdate},
        {"Corpse update", avg.corpseUpdate},
        {"Scent deposit pre-pass", avg.scentDeposit},
        {"Death checks", avg.creatureDeathCheck},
        {"Creature update", avg.creatureUpdate},
        {"Phenotype context", avg.creaturePhenotypeContext},
        {"Hungry behavior", avg.behaviorHungry},
        {"Thirsty behavior", avg.behaviorThirsty},
        {"Amorous behavior", avg.behaviorAmorous},
        {"Content behavior", avg.behaviorContent},
        {"Tired behavior", avg.behaviorTired},
        {"Creature removal", avg.creatureRemoval}
    };
    
    std::sort(hotspots.begin(), hotspots.end(),
              [](const Hotspot& a, const Hotspot& b) { return a.time > b.time; });
    
    for (size_t i = 0; i < std::min(size_t(5), hotspots.size()); i++) {
        double pct = (total > 0) ? (hotspots[i].time / total * 100.0) : 0.0;
        std::cout << "  " << (i + 1) << ". " << std::left << std::setw(25) 
                  << hotspots[i].name
                  << std::right << std::setw(10) << hotspots[i].time << " µs"
                  << " (" << std::setprecision(1) << pct << "%)" << std::endl;
    }
    
    std::cout << std::endl;
}

//============================================================================
// Main
//============================================================================

int main(int argc, char* argv[]) {
    // Parse arguments
    unsigned creatureCount = 200;
    int benchmarkTicks = 100;
    
    if (argc >= 2) {
        creatureCount = static_cast<unsigned>(std::atoi(argv[1]));
        if (creatureCount < 1) creatureCount = 200;
    }
    
    if (argc >= 3) {
        benchmarkTicks = std::atoi(argv[2]);
        if (benchmarkTicks < 1) benchmarkTicks = 100;
    }
    
    std::cout << "============================================" << std::endl;
    std::cout << "  EcoSim Detailed Profiler" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  World size: " << MAP_ROWS << "x" << MAP_COLS << std::endl;
    std::cout << "  Initial creatures: " << creatureCount << std::endl;
    std::cout << "  Benchmark ticks: " << benchmarkTicks << std::endl;
    std::cout << std::endl;
    
    // Initialize genetics
    Creature::initializeGeneRegistry();
    
    // Create world
    std::cout << "Creating world..." << std::endl;
    World w = createBenchmarkWorld();
    
    // Add plants
    std::cout << "Adding plants..." << std::endl;
    addPlants(w);
    
    // Warm up plants
    std::cout << "Warming up plants (50 ticks)..." << std::endl;
    for (int i = 0; i < 50; i++) {
        w.plants().tick(static_cast<unsigned>(i));
    }
    
    // Spawn creatures
    std::cout << "Spawning " << creatureCount << " creatures..." << std::endl;
    std::vector<Creature> creatures;
    spawnCreatures(w, creatures, creatureCount);
    
    // Stats
    Calendar calendar;
    GeneralStats gs = { calendar, 0, 0, 0, 0 };
    
    // Warmup phase
    std::cout << "Running warmup (10 ticks)..." << std::endl;
    for (int i = 0; i < 10; i++) {
        executeProfiledTick(w, creatures, gs, static_cast<unsigned>(i));
    }
    
    // Benchmark phase
    std::cout << "Running benchmark (" << benchmarkTicks << " ticks)..." << std::endl;
    PhaseTimings totalTimings;
    totalTimings.reset();
    
    for (int i = 0; i < benchmarkTicks; i++) {
        PhaseTimings tickTimings = executeProfiledTick(w, creatures, gs, 
                                                        static_cast<unsigned>(i + 10));
        totalTimings.accumulate(tickTimings);
        
        if ((i + 1) % 25 == 0) {
            std::cout << "  Completed " << (i + 1) << "/" << benchmarkTicks 
                      << " ticks (" << creatures.size() << " creatures)" << std::endl;
        }
    }
    
    // Compute averages
    PhaseTimings avgTimings = totalTimings.average(benchmarkTicks);
    
    // Print report
    printTimingReport(avgTimings, benchmarkTicks);
    
    std::cout << "============================================" << std::endl;
    std::cout << "  Profiling complete!" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return 0;
}
