#include "../../include/objects/creature/creature.hpp"
#include "../../include/world/world.hpp"
#include "../../include/fileHandling.hpp"

#include <iostream>
#include <string>

using namespace std;

#define ROWS 100
#define COLS 100

int main () {
	double 			  seed		  = 7.1231;
	double 			  scale		  = 0.0039;
	double 			  freq		  = 4.4; 
	double			  exponent	= 0.6;
	unsigned int 	terraces	= 64;
	World world (seed, scale, freq, exponent, terraces, ROWS, COLS);

  unsigned int clock = 10;
  vector<Creature> creatures;

  Genome g;
  g.randomise();

  Creature c (5, 5, g);

  creatures.push_back(c);

  cout << endl << "BEFORE SAVE/LOAD" << endl;
  cout << c.toString();

  FileHandling::saveState (world, creatures, clock);
  creatures.clear();
  FileHandling::loadState (world, creatures, clock);

  cout << endl << "AFTER SAVE/LOAD" << endl;
  cout << creatures.at(0).toString() << endl;

}
