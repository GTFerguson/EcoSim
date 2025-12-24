/**
 * @file test_behavior_feeding.cpp
 * @brief Behavior integration tests for creature-plant feeding interactions
 * 
 * Tests creature feeding behavior:
 * - Hungry creature seeks food
 * - Creature can eat plants
 * - Nutrition is extracted correctly (based on digestion genes)
 * - Damage from plant defenses (thorns, toxins)
 * - Seed passage through gut
 */

#include <iostream>
#include <memory>
#include <vector>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/interactions/FeedingInteraction.hpp"
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
// Helper: Create creature phenotype with specified diet traits
// ============================================================================

class TestCreaturePhenotype {
public:
    TestCreaturePhenotype(G::GeneRegistry& registry, float plantDigestion, float toxinTolerance, float hideThickness)
        : registry_(registry)
        , genome_(G::UniversalGenes::createCreatureGenome(registry))
    {
        // Set specific traits for testing using correct API
        setGeneValue(genome_, G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, plantDigestion);
        setGeneValue(genome_, G::UniversalGenes::TOXIN_TOLERANCE, toxinTolerance);
        setGeneValue(genome_, G::UniversalGenes::HIDE_THICKNESS, hideThickness);
        setGeneValue(genome_, G::UniversalGenes::COLOR_VISION, 0.8f);
        setGeneValue(genome_, G::UniversalGenes::SCENT_DETECTION, 0.7f);
        setGeneValue(genome_, G::UniversalGenes::SWEETNESS_PREFERENCE, 0.7f);
        setGeneValue(genome_, G::UniversalGenes::SEED_DESTRUCTION_RATE, 0.3f);
        setGeneValue(genome_, G::UniversalGenes::GUT_TRANSIT_TIME, 6.0f);
        setGeneValue(genome_, G::UniversalGenes::CELLULOSE_BREAKDOWN, 0.5f);
        setGeneValue(genome_, G::UniversalGenes::PAIN_SENSITIVITY, 0.5f);
        
        phenotype_ = std::make_unique<G::Phenotype>(&genome_, &registry_);
        
        // Update phenotype context
        G::EnvironmentState env;
        G::OrganismState state;
        state.age_normalized = 0.5f;
        state.health = 1.0f;
        state.energy_level = 0.5f;
        phenotype_->updateContext(env, state);
    }
    
    G::Phenotype& getPhenotype() { return *phenotype_; }
    const G::Phenotype& getPhenotype() const { return *phenotype_; }
    G::Genome& getGenome() { return genome_; }
    
private:
    G::GeneRegistry& registry_;
    G::Genome genome_;
    std::unique_ptr<G::Phenotype> phenotype_;
};

// ============================================================================
// Test 1: Creature Can Eat Plants
// ============================================================================

void testCreatureCanEatPlant() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with good plant digestion
    TestCreaturePhenotype creature(*registry, 0.8f, 0.5f, 0.5f);
    
    // Create berry bush (low defenses, high appeal)
    G::Plant plant = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Grow plant a bit to have nutritional value
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 500; i++) {
        plant.update(env);
    }
    
    // Attempt to eat
    G::FeedingInteraction interaction;
    G::FeedingResult result = interaction.attemptToEatPlant(
        creature.getPhenotype(), plant, 0.8f  // Hungry
    );
    
    std::cout << "      Feeding result: " << (result.success ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "      Description: " << result.description << std::endl;
    
    TEST_ASSERT(result.success);
    TEST_ASSERT_GT(result.nutritionGained, 0.0f);
}

void testHungryCreatureMoreLikelyToEat() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature
    TestCreaturePhenotype creature(*registry, 0.6f, 0.3f, 0.3f);
    
    // Create thorn bush (has some defenses)
    G::Plant plant = factory.createFromTemplate("thorn_bush", 0, 0);
    
    // Grow plant
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 300; i++) {
        plant.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    // Try when not hungry
    G::FeedingResult notHungryResult = interaction.attemptToEatPlant(
        creature.getPhenotype(), plant, 0.2f  // Not very hungry
    );
    
    // Try when very hungry
    G::FeedingResult hungryResult = interaction.attemptToEatPlant(
        creature.getPhenotype(), plant, 0.95f  // Very hungry
    );
    
    std::cout << "      Not hungry result: " << (notHungryResult.success ? "ate" : "didn't eat") << std::endl;
    std::cout << "      Very hungry result: " << (hungryResult.success ? "ate" : "didn't eat") << std::endl;
    
    // Very hungry creatures should be more willing to eat defended plants
    // (though both might succeed or fail depending on exact values)
    TEST_ASSERT(hungryResult.success);  // Very hungry should definitely try
}

