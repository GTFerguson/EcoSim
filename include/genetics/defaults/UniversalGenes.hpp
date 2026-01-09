#pragma once

#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/Genome.hpp"
#include <unordered_map>
#include <string>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Gene categories for the unified organism genome
 */
enum class GeneCategory {
    Universal,      // All organisms (lifespan, size, metabolism)
    Mobility,       // Movement-related (locomotion, navigation, sight)
    Autotrophy,     // Self-feeding (photosynthesis, roots, water storage)
    Heterotrophy,   // Other-feeding (hunting, digestion, diet)
    Reproduction,   // Breeding (offspring, spreading, mate selection)
    Morphology,     // Physical structure (teeth, gut, hide)
    Behavior,       // Behavioral traits (caching, grooming, preferences)
    PlantDefense    // Plant defensive traits (toxins, thorns, regrowth)
};

/**
 * @brief Emergent diet type calculated from digestion genes
 * 
 * This replaces the categorical DIET_TYPE gene with an emergent classification
 * based on actual digestive capabilities.
 */
enum class DietType {
    HERBIVORE,   // High plant digestion, low meat digestion
    FRUGIVORE,   // High color vision, moderate plant, low cellulose
    OMNIVORE,    // Balanced digestion capabilities
    CARNIVORE,   // High meat digestion, low plant digestion
    NECROVORE    // High toxin tolerance, moderate meat digestion (corpse feeders)
};

/**
 * @brief Unified gene definitions for all organism types
 * 
 * This replaces separate DefaultGenes and PlantGenes with a single
 * unified registry. Any organism can potentially express any gene,
 * with expression levels determining actual capabilities.
 * 
 * Expression Strength Convention:
 * - 1.0 = Fully active gene
 * - 0.5 = Partially active
 * - 0.1 = Dormant but can be activated by evolution
 * - 0.0 = Completely suppressed
 * 
 * Phase 2.1 Update: Added 29 new genes for coevolution system:
 * - 7 Heterotrophy/Digestion genes
 * - 9 Morphology/Anatomy genes
 * - 5 Behavior genes
 * - 2 Seed Interaction genes
 * - 6 Plant Defense genes
 */
class UniversalGenes {
public:
    /**
     * @brief Register ALL genes (creature + plant + shared)
     * @param registry The registry to populate with all gene definitions
     *
     * This creates 67 genes across 12 categories.
     */
    static void registerDefaults(GeneRegistry& registry);
    
    /**
     * @brief Create genome with default expression for creature-like organisms
     * @param registry The registry containing gene definitions
     * @return A new Genome with high expression for mobility/heterotrophy,
     *         low expression for autotrophy genes
     */
    static Genome createCreatureGenome(const GeneRegistry& registry);
    
    /**
     * @brief Create genome with default expression for plant-like organisms
     * @param registry The registry containing gene definitions
     * @return A new Genome with high expression for autotrophy genes,
     *         low expression for mobility/heterotrophy
     */
    static Genome createPlantGenome(const GeneRegistry& registry);
    
    /**
     * @brief Create genome with random expression (could be anything!)
     * @param registry The registry containing gene definitions
     * @return A new Genome with randomized expression levels
     */
    static Genome createRandomGenome(const GeneRegistry& registry);
    
    /**
     * @brief Get category for a gene
     * @param gene_id The gene ID to look up
     * @return The GeneCategory for this gene
     */
    static GeneCategory getCategory(const std::string& gene_id);
    
    // ========== UNIVERSAL GENES (7) ==========
    // All organisms have these
    
    /// Maximum age of organism [100, 1000000], creep 1000
    static constexpr const char* LIFESPAN = "lifespan";
    
    /// Maximum size the organism can reach [0.5, 20.0], creep 0.5
    static constexpr const char* MAX_SIZE = "max_size";
    
    /// Base metabolism rate [0.1, 3.0], creep 0.1
    static constexpr const char* METABOLISM_RATE = "metabolism_rate";
    
    /// HSV hue for visual appearance [0.0, 360.0], creep 10.0
    static constexpr const char* COLOR_HUE = "color_hue";
    
    /// Resistance to damage/stress [0.0, 1.0], creep 0.05
    static constexpr const char* HARDINESS = "hardiness";
    
    /// Minimum survival temperature [−30.0, 30.0], creep 5.0
    static constexpr const char* TEMP_TOLERANCE_LOW = "temp_tolerance_low";
    
    /// Maximum survival temperature [10.0, 60.0], creep 5.0
    static constexpr const char* TEMP_TOLERANCE_HIGH = "temp_tolerance_high";
    
