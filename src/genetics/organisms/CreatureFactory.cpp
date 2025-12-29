/**
 * @file CreatureFactory.cpp
 * @brief Implementation of the CreatureFactory class
 *
 * Implements 11 combat-balanced archetype templates:
 * - Predators: Apex Predator, Pack Hunter, Ambush Predator, Pursuit Hunter
 * - Herbivores: Tank Herbivore, Armored Grazer, Fleet Runner, Spiky Defender, Canopy Forager
 * - Opportunists: Carrion Stalker, Omnivore Generalist
 */

#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/RandomEngine.hpp"
#include <algorithm>
#include <stdexcept>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructor
// ============================================================================

CreatureFactory::CreatureFactory(std::shared_ptr<GeneRegistry> registry)
    : registry_(std::move(registry))
{
    if (!registry_) {
        throw std::invalid_argument("CreatureFactory requires a valid GeneRegistry");
    }
    
    // Ensure UniversalGenes are registered
    UniversalGenes::registerDefaults(*registry_);
}

// ============================================================================
// Creature Creation - Generic
// ============================================================================

std::unique_ptr<Genome> CreatureFactory::createGenomeFromTemplate(const CreatureTemplate& tmpl) const {
    // Create a base creature genome with default values
    auto genome = std::make_unique<Genome>(UniversalGenes::createCreatureGenome(*registry_));
    
    // Apply template gene ranges to the genome
    applyTemplate(*genome, tmpl);
    
    return genome;
}

Creature CreatureFactory::createFromTemplate(const std::string& templateName, int x, int y) const {
    const CreatureTemplate* tmpl = getTemplate(templateName);
    
    if (!tmpl) {
        // Template not found - create with default genome and dynamic classification
        auto genome = std::make_unique<Genome>(UniversalGenes::createCreatureGenome(*registry_));
        // Use the new simplified constructor - phenotype and identity set in constructor
        return Creature(x, y, std::move(genome));
    }
    
    // Create genome directly from template using new simplified approach
    // This replaces the complex 5-step legacy process:
    //   OLD: createLegacyGenome → Creature(legacy) → enableNewGenetics → applyTemplate → rebuildPhenotype → updateIdentity
    //   NEW: createGenomeFromTemplate → Creature(genome) - done! (phenotype + identity set in constructor)
    auto genome = createGenomeFromTemplate(*tmpl);
    
    // Use the new simplified constructor
    // Constructor handles: phenotype creation, health initialization, identity update
    return Creature(x, y, std::move(genome));
}

Creature CreatureFactory::createRandom(int x, int y) const {
    // Create random genome using the utility method
    auto genome = std::make_unique<Genome>(UniversalGenes::createRandomGenome(*registry_));
    
    // Use the new simplified constructor
    // Dynamic classification based on randomly generated genes happens in constructor
    return Creature(x, y, std::move(genome));
}

// ============================================================================
// Creature Creation - Specific Archetypes
// ============================================================================

Creature CreatureFactory::createApexPredator(int x, int y) const {
    return createFromTemplate("apex_predator", x, y);
}

Creature CreatureFactory::createPackHunter(int x, int y) const {
    return createFromTemplate("pack_hunter", x, y);
}

Creature CreatureFactory::createAmbushPredator(int x, int y) const {
    return createFromTemplate("ambush_predator", x, y);
}

Creature CreatureFactory::createPursuitHunter(int x, int y) const {
    return createFromTemplate("pursuit_hunter", x, y);
}

Creature CreatureFactory::createTankHerbivore(int x, int y) const {
    return createFromTemplate("tank_herbivore", x, y);
}

Creature CreatureFactory::createArmoredGrazer(int x, int y) const {
    return createFromTemplate("armored_grazer", x, y);
}

Creature CreatureFactory::createFleetRunner(int x, int y) const {
    return createFromTemplate("fleet_runner", x, y);
}

Creature CreatureFactory::createSpikyDefender(int x, int y) const {
    return createFromTemplate("spiky_defender", x, y);
}

Creature CreatureFactory::createCanopyForager(int x, int y) const {
    return createFromTemplate("canopy_forager", x, y);
}

Creature CreatureFactory::createCarrionStalker(int x, int y) const {
    return createFromTemplate("carrion_stalker", x, y);
}

Creature CreatureFactory::createOmnivoreGeneralist(int x, int y) const {
    return createFromTemplate("omnivore_generalist", x, y);
}

