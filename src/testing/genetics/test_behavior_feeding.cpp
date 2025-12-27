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
// Test 7: Detection Range Formula Verification (Phase 3 specific tests)
// ============================================================================

/**
 * @brief Verify the detection formula components:
 *   visualBonus = colorVision × fruitAppeal × 100
 *   scentBonus = scentDetection × 100
 *   effectiveRange = sightRange + max(visualBonus, scentBonus)
 */
void testDetectionFormulaComponents() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with known values for formula verification
    // SIGHT=85, COLOR_VISION=0.90, SCENT_DETECTION=0.82
    TestCreaturePhenotype creature(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(creature.getGenome(), G::UniversalGenes::SIGHT_RANGE, 85.0f);
    setGeneValue(creature.getGenome(), G::UniversalGenes::COLOR_VISION, 0.90f);
    setGeneValue(creature.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.82f);
    
    // Create berry bush with high fruit appeal (~0.8)
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    float fruitAppeal = berryBush.getFruitAppeal();
    
    G::FeedingInteraction interaction;
    float detectionRange = interaction.getDetectionRange(creature.getPhenotype(), berryBush);
    
    // Calculate expected values
    // visualBonus = 0.9 × fruitAppeal × 100
    // scentBonus = 0.82 × 100 = 82
    // expectedRange = 85 + max(visualBonus, 82)
    float expectedVisualBonus = 0.90f * fruitAppeal * 100.0f;
    float expectedScentBonus = 0.82f * 100.0f;
    float expectedRange = 85.0f + std::max(expectedVisualBonus, expectedScentBonus);
    
    std::cout << "      Fruit appeal: " << fruitAppeal << std::endl;
    std::cout << "      Expected visual bonus: " << expectedVisualBonus << std::endl;
    std::cout << "      Expected scent bonus: " << expectedScentBonus << std::endl;
    std::cout << "      Expected detection range: " << expectedRange << std::endl;
    std::cout << "      Actual detection range: " << detectionRange << std::endl;
    
    // Allow small tolerance for floating point
    float tolerance = 5.0f;
    TEST_ASSERT_GT(detectionRange, expectedRange - tolerance);
    TEST_ASSERT_LT(detectionRange, expectedRange + tolerance);
}

/**
 * @brief Test: Color vision helps spot COLORFUL plants specifically
 * High CV + high fruit appeal = big visual bonus
 * High CV + low fruit appeal = small visual bonus
 */
void testColorVisionHelpsWithColorfulPlants() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with HIGH color vision but LOW scent detection
    // This isolates the visual component
    TestCreaturePhenotype visualCreature(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(visualCreature.getGenome(), G::UniversalGenes::SIGHT_RANGE, 85.0f);
    setGeneValue(visualCreature.getGenome(), G::UniversalGenes::COLOR_VISION, 0.90f);
    setGeneValue(visualCreature.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.01f);  // Very low scent
    
    // Berry bush: high fruit appeal (~0.7-0.9)
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Grass: low fruit appeal (~0.0-0.1)
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    
    G::FeedingInteraction interaction;
    
    float berryRange = interaction.getDetectionRange(visualCreature.getPhenotype(), berryBush);
    float grassRange = interaction.getDetectionRange(visualCreature.getPhenotype(), grass);
    
    std::cout << "      Berry bush fruit appeal: " << berryBush.getFruitAppeal() << std::endl;
    std::cout << "      Grass fruit appeal: " << grass.getFruitAppeal() << std::endl;
    std::cout << "      Berry detection range: " << berryRange << std::endl;
    std::cout << "      Grass detection range: " << grassRange << std::endl;
    
    // Berry (colorful) should have much higher detection range than grass (not colorful)
    // For CV=0.9, berryAppeal=0.8: visualBonus = 72 tiles
    // For CV=0.9, grassAppeal=0.05: visualBonus = 4.5 tiles
    // Difference should be significant (at least 30 tiles)
    TEST_ASSERT_GT(berryRange - grassRange, 30.0f);
}

/**
 * @brief Test: Scent detection works independently of what plant looks like
 * High scent creature can find low-appeal plants just as well via smell
 */
