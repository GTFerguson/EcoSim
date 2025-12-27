#include "logging/Logger.hpp"
#include "genetics/interactions/DamageTypes.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace logging {

// === Singleton Instance ===

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    // Default initialization done via member initializers
}

Logger::~Logger() {
    flush();
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

// === Configuration ===

void Logger::configure(const LoggerConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config = config;
    
    // Reopen file if path changed
    if (m_config.fileOutput && !m_config.logFilePath.empty()) {
        if (m_fileStream.is_open()) {
            m_fileStream.close();
        }
        m_fileStream.open(m_config.logFilePath, std::ios::out | std::ios::trunc);
        m_fileHeaderWritten = false;
        if (m_fileStream.is_open() && m_config.csvFormat) {
            writeFileHeader();
        }
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.minLevel = level;
}

void Logger::setFlushMode(FlushMode mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.flushMode = mode;
}

void Logger::setConsoleOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.consoleOutput = enabled;
}

void Logger::setFileOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.fileOutput = enabled;
    
    if (enabled && !m_fileStream.is_open() && !m_config.logFilePath.empty()) {
        m_fileStream.open(m_config.logFilePath, std::ios::out | std::ios::trunc);
        m_fileHeaderWritten = false;
        if (m_fileStream.is_open() && m_config.csvFormat) {
            writeFileHeader();
        }
    }
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.logFilePath = path;
    
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
    
    if (m_config.fileOutput && !path.empty()) {
        m_fileStream.open(path, std::ios::out | std::ios::trunc);
        m_fileHeaderWritten = false;
        if (m_fileStream.is_open() && m_config.csvFormat) {
            writeFileHeader();
        }
    }
}

// === Tick Management ===

void Logger::setCurrentTick(int tick) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentTick = tick;
}

int Logger::getCurrentTick() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentTick;
}

void Logger::onTickEnd() {
    if (m_config.flushMode == FlushMode::PER_TICK) {
        flush();
    }
}

// === Creature Lifecycle ===

void Logger::creatureBorn(int id, const std::string& type, int parentId1, int parentId2) {
    std::ostringstream details;
    details << "parents:" << parentId1 << "," << parentId2;
    log(LogLevel::INFO, "CREATURE_BORN", id, type, details.str());
}

void Logger::creatureDied(int id, const std::string& type, const std::string& cause, float energy, int age) {
    std::ostringstream details;
    details << "cause:" << cause << ",energy:" << std::fixed << std::setprecision(1) << energy << ",age:" << age;
    log(LogLevel::INFO, "CREATURE_DIED", id, type, details.str());
    
    // Update death statistics
    std::lock_guard<std::mutex> lock(m_mutex);
    m_deathStats.totalCreatureDeaths++;
    m_deathStats.creatureDeathsByCause[cause]++;
    m_deathStats.creatureDeathsByType[type]++;
}

// === Combat Events ===

void Logger::combatEngaged(int attackerId, const std::string& attackerName, int defenderId, const std::string& defenderName) {
    std::ostringstream details;
    details << "attacker:" << attackerId << "(" << attackerName << ")"
            << ",defender:" << defenderId << "(" << defenderName << ")";
    log(LogLevel::INFO, "COMBAT_ENGAGED", attackerId, attackerName, details.str());
}

void Logger::combatAttack(int attackerId, int defenderId, float damage) {
    std::ostringstream details;
    details << "defender:" << defenderId
            << ",damage:" << std::fixed << std::setprecision(1) << damage;
    log(LogLevel::INFO, "COMBAT_ATTACK", attackerId, "", details.str());
}

void Logger::combatKill(int killerId, const std::string& killerName, int victimId, const std::string& victimName) {
    std::ostringstream details;
    details << "killer:" << killerId << "(" << killerName << ")"
            << ",victim:" << victimId << "(" << victimName << ")";
    log(LogLevel::INFO, "COMBAT_KILL", victimId, victimName, details.str());
}

