/**
 *	Title	  : Ecosim - Statistics
 *	Author	: Gary Ferguson
 *	Date	  : April 18th, 2020
 *	Purpose	: Collects data on the simulation as it's ran 
 *	          putting it together for analysis by the user.
 */

#include "../../include/statistics/statistics.hpp"

using namespace std;

//================================================================================
//  Record Statictics Methods
//================================================================================
void Statistics::addRecord (const GeneralStats &gs) {
  _records.push_back (gs);
}

//================================================================================
//  Clear Statistics 
//================================================================================
void Statistics::clearRecords () { _records.clear (); }

//================================================================================
//  Get Statistics
//================================================================================
vector<GeneralStats> Statistics::getRecords () const {
  return _records;
}

//================================================================================
//  Accumulate
//================================================================================
void Statistics::accumulate () {
  size_t rSize = _records.size ();

  GeneralStats accum = { _records.at(rSize-1).calendar, 
                         _records.at(rSize-1).population, 
                         0, 0, 0 }; 

  for (GeneralStats record : _records) {
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
  vector<GeneralStats>::iterator it = _records.begin();
  vector<GeneralStats> temp;

  GeneralStats accum = *it;
  it++;

  while (it < _records.end()) {
    if (it->calendar.getHour() == accum.calendar.getHour()) {
      accum.population         = it->population;
      accum.foodAte           += it->foodAte;
      accum.births            += it->births;
      accum.deaths.oldAge     += it->deaths.oldAge;
      accum.deaths.starved    += it->deaths.starved;
      accum.deaths.dehydrated += it->deaths.dehydrated;
      accum.deaths.discomfort += it->deaths.discomfort;
      accum.deaths.predator   += it->deaths.predator;
      _records.erase (it);
    } else {
      temp.push_back (accum);
      accum = *it;
      it++;
    }
  }
  _records = temp;
}

//================================================================================
//  To String Methods
//================================================================================
string Statistics::toString () const {
  size_t rSize = _records.size ();
  ostringstream ss;

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
