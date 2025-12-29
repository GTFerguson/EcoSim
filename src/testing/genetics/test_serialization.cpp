/**
 * @file test_serialization.cpp
 * @brief Comprehensive tests for JSON save/load system
 * 
 * Tests all serialization layers:
 * - Gene serialization
 * - Chromosome serialization
 * - Genome serialization
 * - Creature serialization
 * - Plant serialization
 * - FileHandling integration
 * - SaveMetadata queries
 */

#include <iostream>
#include <memory>
#include <filesystem>
#include <fstream>
#include "test_framework.hpp"

#include "genetics/core/Gene.hpp"
#include "genetics/core/Chromosome.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "objects/creature/creature.hpp"
#include "fileHandling.hpp"
#include "calendar.hpp"
#include "world/world.hpp"

#include <nlohmann/json.hpp>

namespace G = EcoSim::Genetics;
namespace fs = std::filesystem;
using json = nlohmann::json;

// ============================================================================
// Test Helpers
// ============================================================================

namespace {

// Temporary directory for test saves
const std::string TEST_SAVE_DIR = "test_saves_temp";

// Helper to create a World for testing
World createTestWorld(unsigned rows = 50, unsigned cols = 50) {
    MapGen mapGen;
    mapGen.rows = rows;
    mapGen.cols = cols;
    mapGen.seed = 12345.0;
    mapGen.scale = 0.01;
    mapGen.freq = 1.0;
    mapGen.exponent = 1.0;
    mapGen.terraces = 20;
    mapGen.isIsland = false;
    
    OctaveGen octaveGen;
    octaveGen.quantity = 4;
    octaveGen.minWeight = 0.1;
    octaveGen.maxWeight = 0.5;
    octaveGen.freqInterval = 1.0;
    
    return World(mapGen, octaveGen);
}

// Helper to create a test gene with specific values
G::Gene createTestGene(const std::string& id, float val1, float val2) {
    G::Allele a1{G::GeneValue{val1}};
    G::Allele a2{G::GeneValue{val2}};
    return G::Gene(id, a1, a2);
}

// Helper to create a homozygous test gene
G::Gene createHomozygousGene(const std::string& id, float value) {
    return G::Gene(id, G::GeneValue{value});
}

// Helper to create a test creature with specific state
std::unique_ptr<Creature> createTestCreature(int x, int y, float hunger, float thirst) {
    Creature::initializeGeneRegistry();
    auto& registry = Creature::getGeneRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);
    auto genomePtr = std::make_unique<G::Genome>(genome);
    return std::make_unique<Creature>(x, y, hunger, thirst, std::move(genomePtr));
}

// Helper to create test plant
std::unique_ptr<G::Plant> createTestPlant(int x, int y, G::GeneRegistry& registry) {
    return std::make_unique<G::Plant>(x, y, registry);
}

// Helper to clean up test directory
void cleanupTestDir() {
    if (fs::exists(TEST_SAVE_DIR)) {
        fs::remove_all(TEST_SAVE_DIR);
    }
    // Also clean up the saves subdirectory that FileHandling creates
    if (fs::exists("saves/" + TEST_SAVE_DIR)) {
        fs::remove_all("saves/" + TEST_SAVE_DIR);
    }
}

// Helper to setup test directory
void setupTestDir() {
    cleanupTestDir();
    // FileHandling prepends "saves/" to the path, so we need to create that structure
    fs::create_directories("saves/" + TEST_SAVE_DIR);
    fs::create_directories(TEST_SAVE_DIR);
}

} // anonymous namespace

// ============================================================================
// Gene Serialization Tests
// ============================================================================

void testGeneRoundtrip() {
    // Create gene with specific values
    G::Gene original = createTestGene("test_gene", 0.75f, 0.25f);
    
    // Serialize to JSON
    json j = original.toJson();
    
    // Deserialize back
    G::Gene restored = G::Gene::fromJson(j);
    
    // Verify all values match
    TEST_ASSERT_EQ(original.getId(), restored.getId());
    
    // Check allele values (using numeric extraction)
    float origVal1 = original.getAllele1().value.index() == 0 
        ? std::get<float>(original.getAllele1().value) : 0.0f;
    float origVal2 = original.getAllele2().value.index() == 0 
        ? std::get<float>(original.getAllele2().value) : 0.0f;
    float restVal1 = restored.getAllele1().value.index() == 0 
        ? std::get<float>(restored.getAllele1().value) : 0.0f;
    float restVal2 = restored.getAllele2().value.index() == 0 
        ? std::get<float>(restored.getAllele2().value) : 0.0f;
    
    TEST_ASSERT_NEAR(origVal1, restVal1, 0.0001f);
    TEST_ASSERT_NEAR(origVal2, restVal2, 0.0001f);
}

