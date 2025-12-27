#ifndef GENOME_STATS_H
#define GENOME_STATS_H

#include "../objects/creature/creature.hpp"

#include <vector>
#include <sstream>
#include <limits>
#include <math.h>

/**
 * @class GenomeStats
 * @brief Statistical analysis of genome traits across a creature population.
 *
 * Calculates descriptive statistics (min, max, mean, variance, standard deviation)
 * for each genetic trait across all creatures at a specific point in time.
 * This enables analysis of genetic diversity, trait distributions, and evolutionary
 * trends over the course of a simulation.
 *
 * Traits analyzed:
 * - Lifespan: Maximum age before natural death
 * - Sight: Vision range for detecting food/threats
 * - Flee: Speed when escaping threats
 * - Pursue: Speed when chasing food/mates
 * - Hunger: Rate of energy depletion
 * - Thirst: Rate of hydration depletion
 * - Mate: Willingness to reproduce
 * - ComfInc: Rate of comfort increase
 * - ComfDec: Rate of comfort decrease
 *
 * @note Thread Safety: This class is NOT thread-safe. All methods must be called
 * from a single thread, or external synchronization must be provided by the caller.
 *
 * Usage Example:
 * @code
 *   std::vector<Creature> population = getPopulation();
 *   unsigned int currentTime = calendar.getTotalHours();
 *   GenomeStats stats(population, currentTime);
 *   std::string csv = stats.toString(true);  // Export with header
 * @endcode
 */
class GenomeStats {
  private:
    /**
     * @struct UIGeneStat
     * @brief Statistical summary for unsigned integer genetic traits.
     *
     * Stores descriptive statistics for traits like lifespan, sight, flee, and pursue.
     */
    struct UIGeneStat {
      unsigned int max;      ///< Maximum value observed in population
      unsigned int min;      ///< Minimum value observed in population
      unsigned int mean;     ///< Average value across population
      unsigned int variance; ///< Variance (measure of spread from mean)
      unsigned int stdDev;   ///< Standard deviation (square root of variance)
      
      /// Initialize with sentinel values for min/max tracking
      UIGeneStat () : max (0),
                    min (std::numeric_limits<unsigned int>::max()),
                    mean (0),
                    variance (0),
                    stdDev (0)
                    {}
    };

    /**
     * @struct FGeneStat
     * @brief Statistical summary for floating-point genetic traits.
     *
     * Stores descriptive statistics for traits like hunger, thirst, and comfort rates.
     */
    struct FGeneStat {
      float max;      ///< Maximum value observed in population
      float min;      ///< Minimum value observed in population
      float mean;     ///< Average value across population
      float variance; ///< Variance (measure of spread from mean)
      float stdDev;   ///< Standard deviation (square root of variance)
      
      /// Initialize with sentinel values for min/max tracking
      FGeneStat () : max (std::numeric_limits<float>::lowest()),
                    min (std::numeric_limits<float>::max()),
                    mean (0.0f),
                    variance (0.0f),
                    stdDev (0.0f)
                    {}
    };

    /**
     * @struct SGenome
     * @brief Accumulator for genome trait sums during statistical calculations.
     *
     * Used internally to sum all trait values before computing means and variances.
     */
    struct SGenome {
      unsigned int  lifespan = 0, sight = 0, flee = 0, pursue = 0;
      float         hunger = 0.0f, thirst = 0.0f, mate = 0.0f, comfInc = 0.0f, comfDec = 0.0f;
    };

    unsigned int  time;                                        ///< Simulation time when statistics were calculated
    UIGeneStat    lifespan, sight, flee, pursue;              ///< Statistics for integer traits
    FGeneStat     hunger, thirst, mate, comfInc, comfDec;     ///< Statistics for float traits
  
    /// Helper to update maximum value during min/max scanning
    void checkMaxValue  (unsigned int &current, const unsigned int &value);
    /// Helper to update minimum value during min/max scanning
    void checkMinValue  (unsigned int &current, const unsigned int &value);
    /// Helper to update maximum value during min/max scanning (float version)
    void checkMaxValue  (float &current,        const float &value);
    /// Helper to update minimum value during min/max scanning (float version)
    void checkMinValue  (float &current,        const float &value);
    /// Scan creature for maximum trait values
    void checkMaxValues (const Creature &c);
    /// Scan creature for minimum trait values
    void checkMinValues (const Creature &c);
    /// Calculate mean values from accumulated sums
    void setMeans       (const SGenome &sums, const size_t &size);
    /// Calculate standard deviations from variances
    void setStdDev      (const size_t &size);
    /// Calculate variances using two-pass algorithm
    void setVariances   (const std::vector<Creature> &c,
                         const SGenome &sums, const size_t &size);
    /// Add creature trait values to running sums
    void accumulateSum  (SGenome &sums, const Creature &c);

  public:
    /**
     * @brief Constructs genome statistics from a creature population.
     * @param c Vector of all living creatures to analyze
     * @param time Simulation time (typically in hours) when analysis occurred
     *
     * Performs complete statistical analysis in constructor, calculating min, max,
     * mean, variance, and standard deviation for all genetic traits.
     *
     * @note If population is empty, all statistics will remain at default values.
     */
    GenomeStats (const std::vector<Creature> &c, const unsigned int &time);
    
    /**
     * @brief Exports genome statistics as CSV format.
     * @param includeHeader If true, includes column names as first row
     * @return String containing CSV data with columns: Time, Trait, Min, Max,
     *         Mean, Variance, StdDev. Each trait gets its own row.
     *
     * Output format is compatible with standard CSV parsers and spreadsheet tools.
     * Useful for tracking genetic diversity and evolutionary trends over time.
     */
    std::string toString(bool includeHeader = false) const;

};

#endif
