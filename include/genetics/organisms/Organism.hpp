#pragma once

#include "genetics/interfaces/IPositionable.hpp"
#include "genetics/interfaces/ILifecycle.hpp"
#include "genetics/interfaces/IGenetic.hpp"
#include "genetics/interfaces/IReproducible.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"
#include <memory>

namespace EcoSim {
namespace Genetics {

class GeneRegistry;

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
    void setWorldPosition(float x, float y) override = 0;
    
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
    unsigned int getMaxLifespan() const override = 0;  // Gene-dependent
    
    // ========================================================================
    // IGenetic - Fully implemented
    // ========================================================================
    
    const Genome& getGenome() const override { return genome_; }
    Genome& getGenomeMutable() override { return genome_; }
    const Phenotype& getPhenotype() const override { return phenotype_; }
    void updatePhenotype() override;
    
    // ========================================================================
    // IReproducible - Subclasses implement (highly type-specific)
    // ========================================================================
    
    bool canReproduce() const override = 0;
    float getReproductiveUrge() const override = 0;
    float getReproductionEnergyCost() const override = 0;
    ReproductionMode getReproductionMode() const override = 0;
    bool isCompatibleWith(const Organism& other) const override = 0;
    std::unique_ptr<Organism> reproduce(
        const Organism* partner = nullptr) override = 0;
    
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
    virtual void grow() = 0;
    
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

protected:
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
     * @brief Rebind phenotype's genome pointer after move
     * 
     * The Phenotype holds a pointer to the Genome for trait expression.
     * After moving, this pointer must be updated to point to THIS
     * organism's genome, not the moved-from organism's genome.
     */
    void rebindPhenotypeGenome();
    
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
};

} // namespace Genetics
} // namespace EcoSim