void Logger::combatFlee(int fleeingId, const std::string& fleeingName, int threatId, const std::string& threatName) {
    std::ostringstream details;
    details << "threat:" << threatId << "(" << threatName << ")";
    log(LogLevel::INFO, "COMBAT_FLEE", fleeingId, fleeingName, details.str());
}

void Logger::scavenging(int creatureId, const std::string& creatureName, float nutritionGained) {
    std::ostringstream details;
    details << "nutrition:" << std::fixed << std::setprecision(1) << nutritionGained;
    log(LogLevel::DEBUG, "SCAVENGING", creatureId, creatureName, details.str());
}

void Logger::combatEvent(const CombatLogEvent& event) {
    using namespace EcoSim::Genetics;
    
    std::ostringstream details;
    
    // Get string representations of enums
    const char* weaponStr = weaponTypeToString(event.weapon);
    const char* damageTypeStr = damageTypeToString(event.primaryDamageType);
    const char* defenseStr = defenseTypeToString(event.defenseUsed);
    
    // Build output based on verbosity level
    switch (m_config.combatDetail) {
        case CombatLogDetail::MINIMAL:
            // Minimal: "#3→#4 15.8 dmg | Atk:95/100 Def:100→84/100"
            details << "#" << event.attackerId << "→#" << event.defenderId
                    << " " << std::fixed << std::setprecision(1) << event.finalDamage << " dmg"
                    << " | Atk:" << static_cast<int>(event.attackerHealthBefore)
                    << "/" << static_cast<int>(event.attackerMaxHealth)
                    << " Def:" << static_cast<int>(event.defenderHealthBefore)
                    << "→" << static_cast<int>(event.defenderHealthAfter)
                    << "/" << static_cast<int>(event.defenderMaxHealth);
            if (event.defenderDied) details << " [KILL]";
            break;
            
        case CombatLogDetail::STANDARD:
            // Standard: "#3→#4 Teeth 15.8 Piercing | Atk:95/100 | Def:100→84/100"
            details << "#" << event.attackerId << "→#" << event.defenderId
                    << " " << weaponStr << " "
                    << std::fixed << std::setprecision(1) << event.finalDamage
                    << " " << damageTypeStr
                    << " | Atk:" << static_cast<int>(event.attackerHealthBefore)
                    << "/" << static_cast<int>(event.attackerMaxHealth)
                    << " | Def:" << static_cast<int>(event.defenderHealthBefore)
                    << "→" << static_cast<int>(event.defenderHealthAfter)
                    << "/" << static_cast<int>(event.defenderMaxHealth);
            if (event.causedBleeding) details << " [BLEEDING]";
            if (event.defenderDied) details << " [KILL]";
            break;
            
        case CombatLogDetail::DETAILED:
            // Detailed: Names + weapon details + both HP values
            details << event.attackerName << " #" << event.attackerId
                    << " → " << event.defenderName << " #" << event.defenderId << "\n";
            details << "  " << weaponStr << " (" << damageTypeStr << ") "
                    << std::fixed << std::setprecision(1) << event.rawDamage
                    << " raw → " << event.finalDamage << " final"
                    << " (x" << std::setprecision(2) << event.effectivenessMultiplier
                    << " vs " << defenseStr << ")\n";
            details << "  Attacker: " << std::fixed << std::setprecision(1)
                    << event.attackerHealthBefore << "/" << event.attackerMaxHealth
                    << " | Defender: " << event.defenderHealthBefore
                    << " → " << event.defenderHealthAfter
                    << "/" << event.defenderMaxHealth;
            if (event.causedBleeding) details << " [BLEEDING]";
            if (event.defenderDied) details << " [KILL]";
            break;
            
        case CombatLogDetail::DEBUG:
            // Debug: Full multi-line output with all data
            details << "=== COMBAT ===\n";
            details << "  Attacker: " << event.attackerName << " (#" << event.attackerId << ")\n";
            details << "    Health: " << std::fixed << std::setprecision(1)
                    << event.attackerHealthBefore << "/" << event.attackerMaxHealth
                    << " → " << event.attackerHealthAfter << "/" << event.attackerMaxHealth;
            if (event.attackerStaminaBefore > 0 || event.attackerStaminaAfter > 0) {
                details << " | Stamina: " << event.attackerStaminaBefore
                        << " → " << event.attackerStaminaAfter;
            }
            details << "\n  Defender: " << event.defenderName << " (#" << event.defenderId << ")\n";
            details << "    Health: " << std::fixed << std::setprecision(1)
                    << event.defenderHealthBefore << "/" << event.defenderMaxHealth
                    << " → " << event.defenderHealthAfter << "/" << event.defenderMaxHealth
                    << " (Taking " << event.finalDamage << " damage)\n";
            details << "  Attack: " << weaponStr
                    << " | Type: " << damageTypeStr
                    << " | Raw: " << std::setprecision(1) << event.rawDamage << "\n";
            details << "  Defense: " << defenseStr
                    << " (" << std::setprecision(2) << event.defenseValue << ")"
                    << " | Effectiveness: x" << event.effectivenessMultiplier << "\n";
            details << "  Effects:";
            if (event.causedBleeding) details << " [BLEEDING]";
            if (event.critical) details << " [CRITICAL]";
            if (!event.causedBleeding && !event.critical) details << " none";
            details << "\n  Outcome: ";
            if (event.hit) {
                details << "Hit";
                if (event.defenderDied) details << ", Defender Killed";
                else details << ", Defender Alive";
            } else {
                details << "Missed";
            }
            break;
    }
    
    // Use -1 and empty string to avoid duplicate header from log()
    log(LogLevel::INFO, "COMBAT", -1, "", details.str());
}

