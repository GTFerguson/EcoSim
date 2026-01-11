/**
 * @file headlessSimulation.cpp
 * @brief Headless simulation runner for debugging creature movement and segfaults
 * 
 * Runs the EcoSim simulation without GUI (no SDL2) as fast as possible.
 * Designed for quick debugging of:
 * - Creature movement issues (stuck creatures)
 * - Segfaults during simulation
 * - Behavioral bugs
 * 
 * Features:
 * - Signal handler for SIGSEGV with stack trace
 * - Configurable debug logging
 * - No FPS limit (runs at maximum speed)
 * - Periodic status reports
 * 
 * Usage:
 *   ./HeadlessSimulation [options]
 * 
 * Options:
 *   -t, --ticks N       Number of ticks to run (default: 1000)
 *   -p, --population N  Initial creature population (default: 100)
 *   -s, --seed N        World generation seed (default: random)
 *   -v, --verbose       Enable verbose debug output
 *   --nav-debug         Enable navigator debug logging
 *   --behavior-debug    Enable creature behavior debug logging
 */

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <map>

// Platform-specific backtrace support
#ifdef __APPLE__
#include <execinfo.h>
#include <unistd.h>
#define HAS_BACKTRACE 1
#elif defined(__linux__)
#include <execinfo.h>
#include <unistd.h>
#define HAS_BACKTRACE 1
#else
#define HAS_BACKTRACE 0
#endif

// Core includes
#include "../../include/world/world.hpp"
#include "../../include/world/ClimateWorldGenerator.hpp"
#include "../../include/objects/creature/creature.hpp"
#include "../../include/calendar.hpp"
#include "../../include/statistics/statistics.hpp"
#include "../../include/logging/Logger.hpp"

// Genetics system
#include "../../include/genetics/defaults/UniversalGenes.hpp"
#include "../../include/genetics/organisms/PlantFactory.hpp"
#include "../../include/genetics/organisms/CreatureFactory.hpp"
#include "../../include/genetics/organisms/BiomeVariantExamples.hpp"

using namespace std;
using namespace logging;

//================================================================================
// Configuration
//================================================================================
struct SimulationConfig {
    int maxTicks = 1000;
    unsigned population = 100;
    unsigned seed = 0;
    bool verbose = false;
    bool navDebug = false;
    bool behaviorDebug = false;
    unsigned mapWidth = 200;
    unsigned mapHeight = 200;
    int statusInterval = 100;
};

//================================================================================
// Global state for signal handler
//================================================================================
static int g_currentTick = 0;
static size_t g_creatureCount = 0;
static std::string g_lastAction = "initializing";

//================================================================================
// Signal Handler
//================================================================================
void signalHandler(int sig) {
    fprintf(stderr, "\n");
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "  SIGNAL CAUGHT: %d (%s)\n", sig,
            sig == SIGSEGV ? "SIGSEGV - Segmentation Fault" :
            sig == SIGABRT ? "SIGABRT - Abort" :
            sig == SIGFPE ? "SIGFPE - Floating Point Exception" :
            "Unknown");
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "State at crash:\n");
    fprintf(stderr, "  Current tick: %d\n", g_currentTick);
    fprintf(stderr, "  Creature count: %zu\n", g_creatureCount);
    fprintf(stderr, "  Last action: %s\n", g_lastAction.c_str());
    fprintf(stderr, "========================================\n");
    
#if HAS_BACKTRACE
    fprintf(stderr, "Stack trace:\n");
    void* array[50];
    size_t size = backtrace(array, 50);
    backtrace_symbols_fd(array, static_cast<int>(size), STDERR_FILENO);
    fprintf(stderr, "========================================\n");
#else
    fprintf(stderr, "Stack trace not available on this platform.\n");
    fprintf(stderr, "Run with debugger for more details.\n");
    fprintf(stderr, "========================================\n");
#endif
    
    exit(1);
}