void testGeneHomozygousRoundtrip() {
    // Create homozygous gene
    G::Gene original = createHomozygousGene("homozygous_gene", 0.5f);
    
    json j = original.toJson();
    G::Gene restored = G::Gene::fromJson(j);
    
    TEST_ASSERT_EQ(original.getId(), restored.getId());
    
    // Both alleles should have same value
    float restVal1 = std::get<float>(restored.getAllele1().value);
    float restVal2 = std::get<float>(restored.getAllele2().value);
    TEST_ASSERT_NEAR(restVal1, restVal2, 0.0001f);
    TEST_ASSERT_NEAR(restVal1, 0.5f, 0.0001f);
}

void testGeneEdgeValues() {
    // Test with min value (0.0)
    G::Gene minGene = createHomozygousGene("min_gene", 0.0f);
    json jMin = minGene.toJson();
    G::Gene restoredMin = G::Gene::fromJson(jMin);
    TEST_ASSERT_NEAR(std::get<float>(restoredMin.getAllele1().value), 0.0f, 0.0001f);
    
    // Test with max value (1.0)
    G::Gene maxGene = createHomozygousGene("max_gene", 1.0f);
    json jMax = maxGene.toJson();
    G::Gene restoredMax = G::Gene::fromJson(jMax);
    TEST_ASSERT_NEAR(std::get<float>(restoredMax.getAllele1().value), 1.0f, 0.0001f);
    
    // Test with large value (for genes like LIFESPAN)
    G::Gene largeGene = createHomozygousGene("large_gene", 5000.0f);
    json jLarge = largeGene.toJson();
    G::Gene restoredLarge = G::Gene::fromJson(jLarge);
    TEST_ASSERT_NEAR(std::get<float>(restoredLarge.getAllele1().value), 5000.0f, 0.1f);
}

void testGeneFromJsonMissingId() {
    json j = {
        {"allele1", {{"value", 0.5f}}},
        {"allele2", {{"value", 0.5f}}}
    };
    
    bool threw = false;
    try {
        G::Gene::fromJson(j);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);
}

void testGeneFromJsonMissingAllele() {
    json j = {
        {"id", "test_gene"},
        {"allele1", {{"value", 0.5f}}}
        // Missing allele2
    };
    
    bool threw = false;
    try {
        G::Gene::fromJson(j);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);
}

void testGeneFromJsonMissingValue() {
    json j = {
        {"id", "test_gene"},
        {"allele1", {{"value", 0.5f}}},
        {"allele2", {}}  // Missing value field
    };
    
    bool threw = false;
    try {
        G::Gene::fromJson(j);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);
}

// ============================================================================
// Chromosome Serialization Tests
// ============================================================================

void testChromosomeRoundtrip() {
    // Create chromosome with multiple genes
    G::Chromosome original(G::ChromosomeType::Metabolism);
    original.addGene(createTestGene("gene_a", 0.3f, 0.7f));
    original.addGene(createTestGene("gene_b", 0.5f, 0.5f));
    original.addGene(createTestGene("gene_c", 0.1f, 0.9f));
    
    // Serialize and deserialize
    json j = original.toJson();
    G::Chromosome restored = G::Chromosome::fromJson(j);
    
    // Verify type
    TEST_ASSERT_EQ(static_cast<int>(original.getType()), static_cast<int>(restored.getType()));
    
    // Verify all genes present
    TEST_ASSERT_EQ(original.size(), restored.size());
    TEST_ASSERT(restored.hasGene("gene_a"));
    TEST_ASSERT(restored.hasGene("gene_b"));
    TEST_ASSERT(restored.hasGene("gene_c"));
}

void testEmptyChromosome() {
    G::Chromosome empty(G::ChromosomeType::Sensory);
    TEST_ASSERT_EQ(0u, empty.size());
    
    json j = empty.toJson();
    G::Chromosome restored = G::Chromosome::fromJson(j);
    
    TEST_ASSERT_EQ(0u, restored.size());
    TEST_ASSERT_EQ(static_cast<int>(G::ChromosomeType::Sensory), static_cast<int>(restored.getType()));
}

void testAllChromosomeTypes() {
    std::vector<G::ChromosomeType> types = {
        G::ChromosomeType::Morphology,
        G::ChromosomeType::Sensory,
        G::ChromosomeType::Metabolism,
        G::ChromosomeType::Locomotion,
        G::ChromosomeType::Behavior,
        G::ChromosomeType::Reproduction,
        G::ChromosomeType::Environmental,
        G::ChromosomeType::Lifespan
    };
    
    for (auto type : types) {
        G::Chromosome original(type);
        original.addGene(createHomozygousGene("test_gene", 0.5f));
        
        json j = original.toJson();
        G::Chromosome restored = G::Chromosome::fromJson(j);
        
        TEST_ASSERT_EQ(static_cast<int>(type), static_cast<int>(restored.getType()));
    }
}

void testChromosomeFromJsonMissingType() {
    json j = {
        {"genes", json::array()}
    };
    
    bool threw = false;
    try {
        G::Chromosome::fromJson(j);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);
}

