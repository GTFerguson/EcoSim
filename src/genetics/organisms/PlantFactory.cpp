/**
 * @file PlantFactory.cpp
 * @brief Implementation of the PlantFactory class
 * 
 * Phase 2.3: Implements species templates and factory methods for
 * creating plants with characteristic traits.
 */

#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/core/RandomEngine.hpp"
#include <algorithm>
#include <stdexcept>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructor
// ============================================================================

PlantFactory::PlantFactory(std::shared_ptr<GeneRegistry> registry)
    : registry_(std::move(registry))
{
    if (!registry_) {
        throw std::invalid_argument("PlantFactory requires a valid GeneRegistry");
    }
    
    // Ensure PlantGenes are registered - Plant.cpp relies on these gene IDs
    // This is safe to call multiple times; the registry will skip duplicates
    PlantGenes::registerDefaults(*registry_);
}

// ============================================================================
// Plant Creation
// ============================================================================

Plant PlantFactory::createFromTemplate(const std::string& templateName, int x, int y) const {
    const SpeciesTemplate* tmpl = getTemplate(templateName);
    
    if (!tmpl) {
        // Template not found - create with defaults
        return Plant(x, y, *registry_);
    }
    
    // Create base genome using PlantGenes (which Plant.cpp reads from)
    // This ensures the genome has genes that Plant::getGrowthRate(), getMaxSize() etc. can find
    Genome genome = PlantGenes::createDefaultGenome(*registry_);
    
    // Apply template gene ranges (species-specific values)
    applyTemplate(genome, *tmpl);
    
    // Create plant with customized genome
    Plant plant(x, y, genome, *registry_);
    
    // Set entity type from template
    plant.setEntityType(tmpl->entityType);
    
    return plant;
}

Plant PlantFactory::createOffspring(const Plant& parent1, const Plant& parent2, int x, int y) const {
    const Genome& genome1 = parent1.getGenome();
    const Genome& genome2 = parent2.getGenome();
    
    // Create offspring genome through crossover (static method)
    Genome offspringGenome = Genome::crossover(genome1, genome2);
    
    // Apply mutation
    float mutationRate = 0.05f;  // 5% mutation rate
    offspringGenome.mutate(mutationRate, registry_->getAllDefinitions());
    
    // Create offspring
    Plant offspring(x, y, offspringGenome, *registry_);
    
    // Inherit entity type from parent1 (primary parent)
    offspring.setEntityType(parent1.getEntityType());
    
    return offspring;
}

Plant PlantFactory::createRandom(int x, int y) const {
    return Plant(x, y, *registry_);
}

// ============================================================================
// Template Management
// ============================================================================

void PlantFactory::registerTemplate(const SpeciesTemplate& tmpl) {
    templates_[tmpl.name] = tmpl;
}