    // ========== MOBILITY GENES (5) ==========
    // Active in creatures, dormant in plants (can evolve!)
    
    /// Movement speed capability [0.0, 2.0], creep 0.1
    static constexpr const char* LOCOMOTION = "locomotion";
    
    /// Vision range [0.0, 200.0], creep 5.0
    static constexpr const char* SIGHT_RANGE = "sight_range";
    
    /// Navigation/pathfinding ability [0.0, 1.0], creep 0.05
    static constexpr const char* NAVIGATION_ABILITY = "navigation_ability";
    
    /// Distance at which fleeing is triggered [0.0, 30.0], creep 2.0
    static constexpr const char* FLEE_THRESHOLD = "flee_threshold";
    
    /// Distance at which pursuing is triggered [0.0, 30.0], creep 2.0
    static constexpr const char* PURSUE_THRESHOLD = "pursue_threshold";
    
    // ========== AUTOTROPHY GENES (5) ==========
    // Active in plants, dormant in creatures (can evolve!)
    
    /// Photosynthesis efficiency [0.0, 1.0], creep 0.05
    static constexpr const char* PHOTOSYNTHESIS = "photosynthesis";
    
    /// Root system depth [0.0, 10.0], creep 0.5
    static constexpr const char* ROOT_DEPTH = "root_depth";
    
    /// Water storage capacity [0.0, 1.0], creep 0.05
    static constexpr const char* WATER_STORAGE = "water_storage";
    
    /// Light requirements [0.0, 1.0], creep 0.05
    static constexpr const char* LIGHT_REQUIREMENT = "light_requirement";
    
    /// Water requirements [0.0, 1.0], creep 0.05
    static constexpr const char* WATER_REQUIREMENT = "water_requirement";
    
    // ========== HETEROTROPHY GENES - ORIGINAL (5) ==========
    // Active in creatures, dormant in plants (carnivorous plants can activate!)
    
    /// Hunting/predation instinct [0.0, 1.0], creep 0.05
    static constexpr const char* HUNT_INSTINCT = "hunt_instinct";
    
    /// Hunger threshold before seeking food [0.0, 10.0], creep 0.05
    static constexpr const char* HUNGER_THRESHOLD = "hunger_threshold";
    
    /// Thirst threshold before seeking water [0.0, 10.0], creep 0.05
    static constexpr const char* THIRST_THRESHOLD = "thirst_threshold";
    
    /// @deprecated Use emergent diet calculation from digestion genes instead
    /// Diet type enum (herbivore=0, omnivore=1, scavenger=2, predator=3) [0, 3]
    [[deprecated("Use calculateDietType() with digestion genes instead")]]
    static constexpr const char* DIET_TYPE = "diet_type";
    
    /// Digestive efficiency [0.1, 1.0], creep 0.05
    static constexpr const char* DIGESTIVE_EFFICIENCY = "digestive_efficiency";
    
    /// How nutritious when eaten [1.0, 100.0], creep 5.0
    static constexpr const char* NUTRIENT_VALUE = "nutrient_value";
    
    // ========== HETEROTROPHY GENES - NEW FOR COEVOLUTION (7) ==========
    // Specialized digestion genes that enable emergent diet types
    
    /// Extract calories from plant matter [0.0, 1.0], creep 0.05
    static constexpr const char* PLANT_DIGESTION_EFFICIENCY = "plant_digestion_efficiency";
    
    /// Extract calories from meat [0.0, 1.0], creep 0.05
    static constexpr const char* MEAT_DIGESTION_EFFICIENCY = "meat_digestion_efficiency";
    
    /// Digest tough plant fibers (cellulose) [0.0, 1.0], creep 0.05
    static constexpr const char* CELLULOSE_BREAKDOWN = "cellulose_breakdown";
    
    /// Resist plant defensive toxins [0.0, 1.0], creep 0.05
    static constexpr const char* TOXIN_TOLERANCE = "toxin_tolerance";
    
    /// Metabolize and detoxify plant compounds [0.0, 1.0], creep 0.05
    static constexpr const char* TOXIN_METABOLISM = "toxin_metabolism";
    
    /// Smell food sources from distance [0.0, 1.0], creep 0.05
    static constexpr const char* SCENT_DETECTION = "scent_detection";
    
    /// Detect ripe fruit via color vision [0.0, 1.0], creep 0.05
    static constexpr const char* COLOR_VISION = "color_vision";
    
    // ========== MORPHOLOGY GENES - NEW (9) ==========
    // Physical anatomy affecting diet and survival
    