void testChromosomeFromJsonMissingGenes() {
    json j = {
        {"type", "Metabolism"}
    };
    
    bool threw = false;
    try {
        G::Chromosome::fromJson(j);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);
}

void testChromosomeFromJsonInvalidType() {
    json j = {
        {"type", "InvalidType"},
        {"genes", json::array()}
    };
    
    bool threw = false;
    try {
        G::Chromosome::fromJson(j);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);
}

// ============================================================================
// Genome Serialization Tests
// ============================================================================

void testGenomeRoundtrip() {
    // Create genome with genes on multiple chromosomes
    G::Genome original;
    original.addGene(createTestGene("morph_gene", 0.3f, 0.7f), G::ChromosomeType::Morphology);
    original.addGene(createTestGene("meta_gene", 0.5f, 0.5f), G::ChromosomeType::Metabolism);
    original.addGene(createTestGene("life_gene", 0.2f, 0.8f), G::ChromosomeType::Lifespan);
    
    // Serialize and deserialize
    json j = original.toJson();
    G::Genome restored = G::Genome::fromJson(j);
    
    // Verify genes preserved
    TEST_ASSERT(restored.hasGene("morph_gene"));
    TEST_ASSERT(restored.hasGene("meta_gene"));
    TEST_ASSERT(restored.hasGene("life_gene"));
    
    // Verify values preserved
    float origVal = original.getGene("morph_gene").getNumericValue(G::DominanceType::Incomplete);
    float restVal = restored.getGene("morph_gene").getNumericValue(G::DominanceType::Incomplete);
    TEST_ASSERT_NEAR(origVal, restVal, 0.0001f);
}

void testGenomeWithCreatureGenes() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome original = G::UniversalGenes::createCreatureGenome(registry);
    
    // Verify original has expected genes
    TEST_ASSERT(original.hasGene(G::UniversalGenes::LIFESPAN));
    TEST_ASSERT(original.hasGene(G::UniversalGenes::SIGHT_RANGE));
    
    // Serialize and deserialize
    json j = original.toJson();
    G::Genome restored = G::Genome::fromJson(j);
    
    // Verify all genes preserved
    TEST_ASSERT(restored.hasGene(G::UniversalGenes::LIFESPAN));
    TEST_ASSERT(restored.hasGene(G::UniversalGenes::SIGHT_RANGE));
    
    // Verify gene count matches
    TEST_ASSERT_EQ(original.getTotalGeneCount(), restored.getTotalGeneCount());
}

void testGenomeWithPlantGenes() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Genome original = G::UniversalGenes::createPlantGenome(registry);
    
    // Serialize and deserialize
    json j = original.toJson();
    G::Genome restored = G::Genome::fromJson(j);
    
    // Verify gene count matches
    TEST_ASSERT_EQ(original.getTotalGeneCount(), restored.getTotalGeneCount());
}

void testGenomeLoadFromJson() {
    // Test the loadFromJson method (updates existing genome)
    G::Genome original;
    original.addGene(createTestGene("original_gene", 0.1f, 0.2f), G::ChromosomeType::Behavior);
    
    // Create different genome data
    G::Genome other;
    other.addGene(createTestGene("new_gene", 0.8f, 0.9f), G::ChromosomeType::Sensory);
    json otherJson = other.toJson();
    
    // Load into original
    original.loadFromJson(otherJson);
    
    // Should have new gene, not original
    TEST_ASSERT(original.hasGene("new_gene"));
}

void testGenomeFromJsonMissingChromosomes() {
    json j = {};  // Missing chromosomes array
    
    bool threw = false;
    try {
        G::Genome::fromJson(j);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);
}

// ============================================================================
// Creature Serialization Tests
// ============================================================================

void testCreatureRoundtrip() {
    Creature::initializeGeneRegistry();
    
    // Create creature with specific state
    auto creature = createTestCreature(50, 75, 0.3f, 0.6f);
    creature->setFatigue(0.2f);
    creature->setMate(0.5f);
    creature->setAge(100);
    creature->setMotivation(Motivation::Hungry);
    creature->setAction(Action::Searching);
    
    // Serialize
    json j = creature->toJson();
    
    // Deserialize
    Creature restored = Creature::fromJson(j, 200, 200);
    
    // Verify state
    TEST_ASSERT_EQ(creature->getX(), restored.getX());
    TEST_ASSERT_EQ(creature->getY(), restored.getY());
    TEST_ASSERT_NEAR(creature->getHunger(), restored.getHunger(), 0.01f);
    TEST_ASSERT_NEAR(creature->getThirst(), restored.getThirst(), 0.01f);
    TEST_ASSERT_NEAR(creature->getFatigue(), restored.getFatigue(), 0.01f);
    TEST_ASSERT_EQ(creature->getAge(), restored.getAge());
}