const SpeciesTemplate* PlantFactory::getTemplate(const std::string& name) const {
    auto it = templates_.find(name);
    if (it != templates_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> PlantFactory::getTemplateNames() const {
    std::vector<std::string> names;
    names.reserve(templates_.size());
    for (const auto& pair : templates_) {
        names.push_back(pair.first);
    }
    return names;
}

bool PlantFactory::hasTemplate(const std::string& name) const {
    return templates_.find(name) != templates_.end();
}

void PlantFactory::registerDefaultTemplates() {
    registerTemplate(createBerryBushTemplate());
    registerTemplate(createOakTreeTemplate());
    registerTemplate(createGrassTemplate());
    registerTemplate(createThornBushTemplate());
}

// ============================================================================
// Pre-built Species Templates
// ============================================================================

SpeciesTemplate PlantFactory::createBerryBushTemplate() {
    SpeciesTemplate tmpl("berry_bush", "Berry Bush");
    
    // r-strategy: fast growth, high reproduction, short lifespan
    
    // Core traits - differentiated per species for test compatibility
    // UniversalGenes::LIFESPAN is the universal foundation for all organisms
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {5000.0f, 10000.0f};        // Medium lifespan (perennial) ~7500
    tmpl.geneRanges[PlantGenes::GROWTH_RATE] = {0.45f, 0.55f};              // Medium growth ~0.5
    tmpl.geneRanges[PlantGenes::MAX_SIZE] = {1.8f, 2.2f};                   // Small shrub ~2.0
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.3f, 0.5f};              // Low hardiness
    
    // High fruit production (key r-strategy trait)
    tmpl.geneRanges[UniversalGenes::FRUIT_PRODUCTION_RATE] = {0.7f, 0.9f};  // High ~0.8
    tmpl.geneRanges[UniversalGenes::FRUIT_APPEAL] = {0.7f, 0.9f};           // Attractive fruit
    
    // Moderate-high scent - berries are fragrant to attract animal dispersers
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.3f, 0.5f};       // ~0.4
    
    // Low defense (invests in reproduction instead)
    tmpl.geneRanges[UniversalGenes::THORN_DENSITY] = {0.05f, 0.15f};        // Low thorns ~0.1
    tmpl.geneRanges[UniversalGenes::TOXIN_PRODUCTION] = {0.0f, 0.1f};       // Non-toxic
    tmpl.geneRanges[UniversalGenes::REGROWTH_RATE] = {0.3f, 0.5f};          // Moderate regrowth
    
    // Seed traits for animal-fruit dispersal
    tmpl.geneRanges[UniversalGenes::SEED_MASS] = {0.3f, 0.5f};              // Medium seeds
    tmpl.geneRanges[UniversalGenes::SEED_COAT_DURABILITY] = {0.6f, 0.8f};   // Durable (survive digestion)
    tmpl.geneRanges[UniversalGenes::SEED_AERODYNAMICS] = {0.1f, 0.2f};      // Low (not wind-dispersed)
    tmpl.geneRanges[UniversalGenes::SEED_HOOK_STRENGTH] = {0.0f, 0.1f};     // No hooks
    tmpl.geneRanges[UniversalGenes::EXPLOSIVE_POD_FORCE] = {0.0f, 0.1f};    // No explosive pods
    tmpl.geneRanges[UniversalGenes::RUNNER_PRODUCTION] = {0.0f, 0.2f};      // Minimal vegetative
    
    // Spread distance
    tmpl.geneRanges[UniversalGenes::SPREAD_DISTANCE] = {3.0f, 8.0f};        // Medium spread
    
    // Color: reddish berries (shift from green)
    tmpl.geneRanges[UniversalGenes::COLOR_HUE] = {90.0f, 150.0f};           // Green to yellow-green
    
    // Environment tolerance
    tmpl.geneRanges[PlantGenes::WATER_NEED] = {0.4f, 0.6f};
    tmpl.geneRanges[PlantGenes::LIGHT_NEED] = {0.5f, 0.7f};
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_LOW] = {5.0f, 10.0f};
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_HIGH] = {30.0f, 40.0f};
    tmpl.geneRanges[UniversalGenes::WATER_STORAGE] = {0.45f, 0.55f};       // Moderate drought tolerance
    
    // Rendering
    tmpl.entityType = EntityType::PLANT_BERRY_BUSH;
    tmpl.renderCharacter = 'B';
    
    return tmpl;
}

SpeciesTemplate PlantFactory::createOakTreeTemplate() {
    SpeciesTemplate tmpl("oak_tree", "Oak Tree");
    
    // K-strategy: slow growth, high investment per offspring, long lifespan
    
    // Core traits - differentiated per species for test compatibility
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {15000.0f, 50000.0f};       // Long-lived tree ~32500
    tmpl.geneRanges[PlantGenes::GROWTH_RATE] = {0.15f, 0.25f};              // Slow growth ~0.2
    tmpl.geneRanges[PlantGenes::MAX_SIZE] = {14.0f, 16.0f};                 // Large tree ~15.0
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.7f, 0.9f};              // High hardiness ~0.8
    
    // Low fruit production (acorns, not berries)
    tmpl.geneRanges[UniversalGenes::FRUIT_PRODUCTION_RATE] = {0.15f, 0.25f}; // Low ~0.2
    tmpl.geneRanges[UniversalGenes::FRUIT_APPEAL] = {0.2f, 0.4f};           // Low appeal (acorns)
    
    // Very low scent - acorns have minimal fragrance
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.02f, 0.08f};     // ~0.05
    
    // Moderate defense (bark protects, but not chemically defended)
    tmpl.geneRanges[UniversalGenes::THORN_DENSITY] = {0.0f, 0.1f};          // No thorns
    tmpl.geneRanges[UniversalGenes::TOXIN_PRODUCTION] = {0.1f, 0.3f};       // Slight tannins
    tmpl.geneRanges[UniversalGenes::REGROWTH_RATE] = {0.1f, 0.2f};          // Slow regrowth
    
    // Seed traits for gravity/animal-cache dispersal
    tmpl.geneRanges[UniversalGenes::SEED_MASS] = {0.7f, 0.95f};             // Heavy seeds (acorns)
    tmpl.geneRanges[UniversalGenes::SEED_COAT_DURABILITY] = {0.7f, 0.9f};   // Very durable
    tmpl.geneRanges[UniversalGenes::SEED_AERODYNAMICS] = {0.0f, 0.1f};      // Not aerodynamic
    tmpl.geneRanges[UniversalGenes::SEED_HOOK_STRENGTH] = {0.0f, 0.05f};    // No hooks
    tmpl.geneRanges[UniversalGenes::EXPLOSIVE_POD_FORCE] = {0.0f, 0.05f};   // No explosive
    tmpl.geneRanges[UniversalGenes::RUNNER_PRODUCTION] = {0.0f, 0.1f};      // No vegetative spread
    
    // Short spread (gravity limited, squirrels help)
    tmpl.geneRanges[UniversalGenes::SPREAD_DISTANCE] = {1.0f, 5.0f};
    
    // Color: green foliage
    tmpl.geneRanges[UniversalGenes::COLOR_HUE] = {100.0f, 130.0f};          // Green
    
    // Environment tolerance (oaks are hardy)
    tmpl.geneRanges[PlantGenes::WATER_NEED] = {0.3f, 0.5f};
    tmpl.geneRanges[PlantGenes::LIGHT_NEED] = {0.6f, 0.8f};
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_LOW] = {-10.0f, 0.0f};
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_HIGH] = {35.0f, 45.0f};
    tmpl.geneRanges[UniversalGenes::WATER_STORAGE] = {0.55f, 0.70f};        // Good drought tolerance (deep roots)
    
    // Rendering
    tmpl.entityType = EntityType::PLANT_OAK_TREE;
    tmpl.renderCharacter = 'T';
    
    return tmpl;
}