//================================================================================
// Argument Parsing
//================================================================================
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n\n"
              << "Options:\n"
              << "  -t, --ticks N         Number of ticks to run (default: 1000)\n"
              << "  -p, --population N    Initial creature population (default: 100)\n"
              << "  -s, --seed N          World generation seed (default: random)\n"
              << "  -w, --width N         Map width (default: 200)\n"
              << "  -h, --height N        Map height (default: 200)\n"
              << "  -i, --interval N      Status report interval (default: 100)\n"
              << "  -v, --verbose         Enable verbose debug output\n"
              << "  --nav-debug           Enable navigator debug logging\n"
              << "  --behavior-debug      Enable creature behavior debug logging\n"
              << "  --help                Show this help message\n";
}

SimulationConfig parseArgs(int argc, char* argv[]) {
    SimulationConfig config;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        } else if ((arg == "-t" || arg == "--ticks") && i + 1 < argc) {
            config.maxTicks = std::atoi(argv[++i]);
        } else if ((arg == "-p" || arg == "--population") && i + 1 < argc) {
            config.population = static_cast<unsigned>(std::atoi(argv[++i]));
        } else if ((arg == "-s" || arg == "--seed") && i + 1 < argc) {
            config.seed = static_cast<unsigned>(std::atoi(argv[++i]));
        } else if ((arg == "-w" || arg == "--width") && i + 1 < argc) {
            config.mapWidth = static_cast<unsigned>(std::atoi(argv[++i]));
        } else if ((arg == "-h" || arg == "--height") && i + 1 < argc) {
            config.mapHeight = static_cast<unsigned>(std::atoi(argv[++i]));
        } else if ((arg == "-i" || arg == "--interval") && i + 1 < argc) {
            config.statusInterval = std::atoi(argv[++i]);
        } else if (arg == "-v" || arg == "--verbose") {
            config.verbose = true;
        } else if (arg == "--nav-debug") {
            config.navDebug = true;
        } else if (arg == "--behavior-debug") {
            config.behaviorDebug = true;
        }
    }
    
    // Generate random seed if not specified
    if (config.seed == 0) {
        std::random_device rd;
        config.seed = rd();
    }
    
    return config;
}

//================================================================================
// World Initialization (using ClimateWorldGenerator)
//================================================================================
World initializeWorld(const SimulationConfig& config) {
    // Create default world parameters for MapGen (base structure)
    MapGen mg {
        static_cast<double>(config.seed),
        0.0035,   // scale
        4.0,      // frequency
        0.8,      // exponent
        64,       // terraces
        config.mapHeight,
        config.mapWidth,
        false     // not wrapped
    };
    
    OctaveGen og { 2, 0.25, 0.5, 2 };
    
    // Create world with basic parameters
    World world(mg, og);
    
    // Regenerate using ClimateWorldGenerator for realistic biomes
    world.regenerateClimate(config.seed);
    
    return world;
}

