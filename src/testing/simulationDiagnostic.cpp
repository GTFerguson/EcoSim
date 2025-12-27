/**
 * Simulation Diagnostic Runner
 * 
 * Runs the EcoSim simulation in headless mode (no GUI) for diagnostic purposes.
 * Outputs detailed logging to both console and file, then prints analysis summaries.
 * 
 * Usage:
 *   ./SimDiagnostic [tick_count] [log_file]
 * 
 * Arguments:
 *   tick_count - Number of simulation ticks to run (default: 1000)
 *   log_file   - Path to output log file (default: simulation_diagnostic.csv)
 * 
 * Example:
 *   ./SimDiagnostic 5000 my_simulation.csv
 */

#include "../../include/logging/Logger.hpp"
#include "../../include/objects/creature/creature.hpp"
#include "../../include/objects/food.hpp"
#include "../../include/objects/spawner.hpp"
#include "../../include/world/world.hpp"
#include "../../include/calendar.hpp"
#include "../../include/statistics/statistics.hpp"

// Genetics system
#include "../../include/genetics/defaults/UniversalGenes.hpp"
#include "../../include/genetics/organisms/PlantFactory.hpp"
#include "../../include/genetics/organisms/CreatureFactory.hpp"

#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace logging;

//================================================================================
//  Simulation Constants (matching main.cpp)
//================================================================================
const static unsigned INITIAL_POPULATION      = 200;
const static unsigned MAP_ROWS                = 500;
const static unsigned MAP_COLS                = 500;
const static double   WORLD_DEFAULT_SCALE     = 0.0035;
const static double   WORLD_DEFAULT_FREQUENCY = 4;
const static double   WORLD_DEFAULT_EXPONENT  = 0.8;
const static unsigned WORLD_DEFAULT_TERRACES  = 64;

// Food constants
const static float    APPLE_CALS              = 1.5f;
const static unsigned APPLE_LIFESPAN          = 1000;
const static unsigned APPLE_RATE              = 1500;
const static unsigned APPLE_MIN_RANGE         = 2;
const static unsigned APPLE_MAX_RANGE         = 4;
const static unsigned APPLE_MIN_ALTITUDE      = 180;
const static unsigned APPLE_MAX_ALTITUDE      = 195;
const static float    BANANA_CALS             = 2.0f;
const static unsigned BANANA_LIFESPAN         = 500;
const static unsigned BANANA_RATE             = 1000;
const static unsigned BANANA_MIN_RANGE        = 2;
const static unsigned BANANA_MAX_RANGE        = 4;
const static unsigned BANANA_MIN_ALTITUDE     = 160;
const static unsigned BANANA_MAX_ALTITUDE     = 170;
const static float    CORPSE_CALS             = 1.0f;
const static unsigned CORPSE_LIFESPAN         = 2000;

// Genetics plant constants
const static unsigned GRASS_MIN_ALTITUDE      = 165;
const static unsigned GRASS_MAX_ALTITUDE      = 200;
const static unsigned GRASS_SPAWN_RATE        = 5;
const static unsigned BERRY_MIN_ALTITUDE      = 170;
const static unsigned BERRY_MAX_ALTITUDE      = 190;
const static unsigned BERRY_SPAWN_RATE        = 3;
const static unsigned OAK_MIN_ALTITUDE        = 175;
const static unsigned OAK_MAX_ALTITUDE        = 195;
const static unsigned OAK_SPAWN_RATE          = 2;
const static unsigned THORN_MIN_ALTITUDE      = 160;
const static unsigned THORN_MAX_ALTITUDE      = 175;
const static unsigned THORN_SPAWN_RATE        = 2;

//================================================================================
//  Random Number Generator
//================================================================================
static std::mt19937 s_gen(std::random_device{}());

double randSeed() {
    std::uniform_real_distribution<> dis(0, 10);
    return dis(s_gen);
}

//================================================================================
//  Simulation Helpers
//================================================================================

/**
 * Takes a turn for a single creature with logging
 */
