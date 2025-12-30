/**
 * Breeding Diagnostic Runner
 * 
 * Runs the EcoSim simulation in headless mode (no GUI) specifically to diagnose
 * breeding behavior issues. Tracks detailed breeding-related data including:
 * - Count of creatures in breeding state per tick
 * - Mate-seeking behavior and results
 * - Breeding attempts and outcomes
 * - Key variable values (_mate, getTMate(), etc.)
 * 
 * Usage:
 *   ./BreedingDiagnostic [tick_count] [csv_file]
 * 
 * Arguments:
 *   tick_count - Number of simulation ticks to run (default: 1000)
 *   csv_file   - Path to output CSV file (default: breeding_diagnostic.csv)
 * 
 * Example:
 *   ./BreedingDiagnostic 2000 my_breeding.csv
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
#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <cmath>

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
//  Breeding Diagnostic Data Structure
//================================================================================
struct BreedingDiagnosticData {
    int tick;
    int totalPopulation;
    int creaturesInBreedState;
    int creaturesSeekingMate;  // Those who actually call findMate()
    int matingAttempts;
    int successfulBirths;
    float avgMateValueBreeding;  // Average _mate value for creatures in breed state
    float avgThresholdValue;     // Average getTMate() threshold
    int mateFoundCount;          // How many findMate() calls found a mate
    float minDistanceToMate;     // Minimum distance to potential mate
    float avgDistanceToMate;     // Average distance to potential mates
    std::string noMateReasons;   // Aggregated reasons why no mate found
    
    // Additional diagnostic fields
    int creaturesAboveThreshold;  // _mate > getTMate()
    int creaturesHungry;          // In hungry state
    int creaturesThirsty;         // In thirsty state
    float avgHunger;
    float avgThirst;
};

//================================================================================
//  Breeding Diagnostic Tracker
//================================================================================
class BreedingDiagnosticTracker {
private:
    std::vector<BreedingDiagnosticData> m_history;
    std::ofstream m_csvFile;
    int m_currentTick = 0;
    
    // Per-tick accumulators (reset each tick)
    int m_matingAttempts = 0;
    int m_birthsThisTick = 0;
    int m_mateFoundCount = 0;
    std::map<std::string, int> m_noMateReasons;

public:
    BreedingDiagnosticTracker() = default;
    
    bool openCsvFile(const std::string& path) {
        m_csvFile.open(path, std::ios::out | std::ios::trunc);
        if (m_csvFile.is_open()) {
            // Write CSV header
            m_csvFile << "tick,population,creatures_in_breed_state,creatures_seeking_mate,"
                      << "mating_attempts,successful_births,avg_mate_value_breeding,"
                      << "avg_threshold_value,mate_found_count,min_distance_to_mate,"
                      << "avg_distance_to_mate,creatures_above_threshold,creatures_hungry,"
                      << "creatures_thirsty,avg_hunger,avg_thirst,reason_no_mate\n";
            return true;
        }
        return false;
    }
    
    void setCurrentTick(int tick) {
        m_currentTick = tick;
    }
    
    void resetTickAccumulators() {
        m_matingAttempts = 0;
        m_birthsThisTick = 0;
        m_mateFoundCount = 0;
        m_noMateReasons.clear();
    }
    
    void recordMatingAttempt(bool foundMate, const std::string& reason) {
        m_matingAttempts++;
        if (foundMate) {
            m_mateFoundCount++;
        } else if (!reason.empty()) {
            m_noMateReasons[reason]++;
        }
    }
    
    void recordBirth() {
        m_birthsThisTick++;
    }
    
    /**
     * Analyze all creatures and record breeding diagnostic data for this tick
     */
    BreedingDiagnosticData analyzeCreatures(const std::vector<Creature>& creatures, int tick) {
        BreedingDiagnosticData data;
        data.tick = tick;
        data.totalPopulation = creatures.size();
        data.creaturesInBreedState = 0;
        data.creaturesSeekingMate = 0;
        data.avgMateValueBreeding = 0.0f;
        data.avgThresholdValue = 0.0f;
        data.minDistanceToMate = std::numeric_limits<float>::max();
        data.avgDistanceToMate = 0.0f;
        data.creaturesAboveThreshold = 0;
        data.creaturesHungry = 0;
        data.creaturesThirsty = 0;
        data.avgHunger = 0.0f;
        data.avgThirst = 0.0f;
        
        float totalMateValue = 0.0f;
        float totalThreshold = 0.0f;
        float totalHunger = 0.0f;
        float totalThirst = 0.0f;
        int breedingCreatureCount = 0;
        int distanceCount = 0;
        float totalDistance = 0.0f;
        
        // First pass: count states and gather statistics
        std::vector<const Creature*> breedingCreatures;
        
        for (const auto& creature : creatures) {
            totalHunger += creature.getHunger();
            totalThirst += creature.getThirst();
            
            Motivation motivation = creature.getMotivation();
            
            switch (motivation) {
                case Motivation::Amorous:
                    data.creaturesInBreedState++;
                    breedingCreatures.push_back(&creature);
                    totalMateValue += creature.getMate();
                    totalThreshold += creature.getTMate();
                    breedingCreatureCount++;
                    break;
                case Motivation::Hungry:
                    data.creaturesHungry++;
                    break;
                case Motivation::Thirsty:
                    data.creaturesThirsty++;
                    break;
                default:
                    break;
            }
            
            // Check if _mate is above threshold
            if (creature.getMate() > creature.getTMate()) {
                data.creaturesAboveThreshold++;
            }
        }
        
        // Second pass: calculate distances between ALL breeding creatures
        // GENETICS-MIGRATION: Removed diet filtering since findMate() no longer
        // requires same diet. Mate compatibility is now determined by genetic
        // similarity via checkFitness() when creatures actually meet.
        for (size_t i = 0; i < breedingCreatures.size(); i++) {
            const Creature* c1 = breedingCreatures[i];
            
            for (size_t j = i + 1; j < breedingCreatures.size(); j++) {
                const Creature* c2 = breedingCreatures[j];
                
                // Calculate distance between ALL breeding creatures (no diet filter)
                float dx = static_cast<float>(c1->getX() - c2->getX());
                float dy = static_cast<float>(c1->getY() - c2->getY());
                float distance = std::sqrt(dx * dx + dy * dy);
                
                totalDistance += distance;
                distanceCount++;
                
                if (distance < data.minDistanceToMate) {
                    data.minDistanceToMate = distance;
                }
            }
        }
        
        // Calculate averages
        if (creatures.size() > 0) {
            data.avgHunger = totalHunger / creatures.size();
            data.avgThirst = totalThirst / creatures.size();
        }
        
        if (breedingCreatureCount > 0) {
            data.avgMateValueBreeding = totalMateValue / breedingCreatureCount;
            data.avgThresholdValue = totalThreshold / breedingCreatureCount;
        }
        
        if (distanceCount > 0) {
            data.avgDistanceToMate = totalDistance / distanceCount;
        } else {
            data.minDistanceToMate = -1.0f;  // No potential mates
            data.avgDistanceToMate = -1.0f;
        }
        
        // Copy accumulated data
        data.matingAttempts = m_matingAttempts;
        data.successfulBirths = m_birthsThisTick;
        data.mateFoundCount = m_mateFoundCount;
        data.creaturesSeekingMate = m_matingAttempts;  // Each attempt = 1 creature seeking
        
        // Aggregate no-mate reasons
        std::ostringstream reasons;
        for (const auto& [reason, count] : m_noMateReasons) {
            if (reasons.tellp() > 0) reasons << ";";
            reasons << reason << ":" << count;
        }
        data.noMateReasons = reasons.str().empty() ? "none" : reasons.str();
        
        return data;
    }
    
    void recordSnapshot(const BreedingDiagnosticData& data) {
        m_history.push_back(data);
        
        // Write to CSV
        if (m_csvFile.is_open()) {
            m_csvFile << data.tick << ","
                      << data.totalPopulation << ","
                      << data.creaturesInBreedState << ","
                      << data.creaturesSeekingMate << ","
                      << data.matingAttempts << ","
                      << data.successfulBirths << ","
                      << std::fixed << std::setprecision(3) << data.avgMateValueBreeding << ","
                      << std::fixed << std::setprecision(3) << data.avgThresholdValue << ","
                      << data.mateFoundCount << ","
                      << std::fixed << std::setprecision(2) << data.minDistanceToMate << ","
                      << std::fixed << std::setprecision(2) << data.avgDistanceToMate << ","
                      << data.creaturesAboveThreshold << ","
                      << data.creaturesHungry << ","
                      << data.creaturesThirsty << ","
                      << std::fixed << std::setprecision(3) << data.avgHunger << ","
                      << std::fixed << std::setprecision(3) << data.avgThirst << ","
                      << "\"" << data.noMateReasons << "\"\n";
        }
    }
    
    void printSummary() {
        std::cout << "\n========== BREEDING DIAGNOSTIC SUMMARY ==========\n";
        
        if (m_history.empty()) {
            std::cout << "No data collected.\n";
            return;
        }
        
        // Calculate aggregate statistics
        int totalBreedingCreatureTicks = 0;
        int totalMatingAttempts = 0;
        int totalBirths = 0;
        int totalMatesFound = 0;
        int ticksWithBreedingCreatures = 0;
        int ticksWithNoMates = 0;
        float totalAvgMateValue = 0.0f;
        float totalAvgThreshold = 0.0f;
        int validMateValueCount = 0;
        
        for (const auto& data : m_history) {
            totalBreedingCreatureTicks += data.creaturesInBreedState;
            totalMatingAttempts += data.matingAttempts;
            totalBirths += data.successfulBirths;
            totalMatesFound += data.mateFoundCount;
            
            if (data.creaturesInBreedState > 0) {
                ticksWithBreedingCreatures++;
                totalAvgMateValue += data.avgMateValueBreeding;
                totalAvgThreshold += data.avgThresholdValue;
                validMateValueCount++;
            }
            
            if (data.creaturesInBreedState > 0 && data.mateFoundCount == 0) {
                ticksWithNoMates++;
            }
        }
        
        std::cout << "Simulation ran for " << m_history.size() << " ticks\n\n";
        
        std::cout << "--- Population in Breed State ---\n";
        std::cout << "Total creature-ticks in breed state: " << totalBreedingCreatureTicks << "\n";
        std::cout << "Ticks with creatures in breed state: " << ticksWithBreedingCreatures << "\n";
        std::cout << "Ticks with NO mates found: " << ticksWithNoMates << "\n";
        
        if (validMateValueCount > 0) {
            std::cout << "Average _mate value (breeding creatures): " 
                      << std::fixed << std::setprecision(3) 
                      << (totalAvgMateValue / validMateValueCount) << "\n";
            std::cout << "Average getTMate() threshold: " 
                      << std::fixed << std::setprecision(3) 
                      << (totalAvgThreshold / validMateValueCount) << "\n";
        }
        
        std::cout << "\n--- Mating Results ---\n";
        std::cout << "Total mating attempts (findMate calls): " << totalMatingAttempts << "\n";
        std::cout << "Total mates found: " << totalMatesFound << "\n";
        std::cout << "Total successful births: " << totalBirths << "\n";
        
        if (totalMatingAttempts > 0) {
            float mateFoundRate = 100.0f * totalMatesFound / totalMatingAttempts;
            std::cout << "Mate found rate: " << std::fixed << std::setprecision(1) 
                      << mateFoundRate << "%\n";
        }
        
        // Analyze no-mate reasons across all ticks
        std::map<std::string, int> aggregatedReasons;
        for (const auto& data : m_history) {
            // Parse the reasons string
            std::istringstream iss(data.noMateReasons);
            std::string token;
            while (std::getline(iss, token, ';')) {
                size_t colonPos = token.find(':');
                if (colonPos != std::string::npos) {
                    std::string reason = token.substr(0, colonPos);
                    int count = std::stoi(token.substr(colonPos + 1));
                    aggregatedReasons[reason] += count;
                }
            }
        }
        
        if (!aggregatedReasons.empty()) {
            std::cout << "\n--- Reasons No Mate Found ---\n";
            for (const auto& [reason, count] : aggregatedReasons) {
                std::cout << "  " << reason << ": " << count << "\n";
            }
        }
        
        // Distance analysis
        std::cout << "\n--- Distance to Potential Mates ---\n";
        float totalMinDist = 0.0f;
        float totalAvgDist = 0.0f;
        int validDistCount = 0;
        
        for (const auto& data : m_history) {
            if (data.minDistanceToMate >= 0) {
                totalMinDist += data.minDistanceToMate;
                totalAvgDist += data.avgDistanceToMate;
                validDistCount++;
            }
        }
        
        if (validDistCount > 0) {
            std::cout << "Average minimum distance: " << std::fixed << std::setprecision(2)
                      << (totalMinDist / validDistCount) << "\n";
            std::cout << "Average mean distance: " << std::fixed << std::setprecision(2)
                      << (totalAvgDist / validDistCount) << "\n";
        } else {
            std::cout << "No ticks with multiple breeding creatures\n";
        }
        
        std::cout << "================================================\n\n";
    }
    
    void close() {
        if (m_csvFile.is_open()) {
            m_csvFile.close();
        }
    }
    
    const std::vector<BreedingDiagnosticData>& getHistory() const {
        return m_history;
    }
};

