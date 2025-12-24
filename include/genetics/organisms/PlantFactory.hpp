#pragma once

/**
 * @file PlantFactory.hpp
 * @brief Factory class for creating Plant instances with species templates
 * 
 * Phase 2.3: Implements species templates (Berry Bush, Oak Tree, Grass, Thorn Bush)
 * that define characteristic gene ranges for each species type. The factory
 * supports both template-based creation with variation and sexual reproduction
 * from parent plants.
 * 
 * @see docs/technical/genetics/plant-prefabs.md
 * @see docs/technical/genetics/plant-propagation-strategies.md
 */

#include "genetics/organisms/Plant.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Species template defining characteristic gene ranges
 * 
 * Templates define the "species identity" - the range of gene values that
 * make a plant recognizable as that species. Plants created from templates
 * have random values within these ranges, providing natural variation.
 * 
 * Example: Berry bushes always have fast growth (1.2-1.8) while oak trees
 * always have slow growth (0.2-0.4).
 */
struct SpeciesTemplate {
    /// Species name (e.g., "berry_bush", "oak_tree")
    std::string name;
    
    /// Display name for UI (e.g., "Berry Bush", "Oak Tree")
    std::string displayName;
    
    /// Gene ranges: gene_id -> (min_value, max_value)
    std::unordered_map<std::string, std::pair<float, float>> geneRanges;
    
    /// Expression strength for creature-type genes (dormant for plants)
    float expressionStrengthCreature = 0.1f;
    
    /// Expression strength for plant-type genes (active)
    float expressionStrengthPlant = 1.0f;
    
    /// Entity type for rendering
    EntityType entityType = EntityType::PLANT_GENERIC;
    
    /// Render character
    char renderCharacter = 'p';
    
    /**
     * @brief Default constructor
     */
    SpeciesTemplate() = default;
    
    /**
     * @brief Construct template with name
     * @param n Species name
     * @param display Display name (defaults to species name if empty)
     */
    SpeciesTemplate(const std::string& n, const std::string& display = "")
        : name(n), displayName(display.empty() ? n : display) {}
};

/**
 * @brief Factory for creating Plant instances with species templates
 * 
 * The PlantFactory provides:
 * - Template-based plant creation with natural variation
 * - Sexual reproduction combining parent genes
 * - Pre-built templates for common species
 * 
 * Usage:
 * @code
 *   GeneRegistry registry;
 *   UniversalGenes::registerDefaults(registry);
 *   
 *   PlantFactory factory(std::make_shared<GeneRegistry>(registry));
 *   factory.registerTemplate(PlantFactory::createBerryBushTemplate());
 *   factory.registerTemplate(PlantFactory::createOakTreeTemplate());
 *   
 *   // Create a berry bush at position (10, 20)
 *   Plant bush = factory.createFromTemplate("berry_bush", 10, 20);
 *   
 *   // Create offspring from two parents
 *   Plant offspring = factory.createOffspring(parent1, parent2, 15, 25);
 * @endcode
 */
class PlantFactory {
public:
    /**
     * @brief Construct factory with gene registry
     * @param registry Shared pointer to gene registry
     */
    explicit PlantFactory(std::shared_ptr<GeneRegistry> registry);
    
    // ========================================================================
    // Plant Creation
    // ========================================================================
    
    /**
     * @brief Create plant from species template with random variation
     * @param templateName Name of the registered template
     * @param x X position in world
     * @param y Y position in world
     * @return New Plant instance, or null plant if template not found
     * 
     * The created plant will have gene values randomly selected from
     * the template's defined ranges.
     */
    Plant createFromTemplate(const std::string& templateName, int x, int y) const;
    
    /**
     * @brief Create offspring from two parent plants (sexual reproduction)
     * @param parent1 First parent plant
     * @param parent2 Second parent plant
     * @param x X position for offspring
     * @param y Y position for offspring
     * @return New Plant instance with combined/mutated genes
     * 
     * Offspring genes are selected from parents using Mendelian inheritance
     * (50% chance from each parent per gene) with small mutations applied.
     */
    Plant createOffspring(const Plant& parent1, const Plant& parent2, int x, int y) const;
    
    /**
     * @brief Create a random plant (no template, full genome randomization)
     * @param x X position in world
     * @param y Y position in world
     * @return New Plant instance with random genes
     */
    Plant createRandom(int x, int y) const;
    
    // ========================================================================
    // Template Management
    // ========================================================================
    
    /**
     * @brief Register a species template
     * @param tmpl The template to register
     * 
     * Templates are stored by name. Registering a template with an existing
     * name will overwrite the previous template.
     */
    void registerTemplate(const SpeciesTemplate& tmpl);
    
    /**
     * @brief Get a registered template by name
     * @param name Template name to look up
     * @return Pointer to template, or nullptr if not found
     */
    const SpeciesTemplate* getTemplate(const std::string& name) const;
    
    /**
     * @brief Get names of all registered templates
     * @return Vector of template names
     */
    std::vector<std::string> getTemplateNames() const;
    
    /**
     * @brief Check if a template is registered
     * @param name Template name to check
     * @return true if template exists
     */
    bool hasTemplate(const std::string& name) const;
    
    /**
     * @brief Register all default species templates
     * 
     * Registers: berry_bush, oak_tree, grass, thorn_bush
     */
    void registerDefaultTemplates();
    
    // ========================================================================
    // Pre-built Species Templates
    // ========================================================================
    
    /**
     * @brief Create Berry Bush template (r-strategy)
     * @return Species template for berry bushes
     * 
     * Traits:
     * - Fast growth (1.5)
     * - High fruit production (0.8)
     * - Low thorns (0.1)
     * - Short lifespan (500)
     */
    static SpeciesTemplate createBerryBushTemplate();
    
    /**
     * @brief Create Oak Tree template (K-strategy)
     * @return Species template for oak trees
     * 
     * Traits:
     * - Slow growth (0.3)
     * - Low fruit production (0.2)
     * - High hardiness (0.8)
     * - Long lifespan (5000)
     */
    static SpeciesTemplate createOakTreeTemplate();
    
    /**
     * @brief Create Grass template (grazing-adapted)
     * @return Species template for grass
     * 
     * Traits:
     * - Very fast growth (2.0)
     * - Maximum regrowth (0.9)
     * - Vegetative spread (runner_production = 1.5)
     * - Tiny size (0.3)
     */
    static SpeciesTemplate createGrassTemplate();
    
    /**
     * @brief Create Thorn Bush template (defensive)
     * @return Species template for thorn bushes
     * 
     * Traits:
     * - High thorns (0.8)
     * - High toxins (0.6)
     * - Slow growth (0.4)
     * - Medium lifespan (1500)
     */
    static SpeciesTemplate createThornBushTemplate();
    
private:
    /// Gene registry for genome creation
    std::shared_ptr<GeneRegistry> registry_;
    
    /// Registered species templates
    std::unordered_map<std::string, SpeciesTemplate> templates_;
    
    /**
     * @brief Apply template gene ranges to a genome
     * @param genome Genome to modify
     * @param tmpl Template with gene ranges
     */
    void applyTemplate(Genome& genome, const SpeciesTemplate& tmpl) const;
    
    /**
     * @brief Get random value within range
     * @param min Minimum value
     * @param max Maximum value
     * @return Random value in [min, max]
     */
    static float randomInRange(float min, float max);
};

} // namespace Genetics
} // namespace EcoSim