void takeTurnWithLogging(World& w, GeneralStats& gs, vector<Creature>& c,
                         const unsigned int& cIndex, Logger& logger) {
    Creature* activeC = &c.at(cIndex);
    int creatureId = static_cast<int>(cIndex);  // Use index as ID for logging
    
    short dc = activeC->deathCheck();
    if (dc != 0) {
        // Determine death cause
        std::string cause;
        switch (dc) {
            case 1: cause = "old_age"; gs.deaths.oldAge++; break;
            case 2: cause = "starvation"; gs.deaths.starved++; break;
            case 3: cause = "dehydration"; gs.deaths.dehydrated++; break;
            case 4: cause = "discomfort"; gs.deaths.discomfort++; break;
            default: cause = "unknown"; break;
        }
        
        // Log death
        logger.creatureDied(creatureId, "creature", cause, 
                           activeC->getHunger(), activeC->getAge());
        
        // Create corpse
        float calories = CORPSE_CALS + activeC->getHunger();
        if (calories > 0.0f) {
            Food corpse(2, "Corpse", "Dead animal", true, 'c', 1, calories, CORPSE_LIFESPAN);
            w.addFood(activeC->getX(), activeC->getY(), corpse);
        }
        
        c.erase(c.begin() + cIndex);
    } else {
        activeC->update();
        
        switch(activeC->getMotivation()) {
            case Motivation::Content:   activeC->contentBehavior(w, c, cIndex); break;
            case Motivation::Hungry:    activeC->hungryBehavior(w, c, cIndex, gs); break;
            case Motivation::Thirsty:   activeC->thirstyBehavior(w, c, cIndex); break;
            case Motivation::Amorous:   activeC->amorousBehavior(w, c, cIndex, gs); break;
            case Motivation::Tired:     break;
        }
    }
}

/**
 * Advances simulation one tick with logging
 */
void advanceSimulationWithLogging(World& w, vector<Creature>& c, GeneralStats& gs, Logger& logger) {
    w.updateAllObjects();
    
    for (int i = static_cast<int>(c.size()) - 1; i >= 0; i--) {
        takeTurnWithLogging(w, gs, c, i, logger);
    }
    
    gs.population = c.size();
}

/**
 * Initialize world with default parameters
 */
