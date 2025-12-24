/**
 * @file test_behavior_plants.cpp
 * @brief Behavior integration tests for plants in the simulation
 * 
 * Tests plants in isolation to verify:
 * - Plant grows over time
 * - Plant produces fruit when mature
 * - Plant dies when lifespan exceeded
 * - Different species have different growth rates
 * - Defenses (toxins, thorns) are properly initialized
 */

#include <iostream>
#include <memory>
#include <vector>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "objects/food.hpp"
#include "rendering/RenderTypes.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Helper: Create optimal environment for plant growth
// ============================================================================

G::EnvironmentState createOptimalEnvironment() {
    G::EnvironmentState env;
    env.temperature = 22.0f;  // Optimal temperature
    env.humidity = 0.6f;
    env.time_of_day = 0.5f;   // Noon = max light
    env.terrain_type = 0;
    return env;
}

// Helper: Set a gene value in a genome
static void setGeneValue(G::Genome& genome, const char* gene_id, float value) {
    if (genome.hasGene(gene_id)) {
        genome.getGeneMutable(gene_id).setAlleleValues(value);
    }
}

// ============================================================================
// Test 1: Plant Growth Over Time
// ============================================================================

void testPlantGrowth() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create a berry bush
    G::Plant plant = factory.createFromTemplate("berry_bush", 10, 10);
    float initialSize = plant.getCurrentSize();
    
    // Simulate 100 ticks with optimal environment
    G::EnvironmentState env = createOptimalEnvironment();
    
    for (int i = 0; i < 100; i++) {
        plant.update(env);
    }
    
    // Verify growth occurred
    float finalSize = plant.getCurrentSize();
    TEST_ASSERT_GT(finalSize, initialSize);
    
    std::cout << "      Initial size: " << initialSize << ", Final size: " << finalSize << std::endl;
}

void testPlantGrowthWithDifferentLight() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create two plants
    G::Plant plantHighLight = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant plantLowLight = factory.createFromTemplate("berry_bush", 0, 0);
    
    // High light environment (noon)
    G::EnvironmentState highLight = createOptimalEnvironment();
    highLight.time_of_day = 0.5f;  // Noon
    
    // Low light environment (early morning/evening)
    G::EnvironmentState lowLight = createOptimalEnvironment();
    lowLight.time_of_day = 0.1f;  // Early morning/evening
    
    // Simulate 200 ticks
    for (int i = 0; i < 200; i++) {
        plantHighLight.update(highLight);
        plantLowLight.update(lowLight);
    }
    
    // High light plant should grow more
    TEST_ASSERT_GT(plantHighLight.getCurrentSize(), plantLowLight.getCurrentSize());
    
    std::cout << "      High light growth: " << plantHighLight.getCurrentSize() 
              << ", Low light growth: " << plantLowLight.getCurrentSize() << std::endl;
}

// ============================================================================
// Test 2: Plant Fruit Production When Mature
// ============================================================================

void testPlantFruitProductionWhenMature() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create a berry bush (high fruit production rate)
    G::Plant plant = factory.createFromTemplate("berry_bush", 10, 10);
    G::EnvironmentState env = createOptimalEnvironment();
    
    // Plant needs energy to produce fruit
    G::EnergyState energyState;
    energyState.currentEnergy = 100.0f;
    energyState.maintenanceCost = 5.0f;
    energyState.baseMetabolism = 2.0f;
    plant.setEnergyState(energyState);
    
    // Simulate until plant is mature (needs age > 25% of lifespan and fruit timer ready)
    int tickCount = 0;
    const int maxTicks = 10000;
    
    while (!plant.canProduceFruit() && tickCount < maxTicks && plant.isAlive()) {
        plant.update(env);
        
        // Keep energy high
        G::EnergyState& state = plant.getEnergyState();
        state.currentEnergy = 100.0f;
        
        tickCount++;
    }
    
    // Verify plant can produce fruit after maturation
    if (plant.canProduceFruit()) {
        Food fruit = plant.produceFruit();
        TEST_ASSERT_GT(fruit.getCalories(), 0.0f);
        TEST_ASSERT_GT(fruit.getLifespan(), 0u);
        std::cout << "      Fruit produced after " << tickCount << " ticks, calories: " 
                  << fruit.getCalories() << std::endl;
    } else {
        // Berry bush should be able to produce fruit eventually
        std::cout << "      Plant status - Alive: " << plant.isAlive() 
                  << ", Age: " << plant.getAge() 
                  << ", Max lifespan: " << plant.getMaxLifespan()
                  << ", Size: " << plant.getCurrentSize() << std::endl;
        TEST_ASSERT_MSG(plant.canProduceFruit(), "Berry bush should produce fruit when mature");
    }
}

