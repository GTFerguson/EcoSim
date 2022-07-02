#ifndef STATISTICS_H 
#define STATISTICS_H 

/**
 *  Title   : Ecosim - Statistics 
 *  Author  : Gary Ferguson
 *	Date	  : April 18th, 2020
 *	Purpose	: Collects data on the simulation as it's ran 
 *	          putting it together for analysis by the user.
 */

#include "../../include/calendar.hpp"

#include <vector>
#include <iterator>
#include <string>
#include <sstream>

struct DeathStats {
  unsigned oldAge, starved, dehydrated, discomfort, predator;
};

struct GeneralStats {
  Calendar    calendar;
  unsigned    population;
  unsigned    births;
  unsigned    foodAte;
  DeathStats  deaths;
};

class Statistics {
  private:
    std::vector<GeneralStats> _records;
    //std::vector<GenomeStats>  _genomeRecords;

  public:
    //============================================================================
    //  Record Statistics
    //============================================================================
    void addRecord (const GeneralStats &gs);
    //void addGenomeRecord  (const std::vector<Creature> &c, const unsigned int &time);

    //============================================================================
    //  Clear Statistics
    //============================================================================
    void clearRecords (); 

    //============================================================================
    //  Get Statistics
    //============================================================================
    std::vector<GeneralStats> getRecords () const;

    //============================================================================
    //  Accumulate
    //============================================================================
    void accumulate ();
    void accumulateByHour ();
    
    //============================================================================
    //  To String
    //============================================================================
    std::string toString      () const;
};

#endif