//================================================================================
// Creature Population (biome-based)
//================================================================================
void populateWorldByBiome(World& w, std::vector<Creature>& creatures, 
                          unsigned amount, const SimulationConfig& config) {
    using namespace EcoSim;
    using namespace EcoSim::Genetics;
    
    auto registry = std::make_shared<GeneRegistry>();
    BiomeVariantFactory biomeFactory(registry);
    CreatureFactory standardFactory(registry);
    standardFactory.registerDefaultTemplates();
    
    std::cout << "[Headless] Populating by biome with " << amount << " creatures..." << std::endl;
    
    // Collect valid spawn positions for each biome category
    std::vector<std::pair<int, int>> tundraPositions;
    std::vector<std::pair<int, int>> desertPositions;
    std::vector<std::pair<int, int>> tropicalPositions;
    std::vector<std::pair<int, int>> temperatePositions;
    
    const auto& grid = w.grid();
    for (unsigned x = 0; x < grid.width(); ++x) {
        for (unsigned y = 0; y < grid.height(); ++y) {
            if (!grid(x, y).isPassable()) continue;
            
            int biomeInt = w.environment().getBiome(static_cast<int>(x), static_cast<int>(y));
            Biome biome = static_cast<Biome>(biomeInt);
            
            switch (biome) {
                case Biome::OCEAN_DEEP:
                case Biome::OCEAN_SHALLOW:
                case Biome::OCEAN_COAST:
                case Biome::FRESHWATER:
                    continue;  // Skip water biomes
                
                case Biome::ICE_SHEET:
                case Biome::TUNDRA:
                case Biome::TAIGA:
                case Biome::BOREAL_FOREST:
                case Biome::ALPINE_TUNDRA:
                case Biome::GLACIER:
                    tundraPositions.push_back({x, y});
                    break;
                    
                case Biome::DESERT_HOT:
                case Biome::DESERT_COLD:
                case Biome::STEPPE:
                case Biome::SHRUBLAND:
                    desertPositions.push_back({x, y});
                    break;
                    
                case Biome::TROPICAL_RAINFOREST:
                case Biome::TROPICAL_SEASONAL_FOREST:
                case Biome::SAVANNA:
                    tropicalPositions.push_back({x, y});
                    break;
                    
                default:
                    temperatePositions.push_back({x, y});
                    break;
            }
        }
    }
    
    unsigned totalPositions = static_cast<unsigned>(
        tundraPositions.size() + desertPositions.size() +
        tropicalPositions.size() + temperatePositions.size());
    
    if (totalPositions == 0) {
        std::cerr << "[Headless] Error: No valid spawn positions found!" << std::endl;
        return;
    }
    
    // Calculate distribution based on biome area
    auto calculateBiomeCount = [&](size_t biomePositions) -> unsigned {
        if (biomePositions == 0) return 0;
        float proportion = static_cast<float>(biomePositions) / static_cast<float>(totalPositions);
        return std::max(2u, static_cast<unsigned>(amount * proportion));
    };
    
    unsigned tundraCount = calculateBiomeCount(tundraPositions.size());
    unsigned desertCount = calculateBiomeCount(desertPositions.size());
    unsigned tropicalCount = calculateBiomeCount(tropicalPositions.size());
    unsigned temperateCount = calculateBiomeCount(temperatePositions.size());
    
    std::mt19937 rng(config.seed);
    
    // Helper lambda for spawning creatures in a biome
    auto spawnInBiome = [&](
            std::vector<std::pair<int, int>>& positions,
            unsigned count,
            std::function<Creature(int, int)> createHerbivore,
            std::function<Creature(int, int)> createCarnivore) {
        
        if (positions.empty() || count == 0) return;
        
        std::uniform_int_distribution<size_t> posDist(0, positions.size() - 1);
        
        unsigned herbivoreCount = static_cast<unsigned>(count * 0.70f);
        unsigned carnivoreCount = count - herbivoreCount;
        
        for (unsigned i = 0; i < herbivoreCount && !positions.empty(); ++i) {
            size_t idx = posDist(rng);
            auto [x, y] = positions[idx];
            Creature creature = createHerbivore(x, y);
            creature.setXY(x, y);
            creature.setWorldPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            creatures.push_back(std::move(creature));
        }
        
        for (unsigned i = 0; i < carnivoreCount && !positions.empty(); ++i) {
            size_t idx = posDist(rng);
            auto [x, y] = positions[idx];
            Creature creature = createCarnivore(x, y);
            creature.setXY(x, y);
            creature.setWorldPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            creatures.push_back(std::move(creature));
        }
    };
    
    // Spawn in each biome type
    spawnInBiome(tundraPositions, tundraCount,
        [&](int x, int y) { return biomeFactory.createWoollyMammoth(x, y); },
        [&](int x, int y) { return biomeFactory.createArcticWolf(x, y); });
    
    spawnInBiome(desertPositions, desertCount,
        [&](int x, int y) { return biomeFactory.createDesertCamel(x, y); },
        [&](int x, int y) { return biomeFactory.createDesertFennec(x, y); });
    
    spawnInBiome(tropicalPositions, tropicalCount,
        [&](int x, int y) { return biomeFactory.createJungleElephant(x, y); },
        [&](int x, int y) { return biomeFactory.createTropicalJaguar(x, y); });
    
    // Temperate uses standard archetypes
    if (!temperatePositions.empty() && temperateCount > 0) {
        std::uniform_int_distribution<size_t> posDist(0, temperatePositions.size() - 1);
        std::vector<Creature> tempCreatures = standardFactory.createEcosystemMix(
            temperateCount, config.mapWidth, config.mapHeight);
        
        for (auto& creature : tempCreatures) {
            size_t idx = posDist(rng);
            auto [x, y] = temperatePositions[idx];
            creature.setXY(x, y);
            creature.setWorldPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            creatures.push_back(std::move(creature));
        }
    }
    
    std::cout << "[Headless] Spawned " << creatures.size() << " creatures" << std::endl;
    std::cout << "  Tundra: " << tundraCount << ", Desert: " << desertCount
              << ", Tropical: " << tropicalCount << ", Temperate: " << temperateCount << std::endl;
}

