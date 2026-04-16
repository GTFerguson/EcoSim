/**
 * @file test_organism_factory.cpp
 * @brief Tests for gene-driven component attachment in OrganismFactory.
 *
 * The factory is the mechanism by which creature-ness and plant-ness
 * become emergent properties of a genome's expressed capabilities. These
 * tests pin the gene → component mapping so a threshold change (or an
 * accidental regression) is caught immediately.
 *
 * Covered shapes:
 *  - Creature-like genome  → Mobility + Heterotrophy + Combat + Thermal
 *  - Plant-like genome     → Autotrophy (no Mobility, no Heterotrophy)
 *  - Mobile autotroph      → Mobility + Autotrophy
 *  - Sessile heterotroph   → Heterotrophy (no Mobility) — carnivorous plant
 *  - Phototroph hybrid     → Mobility + Heterotrophy + Autotrophy
 *  - Threshold edge cases  — trait just above/below the gate
 *  - Archetype selection   — creature vs plant catalogue based on signature
 *  - Species name          — set for heterotrophs, empty for pure autotrophs
 */

#include "test_framework.hpp"

#include "genetics/organisms/OrganismFactory.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"
#include "genetics/components/MobilityComponent.hpp"
#include "genetics/components/HeterotrophyComponent.hpp"
#include "genetics/components/AutotrophyComponent.hpp"
#include "genetics/components/CombatComponent.hpp"
#include "genetics/components/ThermalComponent.hpp"
#include "genetics/components/ReproductionComponent.hpp"
#include "genetics/components/IdentityComponent.hpp"

namespace G = EcoSim::Genetics;

// ============================================================================
// Helpers
// ============================================================================

namespace {

G::GeneRegistry buildFullRegistry() {
    G::GeneRegistry registry;
    G::UniversalGenes::registerDefaults(registry);
    G::PlantGenes::registerDefaults(registry);
    return registry;
}

// Force a gene's value in an existing genome. The trait must already
// exist in the genome (ensure via createCreatureGenome / createPlantGenome).
void setGeneValue(G::Genome& genome, const std::string& id, float value) {
    if (genome.hasGene(id)) {
        genome.getGeneMutable(id).setAlleleValues(value);
    }
}

} // namespace

// ============================================================================
// Canonical creature / plant genome tests
// ============================================================================

void testFactoryCreatureGenomeAttachesMobilityAndHeterotrophy() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(sig.mobility);
    TEST_ASSERT(sig.heterotrophy);
    TEST_ASSERT(!sig.autotrophy);
    TEST_ASSERT(sig.reproduction);
    TEST_ASSERT(sig.identity);
}

void testFactoryPlantGenomeAttachesAutotrophyNotMobility() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createPlantGenome(registry);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(sig.autotrophy);
    TEST_ASSERT(!sig.mobility);
    TEST_ASSERT(!sig.heterotrophy);
    TEST_ASSERT(sig.reproduction);
    TEST_ASSERT(sig.identity);
}

// ============================================================================
// Hybrid genome tests — the whole reason gene-driven attachment exists
// ============================================================================

void testFactoryMobileAutotrophAttachesBoth() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createPlantGenome(registry);

    // Plant genome with locomotion evolved above threshold → mobile plant
    // (algae-like). Should carry both Mobility and Autotrophy.
    setGeneValue(genome, G::UniversalGenes::LOCOMOTION, 0.8f);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(sig.mobility);
    TEST_ASSERT(sig.autotrophy);
    TEST_ASSERT(!sig.heterotrophy);
}

void testFactorySessileHeterotrophAttachesHeterotrophyNotMobility() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    // Creature genome with locomotion crushed below threshold →
    // sessile heterotroph (carnivorous plant, barnacle). Keeps
    // heterotrophy but loses mobility.
    setGeneValue(genome, G::UniversalGenes::LOCOMOTION, 0.0f);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(!sig.mobility);
    TEST_ASSERT(sig.heterotrophy);
}

void testFactoryPhototrophAttachesAllThree() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    // Creature genome with photosynthesis evolved → phototroph.
    // Mobile, heterotrophic AND autotrophic (mixed-strategy).
    setGeneValue(genome, G::UniversalGenes::PHOTOSYNTHESIS, 0.9f);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(sig.mobility);
    TEST_ASSERT(sig.heterotrophy);
    TEST_ASSERT(sig.autotrophy);
}

// ============================================================================
// Threshold edge cases
// ============================================================================

void testFactoryLocomotionBelowThresholdDoesNotAttachMobility() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    // Just below the gate (0.1) — should NOT attach Mobility.
    setGeneValue(genome, G::UniversalGenes::LOCOMOTION, 0.05f);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(!sig.mobility);
}

void testFactoryLocomotionAtThresholdAttachesMobility() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    // Exactly at threshold (0.1) — should attach (>= comparison).
    setGeneValue(genome, G::UniversalGenes::LOCOMOTION, 0.1f);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(sig.mobility);
}

void testFactoryDigestionGatesHeterotrophy() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    // Kill both digestion genes → no heterotrophy.
    setGeneValue(genome, G::UniversalGenes::MEAT_DIGESTION_EFFICIENCY, 0.0f);
    setGeneValue(genome, G::UniversalGenes::PLANT_DIGESTION_EFFICIENCY, 0.0f);

    auto sig = G::OrganismFactory::classifyComponentSet(genome);

    TEST_ASSERT(!sig.heterotrophy);
}