World initializeWorld() {
    double seed = randSeed();
    
    MapGen mg { seed,
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

/**
 * Populate world with creatures using CreatureFactory
 */
void populateWorld(World& w, vector<Creature>& c, unsigned amount) {
    std::uniform_int_distribution<int> colDis(0, MAP_COLS - 1);
    std::uniform_int_distribution<int> rowDis(0, MAP_ROWS - 1);
    const unsigned MAX_ATTEMPTS = 10000;
    
    auto registry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
    EcoSim::Genetics::CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Population distribution
    unsigned grazers = static_cast<unsigned>(amount * 0.40);
    unsigned browsers = static_cast<unsigned>(amount * 0.25);
    unsigned hunters = static_cast<unsigned>(amount * 0.10);
    unsigned foragers = static_cast<unsigned>(amount * 0.10);
    unsigned scavengers = static_cast<unsigned>(amount * 0.05);
    unsigned randoms = amount - grazers - browsers - hunters - foragers - scavengers;
    
    std::vector<std::pair<std::string, unsigned>> spawnOrder = {
        {"grazer", grazers},
        {"browser", browsers},
        {"hunter", hunters},
        {"forager", foragers},
        {"scavenger", scavengers},
        {"", randoms}
    };
    
    Logger& logger = Logger::getInstance();
    
    for (const auto& [templateName, count] : spawnOrder) {
        for (unsigned i = 0; i < count; i++) {
            int x, y;
            unsigned attempts = 0;
            do {
                x = colDis(s_gen);
                y = rowDis(s_gen);
                if (++attempts > MAX_ATTEMPTS) {
                    std::cerr << "[Diagnostic] Warning: Could not find passable tile" << std::endl;
                    return;
                }
            } while (!w.getGrid().at(x).at(y).isPassable());
            
            Creature newC = templateName.empty()
                ? factory.createRandom(x, y)
                : factory.createFromTemplate(templateName, x, y);
            
            // Log creature birth
            logger.creatureBorn(static_cast<int>(c.size()), 
                               templateName.empty() ? "random" : templateName, 
                               -1, -1);  // No parents for initial population
            
            c.push_back(std::move(newC));
        }
    }
}

/**
 * Add food spawners to world
 */
void addFoodSpawners(World& w) {
    Food banana(0, "Banana", "A curved yellow fruit", true, ')', 1,
                BANANA_CALS, BANANA_LIFESPAN);
    Food apple(1, "Apple", "A delicious red apple", true, '*', 1,
               APPLE_CALS, APPLE_LIFESPAN);
    
    Spawner bananaPlant("Banana Plant", "A tall plant that makes bananas",
                        true, 'T', 13, BANANA_RATE, BANANA_MIN_RANGE,
                        BANANA_MAX_RANGE, banana);
    Spawner appleTree("Apple Tree", "A big tree that makes apples",
                      true, '^', 13, APPLE_RATE, APPLE_MIN_RANGE,
                      APPLE_MAX_RANGE, apple);
    
    w.addTrees(APPLE_MIN_ALTITUDE, APPLE_MAX_ALTITUDE, 2, appleTree);
    w.addTrees(BANANA_MIN_ALTITUDE, BANANA_MAX_ALTITUDE, 2, bananaPlant);
}

/**
 * Add genetics-based plants
 */
void addGeneticsPlants(World& w) {
    w.initializeGeneticsPlants();
    
    w.addGeneticsPlants(GRASS_MIN_ALTITUDE, GRASS_MAX_ALTITUDE,
                        GRASS_SPAWN_RATE, "grass");
    w.addGeneticsPlants(BERRY_MIN_ALTITUDE, BERRY_MAX_ALTITUDE,
                        BERRY_SPAWN_RATE, "berry_bush");
    w.addGeneticsPlants(OAK_MIN_ALTITUDE, OAK_MAX_ALTITUDE,
                        OAK_SPAWN_RATE, "oak_tree");
    w.addGeneticsPlants(THORN_MIN_ALTITUDE, THORN_MAX_ALTITUDE,
                        THORN_SPAWN_RATE, "thorn_bush");
}

/**
 * Print progress bar
 */
void printProgress(int current, int total, int barWidth = 50) {
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(barWidth * progress);
    
    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) 
              << (progress * 100.0) << "% (tick " << current << "/" << total << ")";
    std::cout.flush();
}

//================================================================================
//  Main Diagnostic Runner
//================================================================================
int main(int argc, char* argv[]) {
    // Parse command line arguments
    int tickCount = 1000;
    std::string logFile = "simulation_diagnostic.csv";
    
    if (argc >= 2) {
        tickCount = std::atoi(argv[1]);
        if (tickCount <= 0) {
            std::cerr << "Invalid tick count. Using default: 1000" << std::endl;
            tickCount = 1000;
        }
    }
    
    if (argc >= 3) {
        logFile = argv[2];
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "  EcoSim Diagnostic Runner" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Ticks to run: " << tickCount << std::endl;
    std::cout << "Log file: " << logFile << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Configure logger
    Logger& logger = Logger::getInstance();
    LoggerConfig config;
    config.minLevel = LogLevel::INFO;
    config.flushMode = FlushMode::PERIODIC;
    config.periodicFlushCount = 100;
    config.consoleOutput = false;  // Disable console output during run
    config.fileOutput = true;
    config.logFilePath = logFile;
    config.csvFormat = true;
    logger.configure(config);
    
    // Initialize creature gene registry
    std::cout << "[Diagnostic] Initializing genetics system..." << std::endl;
    Creature::initializeGeneRegistry();
    
    // Initialize world
    std::cout << "[Diagnostic] Generating world..." << std::endl;
    World w = initializeWorld();
    
    // Create creatures and food
    std::vector<Creature> creatures;
    Calendar calendar;
    
    std::cout << "[Diagnostic] Adding food spawners..." << std::endl;
    addFoodSpawners(w);
    
    std::cout << "[Diagnostic] Adding genetics-based plants..." << std::endl;
    addGeneticsPlants(w);
    
    // GENETICS-MIGRATION: Plants need time to grow and produce food before creatures spawn
    // Run plant updates for a warm-up period to let them mature
    const int PLANT_WARMUP_TICKS = 100;
    std::cout << "[Diagnostic] Running plant warm-up period (" << PLANT_WARMUP_TICKS << " ticks)..." << std::endl;
    
    for (int warmupTick = 0; warmupTick < PLANT_WARMUP_TICKS; warmupTick++) {
        w.updateGeneticsPlants();
        if (warmupTick % 20 == 0) {
            std::cout << "  Plant warmup: " << warmupTick << "/" << PLANT_WARMUP_TICKS << std::endl;
        }
    }
    
    std::cout << "[Diagnostic] Populating world with " << INITIAL_POPULATION << " creatures..." << std::endl;
    populateWorld(w, creatures, INITIAL_POPULATION);
    
    // Count initial food
    int initialFood = 0;
    const auto& grid = w.getGrid();
    for (const auto& col : grid) {
        for (const auto& tile : col) {
            initialFood += tile.getFoodVec().size();
        }
    }
    
    std::cout << "[Diagnostic] Initial state:" << std::endl;
    std::cout << "  - Creatures: " << creatures.size() << std::endl;
    std::cout << "  - Food items: " << initialFood << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[Diagnostic] Running simulation..." << std::endl;
    
    // Record start time
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Run simulation
    GeneralStats gs = { calendar, 0, 0, 0, 0 };
    int snapshotInterval = tickCount / 20;  // 20 snapshots during run
    if (snapshotInterval < 1) snapshotInterval = 1;
    
    for (int tick = 0; tick < tickCount; tick++) {
        logger.setCurrentTick(tick);
        
        // Reset stats for this tick
        gs = { calendar, 0, 0, 0, 0 };
        
        // Advance simulation
        advanceSimulationWithLogging(w, creatures, gs, logger);
        
        // Take population snapshot at intervals
        if (tick % snapshotInterval == 0) {
            int foodCount = 0;
            const auto& gridRef = w.getGrid();
            for (const auto& col : gridRef) {
                for (const auto& tile : col) {
                    foodCount += tile.getFoodVec().size();
                }
            }
            logger.populationSnapshot(tick, creatures.size(), 0, foodCount);
        }
        
        // Check for extinction
        if (creatures.empty()) {
            logger.extinction("all_creatures");
            std::cout << std::endl;
            std::cout << "[Diagnostic] EXTINCTION EVENT at tick " << tick << std::endl;
            break;
        }
        
        // Print progress
        if (tick % 10 == 0) {
            printProgress(tick, tickCount);
        }
        
        // Advance calendar
        calendar++;
        
        // End of tick
        logger.onTickEnd();
    }
    
    printProgress(tickCount, tickCount);
    std::cout << std::endl;
    
    // Record end time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Flush remaining logs
    logger.flush();
    
    // Final statistics
    int finalFood = 0;
    const auto& finalGrid = w.getGrid();
    for (const auto& col : finalGrid) {
        for (const auto& tile : col) {
            finalFood += tile.getFoodVec().size();
        }
    }
    
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[Diagnostic] Simulation complete!" << std::endl;
    std::cout << "  - Duration: " << duration.count() << "ms" << std::endl;
    std::cout << "  - Ticks/second: " << std::fixed << std::setprecision(1) 
              << (tickCount * 1000.0 / duration.count()) << std::endl;
    std::cout << "  - Final creatures: " << creatures.size() << std::endl;
    std::cout << "  - Final food: " << finalFood << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Print analysis summaries
    std::cout << std::endl;
    logger.printDeathSummary();
    logger.printPopulationHistory();
    logger.printFeedingStats();
    
    std::cout << "========================================" << std::endl;
    std::cout << "Log file saved to: " << logFile << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
