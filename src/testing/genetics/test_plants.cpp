/**
 * @file test_plants.cpp
 * @brief Tests for Plant System (Phase 2.3)
 * 
 * Tests plant defense, food production, seed dispersal, and PlantFactory.
 */

#include <iostream>
#include <memory>
#include "test_framework.hpp"

#include "genetics/core/GeneticTypes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "rendering/RenderTypes.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Plant Defense Tests
// ============================================================================

void testPlantToxicity() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(10, 20, registry);
    
    float toxicity = plant.getToxicity();
    TEST_ASSERT_GE(toxicity, 0.0f);
    TEST_ASSERT_LE(toxicity, 1.0f);
}

void testPlantThornDamage() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(10, 20, registry);
    
    float thornDamage = plant.getThornDamage();
    TEST_ASSERT_GE(thornDamage, 0.0f);
    TEST_ASSERT_LE(thornDamage, 1.0f);
}

void testPlantRegrowthRate() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(10, 20, registry);
    
    float regrowthRate = plant.getRegrowthRate();
    TEST_ASSERT_GE(regrowthRate, 0.0f);
    TEST_ASSERT_LE(regrowthRate, 1.0f);
}

// ============================================================================
// Plant Food Production Tests
// ============================================================================

void testPlantFruitProductionRate() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(10, 20, registry);
    
    float fruitRate = plant.getFruitProductionRate();
    TEST_ASSERT_GE(fruitRate, 0.0f);
    TEST_ASSERT_LE(fruitRate, 1.0f);
}

void testPlantFruitAppeal() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(10, 20, registry);
    
    float fruitAppeal = plant.getFruitAppeal();
    TEST_ASSERT_GE(fruitAppeal, 0.0f);
    TEST_ASSERT_LE(fruitAppeal, 1.0f);
}

// ============================================================================
// Seed Propagation Genes Tests
// ============================================================================

void testSeedPropagationGenesRegistered() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::SEED_MASS));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::SEED_AERODYNAMICS));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::SEED_HOOK_STRENGTH));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::EXPLOSIVE_POD_FORCE));
    TEST_ASSERT(registry.hasGene(G::UniversalGenes::RUNNER_PRODUCTION));
}

void testSeedPropagationGenesCategories() {
    TEST_ASSERT_EQ(G::GeneCategory::Reproduction, G::UniversalGenes::getCategory(G::UniversalGenes::SEED_MASS));
    TEST_ASSERT_EQ(G::GeneCategory::Reproduction, G::UniversalGenes::getCategory(G::UniversalGenes::SEED_AERODYNAMICS));
    TEST_ASSERT_EQ(G::GeneCategory::Reproduction, G::UniversalGenes::getCategory(G::UniversalGenes::SEED_HOOK_STRENGTH));
}

void testSeedPropagationGenesInPlant() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome plantGenome = G::UniversalGenes::createPlantGenome(registry);
    TEST_ASSERT(plantGenome.hasGene(G::UniversalGenes::SEED_MASS));
    TEST_ASSERT(plantGenome.hasGene(G::UniversalGenes::SEED_AERODYNAMICS));
    TEST_ASSERT(plantGenome.hasGene(G::UniversalGenes::SEED_HOOK_STRENGTH));
}

// ============================================================================
// Emergent Dispersal Strategy Tests
// ============================================================================

void testDispersalStrategyValid() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    G::DispersalStrategy strategy = plant.getPrimaryDispersalStrategy();
    
    TEST_ASSERT(strategy == G::DispersalStrategy::GRAVITY ||
                strategy == G::DispersalStrategy::WIND ||
                strategy == G::DispersalStrategy::ANIMAL_FRUIT ||
                strategy == G::DispersalStrategy::ANIMAL_BURR ||
                strategy == G::DispersalStrategy::EXPLOSIVE ||
                strategy == G::DispersalStrategy::VEGETATIVE);
}

void testSeedProperties() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(0, 0, registry);
    
    float seedMass = plant.getSeedMass();
    float seedAero = plant.getSeedAerodynamics();
    float seedHook = plant.getSeedHookStrength();
    float seedDurability = plant.getSeedCoatDurability();
    
    TEST_ASSERT_GE(seedMass, 0.0f);
    TEST_ASSERT_GE(seedAero, 0.0f);
    TEST_ASSERT_LE(seedAero, 1.0f);
    TEST_ASSERT_GE(seedHook, 0.0f);
    TEST_ASSERT_LE(seedHook, 1.0f);
    TEST_ASSERT_GE(seedDurability, 0.0f);
    TEST_ASSERT_LE(seedDurability, 1.0f);
}

// ============================================================================
// PlantFactory Tests
// ============================================================================

void testPlantFactoryDefaultTemplates() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::string> templateNames = factory.getTemplateNames();
    TEST_ASSERT_EQ(4u, templateNames.size());
    
    TEST_ASSERT(factory.hasTemplate("berry_bush"));
    TEST_ASSERT(factory.hasTemplate("oak_tree"));
    TEST_ASSERT(factory.hasTemplate("grass"));
    TEST_ASSERT(factory.hasTemplate("thorn_bush"));
}

void testPlantFactoryCreateFromTemplate() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 10, 20);
    TEST_ASSERT_EQ(10, berryBush.getX());
    TEST_ASSERT_EQ(20, berryBush.getY());
    TEST_ASSERT_EQ(EntityType::PLANT_BERRY_BUSH, berryBush.getEntityType());
}