//================================================================================
// Plant Initialization
//================================================================================
void addGeneticsPlants(World& w) {
    w.plants().initialize();
    const unsigned BIOME_PLANT_RATE = 5;
    w.plants().addPlantsByBiome(BIOME_PLANT_RATE);
}

//================================================================================
// Simulation Tick (from main.cpp takeTurn logic)
//================================================================================
bool takeTurn(World& w, GeneralStats& gs, std::vector<Creature>& c, 
              unsigned int cIndex, const SimulationConfig& config) {
    Creature* activeC = &c.at(cIndex);
    
    g_lastAction = "checking death for creature " + std::to_string(activeC->getId());
    
    short dc = activeC->deathCheck();
    if (dc != 0) {
        std::string deathCause;
        switch (dc) {
            case 1: gs.deaths.oldAge++;     deathCause = "old_age";     break;
            case 2: gs.deaths.starved++;    deathCause = "starvation";  break;
            case 3: gs.deaths.dehydrated++; deathCause = "dehydration"; break;
            case 4: gs.deaths.discomfort++; deathCause = "discomfort";  break;
            case 5: gs.deaths.predator++;   deathCause = "combat";      break;
            default: deathCause = "unknown"; break;
        }
        
        if (config.verbose) {
            std::cout << "  [Death] Creature " << activeC->getId() 
                      << " died from " << deathCause << std::endl;
        }
        
        float creatureSize = activeC->getMaxHealth() / 50.0f;
        if (creatureSize > 0.1f) {
            float bodyCondition = std::max(0.0f, std::min(1.0f, activeC->getHunger() / 10.0f));
            w.addCorpse(activeC->getWorldX(), activeC->getWorldY(), 
                       creatureSize, activeC->generateName(), bodyCondition);
        }
        
        activeC->setHealth(-1.0f);
        return true;
    } else {
        g_lastAction = "updating creature " + std::to_string(activeC->getId());
        activeC->update();
        
        auto localEnv = w.environment().getEnvironmentStateAt(
            static_cast<int>(activeC->getWorldX()),
            static_cast<int>(activeC->getWorldY()));
        activeC->updatePhenotypeContext(localEnv);
        
        g_lastAction = "executing behavior for creature " + std::to_string(activeC->getId()) +
                       " motivation=" + std::to_string(static_cast<int>(activeC->getMotivation()));
        
        switch(activeC->getMotivation()) {
            case Motivation::Hungry:
                activeC->hungryBehavior(w, c, cIndex, gs);
                break;
            case Motivation::Thirsty:
                activeC->thirstyBehavior(w, c, cIndex);
                break;
            case Motivation::Amorous:
                activeC->amorousBehavior(w, c, cIndex, gs);
                break;
            case Motivation::Content:
                activeC->contentBehavior(w, c, cIndex);
                break;
            case Motivation::Tired:
                activeC->tiredBehavior(w, c, cIndex);
                break;
        }
        return false;
    }
}