// ============================================================================
// Creature Creation - Category-based
// ============================================================================

Creature CreatureFactory::createPredator(int x, int y) const {
    // Randomly select from predator archetypes
    int choice = randomIntInRange(0, 3);
    switch (choice) {
        case 0: return createApexPredator(x, y);
        case 1: return createPackHunter(x, y);
        case 2: return createAmbushPredator(x, y);
        case 3: return createPursuitHunter(x, y);
        default: return createApexPredator(x, y);
    }
}

Creature CreatureFactory::createHerbivore(int x, int y) const {
    // Randomly select from herbivore archetypes (including canopy forager)
    int choice = randomIntInRange(0, 4);
    switch (choice) {
        case 0: return createTankHerbivore(x, y);
        case 1: return createArmoredGrazer(x, y);
        case 2: return createFleetRunner(x, y);
        case 3: return createSpikyDefender(x, y);
        case 4: return createCanopyForager(x, y);
        default: return createTankHerbivore(x, y);
    }
}

Creature CreatureFactory::createOpportunist(int x, int y) const {
    // Randomly select from opportunist archetypes
    int choice = randomIntInRange(0, 1);
    switch (choice) {
        case 0: return createCarrionStalker(x, y);
        case 1: return createOmnivoreGeneralist(x, y);
        default: return createCarrionStalker(x, y);
    }
}

// ============================================================================
// Ecosystem Mix Creation
// ============================================================================

std::vector<Creature> CreatureFactory::createEcosystemMix(int count, int worldWidth, int worldHeight) const {
    std::vector<Creature> creatures;
    creatures.reserve(count);
    
    // Distribution: 60% herbivores, 25% predators, 15% opportunists
    int herbivoreCount = static_cast<int>(count * 0.60f);
    int predatorCount = static_cast<int>(count * 0.25f);
    int opportunistCount = count - herbivoreCount - predatorCount;
    
    // Create herbivores (60%)
    for (int i = 0; i < herbivoreCount; ++i) {
        int x = randomIntInRange(0, worldWidth - 1);
        int y = randomIntInRange(0, worldHeight - 1);
        creatures.push_back(createHerbivore(x, y));
    }
    
    // Create predators (25%)
    for (int i = 0; i < predatorCount; ++i) {
        int x = randomIntInRange(0, worldWidth - 1);
        int y = randomIntInRange(0, worldHeight - 1);
        creatures.push_back(createPredator(x, y));
    }
    
    // Create opportunists (15%)
    for (int i = 0; i < opportunistCount; ++i) {
        int x = randomIntInRange(0, worldWidth - 1);
        int y = randomIntInRange(0, worldHeight - 1);
        creatures.push_back(createOpportunist(x, y));
    }
    
    return creatures;
}

// ============================================================================
// Template Management
// ============================================================================

void CreatureFactory::registerTemplate(const CreatureTemplate& tmpl) {
    templates_[tmpl.name] = tmpl;
}