void testCreaturePositionBounds() {
    Creature::initializeGeneRegistry();
    
    // Create creature
    auto creature = createTestCreature(150, 180, 0.5f, 0.5f);
    
    json j = creature->toJson();
    
    // Load with smaller map - position should be clamped
    Creature restored = Creature::fromJson(j, 100, 100);
    
    // Position should be clamped to map bounds
    TEST_ASSERT_LT(restored.getX(), 100);
    TEST_ASSERT_LT(restored.getY(), 100);
    TEST_ASSERT_GE(restored.getX(), 0);
    TEST_ASSERT_GE(restored.getY(), 0);
}

void testCreatureWithCombatState() {
    Creature::initializeGeneRegistry();
    
    auto creature = createTestCreature(25, 30, 0.5f, 0.5f);
    creature->setInCombat(true);
    creature->setTargetId(42);
    creature->setCombatCooldown(5);
    creature->setFleeing(true);
    
    json j = creature->toJson();
    Creature restored = Creature::fromJson(j, 100, 100);
    
    TEST_ASSERT(restored.isInCombat());
    TEST_ASSERT_EQ(42, restored.getTargetId());
    TEST_ASSERT_EQ(5, restored.getCombatCooldown());
    TEST_ASSERT(restored.isFleeing());
}

void testCreatureWithDamage() {
    Creature::initializeGeneRegistry();
    
    auto creature = createTestCreature(10, 10, 0.5f, 0.5f);
    float maxHealth = creature->getMaxHealth();
    creature->takeDamage(maxHealth * 0.3f);  // 30% damage
    
    json j = creature->toJson();
    Creature restored = Creature::fromJson(j, 100, 100);
    
    // Health should be restored
    TEST_ASSERT_NEAR(creature->getHealth(), restored.getHealth(), 0.1f);
    TEST_ASSERT_LT(restored.getHealth(), maxHealth);
}

void testCreaturePhenotypeRegenerated() {
    Creature::initializeGeneRegistry();
    
    auto creature = createTestCreature(10, 10, 0.5f, 0.5f);
    
    json j = creature->toJson();
    Creature restored = Creature::fromJson(j, 100, 100);
    
    // Phenotype should be regenerated and functional
    const auto& phenotype = restored.getPhenotype();
    
    // Should have trait values (phenotype regenerated from genome)
    float sightRange = restored.getSightRange();
    TEST_ASSERT_GT(sightRange, 0.0f);
}

void testCreatureWorldPosition() {
    Creature::initializeGeneRegistry();
    
    auto creature = createTestCreature(10, 10, 0.5f, 0.5f);
    creature->setWorldPosition(10.5f, 10.7f);
    
    json j = creature->toJson();
    Creature restored = Creature::fromJson(j, 100, 100);
    
    // World position should be preserved
    TEST_ASSERT_NEAR(creature->getWorldX(), restored.getWorldX(), 0.01f);
    TEST_ASSERT_NEAR(creature->getWorldY(), restored.getWorldY(), 0.01f);
}

// ============================================================================
// Plant Serialization Tests
// ============================================================================

void testPlantRoundtrip() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant original(25, 50, registry);
    
    // Set some state
    for (int i = 0; i < 100; i++) {
        G::EnvironmentState env;
        original.update(env);
    }
    
    json j = original.toJson();
    G::Plant restored = G::Plant::fromJson(j, registry);
    
    // Verify position
    TEST_ASSERT_EQ(original.getX(), restored.getX());
    TEST_ASSERT_EQ(original.getY(), restored.getY());
    
    // Verify lifecycle state
    TEST_ASSERT_EQ(original.getAge(), restored.getAge());
    TEST_ASSERT_EQ(original.isAlive(), restored.isAlive());
}

void testPlantLifeStages() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    // Test seedling
    G::Plant seedling(10, 10, registry);
    json jSeedling = seedling.toJson();
    G::Plant restoredSeedling = G::Plant::fromJson(jSeedling, registry);
    TEST_ASSERT_EQ(seedling.isAlive(), restoredSeedling.isAlive());
    
    // Test mature plant (age it)
    G::Plant mature(20, 20, registry);
    G::EnvironmentState env;
    for (int i = 0; i < 1000; i++) {
        mature.update(env);
    }
    
    json jMature = mature.toJson();
    G::Plant restoredMature = G::Plant::fromJson(jMature, registry);
    TEST_ASSERT_EQ(mature.getAge(), restoredMature.getAge());
}

void testPlantWithSeeds() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Berry bush should have seeds
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 30, 30);
    
    // Age it to maturity
    G::EnvironmentState env;
    for (int i = 0; i < 1500; i++) {
        berryBush.update(env);
    }
    
    json j = berryBush.toJson();
    G::Plant restored = G::Plant::fromJson(j, *registry);
    
    // Seed state should be preserved
    TEST_ASSERT_EQ(berryBush.getSeedCount(), restored.getSeedCount());
}

