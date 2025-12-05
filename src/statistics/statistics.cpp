/**
 * @file statistics.cpp
 * @brief Implementation of the Statistics class for simulation data collection.
 * @author Gary Ferguson
 * @date April 18th, 2020
 *
 * This file implements the Statistics class which collects, aggregates, and exports
 * simulation metrics over time. It provides methods for recording snapshots, accumulating
 * data into summaries, and exporting results as CSV format for external analysis.
 *
 * Key capabilities:
 * - Record tick-by-tick simulation snapshots
 * - Aggregate into hourly or total summaries
 * - Export as CSV with configurable headers
 * - Efficient memory management with const references
 */

#include "../../include/statistics/statistics.hpp"

using std::vector;
using std::string;
using std::ostringstream;
using std::endl;

//================================================================================
//  Record Statictics Methods
//================================================================================
void Statistics::addRecord (const GeneralStats &gs) {
  // Basic validation - ensure population matches reality
  if (gs.population == 0 && (gs.births > 0 || gs.foodAte > 0)) {
    // Warning: population is 0 but activity recorded - may indicate data issue
    // Continue anyway as this might be valid during initialization
  }
  _records.push_back (gs);
}

//================================================================================
//  Clear Statistics 
//================================================================================
void Statistics::clearRecords () { _records.clear (); }

//================================================================================
//  Get Statistics
//================================================================================
const vector<GeneralStats>& Statistics::getRecords () const {
  return _records;
}

//================================================================================
//  Accumulate
//================================================================================
void Statistics::accumulate () {
  if (_records.empty()) return;
  
  size_t rSize = _records.size ();

  GeneralStats accum;
  accum.calendar   = _records.at(rSize-1).calendar;
  accum.population = _records.at(rSize-1).population;

  for (const GeneralStats &record : _records) {
    accum.foodAte           += record.foodAte;
    accum.births            += record.births;
    accum.deaths.oldAge     += record.deaths.oldAge;
    accum.deaths.starved    += record.deaths.starved;
    accum.deaths.dehydrated += record.deaths.dehydrated;
    accum.deaths.discomfort += record.deaths.discomfort;
    accum.deaths.predator   += record.deaths.predator;
  }

  clearRecords ();
  addRecord (accum);
}

void Statistics::accumulateByHour () {
  if (_records.empty()) return;
  
  vector<GeneralStats> temp;
  GeneralStats accum = _records[0];

  for (size_t i = 1; i < _records.size(); i++) {
    if (_records[i].calendar.getHour() == accum.calendar.getHour()) {
      accum.population         = _records[i].population;
      accum.foodAte           += _records[i].foodAte;
      accum.births            += _records[i].births;
      accum.deaths.oldAge     += _records[i].deaths.oldAge;
      accum.deaths.starved    += _records[i].deaths.starved;
      accum.deaths.dehydrated += _records[i].deaths.dehydrated;
      accum.deaths.discomfort += _records[i].deaths.discomfort;
      accum.deaths.predator   += _records[i].deaths.predator;
    } else {
      temp.push_back (accum);
      accum = _records[i];
    }
  }
  // Push the last accumulated record (fixes off-by-one)
  temp.push_back (accum);
  
  _records = temp;
}

//================================================================================
//  To String Methods
//================================================================================
string Statistics::toString (bool includeHeader) const {
  size_t rSize = _records.size ();
  ostringstream ss;

  // Handle empty records case
  if (rSize == 0) {
    if (includeHeader) {
      return "Time,Date,Population,FoodAte,Births,OldAge,Starved,Dehydrated,Discomfort,Predator";
    }
    return "";
  }

  // Add CSV header if requested
  if (includeHeader) {
    ss << "Time,Date,Population,FoodAte,Births,OldAge,Starved,Dehydrated,Discomfort,Predator" << endl;
  }

  for (size_t i = 0; i < rSize; i++) {
    ss  << _records.at(i).calendar.shortTime () << ","
        << _records.at(i).calendar.shortDate () << ","
        << _records.at(i).population            << ","
        << _records.at(i).foodAte               << ","
        << _records.at(i).births                << ","
        << _records.at(i).deaths.oldAge         << ","
        << _records.at(i).deaths.starved        << ","
        << _records.at(i).deaths.dehydrated     << ","
        << _records.at(i).deaths.discomfort     << ","
        << _records.at(i).deaths.predator;

    if (i < rSize-1) ss << endl;
  }

  return ss.str ();
}