// Global diagnostic tracker
static BreedingDiagnosticTracker g_breedingTracker;

//================================================================================
//  Simulation Helpers
//================================================================================

/**
 * Takes a turn for a single creature with breeding diagnostics
 */
void takeTurnWithBreedingDiagnostics(World& w, GeneralStats& gs, vector<Creature>& c,
                                     const unsigned int& cIndex) {
    Creature* activeC = &c.at(cIndex);
    
    short dc = activeC->deathCheck();
    if (dc != 0) {
        // Create corpse
        float calories = CORPSE_CALS + activeC->getHunger();
        if (calories > 0.0f) {
            Food corpse(2, "Corpse", "Dead animal", true, 'c', 1, calories, CORPSE_LIFESPAN);
            w.addFood(activeC->getX(), activeC->getY(), corpse);
        }
        
        c.erase(c.begin() + cIndex);
    } else {
        activeC->update();
        
        unsigned birthsBefore = gs.births;
        
        switch(activeC->getMotivation()) {
            case Motivation::Content:   activeC->contentBehavior(w, c, cIndex); break;
            case Motivation::Hungry:    activeC->hungryBehavior(w, c, cIndex, gs); break;
            case Motivation::Thirsty:   activeC->thirstyBehavior(w, c, cIndex); break;
            case Motivation::Amorous:
                {
                    // Use amorousBehavior which includes scent-following (Phase 2: Gradient Navigation)
                    // This will:
                    // 1. Deposit breeding scent
                    // 2. Try visual mate finding via findMate()
                    // 3. If visual fails, try scent-based navigation via detectMateDirection()
                    // 4. Fall back to wandering if no scent found
                    activeC->amorousBehavior(w, c, cIndex, gs);
                    
                    // Check if birth occurred (by comparing birth count)
                    bool hadBirth = (gs.births > birthsBefore);
                    
                    // Determine reason for not finding mate (for diagnostic tracking)
                    // GENETICS-MIGRATION: Removed diet filtering - any breeding creature is a potential mate
                    std::string reason = "unknown";
                    if (!hadBirth) {
                        // Check potential reasons
                        int inBreedStateCount = 0;
                        int inSightRangeCount = 0;
                        int inAdjacentCount = 0;
                        unsigned sightRange = activeC->getSightRange();
                        
                        for (const auto& other : c) {
                            if (&other == activeC) continue;
                            
                            // Count ALL breeding creatures, not just same diet
                            if (other.getMotivation() == Motivation::Amorous) {
                                inBreedStateCount++;
                                
                                unsigned diffX = abs(activeC->getX() - other.getX());
                                unsigned diffY = abs(activeC->getY() - other.getY());
                                
                                // Check if within sight range (for findMate visual detection)
                                if (diffX < sightRange && diffY < sightRange) {
                                    inSightRangeCount++;
                                }
                                
                                // Check if adjacent (for breeding)
                                if (diffX <= 1 && diffY <= 1) {
                                    inAdjacentCount++;
                                }
                            }
                        }
                        
                        if (inBreedStateCount == 0) {
                            reason = "no_other_breeding_creatures";
                        } else if (inAdjacentCount > 0) {
                            reason = "adjacent_but_not_breeding";
                        } else if (inSightRangeCount > 0) {
                            reason = "in_sight_navigating";
                        } else {
                            reason = "following_scent_trail";
                        }
                    }
                    
                    g_breedingTracker.recordMatingAttempt(hadBirth, reason);
                    
                    if (hadBirth) {
                        g_breedingTracker.recordBirth();
                    }
                }
                break;
            case Motivation::Tired:     break;
        }
    }
}