void testPlantDispersalStrategies() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Test different plant types
    std::vector<std::string> templates = {"berry_bush", "oak_tree", "grass", "thorn_bush"};
    
    for (const auto& templateName : templates) {
        G::Plant plant = factory.createFromTemplate(templateName, 10, 10);
        G::DispersalStrategy strategy = plant.getPrimaryDispersalStrategy();
        
        json j = plant.toJson();
        G::Plant restored = G::Plant::fromJson(j, *registry);
        
        // Dispersal strategy should be consistent after reload (derived from genes)
        TEST_ASSERT_EQ(static_cast<int>(strategy), 
                      static_cast<int>(restored.getPrimaryDispersalStrategy()));
    }
}

void testPlantEntityTypePreserved() {
    auto registry = std::make_shared<G::GeneRegistry>();
    G::UniversalGenes::registerDefaults(*registry);
    
    G::PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    G::Plant berryBush = factory.createFromTemplate("berry_bush", 10, 10);
    TEST_ASSERT_EQ(EntityType::PLANT_BERRY_BUSH, berryBush.getEntityType());
    
    json j = berryBush.toJson();
    G::Plant restored = G::Plant::fromJson(j, *registry);
    
    TEST_ASSERT_EQ(EntityType::PLANT_BERRY_BUSH, restored.getEntityType());
}

void testPlantHealth() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    
    G::Plant plant(10, 10, registry);
    plant.takeDamage(0.3f);  // 30% damage
    
    json j = plant.toJson();
    G::Plant restored = G::Plant::fromJson(j, registry);
    
    TEST_ASSERT_NEAR(plant.getHealth(), restored.getHealth(), 0.01f);
}

// ============================================================================
// FileHandling Integration Tests
// ============================================================================

void testSaveLoadEmptyWorld() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    std::vector<Creature> creatures;
    Calendar calendar;
    unsigned tick = 0;
    
    // Create minimal world
    World world = createTestWorld(50, 50);
    
    // Save
    bool saveResult = fh.saveGameJson("test_empty.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(saveResult);
    
    // Load
    std::vector<Creature> loadedCreatures;
    Calendar loadedCalendar;
    unsigned loadedTick = 0;
    
    bool loadResult = fh.loadGameJson("test_empty.json", loadedCreatures, world, loadedCalendar, loadedTick, 50, 50);
    TEST_ASSERT(loadResult);
    
    TEST_ASSERT_EQ(0u, loadedCreatures.size());
    TEST_ASSERT_EQ(tick, loadedTick);
    
    cleanupTestDir();
}