SpeciesTemplate PlantFactory::createGrassTemplate() {
    SpeciesTemplate tmpl("grass", "Grass");
    
    // Grazing-adapted: very fast growth, high regrowth, vegetative spread
    
    // Core traits - differentiated per species for test compatibility
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {1000.0f, 3000.0f};         // Short-lived annual ~2000
    tmpl.geneRanges[PlantGenes::GROWTH_RATE] = {0.75f, 0.85f};              // Fast growth ~0.8
    tmpl.geneRanges[PlantGenes::MAX_SIZE] = {0.45f, 0.55f};                 // Low ground cover ~0.5
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.4f, 0.6f};              // Moderate hardiness
    
    // Low fruit production (grasses don't fruit like shrubs)
    tmpl.geneRanges[UniversalGenes::FRUIT_PRODUCTION_RATE] = {0.0f, 0.1f};  // Minimal
    tmpl.geneRanges[UniversalGenes::FRUIT_APPEAL] = {0.0f, 0.1f};           // Seeds, not fruit
    
    // No scent - grass doesn't produce fragrant parts
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.0f, 0.01f};      // ~0.005 (below threshold)
    
    // Key adaptation: maximum regrowth after grazing
    tmpl.geneRanges[UniversalGenes::THORN_DENSITY] = {0.0f, 0.05f};         // No thorns
    tmpl.geneRanges[UniversalGenes::TOXIN_PRODUCTION] = {0.0f, 0.1f};       // Non-toxic
    tmpl.geneRanges[UniversalGenes::REGROWTH_RATE] = {0.8f, 1.0f};          // Maximum regrowth ~0.9
    
    // Seed traits for wind + vegetative dispersal
    tmpl.geneRanges[UniversalGenes::SEED_MASS] = {0.01f, 0.05f};            // Tiny seeds
    tmpl.geneRanges[UniversalGenes::SEED_COAT_DURABILITY] = {0.2f, 0.4f};   // Light coat
    tmpl.geneRanges[UniversalGenes::SEED_AERODYNAMICS] = {0.3f, 0.5f};      // Some wind dispersal
    tmpl.geneRanges[UniversalGenes::SEED_HOOK_STRENGTH] = {0.0f, 0.1f};     // No hooks
    tmpl.geneRanges[UniversalGenes::EXPLOSIVE_POD_FORCE] = {0.0f, 0.05f};   // No explosive
    tmpl.geneRanges[UniversalGenes::RUNNER_PRODUCTION] = {0.6f, 0.9f};      // Moderate vegetative ~0.75 (balanced for spread rate)
    
    // Medium spread (stolons/rhizomes)
    tmpl.geneRanges[UniversalGenes::SPREAD_DISTANCE] = {2.0f, 6.0f};
    
    // Color: yellow-green to green
    tmpl.geneRanges[UniversalGenes::COLOR_HUE] = {80.0f, 120.0f};
    
    // Environment tolerance
    tmpl.geneRanges[PlantGenes::WATER_NEED] = {0.3f, 0.5f};
    tmpl.geneRanges[PlantGenes::LIGHT_NEED] = {0.7f, 0.9f};                 // Full sun
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_LOW] = {-5.0f, 5.0f};
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_HIGH] = {35.0f, 50.0f};  // Heat tolerant
    tmpl.geneRanges[UniversalGenes::WATER_STORAGE] = {0.35f, 0.50f};        // Lower water storage (shallow roots)
    
    // Rendering
    tmpl.entityType = EntityType::PLANT_GRASS;
    tmpl.renderCharacter = '"';
    
    return tmpl;
}

