/**
 * @file CreatureFactory.cpp
 * @brief Implementation of the CreatureFactory class
 * 
 * Implements diet-niche templates (Grazer, Browser, Hunter, Forager, Scavenger)
 * that define characteristic gene ranges for each niche type.
 */

#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <random>
#include <algorithm>
#include <stdexcept>

namespace EcoSim {
namespace Genetics {

// Thread-local random number generator
static thread_local std::mt19937 s_rng(std::random_device{}());

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
// Creature Creation
// ============================================================================

Creature CreatureFactory::createFromTemplate(const std::string& templateName, int x, int y) const {
    const CreatureTemplate* tmpl = getTemplate(templateName);
    
    if (!tmpl) {
        // Template not found - create with default genome
        ::Genome defaultGenome;
        return Creature(x, y, defaultGenome);
    }
    
    // Create a legacy genome for the creature constructor
    ::Genome legacyGenome = createLegacyGenome(*tmpl);
    
    // Create creature with legacy genome
    Creature creature(x, y, legacyGenome);
    
    // Enable new genetics - this will call UniversalGenes::createCreatureGenome()
    // then overlay the legacy values
    creature.enableNewGenetics(true);
    
    return creature;
}

Creature CreatureFactory::createRandom(int x, int y) const {
    ::Genome randomGenome;
    Creature creature(x, y, randomGenome);
    creature.enableNewGenetics(true);
    return creature;
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
    registerTemplate(createGrazerTemplate());
    registerTemplate(createBrowserTemplate());
    registerTemplate(createHunterTemplate());
    registerTemplate(createForagerTemplate());
    registerTemplate(createScavengerTemplate());
}

// ============================================================================
// Pre-built Diet-Niche Templates
// ============================================================================

CreatureTemplate CreatureFactory::createGrazerTemplate() {
    CreatureTemplate tmpl("grazer", "Grazer");
    
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
    
    // Olfactory system - Grass Grazer: Low scent needs (grass abundant)
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.15f, 0.25f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.2f, 0.4f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.25f, 0.35f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.05f, 0.15f};
    
    // Legacy compatibility
    tmpl.legacyDiet = Diet::banana;
    tmpl.renderCharacter = 'Q';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createBrowserTemplate() {
    CreatureTemplate tmpl("browser", "Browser");
    
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
    
    // Seed interaction - good disperser
    tmpl.geneRanges[UniversalGenes::GUT_TRANSIT_TIME] = {1.0f, 2.5f};
    tmpl.geneRanges[UniversalGenes::SEED_DESTRUCTION_RATE] = {0.1f, 0.3f};
    
    // Olfactory system - Frugivore (Browser): Needs to find scattered fruit
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.35f, 0.45f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.55f, 0.65f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.15f, 0.25f};
    
    // Legacy compatibility
    tmpl.legacyDiet = Diet::apple;
    tmpl.renderCharacter = '0';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createHunterTemplate() {
    CreatureTemplate tmpl("hunter", "Hunter");
    
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
    
    // Olfactory system - Carnivore (Hunter): High detection, high stealth
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.35f, 0.45f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.75f, 0.85f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.45f, 0.55f};
    
    // Legacy compatibility
    tmpl.legacyDiet = Diet::predator;
    tmpl.renderCharacter = 'M';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createForagerTemplate() {
    CreatureTemplate tmpl("forager", "Forager");
    
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
    
    // Olfactory system - Omnivore (Forager): Balanced sensory capabilities
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.35f, 0.55f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.45f, 0.55f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.25f, 0.35f};
    
    // Legacy compatibility
    tmpl.legacyDiet = Diet::apple;
    tmpl.renderCharacter = '0';
    
    return tmpl;
}

CreatureTemplate CreatureFactory::createScavengerTemplate() {
    CreatureTemplate tmpl("scavenger", "Scavenger");
    
    // Scavenger (Carnivore - carrion specialist)
    
    // Carrion eating traits - HIGH
    tmpl.geneRanges[UniversalGenes::MEAT_DIGESTION_EFFICIENCY] = {0.7f, 0.9f};
    tmpl.geneRanges[UniversalGenes::TOXIN_TOLERANCE] = {0.8f, 1.0f};
    tmpl.geneRanges[UniversalGenes::TOXIN_METABOLISM] = {0.7f, 0.9f};
    tmpl.geneRanges[UniversalGenes::STOMACH_ACIDITY] = {0.8f, 1.0f};
    
    // Amazing sense of smell - HIGH
    tmpl.geneRanges[UniversalGenes::SCENT_DETECTION] = {0.85f, 1.0f};
    
    // Low speed
    tmpl.geneRanges[UniversalGenes::LOCOMOTION] = {0.5f, 0.8f};
    
    // Low aggression
    tmpl.geneRanges[UniversalGenes::HUNT_INSTINCT] = {0.1f, 0.3f};
    tmpl.geneRanges[UniversalGenes::FLEE_THRESHOLD] = {5.0f, 12.0f};
    tmpl.geneRanges[UniversalGenes::PURSUE_THRESHOLD] = {1.0f, 4.0f};
    
    // Short gut - meat eater
    tmpl.geneRanges[UniversalGenes::GUT_LENGTH] = {0.35f, 0.5f};
    
    // Low plant digestion
    tmpl.geneRanges[UniversalGenes::PLANT_DIGESTION_EFFICIENCY] = {0.1f, 0.3f};
    tmpl.geneRanges[UniversalGenes::CELLULOSE_BREAKDOWN] = {0.05f, 0.2f};
    
    // Body - medium, built for endurance
    tmpl.geneRanges[UniversalGenes::MAX_SIZE] = {3.0f, 6.0f};
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.6f, 0.8f};
    tmpl.geneRanges[UniversalGenes::SIGHT_RANGE] = {60.0f, 100.0f};
    
