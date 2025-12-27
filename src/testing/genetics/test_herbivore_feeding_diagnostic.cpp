/**
 * @file test_herbivore_feeding_diagnostic.cpp
 * @brief Diagnostic test to observe hungry herbivore feeding behavior
 *
 * Creates a controlled test environment:
 * - 50x50 world with center at (25, 25)
 * - 4 of each plant prefab (mature) at controlled distances:
 *   - First of each type at distance 2 (close)
 *   - Second at distance 5 (short)
 *   - Third at distance 10 (medium)
 *   - Fourth at distance 25 (far)
 * - All herbivores placed at center (25, 25) for fair comparison
 * - Logs all feeding attempts and outcomes
 *
 * Plant Labels:
 *   B1, B2, B3, B4 = berry_bush at distances 2, 5, 10, 25
 *   G1, G2, G3, G4 = grass at distances 2, 5, 10, 25
 *   O1, O2, O3, O4 = oak_tree at distances 2, 5, 10, 25
 *   T1, T2, T3, T4 = thorn_bush at distances 2, 5, 10, 25
 *
 * Purpose: Empirically determine why herbivores wander instead of eating
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>

#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/EnergyBudget.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "objects/creature/creature.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Constants for controlled layout
// ============================================================================
const int CENTER_X = 25;
const int CENTER_Y = 25;
const int DIST_CLOSE = 2;
const int DIST_SHORT = 5;
const int DIST_MEDIUM = 10;
const int DIST_FAR = 25;

// Plant type letters
const char* PLANT_LETTERS[] = {"B", "G", "O", "T"};  // Berry, Grass, Oak, Thorn
// Labels: 1=close(2), 2=short(5), 3=medium(10), 4=far(25)

// ============================================================================
// Diagnostic Helpers
// ============================================================================

void printSeparator(const std::string& title) {
    std::cout << "\n========================================" << std::endl;
    std::cout << title << std::endl;
    std::cout << "========================================\n" << std::endl;
}

std::string getPlantLabel(const std::string& plantType, int instance) {
    char letter = '?';
    if (plantType == "berry_bush") letter = 'B';
    else if (plantType == "grass") letter = 'G';
    else if (plantType == "oak_tree") letter = 'O';
    else if (plantType == "thorn_bush") letter = 'T';
    return std::string(1, letter) + std::to_string(instance + 1);
}

float getDistanceFromCenter(int x, int y) {
    int dx = x - CENTER_X;
    int dy = y - CENTER_Y;
    return std::sqrt(static_cast<float>(dx * dx + dy * dy));
}

void printPlantInfo(const G::Plant& plant, const std::string& label) {
    float dist = getDistanceFromCenter(plant.getX(), plant.getY());
    std::cout << label << " at (" << plant.getX() << "," << plant.getY()
              << ") dist=" << std::fixed << std::setprecision(1) << dist << std::endl;
    std::cout << "  Size: " << std::setprecision(2) << plant.getCurrentSize()
              << "/" << plant.getMaxSize() << std::endl;
    std::cout << "  Nutrition: " << plant.getNutrientValue()
              << "  Appeal: " << plant.getFruitAppeal() << std::endl;
    std::cout << "  Thorns: " << plant.getThornDamage()
              << "  Toxicity: " << plant.getToxicity() << std::endl;
}

void printCreatureInfo(const Creature& creature, const std::string& label) {
    std::cout << label << " (" << creature.getArchetypeLabel() << ") at ("
              << creature.getX() << "," << creature.getY() << ")" << std::endl;
    
    DietType diet = creature.getDietType();
    std::cout << "  Diet: ";
    switch (diet) {
        case DietType::HERBIVORE: std::cout << "HERBIVORE"; break;
        case DietType::CARNIVORE: std::cout << "CARNIVORE"; break;
        case DietType::OMNIVORE: std::cout << "OMNIVORE"; break;
    }
    
    const G::Phenotype* phenotype = creature.getPhenotype();
    if (phenotype) {
        std::cout << "  PlantDig=" << std::fixed << std::setprecision(2)
                  << phenotype->getTrait(G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY)
                  << "  ColorVis=" << phenotype->getTrait(G::UniversalGenes::COLOR_VISION)
                  << "  Scent=" << phenotype->getTrait(G::UniversalGenes::SCENT_DETECTION)
                  << "  Sight=" << phenotype->getTrait(G::UniversalGenes::SIGHT_RANGE) << std::endl;
    }
}

// ============================================================================
// Test FeedingInteraction directly
// ============================================================================

void testFeedingInteractionDirect(const Creature& creature, const std::string& creatureLabel,
                                   G::Plant& plant, const std::string& plantLabel) {
    const G::Phenotype* phenotype = creature.getPhenotype();
    if (!phenotype) {
        std::cout << "  ERROR: Creature has no phenotype!" << std::endl;
        return;
    }
    
    // Calculate actual distance
    float dist = getDistanceFromCenter(plant.getX(), plant.getY());
    
    G::FeedingInteraction interaction;
    
    // Test canDetectPlant at actual distance
    bool canDetect = interaction.canDetectPlant(*phenotype, plant, dist);
    float detectionRange = interaction.getDetectionRange(*phenotype, plant);
    bool canOvercome = interaction.canOvercomeDefenses(*phenotype, plant);
    
    // Test attemptToEatPlant with high hunger (0.9)
    G::FeedingResult result = interaction.attemptToEatPlant(*phenotype, plant, 0.9f);
    
    // Single-line summary
    std::cout << creatureLabel << " vs " << plantLabel << " (d=" << std::fixed << std::setprecision(1)
              << dist << ", range=" << detectionRange << "): ";
    
    if (dist > detectionRange) {
        std::cout << "OUT OF RANGE";
    } else if (!canDetect) {
        std::cout << "CANNOT DETECT";
    } else if (!canOvercome) {
        std::cout << "CANNOT OVERCOME DEFENSES";
    } else if (result.success) {
        std::cout << "SUCCESS (nutr=" << std::setprecision(1) << result.nutritionGained << ")";
    } else {
        std::cout << "FAILED: " << result.description;
    }
    std::cout << std::endl;
}

// ============================================================================
// Main Diagnostic
// ============================================================================

struct PlantWithLabel {
    G::Plant plant;
    std::string label;
    std::string type;
};

int main() {
    std::cout << "================================================================" << std::endl;
    std::cout << "  HERBIVORE FEEDING DIAGNOSTIC v3" << std::endl;
    std::cout << "  Controlled layout with distance-based testing" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "\nLayout:" << std::endl;
    std::cout << "  All creatures at CENTER (" << CENTER_X << "," << CENTER_Y << ")" << std::endl;
    std::cout << "  Plants at distances: " << DIST_CLOSE << " (close), " << DIST_SHORT << " (short), "
              << DIST_MEDIUM << " (medium), " << DIST_FAR << " (far)" << std::endl;
    std::cout << "  Labels: B=Berry, G=Grass, O=Oak, T=Thorn" << std::endl;
    std::cout << "           1=close(2), 2=short(5), 3=medium(10), 4=far(25)" << std::endl;
    
    // Initialize gene registry
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    // Create factories
    G::PlantFactory plantFactory(registry);
    plantFactory.registerDefaultTemplates();
    
    G::CreatureFactory creatureFactory(registry);
    creatureFactory.registerDefaultTemplates();
    
    // ========================================================================
    // Create Plants at controlled distances
    // ========================================================================
    printSeparator("CREATING PLANTS");
    
    std::vector<PlantWithLabel> plants;
    std::vector<std::string> plantTypes = {"berry_bush", "grass", "oak_tree", "thorn_bush"};
    int distances[] = {DIST_CLOSE, DIST_SHORT, DIST_MEDIUM, DIST_FAR};
    
    // Environment for growing plants
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.humidity = 0.6f;
    env.time_of_day = 0.5f;
    env.terrain_type = 0;
    
    // Place plants in different directions from center
    // Type 0 (Berry): East direction (+x)
    // Type 1 (Grass): North direction (+y)
    // Type 2 (Oak): West direction (-x)
    // Type 3 (Thorn): South direction (-y)
    int typeIdx = 0;
    for (const auto& plantType : plantTypes) {
        for (int i = 0; i < 4; i++) {
            int dist = distances[i];
            int px = CENTER_X, py = CENTER_Y;
            
            switch (typeIdx) {
                case 0: px = CENTER_X + dist; break;  // Berry: East
                case 1: py = CENTER_Y + dist; break;  // Grass: North
                case 2: px = CENTER_X - dist; break;  // Oak: West
                case 3: py = CENTER_Y - dist; break;  // Thorn: South
            }
            
            G::Plant plant = plantFactory.createFromTemplate(plantType, px, py);
            
            // Initialize energy
            G::EnergyState energyState;
            energyState.currentEnergy = 100.0f;
            energyState.maintenanceCost = 5.0f;
            energyState.baseMetabolism = 2.0f;
            plant.setEnergyState(energyState);
            
            // Grow plant to maturity (1000 ticks)
            for (int tick = 0; tick < 1000; tick++) {
                plant.update(env);
                plant.getEnergyState().currentEnergy = 100.0f;
            }
            
            std::string label = getPlantLabel(plantType, i);
            plants.push_back({std::move(plant), label, plantType});
        }
        typeIdx++;
    }
    
    std::cout << "Created " << plants.size() << " plants:\n" << std::endl;
    for (auto& p : plants) {
        printPlantInfo(p.plant, p.label);
    }
    
    // ========================================================================
    // Create Creatures ALL at center position
    // ========================================================================
    printSeparator("CREATING CREATURES (all at center)");
    
    Creature::initializeGeneRegistry();
    Creature::initializeInteractionSystems();
    
    std::vector<std::string> herbivoreTemplates = {
        "tank_herbivore",
        "armored_grazer",
        "fleet_runner",
        "spiky_defender",
        "canopy_forager",
        "omnivore_generalist"
    };
    
    struct CreatureWithLabel {
        Creature creature;
        std::string label;
    };
    std::vector<CreatureWithLabel> creatures;
    int creatureNum = 1;
    
    for (const auto& tmpl : herbivoreTemplates) {
        if (creatureFactory.hasTemplate(tmpl)) {
            // ALL creatures at center
            Creature c = creatureFactory.createFromTemplate(tmpl, CENTER_X, CENTER_Y);
            c.setHunger(0.1f);  // Very hungry
            c.setThirst(Creature::RESOURCE_LIMIT);  // Not thirsty
            
            std::string label = "C" + std::to_string(creatureNum++);
            creatures.push_back({std::move(c), label});
        }
    }
    
    std::cout << "Created " << creatures.size() << " creatures at center:\n" << std::endl;
    for (auto& c : creatures) {
        printCreatureInfo(c.creature, c.label);
    }
    
    // ========================================================================
    // Test: Close plants (distance 2) - Should ALL succeed
    // ========================================================================
    printSeparator("CLOSE RANGE TESTS (distance " + std::to_string(DIST_CLOSE) + ")");
    
    std::cout << "Testing each creature vs each plant type at CLOSE range:\n" << std::endl;
    for (auto& c : creatures) {
        std::cout << c.label << " (" << c.creature.getArchetypeLabel() << "):" << std::endl;
        for (auto& p : plants) {
            if (p.label.back() == '1') {  // Close plants only
                testFeedingInteractionDirect(c.creature, c.label, p.plant, p.label);
            }
        }
        std::cout << std::endl;
    }
    
    // ========================================================================
    // Test: Short distance (5) - Critical for typical movement range
    // ========================================================================
    printSeparator("SHORT RANGE TESTS (distance " + std::to_string(DIST_SHORT) + ")");
    
    std::cout << "Testing each creature vs each plant type at SHORT range:\n" << std::endl;
    for (auto& c : creatures) {
        std::cout << c.label << " (" << c.creature.getArchetypeLabel() << "):" << std::endl;
        for (auto& p : plants) {
            if (p.label.back() == '2') {  // Short plants only
                testFeedingInteractionDirect(c.creature, c.label, p.plant, p.label);
            }
        }
        std::cout << std::endl;
    }
    
    // ========================================================================
    // Test: Medium distance (10) - May start failing for some
    // ========================================================================
    printSeparator("MEDIUM RANGE TESTS (distance " + std::to_string(DIST_MEDIUM) + ")");
    
    std::cout << "Testing each creature vs each plant type at MEDIUM range:\n" << std::endl;
    for (auto& c : creatures) {
        std::cout << c.label << " (" << c.creature.getArchetypeLabel() << "):" << std::endl;
        for (auto& p : plants) {
            if (p.label.back() == '3') {  // Medium plants only
                testFeedingInteractionDirect(c.creature, c.label, p.plant, p.label);
            }
        }
        std::cout << std::endl;
    }
    
    // ========================================================================
    // Test: Far distance (25) - Likely many failures
    // ========================================================================
    printSeparator("FAR RANGE TESTS (distance " + std::to_string(DIST_FAR) + ")");
    
    std::cout << "Testing each creature vs each plant type at FAR range:\n" << std::endl;
    for (auto& c : creatures) {
        std::cout << c.label << " (" << c.creature.getArchetypeLabel() << "):" << std::endl;
        for (auto& p : plants) {
            if (p.label.back() == '4') {  // Far plants only
                testFeedingInteractionDirect(c.creature, c.label, p.plant, p.label);
            }
        }
        std::cout << std::endl;
    }
    
    // ========================================================================
    // Summary Table
    // ========================================================================
    printSeparator("SUMMARY");
    
    std::cout << "Expected Results:" << std::endl;
    std::cout << "- Close (d=2): ALL herbivores should succeed with ALL plant types" << std::endl;
    std::cout << "- Short (d=5): Should succeed - typical movement range" << std::endl;
    std::cout << "- Medium (d=10): May fail if detection range < 10" << std::endl;
    std::cout << "- Far (d=25): Likely fail for most creatures" << std::endl;
    std::cout << std::endl;
    std::cout << "If CLOSE range fails, the issue is NOT distance." << std::endl;
    std::cout << "Check for: detection threshold, defense threshold, or attraction failure." << std::endl;
    
    return 0;
}
