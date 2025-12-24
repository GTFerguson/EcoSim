/**
 * @file ScentLayer.hpp
 * @brief Scent-based environmental layer for olfactory communication
 * 
 * Phase 1 of the Sensory System implementation. The ScentLayer provides
 * a sparse, performance-optimized storage system for scent deposits that
 * enable creatures to find mates through pheromone trails rather than
 * relying solely on limited visual range.
 * 
 * Design Goals:
 * - Sparse storage: Only tiles with active scents consume memory
 * - Performance target: <1ms overhead for 250,000 tile maps
 * - Batch decay processing: Every 10 ticks instead of every tick
 * - Support for multiple scent types (breeding, territorial, alarm, etc.)
 * 
 * @see plans/mating-behavior-system-design.md for full specification
 */

#ifndef SCENT_LAYER_HPP
#define SCENT_LAYER_HPP

#include <array>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace EcoSim {

/**
 * @brief Types of scent that creatures can deposit
 * 
 * Different scent types serve different biological purposes and may
 * be detected/processed differently by creatures based on their
 * sensory genes.
 */
enum class ScentType {
    MATE_SEEKING,      ///< Breeding pheromones for mate attraction
    TERRITORIAL,       ///< Area marking to claim territory
    ALARM,            ///< Danger signals to warn others
    FOOD_TRAIL,       ///< Path markers to food sources
    PREDATOR_MARK     ///< Indicates predator presence
};

/**
 * @brief A single scent deposit on a tile
 * 
 * Scent deposits contain:
 * - Type information for behavioral response
 * - Source creature ID for recognition/avoidance
 * - Intensity that decays over time
 * - Genetic signature for individual recognition
 * - Timing data for decay calculations
 */
struct ScentDeposit {
    ScentType type;                       ///< Category of scent
    int creatureId;                       ///< ID of depositing creature (-1 if unknown)
    float intensity;                      ///< Current strength [0.0-1.0]
    std::array<float, 8> signature;       ///< Genetic fingerprint (8 dimensions)
    unsigned int tickDeposited;           ///< When the scent was created
    unsigned int decayRate;               ///< Ticks until fully decayed
    
    /**
     * @brief Default constructor creates an empty deposit
     */
    ScentDeposit() 
        : type(ScentType::MATE_SEEKING)
        , creatureId(-1)
        , intensity(0.0f)
        , signature({0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f})
        , tickDeposited(0)
        , decayRate(100)
    {}
    
    /**
     * @brief Full constructor for creating a scent deposit
     * @param t Scent type
     * @param cId Creature ID
     * @param inten Initial intensity
     * @param sig Genetic signature
     * @param tick Current tick when deposited
     * @param decay Ticks until decay
     */
    ScentDeposit(ScentType t, int cId, float inten, 
                 const std::array<float, 8>& sig,
                 unsigned int tick, unsigned int decay)
        : type(t)
        , creatureId(cId)
        , intensity(inten)
        , signature(sig)
        , tickDeposited(tick)
        , decayRate(decay)
    {}
    
    /**
     * @brief Check if the scent has decayed completely
     * @param currentTick Current simulation tick
     * @return true if the scent should be removed
     */
    bool isDecayed(unsigned int currentTick) const {
        return currentTick >= (tickDeposited + decayRate) || intensity <= 0.0f;
    }
    
    /**
     * @brief Calculate current intensity after decay
     * @param currentTick Current simulation tick
     * @return Decayed intensity value [0.0-1.0]
     */
    float getDecayedIntensity(unsigned int currentTick) const {
        if (isDecayed(currentTick)) return 0.0f;
        
        unsigned int elapsed = currentTick - tickDeposited;
        float decayFraction = static_cast<float>(elapsed) / static_cast<float>(decayRate);
        return intensity * (1.0f - decayFraction);
    }
};

/**
 * @brief Sparse scent storage layer for the simulation world
 * 
 * Uses an unordered_map with tile coordinates as keys for O(1) average
 * lookup/insertion. Only stores tiles that have active scent deposits,
 * making memory usage proportional to scent activity rather than world size.
 * 
 * Performance characteristics:
 * - Deposit: O(1) average
 * - Query: O(k) where k is scents on tile (typically small)
 * - Decay update: O(n) where n is active scent count
 * - Memory: ~48 bytes per ScentDeposit + map overhead
 * 
 * Usage:
 * @code
 * ScentLayer scents(500, 500);  // 500x500 world
 * 
 * // Deposit breeding pheromone
 * scents.deposit(x, y, ScentDeposit(
 *     ScentType::MATE_SEEKING, 
 *     creature.getId(),
 *     0.8f,  // intensity
 *     creature.getGeneticSignature(),
 *     currentTick,
 *     200  // decay over 200 ticks
 * ));
 * 
 * // Query scents at position
 * auto scents_here = scents.getScentsAt(x, y);
 * for (const auto& scent : scents_here) {
 *     if (scent.type == ScentType::MATE_SEEKING) {
 *         // Found potential mate scent!
 *     }
 * }
 * 
 * // Update decay (call periodically)
 * scents.update(currentTick);
 * @endcode
 */
