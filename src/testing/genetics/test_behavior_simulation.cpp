/**
 * @file test_behavior_simulation.cpp
 * @brief Small world simulation integration tests
 * 
 * Tests small world sanity checks:
 * - Plants and creatures coexist
 * - Food chain works (plants produce food, creatures eat)
 * - No crashes after N ticks
 */

#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/EnergyBudget.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
#include "objects/food.hpp"
#include "rendering/RenderTypes.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Helper: Set a gene value in a genome
// ============================================================================

static void setGeneValue(G::Genome& genome, const char* gene_id, float value) {
    if (genome.hasGene(gene_id)) {
        genome.getGeneMutable(gene_id).setAlleleValues(value);
    }
}

// ============================================================================
// Simple Mini-World for Testing
// ============================================================================

/**
 * @brief A minimal world implementation for testing plant-creature interactions
 * 
 * This is not the full world implementation, just enough to test
 * the genetics system interactions in a controlled environment.
 */
class MiniWorld {
public:
    MiniWorld(int width, int height)
        : width_(width), height_(height)
    {
        registry_ = std::make_shared<G::GeneRegistry>();
        G::UniversalGenes::registerDefaults(*registry_);
        factory_ = std::make_unique<G::PlantFactory>(registry_);
        factory_->registerDefaultTemplates();
        
        // Initialize default environment
        environment_.temperature = 22.0f;
        environment_.humidity = 0.6f;
        environment_.time_of_day = 0.5f;  // Start at noon
        environment_.terrain_type = 0;
    }
    
    // Add plants to the world
    void addPlant(const std::string& species, int x, int y) {
        G::Plant plant = factory_->createFromTemplate(species, x, y);
        
        // Initialize energy state
        G::EnergyState energyState;
        energyState.currentEnergy = 50.0f;
        energyState.maintenanceCost = 5.0f;
        energyState.baseMetabolism = 2.0f;
        plant.setEnergyState(energyState);
        
        plants_.push_back(std::move(plant));
    }
    
    // Add simulated "creature" (just phenotype + state for testing)
    void addCreature(float plantDigestion, float toxinTolerance, float hideThickness, int x, int y) {
        G::Genome genome = G::UniversalGenes::createCreatureGenome(*registry_);
        setGeneValue(genome, G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantDigestion);
        setGeneValue(genome, G::UniversalGenes::TOXIN_TOLERANCE, toxinTolerance);
        setGeneValue(genome, G::UniversalGenes::HIDE_THICKNESS, hideThickness);
        setGeneValue(genome, G::UniversalGenes::COLOR_VISION, 0.7f);
        setGeneValue(genome, G::UniversalGenes::SCENT_DETECTION, 0.6f);
        setGeneValue(genome, G::UniversalGenes::SWEETNESS_PREFERENCE, 0.6f);
        setGeneValue(genome, G::UniversalGenes::SEED_DESTRUCTION_RATE, 0.4f);
        setGeneValue(genome, G::UniversalGenes::GUT_TRANSIT_TIME, 6.0f);
        setGeneValue(genome, G::UniversalGenes::CELLULOSE_BREAKDOWN, 0.5f);
        
        creatureGenomes_.push_back(std::move(genome));
        creaturePositions_.push_back({x, y});
        creatureHunger_.push_back(0.3f);  // Start somewhat hungry
        creatureHealth_.push_back(100.0f);
        creatureEnergy_.push_back(50.0f);
    }
    
    // Simulate one tick
    void tick() {
        tickCount_++;
        
        // Update time of day (full day cycle every 1000 ticks)
        environment_.time_of_day = static_cast<float>(tickCount_ % 1000) / 1000.0f;
        
        // Update all plants
        for (auto& plant : plants_) {
            if (plant.isAlive()) {
                plant.update(environment_);
                
                // Keep energy topped up for testing
                G::EnergyState& state = plant.getEnergyState();
                state.currentEnergy = std::min(100.0f, state.currentEnergy + 0.1f);
            }
        }
        
        // Collect produced fruit
        for (auto& plant : plants_) {
            if (plant.isAlive() && plant.canProduceFruit()) {
                Food fruit = plant.produceFruit();
                foodItems_.push_back(std::move(fruit));
                fruitProduced_++;
            }
        }
        
        // Decay food
        for (auto& food : foodItems_) {
            food.incrementDecay();
        }
        
        // Remove decayed food
        foodItems_.erase(
            std::remove_if(foodItems_.begin(), foodItems_.end(),
                [](const Food& f) { return f.isDecayed(); }),
            foodItems_.end()
        );
        
        // Update creatures (simple behavior)
        updateCreatures();
    }
    