    // Teeth
    tmpl.geneRanges[UniversalGenes::TOOTH_SHARPNESS] = {0.5f, 0.7f};
    tmpl.geneRanges[UniversalGenes::JAW_STRENGTH] = {0.5f, 0.75f};
    
    // Lower metabolism
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {8000.0f, 14000.0f};
    tmpl.geneRanges[UniversalGenes::METABOLISM_RATE] = {0.7f, 1.0f};
    
    // Olfactory system - Scavenger: Amazing scent detection for finding carrion
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.3f, 0.4f};
    tmpl.geneRanges[UniversalGenes::SCENT_SIGNATURE_VARIANCE] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::OLFACTORY_ACUITY] = {0.85f, 0.95f};
    tmpl.geneRanges[UniversalGenes::SCENT_MASKING] = {0.2f, 0.3f};
    
    // Legacy compatibility
    tmpl.legacyDiet = Diet::scavenger;
    tmpl.renderCharacter = 'm';
    
    return tmpl;
}

// ============================================================================
// Private Helpers
// ============================================================================

void CreatureFactory::applyTemplate(Genome& genome, const CreatureTemplate& tmpl) const {
    for (const auto& [geneId, range] : tmpl.geneRanges) {
        float value = randomInRange(range.first, range.second);
        
        if (genome.hasGene(geneId)) {
            Gene& gene = genome.getGeneMutable(geneId);
            gene.setAlleleValues(value);
        } else if (registry_->hasGene(geneId)) {
            const GeneDefinition& definition = registry_->getDefinition(geneId);
            Allele allele(value, 1.0f);
            Gene newGene(geneId, allele, allele);
            genome.addGene(newGene, definition.getChromosome());
        }
    }
}

float CreatureFactory::randomInRange(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(s_rng);
}

::Genome CreatureFactory::createLegacyGenome(const CreatureTemplate& tmpl) const {
    unsigned lifespan = 10000;
    auto lifespanIt = tmpl.geneRanges.find(UniversalGenes::LIFESPAN);
    if (lifespanIt != tmpl.geneRanges.end()) {
        lifespan = static_cast<unsigned>(randomInRange(lifespanIt->second.first, lifespanIt->second.second));
    }
    
    unsigned sight = 50;
    auto sightIt = tmpl.geneRanges.find(UniversalGenes::SIGHT_RANGE);
    if (sightIt != tmpl.geneRanges.end()) {
        sight = static_cast<unsigned>(randomInRange(sightIt->second.first, sightIt->second.second));
    }
    
    float hunger = 5.0f;
    auto hungerIt = tmpl.geneRanges.find(UniversalGenes::HUNGER_THRESHOLD);
    if (hungerIt != tmpl.geneRanges.end()) {
        hunger = randomInRange(hungerIt->second.first, hungerIt->second.second);
    }
    
    float thirst = 5.0f;
    auto thirstIt = tmpl.geneRanges.find(UniversalGenes::THIRST_THRESHOLD);
    if (thirstIt != tmpl.geneRanges.end()) {
        thirst = randomInRange(thirstIt->second.first, thirstIt->second.second);
    }
    
    float fatigue = 5.0f;
    auto fatigueIt = tmpl.geneRanges.find(UniversalGenes::FATIGUE_THRESHOLD);
    if (fatigueIt != tmpl.geneRanges.end()) {
        fatigue = randomInRange(fatigueIt->second.first, fatigueIt->second.second);
    }
    
    float mate = 3.0f;
    auto mateIt = tmpl.geneRanges.find(UniversalGenes::MATE_THRESHOLD);
    if (mateIt != tmpl.geneRanges.end()) {
        mate = randomInRange(mateIt->second.first, mateIt->second.second);
    }
    
    float comfInc = 0.004f;
    auto comfIncIt = tmpl.geneRanges.find(UniversalGenes::COMFORT_INCREASE);
    if (comfIncIt != tmpl.geneRanges.end()) {
        comfInc = randomInRange(comfIncIt->second.first, comfIncIt->second.second);
    }
    
    float comfDec = 0.002f;
    auto comfDecIt = tmpl.geneRanges.find(UniversalGenes::COMFORT_DECREASE);
    if (comfDecIt != tmpl.geneRanges.end()) {
        comfDec = randomInRange(comfDecIt->second.first, comfDecIt->second.second);
    }
    
    unsigned flee = 3;
    auto fleeIt = tmpl.geneRanges.find(UniversalGenes::FLEE_THRESHOLD);
    if (fleeIt != tmpl.geneRanges.end()) {
        flee = static_cast<unsigned>(randomInRange(fleeIt->second.first, fleeIt->second.second));
    }
    
    unsigned pursue = 6;
    auto pursueIt = tmpl.geneRanges.find(UniversalGenes::PURSUE_THRESHOLD);
    if (pursueIt != tmpl.geneRanges.end()) {
        pursue = static_cast<unsigned>(randomInRange(pursueIt->second.first, pursueIt->second.second));
    }
    
    bool flocks = (tmpl.legacyDiet != Diet::predator);
    
    return ::Genome(lifespan, hunger, thirst, fatigue, mate,
                    comfInc, comfDec, sight, tmpl.legacyDiet, flocks, flee, pursue);
}

} // namespace Genetics
} // namespace EcoSim