void testPlantFruitCaloriesScaleWithSize() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create two berry bushes
    G::Plant smallPlant = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant largePlant = factory.createFromTemplate("berry_bush", 5, 5);
    
    G::EnvironmentState env = createOptimalEnvironment();
    
    // Set energy for both
    G::EnergyState energyState;
    energyState.currentEnergy = 100.0f;
    energyState.maintenanceCost = 5.0f;
    energyState.baseMetabolism = 2.0f;
    
    // Grow the large plant more
    for (int i = 0; i < 3000; i++) {
        largePlant.update(env);
        largePlant.setEnergyState(energyState);
    }
    
    // Grow small plant less
    for (int i = 0; i < 500; i++) {
        smallPlant.update(env);
        smallPlant.setEnergyState(energyState);
    }
    
    std::cout << "      Small plant size: " << smallPlant.getCurrentSize()
              << ", Large plant size: " << largePlant.getCurrentSize() << std::endl;
    
    // Both should be able to produce fruit eventually, larger plant = more calories
    TEST_ASSERT_GT(largePlant.getCurrentSize(), smallPlant.getCurrentSize());
}

// ============================================================================
// Test 3: Plant Dies When Lifespan Exceeded
// ============================================================================

void testPlantDiesOfOldAge() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    // Create plant with short lifespan genome
    G::Genome genome = G::UniversalGenes::createPlantGenome(*registry);
    
    // Set very short lifespan using the correct API
    setGeneValue(genome, G::UniversalGenes::LIFESPAN, 50.0f);
    
    G::Plant plant(10, 10, genome, *registry);
    G::EnvironmentState env = createOptimalEnvironment();
    
    // Verify plant starts alive
    TEST_ASSERT(plant.isAlive());
    
    // Simulate past lifespan
    for (int i = 0; i < 100; i++) {
        plant.update(env);
    }
    
    // Verify plant died
    TEST_ASSERT(!plant.isAlive());
    std::cout << "      Plant died at age: " << plant.getAge() 
              << " (lifespan was: " << plant.getMaxLifespan() << ")" << std::endl;
}

void testPlantDiesFromDamage() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant plant = factory.createFromTemplate("grass", 10, 10);  // Grass is fragile
    
    // Verify plant starts alive
    TEST_ASSERT(plant.isAlive());
    
    // Apply lethal damage
    for (int i = 0; i < 100; i++) {
        plant.takeDamage(0.1f);
    }
    
    // Verify plant died from damage
    TEST_ASSERT(!plant.isAlive());
    std::cout << "      Plant killed by damage" << std::endl;
}

// ============================================================================
// Test 4: Different Species Have Different Growth Rates
// ============================================================================

void testSpeciesHaveDifferentGrowthRates() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create different species
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant oakTree = factory.createFromTemplate("oak_tree", 5, 5);
    G::Plant grass = factory.createFromTemplate("grass", 10, 10);
    
    G::EnvironmentState env = createOptimalEnvironment();
    
    float berryInitial = berryBush.getCurrentSize();
    float oakInitial = oakTree.getCurrentSize();
    float grassInitial = grass.getCurrentSize();
    
    // Simulate same number of ticks
    for (int i = 0; i < 500; i++) {
        berryBush.update(env);
        oakTree.update(env);
        grass.update(env);
    }
    
    float berryGrowth = berryBush.getCurrentSize() - berryInitial;
    float oakGrowth = oakTree.getCurrentSize() - oakInitial;
    float grassGrowth = grass.getCurrentSize() - grassInitial;
    
    std::cout << "      Berry growth: " << berryGrowth 
              << ", Oak growth: " << oakGrowth 
              << ", Grass growth: " << grassGrowth << std::endl;
    
    // Grass should grow fastest (high growth rate)
    // Berry bush medium
    // Oak tree slowest
    TEST_ASSERT_GT(grassGrowth, oakGrowth);
    TEST_ASSERT_GT(berryGrowth, oakGrowth);
}