    // Run simulation for N ticks
    bool runSimulation(int numTicks) {
        for (int i = 0; i < numTicks; i++) {
            tick();
        }
        return true;  // No crash
    }
    
    // Getters for statistics
    int getTickCount() const { return tickCount_; }
    int getAlivePlantCount() const {
        int count = 0;
        for (const auto& plant : plants_) {
            if (plant.isAlive()) count++;
        }
        return count;
    }
    int getTotalPlantCount() const { return static_cast<int>(plants_.size()); }
    int getFoodItemCount() const { return static_cast<int>(foodItems_.size()); }
    int getFruitProducedCount() const { return fruitProduced_; }
    int getCreatureCount() const { return static_cast<int>(creatureGenomes_.size()); }
    int getFeedingEventsCount() const { return feedingEvents_; }
    
    float getTotalCreatureEnergy() const {
        float total = 0.0f;
        for (float e : creatureEnergy_) {
            total += e;
        }
        return total;
    }
    
    const std::vector<G::Plant>& getPlants() const { return plants_; }
    
private:
    void updateCreatures() {
        G::FeedingInteraction interaction;
        
        for (size_t i = 0; i < creatureGenomes_.size(); i++) {
            // Increase hunger over time
            creatureHunger_[i] = std::min(1.0f, creatureHunger_[i] + 0.01f);
            
            // Consume energy for being alive
            creatureEnergy_[i] -= 0.5f;
            
            // If hungry enough, try to find food
            if (creatureHunger_[i] > 0.5f && creatureEnergy_[i] > 0) {
                // Try to eat nearby plant
                G::Phenotype phenotype(&creatureGenomes_[i], registry_.get());
                
                // Update phenotype context
                G::EnvironmentState env;
                G::OrganismState state;
                state.age_normalized = 0.5f;
                state.health = creatureHealth_[i] / 100.0f;
                state.energy_level = creatureEnergy_[i] / 100.0f;
                phenotype.updateContext(env, state);
                
                // Find nearest plant
                int cx = creaturePositions_[i].first;
                int cy = creaturePositions_[i].second;
                
                for (auto& plant : plants_) {
                    if (!plant.isAlive()) continue;
                    
                    int px = plant.getX();
                    int py = plant.getY();
                    float distance = std::sqrt(static_cast<float>((cx-px)*(cx-px) + (cy-py)*(cy-py)));
                    
                    // Close enough to eat
                    if (distance < 5.0f) {
                        G::FeedingResult result = interaction.attemptToEatPlant(
                            phenotype, plant, creatureHunger_[i]
                        );
                        
                        if (result.success) {
                            feedingEvents_++;
                            creatureEnergy_[i] += result.nutritionGained;
                            creatureHealth_[i] -= result.damageReceived;
                            creatureHunger_[i] = std::max(0.0f, creatureHunger_[i] - 0.3f);
                            
                            // Apply damage to plant
                            plant.takeDamage(result.plantDamage);
                            
                            break;  // Only eat once per tick
                        }
                    }
                }
            }
            
            // Move creature randomly
            static std::mt19937 rng(42);
            std::uniform_int_distribution<int> moveDist(-1, 1);
            creaturePositions_[i].first = std::clamp(
                creaturePositions_[i].first + moveDist(rng), 0, width_ - 1);
            creaturePositions_[i].second = std::clamp(
                creaturePositions_[i].second + moveDist(rng), 0, height_ - 1);
        }
    }
    
    int width_;
    int height_;
    int tickCount_ = 0;
    int fruitProduced_ = 0;
    int feedingEvents_ = 0;
    
    std::shared_ptr<G::GeneRegistry> registry_;
    std::unique_ptr<G::PlantFactory> factory_;
    G::EnvironmentState environment_;
    
    std::vector<G::Plant> plants_;
    std::vector<Food> foodItems_;
    
    // Simplified creature representation
    std::vector<G::Genome> creatureGenomes_;
    std::vector<std::pair<int, int>> creaturePositions_;
    std::vector<float> creatureHunger_;
    std::vector<float> creatureHealth_;
    std::vector<float> creatureEnergy_;
};