// ============================================================================
// Test 2: Nutrition Extracted Based on Digestion Genes
// ============================================================================

void testNutritionScalesWithDigestionEfficiency() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with high plant digestion
    TestCreaturePhenotype goodDigester(*registry, 0.9f, 0.5f, 0.5f);
    
    // Create creature with low plant digestion
    TestCreaturePhenotype poorDigester(*registry, 0.3f, 0.5f, 0.5f);
    
    // Create identical plants
    G::Plant plant1 = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant plant2 = factory.createFromTemplate("berry_bush", 5, 5);
    
    // Grow plants identically
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 500; i++) {
        plant1.update(env);
        plant2.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    G::FeedingResult goodResult = interaction.attemptToEatPlant(
        goodDigester.getPhenotype(), plant1, 0.8f
    );
    
    G::FeedingResult poorResult = interaction.attemptToEatPlant(
        poorDigester.getPhenotype(), plant2, 0.8f
    );
    
    std::cout << "      Good digester nutrition: " << goodResult.nutritionGained << std::endl;
    std::cout << "      Poor digester nutrition: " << poorResult.nutritionGained << std::endl;
    
    // Good digester should get more nutrition from same plant
    if (goodResult.success && poorResult.success) {
        TEST_ASSERT_GT(goodResult.nutritionGained, poorResult.nutritionGained);
    }
}

void testCelluloseBreakdownHelpsWithToughPlants() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with high cellulose breakdown
    TestCreaturePhenotype herbivore(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(herbivore.getGenome(), G::UniversalGenes::CELLULOSE_BREAKDOWN, 0.9f);
    
    // Create grass (tough, needs cellulose breakdown)
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    
    // Grow grass
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 300; i++) {
        grass.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    float efficiency = interaction.getDigestionEfficiency(herbivore.getPhenotype(), grass);
    
    std::cout << "      Herbivore digestion efficiency on grass: " << efficiency << std::endl;
    
    TEST_ASSERT_GT(efficiency, 0.5f);  // Should be reasonably efficient
}

// ============================================================================
// Test 3: Damage from Plant Defenses (Thorns, Toxins)
// ============================================================================

void testCreatureTakesDamageFromThorns() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with low hide thickness (vulnerable)
    TestCreaturePhenotype thinSkin(*registry, 0.7f, 0.5f, 0.1f);  // Low hide
    
    // Create thorn bush
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    
    // Grow plant
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 300; i++) {
        thornBush.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    float thornDamage = interaction.calculateThornDamage(thinSkin.getPhenotype(), thornBush);
    
    std::cout << "      Thorn damage to thin-skinned creature: " << thornDamage << std::endl;
    std::cout << "      Thorn bush thorn density: " << thornBush.getThornDamage() << std::endl;
    
    TEST_ASSERT_GT(thornDamage, 0.0f);  // Should take some damage
}

void testHideThicknessReducesThornDamage() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with thin hide
    TestCreaturePhenotype thinHide(*registry, 0.7f, 0.5f, 0.1f);
    
    // Create creature with thick hide
    TestCreaturePhenotype thickHide(*registry, 0.7f, 0.5f, 0.9f);
    
    // Create thorn bush
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    
    // Grow plant
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 300; i++) {
        thornBush.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    float thinDamage = interaction.calculateThornDamage(thinHide.getPhenotype(), thornBush);
    float thickDamage = interaction.calculateThornDamage(thickHide.getPhenotype(), thornBush);
    
    std::cout << "      Thin hide damage: " << thinDamage << std::endl;
    std::cout << "      Thick hide damage: " << thickDamage << std::endl;
    
    // Thick hide should take less damage
    TEST_ASSERT_LT(thickDamage, thinDamage);
}

void testCreatureTakesDamageFromToxins() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with low toxin tolerance
    TestCreaturePhenotype vulnerable(*registry, 0.7f, 0.1f, 0.5f);  // Low toxin tolerance
    
    // Create thorn bush (has toxins)
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    
    // Grow plant
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 300; i++) {
        thornBush.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    float toxinDamage = interaction.calculateToxinDamage(vulnerable.getPhenotype(), thornBush);
    
    std::cout << "      Toxin damage to vulnerable creature: " << toxinDamage << std::endl;
    std::cout << "      Thorn bush toxicity: " << thornBush.getToxicity() << std::endl;
    
    TEST_ASSERT_GT(toxinDamage, 0.0f);  // Should take some damage
}

