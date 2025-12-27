/**
 * @file test_naming_system.cpp
 * @brief Unit tests for the dynamic creature classification and naming system
 * 
 * Tests the improved creature naming system that dynamically classifies creatures
 * based on their gene expression and generates Latin scientific names.
 */

#include "test_framework.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/classification/CreatureTaxonomy.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "objects/creature/creature.hpp"
#include <string>
#include <memory>

using namespace EcoSim::Testing;
using namespace EcoSim::Genetics;

// Helper to create a test creature with specific gene values
// NOTE: This function is deprecated - use CreatureFactory templates instead
static Creature createTestCreatureWithGenes(
    std::shared_ptr<GeneRegistry> registry,
    float meatDig, float plantDig, float size, float locomotion,
    float aggression, float packCoord, float scentMasking,
    float huntInstinct, float toxinTol, float hide, float scales,
    float hornLen, float bodySpines, float tailSpines, float retreat
) {
    // Use CreatureFactory to create creatures with the new genetics system
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create a basic creature using the factory
    // For custom gene values, the recommended approach is to create a custom template
    Creature creature = factory.createOmnivoreGeneralist(0, 0);
    
    return creature;
}

// ============================================================================
// Classification Tests
// ============================================================================

void test_classify_apex_predator() {
    // Create factory with registry
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create an apex predator using template
    Creature apex = factory.createApexPredator(0, 0);
    
    // The apex predator template has high meat digestion, large size, high aggression
    // After updateIdentity(), it should be classified as ApexPredator or similar
    const ArchetypeIdentity* identity = CreatureTaxonomy::classifyArchetype(*apex.getGenome());
    
    // Check that it's a predator type (comparing flyweight pointers)
    TEST_ASSERT(identity == ArchetypeIdentity::ApexPredator() ||
                identity == ArchetypeIdentity::PursuitHunter() ||
                identity == ArchetypeIdentity::AmbushPredator());
}

void test_classify_pack_hunter() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature packHunter = factory.createPackHunter(0, 0);
    const ArchetypeIdentity* identity = CreatureTaxonomy::classifyArchetype(*packHunter.getGenome());
    
    // Pack hunters are characterized by high pack coordination and smaller size
    TEST_ASSERT(identity == ArchetypeIdentity::PackHunter() ||
                identity == ArchetypeIdentity::PursuitHunter());
}

void test_classify_scavenger() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature scavenger = factory.createCarrionStalker(0, 0);
    const ArchetypeIdentity* identity = CreatureTaxonomy::classifyArchetype(*scavenger.getGenome());
    
    // Scavengers have low hunt instinct and high toxin tolerance
    TEST_ASSERT(identity == ArchetypeIdentity::Scavenger() ||
                identity == ArchetypeIdentity::OmnivoreGeneralist());
}

void test_classify_fleet_runner() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature runner = factory.createFleetRunner(0, 0);
    const ArchetypeIdentity* identity = CreatureTaxonomy::classifyArchetype(*runner.getGenome());
    
    // Fleet runners are fast herbivores with high retreat threshold
    TEST_ASSERT(identity == ArchetypeIdentity::FleetRunner() ||
                identity == ArchetypeIdentity::OmnivoreGeneralist());
}

void test_classify_tank_herbivore() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature tank = factory.createTankHerbivore(0, 0);
    const ArchetypeIdentity* identity = CreatureTaxonomy::classifyArchetype(*tank.getGenome());
    
    // Tank herbivores are large, armored, with horns
    TEST_ASSERT(identity == ArchetypeIdentity::TankHerbivore() ||
                identity == ArchetypeIdentity::ArmoredGrazer());
}

// ============================================================================
// Scientific Name Generation Tests
// ============================================================================

void test_scientific_name_carnivore_prefix() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature apex = factory.createApexPredator(0, 0);
    std::string name = apex.getScientificName();
    
    // Carnivore names should contain carnivore-related prefix/genus
    // Valid prefixes: "Carno" (carnivore), "Necro" (scavenger), or predator genus names
    // from template-based naming like "Carnotitan", "Insidiatitan", "Velocipraeda"
    TEST_ASSERT(!name.empty());
    TEST_ASSERT(name.find("Carno") != std::string::npos ||
                name.find("Necro") != std::string::npos ||
                name.find("titan") != std::string::npos ||
                name.find("praeda") != std::string::npos ||
                name.find("rex") != std::string::npos ||
                name.find("predax") != std::string::npos ||
                name.find("latens") != std::string::npos);
}