void Logger::setCombatLogDetail(CombatLogDetail level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.combatDetail = level;
}

CombatLogDetail Logger::getCombatLogDetail() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config.combatDetail;
}

// === Plant Lifecycle ===

void Logger::plantSpawned(int id, const std::string& species, int x, int y) {
    std::ostringstream details;
    details << "pos:" << x << "," << y;
    log(LogLevel::INFO, "PLANT_SPAWNED", id, species, details.str());
}

void Logger::plantDied(int id, const std::string& species, const std::string& cause, int age) {
    std::ostringstream details;
    details << "cause:" << cause << ",age:" << age;
    log(LogLevel::INFO, "PLANT_DIED", id, species, details.str());
    
    // Update death statistics
    std::lock_guard<std::mutex> lock(m_mutex);
    m_deathStats.totalPlantDeaths++;
    m_deathStats.plantDeathsByCause[cause]++;
    m_deathStats.plantDeathsBySpecies[species]++;
}

// === Feeding & Consumption ===

void Logger::feeding(int creatureId, int plantId, bool success, float nutritionGained, float damageReceived) {
    std::ostringstream details;
    details << "plant:" << plantId 
            << ",success:" << (success ? "true" : "false")
            << ",nutrition:" << std::fixed << std::setprecision(1) << nutritionGained 
            << ",damage:" << std::fixed << std::setprecision(1) << damageReceived;
    log(LogLevel::INFO, "FEEDING", creatureId, "", details.str());
    
    // Update feeding statistics
    std::lock_guard<std::mutex> lock(m_mutex);
    m_feedingStats.totalAttempts++;
    if (success) {
        m_feedingStats.successfulFeedings++;
        m_feedingStats.totalNutritionGained += nutritionGained;
        m_feedingStats.totalDamageReceived += damageReceived;
    }
}

void Logger::foodConsumed(int creatureId, int foodId, float calories) {
    std::ostringstream details;
    details << "food:" << foodId << ",calories:" << std::fixed << std::setprecision(1) << calories;
    log(LogLevel::INFO, "FOOD_CONSUMED", creatureId, "", details.str());
}