    /// Relative gut length - longer for herbivores [0.3, 1.0], creep 0.05
    static constexpr const char* GUT_LENGTH = "gut_length";
    
    /// Tooth sharpness - high for carnivores [0.0, 1.0], creep 0.05
    static constexpr const char* TOOTH_SHARPNESS = "tooth_sharpness";
    
    /// Tooth grinding surface - high for herbivores [0.0, 1.0], creep 0.05
    static constexpr const char* TOOTH_GRINDING = "tooth_grinding";
    
    /// Stomach acidity - high for meat digestion [0.0, 1.0], creep 0.05
    static constexpr const char* STOMACH_ACIDITY = "stomach_acidity";
    
    /// Jaw strength for cracking hard nuts/shells [0.0, 1.0], creep 0.05
    static constexpr const char* JAW_STRENGTH = "jaw_strength";
    
    /// Jaw speed for fast snapping [0.0, 1.0], creep 0.05
    static constexpr const char* JAW_SPEED = "jaw_speed";
    
    /// Hide thickness for thorn resistance [0.0, 1.0], creep 0.05
    static constexpr const char* HIDE_THICKNESS = "hide_thickness";
    
    /// Fur density affecting burr attachment [0.0, 1.0], creep 0.05
    static constexpr const char* FUR_DENSITY = "fur_density";
    
    /// Mucus protection for gut lining [0.0, 1.0], creep 0.05
    static constexpr const char* MUCUS_PROTECTION = "mucus_protection";
    
    // ========== BEHAVIOR GENES - NEW (5) ==========
    // Behavioral traits for diet and seed dispersal
    
    /// Attraction to sweet/fruit flavors [0.0, 1.0], creep 0.05
    static constexpr const char* SWEETNESS_PREFERENCE = "sweetness_preference";
    
    // ========== OLFACTORY SYSTEM GENES (4) ==========
    // Sensory genes for scent-based navigation and communication
    
    /// Rate of general scent emission [0.0, 1.0], creep 0.05
    static constexpr const char* SCENT_PRODUCTION = "scent_production";
    
    /// How much genetic signature varies from genotype [0.0, 1.0], creep 0.05
    static constexpr const char* SCENT_SIGNATURE_VARIANCE = "scent_signature_variance";
    
    /// Ability to distinguish different scent types [0.0, 1.0], creep 0.05
    static constexpr const char* OLFACTORY_ACUITY = "olfactory_acuity";
    
    /// Reduce own scent detectability [0.0, 1.0], creep 0.05
    static constexpr const char* SCENT_MASKING = "scent_masking";
    
    /// Squirrel-like food hoarding instinct [0.0, 1.0], creep 0.05
    static constexpr const char* CACHING_INSTINCT = "caching_instinct";
    
    /// Ability to remember food cache locations [0.0, 1.0], creep 0.05
    static constexpr const char* SPATIAL_MEMORY = "spatial_memory";
    
    /// Frequency of self-grooming (burr removal) [0.0, 1.0], creep 0.05
    static constexpr const char* GROOMING_FREQUENCY = "grooming_frequency";
    
    /// Pain sensitivity/tolerance level [0.0, 1.0], creep 0.05
    static constexpr const char* PAIN_SENSITIVITY = "pain_sensitivity";
    
    /// Environmental pathfinding sensitivity [0.0, 2.0], creep 0.1
    /// 0.0 = risk-taker, ignores environmental danger in pathfinding
    /// 1.0 = balanced, moderate danger avoidance
    /// 2.0 = risk-averse, strongly avoids hostile biomes
    static constexpr const char* ENVIRONMENTAL_SENSITIVITY = "environmental_sensitivity";
    
    // ========== HEALTH/HEALING GENES (3) - Phase 1b Combat ==========
    // Health management and wound recovery
    
    /// Multiplier on base healing rate [0.0, 2.0], creep 0.1
    /// Higher values = faster wound recovery, costs more energy
    static constexpr const char* REGENERATION_RATE = "regeneration_rate";
    
    /// Reduces behavioral penalty from wounds [0.0, 1.0], creep 0.05
    /// High tolerance = less speed/behavior penalty when injured
    static constexpr const char* WOUND_TOLERANCE = "wound_tolerance";
    
    /// Reduces bleed damage over time [0.0, 1.0], creep 0.05
    /// High resistance = wounds stop bleeding faster
    static constexpr const char* BLEEDING_RESISTANCE = "bleeding_resistance";
    