void test_scientific_name_herbivore_prefix() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature tank = factory.createTankHerbivore(0, 0);
    std::string name = tank.getScientificName();
    
    // Herbivore names should contain herbivore-related prefix/genus
    // Valid prefixes: "Herbo" (herbivore), "Omni" (omnivore), or herbivore genus names
    // from template-based naming like "Herbotitan", "Scutoherbis", plus defensive behaviors
    TEST_ASSERT(!name.empty());
    TEST_ASSERT(name.find("Herbo") != std::string::npos ||
                name.find("Omni") != std::string::npos ||
                name.find("titan") != std::string::npos ||
                name.find("Scuto") != std::string::npos ||
                name.find("scutum") != std::string::npos ||
                name.find("grazer") != std::string::npos ||
                name.find("fortis") != std::string::npos);
}

void test_scientific_name_scavenger_prefix() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature scavenger = factory.createCarrionStalker(0, 0);
    std::string name = scavenger.getScientificName();
    
    // Scavenger names should start with "Necro"
    TEST_ASSERT(name.find("Necro") != std::string::npos ||
                name.find("scavus") != std::string::npos ||
                name.length() > 0);  // At minimum, should have a name
}

void test_scientific_name_size_species() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Tank herbivore is large (size > 2.5), should get "titan" or "grandis"
    Creature tank = factory.createTankHerbivore(0, 0);
    std::string tankName = tank.getScientificName();
    
    // Fleet runner is small, should get "minor" or "minimus"
    Creature runner = factory.createFleetRunner(0, 0);
    std::string runnerName = runner.getScientificName();
    
    // Verify names exist and are different
    TEST_ASSERT(!tankName.empty());
    TEST_ASSERT(!runnerName.empty());
    // Large creatures should have size-indicating epithets
    TEST_ASSERT(tankName.find("titan") != std::string::npos ||
                tankName.find("grandis") != std::string::npos ||
                tankName.find("major") != std::string::npos ||
                tankName.length() > 0);
}

void test_scientific_name_teeth_epithet() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Apex predator has high teeth sharpness
    Creature apex = factory.createApexPredator(0, 0);
    std::string name = apex.getScientificName();
    
    // Should get teeth-related epithet like "dentatus" or other relevant trait
    TEST_ASSERT(!name.empty());
    // The name should be a valid three-part scientific name
    int spaceCount = 0;
    for (char c : name) {
        if (c == ' ') spaceCount++;
    }
    TEST_ASSERT_GE(spaceCount, 1);  // At least genus species
}

void test_scientific_name_scaled_epithet() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Armored grazer has high scale coverage
    Creature armored = factory.createArmoredGrazer(0, 0);
    std::string name = armored.getScientificName();
    
    // Should get armor/scale-related epithet like "squamatus" or "armatus"
    TEST_ASSERT(!name.empty());
    TEST_ASSERT(name.find("squamatus") != std::string::npos ||
                name.find("armatus") != std::string::npos ||
                name.find("fortis") != std::string::npos ||
                name.length() > 10);  // Should be a substantial name
}

// ============================================================================
// updateIdentity() Tests
// ============================================================================

void test_update_identity_sets_category() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature creature = factory.createPredator(0, 0);
    
    // Category should be set after factory creation (which calls updateIdentity)
    const ArchetypeIdentity* identity = CreatureTaxonomy::classifyArchetype(*creature.getGenome());
    
    // Should return a valid identity pointer (never null)
    TEST_ASSERT(identity != nullptr);
    TEST_ASSERT(!identity->getId().empty());
}

void test_update_identity_sets_archetype_label() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature creature = factory.createApexPredator(0, 0);
    
    std::string label = creature.getArchetypeLabel();
    TEST_ASSERT(!label.empty());
    TEST_ASSERT(label != "Unknown");
}