void Logger::starvation(int creatureId, float energyBefore, float energyAfter) {
    std::ostringstream details;
    details << "energyBefore:" << std::fixed << std::setprecision(1) << energyBefore 
            << ",energyAfter:" << std::fixed << std::setprecision(1) << energyAfter;
    log(LogLevel::WARN, "STARVATION", creatureId, "", details.str());
}

// === Reproduction ===

void Logger::matingAttempt(int creature1, int creature2, bool success) {
    std::ostringstream details;
    details << "partner:" << creature2 << ",success:" << (success ? "true" : "false");
    log(LogLevel::INFO, "MATING_ATTEMPT", creature1, "", details.str());
}

void Logger::offspring(int parentId1, int parentId2, int offspringId, const std::string& type) {
    std::ostringstream details;
    details << "parents:" << parentId1 << "," << parentId2 << ",offspring:" << offspringId;
    log(LogLevel::INFO, "OFFSPRING", offspringId, type, details.str());
}

void Logger::seedDispersal(int plantId, const std::string& strategy, int targetX, int targetY, bool viable) {
    std::ostringstream details;
    details << "strategy:" << strategy 
            << ",target:" << targetX << "," << targetY 
            << ",viable:" << (viable ? "true" : "false");
    log(LogLevel::INFO, "SEED_DISPERSAL", plantId, "", details.str());
}

void Logger::seedGermination(int seedId, int newPlantId, int x, int y) {
    std::ostringstream details;
    details << "seed:" << seedId << ",pos:" << x << "," << y;
    log(LogLevel::INFO, "SEED_GERMINATION", newPlantId, "", details.str());
}

// === Breeding Diagnostics ===

void Logger::breedingAttempt(int creatureId, bool foundMate, const std::string& reason) {
    std::ostringstream details;
    details << "foundMate:" << (foundMate ? "true" : "false");
    if (!foundMate && !reason.empty()) {
        details << ",reason:" << reason;
    }
    log(LogLevel::INFO, "BREEDING_ATTEMPT", creatureId, "", details.str());
    
    // Update breeding statistics
    std::lock_guard<std::mutex> lock(m_mutex);
    m_breedingStats.totalMatingAttempts++;
    if (foundMate) {
        m_breedingStats.totalMateFound++;
    } else {
        m_breedingStats.noMateReasons[reason]++;
        m_breedingStats.failedBreedings++;
    }
}

void Logger::birthEvent(int parentId, int offspringId) {
    std::ostringstream details;
    details << "parent:" << parentId << ",offspring:" << offspringId;
    log(LogLevel::INFO, "BIRTH_EVENT", offspringId, "", details.str());
    
    // Update breeding statistics
    std::lock_guard<std::mutex> lock(m_mutex);
    m_breedingStats.successfulBreedings++;
}

void Logger::breedingStateCount(int tick, int inBreedState, int seekingMate, float avgMateValue, float avgThreshold) {
    std::ostringstream details;
    details << "inBreedState:" << inBreedState
            << ",seekingMate:" << seekingMate
            << ",avgMateValue:" << std::fixed << std::setprecision(2) << avgMateValue
            << ",avgThreshold:" << std::fixed << std::setprecision(2) << avgThreshold;
    log(LogLevel::INFO, "BREEDING_STATE_COUNT", -1, "", details.str());
    
    // Update cumulative stats
    std::lock_guard<std::mutex> lock(m_mutex);
    m_breedingStats.totalInBreedState += inBreedState;
    m_breedingStats.totalSeekingMate += seekingMate;
    m_breedingStats.totalMateValue += avgMateValue * inBreedState;
    m_breedingStats.totalThresholdValue += avgThreshold * inBreedState;
}