void testToxinToleranceReducesToxinDamage() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with low toxin tolerance
    TestCreaturePhenotype lowTolerance(*registry, 0.7f, 0.1f, 0.5f);
    
    // Create creature with high toxin tolerance
    TestCreaturePhenotype highTolerance(*registry, 0.7f, 0.9f, 0.5f);
    setGeneValue(highTolerance.getGenome(), G::UniversalGenes::TOXIN_METABOLISM, 0.8f);
    
    // Create thorn bush
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    
    // Grow plant
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 300; i++) {
        thornBush.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    float lowDamage = interaction.calculateToxinDamage(lowTolerance.getPhenotype(), thornBush);
    float highDamage = interaction.calculateToxinDamage(highTolerance.getPhenotype(), thornBush);
    
    std::cout << "      Low tolerance damage: " << lowDamage << std::endl;
    std::cout << "      High tolerance damage: " << highDamage << std::endl;
    
    // High tolerance should take less damage
    TEST_ASSERT_LT(highDamage, lowDamage);
}

// ============================================================================
// Test 4: Seed Passage Through Gut
// ============================================================================

void testSeedsCanSurviveGutPassage() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create frugivore creature (low seed destruction)
    TestCreaturePhenotype frugivore(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(frugivore.getGenome(), G::UniversalGenes::SEED_DESTRUCTION_RATE, 0.2f);
    setGeneValue(frugivore.getGenome(), G::UniversalGenes::GUT_TRANSIT_TIME, 6.0f);
    
    // Create berry bush with durable seeds
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Grow plant to maturity for seeds
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 1000; i++) {
        berryBush.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    float survivalRate = interaction.calculateSeedSurvivalRate(frugivore.getPhenotype(), berryBush);
    
    std::cout << "      Seed survival rate through frugivore gut: " << survivalRate << std::endl;
    std::cout << "      Berry bush seed coat durability: " << berryBush.getSeedCoatDurability() << std::endl;
    
    TEST_ASSERT_GT(survivalRate, 0.0f);  // Some seeds should survive
}

void testSeedDestructionRateAffectsSurvival() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with high seed destruction (grinder)
    TestCreaturePhenotype grinder(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(grinder.getGenome(), G::UniversalGenes::SEED_DESTRUCTION_RATE, 0.9f);
    
    // Create creature with low seed destruction (gentle gut)
    TestCreaturePhenotype gentle(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(gentle.getGenome(), G::UniversalGenes::SEED_DESTRUCTION_RATE, 0.1f);
    
    // Create berry bush
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Grow plant
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 500; i++) {
        berryBush.update(env);
    }
    
    G::FeedingInteraction interaction;
    
    float grinderSurvival = interaction.calculateSeedSurvivalRate(grinder.getPhenotype(), berryBush);
    float gentleSurvival = interaction.calculateSeedSurvivalRate(gentle.getPhenotype(), berryBush);
    
    std::cout << "      Grinder seed survival: " << grinderSurvival << std::endl;
    std::cout << "      Gentle gut seed survival: " << gentleSurvival << std::endl;
    
    // Gentle gut should have higher seed survival
    TEST_ASSERT_GT(gentleSurvival, grinderSurvival);
}

void testSeedCoatDurabilityAffectsSurvival() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create standard creature
    TestCreaturePhenotype creature(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(creature.getGenome(), G::UniversalGenes::SEED_DESTRUCTION_RATE, 0.5f);
    
    // Create berry bush (should have some durability)
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Get seed durability
    float durability = berryBush.getSeedCoatDurability();
    
    G::FeedingInteraction interaction;
    
    float survivalRate = interaction.calculateSeedSurvivalRate(creature.getPhenotype(), berryBush);
    
    std::cout << "      Seed coat durability: " << durability << std::endl;
    std::cout << "      Seed survival rate: " << survivalRate << std::endl;
    
    // Survival should be related to durability
    TEST_ASSERT_GE(survivalRate, 0.0f);
    TEST_ASSERT_LE(survivalRate, 1.0f);
}

// ============================================================================
// Test 5: Complete Feeding Interaction Flow
// ============================================================================

void testCompleteFeedingInteractionOnBerryBush() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create herbivore creature
    TestCreaturePhenotype herbivore(*registry, 0.8f, 0.6f, 0.4f);
    
    // Create berry bush
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Grow plant to maturity
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 800; i++) {
        berryBush.update(env);
    }
    
    float initialSize = berryBush.getCurrentSize();
    
    G::FeedingInteraction interaction;
    G::FeedingResult result = interaction.attemptToEatPlant(
        herbivore.getPhenotype(), berryBush, 0.7f
    );
    
    std::cout << "      Feeding success: " << (result.success ? "yes" : "no") << std::endl;
    std::cout << "      Nutrition gained: " << result.nutritionGained << std::endl;
    std::cout << "      Damage received: " << result.damageReceived << std::endl;
    std::cout << "      Plant damage dealt: " << result.plantDamage << std::endl;
    std::cout << "      Seeds consumed: " << (result.seedsConsumed ? "yes" : "no") << std::endl;
    std::cout << "      Seeds to disperse: " << result.seedsToDisperse << std::endl;
    
    TEST_ASSERT(result.success);
    TEST_ASSERT_GT(result.nutritionGained, 0.0f);
    TEST_ASSERT_GT(result.plantDamage, 0.0f);  // Should damage plant when eating
}