void testPlantFactoryCreateOffspring() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant parent1 = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant parent2 = factory.createFromTemplate("berry_bush", 5, 5);
    
    G::Plant offspring = factory.createOffspring(parent1, parent2, 2, 2);
    TEST_ASSERT_EQ(2, offspring.getX());
    TEST_ASSERT_EQ(2, offspring.getY());
}

void testPlantFactoryCreateRandom() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant randomPlant = factory.createRandom(100, 100);
    TEST_ASSERT_EQ(100, randomPlant.getX());
    TEST_ASSERT_EQ(100, randomPlant.getY());
    TEST_ASSERT(randomPlant.isAlive());
}

// ============================================================================
// Species Template Tests
// ============================================================================

void testBerryBushTemplate() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    
    // Berry bush should have positive fruit production
    float fruitRate = berryBush.getFruitProductionRate();
    TEST_ASSERT_GE(fruitRate, 0.0f);
    TEST_ASSERT_LE(fruitRate, 1.0f);
    
    // Berry bush should have low thorns
    float thornDensity = berryBush.getThornDamage();
    TEST_ASSERT_LE(thornDensity, 0.5f);
    
    // Verify correct entity type
    TEST_ASSERT_EQ(EntityType::PLANT_BERRY_BUSH, berryBush.getEntityType());
}

void testOakTreeTemplate() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant oakTree = factory.createFromTemplate("oak_tree", 0, 0);
    TEST_ASSERT_EQ(EntityType::PLANT_OAK_TREE, oakTree.getEntityType());
    
    // Oak tree should have positive hardiness
    float hardiness = oakTree.getHardiness();
    TEST_ASSERT_GE(hardiness, 0.0f);
    TEST_ASSERT_LE(hardiness, 1.0f);
}

void testGrassTemplate() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    TEST_ASSERT_EQ(EntityType::PLANT_GRASS, grass.getEntityType());
    
    // Grass should have positive regrowth
    float regrowth = grass.getRegrowthRate();
    TEST_ASSERT_GE(regrowth, 0.0f);
    TEST_ASSERT_LE(regrowth, 1.0f);
}

void testThornBushTemplate() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    TEST_ASSERT_EQ(EntityType::PLANT_THORN_BUSH, thornBush.getEntityType());
    
    // Thorn bush should have positive thorns
    float thornDensity = thornBush.getThornDamage();
    TEST_ASSERT_GE(thornDensity, 0.0f);
    TEST_ASSERT_LE(thornDensity, 1.0f);
    
    // Thorn bush should have positive toxins
    float toxicity = thornBush.getToxicity();
    TEST_ASSERT_GE(toxicity, 0.0f);
    TEST_ASSERT_LE(toxicity, 1.0f);
}

// ============================================================================
// Plant Rendering Tests
// ============================================================================

void testPlantRenderCharacters() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 0, 0);
    TEST_ASSERT_EQ('B', berryBush.getRenderCharacter());
    
    G::Plant oakTree = factory.createFromTemplate("oak_tree", 0, 0);
    TEST_ASSERT_EQ('T', oakTree.getRenderCharacter());
    
    G::Plant grass = factory.createFromTemplate("grass", 0, 0);
    TEST_ASSERT_EQ('"', grass.getRenderCharacter());
    
    G::Plant thornBush = factory.createFromTemplate("thorn_bush", 0, 0);
    TEST_ASSERT_EQ('*', thornBush.getRenderCharacter());
}

void testPlantUniqueIds() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant plant1 = factory.createFromTemplate("berry_bush", 0, 0);
    G::Plant plant2 = factory.createFromTemplate("oak_tree", 0, 0);
    
    TEST_ASSERT(plant1.getId() != plant2.getId());
}

// ============================================================================
// Test Runner
// ============================================================================

void runPlantTests() {
    BEGIN_TEST_GROUP("Plant Defense Tests");
    RUN_TEST(testPlantToxicity);
    RUN_TEST(testPlantThornDamage);
    RUN_TEST(testPlantRegrowthRate);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Plant Food Production Tests");
    RUN_TEST(testPlantFruitProductionRate);
    RUN_TEST(testPlantFruitAppeal);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Seed Propagation Genes Tests");
    RUN_TEST(testSeedPropagationGenesRegistered);
    RUN_TEST(testSeedPropagationGenesCategories);
    RUN_TEST(testSeedPropagationGenesInPlant);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Emergent Dispersal Strategy Tests");
    RUN_TEST(testDispersalStrategyValid);
    RUN_TEST(testSeedProperties);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("PlantFactory Tests");
    RUN_TEST(testPlantFactoryDefaultTemplates);
    RUN_TEST(testPlantFactoryCreateFromTemplate);
    RUN_TEST(testPlantFactoryCreateOffspring);
    RUN_TEST(testPlantFactoryCreateRandom);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Species Template Tests");
    RUN_TEST(testBerryBushTemplate);
    RUN_TEST(testOakTreeTemplate);
    RUN_TEST(testGrassTemplate);
    RUN_TEST(testThornBushTemplate);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Plant Rendering Tests");
    RUN_TEST(testPlantRenderCharacters);
    RUN_TEST(testPlantUniqueIds);
    END_TEST_GROUP();
}

#ifdef TEST_PLANTS_STANDALONE
int main() {
    std::cout << "=== Plant System Tests (Phase 2.3) ===" << std::endl;
    runPlantTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