const CreatureTemplate* CreatureFactory::getTemplate(const std::string& name) const {
    auto it = templates_.find(name);
    if (it != templates_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> CreatureFactory::getAvailableTemplates() const {
    return getTemplateNames();
}

std::vector<std::string> CreatureFactory::getTemplateNames() const {
    std::vector<std::string> names;
    names.reserve(templates_.size());
    for (const auto& pair : templates_) {
        names.push_back(pair.first);
    }
    return names;
}

bool CreatureFactory::hasTemplate(const std::string& name) const {
    return templates_.find(name) != templates_.end();
}

void CreatureFactory::registerDefaultTemplates() {
    // Register all 11 archetype templates
    registerTemplate(createApexPredatorTemplate());
    registerTemplate(createPackHunterTemplate());
    registerTemplate(createAmbushPredatorTemplate());
    registerTemplate(createPursuitHunterTemplate());
    registerTemplate(createTankHerbivoreTemplate());
    registerTemplate(createArmoredGrazerTemplate());
    registerTemplate(createFleetRunnerTemplate());
    registerTemplate(createSpikyDefenderTemplate());
    registerTemplate(createCanopyForagerTemplate());
    registerTemplate(createCarrionStalkerTemplate());
    registerTemplate(createOmnivoreGeneralistTemplate());
    
    // Also register legacy templates for backward compatibility
    registerTemplate(createGrazerTemplate());
    registerTemplate(createBrowserTemplate());
    registerTemplate(createHunterTemplate());
    registerTemplate(createForagerTemplate());
}

// ============================================================================
// Pre-built Archetype Templates - Predators
// ============================================================================

CreatureTemplate CreatureFactory::createApexPredatorTemplate() {
    CreatureTemplate tmpl("apex_predator", "Apex Predator", CreatureCategory::ApexPredator);
    
    // Large, powerful, territorial dominant (Carnotitan)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {2.3f, 2.7f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {8000.0f, 12000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.2f, 1.5f};
    
    // Digestion - carnivore
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.90f, 0.98f};
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.85f, 0.95f};
    
    // Combat - Weapons
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::TEETH_SIZE] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::CLAW_LENGTH] = {0.70f, 0.80f};
    
    // Combat - Defense
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.70f, 0.80f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.10f, 0.20f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.30f, 0.40f};
    
    // Combat - Behavior
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.10f, 0.20f};
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.10f, 0.25f};
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.95f, 1.05f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.60f, 0.75f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.50f, 0.65f};
    
    // Hunting behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {80.0f, 120.0f};
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.60f, 0.75f};
    
    tmpl.renderCharacter = 'A';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createPackHunterTemplate() {
    CreatureTemplate tmpl("pack_hunter", "Pack Hunter", CreatureCategory::PackHunter);
    
    // Small, fast, coordinated (Carnosocialis)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {1.0f, 1.4f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {1.3f, 1.5f};
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {6000.0f, 9000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.3f, 1.6f};
    
    // Digestion - carnivore
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.75f, 0.85f};
    
    // Combat - Weapons (smaller but sharp)
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::TEETH_SIZE] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.70f, 0.80f};
    tmpl.geneRanges[UniversalGenes::CLAW_LENGTH] = {0.55f, 0.65f};
    
    // Combat - Defense (light)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.30f, 0.40f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.15f, 0.25f};
    
    // Combat - Behavior (coordinated)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.65f, 0.75f};
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.40f, 0.55f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.30f, 0.40f};
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.85f, 0.95f};  // High pack coordination
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.80f, 0.95f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.40f, 0.50f};
    
    // Hunting behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {70.0f, 100.0f};
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.80f, 0.90f};  // Good scent tracking
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.80f, 0.90f};
    
    tmpl.renderCharacter = 'P';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createAmbushPredatorTemplate() {
    CreatureTemplate tmpl("ambush_predator", "Ambush Predator", CreatureCategory::AmbushPredator);
    
    // Patient, high burst damage (Insidiatitan)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {2.0f, 2.4f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.50f, 0.70f};  // Slow but can burst
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {10000.0f, 14000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {0.80f, 1.0f};  // Low metabolism (patient)
    
    // Digestion - carnivore
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.90f, 0.98f};
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.02f, 0.10f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.85f, 0.95f};
    
    // Combat - Weapons (devastating bite)
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.90f, 0.98f};  // Highest teeth
    tmpl.geneRanges[UniversalGenes::TEETH_SIZE] = {0.90f, 0.98f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.60f, 0.70f};
    tmpl.geneRanges[UniversalGenes::CLAW_LENGTH] = {0.55f, 0.65f};
    
    // Combat - Defense (camouflaged scales)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.65f, 0.75f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.35f, 0.45f};
    
    // Combat - Behavior (patient)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.55f, 0.65f};  // Patient, not aggressive
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.70f, 0.80f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.20f, 0.30f};
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.05f, 0.15f};  // Solitary
    
    // Stealth
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.75f, 0.85f};  // High stealth
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.70f, 0.85f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.60f, 0.70f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.55f, 0.65f};
    
    // Hunting behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.70f, 0.80f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {60.0f, 90.0f};
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.70f, 0.80f};
    
    tmpl.renderCharacter = 'I';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createPursuitHunterTemplate() {
    CreatureTemplate tmpl("pursuit_hunter", "Pursuit Hunter", CreatureCategory::PursuitHunter);
    
    // Fastest predator (Velocipraeda)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {1.3f, 1.7f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {1.8f, 2.2f};  // Fastest creature
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {5000.0f, 8000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.5f, 1.9f};  // High metabolism
    
    // Digestion - carnivore
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.75f, 0.85f};
    
    // Combat - Weapons (slashing claws)
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.65f, 0.75f};
    tmpl.geneRanges[UniversalGenes::TEETH_SIZE] = {0.50f, 0.60f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.85f, 0.95f};  // Primary weapon
    tmpl.geneRanges[UniversalGenes::CLAW_LENGTH] = {0.80f, 0.90f};
    
    // Combat - Defense (light for speed)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.25f, 0.35f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.10f, 0.20f};
    
    // Combat - Behavior (knows when to quit)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.70f, 0.80f};
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.30f, 0.40f};  // Knows when to quit
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.20f, 0.35f};
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.90f, 1.05f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.35f, 0.45f};
    
    // Hunting behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {100.0f, 150.0f};  // Excellent vision
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.55f, 0.65f};
    
    tmpl.renderCharacter = 'V';
    
    return tmpl;
}

