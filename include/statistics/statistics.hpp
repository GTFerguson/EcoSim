#ifndef STATISTICS_H 
#define STATISTICS_H 

/**
 *  Title   : Ecosim - Statistics 
 *  Author  : Gary Ferguson
 *	Date	  : April 18th, 2020
 *	Purpose	: Collects data on the simulation as it's ran 
 *	          putting it together for analysis by the user.
 */

#include "../calendar.hpp"

#include <vector>
#include <iterator>
#include <string>
#include <sstream>

/**
 * @struct DeathStats
 * @brief Tracks breakdown of creature deaths by cause during a simulation period.
 *
 * Each field represents the count of creatures that died from a specific cause.
 * All counts are cumulative for the period being tracked (tick, hour, or entire simulation).
 */
struct DeathStats {
  unsigned oldAge = 0;      ///< Deaths from reaching maximum lifespan
  unsigned starved = 0;     ///< Deaths from hunger reaching zero
  unsigned dehydrated = 0;  ///< Deaths from thirst reaching zero
  unsigned discomfort = 0;  ///< Deaths from comfort level being too low
  unsigned predator = 0;    ///< Deaths from predation by other creatures
};

/**
 * @struct GeneralStats
 * @brief Snapshot of simulation state at a specific point in time.
 *
 * Contains all key metrics for analyzing simulation dynamics. Each record represents
 * a single tick or aggregated period depending on how it was collected.
 */
struct GeneralStats {
  Calendar    calendar;   ///< Simulation time when this snapshot was taken
  unsigned    population; ///< Current living population count
  unsigned    births;     ///< Number of births during this period
  unsigned    foodAte;    ///< Total food consumed by all creatures (legacy system)
  unsigned    feeding = 0; ///< Number of genetics-based plant feeding events
  DeathStats  deaths;     ///< Breakdown of deaths by cause during this period
};

/**
 * @class Statistics
 * @brief Collects and aggregates simulation data over time.
 *
 * Maintains a time-series of GeneralStats records that can be accumulated
 * and exported for analysis. Supports both raw tick-by-tick recording and
 * aggregation into hourly summaries.
 *
 * @note Thread Safety: This class is NOT thread-safe. All methods must be called
 * from a single thread, or external synchronization must be provided by the caller.
 *
 * Usage Example:
 * @code
 *   Statistics stats;
 *   GeneralStats snapshot;
 *   snapshot.calendar = currentTime;
 *   snapshot.population = creatures.size();
 *   stats.addRecord(snapshot);
 *
 *   stats.accumulateByHour();  // Aggregate by hour
 *   std::string csv = stats.toString(true);  // Export with header
 * @endcode
 */
class Statistics {
  private:
    std::vector<GeneralStats> _records;  ///< Time-series of statistical snapshots

  public:
    //============================================================================
    //  Record Statistics
    //============================================================================
    /**
     * @brief Adds a new statistical snapshot to the time-series.
     * @param gs The GeneralStats record to add. Should represent a single tick
     *           or period of simulation time.
     *
     * Records are stored in chronological order. No validation is performed
     * on the timestamp ordering - callers must ensure records are added sequentially.
     */
    void addRecord (const GeneralStats &gs);

    //============================================================================
    //  Clear Statistics
    //============================================================================
    /**
     * @brief Removes all statistical records from memory.
     *
     * Use this to free memory during long-running simulations, or to start
     * fresh statistical tracking for a new simulation phase.
     */
    void clearRecords ();

    //============================================================================
    //  Get Statistics
    //============================================================================
    /**
     * @brief Returns read-only access to all statistical records.
     * @return Const reference to the vector of all GeneralStats records.
     *
     * Efficient accessor that avoids copying. Use for analysis or export
     * without modifying the original data.
     */
    const std::vector<GeneralStats>& getRecords () const;

    //============================================================================
    //  Accumulate
    //============================================================================
    /**
     * @brief Aggregates all records into a single summary record.
     *
     * Sums all births, deaths, and food consumption across all records.
     * The population and calendar are taken from the most recent record.
     * After accumulation, only one record remains in memory.
     *
     * Use this to get total statistics for the entire simulation period.
     */
    void accumulate ();
    
    /**
     * @brief Aggregates records into hourly summaries.
     *
     * Groups all records by simulation hour, summing births, deaths, and
     * food consumption within each hour. Population is taken from the last
     * tick of each hour. After accumulation, there is one record per hour.
     *
     * Use this to analyze hourly trends without storing every tick's data.
     */
    void accumulateByHour ();
    
    //============================================================================
    //  To String
    //============================================================================
    /**
     * @brief Exports all records as CSV format.
     * @param includeHeader If true, includes column names as first row.
     * @return String containing CSV data with columns: Time, Date, Population,
     *         FoodAte, Births, OldAge, Starved, Dehydrated, Discomfort, Predator
     *
     * Output format is compatible with standard CSV parsers and spreadsheet tools.
     * Each row represents one statistical record.
     */
    std::string toString (bool includeHeader = false) const;
};

#endif
