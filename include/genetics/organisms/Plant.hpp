#pragma once

#include "genetics/organisms/Organism.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/EnergyBudget.hpp"
#include "rendering/RenderTypes.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <array>

namespace EcoSim {
namespace Genetics {

/// Emergent seed dispersal strategy determined by physical properties
enum class DispersalStrategy {
    GRAVITY,
    WIND,
    ANIMAL_FRUIT,
    ANIMAL_BURR,
    EXPLOSIVE,
    VEGETATIVE
};

/**
 * @brief Cache for plant growth-related gene values
 *
 * Plants perform 7+ gene lookups per tick in grow(). Since genes don't change
 * after plant creation, we cache these values once at first use to avoid
 * repeated genome traversal. Values are computed lazily on first access.
 */
struct PlantGrowthCache {
    float maxSize = 1.0f;
    float growthRate = 0.1f;
    float lightNeed = 0.5f;
    float waterNeed = 0.5f;
    float tempToleranceLow = 10.0f;
    float tempToleranceHigh = 30.0f;
    float waterStorage = 0.0f;
    bool isComputed = false;
};

/**
 * @brief Plant organism that extends the Organism base class
 * 
 * Plants use the same genome system as creatures but with plant-specific genes.
 * They inherit shared functionality (position, lifecycle, growth, genetics) from
 * Organism and add plant-specific behaviors like photosynthesis-based growth,
 * seed dispersal, and defense mechanisms.
 * 
 * Key differences from creatures:
 * - Plants don't move (position is fixed after creation)
 * - Plants grow in size over time based on growth_rate gene
 * - Plants reproduce by spreading seeds rather than mating
 * - Plants respond to light and water rather than food/thirst
 * 
 * Usage:
 * @code
 *   GeneRegistry plantRegistry;
 *   PlantGenes::registerDefaults(plantRegistry);
 *
 *   Plant plant(10, 20, plantRegistry);  // Create at position (10, 20)
 *   plant.update(environmentState);       // Update each tick
 *
 *   if (plant.canReproduce()) {
 *       auto offspring = plant.reproduce();  // Asexual reproduction
 *   }
 * @endcode
 */
class Plant : public Organism {
public:
    /**
     * @brief Construct a plant with a random genome
     * @param x X position in the world
     * @param y Y position in the world
     * @param registry Gene registry containing plant gene definitions
     */
    Plant(int x, int y, const GeneRegistry& registry);
    
    /**
     * @brief Construct a plant with a specific genome
     * @param x X position in the world
     * @param y Y position in the world
     * @param genome Pre-existing genome (e.g., from parent)
     * @param registry Gene registry for phenotype expression
     */
    Plant(int x, int y, const Genome& genome, const GeneRegistry& registry);
    
    /**
     * @brief Destructor
     */
    ~Plant() override = default;
    
    /**
     * @brief Copy constructor - properly rebinds phenotype pointers
     * @param other Plant to copy from
     *
     * After copying, phenotype_.genome_ points to THIS plant's genome,
     * not the source plant's genome.
     */
    Plant(const Plant& other);
    
    /**
     * @brief Move constructor - properly rebinds phenotype pointers
     * @param other Plant to move from
     *
     * After moving, phenotype_.genome_ points to THIS plant's genome,
     * not the moved-from plant's genome.
     */
    Plant(Plant&& other) noexcept;
    
    /**
     * @brief Copy assignment - properly rebinds phenotype pointers
     * @param other Plant to copy from
     * @return Reference to this
     */
    Plant& operator=(const Plant& other);
    
    /**
     * @brief Move assignment - properly rebinds phenotype pointers
     * @param other Plant to move from
     * @return Reference to this
     */
    Plant& operator=(Plant&& other) noexcept;
    
    // ========================================================================
    // IPositionable overrides - Plants are fixed at tile center
    // ========================================================================
    