// ============================================================================
// Pre-built Archetype Templates - Herbivores
// ============================================================================

CreatureTemplate CreatureFactory::createTankHerbivoreTemplate() {
    CreatureTemplate tmpl("tank_herbivore", "Tank Herbivore", CreatureCategory::TankHerbivore);
    
    // Large armored, horn defense (Herbotitan)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {2.8f, 3.2f};  // Largest creature
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.40f, 0.60f};  // Slow
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {12000.0f, 18000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {0.70f, 0.90f};
    
    // Digestion - herbivore
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.02f, 0.10f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.80f, 0.90f};
    
    // Combat - Weapons (horns)
    tmpl.geneRanges[UniversalGenes::HORN_LENGTH] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::HORN_POINTINESS] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.10f, 0.20f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.10f, 0.20f};
    
    // Combat - Defense (heavily armored)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.90f, 0.98f};  // Thick hide
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.30f, 0.40f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.65f, 0.75f};
    
    // Combat - Behavior (defensive)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.35f, 0.45f};  // Low aggression
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.50f, 0.60f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.15f, 0.25f};  // Stands ground
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.30f, 0.45f};  // Herds
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.60f, 0.75f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.70f, 0.85f};  // High tolerance
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.65f, 0.75f};
    
    // Behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.02f, 0.10f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {50.0f, 80.0f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {5.0f, 10.0f};
    
    tmpl.renderCharacter = 'T';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createArmoredGrazerTemplate() {
    CreatureTemplate tmpl("armored_grazer", "Armored Grazer", CreatureCategory::ArmoredGrazer);
    
    // Scales + tail club (Ankylosaurus-like)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {2.3f, 2.7f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.30f, 0.50f};  // Very slow
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {14000.0f, 20000.0f};  // Long-lived
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {0.60f, 0.80f};
    
    // Digestion - herbivore
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.02f, 0.10f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.85f, 0.95f};
    
    // Combat - Weapons (tail club)
    tmpl.geneRanges[UniversalGenes::TAIL_MASS] = {0.85f, 0.95f};  // Heavy tail club
    tmpl.geneRanges[UniversalGenes::TAIL_LENGTH] = {0.65f, 0.75f};
    tmpl.geneRanges[UniversalGenes::HORN_LENGTH] = {0.20f, 0.30f};
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.10f, 0.20f};
    
    // Combat - Defense (extremely armored)
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.90f, 0.98f};  // Maximum scales
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.75f, 0.85f};
    
    // Combat - Behavior (never runs)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.30f, 0.40f};
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.05f, 0.15f};  // Never runs
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.25f, 0.35f};
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.50f, 0.65f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.80f, 0.90f};  // Highest tolerance
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.70f, 0.80f};
    
    // Behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.02f, 0.08f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {40.0f, 60.0f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {2.0f, 5.0f};
    
    tmpl.renderCharacter = 'K';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createFleetRunnerTemplate() {
    CreatureTemplate tmpl("fleet_runner", "Fleet Runner", CreatureCategory::FleetRunner);
    
    // Pure speed, no combat (Herbocursus)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {0.8f, 1.2f};  // Small
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {1.7f, 2.1f};  // Very fast
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {5000.0f, 8000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.3f, 1.6f};
    
    // Digestion - herbivore
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.60f, 0.70f};
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.60f, 0.70f};
    
    // Combat - Weapons (none)
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::HORN_LENGTH] = {0.05f, 0.15f};
    
    // Combat - Defense (minimal)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.10f, 0.20f};
    
    // Combat - Behavior (flee immediately)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.02f, 0.10f};  // Very low aggression
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.85f, 0.95f};  // Runs immediately
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.50f, 0.65f};  // Herds for safety
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {1.0f, 1.2f};  // Fast recovery
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.25f, 0.35f};  // Low tolerance
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.30f, 0.40f};
    
    // Behavior (excellent detection)
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.02f, 0.10f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {100.0f, 140.0f};  // Excellent vision
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {12.0f, 18.0f};  // Detects threats far away
    
    tmpl.renderCharacter = 'R';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createSpikyDefenderTemplate() {
    CreatureTemplate tmpl("spiky_defender", "Spiky Defender", CreatureCategory::SpikyDefender);
    
    // Porcupine-like counter-damage (Spinosus)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {1.1f, 1.5f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.70f, 0.90f};  // Moderate speed
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {8000.0f, 12000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {0.90f, 1.1f};
    
    // Digestion - herbivore
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.70f, 0.80f};
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.70f, 0.80f};
    
    // Combat - Weapons (spines!)
    tmpl.geneRanges[UniversalGenes::BODY_SPINES] = {0.90f, 0.98f};  // Maximum body spines
    tmpl.geneRanges[UniversalGenes::TAIL_SPINES] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::HORN_LENGTH] = {0.25f, 0.35f};
    
    // Combat - Defense (spikes + moderate hide)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.55f, 0.65f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.20f, 0.30f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.30f, 0.40f};
    
    // Combat - Behavior (defensive counter-attacker)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.25f, 0.35f};  // Low aggression
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.35f, 0.45f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.25f, 0.35f};
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.35f, 0.50f};
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.75f, 0.90f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.50f, 0.60f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.55f, 0.65f};
    
    // Behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.02f, 0.10f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {50.0f, 80.0f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {6.0f, 10.0f};
    
    tmpl.renderCharacter = 'S';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createCanopyForagerTemplate() {
    CreatureTemplate tmpl("canopy_forager", "Canopy Forager", CreatureCategory::CanopyForager);
    
    // Monkey-inspired arboreal fruit-eating specialist
    // Excellent sensory abilities for finding ripe fruit
    // Core stats - moderate size, agile
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {1.0f, 1.6f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {1.0f, 1.3f};  // Agile for reaching fruit
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {7000.0f, 11000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.1f, 1.4f};  // Moderate-high for fruit diet
    
    // Digestion - fruit specialist
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.50f, 0.60f};  // Moderate plant digestion
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.10f, 0.20f};  // Low meat digestion
    tmpl.geneRanges[UniversalGenes::SWEETNESS_PREFERENCE] = {0.80f, 0.95f};  // HIGH fruit/sweetness preference
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.25f, 0.40f};  // Low - fruit has little cellulose
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.50f, 0.65f};  // Moderate gut
    tmpl.geneRanges[UniversalGenes::GUT_TRANSIT_TIME] = {1.5f, 3.0f};  // Good for seed dispersal
    tmpl.geneRanges[UniversalGenes::SEED_DESTRUCTION_RATE] = {0.10f, 0.25f};  // Low - good seed disperser
    
    // Combat - Weapons (minimal - peaceful)
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::TEETH_SIZE] = {0.20f, 0.35f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.20f, 0.30f};
    tmpl.geneRanges[UniversalGenes::CLAW_LENGTH] = {0.25f, 0.40f};  // Some climbing ability
    
    // Combat - Defense (light)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.20f, 0.35f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.05f, 0.15f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.20f, 0.30f};
    
    // Combat - Behavior (very low aggression)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.10f, 0.20f};  // Very low aggression
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.70f, 0.85f};  // Quick to flee
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.40f, 0.55f};  // Some social behavior
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.85f, 1.0f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.30f, 0.45f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.35f, 0.45f};
    
    // Sensory - EXCELLENT for finding fruit
    tmpl.geneRanges[UniversalGenes::COLOR_VISION] = {0.85f, 0.95f};  // Excellent color vision for ripe fruit
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {70.0f, 100.0f};  // Good vision
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.75f, 0.88f};  // Good fruit scent detection
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.70f, 0.85f};  // Strong smell for ripeness
    tmpl.geneRanges[UniversalGenes::SPATIAL_MEMORY] = {0.70f, 0.85f};  // Remembers fruit tree locations
    
    // Behavior - Peaceful forager
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.02f, 0.10f};  // Almost no hunting
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {10.0f, 15.0f};  // Alert to threats
    tmpl.geneRanges[UniversalGenes::CACHING_INSTINCT] = {0.35f, 0.50f};  // Some food caching
    
    // Olfactory system
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.30f, 0.45f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.25f, 0.40f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.20f, 0.35f};
    
    tmpl.renderCharacter = 'C';
    
    return tmpl;
}