    // ========== COMBAT GENES - WEAPON SHAPES (13) ==========
    // Physical weapon morphology affecting combat damage types
    
    // Teeth (3 genes)
    /// Sharp→pierce, dull→blunt [0.0, 1.0], creep 0.05
    static constexpr const char* TEETH_SHARPNESS = "teeth_sharpness";
    
    /// Adds slash damage component [0.0, 1.0], creep 0.05
    static constexpr const char* TEETH_SERRATION = "teeth_serration";
    
    /// Scales base bite damage [0.0, 1.0], creep 0.05
    static constexpr const char* TEETH_SIZE = "teeth_size";
    
    // Claws (3 genes)
    /// Reach & damage scaling [0.0, 1.0], creep 0.05
    static constexpr const char* CLAW_LENGTH = "claw_length";
    
    /// Curved→pierce, straight→slash [0.0, 1.0], creep 0.05
    static constexpr const char* CLAW_CURVATURE = "claw_curvature";
    
    /// Overall cutting ability [0.0, 1.0], creep 0.05
    static constexpr const char* CLAW_SHARPNESS = "claw_sharpness";
    
    // Horns (3 genes)
    /// Reach & charge damage [0.0, 1.0], creep 0.05
    static constexpr const char* HORN_LENGTH = "horn_length";
    
    /// Pointed→pierce, broad→blunt [0.0, 1.0], creep 0.05
    static constexpr const char* HORN_POINTINESS = "horn_pointiness";
    
    /// Narrow→gore, wide→sweep [0.0, 1.0], creep 0.05
    static constexpr const char* HORN_SPREAD = "horn_spread";
    
    // Tail (3 genes)
    /// Reach & whip damage [0.0, 1.0], creep 0.05
    static constexpr const char* TAIL_LENGTH = "tail_length";
    
    /// Heavy→blunt (club), light→slash (whip) [0.0, 1.0], creep 0.05
    static constexpr const char* TAIL_MASS = "tail_mass";
    
    /// Adds pierce component [0.0, 1.0], creep 0.05
    static constexpr const char* TAIL_SPINES = "tail_spines";
    
    // Body (1 gene - body_mass covered by MAX_SIZE)
    /// Counter-damage pierce (porcupine-like) [0.0, 1.0], creep 0.05
    static constexpr const char* BODY_SPINES = "body_spines";
    
    // ========== COMBAT GENES - DEFENSE (2 new) ==========
    // Additional defensive traits (HIDE_THICKNESS exists above)
    
    /// Reduces slashing damage [0.0, 1.0], creep 0.05
    static constexpr const char* SCALE_COVERAGE = "scale_coverage";
    
    /// Reduces blunt damage [0.0, 1.0], creep 0.05
    static constexpr const char* FAT_LAYER_THICKNESS = "fat_layer_thickness";
    
    // ========== COMBAT GENES - BEHAVIOR (4) ==========
    // Combat behavioral traits
    
    /// Willingness to initiate/continue combat [0.0, 1.0], creep 0.05
    static constexpr const char* COMBAT_AGGRESSION = "combat_aggression";
    
    /// Health % at which creature flees [0.0, 1.0], creep 0.05
    static constexpr const char* RETREAT_THRESHOLD = "retreat_threshold";
    
    /// Aggression toward same-species [0.0, 1.0], creep 0.05
    static constexpr const char* TERRITORIAL_AGGRESSION = "territorial_aggression";
    
    /// Future: pack hunting behavior [0.0, 1.0], creep 0.05
    static constexpr const char* PACK_COORDINATION = "pack_coordination";
    
    // ========== SEED INTERACTION GENES (2) ==========
    // How seeds pass through digestive system
    
    /// Hours for seed to pass through gut [0.5, 4.0], creep 0.2
    static constexpr const char* GUT_TRANSIT_TIME = "gut_transit_time";
    
    /// Rate at which seeds are ground vs passed intact [0.0, 1.0], creep 0.05
    static constexpr const char* SEED_DESTRUCTION_RATE = "seed_destruction_rate";
    
    // ========== PLANT DEFENSE GENES - NEW (6) ==========
    // Plant defensive and reproductive traits
    
    /// Chemical defense production (toxins) [0.0, 1.0], creep 0.05
    static constexpr const char* TOXIN_PRODUCTION = "toxin_production";
    
    /// Physical defense (thorns, spines) density [0.0, 1.0], creep 0.05
    static constexpr const char* THORN_DENSITY = "thorn_density";
    
    /// Recovery rate after grazing damage [0.0, 1.0], creep 0.05
    static constexpr const char* REGROWTH_RATE = "regrowth_rate";
    