void advanceSimulation(World& w, std::vector<Creature>& c, GeneralStats& gs,
                       const SimulationConfig& config) {
    g_lastAction = "updating environment tick cache";
    unsigned int currentTick = w.getCurrentTick();
    w.environment().updateTickCache(static_cast<int>(currentTick));
    
    g_lastAction = "rebuilding creature spatial index";
    w.rebuildCreatureIndex(c);
    
    g_lastAction = "updating world objects";
    w.updateAllObjects();
    
    g_lastAction = "updating scent layer";
    w.updateScentLayer();
    
    g_lastAction = "ticking corpses";
    w.tickCorpses();
    
    // Pre-pass: deposit breeding scents
    g_lastAction = "depositing breeding scents";
    for (auto& creature : c) {
        if (creature.getMotivation() == Motivation::Amorous) {
            creature.depositBreedingScent(w.getScentLayer(), currentTick);
        }
    }
    
    // Main creature loop
    g_lastAction = "processing creature turns";
    for (size_t i = 0; i < c.size(); ++i) {
        if (!c[i].isAlive()) continue;
        takeTurn(w, gs, c, static_cast<unsigned int>(i), config);
    }
    
    // Remove dead creatures
    g_lastAction = "removing dead creatures";
    c.erase(
        std::remove_if(c.begin(), c.end(),
            [](const Creature& creature) { return !creature.isAlive(); }),
        c.end()
    );
    
    gs.population = c.size();
    g_creatureCount = c.size();
}

//================================================================================
// Status Reporting
//================================================================================
void printStatus(int tick, const std::vector<Creature>& creatures, const GeneralStats& gs,
                 const SimulationConfig& config) {
    // Count motivations
    std::map<Motivation, int> motivationCounts;
    int stuckCount = 0;
    
    for (const auto& c : creatures) {
        motivationCounts[c.getMotivation()]++;
        // A creature might be "stuck" if it hasn't moved recently (heuristic)
    }
    
    std::cout << "[Tick " << std::setw(5) << tick << "] "
              << "Pop: " << std::setw(4) << creatures.size()
              << " | Deaths: OA=" << gs.deaths.oldAge
              << " ST=" << gs.deaths.starved
              << " DH=" << gs.deaths.dehydrated
              << " DC=" << gs.deaths.discomfort
              << " PR=" << gs.deaths.predator
              << " | Births: " << gs.births
              << std::endl;
    
    if (config.verbose) {
        std::cout << "         Motivations: "
                  << "Content=" << motivationCounts[Motivation::Content]
                  << " Hungry=" << motivationCounts[Motivation::Hungry]
                  << " Thirsty=" << motivationCounts[Motivation::Thirsty]
                  << " Amorous=" << motivationCounts[Motivation::Amorous]
                  << " Tired=" << motivationCounts[Motivation::Tired]
                  << std::endl;
    }
}