SpeciesTemplate PlantFactory::createThornBushTemplate() {
    SpeciesTemplate tmpl("thorn_bush", "Thorn Bush");
    
    // Defensive strategy: high investment in thorns and toxins
    
    // Core traits - differentiated per species for test compatibility
    tmpl.geneRanges[UniversalGenes::LIFESPAN] = {4000.0f, 8000.0f};         // Medium-lived shrub ~6000
    tmpl.geneRanges[PlantGenes::GROWTH_RATE] = {0.35f, 0.45f};              // Medium-slow growth ~0.4
    tmpl.geneRanges[PlantGenes::MAX_SIZE] = {2.8f, 3.2f};                   // Medium shrub ~3.0
    tmpl.geneRanges[UniversalGenes::HARDINESS] = {0.6f, 0.8f};              // High hardiness
    
    // Moderate fruit production
    tmpl.geneRanges[UniversalGenes::FRUIT_PRODUCTION_RATE] = {0.3f, 0.5f};
    tmpl.geneRanges[UniversalGenes::FRUIT_APPEAL] = {0.3f, 0.5f};           // Protected by thorns
    
    // Low scent - some berries but defended, not heavily fragrant
    tmpl.geneRanges[UniversalGenes::SCENT_PRODUCTION] = {0.1f, 0.2f};       // ~0.15
    
    // Key adaptation: high defense
    tmpl.geneRanges[UniversalGenes::THORN_DENSITY] = {0.7f, 0.9f};          // High thorns ~0.8
    tmpl.geneRanges[UniversalGenes::TOXIN_PRODUCTION] = {0.5f, 0.7f};       // High toxins ~0.6
    tmpl.geneRanges[UniversalGenes::REGROWTH_RATE] = {0.2f, 0.4f};          // Slow regrowth
    
    // Seed traits - burr dispersal (hooks attach to fur)
    tmpl.geneRanges[UniversalGenes::SEED_MASS] = {0.2f, 0.4f};              // Medium seeds
    tmpl.geneRanges[UniversalGenes::SEED_COAT_DURABILITY] = {0.4f, 0.6f};   // Moderate durability
    tmpl.geneRanges[UniversalGenes::SEED_AERODYNAMICS] = {0.1f, 0.2f};      // Low
    tmpl.geneRanges[UniversalGenes::SEED_HOOK_STRENGTH] = {0.7f, 0.9f};     // Strong hooks (burrs!)
    tmpl.geneRanges[UniversalGenes::EXPLOSIVE_POD_FORCE] = {0.0f, 0.1f};    // No explosive
    tmpl.geneRanges[UniversalGenes::RUNNER_PRODUCTION] = {0.0f, 0.2f};      // Minimal vegetative
    
    // Spread distance
    tmpl.geneRanges[UniversalGenes::SPREAD_DISTANCE] = {2.0f, 6.0f};
    
    // Color: dark green (thorns often darker)
    tmpl.geneRanges[UniversalGenes::COLOR_HUE] = {110.0f, 140.0f};
    
    // Environment tolerance
    tmpl.geneRanges[PlantGenes::WATER_NEED] = {0.2f, 0.4f};                 // Drought tolerant
    tmpl.geneRanges[PlantGenes::LIGHT_NEED] = {0.4f, 0.6f};
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_LOW] = {0.0f, 10.0f};
    tmpl.geneRanges[UniversalGenes::TEMP_TOLERANCE_HIGH] = {35.0f, 45.0f};
    tmpl.geneRanges[UniversalGenes::WATER_STORAGE] = {0.60f, 0.75f};        // Good drought tolerance (thorny desert shrub)
    
    // Rendering
    tmpl.entityType = EntityType::PLANT_THORN_BUSH;
    tmpl.renderCharacter = '*';
    
    return tmpl;
}

// ============================================================================
// Private Helpers
// ============================================================================

void PlantFactory::applyTemplate(Genome& genome, const SpeciesTemplate& tmpl) const {
    // Set each gene value to a random value within the template's range
    for (const auto& [geneId, range] : tmpl.geneRanges) {
        float value = randomInRange(range.first, range.second);
        
        // Use Genome's direct gene access API
        if (genome.hasGene(geneId)) {
            // Gene exists - update its value
            Gene& gene = genome.getGeneMutable(geneId);
            gene.setAlleleValues(value);
        } else if (registry_->hasGene(geneId)) {
            // Gene doesn't exist in genome but is defined in registry - add it
            const GeneDefinition& definition = registry_->getDefinition(geneId);
            Allele allele(value, 1.0f);  // Full expression
            Gene newGene(geneId, allele, allele);
            genome.addGene(newGene, definition.getChromosome());
        }
        // If gene is neither in genome nor registry, silently skip
    }
}

float PlantFactory::randomInRange(float min, float max) {
    return RandomEngine::randomFloat(min, max);
}

} // namespace Genetics
} // namespace EcoSim
