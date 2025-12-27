#pragma once

/**
 * @file CreatureFactory.hpp
 * @brief Factory class for creating Creature instances with combat-balanced archetype templates
 *
 * Implements 11 creature archetypes (Apex Predator, Pack Hunter, Ambush Predator,
 * Pursuit Hunter, Tank Herbivore, Armored Grazer, Fleet Runner, Spiky Defender,
 * Canopy Forager, Carrion Stalker, Omnivore Generalist) that define characteristic gene ranges for
 * each archetype. The factory supports both template-based creation with variation
 * and integration with the existing Creature class.
 *
 * @see docs/technical/genetics/creature-plant-coevolution.md
 * @see include/genetics/organisms/PlantFactory.hpp
 * @see plans/creature-prefab-design.md
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
 * @brief Creature archetype categories for ecosystem analysis
 *
 * Categorizes creatures by their ecological role, enabling ecosystem
 * balance analysis and combat dynamics visualization.
 */
enum class CreatureCategory {
    // Predators
    ApexPredator,      ///< Large territorial dominant (Carnotitan)
    PackHunter,        ///< Coordinated group hunters (Carnosocialis)
    AmbushPredator,    ///< Patient opportunistic (Insidiatitan)
    PursuitHunter,     ///< Speed-based chasers (Velocipraeda)
    
    // Herbivores
    TankHerbivore,     ///< Large armored defensive (Herbotitan)
    ArmoredGrazer,     ///< Scaled with tail defense (Ankylosaurus-like)
    FleetRunner,       ///< Speed-based escape (Herbocursus)
    SpikyDefender,     ///< Counter-attack spines (Spinosus)
    CanopyForager,     ///< Fruit-eating arboreal specialist (monkey-inspired)
    
    // Opportunists
    CarrionStalker,    ///< Corpse-feeders (Necrophagus)
    OmnivoreGeneralist ///< Adaptable (Omniflexus)
};

/**
 * @brief Archetype template defining characteristic gene ranges for creatures
 *
 * Templates define the "archetype identity" - the range of gene values that
 * characterize a creature's combat role and ecological niche. Creatures created
 * from templates have random values within these ranges, providing natural variation.
 *
 * Example: Apex Predators have high teeth_sharpness (0.85-0.95) while
 * Fleet Runners have high locomotion (1.7-2.1).
 */
struct CreatureTemplate {
    /// Archetype name (e.g., "apex_predator", "fleet_runner")
    std::string name;
    
    /// Display name for UI (e.g., "Apex Predator", "Fleet Runner")
    std::string displayName;
    
    /// Category for ecosystem analysis
    CreatureCategory category = CreatureCategory::OmnivoreGeneralist;
    
    /// Gene ranges: gene_id -> (min_value, max_value)
    std::unordered_map<std::string, std::pair<float, float>> geneRanges;
    
    /// Render character
    char renderCharacter = '?';
    
    /**
     * @brief Default constructor
     */
    CreatureTemplate() = default;
    
    /**
     * @brief Construct template with name and category
     * @param n Archetype name
     * @param display Display name (defaults to archetype name if empty)
     * @param cat Creature category
     */
    CreatureTemplate(const std::string& n, const std::string& display = "",
                     CreatureCategory cat = CreatureCategory::OmnivoreGeneralist)
        : name(n), displayName(display.empty() ? n : display), category(cat) {}
};

