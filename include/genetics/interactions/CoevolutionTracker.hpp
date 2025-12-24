#ifndef ECOSIM_GENETICS_COEVOLUTION_TRACKER_HPP
#define ECOSIM_GENETICS_COEVOLUTION_TRACKER_HPP

#include <string>
#include <vector>
#include <map>
#include <deque>
#include <memory>
#include <utility>
#include "objects/creature/creature.hpp"
#include "genetics/organisms/Plant.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * @brief Statistics about a coevolutionary relationship between genes
 * 
 * Tracks correlation and trends between creature and plant gene frequencies
 * over generations to detect evolutionary arms races.
 */
struct CoevolutionStats {
    std::string creatureGene;           ///< Name of creature gene being tracked
    std::string plantGene;              ///< Name of plant gene being tracked
    float correlationCoefficient;       ///< Pearson correlation (-1 to 1)
    int generationsTracked;             ///< Number of generations with data
    std::string trend;                  ///< "escalating", "stable", "declining", "oscillating"
    float creatureMeanValue;            ///< Mean creature gene value this generation
    float plantMeanValue;               ///< Mean plant gene value this generation
    float creatureVariance;             ///< Variance in creature gene
    float plantVariance;                ///< Variance in plant gene
    
    CoevolutionStats()
        : correlationCoefficient(0.0f)
        , generationsTracked(0)
        , trend("unknown")
        , creatureMeanValue(0.0f)
        , plantMeanValue(0.0f)
        , creatureVariance(0.0f)
        , plantVariance(0.0f)
    {}
};

/**
 * @brief Gene frequency record for a single generation
 */
struct GeneFrequencyRecord {
    int generation;
    std::map<std::string, float> meanValues;       ///< Gene name -> mean value
    std::map<std::string, float> variances;        ///< Gene name -> variance
    std::map<std::string, int> sampleCounts;       ///< Gene name -> sample count
};

/**
 * @brief Configuration for arms race detection
 */
struct ArmsRaceConfig {
    float correlationThreshold = 0.5f;             ///< Min correlation to consider linked
    float escalationThreshold = 0.1f;              ///< Min change per generation for escalation
    int minGenerationsForTrend = 5;                ///< Minimum generations to detect trend
    int maxHistoryGenerations = 100;               ///< How many generations to keep in memory
    
    // Known coevolutionary gene pairs to track
    std::vector<std::pair<std::string, std::string>> trackedPairs;
};

/**
 * @brief Tracks population-level gene frequencies to detect coevolutionary dynamics
 * 
 * This class monitors gene frequency changes over generations in both creatures
 * and plants to identify evolutionary arms races - situations where selection
 * pressure from one species drives evolutionary change in another, and vice versa.
 * 
 * Common coevolutionary pairs tracked:
 * - Plant TOXIN_PRODUCTION vs Creature TOXIN_TOLERANCE/TOXIN_METABOLISM
 * - Plant THORN_DENSITY vs Creature HIDE_THICKNESS
 * - Plant SEED_COAT_DURABILITY vs Creature SEED_DESTRUCTION_RATE
 * - Plant FRUIT_APPEAL vs Creature COLOR_VISION
 * 
 * @example
 * ```cpp
 * CoevolutionTracker tracker;
 * 
 * // Each generation, record population data
 * tracker.recordCreatureGeneration(allCreatures);
 * tracker.recordPlantGeneration(allPlants);
 * 
 * // Check for active arms races
 * if (tracker.isArmsRaceActive("TOXIN_TOLERANCE", "TOXIN_PRODUCTION")) {
 *     auto stats = tracker.getCoevolutionStats("TOXIN_TOLERANCE", "TOXIN_PRODUCTION");
 *     std::cout << "Arms race trend: " << stats.trend << std::endl;
 * }
 * ```
 */
class CoevolutionTracker {
public:
    // ========================================================================
    // Constructors
    // ========================================================================
    
    /**
     * @brief Default constructor with standard coevolutionary pairs
     */
    CoevolutionTracker();
    
    /**
     * @brief Constructor with custom configuration
     * @param config Configuration for arms race detection parameters
     */
    explicit CoevolutionTracker(const ArmsRaceConfig& config);
    
    // ========================================================================
    // Population Recording
    // ========================================================================
    
    /**
     * @brief Record gene frequencies from current creature population
     * 
     * Should be called once per generation with all living creatures.
     * Extracts gene values from each creature's phenotype and calculates
     * population statistics.
     * 
     * @param creatures Vector of all creatures in the population
     */
    void recordCreatureGeneration(const std::vector<Creature>& creatures);
    
    /**
     * @brief Record gene frequencies from current plant population
     * 
     * Should be called once per generation with all living plants.
     * Extracts gene values from each plant's phenotype and calculates
     * population statistics.
     * 
     * @param plants Vector of all plants in the population
     */
    void recordPlantGeneration(const std::vector<Plant>& plants);
    
    /**
     * @brief Advance to next generation
     * 
     * Call this after recording both creature and plant data for the generation.
     * Finalizes current generation's statistics and prepares for next.
     */
    void advanceGeneration();
    
    // ========================================================================
    // Arms Race Detection
    // ========================================================================
    