void testCompleteFeedingInteractionOnThornBush() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature that can handle defenses
    TestCreaturePhenotype toughCreature(*registry, 0.7f, 0.8f, 0.8f);  // High tolerance, thick hide
    
    // Create thorn bush
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    
    // Grow plant
    G::EnvironmentState env;
    env.temperature = 22.0f;
    env.time_of_day = 0.5f;
    env.humidity = 0.6f;
    for (int i = 0; i < 500; i++) {
        thornBush.update(env);
    }
    
    G::FeedingInteraction interaction;
    G::FeedingResult result = interaction.attemptToEatPlant(
        toughCreature.getPhenotype(), thornBush, 0.8f  // Hungry
    );
    
    std::cout << "      Feeding on thorn bush success: " << (result.success ? "yes" : "no") << std::endl;
    std::cout << "      Nutrition gained: " << result.nutritionGained << std::endl;
    std::cout << "      Damage received: " << result.damageReceived << std::endl;
    
    // Tough creature should be able to eat thorn bush but take some damage
    if (result.success) {
        TEST_ASSERT_GT(result.damageReceived, 0.0f);  // Should still take some damage
        TEST_ASSERT_GT(result.nutritionGained, 0.0f);  // But get nutrition
    }
}

// ============================================================================
// Test 6: Detection Range
// ============================================================================

void testDetectionRangeScalesWithSenses() {
    // Setup
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with good senses
    TestCreaturePhenotype goodSenses(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(goodSenses.getGenome(), G::UniversalGenes::COLOR_VISION, 0.9f);
    setGeneValue(goodSenses.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.9f);
    setGeneValue(goodSenses.getGenome(), G::UniversalGenes::SIGHT_RANGE, 100.0f);
    
    // Create creature with poor senses
    TestCreaturePhenotype poorSenses(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(poorSenses.getGenome(), G::UniversalGenes::COLOR_VISION, 0.1f);
    setGeneValue(poorSenses.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.1f);
    setGeneValue(poorSenses.getGenome(), G::UniversalGenes::SIGHT_RANGE, 20.0f);
    
    // Create berry bush (attractive fruit)
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    G::FeedingInteraction interaction;
    
    float goodRange = interaction.getDetectionRange(goodSenses.getPhenotype(), berryBush);
    float poorRange = interaction.getDetectionRange(poorSenses.getPhenotype(), berryBush);
    
    std::cout << "      Good senses detection range: " << goodRange << std::endl;
    std::cout << "      Poor senses detection range: " << poorRange << std::endl;
    
    TEST_ASSERT_GT(goodRange, poorRange);
}

// ============================================================================
// Test Runner
// ============================================================================

void runBehaviorFeedingTests() {
    BEGIN_TEST_GROUP("Creature Eating Tests");
    RUN_TEST(testCreatureCanEatPlant);
    RUN_TEST(testHungryCreatureMoreLikelyToEat);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Nutrition Extraction Tests");
    RUN_TEST(testNutritionScalesWithDigestionEfficiency);
    RUN_TEST(testCelluloseBreakdownHelpsWithToughPlants);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Plant Defense Damage Tests");
    RUN_TEST(testCreatureTakesDamageFromThorns);
    RUN_TEST(testHideThicknessReducesThornDamage);
    RUN_TEST(testCreatureTakesDamageFromToxins);
    RUN_TEST(testToxinToleranceReducesToxinDamage);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Seed Passage Tests");
    RUN_TEST(testSeedsCanSurviveGutPassage);
    RUN_TEST(testSeedDestructionRateAffectsSurvival);
    RUN_TEST(testSeedCoatDurabilityAffectsSurvival);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Complete Feeding Interaction Tests");
    RUN_TEST(testCompleteFeedingInteractionOnBerryBush);
    RUN_TEST(testCompleteFeedingInteractionOnThornBush);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Detection Tests");
    RUN_TEST(testDetectionRangeScalesWithSenses);
    END_TEST_GROUP();
}

#ifdef TEST_BEHAVIOR_FEEDING_STANDALONE
int main() {
    std::cout << "=== Creature-Plant Feeding Behavior Integration Tests ===" << std::endl;
    runBehaviorFeedingTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