/**
 * @brief Factory for creating Creature instances with combat-balanced archetype templates
 *
 * The CreatureFactory provides:
 * - Template-based creature creation with natural variation
 * - 10 combat-balanced archetype templates
 * - Category labeling and scientific name generation
 * - Ecosystem mix creation for balanced spawning
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
 *   // Create an apex predator at position (10, 20)
 *   Creature apex = factory.createApexPredator(10, 20);
 *
 *   // Create a balanced ecosystem mix
 *   auto creatures = factory.createEcosystemMix(50, 200, 200);
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
    // Creature Creation - Generic
    // ========================================================================
    
    /**
     * @brief Create creature from archetype template with random variation
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
    // Creature Creation - Specific Archetypes
    // ========================================================================
    
    /**
     * @brief Create Apex Predator (Carnotitan) - Large territorial dominant
     * @param x X position in world
     * @param y Y position in world
     * @return New Apex Predator creature
     */
    Creature createApexPredator(int x, int y) const;
    
    /**
     * @brief Create Pack Hunter (Carnosocialis) - Coordinated group hunters
     * @param x X position in world
     * @param y Y position in world
     * @return New Pack Hunter creature
     */
    Creature createPackHunter(int x, int y) const;
    
    /**
     * @brief Create Ambush Predator (Insidiatitan) - Patient opportunistic
     * @param x X position in world
     * @param y Y position in world
     * @return New Ambush Predator creature
     */
    Creature createAmbushPredator(int x, int y) const;
    
    /**
     * @brief Create Pursuit Hunter (Velocipraeda) - Speed-based chasers
     * @param x X position in world
     * @param y Y position in world
     * @return New Pursuit Hunter creature
     */
    Creature createPursuitHunter(int x, int y) const;
    
    /**
     * @brief Create Tank Herbivore (Herbotitan) - Large armored defensive
     * @param x X position in world
     * @param y Y position in world
     * @return New Tank Herbivore creature
     */
    Creature createTankHerbivore(int x, int y) const;
    
    /**
     * @brief Create Armored Grazer - Scaled with tail defense (Ankylosaurus-like)
     * @param x X position in world
     * @param y Y position in world
     * @return New Armored Grazer creature
     */
    Creature createArmoredGrazer(int x, int y) const;
    
    /**
     * @brief Create Fleet Runner (Herbocursus) - Speed-based escape
     * @param x X position in world
     * @param y Y position in world
     * @return New Fleet Runner creature
     */
    Creature createFleetRunner(int x, int y) const;
    
    /**
     * @brief Create Spiky Defender (Spinosus) - Counter-attack spines
     * @param x X position in world
     * @param y Y position in world
     * @return New Spiky Defender creature
     */
    Creature createSpikyDefender(int x, int y) const;
    
    /**
     * @brief Create Canopy Forager - Fruit-eating arboreal specialist
     * @param x X position in world
     * @param y Y position in world
     * @return New Canopy Forager creature
     */
    Creature createCanopyForager(int x, int y) const;
    
    /**
     * @brief Create Carrion Stalker (Necrophagus) - Corpse-feeders
     * @param x X position in world
     * @param y Y position in world
     * @return New Carrion Stalker creature
     */
    Creature createCarrionStalker(int x, int y) const;
    
    /**
     * @brief Create Omnivore Generalist (Omniflexus) - Adaptable
     * @param x X position in world
     * @param y Y position in world
     * @return New Omnivore Generalist creature
     */
    Creature createOmnivoreGeneralist(int x, int y) const;
    
    // ========================================================================
    // Creature Creation - Category-based
    // ========================================================================
    
    /**
     * @brief Create random predator (any predator archetype)
     * @param x X position in world
     * @param y Y position in world
     * @return New predator creature
     */
    Creature createPredator(int x, int y) const;
    
    /**
     * @brief Create random herbivore (any herbivore archetype)
     * @param x X position in world
     * @param y Y position in world
     * @return New herbivore creature
     */
    Creature createHerbivore(int x, int y) const;
    
    /**
     * @brief Create random opportunist (carrion stalker or omnivore)
     * @param x X position in world
     * @param y Y position in world
     * @return New opportunist creature
     */
    Creature createOpportunist(int x, int y) const;
    
    // ========================================================================
    // Ecosystem Mix Creation
    // ========================================================================
    
    /**
     * @brief Create a balanced ecosystem mix of creatures
     * @param count Total number of creatures to create
     * @param worldWidth World width for random positioning
     * @param worldHeight World height for random positioning
     * @return Vector of balanced creature mix
     *
     * Distribution: 60% herbivores, 25% predators, 15% opportunists
     */
    std::vector<Creature> createEcosystemMix(int count, int worldWidth, int worldHeight) const;
    
    // ========================================================================
    // Template Management
    // ========================================================================
    
    /**
     * @brief Register an archetype template
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
     * @brief Register all 11 archetype templates
     *
     * Registers: apex_predator, pack_hunter, ambush_predator, pursuit_hunter,
     *            tank_herbivore, armored_grazer, fleet_runner, spiky_defender,
     *            canopy_forager, carrion_stalker, omnivore_generalist
     */
    void registerDefaultTemplates();
    
    // ========================================================================
    // Pre-built Archetype Templates - Predators
    // ========================================================================
    
    /**
     * @brief Create Apex Predator template (Carnotitan)
     * @return Archetype template for apex predators
     *
     * Traits: Large, powerful, territorial dominant
     * - High: teeth_sharpness, teeth_size, hide_thickness, combat_aggression
     * - Medium-High: max_size, locomotion, territorial_aggression
     * - Low: retreat_threshold
     */
    static CreatureTemplate createApexPredatorTemplate();
    
    /**
     * @brief Create Pack Hunter template (Carnosocialis)
     * @return Archetype template for pack hunters
     *
     * Traits: Small, fast, coordinated
     * - High: pack_coordination, locomotion, scent_acuity
     * - Medium: teeth_sharpness, combat_aggression
     * - Low: max_size
     */
    static CreatureTemplate createPackHunterTemplate();
    
    /**
     * @brief Create Ambush Predator template (Insidiatitan)
     * @return Archetype template for ambush predators
     *
     * Traits: Patient, high burst damage
     * - High: teeth_sharpness, teeth_size, scent_masking
     * - Medium-High: max_size, hide_thickness, scale_coverage
     * - Low: locomotion, combat_aggression (patient)
     */
    static CreatureTemplate createAmbushPredatorTemplate();
    
    /**
     * @brief Create Pursuit Hunter template (Velocipraeda)
     * @return Archetype template for pursuit hunters
     *
     * Traits: Fastest predator
     * - High: locomotion (highest), claw_sharpness, claw_length
     * - Medium: max_size, combat_aggression
     * - Medium-High: retreat_threshold (knows when to quit)
     */
    static CreatureTemplate createPursuitHunterTemplate();
    
    // ========================================================================
    // Pre-built Archetype Templates - Herbivores
    // ========================================================================
    
    /**
     * @brief Create Tank Herbivore template (Herbotitan)
     * @return Archetype template for tank herbivores
     *
     * Traits: Large armored, horn defense
     * - High: max_size, hide_thickness, horn_length, horn_pointiness
     * - Medium: fat_layer_thickness, plant_digestion
     * - Low: locomotion, combat_aggression, retreat_threshold (stands ground)
     */
    static CreatureTemplate createTankHerbivoreTemplate();
    
    /**
     * @brief Create Armored Grazer template (Ankylosaurus-like)
     * @return Archetype template for armored grazers
     *
     * Traits: Scales + tail club
     * - High: scale_coverage, tail_mass, fat_layer_thickness
     * - Medium: max_size, tail_length
     * - Low: locomotion, combat_aggression, retreat_threshold (never runs)
     */
    static CreatureTemplate createArmoredGrazerTemplate();
    
    /**
     * @brief Create Fleet Runner template (Herbocursus)
     * @return Archetype template for fleet runners
     *
     * Traits: Pure speed, no combat
     * - High: locomotion, sight_range, flee_threshold, retreat_threshold
     * - Medium: plant_digestion
     * - Very Low: combat_aggression, max_size
     */
    static CreatureTemplate createFleetRunnerTemplate();
    
    /**
     * @brief Create Spiky Defender template (Spinosus)
     * @return Archetype template for spiky defenders
     *
     * Traits: Porcupine-like counter-damage
     * - High: body_spines, tail_spines
     * - Medium: hide_thickness, locomotion, max_size
     * - Low-Medium: combat_aggression, retreat_threshold
     */
    static CreatureTemplate createSpikyDefenderTemplate();
    
    /**
     * @brief Create Canopy Forager template - fruit-eating arboreal specialist
     * @return Archetype template for canopy foragers
     *
     * Traits: Monkey-inspired fruit-eating specialist with excellent sensory abilities
     * - High: sweetness_preference, color_vision, scent_detection, spatial_memory
     * - Medium: plant_digestion, locomotion
     * - Low: meat_digestion, combat_aggression
     */
    static CreatureTemplate createCanopyForagerTemplate();
    
    // ========================================================================
    // Pre-built Archetype Templates - Opportunists
    // ========================================================================
    
    /**
     * @brief Create Carrion Stalker template (Necrophagus)
     * @return Archetype template for carrion stalkers
     *
     * Traits: Corpse specialist
     * - High: toxin_tolerance, olfactory_acuity
     * - Medium: meat_digestion, locomotion, retreat_threshold
     * - Low: hunt_instinct, combat_aggression
     */
    static CreatureTemplate createCarrionStalkerTemplate();
    
    /**
     * @brief Create Omnivore Generalist template (Omniflexus)
     * @return Archetype template for omnivore generalists
     *
     * Traits: Jack of all trades
     * - Medium: all stats balanced around 0.5
     * - Moderate combat capability, moderate speed, moderate defense
     */
    static CreatureTemplate createOmnivoreGeneralistTemplate();
    
    // ========================================================================
    // Legacy Diet-Niche Templates (Deprecated - kept for compatibility)
    // ========================================================================
    
    /// @deprecated Use createTankHerbivoreTemplate or createArmoredGrazerTemplate instead
    static CreatureTemplate createGrazerTemplate();
    
    /// @deprecated Use createFleetRunnerTemplate instead
    static CreatureTemplate createBrowserTemplate();
    
    /// @deprecated Use createApexPredatorTemplate or createPursuitHunterTemplate instead
    static CreatureTemplate createHunterTemplate();
    
    /// @deprecated Use createOmnivoreGeneralistTemplate instead
    static CreatureTemplate createForagerTemplate();
    
private:
    /// Gene registry for genome creation
    std::shared_ptr<GeneRegistry> registry_;
    
    /// Registered archetype templates
    std::unordered_map<std::string, CreatureTemplate> templates_;
    
    /**
     * @brief Create a new genome directly from a template
     * @param tmpl Template with gene ranges to apply
     * @return Unique pointer to a new Genome with template values applied
     *
     * This is the preferred method for genome creation - creates the genome
     * and applies template values in one step, ready for the simplified
     * Creature(x, y, genome) constructor.
     */
    std::unique_ptr<Genome> createGenomeFromTemplate(const CreatureTemplate& tmpl) const;
    
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
     * @brief Get random integer within range
     * @param min Minimum value
     * @param max Maximum value
     * @return Random integer in [min, max]
     */
    static int randomIntInRange(int min, int max);
};

} // namespace Genetics
} // namespace EcoSim