    /**
     * @brief Get world X coordinate (float precision)
     * @return X position in world coordinates (tile center)
     *
     * Plants are positioned at tile center (x + 0.5). This is temporary until
     * plants are overhauled to support variable sizes (multiple small plants
     * per tile or large plants spanning multiple tiles).
     */
    float getWorldX() const override { return static_cast<float>(x_) + 0.5f; }
    
    /**
     * @brief Get world Y coordinate (float precision)
     * @return Y position in world coordinates (tile center)
     *
     * Plants are positioned at tile center (y + 0.5). This is temporary until
     * plants are overhauled to support variable sizes.
     */
    float getWorldY() const override { return static_cast<float>(y_) + 0.5f; }
    
    /**
     * @brief Set world position (plants typically don't move, but seeds can be placed)
     * @param x New X position
     * @param y New Y position
     *
     * For plants, this truncates to integer tile position.
     */
    void setWorldPosition(float x, float y) override {
        x_ = static_cast<int>(x);
        y_ = static_cast<int>(y);
    }
    
    // ========================================================================
    // ILifecycle overrides - Gene-dependent lifespan
    // ========================================================================
    
    /**
     * @brief Get maximum lifespan based on genome
     * @return Maximum age before natural death
     */
    unsigned int getMaxLifespan() const override;
    
    // ========================================================================
    // IReproducible overrides - Asexual seed-based reproduction
    // ========================================================================
    
    /**
     * @brief Check if plant can reproduce
     * @return true if plant is mature and can spread seeds
     */
    bool canReproduce() const override;
    
    /**
     * @brief Get reproductive urge (normalized 0.0-1.0)
     * @return Always 1.0 for plants when mature, 0.0 otherwise
     *
     * Plants don't have urges - they reproduce when conditions are met.
     */
    float getReproductiveUrge() const override;
    
    /**
     * @brief Get energy cost of reproduction
     * @return Energy cost based on seed production genes
     */
    float getReproductionEnergyCost() const override;
    
    /**
     * @brief Get reproduction mode
     * @return Always ASEXUAL for plants (clonal with mutation)
     */
    ReproductionMode getReproductionMode() const override;
    
    /**
     * @brief Check compatibility with another organism
     * @param other The other organism
     * @return Always false for plants (asexual reproduction)
     */
    bool isCompatibleWith(const Organism& other) const override;
    
    /**
     * @brief Reproduce to create offspring
     * @param partner Unused for asexual plant reproduction (should be nullptr)
     * @return Offspring as Organism pointer
     */
    std::unique_ptr<Organism> reproduce(
        const Organism* partner = nullptr) override;
    
    // ========================================================================
    // IGenetic overrides
    // ========================================================================
    
    /**
     * @brief Recalculate expressed traits from genome
     */
    void updatePhenotype() override;
    
    // ========================================================================
    // Organism overrides - Growth system
    // ========================================================================
    
    /**
     * @brief Get maximum size from phenotype
     * @return Maximum size the plant can reach
     */
    float getMaxSize() const override;
    
    /**
     * @brief Perform growth for this tick (environment-dependent)
     * 
     * Plant growth depends on light, water, and temperature.
     * This overload uses default environment - prefer update(env) instead.
     */
    void grow() override;
    
    // ========================================================================
    // Plant-specific methods
    // ========================================================================
    
    /**
     * @brief Get growth rate from phenotype
     * @return Growth rate (higher = faster growing)
     */
    float getGrowthRate() const;
    
    /**
     * @brief Get nutrient value when eaten
     * @return Energy provided to herbivores
     */
    float getNutrientValue() const;
    
    /**
     * @brief Get water requirement
     * @return Water need (0.0 = drought resistant, 1.0 = water loving)
     */
    float getWaterNeed() const;
    
    /**
     * @brief Get light requirement
     * @return Light need (0.0 = shade tolerant, 1.0 = full sun)
     */
    float getLightNeed() const;
    
    /**
     * @brief Get hardiness (damage resistance)
     * @return Hardiness value (0.0 to 1.0)
     */
    float getHardiness() const;
    
