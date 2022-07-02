#include "../../include/objects/creature/creature.hpp"
#include "../../include/objects/creature/genome.hpp"
#include "../../include/objects/gameObject.hpp"
#include "../../include/objects/food.hpp"
#include "../../include/objects/spawner.hpp"
#include "../../include/fileHandling.hpp"

#include <iostream>
#include <string>

using namespace std;

int main () {
  GameObject  generic   ("Generic", "desc", true, 'X', 1);
  Food        food      ("Food",    "desc", true, 'Y', 2, 40, 100);
  Genome      genome;
  Genome      genome2;
  Creature    creature  (10, 123, 0.2f, 0.533f, genome);
  Spawner     spawner   ("Spawner", "desc", true, 'S', 12, 50, 3, 10, food);

  cout << endl << "TO STRING TESTING" << endl;
  cout << "GENERIC  :: " << generic.toString()  << endl;
  cout << "FOOD     :: " << food.toString()     << endl;
  cout << "GENOME   :: " << genome.toString()   << endl;
  cout << "CREATURE :: " << creature.toString() << endl;
  cout << "SPAWNER  :: " << spawner.toString()  << endl;

  string str = genome.dietToString ();
  cout << endl << "DIET ENUM TESTING" << endl;
  cout << "Diet Original :: " << str << endl;

  Diet diet = genome.stringToDiet (str);
  genome.setDiet (diet);
  cout << "Diet New      :: " << genome.dietToString() << endl;

  cout << endl << "GENE CREEP TESTING" << endl;
  unsigned int limits [2] = { 0, 5 };
  unsigned int creep = 1;
  unsigned int gene = 1;

  for (size_t i = 0; i < 10; i++)
    cout << genome.geneCreep (gene, creep, limits) << endl;

  cout << endl << "GENE MUTATION TESTING" << endl;
  for (size_t i = 0; i < 10; i++)
    cout << genome.randomValue (limits) << endl;
  for (size_t i = 0; i < 10; i++)
    cout << genome2.randomValue (limits) << endl;
}