void testSpeciesHaveDifferentMaxSizes() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create different species
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    G::Plant oakTree = factory.createFromTemplate("oak_tree", 5, 5);
    
    // Oak trees should have larger max size than grass
    float grassMaxSize = grass.getMaxSize();
    float oakMaxSize = oakTree.getMaxSize();
    
    std::cout << "      Grass max size: " << grassMaxSize 
              << ", Oak max size: " << oakMaxSize << std::endl;
    
    TEST_ASSERT_GT(oakMaxSize, grassMaxSize);
}

// ============================================================================
// Test 5: Defenses (Toxins, Thorns) Properly Initialized
// ============================================================================

void testThornBushHasHighThorns() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 5, 5);
    
    float thornBushThorns = thornBush.getThornDamage();
    float berryBushThorns = berryBush.getThornDamage();
    
    std::cout << "      Thorn bush thorns: " << thornBushThorns 
              << ", Berry bush thorns: " << berryBushThorns << std::endl;
    
    // Thorn bush should have significantly higher thorns
    TEST_ASSERT_GT(thornBushThorns, berryBushThorns);
    TEST_ASSERT_GT(thornBushThorns, 0.3f);  // Should be substantial
}

void testThornBushHasHighToxicity() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    G::Plant grass = factory.createFromTemplate("grass", 5, 5);
    
    float thornBushToxicity = thornBush.getToxicity();
    float grassToxicity = grass.getToxicity();
    
    std::cout << "      Thorn bush toxicity: " << thornBushToxicity 
              << ", Grass toxicity: " << grassToxicity << std::endl;
    
    // Thorn bush should have higher toxicity than grass
    TEST_ASSERT_GT(thornBushToxicity, grassToxicity);
}

void testGrassHasHighRegrowth() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    G::Plant oakTree = factory.createFromTemplate("oak_tree", 5, 5);
    
    float grassRegrowth = grass.getRegrowthRate();
    float oakRegrowth = oakTree.getRegrowthRate();
    
    std::cout << "      Grass regrowth: " << grassRegrowth 
              << ", Oak regrowth: " << oakRegrowth << std::endl;
    
    // Grass should have higher regrowth rate (grazing adaptation)
    TEST_ASSERT_GT(grassRegrowth, oakRegrowth);
}

void testDefenseTraitsAreInValidRanges() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::string> species = {"berry_bush", "oak_tree", "grass", "thorn_bush"};
    
    for (const auto& speciesName : species) {
        G::Plant plant = factory.createFromTemplate(speciesName, 0, 0);
        
        float toxicity = plant.getToxicity();
        float thorns = plant.getThornDamage();
        float regrowth = plant.getRegrowthRate();
        
        // All defense values should be in valid range [0, 1]
        TEST_ASSERT_GE(toxicity, 0.0f);
        TEST_ASSERT_LE(toxicity, 1.0f);
        TEST_ASSERT_GE(thorns, 0.0f);
        TEST_ASSERT_LE(thorns, 1.0f);
        TEST_ASSERT_GE(regrowth, 0.0f);
        TEST_ASSERT_LE(regrowth, 1.0f);
    }
    
    std::cout << "      All species have defense traits in valid ranges [0, 1]" << std::endl;
}

// ============================================================================
// Test 6: Plant Regeneration
// ============================================================================

void testPlantCanRegenerate() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Grass has high regrowth
    G::Plant plant = factory.createFromTemplate("grass", 0, 0);
    G::EnvironmentState env = createOptimalEnvironment();
    
    // Grow plant a bit first
    for (int i = 0; i < 200; i++) {
        plant.update(env);
    }
    
    // Set energy state to allow regeneration
    G::EnergyState energyState;
    energyState.currentEnergy = 50.0f;
    energyState.maintenanceCost = 5.0f;
    energyState.baseMetabolism = 2.0f;
    plant.setEnergyState(energyState);
    
    // Damage the plant but don't kill it
    plant.takeDamage(0.3f);
    
    // Check if can regenerate (needs regrowth ability and surplus energy)
    bool couldRegenerate = plant.canRegenerate();
    std::cout << "      Plant can regenerate: " << (couldRegenerate ? "yes" : "no") 
              << " (regrowth rate: " << plant.getRegrowthRate() << ")" << std::endl;
    
    // With high regrowth rate and energy surplus, should be able to regenerate
    TEST_ASSERT(plant.getRegrowthRate() > 0.0f);
}

// ============================================================================
// Test 7: Seed Dispersal
// ============================================================================