    /**
     * @brief Get color hue for visualization
     * @return HSV hue value (0-360)
     */
    float getColorHue() const;
    
    /**
     * @brief Check if plant is mature enough to spread seeds
     * @return true if plant can produce offspring
     */
    bool canSpreadSeeds() const;
    
    /**
     * @brief Check if current temperature is within tolerance
     * @param temperature Current temperature in Celsius
     * @return true if plant can survive at this temperature
     */
    bool canSurviveTemperature(float temperature) const;
    
    /**
     * @brief Update plant state for one tick
     * @param env Current environment state
     * 
     * Handles:
     * - Growth based on growth_rate and environment
     * - Age increment
     * - Death check (age or environment)
     */
    void update(const EnvironmentState& env);
    
    /**
     * @brief Apply damage to the plant
     * @param amount Damage amount (reduced by hardiness)
     * 
     * Plants can take damage from herbivores or harsh environment.
     * Damage is reduced by the hardiness gene.
     */
    void takeDamage(float amount);
    
    /**
     * @brief Get number of seeds to produce this cycle
     * @return Number of seeds based on seed_production gene and maturity
     */
    int getSeedCount() const;
    
    /**
     * @brief Get seed spread distance
     * @return Maximum distance seeds can travel from parent
     */
    float getSpreadDistance() const;
    
    // ========================================================================
    // Defense System
    // ========================================================================
    
    /**
     * @brief Get toxicity level from TOXIN_PRODUCTION gene
     * @return Toxicity level (0.0 = non-toxic, 1.0 = highly toxic)
     *
     * Higher toxicity damages herbivores that eat this plant,
     * but requires energy to produce and maintain.
     */
    float getToxicity() const;
    
    /**
     * @brief Get thorn damage from THORN_DENSITY gene
     * @return Thorn damage (0.0 = no thorns, 1.0 = very thorny)
     *
     * Higher thorn density damages herbivores on contact,
     * providing physical defense against grazing.
     */
    float getThornDamage() const;
    
    /**
     * @brief Get regrowth rate from REGROWTH_RATE gene
     * @return Regrowth rate (0.0 = slow, 1.0 = very fast)
     *
     * Higher regrowth allows recovery after grazing damage.
     */
    float getRegrowthRate() const;
    
    /**
     * @brief Check if plant can regenerate health
     * @return true if plant has enough energy and regrowth ability
     */
    bool canRegenerate() const;
    
    /**
     * @brief Regenerate health based on regrowth rate
     *
     * Uses energy to restore health. Rate depends on regrowth_rate gene.
     */
    void regenerate();
    
    // ========================================================================
    // Fruit Production - Direct Plant Feeding
    // ========================================================================
    
    /**
     * @brief Check if plant can produce fruit
     * @return true if mature, has energy, and fruit timer ready
     *
     * Requires:
     * - Plant maturity (age > 25% of lifespan)
     * - Sufficient energy (from EnergyBudget)
     * - Fruit timer cooldown complete
     *
     * Note: Creatures feed directly on plants via FeedingInteraction,
     * which handles nutrient extraction and defense bypass.
     */
    bool canProduceFruit() const;
    
    /**
     * @brief Check if plant can spread vegetatively (runners/stolons)
     * @return true if mature and has high runner production
     *
     * Vegetative reproduction is an alternative to fruit-based seed dispersal.
     * Grass and similar plants use this strategy instead of fruiting.
     * Requires:
     * - Plant maturity (50% size, 10% age)
     * - High RUNNER_PRODUCTION gene value (> 0.5)
     * - Cooldown timer ready
     */
    bool canSpreadVegetatively() const;
    
    /**
     * @brief Reset the fruit/dispersal timer after a reproduction attempt
     *
     * Called after any dispersal attempt (successful or not) to enforce
     * the cooldown period before the next attempt.
     */
    void resetFruitTimer() { fruitTimer_ = 0; }
    