//================================================================================
// Main
//================================================================================
int main(int argc, char* argv[]) {
    // Install signal handlers
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    
    // Parse arguments
    SimulationConfig config = parseArgs(argc, argv);
    
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         ECOSIM HEADLESS SIMULATION RUNNER                  ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << "Configuration:\n";
    std::cout << "  Max ticks:     " << config.maxTicks << "\n";
    std::cout << "  Population:    " << config.population << "\n";
    std::cout << "  Seed:          " << config.seed << "\n";
    std::cout << "  Map size:      " << config.mapWidth << "x" << config.mapHeight << "\n";
    std::cout << "  Verbose:       " << (config.verbose ? "yes" : "no") << "\n";
    std::cout << "  Nav debug:     " << (config.navDebug ? "yes" : "no") << "\n";
    std::cout << "  Behavior debug:" << (config.behaviorDebug ? "yes" : "no") << "\n";
    std::cout << "────────────────────────────────────────────────────────────\n";
    
    // Configure logger
    Logger& logger = Logger::getInstance();
    LoggerConfig logConfig;
    logConfig.minLevel = config.verbose ? LogLevel::DEBUG : LogLevel::INFO;
    logConfig.flushMode = FlushMode::PERIODIC;
    logConfig.periodicFlushCount = 100;
    logConfig.consoleOutput = config.verbose;
    logConfig.fileOutput = false;
    logger.configure(logConfig);
    
    // Initialize genetics system
    std::cout << "[Headless] Initializing genetics system...\n";
    g_lastAction = "initializing gene registry";
    Creature::initializeGeneRegistry();
    
    // Generate world
    std::cout << "[Headless] Generating climate-based world (seed=" << config.seed << ")...\n";
    g_lastAction = "generating world";
    World world = initializeWorld(config);
    
    // Initialize plants
    std::cout << "[Headless] Adding genetics-based plants...\n";
    g_lastAction = "adding plants";
    addGeneticsPlants(world);
    
    // Plant warm-up period
    const int PLANT_WARMUP = 50;
    std::cout << "[Headless] Running plant warm-up (" << PLANT_WARMUP << " ticks)...\n";
    for (int i = 0; i < PLANT_WARMUP; ++i) {
        g_lastAction = "plant warmup tick " + std::to_string(i);
        world.updateAllObjects();
    }
    
    // Spawn creatures
    std::vector<Creature> creatures;
    Calendar calendar;
    std::cout << "[Headless] Populating world with " << config.population << " creatures...\n";
    g_lastAction = "populating world";
    populateWorldByBiome(world, creatures, config.population, config);
    g_creatureCount = creatures.size();
    
    std::cout << "────────────────────────────────────────────────────────────\n";
    std::cout << "[Headless] Starting simulation...\n\n";
    
    // Start timing
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Main simulation loop
    GeneralStats gs = { calendar, 0, 0, 0, 0 };
    
    for (int tick = 0; tick < config.maxTicks; ++tick) {
        g_currentTick = tick;
        logger.setCurrentTick(tick);
        
        // Reset per-tick stats
        gs = { calendar, static_cast<unsigned>(creatures.size()), 0, 0, 0 };
        
        // Advance simulation
        advanceSimulation(world, creatures, gs, config);
        
        // Status report
        if (tick % config.statusInterval == 0) {
            printStatus(tick, creatures, gs, config);
        }
        
        // Check for extinction
        if (creatures.empty()) {
            std::cout << "\n[Headless] EXTINCTION at tick " << tick << "!\n";
            break;
        }
        
        calendar++;
    }
    
    // End timing
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Final report
    std::cout << "\n────────────────────────────────────────────────────────────\n";
    std::cout << "[Headless] Simulation complete!\n";
    std::cout << "  Duration:       " << duration.count() << " ms\n";
    std::cout << "  Ticks/second:   " << std::fixed << std::setprecision(1)
              << (g_currentTick * 1000.0 / duration.count()) << "\n";
    std::cout << "  Final pop:      " << creatures.size() << "\n";
    std::cout << "  Total deaths:\n";
    std::cout << "    Old age:      " << gs.deaths.oldAge << "\n";
    std::cout << "    Starvation:   " << gs.deaths.starved << "\n";
    std::cout << "    Dehydration:  " << gs.deaths.dehydrated << "\n";
    std::cout << "    Discomfort:   " << gs.deaths.discomfort << "\n";
    std::cout << "    Predator:     " << gs.deaths.predator << "\n";
    std::cout << "  Total births:   " << gs.births << "\n";
    std::cout << "────────────────────────────────────────────────────────────\n";
    
    if (!creatures.empty()) {
        std::cout << "\n[Headless] SUCCESS - Simulation completed without crash!\n";
    }
    
    return 0;
}