void Logger::recordBreedingSnapshot(const BreedingSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_breedingHistory.push_back(snapshot);
}

void Logger::printBreedingSummary() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::cout << "\n========== BREEDING SUMMARY ==========\n";
    std::cout << "Total Mating Attempts: " << m_breedingStats.totalMatingAttempts << "\n";
    std::cout << "Successful Breedings: " << m_breedingStats.successfulBreedings << "\n";
    std::cout << "Failed Breedings: " << m_breedingStats.failedBreedings << "\n";
    std::cout << "Total Mates Found: " << m_breedingStats.totalMateFound << "\n";
    
    float successRate = m_breedingStats.totalMatingAttempts > 0
        ? (100.0f * m_breedingStats.successfulBreedings / m_breedingStats.totalMatingAttempts) : 0.0f;
    std::cout << "Success Rate: " << std::fixed << std::setprecision(1) << successRate << "%\n";
    
    std::cout << "\n--- No Mate Reasons ---\n";
    for (const auto& [reason, count] : m_breedingStats.noMateReasons) {
        float pct = m_breedingStats.failedBreedings > 0
            ? (100.0f * count / m_breedingStats.failedBreedings) : 0.0f;
        std::cout << "  " << reason << ": " << count
                  << " (" << std::fixed << std::setprecision(1) << pct << "%)\n";
    }
    
    if (!m_breedingHistory.empty()) {
        std::cout << "\n--- Breeding History (per tick snapshots) ---\n";
        std::cout << std::setw(8) << "Tick"
                  << std::setw(12) << "InBreed"
                  << std::setw(12) << "Seeking"
                  << std::setw(12) << "Attempts"
                  << std::setw(10) << "Births"
                  << std::setw(12) << "AvgMate" << "\n";
        std::cout << std::string(66, '-') << "\n";
        
        for (const auto& snap : m_breedingHistory) {
            std::cout << std::setw(8) << snap.tick
                      << std::setw(12) << snap.creaturesInBreedState
                      << std::setw(12) << snap.creaturesSeekingMate
                      << std::setw(12) << snap.matingAttempts
                      << std::setw(10) << snap.successfulBirths
                      << std::setw(12) << std::fixed << std::setprecision(2) << snap.avgMateValue << "\n";
        }
    }
    
    std::cout << "======================================\n\n";
}

void Logger::resetBreedingStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_breedingStats = BreedingStats{};
    m_breedingHistory.clear();
}

// === Population ===

void Logger::populationSnapshot(int tick, int creatures, int plants, int food) {
    std::ostringstream details;
    details << "creatures:" << creatures << ",plants:" << plants << ",food:" << food;
    log(LogLevel::INFO, "POPULATION_SNAPSHOT", -1, "", details.str());
    
    // Store in history
    std::lock_guard<std::mutex> lock(m_mutex);
    m_populationHistory.push_back({tick, creatures, plants, food});
}

void Logger::extinctionWarning(const std::string& type, int remaining) {
    std::ostringstream details;
    details << "remaining:" << remaining;
    log(LogLevel::WARN, "EXTINCTION_WARNING", -1, type, details.str());
}

void Logger::extinction(const std::string& entityType) {
    log(LogLevel::CRITICAL, "EXTINCTION", -1, entityType, "");
}

// === Energy ===

void Logger::energyChange(int entityId, const std::string& reason, float before, float after) {
    std::ostringstream details;
    details << "reason:" << reason 
            << ",before:" << std::fixed << std::setprecision(1) << before 
            << ",after:" << std::fixed << std::setprecision(1) << after
            << ",delta:" << std::fixed << std::setprecision(1) << (after - before);
    log(LogLevel::DEBUG, "ENERGY_CHANGE", entityId, "", details.str());
}

// === Analysis & Output ===

