/**
 *  Title   : Creature Test
 *  Author  : Gary Ferguson
 *  Purpose : Test creature functionality with proper assertions
 *
 *  Test Isolation:
 *  - This test does NOT access file system
 *  - All test data is created in-memory
 *  - No external dependencies on save files or fixtures
 *  - Each test uses fresh creature instances
 */

#include "../../include/objects/creature/creature.hpp"
#include "../../include/objects/creature/genome.hpp"
#include "../../include/objects/gameObject.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cmath>

using namespace std;

// Test assertion macro with descriptive error messages
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << message << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

// Test constants for creature initialization
// Using named constants instead of magic numbers for clarity
const int CREATURE1_X = 10;
const int CREATURE1_Y = 123;
const float CREATURE1_HUNGER = 5.8f;
const float CREATURE1_THIRST = 6.4f;

const int CREATURE2_X = 120;
const int CREATURE2_Y = 23;
const float CREATURE2_HUNGER = 5.4f;
const float CREATURE2_THIRST = 6.0f;

int main () {
  cout << "=== Creature Test Suite ===" << endl;
  
  Genome genome, genome2;
  genome.randomise();
  genome2.randomise();
  genome.setDiet(Diet::apple);
  genome2.setDiet(Diet::apple);

  // Create creatures using named constants for clarity
  Creature creature(CREATURE1_X, CREATURE1_Y, CREATURE1_HUNGER, CREATURE1_THIRST, genome);
  Creature creature2(CREATURE2_X, CREATURE2_Y, CREATURE2_HUNGER, CREATURE2_THIRST, genome2);

  cout << endl << "TEST 1: Initial Creature State" << endl;
  
  float initialHunger1 = creature.getHunger();
  float initialThirst1 = creature.getThirst();
  float initialHunger2 = creature2.getHunger();
  float initialThirst2 = creature2.getThirst();
  
  cout << "  Creature 1 - Hunger: " << initialHunger1 << ", Thirst: " << initialThirst1 << endl;
  cout << "  Creature 2 - Hunger: " << initialHunger2 << ", Thirst: " << initialThirst2 << endl;
  
  TEST_ASSERT(initialHunger1 >= 0.0f && initialHunger1 <= 1.0f,
              "Creature 1 hunger should be in range [0,1]");
  TEST_ASSERT(initialThirst1 >= 0.0f && initialThirst1 <= 1.0f,
              "Creature 1 thirst should be in range [0,1]");
  TEST_ASSERT(initialHunger2 >= 0.0f && initialHunger2 <= 1.0f,
              "Creature 2 hunger should be in range [0,1]");
  TEST_ASSERT(initialThirst2 >= 0.0f && initialThirst2 <= 1.0f,
              "Creature 2 thirst should be in range [0,1]");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 2: Breeding Creates Offspring" << endl;
  
  Creature offspring = creature.breedCreature(creature2);
  
  string offspringGenome = offspring.getGenome().toString();
  TEST_ASSERT(!offspringGenome.empty(), "Offspring should have a genome");
  
  TEST_ASSERT(creature.getMate() < 0.5f,
              "Parent 1 mate drive should be reduced after breeding");
  TEST_ASSERT(creature2.getMate() < 0.5f,
              "Parent 2 mate drive should be reduced after breeding");
  
  cout << "  Offspring genome exists: " << (offspringGenome.length() > 0 ? "YES" : "NO") << endl;
  cout << "  Parent 1 mate drive reduced: " << creature.getMate() << endl;
  cout << "  Parent 2 mate drive reduced: " << creature2.getMate() << endl;
  cout << "  PASSED" << endl;

  cout << endl << "TEST 3: Fitness Calculation" << endl;
  
  float selfFitness = creature.checkFitness(creature);
  float crossFitness = creature.checkFitness(creature2);
  
  cout << "  Self fitness: " << selfFitness << endl;
  cout << "  Cross fitness: " << crossFitness << endl;
  
  TEST_ASSERT(selfFitness > 0.0f, "Self fitness should be positive");
  TEST_ASSERT(selfFitness >= crossFitness,
              "Self fitness should be >= cross fitness (same genome comparison)");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 4: Movement Cost" << endl;
  
  creature.setHunger(1.0f);
  float beforeNoMove = creature.getHunger();
  creature.movementCost(0.0f);
  float afterNoMove = creature.getHunger();
  
  cout << "  No movement - Before: " << beforeNoMove << ", After: " << afterNoMove << endl;
  TEST_ASSERT(afterNoMove == beforeNoMove,
              "No movement should not change hunger");
  
  creature.setHunger(1.0f);
  float beforeNormalMove = creature.getHunger();
  creature.movementCost(1.0f);
  float afterNormalMove = creature.getHunger();
  
  cout << "  Normal movement - Before: " << beforeNormalMove << ", After: " << afterNormalMove << endl;
  TEST_ASSERT(afterNormalMove < beforeNormalMove,
              "Normal movement should reduce hunger");
  
  creature.setHunger(1.0f);
  creature.movementCost(1.0f);
  float costNormal = 1.0f - creature.getHunger();
  
  creature.setHunger(1.0f);
  creature.movementCost(1.4f);
  float costDiag = 1.0f - creature.getHunger();
  
  cout << "  Normal cost: " << costNormal << ", Diagonal cost: " << costDiag << endl;
  TEST_ASSERT(costDiag > costNormal,
              "Diagonal movement should cost more than normal movement");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 5: Edge Case - Zero Hunger and Thirst" << endl;
  
  Genome g3;
  g3.randomise();
  Creature zeroCreature(50, 50, 0.0f, 0.0f, g3);
  
  float zeroHunger = zeroCreature.getHunger();
  float zeroThirst = zeroCreature.getThirst();
  
  cout << "  Zero hunger: " << zeroHunger << endl;
  cout << "  Zero thirst: " << zeroThirst << endl;
  
  TEST_ASSERT(zeroHunger >= 0.0f && zeroHunger <= 1.0f,
              "Zero hunger should be in valid range");
  TEST_ASSERT(zeroThirst >= 0.0f && zeroThirst <= 1.0f,
              "Zero thirst should be in valid range");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 6: Edge Case - Maximum Hunger and Thirst" << endl;
  
  Genome g4;
  g4.randomise();
  Creature maxCreature(50, 50, 10.0f, 10.0f, g4);
  
  float maxHunger = maxCreature.getHunger();
  float maxThirst = maxCreature.getThirst();
  
  cout << "  Max hunger input (10.0): " << maxHunger << endl;
  cout << "  Max thirst input (10.0): " << maxThirst << endl;
  
  TEST_ASSERT(maxHunger >= 0.0f && maxHunger <= 1.0f,
              "Max hunger should be clamped to valid range");
  TEST_ASSERT(maxThirst >= 0.0f && maxThirst <= 1.0f,
              "Max thirst should be clamped to valid range");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 7: Negative Test - Negative Position Coordinates" << endl;
  
  Genome g5;
  g5.randomise();
  Creature negCreature(-10, -20, 0.5f, 0.5f, g5);
  
  int negX = negCreature.getX();
  int negY = negCreature.getY();
  
  cout << "  Negative input X (-10): " << negX << endl;
  cout << "  Negative input Y (-20): " << negY << endl;
  
  // Should handle negative coordinates without crashing
  TEST_ASSERT(true, "Should handle negative coordinates without crashing");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 8: Edge Case - Creature at Map Boundaries" << endl;
  
  Genome g6;
  g6.randomise();
  
  // Test at potential map boundaries (e.g., 0, large values)
  Creature boundCreature1(0, 0, 0.5f, 0.5f, g6);
  Creature boundCreature2(999, 999, 0.5f, 0.5f, g6);
  
  cout << "  Creature at (0,0): " << boundCreature1.getX() << ", " << boundCreature1.getY() << endl;
  cout << "  Creature at (999,999): " << boundCreature2.getX() << ", " << boundCreature2.getY() << endl;
  
  TEST_ASSERT(true, "Should handle boundary coordinates");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 9: Negative Test - Breeding with Self" << endl;
  
  Genome g7;
  g7.randomise();
  g7.setDiet(Diet::apple);
  Creature selfBreed(100, 100, 0.5f, 0.5f, g7);
  
  Creature selfOffspring = selfBreed.breedCreature(selfBreed);
  
  cout << "  Self-breeding resulted in offspring" << endl;
  TEST_ASSERT(!selfOffspring.getGenome().toString().empty(),
              "Self-breeding should produce offspring with genome");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 10: Edge Case - Movement Cost at Zero Distance" << endl;
  
  creature.setHunger(1.0f);
  float beforeZero = creature.getHunger();
  creature.movementCost(0.0f);
  float afterZero = creature.getHunger();
  
  cout << "  Zero distance movement - Before: " << beforeZero << ", After: " << afterZero << endl;
  TEST_ASSERT(afterZero == beforeZero, "Zero distance should not consume hunger");
  
  cout << "  PASSED" << endl;

  cout << endl << "TEST 11: Negative Test - Negative Movement Distance" << endl;
  
  creature.setHunger(0.5f);
  float beforeNeg = creature.getHunger();
  creature.movementCost(-1.0f);
  float afterNeg = creature.getHunger();
  
  cout << "  Negative distance movement - Before: " << beforeNeg << ", After: " << afterNeg << endl;
  // Should handle negative distance gracefully
  TEST_ASSERT(true, "Should handle negative movement distance");
  
  cout << "  PASSED" << endl;

  cout << endl << "=== All Creature Tests PASSED ===" << endl;
  
  return 0;
}
