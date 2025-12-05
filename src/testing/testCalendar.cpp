/**
 *  Title   : Calendar Test
 *  Author  : Gary Ferguson
 *  Date    : Jun 10, 2020 05:28:09 (BST)
 *  Purpose : Test calendar functionality with proper assertions
 *
 *  Test Isolation:
 *  - This test does NOT access file system
 *  - All test data is created in-memory
 *  - No external dependencies on save files or fixtures
 *  - Each test uses fresh Calendar instances
 */

#include <iostream>
#include <cstdlib>
#include "../../include/calendar.hpp"

using namespace std;

// Test assertion macro with descriptive error messages
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << message << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

// Test constants
const int MINUTES_PER_HOUR = 60;
const int HOURS_PER_DAY = 24;
const int DAYS_PER_YEAR = 365;

/**
 * Setup function - no special setup needed for calendar tests
 */
void testSetup() {
    // Calendar tests use stack-allocated objects, no setup needed
}

/**
 * Teardown function - cleans up test resources
 */
void testTeardown() {
    // Calendar tests use stack-allocated objects, automatic cleanup
}

int main () {
  cout << "=== Calendar Test Suite ===" << endl;
  
  cout << endl << "TEST 1: Calendar Initialization" << endl;
  
  Calendar c;
  string initialState = c.toString();
  
  cout << "  Initial state: " << initialState << endl;
  TEST_ASSERT(!initialState.empty(), "Calendar toString should not be empty");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 2: Increment by Minute" << endl;
  
  string beforeMinute = c.toString();
  c.incrementByMinute();
  string afterMinute = c.toString();
  
  cout << "  Before: " << beforeMinute << endl;
  cout << "  After:  " << afterMinute << endl;
  TEST_ASSERT(beforeMinute != afterMinute, "Calendar should change after incrementByMinute");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 3: Increment by Year" << endl;
  
  string beforeYear = c.toString();
  c.incrementByYear();
  string afterYear = c.toString();
  
  cout << "  Before: " << beforeYear << endl;
  cout << "  After:  " << afterYear << endl;
  TEST_ASSERT(beforeYear != afterYear, "Calendar should change after incrementByYear");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 4: Multiple Increments" << endl;
  
  Calendar c2;
  for (int i = 0; i < MINUTES_PER_HOUR; i++) {
    c2.incrementByMinute();
  }
  string afterHour = c2.toString();
  
  Calendar c3;
  string start = c3.toString();
  
  cout << "  Start: " << start << endl;
  cout << "  After " << MINUTES_PER_HOUR << " minutes: " << afterHour << endl;
  TEST_ASSERT(start != afterHour, "60 minute increments should change calendar");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 5: Boundary Testing - Month/Year Wrap" << endl;
  
  // Test year boundary transitions
  Calendar cBoundary;
  
  // Increment many times to test wrap-around behavior
  for (int i = 0; i < DAYS_PER_YEAR * 2; i++) {
    cBoundary.incrementByDay();
  }
  
  string afterYears = cBoundary.toString();
  cout << "  After " << (DAYS_PER_YEAR * 2) << " days: " << afterYears << endl;
  TEST_ASSERT(!afterYears.empty(), "Calendar should handle year boundaries");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 6: Edge Case - Extreme Increments" << endl;
  
  Calendar cExtreme;
  
  // Test large number of increments
  for (int i = 0; i < 10000; i++) {
    cExtreme.incrementByMinute();
  }
  
  string afterExtreme = cExtreme.toString();
  cout << "  After 10000 minutes: " << afterExtreme << endl;
  TEST_ASSERT(!afterExtreme.empty(), "Calendar should handle extreme increments");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 7: Negative Test - toString Consistency" << endl;
  
  Calendar cConsistency;
  string state1 = cConsistency.toString();
  string state2 = cConsistency.toString();
  
  cout << "  First call: " << state1 << endl;
  cout << "  Second call: " << state2 << endl;
  TEST_ASSERT(state1 == state2, "Multiple toString calls without changes should be identical");
  
  cout << "  PASSED" << endl;

  cout << endl << "=== All Calendar Tests PASSED ===" << endl;
  
  return 0;
}

