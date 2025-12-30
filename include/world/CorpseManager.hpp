#pragma once
/**
 * @file CorpseManager.hpp
 * @brief Manages corpse lifecycle including decay and scavenging
 */

#include "Corpse.hpp"
#include <vector>
#include <memory>
#include <string>
#include <cmath>

namespace EcoSim {

/**
 * @class CorpseManager
 * @brief Manages corpse storage, decay processing, and spatial queries
 * 
 * Responsibilities:
 * - Corpse storage and retrieval
 * - Corpse decay processing
 * - Spatial queries for corpses
 * - Corpse removal (exhausted or fully decayed)
 */
class CorpseManager {
public:
    /// Maximum number of corpses for performance
    static constexpr size_t MAX_CORPSES = 100;
    
    //==========================================================================
    // Construction
    //==========================================================================
    
    /**
     * @brief Default constructor
     */
    CorpseManager() = default;
    
    /**
     * @brief Constructor with custom max corpses limit
     * @param maxCorpses Maximum number of corpses to track
     */
    explicit CorpseManager(size_t maxCorpses);
    
    //==========================================================================
    // Corpse Lifecycle
    //==========================================================================
    
    /**
     * @brief Add a corpse to the manager
     * @param x X coordinate
     * @param y Y coordinate
     * @param size Size of the creature (affects nutrition and decay time)
     * @param speciesName Species name for the corpse
     * @param bodyCondition Body condition (0.0-1.0) based on creature's energy when it died
     * 
     * If MAX_CORPSES is reached, removes the most decayed corpse first.
     */
    void addCorpse(float x, float y, float size, 
                   const std::string& speciesName, float bodyCondition = 0.5f);
    
    /**
     * @brief Process decay for all corpses and remove fully decayed ones
     * Call this each tick during the main update loop.
     */
    void tick();
    
    /**
     * @brief Remove all fully decayed corpses
     * Called automatically by tick(), but can be called manually.
     */
    void removeExpiredCorpses();
    
    /**
     * @brief Remove a specific corpse from the manager
     * @param corpse Pointer to the corpse to remove
     */
    void removeCorpse(world::Corpse* corpse);
    
    /**
     * @brief Clear all corpses
     */
    void clear();
    
    //==========================================================================
    // Queries
    //==========================================================================
    
    /**
     * @brief Get all corpses at a specific tile
     * @param x Tile X coordinate
     * @param y Tile Y coordinate
     * @return Vector of pointers to corpses at that location
     */
    std::vector<world::Corpse*> getCorpsesAt(int x, int y);
    
    /**
     * @brief Get all corpses at a specific tile (const version)
     * @param x Tile X coordinate
     * @param y Tile Y coordinate
     * @return Vector of const pointers to corpses at that location
     */
    std::vector<const world::Corpse*> getCorpsesAt(int x, int y) const;
    
    /**
     * @brief Get all corpses within a radius of a point
     * @param x Center X coordinate
     * @param y Center Y coordinate
     * @param radius Search radius
     * @return Vector of pointers to corpses within range
     */
    std::vector<world::Corpse*> getCorpsesInRadius(float x, float y, float radius);
    
    /**
     * @brief Get all corpses within a radius of a point (const version)
     * @param x Center X coordinate
     * @param y Center Y coordinate
     * @param radius Search radius
     * @return Vector of const pointers to corpses within range
     */
    std::vector<const world::Corpse*> getCorpsesInRadius(float x, float y, float radius) const;
    
    /**
     * @brief Find the nearest corpse within range
     * @param x X coordinate to search from
     * @param y Y coordinate to search from
     * @param maxRange Maximum search range
     * @return Pointer to nearest corpse, or nullptr if none found
     */
    world::Corpse* findNearest(float x, float y, float maxRange);
    
    /**
     * @brief Find the nearest corpse within range (const version)
     * @param x X coordinate to search from
     * @param y Y coordinate to search from
     * @param maxRange Maximum search range
     * @return Const pointer to nearest corpse, or nullptr if none found
     */
    const world::Corpse* findNearest(float x, float y, float maxRange) const;
    
    /**
     * @brief Get all corpses
     * @return Const reference to the corpse vector
     */
    const std::vector<std::unique_ptr<world::Corpse>>& getAll() const;
    
    //==========================================================================
    // Statistics
    //==========================================================================
    
    /**
     * @brief Get the current number of corpses
     * @return Number of corpses being tracked
     */
    size_t count() const;
    
    /**
     * @brief Check if there are any corpses
     * @return true if there are no corpses
     */
    bool empty() const;
    
    /**
     * @brief Get total remaining nutrition across all corpses
     * @return Sum of nutritional value from all corpses
     */
    float getTotalNutrition() const;
    
    /**
     * @brief Get total remaining nutrition at a specific tile
     * @param x Tile X coordinate
     * @param y Tile Y coordinate
     * @return Sum of nutritional value from corpses at that location
     */
    float getTotalNutritionAt(int x, int y) const;
    
private:
    std::vector<std::unique_ptr<world::Corpse>> _corpses;
    size_t _maxCorpses = MAX_CORPSES;
    
    /**
     * @brief Calculate squared distance between two points
     * @param x1 First point X
     * @param y1 First point Y
     * @param x2 Second point X
     * @param y2 Second point Y
     * @return Squared distance
     */
    static float distanceSquared(float x1, float y1, float x2, float y2);
};

} // namespace EcoSim
