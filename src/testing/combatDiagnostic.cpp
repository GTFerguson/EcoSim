/**
 * @file combatDiagnostic.cpp
 * @brief Combat system diagnostic - tests combat interactions and logging
 *
 * This diagnostic demonstrates the combat system by:
 * 1. Creating apex predator creatures with high aggression
 * 2. Simulating combat interactions between them
 * 3. Logging all combat events
 */

#include "logging/Logger.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/interactions/CombatInteraction.hpp"
#include "genetics/interactions/CombatAction.hpp"
#include "genetics/interactions/DamageTypes.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "world/Corpse.hpp"
#include "objects/creature/creature.hpp"

#include <iostream>
#include <memory>
#include <iomanip>

using namespace EcoSim::Genetics;
using namespace logging;

/**
 * @brief Helper to set a gene value on a genome
 */
void setGeneValue(Genome& genome, const std::string& geneId, float value) {
    if (genome.hasGene(geneId)) {
        genome.getGeneMutable(geneId).setAlleleValues(value);
    }
}

/**
 * @brief Combat test state - holds genome and phenotype together to avoid dangling pointers
 */
struct CombatTestState {
    std::unique_ptr<Genome> genome;
    std::unique_ptr<Phenotype> phenotype;
    
    CombatTestState(GeneRegistry& registry, float aggression, float meatDigestion,
                    float toothSharpness, float retreatThreshold = 0.3f) {
        genome = std::make_unique<Genome>(UniversalGenes::createCreatureGenome(registry));
        
        // Set combat-relevant genes
        setGeneValue(*genome, UniversalGenes::COMBAT_AGGRESSION, aggression);
        setGeneValue(*genome, UniversalGenes::MEAT_DIGESTION_EFFICIENCY, meatDigestion);
        setGeneValue(*genome, UniversalGenes::TOOTH_SHARPNESS, toothSharpness);
        setGeneValue(*genome, UniversalGenes::RETREAT_THRESHOLD, retreatThreshold);
        
        phenotype = std::make_unique<Phenotype>(genome.get(), &registry);
    }
};

/**
 * @brief Simulate a combat encounter between two creatures
 */
