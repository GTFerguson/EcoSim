#pragma once

#include "rendering/RenderTypes.hpp"

#include <string>
#include <atomic>
#include <unordered_map>
#include <mutex>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Shared flyweight for organism archetype identity
 *
 * Uses Meyer's Singleton pattern for thread-safe static initialization.
 * Maintains atomic population counters for O(1) census queries.
 *
 * Holds the immutable display/identity metadata shared by all organisms
 * classified into this archetype: id, label, render character, semantic
 * entity type, description, and passability. One pointer per organism
 * replaces six per-instance fields.
 *
 * Thread Safety:
 * - All static access methods are thread-safe
 * - Population counters use atomic operations
 *
 * Lifetime:
 * - Flyweight objects live for the duration of the program
 * - Returned pointers are NEVER null and NEVER become invalid
 *
 * @note Callers should NOT delete the returned pointers.
 */
class ArchetypeIdentity {
public:
    // ========== Predefined Archetype Access (Thread-Safe) ==========
    
    /**
     * @brief Get shared identity for Apex Predator archetype
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Large, powerful, territorial dominant predator.
     * Character: 'A', Label: "Apex Predator"
     */
    static const ArchetypeIdentity* ApexPredator();
    
    /**
     * @brief Get shared identity for Pack Hunter archetype
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Small, fast, coordinated group hunters.
     * Character: 'P', Label: "Pack Hunter"
     */
    static const ArchetypeIdentity* PackHunter();
    
    /**
     * @brief Get shared identity for Ambush Predator archetype
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Patient, stealthy, high burst damage.
     * Character: 'I', Label: "Ambush Predator"
     */
    static const ArchetypeIdentity* AmbushPredator();
    
    /**
     * @brief Get shared identity for Pursuit Hunter archetype
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Fastest predator, speed-based chasers.
     * Character: 'V', Label: "Pursuit Hunter"
     */
    static const ArchetypeIdentity* PursuitHunter();
    
    /**
     * @brief Get shared identity for Tank Herbivore archetype
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Large armored defensive herbivore with horn defense.
     * Character: 'T', Label: "Tank Herbivore"
     */
    static const ArchetypeIdentity* TankHerbivore();
    
    /**
     * @brief Get shared identity for Armored Grazer archetype
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Scaled herbivore with tail club defense.
     * Character: 'K', Label: "Armored Grazer"
     */
    static const ArchetypeIdentity* ArmoredGrazer();
    
    /**
     * @brief Get shared identity for Fleet Runner archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * Speed-based escape specialist, minimal combat.
     * Character: 'R', Label: "Fleet Runner"
     */
    static const ArchetypeIdentity* FleetRunner();
    
    /**
     * @brief Get shared identity for Spiky Defender archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * Counter-attack specialist with body spines.
     * Character: 'S', Label: "Spiky Defender"
     */
    static const ArchetypeIdentity* SpikyDefender();
    
    /**
     * @brief Get shared identity for Canopy Forager archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * Fruit-eating specialist with excellent sensory abilities.
     * Monkey-inspired arboreal forager, good seed disperser.
     * Character: 'C', Label: "Canopy Forager"
     */
    static const ArchetypeIdentity* CanopyForager();
    
    /**
     * @brief Get shared identity for Scavenger archetype
     * @return Non-owning pointer to shared flyweight (never null)
     * 
     * Corpse-feeding specialist with high toxin tolerance.
     * Character: 'N', Label: "Carrion Stalker"
     */
    static const ArchetypeIdentity* Scavenger();
    
    /**
     * @brief Get shared identity for Omnivore Generalist archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * Adaptable mixed diet, jack of all trades.
     * Character: 'O', Label: "Omnivore Generalist"
     */
    static const ArchetypeIdentity* OmnivoreGeneralist();

    // ----- Plant archetypes -----

    /**
     * @brief Get shared identity for Berry Bush plant archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * r-strategy shrub with fast growth and berry fruiting.
     * Character: 'B', Label: "Berry Bush"
     */
    static const ArchetypeIdentity* BerryBush();

    /**
     * @brief Get shared identity for Oak Tree plant archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * K-strategy long-lived tree with slow growth.
     * Character: 'T', Label: "Oak Tree"
     */
    static const ArchetypeIdentity* OakTree();

    /**
     * @brief Get shared identity for Grass plant archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * Low-biomass ground cover, grazing-adapted.
     * Character: '"', Label: "Grass"
     */
    static const ArchetypeIdentity* Grass();

    /**
     * @brief Get shared identity for Thorn Bush plant archetype
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * Defensive shrub with spines, deters herbivory.
     * Character: '*', Label: "Thorn Bush"
     */
    static const ArchetypeIdentity* ThornBush();

    /**
     * @brief Get shared identity for generic unclassified plant
     * @return Non-owning pointer to shared flyweight (never null)
     *
     * Fallback archetype for plants whose genome does not cleanly
     * match any specialised plant classifier bucket.
     * Character: '.', Label: "Plant"
     */
    static const ArchetypeIdentity* GenericPlant();

    // ========== Identity Properties (Immutable) ==========

    /**
     * @brief Get unique identifier for this archetype
     * @return ID like "apex_predator", "pack_hunter", etc.
     */
    const std::string& getId() const { return id_; }

    /**
     * @brief Get human-readable label for this archetype
     * @return Label like "Apex Predator", "Carrion Stalker", etc.
     */
    const std::string& getLabel() const { return label_; }

    /**
     * @brief Get render character for this archetype
     * @return Character like 'A', 'P', 'I', etc.
     */
    char getRenderChar() const { return renderChar_; }

    /**
     * @brief Get semantic entity type for renderer-agnostic rendering
     */
    EntityType getEntityType() const { return entityType_; }

    /**
     * @brief Get short human-readable description
     */
    const std::string& getDesc() const { return desc_; }

    /**
     * @brief Whether other organisms can move through a tile occupied by this archetype
     */
    bool getPassable() const { return passable_; }
    
    // ========== Population Tracking (Thread-Safe) ==========
    
    /**
     * @brief Increment population count
     * @note Called automatically by Creature constructor after all setup
     */
    void incrementPopulation() const { ++population_; }
    
    /**
     * @brief Decrement population count
     * @note Called automatically by Creature destructor
     */
    void decrementPopulation() const { --population_; }
    
    /**
     * @brief Get current population count
     * @return Number of living creatures with this archetype
     */
    int getPopulation() const { return population_.load(); }
    
    // ========== Non-copyable (Flyweight Pattern) ==========
    ArchetypeIdentity(const ArchetypeIdentity&) = delete;
    ArchetypeIdentity& operator=(const ArchetypeIdentity&) = delete;
    
private:
    /**
     * @brief Private constructor - only static factory methods can create
     * @param id Unique identifier (e.g., "apex_predator")
     * @param label Human-readable label (e.g., "Apex Predator")
     * @param renderChar Display character (e.g., 'A')
     * @param entityType Semantic entity type used by renderers
     * @param desc Short human-readable description
     * @param passable Whether organisms can move through a tile occupied by this archetype
     */
    ArchetypeIdentity(std::string id,
                      std::string label,
                      char renderChar,
                      EntityType entityType,
                      std::string desc,
                      bool passable);

    std::string id_;
    std::string label_;
    char renderChar_;
    EntityType entityType_;
    std::string desc_;
    bool passable_;
    mutable std::atomic<int> population_{0};
};

} // namespace Genetics
} // namespace EcoSim