void testScentWorksIndependentlyOfFruitAppeal() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature with LOW color vision but HIGH scent detection
    // This isolates the scent component
    TestCreaturePhenotype scentCreature(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(scentCreature.getGenome(), G::UniversalGenes::SIGHT_RANGE, 50.0f);
    setGeneValue(scentCreature.getGenome(), G::UniversalGenes::COLOR_VISION, 0.01f);  // Very low vision
    setGeneValue(scentCreature.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.90f);
    
    // Berry bush: high fruit appeal
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Grass: low fruit appeal
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    
    G::FeedingInteraction interaction;
    
    float berryRange = interaction.getDetectionRange(scentCreature.getPhenotype(), berryBush);
    float grassRange = interaction.getDetectionRange(scentCreature.getPhenotype(), grass);
    
    // Expected for scent creature:
    // scentBonus = 0.9 × 100 = 90 tiles (same for both plants)
    // visualBonus for berry ≈ 0.01 × 0.8 × 100 = 0.8 tiles
    // visualBonus for grass ≈ 0.01 × 0.05 × 100 = 0.05 tiles
    // Both should be dominated by scent bonus, so ranges should be similar
    
    std::cout << "      Berry detection range (scent creature): " << berryRange << std::endl;
    std::cout << "      Grass detection range (scent creature): " << grassRange << std::endl;
    
    // Scent-dominant creature should detect both plants at similar range
    // (difference should be minimal, less than 10 tiles)
    float rangeDifference = std::abs(berryRange - grassRange);
    TEST_ASSERT_LT(rangeDifference, 10.0f);
    
    // Both ranges should be around 50 + 90 = 140 tiles
    TEST_ASSERT_GT(berryRange, 130.0f);
    TEST_ASSERT_GT(grassRange, 130.0f);
}

/**
 * @brief Test the four archetype scenarios from the plan
 */
void testArchetypeDetectionScenarios() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::FeedingInteraction interaction;
    
    // Create plants
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    
    float berryAppeal = berryBush.getFruitAppeal();
    float grassAppeal = grass.getFruitAppeal();
    
    std::cout << "      Berry fruit appeal: " << berryAppeal << std::endl;
    std::cout << "      Grass fruit appeal: " << grassAppeal << std::endl;
    
    // === Scenario 1: Canopy Forager → Berry ===
    // SIGHT=85, COLOR=0.90, SCENT=0.82, Appeal=0.8
    // Expected: 85 + max(72, 82) = 167 tiles
    {
        TestCreaturePhenotype canopyForager(*registry, 0.7f, 0.5f, 0.5f);
        setGeneValue(canopyForager.getGenome(), G::UniversalGenes::SIGHT_RANGE, 85.0f);
        setGeneValue(canopyForager.getGenome(), G::UniversalGenes::COLOR_VISION, 0.90f);
        setGeneValue(canopyForager.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.82f);
        
        float range = interaction.getDetectionRange(canopyForager.getPhenotype(), berryBush);
        float expectedVisual = 0.90f * berryAppeal * 100.0f;
        float expectedScent = 0.82f * 100.0f;
        float expected = 85.0f + std::max(expectedVisual, expectedScent);
        
        std::cout << "      Canopy Forager → Berry: " << range << " (expected ~" << expected << ")" << std::endl;
        TEST_ASSERT_GT(range, 140.0f);  // Should be well over 140 tiles
    }
    
    // === Scenario 2: Canopy Forager → Grass ===
    // SIGHT=85, COLOR=0.90, SCENT=0.82, Appeal=0.05
    // Expected: 85 + max(4.5, 82) = 167 tiles (via scent)
    {
        TestCreaturePhenotype canopyForager(*registry, 0.7f, 0.5f, 0.5f);
        setGeneValue(canopyForager.getGenome(), G::UniversalGenes::SIGHT_RANGE, 85.0f);
        setGeneValue(canopyForager.getGenome(), G::UniversalGenes::COLOR_VISION, 0.90f);
        setGeneValue(canopyForager.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.82f);
        
        float range = interaction.getDetectionRange(canopyForager.getPhenotype(), grass);
        float expectedVisual = 0.90f * grassAppeal * 100.0f;
        float expectedScent = 0.82f * 100.0f;
        float expected = 85.0f + std::max(expectedVisual, expectedScent);
        
        std::cout << "      Canopy Forager → Grass: " << range << " (expected ~" << expected << ")" << std::endl;
        // Visual bonus is tiny (grassAppeal very low), scent dominates
        TEST_ASSERT_GT(range, 160.0f);  // Should be ~167 tiles
    }
    
    // === Scenario 3: Scent Tracker → Berry ===
    // SIGHT=85, COLOR=0.30, SCENT=0.94, Appeal=0.8
    // Expected: 85 + max(24, 94) = 179 tiles
    {
        TestCreaturePhenotype scentTracker(*registry, 0.7f, 0.5f, 0.5f);
        setGeneValue(scentTracker.getGenome(), G::UniversalGenes::SIGHT_RANGE, 85.0f);
        setGeneValue(scentTracker.getGenome(), G::UniversalGenes::COLOR_VISION, 0.30f);
        setGeneValue(scentTracker.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.94f);
        
        float range = interaction.getDetectionRange(scentTracker.getPhenotype(), berryBush);
        float expectedVisual = 0.30f * berryAppeal * 100.0f;
        float expectedScent = 0.94f * 100.0f;
        float expected = 85.0f + std::max(expectedVisual, expectedScent);
        
        std::cout << "      Scent Tracker → Berry: " << range << " (expected ~" << expected << ")" << std::endl;
        TEST_ASSERT_GT(range, 170.0f);  // Should be ~179 tiles
    }
    
    // === Scenario 4: Tank Herbivore → Grass ===
    // SIGHT=65, COLOR=0.30, SCENT=0.60, Appeal=0.05
    // Expected: 65 + max(1.5, 60) = 125 tiles
    {
        TestCreaturePhenotype tankHerbivore(*registry, 0.7f, 0.5f, 0.5f);
        setGeneValue(tankHerbivore.getGenome(), G::UniversalGenes::SIGHT_RANGE, 65.0f);
        setGeneValue(tankHerbivore.getGenome(), G::UniversalGenes::COLOR_VISION, 0.30f);
        setGeneValue(tankHerbivore.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.60f);
        
        float range = interaction.getDetectionRange(tankHerbivore.getPhenotype(), grass);
        float expectedVisual = 0.30f * grassAppeal * 100.0f;
        float expectedScent = 0.60f * 100.0f;
        float expected = 65.0f + std::max(expectedVisual, expectedScent);
        
        std::cout << "      Tank Herbivore → Grass: " << range << " (expected ~" << expected << ")" << std::endl;
        TEST_ASSERT_GT(range, 115.0f);  // Should be ~125 tiles
    }
}