void Logger::printDeathSummary() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::cout << "\n========== DEATH SUMMARY ==========\n";
    std::cout << "Total Creature Deaths: " << m_deathStats.totalCreatureDeaths << "\n";
    std::cout << "Total Plant Deaths: " << m_deathStats.totalPlantDeaths << "\n";
    
    std::cout << "\n--- Creature Deaths by Cause ---\n";
    for (const auto& [cause, count] : m_deathStats.creatureDeathsByCause) {
        float pct = m_deathStats.totalCreatureDeaths > 0 
            ? (100.0f * count / m_deathStats.totalCreatureDeaths) : 0.0f;
        std::cout << "  " << cause << ": " << count 
                  << " (" << std::fixed << std::setprecision(1) << pct << "%)\n";
    }
    
    std::cout << "\n--- Creature Deaths by Type ---\n";
    for (const auto& [type, count] : m_deathStats.creatureDeathsByType) {
        std::cout << "  " << type << ": " << count << "\n";
    }
    
    std::cout << "\n--- Plant Deaths by Cause ---\n";
    for (const auto& [cause, count] : m_deathStats.plantDeathsByCause) {
        float pct = m_deathStats.totalPlantDeaths > 0 
            ? (100.0f * count / m_deathStats.totalPlantDeaths) : 0.0f;
        std::cout << "  " << cause << ": " << count 
                  << " (" << std::fixed << std::setprecision(1) << pct << "%)\n";
    }
    
    std::cout << "\n--- Plant Deaths by Species ---\n";
    for (const auto& [species, count] : m_deathStats.plantDeathsBySpecies) {
        std::cout << "  " << species << ": " << count << "\n";
    }
    
    std::cout << "===================================\n\n";
}

void Logger::printPopulationHistory() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::cout << "\n========== POPULATION HISTORY ==========\n";
    std::cout << std::setw(8) << "Tick" 
              << std::setw(12) << "Creatures" 
              << std::setw(10) << "Plants" 
              << std::setw(8) << "Food" << "\n";
    std::cout << std::string(38, '-') << "\n";
    
    for (const auto& snap : m_populationHistory) {
        std::cout << std::setw(8) << snap.tick 
                  << std::setw(12) << snap.creatures 
                  << std::setw(10) << snap.plants 
                  << std::setw(8) << snap.food << "\n";
    }
    
    std::cout << "========================================\n\n";
}

void Logger::printFeedingStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::cout << "\n========== FEEDING STATISTICS ==========\n";
    std::cout << "Total Feeding Attempts: " << m_feedingStats.totalAttempts << "\n";
    std::cout << "Successful Feedings: " << m_feedingStats.successfulFeedings << "\n";
    
    float successRate = m_feedingStats.totalAttempts > 0 
        ? (100.0f * m_feedingStats.successfulFeedings / m_feedingStats.totalAttempts) : 0.0f;
    std::cout << "Success Rate: " << std::fixed << std::setprecision(1) << successRate << "%\n";
    
    std::cout << "Total Nutrition Gained: " << std::fixed << std::setprecision(1) 
              << m_feedingStats.totalNutritionGained << "\n";
    std::cout << "Total Damage Received: " << std::fixed << std::setprecision(1) 
              << m_feedingStats.totalDamageReceived << "\n";
    
    if (m_feedingStats.successfulFeedings > 0) {
        float avgNutrition = m_feedingStats.totalNutritionGained / m_feedingStats.successfulFeedings;
        float avgDamage = m_feedingStats.totalDamageReceived / m_feedingStats.successfulFeedings;
        std::cout << "Avg Nutrition per Feeding: " << std::fixed << std::setprecision(2) 
                  << avgNutrition << "\n";
        std::cout << "Avg Damage per Feeding: " << std::fixed << std::setprecision(2) 
                  << avgDamage << "\n";
    }
    
    std::cout << "=========================================\n\n";
}

// === Flush Control ===

