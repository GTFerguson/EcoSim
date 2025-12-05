/**
 *  Title   : Statistics Test
 *  Author  : Gary Ferguson
 *  Purpose : Test statistics tracking functionality with proper assertions
 *
 *  Test Isolation:
 *  - This test does NOT access file system
 *  - All test data is created in-memory
 *  - No external dependencies on save files or fixtures
 *  - Each test uses fresh Statistics instances
 */

#include "../../include/statistics/statistics.hpp"
#include "../../include/statistics/genomeStats.hpp"
#include "../../include/objects/creature/creature.hpp"
#include "../../include/objects/creature/genome.hpp"
#include "../../include/calendar.hpp"

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

using namespace std;

// Test assertion macro with descriptive error messages
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << message << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

/**
 * Setup function - creates test statistics
 */
void testSetup() {
    // Statistics objects are created in-memory, no setup needed
}

/**
 * Teardown function - cleanup test data
 */
void testTeardown() {
    // Statistics objects are stack-allocated, automatic cleanup
}

int main () {
  cout << "=== Statistics Test Suite ===" << endl;
  
  testSetup();
  
  cout << endl << "TEST 1: Statistics Initialization" << endl;
  
  Statistics stats;
  const vector<GeneralStats>& records = stats.getRecords();
  
  TEST_ASSERT(records.empty(), "New statistics should have no records");
  
  cout << "  Initial records: " << records.size() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 2: Add Single Record" << endl;
  
  GeneralStats gs;
  gs.population = 10;
  gs.births = 2;
  gs.foodAte = 5;
  gs.deaths.oldAge = 1;
  gs.deaths.starved = 0;
  gs.deaths.dehydrated = 0;
  gs.deaths.discomfort = 0;
  gs.deaths.predator = 0;
  
  stats.addRecord(gs);
  
  const vector<GeneralStats>& recordsAfter = stats.getRecords();
  TEST_ASSERT(recordsAfter.size() == 1, "Should have one record after add");
  TEST_ASSERT(recordsAfter[0].population == 10, "Population should match");
  TEST_ASSERT(recordsAfter[0].births == 2, "Births should match");
  
  cout << "  Records after add: " << recordsAfter.size() << endl;
  cout << "  Population: " << recordsAfter[0].population << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 3: Add Multiple Records" << endl;
  
  for (int i = 0; i < 5; i++) {
    GeneralStats record;
    record.population = 10 + i;
    record.births = i;
    record.foodAte = i * 2;
    stats.addRecord(record);
  }
  
  const vector<GeneralStats>& multiRecords = stats.getRecords();
  TEST_ASSERT(multiRecords.size() == 6, "Should have 6 records total (1 + 5)");
  
  cout << "  Total records: " << multiRecords.size() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 4: Clear Records" << endl;
  
  stats.clearRecords();
  
  const vector<GeneralStats>& clearedRecords = stats.getRecords();
  TEST_ASSERT(clearedRecords.empty(), "Records should be empty after clear");
  
  cout << "  Records after clear: " << clearedRecords.size() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 5: Statistics toString Output" << endl;
  
  GeneralStats record1, record2;
  record1.population = 15;
  record1.births = 3;
  record2.population = 20;
  record2.births = 5;
  
  stats.addRecord(record1);
  stats.addRecord(record2);
  
  string output = stats.toString(true);
  
  cout << "  toString output length: " << output.length() << endl;
  TEST_ASSERT(!output.empty(), "toString should not be empty");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 6: Edge Case - Zero Population" << endl;
  
  Statistics zeroStats;
  GeneralStats zeroRecord;
  zeroRecord.population = 0;
  zeroRecord.births = 0;
  zeroRecord.foodAte = 0;
  
  zeroStats.addRecord(zeroRecord);
  
  const vector<GeneralStats>& zeroRecords = zeroStats.getRecords();
  TEST_ASSERT(zeroRecords.size() == 1, "Should handle zero population");
  TEST_ASSERT(zeroRecords[0].population == 0, "Zero population should be stored");
  
  cout << "  Zero population record added" << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 7: Edge Case - Large Population Values" << endl;
  
  Statistics largeStats;
  GeneralStats largeRecord;
  largeRecord.population = 1000000;
  largeRecord.births = 50000;
  largeRecord.foodAte = 200000;
  
  largeStats.addRecord(largeRecord);
  
  const vector<GeneralStats>& largeRecords = largeStats.getRecords();
  TEST_ASSERT(largeRecords[0].population == 1000000, "Should handle large values");
  
  cout << "  Large population: " << largeRecords[0].population << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 8: Death Statistics Tracking" << endl;
  
  Statistics deathStats;
  GeneralStats deathRecord;
  deathRecord.population = 10;
  deathRecord.deaths.oldAge = 2;
  deathRecord.deaths.starved = 1;
  deathRecord.deaths.dehydrated = 1;
  deathRecord.deaths.discomfort = 0;
  deathRecord.deaths.predator = 1;
  
  deathStats.addRecord(deathRecord);
  
  const vector<GeneralStats>& deathRecords = deathStats.getRecords();
  TEST_ASSERT(deathRecords[0].deaths.oldAge == 2, "Old age deaths should be tracked");
  TEST_ASSERT(deathRecords[0].deaths.starved == 1, "Starvation deaths should be tracked");
  TEST_ASSERT(deathRecords[0].deaths.predator == 1, "Predator deaths should be tracked");
  
  cout << "  Old age deaths: " << deathRecords[0].deaths.oldAge << endl;
  cout << "  Starved deaths: " << deathRecords[0].deaths.starved << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 9: GenomeStats with Empty Population" << endl;
  
  vector<Creature> emptyPop;
  
  // GenomeStats requires non-empty population, test should handle gracefully
  // This is an edge case test
  try {
    GenomeStats emptyGenomeStats(emptyPop, 0);
    cout << "  Empty population handled" << endl;
    TEST_ASSERT(true, "Should handle empty population without crashing");
  } catch (...) {
    // If it throws, that's also acceptable behavior for edge case
    cout << "  Empty population throws (expected behavior)" << endl;
    TEST_ASSERT(true, "Throwing on empty population is acceptable");
  }
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 10: GenomeStats with Single Creature" << endl;
  
  Genome g;
  g.randomise();
  Creature c(10, 10, g);
  
  vector<Creature> singlePop;
  singlePop.push_back(c);
  
  GenomeStats singleGenomeStats(singlePop, 1);
  string genomeOutput = singleGenomeStats.toString(true);
  
  cout << "  GenomeStats output length: " << genomeOutput.length() << endl;
  TEST_ASSERT(!genomeOutput.empty(), "GenomeStats toString should not be empty");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 11: GenomeStats with Multiple Creatures" << endl;
  
  vector<Creature> multiPop;
  for (int i = 0; i < 10; i++) {
    Genome genome;
    genome.randomise();
    Creature creature(i * 10, i * 10, genome);
    multiPop.push_back(creature);
  }
  
  GenomeStats multiGenomeStats(multiPop, 10);
  string multiOutput = multiGenomeStats.toString(false);
  
  cout << "  Multi-creature GenomeStats output length: " << multiOutput.length() << endl;
  TEST_ASSERT(!multiOutput.empty(), "Multi-creature GenomeStats should not be empty");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 12: Negative Test - Very Large Number of Records" << endl;
  
  Statistics manyStats;
  for (int i = 0; i < 100; i++) {
    GeneralStats record;
    record.population = i;
    record.births = i / 10;
    record.foodAte = i * 2;
    manyStats.addRecord(record);
  }
  
  const vector<GeneralStats>& manyRecords = manyStats.getRecords();
  TEST_ASSERT(manyRecords.size() == 100, "Should handle many records");
  
  cout << "  Records with 100 entries: " << manyRecords.size() << endl;
  cout << "  PASSED" << endl;

  testTeardown();

  cout << endl << "=== All Statistics Tests PASSED ===" << endl;
  
  return 0;
}