void simulateCombatEncounter(
    Logger& logger,
    GeneRegistry& registry,
    int attackerId,
    const std::string& attackerName,
    float attackerAggression,
    int defenderId,
    const std::string& defenderName,
    float defenderAggression,
    int startTick
) {
    std::cout << "\n--- Combat: " << attackerName << " vs " << defenderName << " ---\n";
    
    // Create combat states (which hold both genome and phenotype)
    CombatTestState attackerState(registry, attackerAggression, 0.9f, 0.8f, 0.2f);
    CombatTestState defenderState(registry, defenderAggression, 0.3f, 0.3f, 0.4f);
    
    float attackerHealth = 100.0f;
    float defenderHealth = 100.0f;
    const float MAX_HEALTH = 100.0f;
    
    // Log combat engagement
    logger.setCurrentTick(startTick);
    logger.combatEngaged(attackerId, attackerName, defenderId, defenderName);
    
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Tick " << startTick << ": Combat engaged!\n";
    std::cout << "  " << attackerName << " Health: " << attackerHealth << "/" << MAX_HEALTH << "\n";
    std::cout << "  " << defenderName << " Health: " << defenderHealth << "/" << MAX_HEALTH << "\n";
    
    int tick = startTick + 1;
    bool combatOver = false;
    
    while (!combatOver && tick < startTick + 50) {
        logger.setCurrentTick(tick);
        
        // Attacker's turn
        if (attackerHealth > 0) {
            // Select best attack
            CombatAction action = CombatInteraction::selectBestAction(
                *attackerState.phenotype, *defenderState.phenotype);
            
            // Resolve attack
            AttackResult result = CombatInteraction::resolveAttack(
                *attackerState.phenotype, *defenderState.phenotype, action);
            
            float defenderHealthBefore = defenderHealth;
            defenderHealth -= result.finalDamage;
            if (defenderHealth < 0) defenderHealth = 0;
            
            // Build detailed combat log event
            CombatLogEvent event;
            event.attackerId = attackerId;
            event.defenderId = defenderId;
            event.attackerName = attackerName;
            event.defenderName = defenderName;
            event.weapon = action.weapon;
            event.primaryDamageType = result.primaryType;
            event.rawDamage = result.rawDamage;
            event.finalDamage = result.finalDamage;
            event.effectivenessMultiplier = result.effectivenessMultiplier;
            event.defenseUsed = CombatInteraction::getDefenseProfile(*defenderState.phenotype).getStrongestDefense();
            event.defenseValue = CombatInteraction::getDefenseProfile(*defenderState.phenotype).getDefenseForType(event.defenseUsed);
            event.attackerHealthBefore = attackerHealth;
            event.attackerHealthAfter = attackerHealth;
            event.attackerMaxHealth = MAX_HEALTH;
            event.defenderHealthBefore = defenderHealthBefore;
            event.defenderHealthAfter = defenderHealth;
            event.defenderMaxHealth = MAX_HEALTH;
            event.hit = result.hit;
            event.causedBleeding = result.causedBleeding;
            event.defenderDied = (defenderHealth <= 0);
            
            logger.combatEvent(event);
            
            // Check for kill
            if (defenderHealth <= 0) {
                logger.combatKill(attackerId, attackerName, defenderId, defenderName);
                combatOver = true;
                continue;
            }
            
            // Check for defender retreat
            float healthPercent = defenderHealth / MAX_HEALTH;
            if (CombatInteraction::shouldRetreat(*defenderState.phenotype, healthPercent)) {
                logger.combatFlee(defenderId, defenderName, attackerId, attackerName);
                std::cout << "Tick " << tick << ": " << defenderName << " flees! (health at "
                          << (healthPercent * 100) << "%)\n";
                combatOver = true;
                continue;
            }
        }
        
        tick++;
        
        // Defender counter-attack (if aggressive enough)
        if (!combatOver && defenderHealth > 0 && defenderAggression > 0.5f) {
            logger.setCurrentTick(tick);
            
            CombatAction counterAction = CombatInteraction::selectBestAction(
                *defenderState.phenotype, *attackerState.phenotype);
            
            AttackResult counterResult = CombatInteraction::resolveAttack(
                *defenderState.phenotype, *attackerState.phenotype, counterAction);
            
            float attackerHealthBefore = attackerHealth;
            attackerHealth -= counterResult.finalDamage;
            if (attackerHealth < 0) attackerHealth = 0;
            
            // Build detailed combat log event for counter-attack
            CombatLogEvent counterEvent;
            counterEvent.attackerId = defenderId;
            counterEvent.defenderId = attackerId;
            counterEvent.attackerName = defenderName;
            counterEvent.defenderName = attackerName;
            counterEvent.weapon = counterAction.weapon;
            counterEvent.primaryDamageType = counterResult.primaryType;
            counterEvent.rawDamage = counterResult.rawDamage;
            counterEvent.finalDamage = counterResult.finalDamage;
            counterEvent.effectivenessMultiplier = counterResult.effectivenessMultiplier;
            counterEvent.defenseUsed = CombatInteraction::getDefenseProfile(*attackerState.phenotype).getStrongestDefense();
            counterEvent.defenseValue = CombatInteraction::getDefenseProfile(*attackerState.phenotype).getDefenseForType(counterEvent.defenseUsed);
            counterEvent.attackerHealthBefore = defenderHealth;
            counterEvent.attackerHealthAfter = defenderHealth;
            counterEvent.attackerMaxHealth = MAX_HEALTH;
            counterEvent.defenderHealthBefore = attackerHealthBefore;
            counterEvent.defenderHealthAfter = attackerHealth;
            counterEvent.defenderMaxHealth = MAX_HEALTH;
            counterEvent.hit = counterResult.hit;
            counterEvent.causedBleeding = counterResult.causedBleeding;
            counterEvent.defenderDied = (attackerHealth <= 0);
            
            logger.combatEvent(counterEvent);
            
            if (attackerHealth <= 0) {
                logger.combatKill(defenderId, defenderName, attackerId, attackerName);
                combatOver = true;
            }
        }
        
        tick++;
    }
    
    if (!combatOver) {
        std::cout << "Combat timed out after " << (tick - startTick) << " ticks\n";
    }
    
    std::cout << "\nFinal state:\n";
    std::cout << "  " << attackerName << " Health: " << attackerHealth << "/" << MAX_HEALTH
              << (attackerHealth <= 0 ? " [DEAD]" : "") << "\n";
    std::cout << "  " << defenderName << " Health: " << defenderHealth << "/" << MAX_HEALTH
              << (defenderHealth <= 0 ? " [DEAD]" : "") << "\n";
}

/**
 * @brief Test scavenging behavior
 */