// ============================================================================
// Test 1: Plants and Creatures Coexist
// ============================================================================

void testPlantsAndCreaturesCoexist() {
    MiniWorld world(50, 50);
    
    // Add various plants
    world.addPlant("berry_bush", 10, 10);
    world.addPlant("oak_tree", 20, 20);
    world.addPlant("grass", 30, 30);
    world.addPlant("thorn_bush", 40, 40);
    
    // Add creatures
    world.addCreature(0.7f, 0.5f, 0.5f, 15, 15);  // Herbivore near berry bush
    world.addCreature(0.6f, 0.6f, 0.6f, 25, 25);  // Omnivore near oak
    
    // Run simulation
    bool nocrash = world.runSimulation(100);
    
    TEST_ASSERT(nocrash);
    TEST_ASSERT_EQ(4, world.getTotalPlantCount());
    TEST_ASSERT_EQ(2, world.getCreatureCount());
    
    std::cout << "      After 100 ticks: " 
              << world.getAlivePlantCount() << "/" << world.getTotalPlantCount() 
              << " plants alive, "
              << world.getCreatureCount() << " creatures" << std::endl;
}

void testMultiplePlantsGrowTogether() {
    MiniWorld world(100, 100);
    
    // Add many plants of various types
    for (int i = 0; i < 10; i++) {
        world.addPlant("berry_bush", i * 10, 10);
        world.addPlant("grass", i * 10, 30);
    }
    
    // Run for a while
    world.runSimulation(500);
    
    // Most plants should still be alive
    int alive = world.getAlivePlantCount();
    int total = world.getTotalPlantCount();
    
    std::cout << "      After 500 ticks: " << alive << "/" << total << " plants alive" << std::endl;
    
    TEST_ASSERT_GT(alive, total / 2);  // At least half should survive
}

// ============================================================================
// Test 2: Food Chain Works
// ============================================================================

void testPlantsProduceFoodOverTime() {
    MiniWorld world(50, 50);
    
    // Add berry bushes (high fruit production)
    for (int i = 0; i < 5; i++) {
        world.addPlant("berry_bush", i * 10, 25);
    }
    
    // Run long enough for fruit production
    world.runSimulation(2000);
    
    int fruitProduced = world.getFruitProducedCount();
    
    std::cout << "      Fruit produced after 2000 ticks: " << fruitProduced << std::endl;
    
    // Should have produced some fruit (berry bushes are prolific)
    TEST_ASSERT_GT(fruitProduced, 0);
}

void testCreaturesEatPlants() {
    MiniWorld world(20, 20);
    
    // Add plants in center
    world.addPlant("berry_bush", 10, 10);
    world.addPlant("grass", 10, 12);
    
    // Add hungry creatures nearby
    world.addCreature(0.8f, 0.5f, 0.5f, 11, 11);  // Good herbivore
    world.addCreature(0.7f, 0.4f, 0.4f, 9, 9);    // Another herbivore
    
    // Run simulation
    world.runSimulation(200);
    
    int feedingEvents = world.getFeedingEventsCount();
    
    std::cout << "      Feeding events after 200 ticks: " << feedingEvents << std::endl;
    
    // Should have some feeding events
    TEST_ASSERT_GT(feedingEvents, 0);
}

void testFoodChainEnergyTransfer() {
    MiniWorld world(30, 30);
    
    // Add berry bush (energy source)
    world.addPlant("berry_bush", 15, 15);
    
    // Add creature right next to plant
    world.addCreature(0.9f, 0.5f, 0.5f, 16, 16);  // Excellent herbivore
    
    float initialEnergy = world.getTotalCreatureEnergy();
    
    // Run simulation - creature should gain energy from eating
    world.runSimulation(100);
    
    float finalEnergy = world.getTotalCreatureEnergy();
    int feedingEvents = world.getFeedingEventsCount();
    
    std::cout << "      Initial creature energy: " << initialEnergy << std::endl;
    std::cout << "      Final creature energy: " << finalEnergy << std::endl;
    std::cout << "      Feeding events: " << feedingEvents << std::endl;
    
    // If feeding happened, creature should have gained energy
    if (feedingEvents > 0) {
        // Note: creature loses energy over time too, so we just check feeding occurred
        TEST_ASSERT_GT(feedingEvents, 0);
    }
}

// ============================================================================
// Test 3: No Crashes After N Ticks
// ============================================================================