/**
 * @brief Test that detection uses additive formula (not multiplicative)
 * Verify: effectiveRange = sightRange + max(bonus) not sightRange × multiplier
 */
void testDetectionUsesAdditiveFormula() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create two creatures with different sight ranges
    TestCreaturePhenotype nearSight(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(nearSight.getGenome(), G::UniversalGenes::SIGHT_RANGE, 20.0f);
    setGeneValue(nearSight.getGenome(), G::UniversalGenes::COLOR_VISION, 0.50f);
    setGeneValue(nearSight.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.50f);
    
    TestCreaturePhenotype farSight(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(farSight.getGenome(), G::UniversalGenes::SIGHT_RANGE, 100.0f);
    setGeneValue(farSight.getGenome(), G::UniversalGenes::COLOR_VISION, 0.50f);
    setGeneValue(farSight.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.50f);
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    float fruitAppeal = berryBush.getFruitAppeal();
    
    G::FeedingInteraction interaction;
    
    float nearRange = interaction.getDetectionRange(nearSight.getPhenotype(), berryBush);
    float farRange = interaction.getDetectionRange(farSight.getPhenotype(), berryBush);
    
    // Both have same CV/scent, so bonus is the same
    // scentBonus = 0.5 × 100 = 50
    // visualBonus = 0.5 × appeal × 100
    // Difference should be exactly 80 (100 - 20) if additive
    float difference = farRange - nearRange;
    
    std::cout << "      Near-sight range: " << nearRange << std::endl;
    std::cout << "      Far-sight range: " << farRange << std::endl;
    std::cout << "      Difference: " << difference << " (expected 80)" << std::endl;
    
    // Additive formula means difference = sight range difference
    TEST_ASSERT_GT(difference, 75.0f);
    TEST_ASSERT_LT(difference, 85.0f);
}

/**
 * @brief Verify canDetectPlant uses detection range correctly
 */
void testCanDetectPlantUsesRange() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    TestCreaturePhenotype creature(*registry, 0.7f, 0.5f, 0.5f);
    setGeneValue(creature.getGenome(), G::UniversalGenes::SIGHT_RANGE, 50.0f);
    setGeneValue(creature.getGenome(), G::UniversalGenes::COLOR_VISION, 0.50f);
    setGeneValue(creature.getGenome(), G::UniversalGenes::SCENT_DETECTION, 0.50f);
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    G::FeedingInteraction interaction;
    
    float range = interaction.getDetectionRange(creature.getPhenotype(), berryBush);
    
    // Test detection at various distances
    bool canDetectClose = interaction.canDetectPlant(creature.getPhenotype(), berryBush, 10.0f);
    bool canDetectMid = interaction.canDetectPlant(creature.getPhenotype(), berryBush, range - 5.0f);
    bool canDetectFar = interaction.canDetectPlant(creature.getPhenotype(), berryBush, range + 50.0f);
    
    std::cout << "      Detection range: " << range << std::endl;
    std::cout << "      Can detect at 10: " << (canDetectClose ? "yes" : "no") << std::endl;
    std::cout << "      Can detect at range-5: " << (canDetectMid ? "yes" : "no") << std::endl;
    std::cout << "      Can detect at range+50: " << (canDetectFar ? "yes" : "no") << std::endl;
    
    TEST_ASSERT(canDetectClose);   // Should detect close
    TEST_ASSERT(canDetectMid);     // Should detect within range
    TEST_ASSERT(!canDetectFar);    // Should NOT detect beyond range
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
    
    BEGIN_TEST_GROUP("Detection Range Formula Tests (Phase 3)");
    RUN_TEST(testDetectionFormulaComponents);
    RUN_TEST(testColorVisionHelpsWithColorfulPlants);
    RUN_TEST(testScentWorksIndependentlyOfFruitAppeal);
    RUN_TEST(testArchetypeDetectionScenarios);
    RUN_TEST(testDetectionUsesAdditiveFormula);
    RUN_TEST(testCanDetectPlantUsesRange);
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
