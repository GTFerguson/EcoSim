#ifndef CREATURE_CONTAINER_H
#ifndef CREATURE_CONTAINER_H

#include "creature.hpp";

#include <vector>

class CreatureContainer {
  private:
    std::vector<Creature> _creatures;

  public:
    CreatureContainer ();

    void add    (const Creature &creature);
    void remove (const size_t &index);

    void 

};

#endif