/**
 * Advances simulation one tick with breeding diagnostics
 */
void advanceSimulationWithBreedingDiagnostics(World& w, vector<Creature>& c, GeneralStats& gs) {
    w.updateAllObjects();
    
    // Update scent layer for pheromone decay (Phase 2: Sensory System)
    w.updateScentLayer();
    
    // PRE-PASS: Have ALL breeding creatures deposit scents BEFORE any creature acts
    // This ensures scents from all potential mates are available during detection
    unsigned int currentTick = w.getCurrentTick();
    for (auto& creature : c) {
        if (creature.getMotivation() == Motivation::Amorous) {
            creature.depositBreedingScent(w.getScentLayer(), currentTick);
        }
    }
    
    for (int i = static_cast<int>(c.size()) - 1; i >= 0; i--) {
        takeTurnWithBreedingDiagnostics(w, gs, c, i);
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
    w.plants().initialize();
    
    w.plants().addPlants(GRASS_MIN_ALTITUDE, GRASS_MAX_ALTITUDE,
                        GRASS_SPAWN_RATE, "grass");
    w.plants().addPlants(BERRY_MIN_ALTITUDE, BERRY_MAX_ALTITUDE,
                        BERRY_SPAWN_RATE, "berry_bush");
    w.plants().addPlants(OAK_MIN_ALTITUDE, OAK_MAX_ALTITUDE,
                        OAK_SPAWN_RATE, "oak_tree");
    w.plants().addPlants(THORN_MIN_ALTITUDE, THORN_MAX_ALTITUDE,
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
    std::string csvFile = "breeding_diagnostic.csv";
    
    if (argc >= 2) {
        tickCount = std::atoi(argv[1]);
        if (tickCount <= 0) {
            std::cerr << "Invalid tick count. Using default: 1000" << std::endl;
            tickCount = 1000;
        }
    }
    
    if (argc >= 3) {
        csvFile = argv[2];
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "  EcoSim Breeding Diagnostic Runner" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Ticks to run: " << tickCount << std::endl;
    std::cout << "CSV output: " << csvFile << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Open CSV file
    if (!g_breedingTracker.openCsvFile(csvFile)) {
        std::cerr << "Failed to open CSV file: " << csvFile << std::endl;
        return 1;
    }
    
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
    
    // Plant warm-up period
    const int PLANT_WARMUP_TICKS = 100;
    std::cout << "[Diagnostic] Running plant warm-up period (" << PLANT_WARMUP_TICKS << " ticks)..." << std::endl;
    
    for (int warmupTick = 0; warmupTick < PLANT_WARMUP_TICKS; warmupTick++) {
        w.plants().tick(static_cast<unsigned>(warmupTick));
        if (warmupTick % 20 == 0) {
            std::cout << "  Plant warmup: " << warmupTick << "/" << PLANT_WARMUP_TICKS << std::endl;
        }
    }
    
    std::cout << "[Diagnostic] Populating world with " << INITIAL_POPULATION << " creatures..." << std::endl;
    populateWorld(w, creatures, INITIAL_POPULATION);
    
    std::cout << "[Diagnostic] Initial state:" << std::endl;
    std::cout << "  - Creatures: " << creatures.size() << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[Diagnostic] Running simulation with breeding diagnostics..." << std::endl;
    
    // Record start time
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Run simulation
    GeneralStats gs = { calendar, 0, 0, 0, 0 };
    int snapshotInterval = 1;  // Record every tick for detailed diagnostics
    
    for (int tick = 0; tick < tickCount; tick++) {
        g_breedingTracker.setCurrentTick(tick);
        g_breedingTracker.resetTickAccumulators();
        
        // Reset per-tick stats
        gs = { calendar, 0, 0, 0, 0 };
        
        // Advance simulation
        advanceSimulationWithBreedingDiagnostics(w, creatures, gs);
        
        // Record breeding snapshot
        if (tick % snapshotInterval == 0) {
            BreedingDiagnosticData data = g_breedingTracker.analyzeCreatures(creatures, tick);
            g_breedingTracker.recordSnapshot(data);
        }
        
        // Check for extinction
        if (creatures.empty()) {
            std::cout << std::endl;
            std::cout << "[Diagnostic] EXTINCTION EVENT at tick " << tick << std::endl;
            break;
        }
        
        // Print progress every 10 ticks
        if (tick % 10 == 0) {
            printProgress(tick, tickCount);
        }
        
        // Advance calendar
        calendar++;
    }
    
    printProgress(tickCount, tickCount);
    std::cout << std::endl;
    
    // Record end time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Close CSV file
    g_breedingTracker.close();
    
    // Final statistics
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[Diagnostic] Simulation complete!" << std::endl;
    std::cout << "  - Duration: " << duration.count() << "ms" << std::endl;
    std::cout << "  - Ticks/second: " << std::fixed << std::setprecision(1) 
              << (tickCount * 1000.0 / duration.count()) << std::endl;
    std::cout << "  - Final creatures: " << creatures.size() << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Print scent layer statistics
    std::cout << "\n--- Scent Layer Diagnostics ---" << std::endl;
    std::cout << "Active scent tiles: " << w.getScentLayer().getActiveTileCount() << std::endl;
    std::cout << "Total scent deposits: " << w.getScentLayer().getTotalScentCount() << std::endl;
    
    // Print breeding diagnostic summary
    g_breedingTracker.printSummary();
    
    std::cout << "========================================" << std::endl;
    std::cout << "CSV output saved to: " << csvFile << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
