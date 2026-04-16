#pragma once

// Forward declaration for global-namespace World class
class World;

#include "genetics/interfaces/IPositionable.hpp"
#include "genetics/interfaces/ILifecycle.hpp"
#include "genetics/interfaces/IGenetic.hpp"
#include "genetics/interfaces/IReproducible.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/components/MobilityComponent.hpp"
#include "genetics/components/HeterotrophyComponent.hpp"
#include "genetics/components/AutotrophyComponent.hpp"
#include "genetics/components/ReproductionComponent.hpp"
#include "genetics/components/CombatComponent.hpp"
#include "genetics/components/ThermalComponent.hpp"
#include "genetics/components/IdentityComponent.hpp"
#include "genetics/core/MotivationAction.hpp"
#include "genetics/behaviors/BehaviorController.hpp"
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace EcoSim {
class ScentLayer;
}

namespace EcoSim {
namespace Genetics {

class GeneRegistry;
class FeedingInteraction;
class SeedDispersal;
class PerceptionSystem;
class CombatInteraction;
class ArchetypeIdentity;
class BiomeAdaptation;
class Plant;
struct FeedingResult;
struct DispersalEvent;
struct BehaviorContext;
struct BehaviorResult;
enum class DietType;

/**
 * @brief Base class for all living organisms (Plant, Creature).
 * 
 * Consolidates shared functionality for position, lifecycle, growth, and genetics.
 * This abstract class implements the common interfaces and provides protected
 * members that subclasses share, reducing code duplication between Plant and Creature.
 *
 * Implements interfaces:
 * - IPositionable - Position in world (tile coords implemented, world coords virtual)
 * - ILifecycle - Age, lifespan, alive state
 * - IGenetic - Genome and phenotype access
 * - IReproducible - Reproduction capability (all methods pure virtual)
 *
 * Design decisions:
 * - Tile coordinates (x_, y_) stored in base, world coords are subclass-specific
 * - Health system is shared but max health calculation is gene-dependent
 * - Growth state is shared but growth rate/max size are gene-dependent
 * - Copy disabled (complex ownership), move allowed
 */
class Organism : public IPositionable, 
                 public ILifecycle, 
                 public IGenetic, 
                 public IReproducible {
public:
    /**
     * @brief Construct organism at position with genome
     * @param x Tile X coordinate
     * @param y Tile Y coordinate
     * @param genome Genetic data for this organism
     * @param registry Gene registry for phenotype expression
     */
    Organism(int x, int y, Genome genome, const GeneRegistry& registry);
    
    virtual ~Organism() = default;
    
    // Disable copy (complex ownership with phenotype->genome pointer)
    Organism(const Organism&) = delete;
    Organism& operator=(const Organism&) = delete;
    
    // Allow move
    Organism(Organism&&) noexcept;
    Organism& operator=(Organism&&) noexcept;

    // ========================================================================
    // IPositionable - Tile coordinates implemented, world coords pure virtual
    // ========================================================================
    
    int getX() const override { return x_; }
    int getY() const override { return y_; }
    
    /**
     * @brief Set tile position directly
     * @param x New tile X coordinate
     * @param y New tile Y coordinate
     */
    void setPosition(int x, int y) { x_ = x; y_ = y; }
    
    // World coordinates depend on subclass (Plant: tile center, Creature: precise)
    float getWorldX() const override = 0;
    float getWorldY() const override = 0;
    void setWorldPosition(float x, float y) override;
    
    // ========================================================================
    // ILifecycle - Mostly implemented, max lifespan is gene-dependent
    // ========================================================================
    
    /**
     * @brief Age the organism
     * @param ticks Number of ticks to age (default 1)
     */
    void age(unsigned int ticks = 1) override;
    
    bool isAlive() const override { return alive_; }
    unsigned int getAge() const override { return age_; }
    float getAgeNormalized() const override;
    unsigned int getMaxLifespan() const override;  // Default: phenotype LIFESPAN
    
    // ========================================================================
    // IGenetic - Fully implemented
    // ========================================================================
    
    const Genome& getGenome() const override { return genome_; }
    Genome& getGenomeMutable() override { return genome_; }
    const Phenotype& getPhenotype() const override { return phenotype_; }
    void updatePhenotype() override;
    
    // ========================================================================
    // IReproducible — pipeline lives here. Subclasses only override the
    // factory hook (makeOffspring) that constructs the concrete subtype.
    // ========================================================================

    bool canReproduce() const override;
    float getReproductiveUrge() const override;
    float getReproductionEnergyCost() const override;
    ReproductionMode getReproductionMode() const override;

    // Default compatibility: same reproduction mode + non-trivial genetic
    // similarity. Subclasses can still override if they have stricter rules.
    bool isCompatibleWith(const Organism& other) const override;

    // Final dispatch: routes to sexual (with partner) or asexual (without)
    // breeding. Both share the body in this class; only construction is
    // delegated to the subclass via makeOffspring.
    std::unique_ptr<Organism> reproduce(
        const Organism* partner = nullptr) override final;

protected:
    // Subclass-specific factory hook: construct a fresh organism of this
    // concrete type at (x, y) carrying the supplied genome. Called by
    // sexualBreed/asexualBreed once the genome has been crossed and
    // mutated. The only step in reproduction that genuinely depends on
    // the concrete subclass.
    virtual std::unique_ptr<Organism> makeOffspring(
        std::unique_ptr<Genome> offspringGenome, int x, int y) = 0;

    // Shared sexual breeding pipeline: charges parental cost, shares
    // resources where applicable, resets mate drive, performs genome
    // crossover + mutation, delegates construction to makeOffspring.
    std::unique_ptr<Organism> sexualBreed(Organism& mate);

    // Shared asexual breeding pipeline: copies + mutates the genome,
    // picks an offset position based on spread distance, delegates
    // construction to makeOffspring.
    std::unique_ptr<Organism> asexualBreed();

    // Distance an asexual offspring may be placed from the parent.
    // Defaults to 0 (in-place reproduction). Plants override to use
    // their seed-spread gene.
    virtual float getOffspringSpreadDistance() const { return 0.0f; }

public:
    
    // ========================================================================
    // Growth System (shared by Plant and Creature)
    // ========================================================================
    
    float getCurrentSize() const { return currentSize_; }
    virtual float getMaxSize() const = 0;  // Gene-dependent
    bool isMature() const { return mature_; }
    
    /**
     * @brief Get size as ratio of max size
     * @return 0.0 to 1.0 representing growth progress
     */
    float getSizeRatio() const { return maxSize_ > 0.0f ? currentSize_ / maxSize_ : 0.0f; }
    
    /**
     * @brief Perform growth for this tick
     * Subclasses implement with their specific growth logic
     */
    virtual void grow();
    
    // ========================================================================
    // Identity
    // ========================================================================

    int getId() const { return id_; }
    unsigned int getUnsignedId() const { return static_cast<unsigned int>(id_); }
    
    // ========================================================================
    // Health System (shared)
    // ========================================================================
    
    float getHealth() const { return health_; }
    
    /**
     * @brief Get maximum health based on genes
     * @return Max health value (default 1.0 if no gene found)
     */
    float getMaxHealth() const;
    
    /**
     * @brief Set health value (clamped to 0-maxHealth)
     * @param health New health value
     */
    void setHealth(float health);
    
    /**
     * @brief Apply damage to organism
     * @param amount Damage amount (may be reduced by subclass defenses)
     */
    virtual void damage(float amount);
    
    /**
     * @brief Heal the organism
     * @param amount Amount to heal (clamped to maxHealth)
     */
    void heal(float amount);

public:
    /**
     * @brief Set alive state to false
     * Called when death conditions are met (age, health, environment)
     */
    void die() { alive_ = false; }
    
    /**
     * @brief Increment age counter
     */
    void incrementAge() { ++age_; }
    
    /**
     * @brief Set maturity state
     * @param mature New maturity state
     */
    void setMature(bool mature) { mature_ = mature; }
    
    /**
     * @brief Update current size
     * @param size New size value
     */
    void setCurrentSize(float size) { currentSize_ = size; }

    /**
     * @brief Set maximum size directly (deserialization / testing).
     */
    void setMaxSize(float size) { maxSize_ = size; }
    
    /**
     * @brief Rebind phenotype's genome pointer after move
     *
     * The Phenotype holds a pointer to the Genome for trait expression.
     * After moving, this pointer must be updated to point to THIS
     * organism's genome, not the moved-from organism's genome.
     */
    void rebindPhenotypeGenome();

    // ========================================================================
    // Optional components (attached at construction based on gene expression)
    //
    // A component is present only when the organism's genes justify it —
    // sessile photosynthesisers skip MobilityComponent, inert fruit skips
    // HeterotrophyComponent, etc. Accessors return nullptr when absent,
    // callers null-check before use.
    // ========================================================================

    MobilityComponent*     mobility()     { return mobility_.get(); }
    HeterotrophyComponent* heterotrophy() { return heterotrophy_.get(); }
    AutotrophyComponent*   autotrophy()   { return autotrophy_.get(); }
    ReproductionComponent* reproduction() { return reproduction_.get(); }
    CombatComponent*       combat()       { return combat_.get(); }
    ThermalComponent*      thermal()      { return thermal_.get(); }
    IdentityComponent*     identity()     { return identity_.get(); }

    const MobilityComponent*     mobility()     const { return mobility_.get(); }
    const HeterotrophyComponent* heterotrophy() const { return heterotrophy_.get(); }
    const AutotrophyComponent*   autotrophy()   const { return autotrophy_.get(); }
    const ReproductionComponent* reproduction() const { return reproduction_.get(); }
    const CombatComponent*       combat()       const { return combat_.get(); }
    const ThermalComponent*      thermal()      const { return thermal_.get(); }
    const IdentityComponent*     identity()     const { return identity_.get(); }

    void attachMobility(std::unique_ptr<MobilityComponent> c)         { mobility_     = std::move(c); }
    void attachHeterotrophy(std::unique_ptr<HeterotrophyComponent> c) { heterotrophy_ = std::move(c); }
    void attachAutotrophy(std::unique_ptr<AutotrophyComponent> c)     { autotrophy_   = std::move(c); }
    void attachReproduction(std::unique_ptr<ReproductionComponent> c) { reproduction_ = std::move(c); }
    void attachCombat(std::unique_ptr<CombatComponent> c)             { combat_       = std::move(c); }
    void attachThermal(std::unique_ptr<ThermalComponent> c)           { thermal_      = std::move(c); }
    void attachIdentity(std::unique_ptr<IdentityComponent> c)         { identity_     = std::move(c); }

    // ========================================================================
    // Behavior controller (shared between creatures and plants)
    // ========================================================================

    BehaviorController* getOrganismBehaviorController()             { return organismBehaviorController_.get(); }
    const BehaviorController* getOrganismBehaviorController() const { return organismBehaviorController_.get(); }
    void setOrganismBehaviorController(std::unique_ptr<BehaviorController> bc) {
        organismBehaviorController_ = std::move(bc);
    }
    bool hasOrganismBehaviorController() const { return organismBehaviorController_ != nullptr; }

    // Pending offspring — MatingBehavior stashes new offspring here when
    // reproduction succeeds. The sim main loop drains this field after
    // each tick and moves offspring into the creature vector.
    std::unique_ptr<Organism> takePendingOffspring() {
        return std::move(pendingOffspring_);
    }
    bool hasPendingOffspring() const { return pendingOffspring_ != nullptr; }
    void setPendingOffspring(std::unique_ptr<Organism> offspring) {
        pendingOffspring_ = std::move(offspring);
    }

    // Position (tile coordinates)
    int x_;
    int y_;

    // Lifecycle state
    unsigned int age_ = 0;
    bool alive_ = true;
    float health_;

    // Growth state
    float currentSize_;
    float maxSize_;
    bool mature_ = false;

    // Genetics
    Genome genome_;
    Phenotype phenotype_;
    const GeneRegistry* registry_;

    // Identity
    int id_;
    static int nextId_;

    // Shared services historically owned by Creature (lazy-init).
    // Declared here so they're accessible from methods that will move
    // onto Organism once the Creature/Plant class deletion completes.
    static std::shared_ptr<GeneRegistry>     s_geneRegistry;
    static std::unique_ptr<FeedingInteraction>  s_feedingInteraction;
    static std::unique_ptr<SeedDispersal>       s_seedDispersal;
    static std::unique_ptr<PerceptionSystem>    s_perceptionSystem;
    static std::unique_ptr<CombatInteraction>   s_combatInteraction;

    // Sequential creature display ID counter (moves to a factory later).
    static int nextCreatureId_;

public:
    // Static ID counter management — historical Creature statics,
    // now on Organism so they can be reached without creature.hpp.
    static void resetIdCounter(int nextId);
    static int  getNextId();

    // Simple getters and setters that delegate to components.
    // Historically lived on Creature; inherited by Creature via base.
    // Tile coordinates (derived from world float position)
    int tileX() const;
    int tileY() const;
    float getMovementSpeed() const;

    // Health / wound / healing (getMaxHealth / heal already on base above)
    float getHealthPercent() const;
    float getWoundSeverity() const;
    float getHealingRate() const;
    void  takeDamage(float amount);
    WoundState getWoundState() const;

    // Combat state setters/getters (delegate to CombatComponent)
    void setInCombat(bool combat);
    void setTargetId(int targetId);
    void setCombatCooldown(int cooldown);
    void setFleeing(bool fleeing);
    bool isInCombat() const;
    bool isFleeing() const;
    int  getTargetId() const;
    int  getCombatCooldown() const;

    // Gene expression query
    float getExpressedValue(const std::string& geneId) const;

    // Shared gene registry (historical Creature statics)
    static void initializeGeneRegistry();
    static GeneRegistry& getGeneRegistry();

    // Phenotype context / thermal cache refresh
    void updatePhenotypeContext(const EnvironmentState& env);
    void updateThermalCache();

    // Behavior system — initializes and ticks the BehaviorController
    void initializeBehaviorController();
    BehaviorContext buildBehaviorContext(World& world,
                                          EcoSim::ScentLayer& scentLayer,
                                          unsigned int currentTick) const;
    BehaviorResult updateWithBehaviors(BehaviorContext& ctx);

    // String / serialization helpers (delegate to CreatureSerialization)
    static Direction stringToDirection(const std::string& str);
    std::string directionToString() const;

    // Name / character generation
    char generateChar();
    std::string generateName();

    // Scent detection / signature methods (delegate to CreatureScent helpers)
    bool hasScentDetection() const;
    std::array<float, 8> computeScentSignature() const;
    void depositBreedingScent(EcoSim::ScentLayer& layer, unsigned int currentTick);
    std::optional<Direction> detectMateDirection(const EcoSim::ScentLayer& scentLayer) const;
    bool findMateScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const;
    bool findFoodScent(const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) const;
    static float calculateSignatureSimilarity(
        const std::array<float, 8>& sig1,
        const std::array<float, 8>& sig2);

    // Plant interaction (delegate to CreaturePlantInteraction helpers)
    FeedingResult eatPlant(Plant& plant);
    bool canEatPlant(const Plant& plant) const;
    float getPlantDetectionRange() const;
    void attachBurr(const Plant& plant);
    std::vector<DispersalEvent> detachBurrs();
    bool hasBurrs() const;
    std::vector<DispersalEvent> getPendingBurrDispersal() const;
    void consumeSeeds(const Plant& plant, int count, float viability);
    std::vector<DispersalEvent> processGutSeeds(int ticksElapsed);
    static void initializeInteractionSystems();

    // Core simulation helpers
    short deathCheck() const;
    float shareResource(const int& amount, float& resource);
    float shareFood(const int& amount);
    float shareWater(const int& amount);
    void  changeDirection(const int& xChange, const int& yChange);
    float calculateDistance(const int& goalX, const int& goalY) const;
    void  movementCost(const float& distance);

    // Taxonomy / classification getters (use IdentityComponent)
    std::string getScientificName() const;
    std::string getArchetypeLabel() const;
    std::string getFullLabel() const;
    const BiomeAdaptation* getBiomeAdaptation() const;
    void reclassifyBiomeAdaptation();

    // Phenotype-derived getters (delegate to phenotype_ + gene traits)
    unsigned  getLifespan   () const;
    unsigned  getSightRange () const;
    float     getTHunger    () const;
    float     getTThirst    () const;
    float     getTFatigue   () const;
    float     getTMate      () const;
    float     getComfInc    () const;
    float     getComfDec    () const;
    DietType  getDietType   () const;
    bool      ifFlocks      () const;
    unsigned  getFlee       () const;
    unsigned  getPursue     () const;

    void setAge        (unsigned age);
    void setHunger     (float hunger);
    void setThirst     (float thirst);
    void setFatigue    (float fatigue);
    void setMate       (float mate);
    void setXY         (int x, int y);
    void setX          (int x);
    void setY          (int y);
    void setMotivation (Motivation m);
    void setAction     (Action a);
    void setWorldX     (float x);
    void setWorldY     (float y);
    float     getHunger     () const;
    float     getThirst     () const;
    float     getFatigue    () const;
    float     getMate       () const;
    float     getMetabolism () const;
    unsigned  getSpeed      () const;
    Direction getDirection  () const;
    Motivation getMotivation() const;
    Action     getAction    () const;

    // Display state: current motivation / action for UI/rendering.
    // Derived from active behavior selection, surfaced here for cheap
    // access by panels and renderers.
    Motivation motivation_ = Motivation::Content;
    Action     action_     = Action::Idle;

    // Optional runtime-state components (nullptr until attached)
    std::unique_ptr<MobilityComponent>     mobility_;
    std::unique_ptr<HeterotrophyComponent> heterotrophy_;
    std::unique_ptr<AutotrophyComponent>   autotrophy_;
    std::unique_ptr<ReproductionComponent> reproduction_;
    std::unique_ptr<CombatComponent>       combat_;
    std::unique_ptr<ThermalComponent>      thermal_;
    std::unique_ptr<IdentityComponent>     identity_;

    // Shared behavior controller — holds IBehavior (active decisions) and
    // IPassiveTick (physiology). Named organismBehaviorController_ to avoid
    // shadowing Creature's legacy _behaviorController during the transition;
    // merged into the sole controller once Creature/Plant are collapsed.
    std::unique_ptr<BehaviorController> organismBehaviorController_;

    // Offspring produced by this organism during the current tick,
    // waiting to be added to the simulation by the main loop.
    std::unique_ptr<Organism> pendingOffspring_;
};

} // namespace Genetics
} // namespace EcoSim