    /**
     * @brief Get runner production rate from RUNNER_PRODUCTION gene
     * @return Production rate (0.0 = none, 1.5+ = aggressive spread)
     */
    float getRunnerProduction() const;
    
    /**
     * @brief Get fruit production rate from FRUIT_PRODUCTION_RATE gene
     * @return Production rate (0.0 = rarely, 1.0 = frequently)
     */
    float getFruitProductionRate() const;
    
    /**
     * @brief Get fruit appeal from FRUIT_APPEAL gene
     * @return Appeal to dispersers (0.0 = unattractive, 1.0 = very attractive)
     *
     * Higher appeal attracts more seed dispersers (frugivores).
     */
    float getFruitAppeal() const;
    
    // ========================================================================
    // Seed Properties
    // ========================================================================
    
    /**
     * @brief Get seed mass from SEED_MASS gene
     * @return Seed mass in mg (0.01 = tiny, 1.0 = large)
     *
     * Affects dispersal: low mass enables wind dispersal,
     * high mass improves germination success.
     */
    float getSeedMass() const;
    
    /**
     * @brief Get seed aerodynamics from SEED_AERODYNAMICS gene
     * @return Aerodynamic rating (0.0 = round/heavy, 1.0 = winged/fluffy)
     *
     * High aerodynamics with low mass enables wind dispersal.
     */
    float getSeedAerodynamics() const;
    
    /**
     * @brief Get seed hook strength from SEED_HOOK_STRENGTH gene
     * @return Hook strength (0.0 = smooth, 1.0 = strongly hooked)
     *
     * High hook strength enables burr dispersal via animal fur.
     */
    float getSeedHookStrength() const;
    
    /**
     * @brief Get seed coat durability from SEED_COAT_DURABILITY gene
     * @return Durability (0.0 = fragile, 1.0 = very durable)
     *
     * High durability allows seeds to survive digestive systems.
     */
    float getSeedCoatDurability() const;
    
    /**
     * @brief Get explosive pod force from EXPLOSIVE_POD_FORCE gene
     * @return Force (0.0 = no explosive mechanism, 1.0 = strong ballistic launch)
     *
     * High values enable mechanical seed launching (ballistic dispersal).
     */
    float getExplosivePodForce() const;
    
    // ========================================================================
    // Emergent Dispersal Strategy
    // ========================================================================
    
    /**
     * @brief Determine primary dispersal strategy from physical properties
     * @return The emergent dispersal strategy based on seed genes
     *
     * Strategy is determined by thresholds on physical properties:
     * - High runner_production -> VEGETATIVE
     * - High explosive_pod_force -> EXPLOSIVE
     * - Low seed_mass + high aerodynamics -> WIND
     * - High seed_hook_strength -> ANIMAL_BURR
     * - High fruit_appeal + durable seeds -> ANIMAL_FRUIT
     * - Default -> GRAVITY
     *
     * This creates emergent behavior from continuous traits rather
     * than categorical gene values.
     */
    DispersalStrategy getPrimaryDispersalStrategy() const;
    
    // ========================================================================
    // Energy Budget Integration
    // ========================================================================
    
    /**
     * @brief Set the plant's energy state from EnergyBudget calculations
     * @param state Energy state computed by EnergyBudget system
     */
    void setEnergyState(const EnergyState& state);
    
    /**
     * @brief Get the plant's current energy state
     * @return Reference to internal energy state
     */
    EnergyState& getEnergyState();
    
    /**
     * @brief Get the plant's current energy state (const)
     * @return Const reference to internal energy state
     */
    const EnergyState& getEnergyState() const;
    
    // ========================================================================
    // Rendering Support
    // ========================================================================
    
    /**
     * @brief Get entity type for rendering
     * @return EntityType appropriate for this plant species
     *
     * Returns based on plant characteristics (to be set by PlantFactory).
     */
    EntityType getEntityType() const;
    