class ScentLayer {
public:
    /**
     * @brief Construct a scent layer for the given world dimensions
     * @param width World width in tiles
     * @param height World height in tiles
     * @param decayInterval Ticks between decay processing (default 10)
     */
    ScentLayer(int width, int height, unsigned int decayInterval = 10);
    
    /**
     * @brief Default constructor for delayed initialization
     */
    ScentLayer();
    
    /**
     * @brief Initialize or reinitialize the layer dimensions
     * @param width World width in tiles
     * @param height World height in tiles
     */
    void initialize(int width, int height);
    
    /**
     * @brief Deposit a scent at the specified tile
     * @param x X coordinate
     * @param y Y coordinate
     * @param scent The scent deposit to add
     * 
     * If a scent from the same creature of the same type already exists,
     * it will be refreshed rather than creating a duplicate.
     */
    void deposit(int x, int y, const ScentDeposit& scent);
    
    /**
     * @brief Get all active scents at a tile position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Vector of scent deposits (empty if none)
     */
    std::vector<ScentDeposit> getScentsAt(int x, int y) const;
    
    /**
     * @brief Get scents of a specific type at a tile
     * @param x X coordinate
     * @param y Y coordinate
     * @param type Scent type to filter for
     * @return Vector of matching scent deposits
     */
    std::vector<ScentDeposit> getScentsOfType(int x, int y, ScentType type) const;
    
    /**
     * @brief Get the strongest scent of a type within a radius
     * @param centerX Center X coordinate
     * @param centerY Center Y coordinate
     * @param radius Search radius in tiles
     * @param type Scent type to search for
     * @param outX Output: X coordinate of strongest scent
     * @param outY Output: Y coordinate of strongest scent
     * @return The strongest scent found, or empty deposit if none
     */
    ScentDeposit getStrongestScentInRadius(
        int centerX, int centerY, int radius,
        ScentType type, int& outX, int& outY) const;
    
    /**
     * @brief Get all scents of a type within a radius
     * @param centerX Center X coordinate
     * @param centerY Center Y coordinate
     * @param radius Search radius in tiles
     * @param type Scent type to search for
     * @return Vector of tuples: (ScentDeposit, x, y) for all matching scents
     */
    std::vector<std::tuple<ScentDeposit, int, int>> getScentsInRadius(
        int centerX, int centerY, int radius, ScentType type) const;
    
    /**
     * @brief Update scent decay and remove expired scents
     * @param currentTick Current simulation tick
     * 
     * This should be called every tick, but actual decay processing
     * only occurs at the configured decayInterval to save performance.
     */
    void update(unsigned int currentTick);
    
    /**
     * @brief Remove all scents from the layer
     */
    void clear();
    
    /**
     * @brief Remove all scents from a specific creature
     * @param creatureId Creature ID to remove scents for
     */
    void removeScentsFromCreature(int creatureId);
    
    /**
     * @brief Get the number of tiles with active scents
     * @return Count of tiles containing at least one scent
     */
    size_t getActiveTileCount() const;
    
    /**
     * @brief Get the total number of active scent deposits
     * @return Total count of all scent deposits
     */
    size_t getTotalScentCount() const;
    
    /**
     * @brief Get world dimensions
     */
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    
    /**
     * @brief Check if coordinates are within bounds
     */
    bool isInBounds(int x, int y) const {
        return x >= 0 && x < _width && y >= 0 && y < _height;
    }

private:
    int _width;
    int _height;
    unsigned int _decayInterval;
    unsigned int _lastDecayTick;
    
    /**
     * @brief Hash function for tile coordinates
     */
    struct TileHash {
        std::size_t operator()(const std::pair<int, int>& coords) const {
            // Cantor pairing function for coordinate hashing
            auto x = static_cast<std::size_t>(coords.first);
            auto y = static_cast<std::size_t>(coords.second);
            return ((x + y) * (x + y + 1)) / 2 + y;
        }
    };
    
    /**
     * @brief Sparse storage: tile coords -> list of scent deposits
     * 
     * Using unordered_map means we only allocate memory for tiles
     * that actually have scents, which is typically a small fraction
     * of the total world size.
     */
    std::unordered_map<std::pair<int, int>, std::vector<ScentDeposit>, TileHash> _scents;
    
    /**
     * @brief Process decay for all scent deposits
     * @param currentTick Current simulation tick
     */
    void processDecay(unsigned int currentTick);
};

} // namespace EcoSim

#endif // SCENT_LAYER_HPP