// ============================================================================
// Pre-built Archetype Templates - Opportunists
// ============================================================================

CreatureTemplate CreatureFactory::createCarrionStalkerTemplate() {
    CreatureTemplate tmpl("carrion_stalker", "Carrion Stalker", CreatureCategory::CarrionStalker);
    
    // Corpse specialist (Necrophagus)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {1.2f, 1.6f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {1.0f, 1.2f};  // Moderate speed
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {10000.0f, 15000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {0.75f, 0.95f};
    
    // Digestion - carrion specialist
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.20f, 0.35f};
    tmpl.geneRanges[UniversalGenes::TOXIN_TOLERANCE] = {0.90f, 0.98f};  // Highest toxin tolerance
    tmpl.geneRanges[UniversalGenes::TOXIN_METABOLISM] = {0.80f, 0.90f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.85f, 0.95f};
    
    // Combat - Weapons (minimal)
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::TEETH_SIZE] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.35f, 0.45f};
    
    // Combat - Defense (moderate)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.10f, 0.20f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.35f, 0.45f};
    
    // Combat - Behavior (avoidant)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.20f, 0.30f};  // Low aggression
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.55f, 0.65f};  // Quick to retreat
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.30f, 0.45f};
    
    // Health
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.85f, 1.0f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.50f, 0.60f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.45f, 0.55f};
    
    // Behavior (excellent corpse detection)
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.05f, 0.15f};  // Avoids hunting
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.90f, 0.98f};  // Excellent smell
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.90f, 0.98f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {70.0f, 100.0f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {8.0f, 12.0f};
    
    tmpl.renderCharacter = 'N';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createOmnivoreGeneralistTemplate() {
    CreatureTemplate tmpl("omnivore_generalist", "Omnivore Generalist", CreatureCategory::OmnivoreGeneralist);
    
    // Jack of all trades (Omniflexus)
    // Core stats
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {1.6f, 2.0f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.90f, 1.1f};  // Average speed
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {8000.0f, 12000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.0f, 1.2f};
    
    // Digestion - balanced omnivore
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.55f, 0.65f};
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.55f, 0.65f};
    tmpl.geneRanges[UniversalGenes::TOXIN_TOLERANCE] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.50f, 0.60f};
    
    // Combat - Weapons (moderate all)
    tmpl.geneRanges[UniversalGenes::TEETH_SHARPNESS] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::TEETH_SIZE] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::CLAW_SHARPNESS] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::CLAW_LENGTH] = {0.45f, 0.55f};
    
    // Combat - Defense (moderate all)
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::SCALE_COVERAGE] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::FAT_LAYER_THICKNESS] = {0.35f, 0.45f};
    
    // Combat - Behavior (balanced)
    tmpl.geneRanges[UniversalGenes::COMBAT_AGGRESSION] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::TERRITORIAL_AGGRESSION] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::RETREAT_THRESHOLD] = {0.35f, 0.45f};
    tmpl.geneRanges[UniversalGenes::PACK_COORDINATION] = {0.45f, 0.55f};
    
    // Health (balanced)
    tmpl.geneRanges[UniversalGenes::REGENERATION_RATE] = {0.80f, 0.95f};
    tmpl.geneRanges[UniversalGenes::WOUND_TOLERANCE] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::BLEEDING_RESISTANCE] = {0.45f, 0.55f};
    
    // Behavior (balanced senses)
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.40f, 0.50f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.50f, 0.60f};
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.55f, 0.65f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {60.0f, 90.0f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {5.0f, 9.0f};
    
    tmpl.renderCharacter = 'O';
    
    return tmpl;
}