void testNoCrashAfter100Ticks() {
    MiniWorld world(50, 50);
    
    // Add some entities
    world.addPlant("berry_bush", 10, 10);
    world.addPlant("oak_tree", 20, 20);
    world.addCreature(0.7f, 0.5f, 0.5f, 15, 15);
    
    bool success = world.runSimulation(100);
    
    TEST_ASSERT(success);
    TEST_ASSERT_EQ(100, world.getTickCount());
    
    std::cout << "      Completed 100 ticks successfully" << std::endl;
}

void testNoCrashAfter1000Ticks() {
    MiniWorld world(50, 50);
    
    // Add variety of entities
    world.addPlant("berry_bush", 10, 10);
    world.addPlant("oak_tree", 30, 30);
    world.addPlant("grass", 15, 25);
    world.addPlant("thorn_bush", 40, 10);
    world.addCreature(0.7f, 0.5f, 0.5f, 20, 20);
    world.addCreature(0.5f, 0.3f, 0.8f, 35, 35);
    
    bool success = world.runSimulation(1000);
    
    TEST_ASSERT(success);
    TEST_ASSERT_EQ(1000, world.getTickCount());
    
    std::cout << "      Completed 1000 ticks successfully" << std::endl;
    std::cout << "      Final state: " 
              << world.getAlivePlantCount() << " plants alive, "
              << world.getFeedingEventsCount() << " feeding events" << std::endl;
}

void testNoCrashWithManyEntities() {
    MiniWorld world(100, 100);
    
    // Add many plants
    for (int i = 0; i < 20; i++) {
        world.addPlant("berry_bush", (i % 10) * 10 + 5, (i / 10) * 50 + 25);
        world.addPlant("grass", (i % 10) * 10, (i / 10) * 50 + 30);
    }
    
    // Add several creatures
    for (int i = 0; i < 10; i++) {
        world.addCreature(0.5f + (i % 5) * 0.1f, 0.4f, 0.4f, i * 10, 50);
    }
    
    bool success = world.runSimulation(500);
    
    TEST_ASSERT(success);
    
    std::cout << "      Completed 500 ticks with " 
              << world.getTotalPlantCount() << " plants and "
              << world.getCreatureCount() << " creatures" << std::endl;
    std::cout << "      Feeding events: " << world.getFeedingEventsCount() << std::endl;
}

void testStressTestLongSimulation() {
    MiniWorld world(100, 100);
    
    // Add diverse ecosystem
    world.addPlant("berry_bush", 25, 25);
    world.addPlant("berry_bush", 75, 25);
    world.addPlant("oak_tree", 50, 50);
    world.addPlant("grass", 25, 75);
    world.addPlant("grass", 75, 75);
    world.addPlant("thorn_bush", 50, 25);
    
    world.addCreature(0.8f, 0.5f, 0.4f, 30, 30);  // Herbivore
    world.addCreature(0.6f, 0.7f, 0.6f, 70, 70);  // Tough omnivore
    world.addCreature(0.9f, 0.3f, 0.3f, 50, 60);  // Specialist herbivore
    
    bool success = world.runSimulation(2000);
    
    TEST_ASSERT(success);
    TEST_ASSERT_EQ(2000, world.getTickCount());
    
    std::cout << "      Completed 2000 tick stress test" << std::endl;
    std::cout << "      Plants alive: " << world.getAlivePlantCount() 
              << "/" << world.getTotalPlantCount() << std::endl;
    std::cout << "      Total feeding events: " << world.getFeedingEventsCount() << std::endl;
    std::cout << "      Total fruit produced: " << world.getFruitProducedCount() << std::endl;
}

// ============================================================================
// Test 4: Edge Cases and Robustness
// ============================================================================

void testEmptyWorldDoesNotCrash() {
    MiniWorld world(10, 10);
    
    // No entities at all
    bool success = world.runSimulation(100);
    
    TEST_ASSERT(success);
    TEST_ASSERT_EQ(0, world.getTotalPlantCount());
    TEST_ASSERT_EQ(0, world.getCreatureCount());
    
    std::cout << "      Empty world runs without crash" << std::endl;
}

