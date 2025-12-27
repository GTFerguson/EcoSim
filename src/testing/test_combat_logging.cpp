/**
 * @file test_combat_logging.cpp
 * @brief Simple test to demonstrate combat logging output
 */

#include "logging/Logger.hpp"
#include <iostream>

int main() {
    using namespace logging;
    
    // Configure logger to output to console
    Logger& logger = Logger::getInstance();
    LoggerConfig config;
    config.consoleOutput = true;
    config.fileOutput = false;
    config.minLevel = LogLevel::DEBUG;
    logger.configure(config);
    
    std::cout << "========================================\n";
    std::cout << "      Combat Logging Demonstration      \n";
    std::cout << "========================================\n\n";
    
    // Simulate a combat scenario
    
    // Tick 50: Predator spots and engages prey
    logger.setCurrentTick(50);
    logger.combatEngaged(5, "predator_carnivore", 12, "prey_herbivore");
    
    // Tick 52: Predator attacks prey
    logger.setCurrentTick(52);
    logger.combatAttack(5, 12, 3.5f);
    
    // Tick 55: Another attack
    logger.setCurrentTick(55);
    logger.combatAttack(5, 12, 4.2f);
    
    // Tick 57: Prey flees
    logger.setCurrentTick(57);
    logger.combatFlee(12, "prey_herbivore", 5, "predator_carnivore");
    
    // Tick 100: New combat - predator catches weakened prey
    logger.setCurrentTick(100);
    logger.combatEngaged(7, "predator_omnivore", 18, "prey_small");
    
    // Tick 102: Attack
    logger.setCurrentTick(102);
    logger.combatAttack(7, 18, 8.0f);
    
    // Tick 104: Kill
    logger.setCurrentTick(104);
    logger.combatKill(7, "predator_omnivore", 18, "prey_small");
    
    // Tick 105: Another creature scavenges the corpse
    logger.setCurrentTick(105);
    logger.scavenging(22, "scavenger_opportunist", 12.5f);
    
    // Tick 110: More scavenging
    logger.setCurrentTick(110);
    logger.scavenging(22, "scavenger_opportunist", 8.3f);
    
    std::cout << "\n========================================\n";
    std::cout << "     Combat Logging Test Complete       \n";
    std::cout << "========================================\n";
    
    return 0;
}
