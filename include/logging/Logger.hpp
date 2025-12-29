#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <chrono>
#include <mutex>

// Forward declarations for combat types
namespace EcoSim { namespace Genetics {
    enum class WeaponType;
    enum class CombatDamageType;
    enum class DefenseType;
}}

namespace logging {

/**
 * @brief Log levels for categorizing messages
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    CRITICAL = 4
};

/**
 * @brief Flush modes for controlling when logs are written
 */
enum class FlushMode {
    IMMEDIATE,   // Flush after every log entry
    PER_TICK,    // Flush at end of each simulation tick
    PERIODIC,    // Flush every N entries
    MANUAL       // Only flush when explicitly called
};

/**
 * @brief Combat log verbosity levels
 *
 * Controls how much detail is included in combat event logs.
 */
enum class CombatLogDetail {
    MINIMAL,    ///< Just damage amount: "#1→#2: 15.8 damage"
    STANDARD,   ///< + weapon, type, health: "Teeth 15.8 Pierce | HP:50→34"
    DETAILED,   ///< + raw damage, effectiveness, defense
    DEBUG       ///< Full multi-line output with all data
};

/**
 * @brief Configuration for the Logger
 */
struct LoggerConfig {
    LogLevel minLevel = LogLevel::INFO;
    FlushMode flushMode = FlushMode::PER_TICK;
    int periodicFlushCount = 100;
    bool consoleOutput = true;
    bool fileOutput = true;
    std::string logFilePath = "simulation_log.csv";
    bool csvFormat = true;
    CombatLogDetail combatDetail = CombatLogDetail::STANDARD;  ///< Combat log verbosity
};

/**
 * @brief Statistics tracking for death causes
 */
struct DeathStats {
    std::map<std::string, int> creatureDeathsByCause;
    std::map<std::string, int> plantDeathsByCause;
    std::map<std::string, int> creatureDeathsByType;
    std::map<std::string, int> plantDeathsBySpecies;
    int totalCreatureDeaths = 0;
    int totalPlantDeaths = 0;
};

/**
 * @brief Population snapshot data
 */
struct PopulationSnapshot {
    int tick;
    int creatures;
    int plants;
    int food;
};

/**
 * @brief Feeding statistics
 */
struct FeedingStats {
    int totalAttempts = 0;
    int successfulFeedings = 0;
    float totalNutritionGained = 0.0f;
    float totalDamageReceived = 0.0f;
    std::map<std::string, int> feedingsByCreatureType;
};

/**
 * @brief Breeding statistics for diagnostic tracking
 */
struct BreedingStats {
    int totalInBreedState = 0;
    int totalSeekingMate = 0;
    int totalMateFound = 0;
    int totalMatingAttempts = 0;
    int successfulBreedings = 0;
    int failedBreedings = 0;
    float totalMateValue = 0.0f;
    float totalThresholdValue = 0.0f;
    std::map<std::string, int> noMateReasons;
};

/**
 * @brief Per-tick breeding snapshot for diagnostic CSV output
 */
struct BreedingSnapshot {
    int tick;
    int creaturesInBreedState;
    int creaturesSeekingMate;
    int matingAttempts;
    int successfulBirths;
    float avgMateValue;
    int mateFoundCount;
    std::string noMateReason;
};

/**
 * @brief Structured combat event for detailed logging
 *
 * Contains all relevant data about a combat action including
 * attacker/defender info, weapon used, damage calculation details,
 * and health state before/after.
 */
struct CombatLogEvent {
    // === Combatant Identification ===
    int attackerId = -1;                    ///< Attacker creature ID
    int defenderId = -1;                    ///< Defender creature ID
    std::string attackerName;               ///< e.g., "ApexPredator_Alpha"
    std::string defenderName;               ///< e.g., "FleetRunner_Beta"
    
    // === Weapon & Attack Info ===
    EcoSim::Genetics::WeaponType weapon;    ///< Type of weapon used
    EcoSim::Genetics::CombatDamageType primaryDamageType;  ///< Main damage type dealt
    
    // === Damage Calculation ===
    float rawDamage = 0.0f;                 ///< Damage before defense
    float finalDamage = 0.0f;               ///< Damage after defense
    float effectivenessMultiplier = 1.0f;   ///< Type effectiveness (0.5-2.0)
    
    // === Defense Info ===
    EcoSim::Genetics::DefenseType defenseUsed;  ///< Defender's primary defense
    float defenseValue = 0.0f;              ///< Defense strength applied
    
    // === Health Readouts ===
    float attackerHealthBefore = 0.0f;
    float attackerHealthAfter = 0.0f;
    float attackerMaxHealth = 0.0f;
    float defenderHealthBefore = 0.0f;
    float defenderHealthAfter = 0.0f;
    float defenderMaxHealth = 0.0f;
    
    // === Effects & Outcomes ===
    bool hit = true;                        ///< Did the attack connect?
    bool causedBleeding = false;            ///< Did attack cause bleeding?
    bool defenderDied = false;              ///< Was this a killing blow?
    bool critical = false;                  ///< Critical hit (future use)
    
    // === Stamina/Energy (Optional) ===
    float attackerStaminaBefore = 0.0f;
    float attackerStaminaAfter = 0.0f;
};

/**
 * @brief Singleton Logger for simulation events
 *
 * Provides comprehensive logging for creature lifecycle, plant lifecycle,
 * feeding, reproduction, population tracking, and energy changes.
 */
class Logger {
public:
    /// Maximum entries in population history before oldest entries are trimmed
    static constexpr size_t MAX_POPULATION_HISTORY_SIZE = 10000;
    