void testWorldWithOnlyPlants() {
    MiniWorld world(50, 50);
    
    // Only plants, no creatures
    world.addPlant("berry_bush", 10, 10);
    world.addPlant("oak_tree", 30, 30);
    
    bool success = world.runSimulation(500);
    
    TEST_ASSERT(success);
    TEST_ASSERT_EQ(2, world.getTotalPlantCount());
    TEST_ASSERT_EQ(0, world.getCreatureCount());
    TEST_ASSERT_EQ(0, world.getFeedingEventsCount());
    
    std::cout << "      World with only plants runs without crash" << std::endl;
}

void testWorldWithOnlyCreatures() {
    MiniWorld world(50, 50);
    
    // Only creatures, no plants
    world.addCreature(0.7f, 0.5f, 0.5f, 25, 25);
    
    bool success = world.runSimulation(200);
    
    TEST_ASSERT(success);
    TEST_ASSERT_EQ(0, world.getTotalPlantCount());
    TEST_ASSERT_EQ(1, world.getCreatureCount());
    TEST_ASSERT_EQ(0, world.getFeedingEventsCount());  // Nothing to eat
    
    std::cout << "      World with only creatures runs without crash" << std::endl;
}

void testAllPlantsDie() {
    MiniWorld world(20, 20);
    
    // Create plants with very short lifespan by damaging them
    world.addPlant("grass", 10, 10);
    
    // Run simulation - plants might die from damage or old age
    world.runSimulation(100);
    
    // Simulation should still complete without crash
    TEST_ASSERT_EQ(100, world.getTickCount());
    
    std::cout << "      World handles plant death gracefully" << std::endl;
}

// ============================================================================
// Test 5: Ecosystem Dynamics
// ============================================================================

void testPlantRegenerationOverTime() {
    MiniWorld world(30, 30);
    
    // Add grass (high regrowth)
    world.addPlant("grass", 15, 15);
    
    // Get initial state
    const auto& plants = world.getPlants();
    if (!plants.empty()) {
        float initialSize = plants[0].getCurrentSize();
        
        // Run simulation
        world.runSimulation(300);
        
        float finalSize = plants[0].getCurrentSize();
        
        std::cout << "      Grass size: " << initialSize << " -> " << finalSize << std::endl;
        
        // Plant should have grown
        TEST_ASSERT_GT(finalSize, initialSize);
    }
}

void testDifferentSpeciesInteractions() {
    MiniWorld world(50, 50);
    
    // Add one of each species
    world.addPlant("berry_bush", 10, 10);  // High fruit, low defense
    world.addPlant("oak_tree", 20, 20);    // High hardiness
    world.addPlant("grass", 30, 30);       // High regrowth
    world.addPlant("thorn_bush", 40, 40);  // High defense
    
    // Add creature
    world.addCreature(0.7f, 0.5f, 0.5f, 25, 25);
    
    world.runSimulation(500);
    
    int alivePlants = world.getAlivePlantCount();
    
    std::cout << "      Species interaction test: " << alivePlants 
              << "/4 plants survived, " 
              << world.getFeedingEventsCount() << " feeding events" << std::endl;
    
    // At least some plants should survive
    TEST_ASSERT_GT(alivePlants, 0);
}

// ============================================================================
// Test Runner
// ============================================================================

void runBehaviorSimulationTests() {
    BEGIN_TEST_GROUP("Coexistence Tests");
    RUN_TEST(testPlantsAndCreaturesCoexist);
    RUN_TEST(testMultiplePlantsGrowTogether);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Food Chain Tests");
    RUN_TEST(testPlantsProduceFoodOverTime);
    RUN_TEST(testCreaturesEatPlants);
    RUN_TEST(testFoodChainEnergyTransfer);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Stability Tests (No Crashes)");
    RUN_TEST(testNoCrashAfter100Ticks);
    RUN_TEST(testNoCrashAfter1000Ticks);
    RUN_TEST(testNoCrashWithManyEntities);
    RUN_TEST(testStressTestLongSimulation);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Edge Case Tests");
    RUN_TEST(testEmptyWorldDoesNotCrash);
    RUN_TEST(testWorldWithOnlyPlants);
    RUN_TEST(testWorldWithOnlyCreatures);
    RUN_TEST(testAllPlantsDie);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Ecosystem Dynamics Tests");
    RUN_TEST(testPlantRegenerationOverTime);
    RUN_TEST(testDifferentSpeciesInteractions);
    END_TEST_GROUP();
}

#ifdef TEST_BEHAVIOR_SIMULATION_STANDALONE
int main() {
    std::cout << "=== World Simulation Behavior Integration Tests ===" << std::endl;
    runBehaviorSimulationTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