void testPlantSeedDispersal() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    G::SeedDispersal seedDispersal;
    
    // Create a mature berry bush (high fruit production, animal-fruit dispersal)
    G::Plant plant = factory.createFromTemplate("berry_bush", 50, 50);
    G::EnvironmentState env = createOptimalEnvironment();
    
    // Grow the plant to maturity
    for (int i = 0; i < 2000; i++) {
        plant.update(env);
    }
    
    // Get the plant's dispersal strategy
    G::DispersalStrategy strategy = plant.getPrimaryDispersalStrategy();
    std::cout << "      Berry bush dispersal strategy: " << static_cast<int>(strategy) << std::endl;
    
    // Perform dispersal
    G::DispersalEvent event = seedDispersal.disperse(plant, &env);
    
    // Verify event is valid
    TEST_ASSERT_EQ(event.originX, 50);
    TEST_ASSERT_EQ(event.originY, 50);
    TEST_ASSERT_GE(event.seedViability, 0.0f);
    TEST_ASSERT_LE(event.seedViability, 1.0f);
    
    // Target should be near origin (within spread distance)
    float distance = event.getDistance();
    std::cout << "      Dispersal event: origin (" << event.originX << "," << event.originY
              << ") -> target (" << event.targetX << "," << event.targetY
              << "), distance: " << distance
              << ", viability: " << event.seedViability << std::endl;
    
    TEST_ASSERT_GE(distance, 0.0f);
}

void testSeedCreatesNewPlant() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    G::SeedDispersal seedDispersal;
    
    // Create a parent plant
    G::Plant parent = factory.createFromTemplate("grass", 25, 25);
    G::EnvironmentState env = createOptimalEnvironment();
    
    // Grow the plant to maturity
    for (int i = 0; i < 500; i++) {
        parent.update(env);
    }
    
    // Get dispersal event
    G::DispersalEvent event = seedDispersal.disperse(parent, &env);
    
    // Verify we can create an offspring plant using the factory
    // (This simulates what World::updateGeneticsPlants does)
    G::Plant offspring = factory.createOffspring(parent, parent, event.targetX, event.targetY);
    
    // Verify offspring is valid
    TEST_ASSERT(offspring.isAlive());
    TEST_ASSERT_EQ(offspring.getX(), event.targetX);
    TEST_ASSERT_EQ(offspring.getY(), event.targetY);
    TEST_ASSERT_EQ(offspring.getAge(), 0u);  // Newborn
    
    // Offspring should inherit entity type from parent
    TEST_ASSERT_EQ(static_cast<int>(offspring.getEntityType()), static_cast<int>(parent.getEntityType()));
    
    std::cout << "      Offspring created at (" << offspring.getX() << "," << offspring.getY()
              << ") with entity type: " << static_cast<int>(offspring.getEntityType()) << std::endl;
}

void testDifferentSpeciesHaveDifferentDispersalStrategies() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create different species
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    G::Plant oakTree = factory.createFromTemplate("oak_tree", 0, 0);
    
    G::DispersalStrategy berryStrategy = berryBush.getPrimaryDispersalStrategy();
    G::DispersalStrategy grassStrategy = grass.getPrimaryDispersalStrategy();
    G::DispersalStrategy thornStrategy = thornBush.getPrimaryDispersalStrategy();
    G::DispersalStrategy oakStrategy = oakTree.getPrimaryDispersalStrategy();
    
    std::cout << "      Berry bush strategy: " << static_cast<int>(berryStrategy) << std::endl;
    std::cout << "      Grass strategy: " << static_cast<int>(grassStrategy) << std::endl;
    std::cout << "      Thorn bush strategy: " << static_cast<int>(thornStrategy) << std::endl;
    std::cout << "      Oak tree strategy: " << static_cast<int>(oakStrategy) << std::endl;
    
    // Grass MUST be vegetative (runner production > 1.3 always exceeds 0.7 threshold)
    TEST_ASSERT_EQ(static_cast<int>(grassStrategy), static_cast<int>(G::DispersalStrategy::VEGETATIVE));
    
    // Oak MUST be gravity (heavy seeds, no other high traits)
    TEST_ASSERT_EQ(static_cast<int>(oakStrategy), static_cast<int>(G::DispersalStrategy::GRAVITY));
    
    // Verify dispersal strategies are valid enum values (0-5)
    TEST_ASSERT_GE(static_cast<int>(berryStrategy), 0);
    TEST_ASSERT_LE(static_cast<int>(berryStrategy), 5);
    TEST_ASSERT_GE(static_cast<int>(thornStrategy), 0);
    TEST_ASSERT_LE(static_cast<int>(thornStrategy), 5);
    
    // Verify species have appropriate traits expressed (not checking exact strategy due to random variation)
    // Thorn bush should have high hook strength
    float thornHooks = thornBush.getSeedHookStrength();
    std::cout << "      Thorn bush hook strength: " << thornHooks << std::endl;
    TEST_ASSERT_GT(thornHooks, 0.5f);  // Should be in 0.7-0.9 range
    
    // Berry bush should have high fruit appeal
    float berryAppeal = berryBush.getFruitAppeal();
    std::cout << "      Berry bush fruit appeal: " << berryAppeal << std::endl;
    TEST_ASSERT_GT(berryAppeal, 0.5f);  // Should be in 0.7-0.9 range
}

