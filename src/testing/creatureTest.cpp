#include "../../include/objects/creature/creature.hpp"
#include "../../include/objects/creature/genome.hpp"
#include "../../include/objects/gameObject.hpp"

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

int main () {
  Genome genome, genome2;
  genome.randomise  ();
  genome2.randomise ();
  genome.setDiet  (Diet::apple);
  genome2.setDiet (Diet::apple);

  Creature creature  (10, 123, 5.8f, 6.4f, genome);
  Creature creature2 (120, 23, 5.4f, 6.0f, genome2);

  cout << endl << "RESOURCE SHARING" << endl;
  cout << "BEFORE"    << endl;
  cout << "PARENT 1"  << endl;
  cout << "Hunger : " << creature.getHunger()   << endl;
  cout << "Thirst : " << creature.getThirst()   << endl;
  cout << "Mate   : " << creature.getMate()     << endl;
  cout << "PARENT 2"  << endl;
  cout << "Hunger : " << creature2.getHunger()  << endl;
  cout << "Thirst : " << creature2.getThirst()  << endl;
  cout << "Mate   : " << creature2.getMate()    << endl;

  Creature offspring = creature.breedCreature (creature2);
  cout << endl << "AFTER" << endl;
  cout << "PARENT 1"  << endl;
  cout << "Hunger : " << creature.getHunger()   << endl;
  cout << "Thirst : " << creature.getThirst()   << endl;
  cout << "Mate   : " << creature.getMate()     << endl;
  cout << "PARENT 2"  << endl;
  cout << "Hunger : " << creature2.getHunger()  << endl;
  cout << "Thirst : " << creature2.getThirst()  << endl;
  cout << "Mate   : " << creature2.getMate()    << endl;
  cout << "OFFSPRING"  << endl;
  cout << "Hunger : " << offspring.getHunger()  << endl;
  cout << "Thirst : " << offspring.getThirst()  << endl;
  cout << "Mate   : " << offspring.getMate()    << endl;
  

  cout << endl << "FITNESS TESTING" << endl;
  cout << "Expected Result : 1.3 approx." << endl;
  cout << "Actual Result   : "<< creature.checkFitness(creature) << endl << endl;
  cout << "Expected Result : Should give a low value" << endl;
  cout << "Actual Result   : "<< creature.checkFitness(creature2) << endl;


  cout << endl << "BREEDING TESTING" << endl;
  bool alive = true;
  for (size_t i = 0; i < 5; i++) {
    Creature offspring = creature.breedCreature (creature2);
    cout << offspring.getGenome().toString() << endl;
  }

  cout << endl << "MOVEMENT COST" << endl;
  creature.setHunger (1.0f); creature.movementCost (0.0f);
  cout << "No Movement      : " << creature.getHunger() << endl;
  creature.setHunger (1.0f); creature.movementCost (1.4f);
  cout << "1 Diag           : " << creature.getHunger() << endl;
  creature.setHunger (1.0f); creature.movementCost (1.0f);
  cout << "1 Normal         : " << creature.getHunger() << endl;
  creature.setHunger (1.0f); creature.movementCost (2.4f);
  cout << "1 Diag 1 Normal  : " << creature.getHunger() << endl;
}
