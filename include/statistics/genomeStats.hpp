#ifndef GENOME_STATS_H
#define GENOME_STATS_H

#include "../objects/creature/genome.hpp"
#include "../objects/creature/creature.hpp"

#include <vector>
#include <sstream>
#include <limits>
#include <math.h>

class GenomeStats {
  private:
    struct UIGeneStat {
      unsigned int max, min, mean, variance, stdDev;
      // Default values
      UIGeneStat () : max (0),
                    min (std::numeric_limits<unsigned int>::max())
                    {}
    };

    struct FGeneStat {
      float max, min, mean, variance, stdDev;
      // Default values
      FGeneStat () : max (std::numeric_limits<float>::min()),
                    min (std::numeric_limits<float>::max())
                    {}
    };

    struct SGenome {
      unsigned int  lifespan, sight, flee, pursue;
      float         hunger, thirst, mate, comfInc, comfDec;
    };

    unsigned int  time;
    UIGeneStat    lifespan, sight, flee, pursue;
    FGeneStat     hunger, thirst, mate, comfInc, comfDec;
    unsigned int  herbCount, carnCount;
    unsigned int  flocks, flees, none;
  
    void checkMaxValue  (unsigned int &current, unsigned int &value);
    void checkMinValue  (unsigned int &current, unsigned int &value);
    void checkMaxValue  (float &current,        float &value);
    void checkMinValue  (float &current,        float &value);
    void checkMaxValues (const Genome *g);
    void checkMinValues (const Genome *g);
    void setStdDev      (const size_t &size);
    void setVariances   (const std::vector<Creature> &c,
                         const SGenome &sums, const size_t &size);
    void accumulateSum  (SGenome &sums, const Genome *g);

  public:
    GenomeStats (const std::vector<Creature> &c, const unsigned int &time);

};

#endif