    /// Rate of fruit/food object production [0.0, 1.0], creep 0.05
    static constexpr const char* FRUIT_PRODUCTION_RATE = "fruit_production_rate";
    
    /// Seed coat durability (survives digestion) [0.0, 1.0], creep 0.05
    static constexpr const char* SEED_COAT_DURABILITY = "seed_coat_durability";
    
    /// Fruit attractiveness to dispersers [0.0, 1.0], creep 0.05
    static constexpr const char* FRUIT_APPEAL = "fruit_appeal";
    
    // ========== REPRODUCTION GENES (11) ==========
    // All organisms, different expression
    
    /// Number of offspring per reproduction [1, 20], creep 1
    static constexpr const char* OFFSPRING_COUNT = "offspring_count";
    
    /// Mate threshold before seeking mate [0.0, 10.0], creep 0.05
    static constexpr const char* MATE_THRESHOLD = "mate_threshold";
    
    /// Seed/offspring spread distance [0.0, 30.0], creep 2.0
    static constexpr const char* SPREAD_DISTANCE = "spread_distance";
    
    /// Fatigue threshold before resting [0.0, 10.0], creep 0.05
    static constexpr const char* FATIGUE_THRESHOLD = "fatigue_threshold";
    
    /// Comfort increase rate [0.001, 0.02], creep 0.002
    static constexpr const char* COMFORT_INCREASE = "comfort_increase";
    
    /// Comfort decrease rate [0.001, 0.02], creep 0.002
    static constexpr const char* COMFORT_DECREASE = "comfort_decrease";
    
    // ========== SEED PROPAGATION GENES (5) - Phase 2.3 ==========
    // Physical properties that determine emergent dispersal strategy
    
    /// Mass of individual seeds (mg) - affects wind dispersal potential [0.01, 1.0], creep 0.05
    /// Low mass enables wind dispersal; high mass for gravity/animal dispersal
    static constexpr const char* SEED_MASS = "seed_mass";
    
    /// Surface area to mass ratio - aerodynamic properties [0.0, 1.0], creep 0.05
    /// High values indicate wings/parachutes for wind dispersal
    static constexpr const char* SEED_AERODYNAMICS = "seed_aerodynamics";
    
    /// Mechanical hook/barb development [0.0, 1.0], creep 0.05
    /// High values enable attachment to animal fur (burr dispersal)
    static constexpr const char* SEED_HOOK_STRENGTH = "seed_hook_strength";
    
    /// Pod tension force for ballistic dispersal [0.0, 1.0], creep 0.05
    /// High values enable explosive seed launching
    static constexpr const char* EXPLOSIVE_POD_FORCE = "explosive_pod_force";
    
    /// Vegetative runner/stolon production rate [0.0, 2.0], creep 0.1
    /// High values indicate clonal spreading strategy
    static constexpr const char* RUNNER_PRODUCTION = "runner_production";

private:
    // Helper to register genes by category
    static void registerUniversalGenes(GeneRegistry& registry);
    static void registerMobilityGenes(GeneRegistry& registry);
    static void registerAutotrophyGenes(GeneRegistry& registry);
    static void registerHeterotrophyGenes(GeneRegistry& registry);
    static void registerReproductionGenes(GeneRegistry& registry);
    static void registerCoevolutionHeterotrophyGenes(GeneRegistry& registry);
    static void registerMorphologyGenes(GeneRegistry& registry);
    static void registerBehaviorGenes(GeneRegistry& registry);
    static void registerSeedInteractionGenes(GeneRegistry& registry);
    static void registerPlantDefenseGenes(GeneRegistry& registry);
    static void registerSeedPropagationGenes(GeneRegistry& registry);  // Phase 2.3
    static void registerOlfactoryGenes(GeneRegistry& registry);        // Phase 1: Sensory System
    static void registerHealthHealingGenes(GeneRegistry& registry);    // Phase 1b: Combat
    static void registerWeaponShapeGenes(GeneRegistry& registry);      // Phase 1c: Combat Weapons
    static void registerCombatDefenseGenes(GeneRegistry& registry);    // Phase 1c: Combat Defense
    static void registerCombatBehaviorGenes(GeneRegistry& registry);   // Phase 1c: Combat Behavior
    
    // Category lookup (lazy initialized)
    static std::unordered_map<std::string, GeneCategory> s_geneCategories;
    static bool s_categoriesInitialized;
    static void initializeCategories();
};

} // namespace Genetics
} // namespace EcoSim