// ============================================================================
// Legacy Diet-Niche Templates (Deprecated - kept for compatibility)
// ============================================================================

CreatureTemplate CreatureFactory::createGrazerTemplate() {
    CreatureTemplate tmpl("grazer", "Grazer", CreatureCategory::TankHerbivore);
    
    // Grazer (Herbivore - eats grass/leaves)
    // High: plant_digestion, cellulose_breakdown, gut_length
    // Medium: body_size, hide_thickness
    // Low: meat_digestion, aggression
    
    // Core digestion traits - HIGH
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.75f, 0.95f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.7f, 0.9f};
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.75f, 0.95f};
    
    // Body traits - MEDIUM
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {4.0f, 8.0f};
    tmpl.geneRanges[UniversalGenes::HIDE_THICKNESS] = {0.4f, 0.7f};
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.5f, 0.7f};
    
    // Teeth adapted for grinding plants
    tmpl.geneRanges[UniversalGenes::TOOTH_GRINDING] = {0.7f, 0.9f};
    tmpl.geneRanges[UniversalGenes::TOOTH_SHARPNESS] = {0.1f, 0.3f};
    
    // Low meat digestion - herbivore
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.05f, 0.2f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.3f, 0.5f};
    
    // Low aggression - prey animal
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.0f, 0.2f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {3.0f, 8.0f};
    tmpl.geneRanges[UniversalGenes::PURSUE_THRESHOLD] = {1.0f, 3.0f};
    
    // Moderate movement
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.6f, 0.9f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {40.0f, 80.0f};
    
    // Metabolism
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {8000.0f, 15000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {0.8f, 1.2f};
    
    // Toxin handling
    tmpl.geneRanges[UniversalGenes::TOXIN_TOLERANCE] = {0.4f, 0.6f};
    
    // Olfactory system
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.2f, 0.4f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.25f, 0.35f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.05f, 0.15f};
    
    tmpl.renderCharacter = 'Q';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createBrowserTemplate() {
    CreatureTemplate tmpl("browser", "Browser", CreatureCategory::FleetRunner);
    
    // Browser (Herbivore - eats fruits/berries)
    
    // Fruit eating traits - HIGH
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.6f, 0.8f};
    tmpl.geneRanges[UniversalGenes::SWEETNESS_PREFERENCE] = {0.7f, 0.95f};
    tmpl.geneRanges[UniversalGenes::COLOR_VISION] = {0.7f, 0.9f};
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.7f, 0.9f};
    
    // Memory for finding fruit sources
    tmpl.geneRanges[UniversalGenes::SPATIAL_MEMORY] = {0.6f, 0.85f};
    tmpl.geneRanges[UniversalGenes::CACHING_INSTINCT] = {0.3f, 0.6f};
    
    // Agility for reaching fruit
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.8f, 1.2f};
    tmpl.geneRanges[UniversalGenes::JAW_SPEED] = {0.5f, 0.7f};
    
    // Low cellulose
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.1f, 0.35f};
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.4f, 0.6f};
    
    // Low meat digestion
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.1f, 0.3f};
    
    // Medium size, agile
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {2.0f, 5.0f};
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {50.0f, 100.0f};
    
    // Metabolism
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {6000.0f, 12000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.0f, 1.4f};
    
    // Seed interaction
    tmpl.geneRanges[UniversalGenes::GUT_TRANSIT_TIME] = {1.0f, 2.5f};
    tmpl.geneRanges[UniversalGenes::SEED_DESTRUCTION_RATE] = {0.1f, 0.3f};
    
    // Olfactory system
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.35f, 0.45f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.55f, 0.65f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.15f, 0.25f};
    
    tmpl.renderCharacter = '0';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createHunterTemplate() {
    CreatureTemplate tmpl("hunter", "Hunter", CreatureCategory::ApexPredator);
    
    // Hunter (Carnivore)
    
    // Core carnivore traits - HIGH
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.8f, 1.0f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.8f, 1.0f};
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.8f, 1.0f};
    
    // Speed and senses for hunting - HIGH
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {1.2f, 1.8f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {80.0f, 150.0f};
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.6f, 0.85f};
    
    // Attack traits
    tmpl.geneRanges[UniversalGenes::JAW_STRENGTH] = {0.6f, 0.85f};
    tmpl.geneRanges[UniversalGenes::JAW_SPEED] = {0.7f, 0.9f};
    tmpl.geneRanges[UniversalGenes::TOOTH_SHARPNESS] = {0.8f, 1.0f};
    tmpl.geneRanges[UniversalGenes::TOOTH_GRINDING] = {0.1f, 0.3f};
    
    // Short gut for meat digestion
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.3f, 0.5f};
    
    // Very low plant digestion
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.0f, 0.15f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.0f, 0.1f};
    
    // Pursuit behavior
    tmpl.geneRanges[UniversalGenes::PURSUE_THRESHOLD] = {10.0f, 20.0f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {0.0f, 3.0f};
    
    // Body - medium-large, athletic
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {5.0f, 12.0f};
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.5f, 0.7f};
    
    // Higher metabolism
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {5000.0f, 10000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.2f, 1.8f};
    
    // Olfactory system
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.35f, 0.45f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.45f, 0.55f};
    
    tmpl.renderCharacter = 'M';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createForagerTemplate() {
    CreatureTemplate tmpl("forager", "Forager", CreatureCategory::OmnivoreGeneralist);
    
    // Forager (Omnivore - balanced)
    
    // Balanced digestion - MEDIUM
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.4f, 0.6f};
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.4f, 0.6f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.4f, 0.6f};
    
    // Medium gut length
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.5f, 0.7f};
    
    // Mixed teeth
    tmpl.geneRanges[UniversalGenes::TOOTH_SHARPNESS] = {0.4f, 0.6f};
    tmpl.geneRanges[UniversalGenes::TOOTH_GRINDING] = {0.4f, 0.6f};
    tmpl.geneRanges[UniversalGenes::JAW_STRENGTH] = {0.4f, 0.6f};
    
    // Body - MEDIUM
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {3.0f, 7.0f};
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.8f, 1.2f};
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.4f, 0.6f};
    
    // Good senses
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {50.0f, 90.0f};
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.5f, 0.7f};
    tmpl.geneRanges[UniversalGenes::COLOR_VISION] = {0.5f, 0.7f};
    tmpl.geneRanges[UniversalGenes::SWEETNESS_PREFERENCE] = {0.4f, 0.6f};
    
    // Balanced behavior
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {4.0f, 8.0f};
    tmpl.geneRanges[UniversalGenes::PURSUE_THRESHOLD] = {4.0f, 8.0f};
    
    // Opportunistic memory
    tmpl.geneRanges[UniversalGenes::SPATIAL_MEMORY] = {0.5f, 0.7f};
    tmpl.geneRanges[UniversalGenes::CACHING_INSTINCT] = {0.4f, 0.6f};
    
    // Moderate toxin handling
    tmpl.geneRanges[UniversalGenes::TOXIN_TOLERANCE] = {0.4f, 0.6f};
    
    // Higher metabolism cost
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {7000.0f, 12000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {1.1f, 1.5f};
    
    // Olfactory system
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.35f, 0.55f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.25f, 0.35f};
    
    tmpl.renderCharacter = '0';
    
    return tmpl;
}