// ============================================================================
// fromGenome — actually constructs an organism
// ============================================================================

void testFactoryFromGenomeCreatureHasCreatureComponents() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    auto organism = G::OrganismFactory::fromGenome(std::move(genome), 10, 20, registry);

    TEST_ASSERT(organism != nullptr);
    TEST_ASSERT(organism->mobility()     != nullptr);
    TEST_ASSERT(organism->heterotrophy() != nullptr);
    TEST_ASSERT(organism->autotrophy()   == nullptr);
    TEST_ASSERT(organism->reproduction() != nullptr);
    TEST_ASSERT(organism->identity()     != nullptr);
    TEST_ASSERT_EQ(10, organism->getX());
    TEST_ASSERT_EQ(20, organism->getY());
}

void testFactoryFromGenomePlantHasPlantComponents() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createPlantGenome(registry);

    auto organism = G::OrganismFactory::fromGenome(std::move(genome), 5, 5, registry);

    TEST_ASSERT(organism != nullptr);
    TEST_ASSERT(organism->autotrophy()   != nullptr);
    TEST_ASSERT(organism->mobility()     == nullptr);
    TEST_ASSERT(organism->heterotrophy() == nullptr);
    TEST_ASSERT(organism->reproduction() != nullptr);
    TEST_ASSERT(organism->identity()     != nullptr);
}

// ============================================================================
// Archetype selection
// ============================================================================

void testFactoryCreatureGenomeGetsCreatureArchetype() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    auto organism = G::OrganismFactory::fromGenome(std::move(genome), 0, 0, registry);

    TEST_ASSERT(organism->identity()->archetype != nullptr);
    // Creature archetypes have char != '.' (GenericPlant) and entity
    // type in the CREATURE family.
    TEST_ASSERT_EQ(EntityType::CREATURE,
                   organism->identity()->archetype->getEntityType());
}

void testFactoryPlantGenomeGetsPlantArchetype() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createPlantGenome(registry);

    auto organism = G::OrganismFactory::fromGenome(std::move(genome), 0, 0, registry);

    TEST_ASSERT(organism->identity()->archetype != nullptr);
    // Plant archetypes have a PLANT_* entity type.
    auto et = organism->identity()->archetype->getEntityType();
    const bool isPlantType =
        et == EntityType::PLANT_BERRY_BUSH
     || et == EntityType::PLANT_OAK_TREE
     || et == EntityType::PLANT_GRASS
     || et == EntityType::PLANT_THORN_BUSH
     || et == EntityType::PLANT_GENERIC;
    TEST_ASSERT(isPlantType);
}

// ============================================================================
// Species name — only heterotrophs get Latin name, plants use archetype label
// ============================================================================

void testFactoryHeterotrophHasSpeciesName() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createCreatureGenome(registry);

    auto organism = G::OrganismFactory::fromGenome(std::move(genome), 0, 0, registry);

    TEST_ASSERT(!organism->identity()->speciesName.empty());
}

void testFactoryPureAutotrophHasNoSpeciesName() {
    auto registry = buildFullRegistry();
    auto genome = G::UniversalGenes::createPlantGenome(registry);

    auto organism = G::OrganismFactory::fromGenome(std::move(genome), 0, 0, registry);

    // speciesName is empty → getName() falls through to archetype label.
    TEST_ASSERT(organism->identity()->speciesName.empty());
    TEST_ASSERT(!organism->getName().empty());
}

// ============================================================================
// Test runner
// ============================================================================

void runOrganismFactoryTests() {
    BEGIN_TEST_GROUP("OrganismFactory: canonical signatures");
    RUN_TEST(testFactoryCreatureGenomeAttachesMobilityAndHeterotrophy);
    RUN_TEST(testFactoryPlantGenomeAttachesAutotrophyNotMobility);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("OrganismFactory: hybrid signatures");
    RUN_TEST(testFactoryMobileAutotrophAttachesBoth);
    RUN_TEST(testFactorySessileHeterotrophAttachesHeterotrophyNotMobility);
    RUN_TEST(testFactoryPhototrophAttachesAllThree);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("OrganismFactory: threshold edges");
    RUN_TEST(testFactoryLocomotionBelowThresholdDoesNotAttachMobility);
    RUN_TEST(testFactoryLocomotionAtThresholdAttachesMobility);
    RUN_TEST(testFactoryDigestionGatesHeterotrophy);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("OrganismFactory: fromGenome build");
    RUN_TEST(testFactoryFromGenomeCreatureHasCreatureComponents);
    RUN_TEST(testFactoryFromGenomePlantHasPlantComponents);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("OrganismFactory: archetype selection");
    RUN_TEST(testFactoryCreatureGenomeGetsCreatureArchetype);
    RUN_TEST(testFactoryPlantGenomeGetsPlantArchetype);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("OrganismFactory: species name");
    RUN_TEST(testFactoryHeterotrophHasSpeciesName);
    RUN_TEST(testFactoryPureAutotrophHasNoSpeciesName);
    END_TEST_GROUP();
}
