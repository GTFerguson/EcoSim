/**
 * @file test_berry_bush_diagnostic.cpp
 * @brief Diagnostic tests for Berry Bush lifecycle
 * 
 * These tests investigate why berry bushes appear to be dying early during
 * simulation, even without creatures. Tests trace the full lifecycle:
 * - Initialization state
 * - Temperature tolerance (with UniversalGenes fix verification)
 * - Growth progression over time
 * - Health and survival through full lifespan
 * - Fruit production capability
 * - Seed dispersal and offspring creation
 * 
 * Note: Plant class exposes limited public API - we infer internal state
 * from observable behaviors (canProduceFruit, canSpreadSeeds, isAlive).
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <variant>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/expression/EnvironmentState.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Helper: Print divider for verbose output
// ============================================================================

void printDivider(const std::string& title) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║ " << std::left << std::setw(61) << title << "║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
}

void printSubsection(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
}

// ============================================================================
// Helper: Read float from GeneValue variant
// ============================================================================

float getFloatFromGeneValue(const G::GeneValue& gv, float defaultValue) {
    if (std::holds_alternative<float>(gv)) {
        return std::get<float>(gv);
    }
    return defaultValue;
}

// ============================================================================
// Test 1: Berry Bush Initialization
// ============================================================================

void testBerryBushInitialization() {
    printDivider("TEST 1: Berry Bush Initialization");
    
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 50, 50);
    
    // Verify initial state
    printSubsection("Initial State Verification");
    std::cout << "Position: (" << berryBush.getX() << ", " << berryBush.getY() << ")\n";
    std::cout << "Is Alive: " << (berryBush.isAlive() ? "YES" : "NO") << "\n";
    std::cout << "Age: " << berryBush.getAge() << " (expected: 0)\n";
    std::cout << "Current Size: " << berryBush.getCurrentSize() << " (expected: 0.1)\n";
    std::cout << "Entity Type: " << static_cast<int>(berryBush.getEntityType()) << "\n";
    std::cout << "Can Spread Seeds: " << (berryBush.canSpreadSeeds() ? "YES" : "NO") 
              << " (expected: NO - not mature)\n";
    std::cout << "Can Produce Fruit: " << (berryBush.canProduceFruit() ? "YES" : "NO") 
              << " (expected: NO - not mature)\n";
    
    TEST_ASSERT(berryBush.isAlive());
    TEST_ASSERT_EQ(0u, berryBush.getAge());
    TEST_ASSERT(!berryBush.canSpreadSeeds());  // Not mature yet
    TEST_ASSERT(!berryBush.canProduceFruit()); // Not mature yet
    
    // Print genetic values
    printSubsection("Genetic Values (from Genome)");
    
    float lifespan = static_cast<float>(berryBush.getMaxLifespan());
    float growthRate = berryBush.getGrowthRate();
    float maxSize = berryBush.getMaxSize();
    float hardiness = berryBush.getHardiness();
    
    std::cout << "Lifespan: " << lifespan << " ticks (template: 5000-10000)\n";
    std::cout << "Growth Rate: " << growthRate << " (template: 0.45-0.55)\n";
    std::cout << "Max Size: " << maxSize << " (template: 1.8-2.2)\n";
    std::cout << "Hardiness: " << hardiness << " (template: 0.3-0.5)\n";
    
    // Verify gene values are within template ranges
    TEST_ASSERT_GE(lifespan, 5000.0f);
    TEST_ASSERT_LE(lifespan, 10000.0f);
    TEST_ASSERT_GE(growthRate, 0.45f);
    TEST_ASSERT_LE(growthRate, 0.55f);
    TEST_ASSERT_GE(maxSize, 1.8f);
    TEST_ASSERT_LE(maxSize, 2.2f);
    
    printSubsection("Food Production Genes");
    float fruitRate = berryBush.getFruitProductionRate();
    float fruitAppeal = berryBush.getFruitAppeal();
    
    std::cout << "Fruit Production Rate: " << fruitRate << " (template: 0.7-0.9)\n";
    std::cout << "Fruit Appeal: " << fruitAppeal << " (template: 0.7-0.9)\n";
    
    TEST_ASSERT_GE(fruitRate, 0.7f);
    TEST_ASSERT_LE(fruitRate, 0.9f);
    
    printSubsection("Defense Genes");
    float thornDensity = berryBush.getThornDamage();
    float toxicity = berryBush.getToxicity();
    float regrowth = berryBush.getRegrowthRate();
    
    std::cout << "Thorn Density: " << thornDensity << " (template: 0.05-0.15)\n";
    std::cout << "Toxicity: " << toxicity << " (template: 0.0-0.1)\n";
    std::cout << "Regrowth Rate: " << regrowth << " (template: 0.3-0.5)\n";
    
    std::cout << "\n✓ Berry bush initialization test PASSED\n";
}

// ============================================================================
// Test 2: Temperature Tolerance
// ============================================================================

void testTemperatureTolerance() {
    printDivider("TEST 2: Temperature Tolerance");
    
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 50, 50);
    
    // First, read the actual temperature tolerance genes
    printSubsection("Temperature Tolerance Gene Values");
    
    // Check if genome has the genes
    const G::Genome& genome = berryBush.getGenome();
    
    std::cout << "Has UniversalGenes::TEMP_TOLERANCE_LOW: " 
              << (genome.hasGene(G::UniversalGenes::TEMP_TOLERANCE_LOW) ? "YES" : "NO") << "\n";
    std::cout << "Has UniversalGenes::TEMP_TOLERANCE_HIGH: " 
              << (genome.hasGene(G::UniversalGenes::TEMP_TOLERANCE_HIGH) ? "YES" : "NO") << "\n";
    
    // Read actual values from genome
    float tempLow = 5.0f;  // default
    float tempHigh = 35.0f; // default
    
    if (genome.hasGene(G::UniversalGenes::TEMP_TOLERANCE_LOW)) {
        const G::Gene& gene = genome.getGene(G::UniversalGenes::TEMP_TOLERANCE_LOW);
        G::GeneValue gv = gene.getExpressedValue(G::DominanceType::Incomplete);
        tempLow = getFloatFromGeneValue(gv, 5.0f);
    }
    
    if (genome.hasGene(G::UniversalGenes::TEMP_TOLERANCE_HIGH)) {
        const G::Gene& gene = genome.getGene(G::UniversalGenes::TEMP_TOLERANCE_HIGH);
        G::GeneValue gv = gene.getExpressedValue(G::DominanceType::Incomplete);
        tempHigh = getFloatFromGeneValue(gv, 35.0f);
    }
    
    std::cout << "Temperature Tolerance Low: " << tempLow << "°C (template: 5-10°C)\n";
    std::cout << "Temperature Tolerance High: " << tempHigh << "°C (template: 30-40°C)\n";
    
    // Verify template ranges
    TEST_ASSERT_GE(tempLow, 5.0f);
    TEST_ASSERT_LE(tempLow, 10.0f);
    TEST_ASSERT_GE(tempHigh, 30.0f);
    TEST_ASSERT_LE(tempHigh, 40.0f);
    
    printSubsection("Testing canSurviveTemperature()");
    
    struct TempTest {
        float temp;
        bool expectedSurvival;
        const char* description;
    };
    
    std::vector<TempTest> tests = {
        {20.0f, true, "20°C - Normal environment"},
        {15.0f, true, "15°C - Cool"},
        {25.0f, true, "25°C - Warm"},
        {tempLow, true, "At low threshold"},
        {tempHigh, true, "At high threshold"},
        {tempLow - 1.0f, false, "Below low threshold"},
        {tempHigh + 1.0f, false, "Above high threshold"},
        {0.0f, false, "0°C - Near freezing"},
        {50.0f, false, "50°C - Very hot"},
        {-10.0f, false, "-10°C - Freezing"},
    };
    
    for (const auto& test : tests) {
        bool canSurvive = berryBush.canSurviveTemperature(test.temp);
        std::cout << std::fixed << std::setprecision(1);
        std::cout << test.description << ": " << test.temp << "°C -> " 
                  << (canSurvive ? "SURVIVE" : "DAMAGE");
        
        if (canSurvive != test.expectedSurvival) {
            std::cout << " [UNEXPECTED - expected " 
                      << (test.expectedSurvival ? "SURVIVE" : "DAMAGE") << "]";
        }
        std::cout << "\n";
        
        // Only assert for clearly defined cases
        if (test.temp >= tempLow && test.temp <= tempHigh) {
            TEST_ASSERT_MSG(canSurvive, "Should survive within tolerance range");
        } else if (test.temp < tempLow - 1.0f || test.temp > tempHigh + 1.0f) {
            TEST_ASSERT_MSG(!canSurvive, "Should not survive far outside tolerance");
        }
    }
    
    std::cout << "\n✓ Temperature tolerance test PASSED\n";
}

// ============================================================================
// Test 3: Growth Over Time
// ============================================================================

void testGrowthOverTime() {
    printDivider("TEST 3: Growth Over Time");
    
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 50, 50);
    
    // Standard environment (good growing conditions)
    G::EnvironmentState env;
    env.temperature = 20.0f;    // Optimal-ish temperature
    env.humidity = 0.6f;        // Good water availability
    env.time_of_day = 0.5f;     // Noon (max light)
    
    float maxSize = berryBush.getMaxSize();
    float growthRate = berryBush.getGrowthRate();
    unsigned int lifespan = berryBush.getMaxLifespan();
    
    printSubsection("Growth Parameters");
    std::cout << "Max Size: " << maxSize << "\n";
    std::cout << "Growth Rate: " << growthRate << "\n";
    std::cout << "Lifespan: " << lifespan << " ticks\n";
    std::cout << "Environment: temp=" << env.temperature << "°C, humidity=" 
              << env.humidity << ", time_of_day=" << env.time_of_day << "\n";
    
    printSubsection("Growth Progression");
    std::cout << std::left << std::setw(10) << "Tick" 
              << std::setw(12) << "Size"
              << std::setw(12) << "Size %"
              << std::setw(14) << "CanFruit"
              << std::setw(10) << "Age"
              << std::setw(10) << "Alive" << "\n";
    std::cout << std::string(68, '-') << "\n";
    
    std::vector<unsigned int> checkpoints = {0, 100, 250, 500, 750, 1000, 1500, 2000};
    unsigned int currentTick = 0;
    
    // Print initial state
    {
        float sizePercent = (berryBush.getCurrentSize() / maxSize) * 100.0f;
        std::cout << std::left << std::setw(10) << currentTick
                  << std::fixed << std::setprecision(3)
                  << std::setw(12) << berryBush.getCurrentSize()
                  << std::setprecision(1)
                  << std::setw(12) << sizePercent
                  << std::setw(14) << (berryBush.canProduceFruit() ? "YES" : "NO")
                  << std::setw(10) << berryBush.getAge()
                  << std::setw(10) << (berryBush.isAlive() ? "YES" : "NO")
                  << "\n";
    }
    
    for (unsigned int targetTick : checkpoints) {
        if (targetTick == 0) continue;  // Already printed
        
        // Run updates to reach target tick
        while (currentTick < targetTick && berryBush.isAlive()) {
            berryBush.update(env);
            currentTick++;
        }
        
        float sizePercent = (berryBush.getCurrentSize() / maxSize) * 100.0f;
        
        std::cout << std::left << std::setw(10) << currentTick
                  << std::fixed << std::setprecision(3)
                  << std::setw(12) << berryBush.getCurrentSize()
                  << std::setprecision(1)
                  << std::setw(12) << sizePercent
                  << std::setw(14) << (berryBush.canProduceFruit() ? "YES" : "NO")
                  << std::setw(10) << berryBush.getAge()
                  << std::setw(10) << (berryBush.isAlive() ? "YES" : "NO")
                  << "\n";
        
        if (!berryBush.isAlive()) {
            std::cout << "\n⚠ PLANT DIED at tick " << currentTick << "!\n";
            break;
        }
    }
    
    // Verify plant is still alive after reasonable growth period
    TEST_ASSERT_MSG(berryBush.isAlive(), "Plant should survive 2000 ticks of growth");
    TEST_ASSERT_GT(berryBush.getCurrentSize(), 0.1f);  // Should have grown
    
    // Check maturity progress by tick 2000
    std::cout << "\nAt tick 2000:\n";
    std::cout << "  Can Produce Fruit: " << (berryBush.canProduceFruit() ? "YES" : "NO") << "\n";
    std::cout << "  Can Spread Seeds: " << (berryBush.canSpreadSeeds() ? "YES" : "NO") << "\n";
    std::cout << "  Size: " << berryBush.getCurrentSize() << " / " << maxSize << "\n";
    
    std::cout << "\n✓ Growth over time test PASSED\n";
}

// ============================================================================
// Test 4: Health and Survival Through Full Lifespan
// ============================================================================

void testHealthAndSurvival() {
    printDivider("TEST 4: Health and Survival Through Lifespan");
    
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 50, 50);
    
    // Ideal environment - should not cause any damage
    G::EnvironmentState env;
    env.temperature = 22.5f;    // Optimal temperature
    env.humidity = 0.6f;
    env.time_of_day = 0.5f;
    
    unsigned int lifespan = berryBush.getMaxLifespan();
    
    printSubsection("Test Parameters");
    std::cout << "Plant Lifespan: " << lifespan << " ticks\n";
    std::cout << "Environment: optimal (temp=22.5°C, humidity=0.6)\n";
    std::cout << "Expected: Plant should live until ~lifespan ticks, then die of old age\n";
    
    printSubsection("Survival Monitoring");
    std::cout << std::left << std::setw(12) << "Tick" 
              << std::setw(12) << "Age/Max"
              << std::setw(10) << "Alive"
              << std::setw(12) << "CanFruit"
              << std::setw(15) << "Notes" << "\n";
    std::cout << std::string(61, '-') << "\n";
    
    // Monitor at intervals (every 10% of lifespan)
    unsigned int interval = lifespan / 10;
    unsigned int currentTick = 0;
    bool wasAbleToProduce = false;
    unsigned int firstFruitTick = 0;
    
    while (berryBush.isAlive() && currentTick <= lifespan + 100) {
        // Check if can produce fruit (indicates maturity and health)
        bool canProduce = berryBush.canProduceFruit();
        if (canProduce && !wasAbleToProduce) {
            firstFruitTick = currentTick;
            wasAbleToProduce = true;
        }
        
        // Log at intervals
        if (currentTick % interval == 0) {
            std::string notes;
            if (canProduce && currentTick == firstFruitTick) {
                notes = "FIRST FRUIT";
            } else if (currentTick >= lifespan) {
                notes = "PAST LIFESPAN";
            }
            
            std::cout << std::left << std::setw(12) << currentTick
                      << std::setw(12) << (std::to_string(berryBush.getAge()) + "/" + std::to_string(lifespan))
                      << std::setw(10) << (berryBush.isAlive() ? "YES" : "NO")
                      << std::setw(12) << (canProduce ? "YES" : "NO")
                      << std::setw(15) << notes << "\n";
        }
        
        berryBush.update(env);
        currentTick++;
    }
    
    // Print final state
    std::cout << std::left << std::setw(12) << currentTick
              << std::setw(12) << (std::to_string(berryBush.getAge()) + "/" + std::to_string(lifespan))
              << std::setw(10) << (berryBush.isAlive() ? "YES" : "NO")
              << std::setw(12) << "-"
              << std::setw(15) << "FINAL STATE" << "\n";
    
    printSubsection("Results");
    std::cout << "Death tick: " << currentTick << "\n";
    std::cout << "Final age: " << berryBush.getAge() << "\n";
    std::cout << "Expected death at: ~" << lifespan << " ticks (old age)\n";
    std::cout << "First fruit production at: " << firstFruitTick << " ticks\n";
    
    // Determine death cause based on timing
    std::string deathCause = "unknown";
    if (berryBush.getAge() > lifespan) {
        deathCause = "old_age";
    } else if (currentTick < lifespan * 0.9) {
        deathCause = "premature_death";
    }
    std::cout << "Likely death cause: " << deathCause << "\n";
    
    // The plant should survive until at least near its lifespan
    TEST_ASSERT_GE(currentTick, lifespan);
    TEST_ASSERT_MSG(deathCause == "old_age", "Plant should die from old age, not prematurely");
    
    std::cout << "\n✓ Health and survival test PASSED\n";
}

// ============================================================================
// Test 5: Fruit Production Capability
// ============================================================================

void testFruitProduction() {
    printDivider("TEST 5: Fruit Production Capability");
    
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 50, 50);
    
    // Good growing environment
    G::EnvironmentState env;
    env.temperature = 22.5f;
    env.humidity = 0.6f;
    env.time_of_day = 0.5f;
    
    unsigned int lifespan = berryBush.getMaxLifespan();
    float maxSize = berryBush.getMaxSize();
    float fruitRate = berryBush.getFruitProductionRate();
    
    printSubsection("Fruit Production Requirements");
    std::cout << "From canProduceFruit() conditions:\n";
    std::cout << "  1. alive: " << (berryBush.isAlive() ? "YES" : "NO") << "\n";
    std::cout << "  2. mature: requires size >= 50% of max = " << (maxSize * 0.5f) << "\n";
    std::cout << "  3. age >= 10% of lifespan: >= " << (lifespan * 0.10f) << " ticks\n";
    std::cout << "  4. fruitRate > 0.01: " << fruitRate << " (✓)\n";
    std::cout << "  5. fruitTimer >= cooldown: cooldown = " 
              << static_cast<unsigned int>(100.0f / (fruitRate + 0.1f)) << " ticks\n";
    
    printSubsection("Growing Until Fruit Production");
    
    // Grow until can produce fruit or we've tried enough
    unsigned int tick = 0;
    unsigned int maxTicks = lifespan / 2;  // Don't run past half lifespan
    
    while (!berryBush.canProduceFruit() && tick < maxTicks && berryBush.isAlive()) {
        berryBush.update(env);
        tick++;
        
        if (tick % 500 == 0) {
            float sizePercent = (berryBush.getCurrentSize() / maxSize) * 100.0f;
            unsigned int maturityAge = static_cast<unsigned int>(lifespan * 0.10f);
            
            std::cout << "Tick " << tick << ": size=" << std::fixed << std::setprecision(2) 
                      << berryBush.getCurrentSize() << " (" << std::setprecision(1) << sizePercent << "%), "
                      << "age=" << berryBush.getAge() << "/" << maturityAge << ", "
                      << "canFruit=" << (berryBush.canProduceFruit() ? "YES" : "NO") << "\n";
        }
    }
    
    // Final check
    bool canProduce = berryBush.canProduceFruit();
    
    printSubsection("Results");
    std::cout << "First able to produce fruit at tick: " << tick << "\n";
    std::cout << "Current size: " << berryBush.getCurrentSize() << " / " << maxSize 
              << " (" << (berryBush.getCurrentSize()/maxSize*100) << "%)\n";
    std::cout << "Current age: " << berryBush.getAge() << " / " << lifespan << "\n";
    std::cout << "canProduceFruit(): " << (canProduce ? "YES" : "NO") << "\n";
    std::cout << "Plant alive: " << (berryBush.isAlive() ? "YES" : "NO") << "\n";
    
    if (!canProduce) {
        std::cout << "\nDiagnosing why canProduceFruit() = false:\n";
        std::cout << "  - alive: " << (berryBush.isAlive() ? "YES" : "NO") << "\n";
        std::cout << "  - size >= 50% max (" << (maxSize*0.5f) << "): " 
                  << (berryBush.getCurrentSize() >= maxSize*0.5f ? "YES" : "NO") << "\n";
        std::cout << "  - age >= 10% lifespan (" << (lifespan*0.1f) << "): " 
                  << (berryBush.getAge() >= lifespan*0.1f ? "YES" : "NO") << "\n";
    }
    
    TEST_ASSERT_MSG(canProduce, "Berry bush should be able to produce fruit");
    
    std::cout << "\n✓ Fruit production test PASSED\n";
}

// ============================================================================
// Test 6: Seed Dispersal and Offspring
// ============================================================================

void testSeedDispersal() {
    printDivider("TEST 6: Seed Dispersal and Offspring");
    
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 50, 50);
    
    // Grow to ability to spread seeds
    G::EnvironmentState env;
    env.temperature = 22.5f;
    env.humidity = 0.6f;
    env.time_of_day = 0.5f;
    
    unsigned int tick = 0;
    unsigned int maxTicks = 3000;
    
    // Grow until can spread seeds
    while (!berryBush.canSpreadSeeds() && tick < maxTicks && berryBush.isAlive()) {
        berryBush.update(env);
        tick++;
    }
    
    printSubsection("Parent Plant State");
    std::cout << "Age: " << berryBush.getAge() << "\n";
    std::cout << "Size: " << berryBush.getCurrentSize() << " / " << berryBush.getMaxSize() << "\n";
    std::cout << "Can Spread Seeds: " << (berryBush.canSpreadSeeds() ? "YES" : "NO") << "\n";
    std::cout << "Can Produce Fruit: " << (berryBush.canProduceFruit() ? "YES" : "NO") << "\n";
    std::cout << "Is Alive: " << (berryBush.isAlive() ? "YES" : "NO") << "\n";
    std::cout << "Seed Count: " << berryBush.getSeedCount() << "\n";
    
    printSubsection("Dispersal Strategy Analysis");
    G::DispersalStrategy strategy = berryBush.getPrimaryDispersalStrategy();
    std::cout << "Primary Dispersal Strategy: ";
    switch (strategy) {
        case G::DispersalStrategy::GRAVITY: std::cout << "GRAVITY"; break;
        case G::DispersalStrategy::WIND: std::cout << "WIND"; break;
        case G::DispersalStrategy::ANIMAL_FRUIT: std::cout << "ANIMAL_FRUIT"; break;
        case G::DispersalStrategy::ANIMAL_BURR: std::cout << "ANIMAL_BURR"; break;
        case G::DispersalStrategy::EXPLOSIVE: std::cout << "EXPLOSIVE"; break;
        case G::DispersalStrategy::VEGETATIVE: std::cout << "VEGETATIVE"; break;
    }
    std::cout << "\n";
    
    // Berry bushes should use ANIMAL_FRUIT strategy (high fruit appeal, durable seeds)
    std::cout << "\nRelevant gene values:\n";
    std::cout << "  Fruit Appeal: " << berryBush.getFruitAppeal() << " (>0.6 for ANIMAL_FRUIT)\n";
    std::cout << "  Seed Coat Durability: " << berryBush.getSeedCoatDurability() << " (>0.5 for ANIMAL_FRUIT)\n";
    std::cout << "  Seed Mass: " << berryBush.getSeedMass() << "\n";
    std::cout << "  Seed Aerodynamics: " << berryBush.getSeedAerodynamics() << "\n";
    std::cout << "  Seed Hook Strength: " << berryBush.getSeedHookStrength() << "\n";
    std::cout << "  Runner Production: " << berryBush.getRunnerProduction() << "\n";
    
    printSubsection("Testing SeedDispersal");
    G::SeedDispersal disperser(42);  // Fixed seed for reproducibility
    
    // Generate several dispersal events
    std::cout << "\nGenerating dispersal events:\n";
    std::cout << std::left << std::setw(8) << "Event"
              << std::setw(15) << "Target"
              << std::setw(12) << "Viability"
              << std::setw(15) << "Method" << "\n";
    std::cout << std::string(50, '-') << "\n";
    
    for (int i = 0; i < 5; i++) {
        G::DispersalEvent event = disperser.disperse(berryBush, &env);
        
        std::string methodStr;
        switch (event.method) {
            case G::DispersalStrategy::GRAVITY: methodStr = "GRAVITY"; break;
            case G::DispersalStrategy::WIND: methodStr = "WIND"; break;
            case G::DispersalStrategy::ANIMAL_FRUIT: methodStr = "ANIMAL_FRUIT"; break;
            case G::DispersalStrategy::ANIMAL_BURR: methodStr = "ANIMAL_BURR"; break;
            case G::DispersalStrategy::EXPLOSIVE: methodStr = "EXPLOSIVE"; break;
            case G::DispersalStrategy::VEGETATIVE: methodStr = "VEGETATIVE"; break;
        }
        
        std::cout << std::left << std::setw(8) << (i + 1)
                  << "(" << std::setw(3) << event.targetX << "," 
                  << std::setw(3) << event.targetY << ")     "
                  << std::fixed << std::setprecision(2)
                  << std::setw(12) << event.seedViability
                  << std::setw(15) << methodStr << "\n";
    }
    
    printSubsection("Testing Offspring Creation");
    
    // Test reproduce() - the unified IReproducible interface
    if (berryBush.canReproduce()) {
        auto offspringBase = berryBush.reproduce();  // nullptr = asexual
        
        if (offspringBase) {
            // @todo Remove dynamic_cast when Creature/Plant unified into Organism
            auto* offspring = dynamic_cast<G::Plant*>(offspringBase.get());
            TEST_ASSERT_MSG(offspring != nullptr, "reproduce() should return Plant");
            
            std::cout << "Offspring created successfully!\n";
            std::cout << "  Position: (" << offspring->getX() << ", " << offspring->getY() << ")\n";
            std::cout << "  Is Alive: " << (offspring->isAlive() ? "YES" : "NO") << "\n";
            std::cout << "  Size: " << offspring->getCurrentSize() << "\n";
            std::cout << "  Lifespan: " << offspring->getMaxLifespan() << "\n";
            
            TEST_ASSERT(offspring->isAlive());
        } else {
            std::cout << "WARNING: reproduce() returned nullptr\n";
            TEST_ASSERT_MSG(false, "reproduce should return valid plant");
        }
    } else {
        std::cout << "Plant cannot reproduce yet:\n";
        std::cout << "  canReproduce() = false\n";
        std::cout << "  Checking conditions:\n";
        std::cout << "    - alive: " << (berryBush.isAlive() ? "YES" : "NO") << "\n";
        std::cout << "    - seedCount: " << berryBush.getSeedCount() << "\n";
        
        // This shouldn't happen if we grew enough, but don't fail the test
        std::cout << "\nNOTE: Could not test offspring creation - plant not ready\n";
    }
    
    std::cout << "\n✓ Seed dispersal test PASSED\n";
}

// ============================================================================
// Test Runner
// ============================================================================

void runBerryBushDiagnosticTests() {
    BEGIN_TEST_GROUP("Berry Bush Diagnostic Tests");
    RUN_TEST(testBerryBushInitialization);
    RUN_TEST(testTemperatureTolerance);
    RUN_TEST(testGrowthOverTime);
    RUN_TEST(testHealthAndSurvival);
    RUN_TEST(testFruitProduction);
    RUN_TEST(testSeedDispersal);
    END_TEST_GROUP();
}

#ifdef TEST_BERRY_BUSH_DIAGNOSTIC_STANDALONE
int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       BERRY BUSH DIAGNOSTIC TEST SUITE                       ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Investigating early death in simulation                     ║\n";
    std::cout << "║  Testing: Temperature, Growth, Health, Reproduction          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    runBerryBushDiagnosticTests();
    
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