    /**
     * @brief Check if an evolutionary arms race is currently active
     * 
     * An arms race is considered active when:
     * - Correlation between genes exceeds threshold
     * - Both genes show directional change over recent generations
     * - The relationship is escalating (both increasing) or oscillating
     * 
     * @param creatureGene Name of creature gene (e.g., "TOXIN_TOLERANCE")
     * @param plantGene Name of plant gene (e.g., "TOXIN_PRODUCTION")
     * @return true if arms race detected, false otherwise
     */
    bool isArmsRaceActive(
        const std::string& creatureGene,
        const std::string& plantGene
    ) const;
    
    /**
     * @brief Get the strength of coevolutionary relationship
     * 
     * Returns a value from 0 to 1 indicating how strongly the two genes
     * appear to be coevolving. Based on correlation strength and trend consistency.
     * 
     * @param creatureGene Name of creature gene
     * @param plantGene Name of plant gene
     * @return Coevolution strength (0-1), 0 if no data
     */
    float getCoevolutionStrength(
        const std::string& creatureGene,
        const std::string& plantGene
    ) const;
    
    // ========================================================================
    // Statistics Retrieval
    // ========================================================================
    
    /**
     * @brief Get detailed statistics for a gene pair
     * 
     * @param creatureGene Name of creature gene
     * @param plantGene Name of plant gene
     * @return CoevolutionStats with correlation, trend, and variance data
     */
    CoevolutionStats getCoevolutionStats(
        const std::string& creatureGene,
        const std::string& plantGene
    ) const;
    
    /**
     * @brief Get all currently active arms races
     * 
     * Scans all tracked gene pairs and returns statistics for those
     * with active coevolutionary dynamics.
     * 
     * @return Vector of CoevolutionStats for active arms races
     */
    std::vector<CoevolutionStats> getActiveArmsRaces() const;
    
    /**
     * @brief Get statistics for all tracked gene pairs
     * 
     * @return Vector of CoevolutionStats for all tracked pairs
     */
    std::vector<CoevolutionStats> getAllTrackedPairs() const;
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Add a gene pair to track
     * 
     * @param creatureGene Name of creature gene
     * @param plantGene Name of plant gene
     */
    void addTrackedPair(const std::string& creatureGene, const std::string& plantGene);
    
    /**
     * @brief Remove a gene pair from tracking
     * 
     * @param creatureGene Name of creature gene
     * @param plantGene Name of plant gene
     */
    void removeTrackedPair(const std::string& creatureGene, const std::string& plantGene);
    
    /**
     * @brief Update configuration
     * @param config New configuration settings
     */
    void setConfig(const ArmsRaceConfig& config);
    
    /**
     * @brief Get current configuration
     * @return Current ArmsRaceConfig
     */
    const ArmsRaceConfig& getConfig() const { return config_; }
    
    // ========================================================================
    // History Access
    // ========================================================================
    
    /**
     * @brief Get the current generation number
     * @return Current generation (starts at 0)
     */
    int getCurrentGeneration() const { return currentGeneration_; }
    
    /**
     * @brief Get historical data for a creature gene
     * 
     * @param geneName Name of creature gene
     * @return Vector of (generation, mean value) pairs
     */
    std::vector<std::pair<int, float>> getCreatureGeneHistory(
        const std::string& geneName
    ) const;
    
    /**
     * @brief Get historical data for a plant gene
     * 
     * @param geneName Name of plant gene
     * @return Vector of (generation, mean value) pairs
     */
    std::vector<std::pair<int, float>> getPlantGeneHistory(
        const std::string& geneName
    ) const;
    
    /**
     * @brief Clear all historical data
     */
    void clearHistory();
    
    /**
     * @brief Reset tracker to initial state
     */
    void reset();
    
private:
    // ========================================================================
    // Internal Methods
    // ========================================================================
    
    /**
     * @brief Calculate Pearson correlation coefficient between two gene histories
     */
    float calculateCorrelation(
        const std::vector<float>& creatureValues,
        const std::vector<float>& plantValues
    ) const;
    
    /**
     * @brief Determine trend from historical values
     * @return "escalating", "declining", "stable", or "oscillating"
     */
    std::string detectTrend(
        const std::vector<float>& creatureValues,
        const std::vector<float>& plantValues
    ) const;
    
    /**
     * @brief Calculate linear regression slope
     */
    float calculateSlope(const std::vector<float>& values) const;
    
    /**
     * @brief Calculate variance of values
     */
    float calculateVariance(const std::vector<float>& values) const;
    
    /**
     * @brief Calculate mean of values
     */
    float calculateMean(const std::vector<float>& values) const;
    
    /**
     * @brief Initialize default tracked gene pairs
     */
    void initializeDefaultPairs();
    
    // ========================================================================
    // Member Variables
    // ========================================================================
    
    ArmsRaceConfig config_;                            ///< Detection configuration
    int currentGeneration_;                            ///< Current generation number
    
    /// Historical creature gene frequencies by generation
    std::deque<GeneFrequencyRecord> creatureHistory_;
    
    /// Historical plant gene frequencies by generation
    std::deque<GeneFrequencyRecord> plantHistory_;
    
    /// Current generation's creature data (before finalization)
    GeneFrequencyRecord currentCreatureData_;
    
    /// Current generation's plant data (before finalization)
    GeneFrequencyRecord currentPlantData_;
    
    /// Cached correlation values for performance
    mutable std::map<std::pair<std::string, std::string>, float> correlationCache_;
    
    /// Flag indicating if cache needs refresh
    mutable bool cacheValid_;
};

} // namespace Genetics
} // namespace EcoSim

#endif // ECOSIM_GENETICS_COEVOLUTION_TRACKER_HPP