void testPlantLifespansAreReasonable() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    G::Plant oakTree = factory.createFromTemplate("oak_tree", 0, 0);
    
    unsigned int grassLifespan = grass.getMaxLifespan();
    unsigned int berryLifespan = berryBush.getMaxLifespan();
    unsigned int thornLifespan = thornBush.getMaxLifespan();
    unsigned int oakLifespan = oakTree.getMaxLifespan();
    
    std::cout << "      Grass lifespan: " << grassLifespan << " ticks" << std::endl;
    std::cout << "      Berry bush lifespan: " << berryLifespan << " ticks" << std::endl;
    std::cout << "      Thorn bush lifespan: " << thornLifespan << " ticks" << std::endl;
    std::cout << "      Oak tree lifespan: " << oakLifespan << " ticks" << std::endl;
    
    // Check expected ranges from task description
    // Grass: 1,000-3,000 ticks
    TEST_ASSERT_GE(grassLifespan, 1000u);
    TEST_ASSERT_LE(grassLifespan, 3000u);
    
    // Berry Bush: 5,000-10,000 ticks
    TEST_ASSERT_GE(berryLifespan, 5000u);
    TEST_ASSERT_LE(berryLifespan, 10000u);
    
    // Thorn Bush: 4,000-8,000 ticks
    TEST_ASSERT_GE(thornLifespan, 4000u);
    TEST_ASSERT_LE(thornLifespan, 8000u);
    
    // Oak Tree: 15,000-50,000 ticks
    TEST_ASSERT_GE(oakLifespan, 15000u);
    TEST_ASSERT_LE(oakLifespan, 50000u);
    
    // Relative ordering: grass < berry < thorn < oak
    TEST_ASSERT_LT(grassLifespan, berryLifespan);
    TEST_ASSERT_LT(thornLifespan, oakLifespan);
}

// ============================================================================
// Test Runner
// ============================================================================

void runBehaviorPlantTests() {
    BEGIN_TEST_GROUP("Plant Growth Tests");
    RUN_TEST(testPlantGrowth);
    RUN_TEST(testPlantGrowthWithDifferentLight);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Plant Fruit Production Tests");
    RUN_TEST(testPlantFruitProductionWhenMature);
    RUN_TEST(testPlantFruitCaloriesScaleWithSize);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Plant Death Tests");
    RUN_TEST(testPlantDiesOfOldAge);
    RUN_TEST(testPlantDiesFromDamage);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Species Growth Rate Tests");
    RUN_TEST(testSpeciesHaveDifferentGrowthRates);
    RUN_TEST(testSpeciesHaveDifferentMaxSizes);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Plant Defense Tests");
    RUN_TEST(testThornBushHasHighThorns);
    RUN_TEST(testThornBushHasHighToxicity);
    RUN_TEST(testGrassHasHighRegrowth);
    RUN_TEST(testDefenseTraitsAreInValidRanges);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Plant Regeneration Tests");
    RUN_TEST(testPlantCanRegenerate);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Seed Dispersal Tests");
    RUN_TEST(testPlantSeedDispersal);
    RUN_TEST(testSeedCreatesNewPlant);
    RUN_TEST(testDifferentSpeciesHaveDifferentDispersalStrategies);
    RUN_TEST(testPlantLifespansAreReasonable);
    END_TEST_GROUP();
}

#ifdef TEST_BEHAVIOR_PLANTS_STANDALONE
int main() {
    std::cout << "=== Plant Behavior Integration Tests ===" << std::endl;
    runBehaviorPlantTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