void Logger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_config.fileOutput && m_fileStream.is_open()) {
        for (const auto& line : m_pendingFileWrites) {
            m_fileStream << line << "\n";
        }
        m_fileStream.flush();
        m_pendingFileWrites.clear();
    }
    
    m_pendingEntries = 0;
}

void Logger::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_deathStats = DeathStats{};
    m_feedingStats = FeedingStats{};
    m_populationHistory.clear();
    m_pendingFileWrites.clear();
    m_pendingEntries = 0;
    m_currentTick = 0;
}

// === Private Helper Methods ===

void Logger::log(LogLevel level, const std::string& event, int entityId,
                 const std::string& entityType, const std::string& details) {
    // Check log level filter
    if (level < m_config.minLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Console output
    if (m_config.consoleOutput) {
        std::string formatted = formatConsoleMessage(m_currentTick, level, event, 
                                                     entityId, entityType, details);
        std::cout << formatted << "\n";
    }
    
    // File output
    if (m_config.fileOutput) {
        if (!m_fileStream.is_open() && !m_config.logFilePath.empty()) {
            m_fileStream.open(m_config.logFilePath, std::ios::out | std::ios::app);
            if (m_fileStream.is_open() && !m_fileHeaderWritten && m_config.csvFormat) {
                writeFileHeader();
            }
        }
        
        if (m_config.csvFormat) {
            std::string csvLine = formatCsvLine(m_currentTick, level, event, 
                                                entityId, entityType, details);
            
            if (m_config.flushMode == FlushMode::IMMEDIATE) {
                m_fileStream << csvLine << "\n";
                m_fileStream.flush();
            } else {
                m_pendingFileWrites.push_back(csvLine);
                m_pendingEntries++;
                
                if (m_config.flushMode == FlushMode::PERIODIC && 
                    m_pendingEntries >= m_config.periodicFlushCount) {
                    // Flush without holding lock (already held)
                    for (const auto& line : m_pendingFileWrites) {
                        m_fileStream << line << "\n";
                    }
                    m_fileStream.flush();
                    m_pendingFileWrites.clear();
                    m_pendingEntries = 0;
                }
            }
        }
    }
}

void Logger::writeToConsole(const std::string& formatted) {
    std::cout << formatted << std::endl;
}

void Logger::writeToFile(const std::string& csvLine) {
    if (m_fileStream.is_open()) {
        m_fileStream << csvLine << "\n";
    }
}

void Logger::writeFileHeader() {
    if (m_fileStream.is_open() && !m_fileHeaderWritten) {
        m_fileStream << "tick,level,event,entity_id,entity_type,details\n";
        m_fileHeaderWritten = true;
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARN:     return "WARN";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

std::string Logger::formatConsoleMessage(int tick, LogLevel level, const std::string& event,
                                         int entityId, const std::string& entityType,
                                         const std::string& details) const {
    std::ostringstream oss;
    oss << "[T:" << tick << "] [" << levelToString(level) << "] " << event;
    
    if (entityId >= 0) {
        oss << " #" << entityId;
    }
    
    if (!entityType.empty()) {
        oss << " (" << entityType << ")";
    }
    
    if (!details.empty()) {
        oss << " " << details;
    }
    
    return oss.str();
}

std::string Logger::formatCsvLine(int tick, LogLevel level, const std::string& event,
                                  int entityId, const std::string& entityType,
                                  const std::string& details) const {
    std::ostringstream oss;
    oss << tick << ","
        << levelToString(level) << ","
        << event << ","
        << (entityId >= 0 ? std::to_string(entityId) : "") << ","
        << entityType << ","
        << "\"" << escapeCSV(details) << "\"";
    return oss.str();
}

std::string Logger::escapeCSV(const std::string& str) const {
    std::string result;
    result.reserve(str.size());
    
    for (char c : str) {
        if (c == '"') {
            result += "\"\"";  // Escape double quotes
        } else {
            result += c;
        }
    }
    
    return result;
}

} // namespace logging