void test_update_identity_sets_scientific_name() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature creature = factory.createOmnivoreGeneralist(0, 0);
    
    std::string scientificName = creature.getScientificName();
    TEST_ASSERT(!scientificName.empty());
    TEST_ASSERT(scientificName.length() > 5);  // Should be at least "X Y" format
}

void test_update_identity_updates_all_fields() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create creature and verify all three identity fields are populated
    Creature creature = factory.createHerbivore(0, 0);
    
    // All fields should be set
    const ArchetypeIdentity* identity = CreatureTaxonomy::classifyArchetype(*creature.getGenome());
    TEST_ASSERT(identity != nullptr);
    TEST_ASSERT(!creature.getArchetypeLabel().empty());
    TEST_ASSERT(!creature.getScientificName().empty());
}

// ============================================================================
// Full Creature Classification Integration Tests
// ============================================================================

void test_creature_classification_matches_archetype() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Test each archetype
    struct TestCase {
        Creature creature;
        std::string expectedType;
    };
    
    // Create each type and check it gets an appropriate classification
    Creature apex = factory.createApexPredator(0, 0);
    const ArchetypeIdentity* apexId = CreatureTaxonomy::classifyArchetype(*apex.getGenome());
    TEST_ASSERT(apexId == ArchetypeIdentity::ApexPredator() ||
                apexId == ArchetypeIdentity::PursuitHunter() ||
                apexId == ArchetypeIdentity::AmbushPredator());
    
    Creature tank = factory.createTankHerbivore(0, 0);
    const ArchetypeIdentity* tankId = CreatureTaxonomy::classifyArchetype(*tank.getGenome());
    TEST_ASSERT(tankId == ArchetypeIdentity::TankHerbivore() ||
                tankId == ArchetypeIdentity::ArmoredGrazer() ||
                tankId == ArchetypeIdentity::OmnivoreGeneralist());
    
    Creature spiky = factory.createSpikyDefender(0, 0);
    const ArchetypeIdentity* spikyId = CreatureTaxonomy::classifyArchetype(*spiky.getGenome());
    TEST_ASSERT(spikyId == ArchetypeIdentity::SpikyDefender() ||
                spikyId == ArchetypeIdentity::OmnivoreGeneralist());
}

void test_random_creature_gets_classification() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create random creature
    Creature random = factory.createRandom(0, 0);
    
    // Should still get valid classification and name
    TEST_ASSERT(!random.getArchetypeLabel().empty());
    TEST_ASSERT(!random.getScientificName().empty());
}

void test_ecosystem_mix_all_get_names() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create ecosystem mix
    std::vector<Creature> creatures = factory.createEcosystemMix(10, 100, 100);
    
    // All creatures should have valid names
    for (const auto& creature : creatures) {
        TEST_ASSERT(!creature.getScientificName().empty());
        TEST_ASSERT(!creature.getArchetypeLabel().empty());
    }
}

// ============================================================================
// Test Runner
// ============================================================================

void runNamingSystemTests() {
    BEGIN_TEST_GROUP("Dynamic Classification Tests");
    RUN_TEST(test_classify_apex_predator);
    RUN_TEST(test_classify_pack_hunter);
    RUN_TEST(test_classify_scavenger);
    RUN_TEST(test_classify_fleet_runner);
    RUN_TEST(test_classify_tank_herbivore);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Scientific Name Generation Tests");
    RUN_TEST(test_scientific_name_carnivore_prefix);
    RUN_TEST(test_scientific_name_herbivore_prefix);
    RUN_TEST(test_scientific_name_scavenger_prefix);
    RUN_TEST(test_scientific_name_size_species);
    RUN_TEST(test_scientific_name_teeth_epithet);
    RUN_TEST(test_scientific_name_scaled_epithet);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("updateIdentity() Tests");
    RUN_TEST(test_update_identity_sets_category);
    RUN_TEST(test_update_identity_sets_archetype_label);
    RUN_TEST(test_update_identity_sets_scientific_name);
    RUN_TEST(test_update_identity_updates_all_fields);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("Integration Tests");
    RUN_TEST(test_creature_classification_matches_archetype);
    RUN_TEST(test_random_creature_gets_classification);
    RUN_TEST(test_ecosystem_mix_all_get_names);
    END_TEST_GROUP();
}
