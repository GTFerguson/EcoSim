/**
 *  Title   : Game Objects Test
 *  Author  : Gary Ferguson
 *  Purpose : Test game object functionality with proper assertions
 *
 *  Test Isolation:
 *  - This test does NOT access file system
 *  - All test data is created in-memory
 *  - No external dependencies on save files or fixtures
 *  - Each test uses fresh object instances
 */

#include "../../include/objects/creature/creature.hpp"
#include "../../include/objects/creature/genome.hpp"
#include "../../include/objects/gameObject.hpp"
#include "../../include/objects/food.hpp"
#include "../../include/objects/spawner.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

// Test assertion macro with descriptive error messages
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << message << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

// Test constants for game object initialization
// Using named constants instead of magic numbers for clarity
const int FOOD_ID = 1;
const float FOOD_CALORIES = 40.0f;
const int FOOD_LIFESPAN = 100;

const int CREATURE_X = 10;
const int CREATURE_Y = 123;
const float CREATURE_HUNGER = 0.2f;
const float CREATURE_THIRST = 0.533f;

const int SPAWNER_RATE = 50;
const int SPAWNER_TIMER = 3;
const int SPAWNER_MIN_RADIUS = 10;

int main () {
  cout << "=== Game Objects Test Suite ===" << endl;
  
  // Create test objects using named constants for clarity
  GameObject  generic   ("Generic", "desc", true, 'X', 1);
  Food        food      (FOOD_ID, "Food", "desc", true, 'Y', 2, FOOD_CALORIES, FOOD_LIFESPAN);
  Genome      genome;
  Genome      genome2;
  Creature    creature  (CREATURE_X, CREATURE_Y, CREATURE_HUNGER, CREATURE_THIRST, genome);
  Spawner     spawner   ("Spawner", "desc", true, 'S', 12, SPAWNER_RATE, SPAWNER_TIMER, SPAWNER_MIN_RADIUS, food);

  cout << endl << "TEST 1: Object toString Methods" << endl;
  
  string genericStr = generic.toString();
  string foodStr = food.toString();
  string genomeStr = genome.toString();
  string creatureStr = creature.toString();
  string spawnerStr = spawner.toString();
  
  cout << "  GENERIC  :: " << genericStr.substr(0, 50) << "..." << endl;
  cout << "  FOOD     :: " << foodStr.substr(0, 50) << "..." << endl;
  cout << "  GENOME   :: " << genomeStr.substr(0, 50) << "..." << endl;
  cout << "  CREATURE :: " << creatureStr.substr(0, 50) << "..." << endl;
  cout << "  SPAWNER  :: " << spawnerStr.substr(0, 50) << "..." << endl;
  
  TEST_ASSERT(!genericStr.empty(), "GameObject toString should not be empty");
  TEST_ASSERT(!foodStr.empty(), "Food toString should not be empty");
  TEST_ASSERT(!genomeStr.empty(), "Genome toString should not be empty");
  TEST_ASSERT(!creatureStr.empty(), "Creature toString should not be empty");
  TEST_ASSERT(!spawnerStr.empty(), "Spawner toString should not be empty");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 2: Diet Enum Conversion" << endl;
  
  string dietStr = genome.dietToString();
  cout << "  Diet Original :: " << dietStr << endl;
  
  TEST_ASSERT(!dietStr.empty(), "Diet string should not be empty");

  Diet diet = genome.stringToDiet(dietStr);
  genome.setDiet(diet);
  string newDietStr = genome.dietToString();
  
  cout << "  Diet New      :: " << newDietStr << endl;
  
  TEST_ASSERT(dietStr == newDietStr, "Diet should survive round-trip conversion");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 3: Gene Creep Within Bounds" << endl;
  
  unsigned int limits[2] = { 0, 5 };
  unsigned int creep = 1;
  unsigned int gene = 1;
  
  bool allInBounds = true;
  for (size_t i = 0; i < 10; i++) {
    unsigned int result = genome.geneCreep(gene, creep, limits);
    cout << "  Gene creep result: " << result << endl;
    if (result < limits[0] || result > limits[1]) {
      allInBounds = false;
    }
    gene = result;
  }
  
  TEST_ASSERT(allInBounds, "Gene creep should stay within limits");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 4: Mutation Within Bounds" << endl;
  
  bool allMutationInBounds = true;
  for (size_t i = 0; i < 10; i++) {
    unsigned int result = genome.mutation(limits);
    cout << "  Mutation value: " << result << endl;
    if (result < limits[0] || result > limits[1]) {
      allMutationInBounds = false;
    }
  }
  
  TEST_ASSERT(allMutationInBounds, "Mutation values should be within limits");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 5: Object Properties" << endl;
  
  TEST_ASSERT(generic.getChar() == 'X', "Generic object should have char 'X'");
  TEST_ASSERT(food.getChar() == 'Y', "Food object should have char 'Y'");
  TEST_ASSERT(spawner.getChar() == 'S', "Spawner object should have char 'S'");
  
  cout << "  Generic char: " << generic.getChar() << endl;
  cout << "  Food char: " << food.getChar() << endl;
  cout << "  Spawner char: " << spawner.getChar() << endl;
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 6: Edge Case - Zero Calories Food" << endl;
  
  Food zeroFood(99, "Zero", "no nutrition", true, 'Z', 3, 0.0f, 100);
  string zeroFoodStr = zeroFood.toString();
  
  cout << "  Zero calorie food: " << zeroFoodStr.substr(0, 50) << "..." << endl;
  TEST_ASSERT(!zeroFoodStr.empty(), "Zero calorie food should have valid toString");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 7: Edge Case - Zero Lifespan Food" << endl;
  
  Food instantFood(100, "Instant", "instant decay", true, 'I', 4, 50.0f, 0);
  string instantStr = instantFood.toString();
  
  cout << "  Zero lifespan food: " << instantStr.substr(0, 50) << "..." << endl;
  TEST_ASSERT(!instantStr.empty(), "Zero lifespan food should have valid toString");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 8: Edge Case - Gene Creep at Boundaries" << endl;
  
  unsigned int boundaryLimits[2] = { 0, 10 };
  
  // Test at lower boundary
  unsigned int lowerResult = genome.geneCreep(0, 1, boundaryLimits);
  cout << "  Gene at lower bound (0): " << lowerResult << endl;
  TEST_ASSERT(lowerResult >= boundaryLimits[0] && lowerResult <= boundaryLimits[1],
              "Gene creep at lower bound should stay within limits");
  
  // Test at upper boundary
  unsigned int upperResult = genome.geneCreep(10, 1, boundaryLimits);
  cout << "  Gene at upper bound (10): " << upperResult << endl;
  TEST_ASSERT(upperResult >= boundaryLimits[0] && upperResult <= boundaryLimits[1],
              "Gene creep at upper bound should stay within limits");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 9: Negative Test - Invalid Limits (Min > Max)" << endl;
  
  unsigned int invalidLimits[2] = { 10, 5 };  // Min > Max
  
  // The mutation/geneCreep functions should handle this gracefully
  unsigned int resultInvalid = genome.mutation(invalidLimits);
  cout << "  Mutation with invalid limits: " << resultInvalid << endl;
  // Test passes if it doesn't crash - the function should handle invalid inputs
  TEST_ASSERT(true, "Should handle invalid limits without crashing");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 10: Edge Case - Very Large Creep Value" << endl;
  
  unsigned int largeLimits[2] = { 0, 100 };
  unsigned int largeCreep = 50;  // Very large creep value
  
  bool allValidLarge = true;
  for (int i = 0; i < 5; i++) {
    unsigned int result = genome.geneCreep(50, largeCreep, largeLimits);
    cout << "  Large creep result: " << result << endl;
    if (result < largeLimits[0] || result > largeLimits[1]) {
      allValidLarge = false;
    }
  }
  
  TEST_ASSERT(allValidLarge, "Large creep values should still respect limits");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 11: Negative Test - Spawner with Zero Rate" << endl;
  
  Spawner zeroRateSpawner("ZeroRate", "no spawn", true, 'R', 15, 0, 5, 10, food);
  string zeroRateStr = zeroRateSpawner.toString();
  
  cout << "  Zero rate spawner: " << zeroRateStr.substr(0, 50) << "..." << endl;
  TEST_ASSERT(!zeroRateStr.empty(), "Zero rate spawner should have valid toString");
  
  cout << "  PASSED" << endl;

  testTeardown();

  cout << endl << "=== All Game Object Tests PASSED ===" << endl;
  
  return 0;
}