    /**
     * @brief Set entity type (used by PlantFactory for species templates)
     * @param type The entity type for this plant
     */
    void setEntityType(EntityType type);
    
    /**
     * @brief Get character for ASCII/text rendering
     * @return Character representing this plant type
     *
     * Examples: 'B' for berry bush, 'T' for tree, '"' for grass
     */
    char getRenderCharacter() const;
    
    /**
     * @brief Get render color based on COLOR_HUE gene
     * @return RGB color for rendering
     */
    Color getRenderColor() const;
    
    // ========================================================================
    // Serialization
    // ========================================================================
    
    /**
     * @brief Serialize plant state to string
     * @return String representation for saving
     */
    std::string toString() const;
    
    /**
     * @brief Create plant from serialized string
     * @param data Serialized plant data
     * @param registry Gene registry for phenotype
     * @return Reconstructed plant
     */
    static std::unique_ptr<Plant> fromString(const std::string& data,
                                              const GeneRegistry& registry);
    
    /**
     * @brief Serialize plant state to JSON
     * @return JSON object containing all serializable plant state
     */
    nlohmann::json toJson() const;
    
    /**
     * @brief Create plant from JSON data
     * @param j JSON object containing plant state
     * @param registry Gene registry for phenotype expression
     * @return Reconstructed plant
     */
    static Plant fromJson(const nlohmann::json& j, const GeneRegistry& registry);
    
    /**
     * @brief Convert DispersalStrategy enum to string
     * @param strategy The dispersal strategy to convert
     * @return String representation of the strategy
     */
    static std::string dispersalStrategyToString(DispersalStrategy strategy);
    
    /**
     * @brief Convert string to DispersalStrategy enum
     * @param str String representation of dispersal strategy
     * @return The corresponding DispersalStrategy enum value
     */
    static DispersalStrategy stringToDispersalStrategy(const std::string& str);
    
    // ========================================================================
    // Scent System
    // ========================================================================
    
    /**
     * @brief Generate this plant's scent signature
     *
     * Creates an 8-element scent signature encoding plant traits:
     * [0] FRUIT_APPEAL, [1] TOXIN_PRODUCTION, [2] THORN_DENSITY,
     * [3] HARDINESS, [4] PLANT_DIGESTION (inverted as "plant-ness"),
     * [5-7] Reserved for plant ID encoding
     *
     * @return ScentSignature representing this plant's scent profile
     */
    std::array<float, 8> getScentSignature() const;
    
    /**
     * @brief Get scent production rate from genes
     * @return Production rate (0.0-1.0, higher = stronger scent)
     */
    float getScentProductionRate() const;

protected:
    // Plant-specific energy budget integration
    EnergyState energyState_;
    
    // Fruit production timing
    unsigned int fruitTimer_ = 0;
    
    // Rendering
    EntityType entityType_ = EntityType::PLANT_GENERIC;
    
    // Static ID generator for unique plant IDs
    static unsigned int nextId_;

private:
    // Cached growth-related gene values (computed once, used every tick)
    mutable PlantGrowthCache growthCache_;
    
    /**
     * @brief Ensure growth cache is computed (lazy initialization)
     *
     * Called at the start of grow() to populate cached gene values.
     * No-op after first call since genes don't change after creation.
     */
    void ensureGrowthCacheComputed() const;

protected:
    // Internal helpers
    void checkDeathConditions(const EnvironmentState& env);
    void grow(const EnvironmentState& env);
    
    /**
     * @brief Read gene value directly from genome (bypasses phenotype traits)
     * @param geneId The gene identifier to read
     * @param defaultValue Value to return if gene not found
     * @return Gene's expressed float value or default
     *
     * This helper is more reliable than phenotype trait lookup because
     * gene IDs don't always match trait names (effect bindings determine trait names).
     */
    float getGeneValueFromGenome(const char* geneId, float defaultValue) const;
};

} // namespace Genetics
} // namespace EcoSim