// ============================================================================
// Private Helpers
// ============================================================================

void CreatureFactory::applyTemplate(Genome& genome, const CreatureTemplate& tmpl) const {
    // DEBUG: Diagnostic output for template application
    #ifdef DEBUG_CLASSIFICATION
    std::cout << "[applyTemplate] Applying template: " << tmpl.name << std::endl;
    std::cout << "[applyTemplate] Template has " << tmpl.geneRanges.size() << " gene ranges" << std::endl;
    #endif
    
    for (const auto& [geneId, range] : tmpl.geneRanges) {
        float value = randomInRange(range.first, range.second);
        
        if (genome.hasGene(geneId)) {
            Gene& gene = genome.getGeneMutable(geneId);
            
            #ifdef DEBUG_CLASSIFICATION
            float oldVal = gene.getNumericValue(DominanceType::Incomplete);
            #endif
            
            gene.setAlleleValues(value);
            
            #ifdef DEBUG_CLASSIFICATION
            float newVal = gene.getNumericValue(DominanceType::Incomplete);
            if (geneId == UniversalGenes::MEAT_DIGESTION_EFFICIENCY ||
                geneId == UniversalGenes::MAX_SIZE ||
                geneId == UniversalGenes::COMBAT_AGGRESSION) {
                std::cout << "[applyTemplate] " << geneId << ": " << oldVal << " -> " << newVal
                          << " (target range: [" << range.first << ", " << range.second << "])" << std::endl;
            }
            #endif
        } else if (registry_->hasGene(geneId)) {
            const GeneDefinition& definition = registry_->getDefinition(geneId);
            Allele allele(value, 1.0f);
            Gene newGene(geneId, allele, allele);
            genome.addGene(newGene, definition.getChromosome());
            
            #ifdef DEBUG_CLASSIFICATION
            std::cout << "[applyTemplate] ADDED NEW GENE: " << geneId << " = " << value << std::endl;
            #endif
        } else {
            #ifdef DEBUG_CLASSIFICATION
            std::cout << "[applyTemplate] WARNING: Gene not found: " << geneId << std::endl;
            #endif
        }
    }
}

float CreatureFactory::randomInRange(float min, float max) {
    return RandomEngine::randomFloat(min, max);
}

int CreatureFactory::randomIntInRange(int min, int max) {
    return RandomEngine::randomInt(min, max);
}


} // namespace Genetics
} // namespace EcoSim