void testScavenging(Logger& logger, int tick) {
    std::cout << "\n--- Scavenging Test ---\n";
    
    // Create a corpse
    world::Corpse corpse(10.0f, 10.0f, 3.0f, "ApexPredator", 0.7f);
    
    std::cout << "Corpse created: Size 3.0, Body condition 0.7\n";
    std::cout << "Initial nutrition available\n";
    
    // Simulate scavenging
    logger.setCurrentTick(tick);
    
    float nutrition1 = corpse.extractNutrition(15.0f);
    logger.scavenging(100, "scavenger_1", nutrition1);
    std::cout << "Tick " << tick << ": Scavenger #100 extracts " << nutrition1 << " nutrition\n";
    
    // Age the corpse
    for (int i = 0; i < 50; i++) {
        corpse.tick();
    }
    
    logger.setCurrentTick(tick + 50);
    float nutrition2 = corpse.extractNutrition(15.0f);
    logger.scavenging(101, "scavenger_2", nutrition2);
    std::cout << "Tick " << (tick + 50) << ": Scavenger #101 extracts " << nutrition2 
              << " nutrition (corpse aged, toxicity: " << corpse.getToxicity() << ")\n";
    
    // Continue aging
    for (int i = 0; i < 100; i++) {
        corpse.tick();
    }
    
    logger.setCurrentTick(tick + 150);
    float nutrition3 = corpse.extractNutrition(15.0f);
    if (nutrition3 > 0) {
        logger.scavenging(102, "scavenger_3", nutrition3);
    }
    std::cout << "Tick " << (tick + 150) << ": Scavenger #102 extracts " << nutrition3 
              << " nutrition (highly decayed, toxicity: " << corpse.getToxicity() << ")\n";
    
    if (corpse.isExhausted()) {
        std::cout << "Corpse exhausted - no more nutrition available\n";
    }
}

/**
 * @brief Test creature factory templates
 */
void testCreatureFactory() {
    std::cout << "\n--- Creature Factory Templates ---\n";
    
    // CreatureFactory's constructor calls UniversalGenes::registerDefaults() internally,
    // so we don't need to call it here. The idempotent check ensures this is safe either way.
    auto registry = std::make_shared<GeneRegistry>();
    
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::cout << "Available templates:\n";
    for (const auto& name : factory.getTemplateNames()) {
        const CreatureTemplate* tmpl = factory.getTemplate(name);
        if (tmpl) {
            std::cout << "  - " << tmpl->displayName << " (" << name << ")\n";
        }
    }
    
    // Create some creatures and display their combat stats
    std::cout << "\nCreating apex predators for combat analysis:\n";
    
    Creature apex1 = factory.createFromTemplate("apex_predator", 0, 0);
    Creature apex2 = factory.createFromTemplate("apex_predator", 1, 1);
    
    std::cout << "  Apex Predator #1:\n";
    std::cout << "    Health: " << apex1.getHealth() << "/" << apex1.getMaxHealth() << "\n";
    std::cout << "    Archetype: " << apex1.getArchetypeLabel() << "\n";
    
    std::cout << "  Apex Predator #2:\n";
    std::cout << "    Health: " << apex2.getHealth() << "/" << apex2.getMaxHealth() << "\n";
    std::cout << "    Archetype: " << apex2.getArchetypeLabel() << "\n";
    
    // Test different creature types
    std::cout << "\nOther archetype examples:\n";
    
    Creature herbivore = factory.createFromTemplate("tank_herbivore", 2, 2);
    std::cout << "  Tank Herbivore: " << herbivore.getArchetypeLabel() << "\n";
    
    Creature scavenger = factory.createFromTemplate("scavenger", 3, 3);
    std::cout << "  Scavenger: " << scavenger.getArchetypeLabel() << "\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "     Combat Diagnostic Simulation\n";
    std::cout << "========================================\n";
    
    // Initialize creature gene registry
    Creature::initializeGeneRegistry();
    
    // Configure logger
    Logger& logger = Logger::getInstance();
    LoggerConfig config;
    config.consoleOutput = true;
    config.fileOutput = false;
    config.minLevel = LogLevel::DEBUG;
    config.combatDetail = CombatLogDetail::DETAILED;  // Try MINIMAL, STANDARD, DETAILED, or DEBUG
    logger.configure(config);
    
    // Create gene registry for phenotype tests
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    std::cout << "\n=== Test 1: Apex Predator vs Prey ===\n";
    simulateCombatEncounter(
        logger, registry,
        1, "ApexPredator_Alpha",  // Attacker: high aggression
        0.9f,
        2, "FleetRunner_Beta",    // Defender: low aggression (prey)
        0.2f,
        100  // Start tick
    );
    
    std::cout << "\n=== Test 2: Predator vs Predator ===\n";
    simulateCombatEncounter(
        logger, registry,
        3, "ApexPredator_Gamma",  // Both aggressive
        0.85f,
        4, "ApexPredator_Delta",
        0.8f,
        200  // Start tick
    );
    
    std::cout << "\n=== Test 3: Scavenging Behavior ===\n";
    testScavenging(logger, 300);
    
    std::cout << "\n=== Test 4: Creature Factory ===\n";
    testCreatureFactory();
    
    std::cout << "\n========================================\n";
    std::cout << "     Combat Diagnostic Complete\n";
    std::cout << "========================================\n";
    
    return 0;
}
