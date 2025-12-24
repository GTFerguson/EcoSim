#pragma once

/**
 * @file CreatureFactory.hpp
 * @brief Factory class for creating Creature instances with diet-niche templates
 * 
 * Implements diet-niche templates (Grazer, Browser, Hunter, Forager, Scavenger)
 * that define characteristic gene ranges for each niche type. The factory
 * supports both template-based creation with variation and integration with
 * the existing Creature class.
 * 
 * @see docs/technical/genetics/creature-plant-coevolution.md
 * @see include/genetics/organisms/PlantFactory.hpp
 */

#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "objects/creature/creature.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Diet-niche template defining characteristic gene ranges for creatures
 * 
 * Templates define the "niche identity" - the range of gene values that
 * characterize a creature's diet and ecological role. Creatures created from
 * templates have random values within these ranges, providing natural variation.
 * 
 * Example: Grazers have high plant_digestion (0.7-0.9) while Hunters
 * have high meat_digestion (0.8-1.0).
 */
struct CreatureTemplate {
    /// Niche name (e.g., "grazer", "hunter")
    std::string name;
    
    /// Display name for UI (e.g., "Grazer", "Hunter")
    std::string displayName;
    
    /// Gene ranges: gene_id -> (min_value, max_value)
    std::unordered_map<std::string, std::pair<float, float>> geneRanges;
    
    /// Legacy diet type for backward compatibility
    Diet legacyDiet = Diet::apple;
    
    /// Render character
    char renderCharacter = '?';
    
    /**
     * @brief Default constructor
     */
    CreatureTemplate() = default;
    
    /**
     * @brief Construct template with name
     * @param n Niche name
     * @param display Display name (defaults to niche name if empty)
     */
    CreatureTemplate(const std::string& n, const std::string& display = "")
        : name(n), displayName(display.empty() ? n : display) {}
};

/**
 * @brief Factory for creating Creature instances with diet-niche templates
 * 
 * The CreatureFactory provides:
 * - Template-based creature creation with natural variation
 * - Pre-built templates for common diet niches (grazer, browser, hunter, forager, scavenger)
 * - Integration with existing legacy Genome system
 * 
 * Usage:
 * @code
 *   auto registry = std::make_shared<GeneRegistry>();
 *   UniversalGenes::registerDefaults(*registry);
 *   
 *   CreatureFactory factory(registry);
 *   factory.registerDefaultTemplates();
 *   
 *   // Create a grazer creature at position (10, 20)
 *   Creature grazer = factory.createFromTemplate("grazer", 10, 20);
 *   
 *   // Create a hunter
 *   Creature hunter = factory.createFromTemplate("hunter", 15, 25);
 * @endcode
 */
class CreatureFactory {
public:
    /**
     * @brief Construct factory with gene registry
     * @param registry Shared pointer to gene registry
     */
    explicit CreatureFactory(std::shared_ptr<GeneRegistry> registry);
    
    // ========================================================================
    // Creature Creation
    // ========================================================================
    
    /**
     * @brief Create creature from diet-niche template with random variation
     * @param templateName Name of the registered template
     * @param x X position in world
     * @param y Y position in world
     * @return New Creature instance, or default creature if template not found
     * 
     * The created creature will have gene values randomly selected from
     * the template's defined ranges.
     */
    Creature createFromTemplate(const std::string& templateName, int x, int y) const;
    
    /**
     * @brief Create a random creature (no template, full genome randomization)
     * @param x X position in world
     * @param y Y position in world
     * @return New Creature instance with random genes
     */
    Creature createRandom(int x, int y) const;
    
    // ========================================================================
    // Template Management
    // ========================================================================
    
    /**
     * @brief Register a diet-niche template
     * @param tmpl The template to register
     * 
     * Templates are stored by name. Registering a template with an existing
     * name will overwrite the previous template.
     */
    void registerTemplate(const CreatureTemplate& tmpl);
    
    /**
     * @brief Get a registered template by name
     * @param name Template name to look up
     * @return Pointer to template, or nullptr if not found
     */
    const CreatureTemplate* getTemplate(const std::string& name) const;
    
    /**
     * @brief Get names of all registered templates (aliases for getTemplateNames)
     * @return Vector of template names
     */
    std::vector<std::string> getAvailableTemplates() const;
    
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
     * @brief Register all default diet-niche templates
     * 
     * Registers: grazer, browser, hunter, forager, scavenger
     */
    void registerDefaultTemplates();
    
    // ========================================================================
    // Pre-built Diet-Niche Templates
    // ========================================================================
    
    /**
     * @brief Create Grazer template (Herbivore - eats grass/leaves)
     * @return Diet-niche template for grazers
     * 
     * Traits:
     * - High: plant_digestion, cellulose_breakdown, gut_length
     * - Medium: body_size, hide_thickness
     * - Low: meat_digestion, aggression
     */
    static CreatureTemplate createGrazerTemplate();
    
    /**
     * @brief Create Browser template (Herbivore - eats fruits/berries)
     * @return Diet-niche template for browsers
     * 
     * Traits:
     * - High: fruit_digestion (via plant_digestion), scent_detection, sweet_preference
     * - Medium: agility (locomotion), spatial_memory
     * - Low: cellulose_breakdown, meat_digestion
     */
    static CreatureTemplate createBrowserTemplate();
    
    /**
     * @brief Create Hunter template (Carnivore)
     * @return Diet-niche template for hunters
     * 
     * Traits:
     * - High: meat_digestion, aggression (hunt_instinct), sight_range, base_speed (locomotion)
     * - Medium: jaw_strength, tooth_sharpness
     * - Low: plant_digestion, cellulose_breakdown
     */
    static CreatureTemplate createHunterTemplate();
    
    /**
     * @brief Create Forager template (Omnivore - balanced)
     * @return Diet-niche template for foragers
     * 
     * Traits:
     * - Medium: plant_digestion, meat_digestion
     * - Medium: body_size (max_size), base_speed (locomotion)
     * - Balanced across all traits (with metabolic overhead)
     */
    static CreatureTemplate createForagerTemplate();
    
    /**
     * @brief Create Scavenger template (Carnivore - carrion specialist)
     * @return Diet-niche template for scavengers
     * 
     * Traits:
     * - High: meat_digestion, scent_detection, toxin_tolerance
     * - Medium: aggression (low), endurance
     * - Low: base_speed (locomotion), aggression (hunt_instinct)
     */
    static CreatureTemplate createScavengerTemplate();
    
private:
    /// Gene registry for genome creation
    std::shared_ptr<GeneRegistry> registry_;
    
    /// Registered diet-niche templates
    std::unordered_map<std::string, CreatureTemplate> templates_;
    
    /**
     * @brief Apply template gene ranges to a genome
     * @param genome Genome to modify
     * @param tmpl Template with gene ranges
     */
    void applyTemplate(Genome& genome, const CreatureTemplate& tmpl) const;
    
    /**
     * @brief Get random value within range
     * @param min Minimum value
     * @param max Maximum value
     * @return Random value in [min, max]
     */
    static float randomInRange(float min, float max);
    
    /**
     * @brief Create a legacy Genome compatible with creature expectations
     * @param tmpl Template to base the genome on
     * @return Legacy Genome object
     */
    ::Genome createLegacyGenome(const CreatureTemplate& tmpl) const;
};

} // namespace Genetics
} // namespace EcoSim