void testSaveLoadWithCreatures() {
    setupTestDir();
    Creature::initializeGeneRegistry();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    std::vector<Creature> creatures;
    
    // Create test creatures
    for (int i = 0; i < 5; i++) {
        auto c = createTestCreature(10 + i * 5, 10 + i * 3, 0.5f, 0.5f);
        creatures.push_back(std::move(*c));
    }
    
    Calendar calendar;
    calendar.incrementByHour();  // Advance 1 hour
    unsigned tick = 1000;
    
    World world = createTestWorld(50, 50);
    
    // Save
    bool saveResult = fh.saveGameJson("test_creatures.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(saveResult);
    
    // Load
    std::vector<Creature> loadedCreatures;
    Calendar loadedCalendar;
    unsigned loadedTick = 0;
    
    bool loadResult = fh.loadGameJson("test_creatures.json", loadedCreatures, world, loadedCalendar, loadedTick, 50, 50);
    TEST_ASSERT(loadResult);
    
    TEST_ASSERT_EQ(5u, loadedCreatures.size());
    TEST_ASSERT_EQ(tick, loadedTick);
    
    cleanupTestDir();
}

void testCalendarStatePreserved() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    std::vector<Creature> creatures;
    
    // Set specific calendar state
    Time time;
    time.minute = 30;
    time.hour = 14;
    Date date;
    date.day = 15;
    date.month = 6;
    date.year = 5;
    Calendar calendar(time, date);
    
    unsigned tick = 50000;
    World world = createTestWorld(50, 50);
    
    bool saveResult = fh.saveGameJson("test_calendar.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(saveResult);
    
    std::vector<Creature> loadedCreatures;
    Calendar loadedCalendar;
    unsigned loadedTick = 0;
    
    bool loadResult = fh.loadGameJson("test_calendar.json", loadedCreatures, world, loadedCalendar, loadedTick, 50, 50);
    TEST_ASSERT(loadResult);
    
    TEST_ASSERT_EQ(30, loadedCalendar.getMinute());
    TEST_ASSERT_EQ(14, loadedCalendar.getHour());
    TEST_ASSERT_EQ(15, loadedCalendar.getDay());
    TEST_ASSERT_EQ(6, loadedCalendar.getMonth());
    TEST_ASSERT_EQ(5u, loadedCalendar.getYear());
    
    cleanupTestDir();
}

void testTickCountPreserved() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    std::vector<Creature> creatures;
    Calendar calendar;
    unsigned tick = 123456;
    World world = createTestWorld(50, 50);
    
    bool saveResult = fh.saveGameJson("test_tick.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(saveResult);
    
    std::vector<Creature> loadedCreatures;
    Calendar loadedCalendar;
    unsigned loadedTick = 0;
    
    bool loadResult = fh.loadGameJson("test_tick.json", loadedCreatures, world, loadedCalendar, loadedTick, 50, 50);
    TEST_ASSERT(loadResult);
    
    TEST_ASSERT_EQ(tick, loadedTick);
    
    cleanupTestDir();
}

void testInvalidJsonHandling() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    // Write corrupted JSON
    std::string filepath = TEST_SAVE_DIR + "/saves/test_corrupt.json";
    fs::create_directories(TEST_SAVE_DIR + "/saves");
    std::ofstream file(filepath);
    file << "{ invalid json content";
    file.close();
    
    std::vector<Creature> creatures;
    Calendar calendar;
    unsigned tick = 0;
    World world = createTestWorld(50, 50);
    
    bool loadResult = fh.loadGameJson("test_corrupt.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(!loadResult);
    
    cleanupTestDir();
}

void testFileNotFoundHandling() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    std::vector<Creature> creatures;
    Calendar calendar;
    unsigned tick = 0;
    World world = createTestWorld(50, 50);
    
    bool loadResult = fh.loadGameJson("nonexistent_file.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(!loadResult);
    
    cleanupTestDir();
}

void testVersionMismatchHandling() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    // Write JSON with wrong version
    std::string filepath = TEST_SAVE_DIR + "/saves/test_version.json";
    fs::create_directories(TEST_SAVE_DIR + "/saves");
    std::ofstream file(filepath);
    json j = {
        {"version", 999},  // Wrong version
        {"world", {{"tick", 0}, {"mapWidth", 50}, {"mapHeight", 50}}},
        {"calendar", {{"minute", 0}, {"hour", 0}, {"day", 1}, {"month", 1}, {"year", 1}}},
        {"creatures", json::array()},
        {"plants", json::array()}
    };
    file << j.dump(2);
    file.close();
    
    std::vector<Creature> creatures;
    Calendar calendar;
    unsigned tick = 0;
    World world = createTestWorld(50, 50);
    
    bool loadResult = fh.loadGameJson("test_version.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(!loadResult);
    
    cleanupTestDir();
}

void testMapDimensionMismatch() {
    setupTestDir();
    Creature::initializeGeneRegistry();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    // Create and save with 100x100 map
    std::vector<Creature> creatures;
    auto c = createTestCreature(80, 90, 0.5f, 0.5f);
    creatures.push_back(std::move(*c));
    
    Calendar calendar;
    unsigned tick = 100;
    World world = createTestWorld(100, 100);
    
    bool saveResult = fh.saveGameJson("test_dimensions.json", creatures, world, calendar, tick, 100, 100);
    TEST_ASSERT(saveResult);
    
    // Load with 50x50 map
    std::vector<Creature> loadedCreatures;
    Calendar loadedCalendar;
    unsigned loadedTick = 0;
    World smallWorld = createTestWorld(50, 50);
    
    // Should still load (with warnings), positions clamped
    bool loadResult = fh.loadGameJson("test_dimensions.json", loadedCreatures, smallWorld, loadedCalendar, loadedTick, 50, 50);
    TEST_ASSERT(loadResult);
    
    // Creature should be loaded with clamped position
    TEST_ASSERT_EQ(1u, loadedCreatures.size());
    TEST_ASSERT_LT(loadedCreatures[0].getX(), 50);
    TEST_ASSERT_LT(loadedCreatures[0].getY(), 50);
    
    cleanupTestDir();
}

void testWorldGenerationParametersPreserved() {
    setupTestDir();
    Creature::initializeGeneRegistry();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    // Create world with specific generation parameters
    MapGen customMapGen;
    customMapGen.rows = 50;
    customMapGen.cols = 50;
    customMapGen.seed = 98765.0;  // Specific seed
    customMapGen.scale = 0.015;   // Custom scale
    customMapGen.freq = 1.5;      // Custom frequency
    customMapGen.exponent = 1.2;  // Custom exponent
    customMapGen.terraces = 25;   // Custom terraces
    customMapGen.isIsland = true; // Island mode
    
    OctaveGen customOctaveGen;
    customOctaveGen.quantity = 6;        // Custom octaves
    customOctaveGen.minWeight = 0.15;    // Custom min weight
    customOctaveGen.maxWeight = 0.6;     // Custom max weight
    customOctaveGen.freqInterval = 1.5;  // Custom frequency interval
    
    World world(customMapGen, customOctaveGen);
    
    // Sample some terrain values before save (use terrain type for comparison)
    auto& gridBefore = world.getGrid();
    TerrainType terrain1 = gridBefore[10][10].getTerrainType();
    TerrainType terrain2 = gridBefore[25][25].getTerrainType();
    TerrainType terrain3 = gridBefore[40][40].getTerrainType();
    unsigned elev1 = gridBefore[10][10].getElevation();
    unsigned elev2 = gridBefore[25][25].getElevation();
    unsigned elev3 = gridBefore[40][40].getElevation();
    
    std::vector<Creature> creatures;
    Calendar calendar;
    unsigned tick = 500;
    
    // Save the game
    bool saveResult = fh.saveGameJson("test_world_gen.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(saveResult);
    
    // Create a new world with different parameters (will be overwritten by load)
    MapGen differentMapGen;
    differentMapGen.rows = 50;
    differentMapGen.cols = 50;
    differentMapGen.seed = 11111.0;  // Different seed
    differentMapGen.scale = 0.001;
    
    OctaveGen differentOctaveGen;
    
    World newWorld(differentMapGen, differentOctaveGen);
    
    // Verify new world has different seed
    TEST_ASSERT(newWorld.getSeed() != customMapGen.seed);
    
    // Load the saved game
    std::vector<Creature> loadedCreatures;
    Calendar loadedCalendar;
    unsigned loadedTick = 0;
    
    bool loadResult = fh.loadGameJson("test_world_gen.json", loadedCreatures, newWorld, loadedCalendar, loadedTick, 50, 50);
    TEST_ASSERT(loadResult);
    
    // Verify MapGen parameters were restored
    MapGen loadedMapGen = newWorld.getMapGen();
    TEST_ASSERT_NEAR(customMapGen.seed, loadedMapGen.seed, 0.001);
    TEST_ASSERT_NEAR(customMapGen.scale, loadedMapGen.scale, 0.0001);
    TEST_ASSERT_NEAR(customMapGen.freq, loadedMapGen.freq, 0.0001);
    TEST_ASSERT_NEAR(customMapGen.exponent, loadedMapGen.exponent, 0.0001);
    TEST_ASSERT_EQ(customMapGen.terraces, loadedMapGen.terraces);
    TEST_ASSERT_EQ(customMapGen.isIsland, loadedMapGen.isIsland);
    
    // Verify OctaveGen parameters were restored
    OctaveGen loadedOctaveGen = newWorld.getOctaveGen();
    TEST_ASSERT_EQ(customOctaveGen.quantity, loadedOctaveGen.quantity);
    TEST_ASSERT_NEAR(customOctaveGen.minWeight, loadedOctaveGen.minWeight, 0.0001);
    TEST_ASSERT_NEAR(customOctaveGen.maxWeight, loadedOctaveGen.maxWeight, 0.0001);
    TEST_ASSERT_NEAR(customOctaveGen.freqInterval, loadedOctaveGen.freqInterval, 0.0001);
    
    // Verify terrain was regenerated identically (same seed = same terrain)
    auto& gridAfter = newWorld.getGrid();
    TEST_ASSERT_EQ(static_cast<int>(terrain1), static_cast<int>(gridAfter[10][10].getTerrainType()));
    TEST_ASSERT_EQ(static_cast<int>(terrain2), static_cast<int>(gridAfter[25][25].getTerrainType()));
    TEST_ASSERT_EQ(static_cast<int>(terrain3), static_cast<int>(gridAfter[40][40].getTerrainType()));
    TEST_ASSERT_EQ(elev1, gridAfter[10][10].getElevation());
    TEST_ASSERT_EQ(elev2, gridAfter[25][25].getElevation());
    TEST_ASSERT_EQ(elev3, gridAfter[40][40].getElevation());
    
    cleanupTestDir();
}

// ============================================================================
// SaveMetadata Tests
// ============================================================================

void testGetSaveMetadataValid() {
    setupTestDir();
    Creature::initializeGeneRegistry();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    // Create save with specific counts
    std::vector<Creature> creatures;
    for (int i = 0; i < 10; i++) {
        auto c = createTestCreature(10 + i, 10 + i, 0.5f, 0.5f);
        creatures.push_back(std::move(*c));
    }
    
    Calendar calendar;
    unsigned tick = 5000;
    World world = createTestWorld(50, 50);
    
    bool saveResult = fh.saveGameJson("test_metadata.json", creatures, world, calendar, tick, 50, 50);
    TEST_ASSERT(saveResult);
    
    // Get metadata
    auto metadata = fh.getSaveMetadata("test_metadata.json");
    TEST_ASSERT(metadata.has_value());
    
    TEST_ASSERT_EQ(FileHandling::SAVE_VERSION, metadata->version);
    TEST_ASSERT_EQ(10, metadata->creatureCount);
    TEST_ASSERT_EQ(tick, metadata->tick);
    TEST_ASSERT(!metadata->savedAt.empty());
    
    cleanupTestDir();
}

void testGetSaveMetadataInvalidFile() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    // Try to get metadata from non-existent file
    auto metadata = fh.getSaveMetadata("nonexistent.json");
    TEST_ASSERT(!metadata.has_value());
    
    cleanupTestDir();
}

void testGetSaveMetadataCorruptFile() {
    setupTestDir();
    
    FileHandling fh(TEST_SAVE_DIR);
    
    // Write corrupted JSON
    std::string filepath = TEST_SAVE_DIR + "/saves/test_corrupt_meta.json";
    fs::create_directories(TEST_SAVE_DIR + "/saves");
    std::ofstream file(filepath);
    file << "not valid json at all {{{";
    file.close();
    
    auto metadata = fh.getSaveMetadata("test_corrupt_meta.json");
    TEST_ASSERT(!metadata.has_value());
    
    cleanupTestDir();
}

// ============================================================================
// Test Runners
// ============================================================================

void runGeneSerializationTests() {
    BEGIN_TEST_GROUP("Gene JSON Serialization");
    RUN_TEST(testGeneRoundtrip);
    RUN_TEST(testGeneHomozygousRoundtrip);
    RUN_TEST(testGeneEdgeValues);
    RUN_TEST(testGeneFromJsonMissingId);
    RUN_TEST(testGeneFromJsonMissingAllele);
    RUN_TEST(testGeneFromJsonMissingValue);
    END_TEST_GROUP();
}

void runChromosomeSerializationTests() {
    BEGIN_TEST_GROUP("Chromosome JSON Serialization");
    RUN_TEST(testChromosomeRoundtrip);
    RUN_TEST(testEmptyChromosome);
    RUN_TEST(testAllChromosomeTypes);
    RUN_TEST(testChromosomeFromJsonMissingType);
    RUN_TEST(testChromosomeFromJsonMissingGenes);
    RUN_TEST(testChromosomeFromJsonInvalidType);
    END_TEST_GROUP();
}

void runGenomeSerializationTests() {
    BEGIN_TEST_GROUP("Genome JSON Serialization");
    RUN_TEST(testGenomeRoundtrip);
    RUN_TEST(testGenomeWithCreatureGenes);
    RUN_TEST(testGenomeWithPlantGenes);
    RUN_TEST(testGenomeLoadFromJson);
    RUN_TEST(testGenomeFromJsonMissingChromosomes);
    END_TEST_GROUP();
}

void runCreatureSerializationTests() {
    BEGIN_TEST_GROUP("Creature JSON Serialization");
    RUN_TEST(testCreatureRoundtrip);
    RUN_TEST(testCreaturePositionBounds);
    RUN_TEST(testCreatureWithCombatState);
    RUN_TEST(testCreatureWithDamage);
    RUN_TEST(testCreaturePhenotypeRegenerated);
    RUN_TEST(testCreatureWorldPosition);
    END_TEST_GROUP();
}

void runPlantSerializationTests() {
    BEGIN_TEST_GROUP("Plant JSON Serialization");
    RUN_TEST(testPlantRoundtrip);
    RUN_TEST(testPlantLifeStages);
    RUN_TEST(testPlantWithSeeds);
    RUN_TEST(testPlantDispersalStrategies);
    RUN_TEST(testPlantEntityTypePreserved);
    RUN_TEST(testPlantHealth);
    END_TEST_GROUP();
}

void runFileHandlingTests() {
    BEGIN_TEST_GROUP("FileHandling JSON Save/Load");
    RUN_TEST(testSaveLoadEmptyWorld);
    RUN_TEST(testSaveLoadWithCreatures);
    RUN_TEST(testCalendarStatePreserved);
    RUN_TEST(testTickCountPreserved);
    RUN_TEST(testWorldGenerationParametersPreserved);
    RUN_TEST(testInvalidJsonHandling);
    RUN_TEST(testFileNotFoundHandling);
    RUN_TEST(testVersionMismatchHandling);
    RUN_TEST(testMapDimensionMismatch);
    END_TEST_GROUP();
}

void runSaveMetadataTests() {
    BEGIN_TEST_GROUP("Save Metadata");
    RUN_TEST(testGetSaveMetadataValid);
    RUN_TEST(testGetSaveMetadataInvalidFile);
    RUN_TEST(testGetSaveMetadataCorruptFile);
    END_TEST_GROUP();
}

void runSerializationTests() {
    runGeneSerializationTests();
    runChromosomeSerializationTests();
    runGenomeSerializationTests();
    runCreatureSerializationTests();
    runPlantSerializationTests();
    runFileHandlingTests();
    runSaveMetadataTests();
}

#ifdef TEST_SERIALIZATION_STANDALONE
int main() {
    std::cout << "=== Serialization System Tests ===" << std::endl;
    runSerializationTests();
    EcoSim::Testing::TestSuite::instance().printSummary();
    return EcoSim::Testing::TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