    /// Maximum entries in breeding history before oldest entries are trimmed
    static constexpr size_t MAX_BREEDING_HISTORY_SIZE = 10000;
    
    /**
     * @brief Get the singleton instance
     */
    static Logger& getInstance();

    // Delete copy and move constructors for singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // === Configuration ===
    void configure(const LoggerConfig& config);
    void setLogLevel(LogLevel level);
    void setFlushMode(FlushMode mode);
    void setConsoleOutput(bool enabled);
    void setFileOutput(bool enabled);
    void setLogFile(const std::string& path);

    // === Event Type Filtering ===
    void enableEventType(const std::string& eventType);
    void disableEventType(const std::string& eventType);
    void enableAllEventTypes();
    void disableAllEventTypes();
    bool isEventTypeEnabled(const std::string& eventType) const;
    void setEventTypeFilter(const std::set<std::string>& allowedTypes);

    // === Tick Management ===
    void setCurrentTick(int tick);
    int getCurrentTick() const;
    void onTickEnd();

    // === Creature Lifecycle ===
    void creatureBorn(int id, const std::string& type, int parentId1, int parentId2);
    void creatureDied(int id, const std::string& type, const std::string& cause, float energy, int age);

    // === Combat Events ===
    void combatEngaged(int attackerId, const std::string& attackerName, int defenderId, const std::string& defenderName);
    void combatAttack(int attackerId, int defenderId, float damage);
    void combatEvent(const CombatLogEvent& event);  ///< Detailed combat event logging
    void combatKill(int killerId, const std::string& killerName, int victimId, const std::string& victimName);
    void combatFlee(int fleeingId, const std::string& fleeingName, int threatId, const std::string& threatName);
    void scavenging(int creatureId, const std::string& creatureName, float nutritionGained);
    
    // === Combat Configuration ===
    void setCombatLogDetail(CombatLogDetail level);
    CombatLogDetail getCombatLogDetail() const;

    // === Plant Lifecycle ===
    void plantSpawned(int id, const std::string& species, int x, int y);
    void plantDied(int id, const std::string& species, const std::string& cause, int age);

    // === Feeding & Consumption ===
    void feeding(int creatureId, int plantId, bool success, float nutritionGained, float damageReceived);
    void foodConsumed(int creatureId, int foodId, float calories);
    void starvation(int creatureId, float energyBefore, float energyAfter);

    // === Reproduction ===
    void matingAttempt(int creature1, int creature2, bool success);
    void offspring(int parentId1, int parentId2, int offspringId, const std::string& type);
    void seedDispersal(int plantId, const std::string& strategy, int targetX, int targetY, bool viable);
    void seedGermination(int seedId, int newPlantId, int x, int y);

    // === Breeding Diagnostics ===
    void breedingAttempt(int creatureId, bool foundMate, const std::string& reason);
    void birthEvent(int parentId, int offspringId);
    void breedingStateCount(int tick, int inBreedState, int seekingMate, float avgMateValue, float avgThreshold);
    void recordBreedingSnapshot(const BreedingSnapshot& snapshot);
    void printBreedingSummary();
    const std::vector<BreedingSnapshot>& getBreedingHistory() const { return m_breedingHistory; }
    const BreedingStats& getBreedingStats() const { return m_breedingStats; }
    void resetBreedingStats();

    // === Population ===
    void populationSnapshot(int tick, int creatures, int plants, int food);
    void extinctionWarning(const std::string& type, int remaining);
    void extinction(const std::string& entityType);

    // === Energy ===
    void energyChange(int entityId, const std::string& reason, float before, float after);

    // === Analysis & Output ===
    void printDeathSummary();
    void printPopulationHistory();
    void printFeedingStats();

    // === Flush Control ===
    void flush();
    void clear();

private:
    Logger();
    ~Logger();

    // Core logging
    void log(LogLevel level, const std::string& event, int entityId, 
             const std::string& entityType, const std::string& details);
    void writeToConsole(const std::string& formatted);
    void writeToFile(const std::string& csvLine);
    void writeFileHeader();

    // Helpers
    std::string levelToString(LogLevel level) const;
    std::string formatConsoleMessage(int tick, LogLevel level, const std::string& event,
                                     int entityId, const std::string& entityType,
                                     const std::string& details) const;
    std::string formatCsvLine(int tick, LogLevel level, const std::string& event,
                              int entityId, const std::string& entityType,
                              const std::string& details) const;
    std::string escapeCSV(const std::string& str) const;

    // Configuration
    LoggerConfig m_config;
    
    // State
    int m_currentTick = 0;
    int m_pendingEntries = 0;
    bool m_fileHeaderWritten = false;
    
    // Statistics
    DeathStats m_deathStats;
    FeedingStats m_feedingStats;
    BreedingStats m_breedingStats;
    std::vector<PopulationSnapshot> m_populationHistory;
    std::vector<BreedingSnapshot> m_breedingHistory;
    
    // File output
    std::ofstream m_fileStream;
    std::vector<std::string> m_pendingFileWrites;
    
    // Event type filtering
    std::set<std::string> m_disabledEventTypes;
    bool m_useEventTypeWhitelist = false;  // false = blacklist mode (disable specific), true = whitelist mode (enable only specific)
    std::set<std::string> m_enabledEventTypes;
    
    // Thread safety
    mutable std::mutex m_mutex;
};

} // namespace logging

#endif // LOGGER_HPP
