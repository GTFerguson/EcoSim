#include "../../include/statistics/genomeStats.hpp"

using namespace std;

Genome::Genome (const vector<Creature> &c, const unsigned int &time) {
  SGenome sumValues;

  size_t cSize = c.size ();

  for (size_t i = 0; i < cSize; i++) {
    Genome *genome = c.at(i).getGenome();

    checkMaxValues  (gs, genome);
    checkMinValues  (gs, genome);
    accumulateSum   (sumValues, genome);
  }
  //  Means
  setMeans (sumValues, cSize);
  setVariances (c, sumValues, cSize);
  setStdDev (cSize);
}

void GenomeStats::checkMaxValue (unsigned int &current, const unsigned int &value) {
  if (value > current) current = value;
}

void GenomeStats::checkMaxValue (float &current, const float &value) {
  if (value > current) current = value;
}

void GenomeStats::checkMinValue (unsigned int &current, const unsigned int &value) {
  if (value < current) current = value;
}

void GenomeStats::checkMinValue (float &current, const float &value) {
  if (value < current) current = value;
}

void GenomeStats::checkMaxValues (const Genome *g) {
  checkMaxValue (lifespan.max, g->getLifespan());
  checkMaxValue (sight.max,    g->getSight()   );
  checkMaxValue (flee.max,     g->getFlee()    );
  checkMaxValue (pursue.max,   g->getPursue()  );
  checkMaxValue (hunger.max,   g->getHunger()  );
  checkMaxValue (thirst.max,   g->getThirst()  );
  checkMaxValue (mate.max,     g->getMate()    );
  checkMaxValue (comfInc.max,  g->getComfInc() );
  checkMaxValue (comfDec.max,  g->getComfDec() );
}

void GenomeStats::checkMinValues (const Genome *g) {
  checkMinValue (lifespan.min, g->getLifespan());
  checkMinValue (sight.min,    g->getSight()   );
  checkMinValue (flee.min,     g->getFlee()    );
  checkMinValue (pursue.min,   g->getPursue()  );
  checkMinValue (hunger.min,   g->getHunger()  );
  checkMinValue (thirst.min,   g->getThirst()  );
  checkMinValue (mate.min,     g->getMate()    );
  checkMinValue (comfInc.min,  g->getComfInc() );
  checkMinValue (comfDec.min,  g->getComfDec() );
}

void GenomeStats::accumulateSum (SGenome &sums, const Genome *g) {
  sums.lifespan += g.getLifespan  ();
  sums.sight    += g.getSight     ();
  sums.flee     += g.getFlee      (); 
  sums.pursue   += g.getPursue    (); 
  sums.hunger   += g.getHunger    (); 
  sums.thirst   += g.getThirst    (); 
  sums.mate     += g.getMate      (); 
  sums.comfInc  += g.getComfInc   (); 
  sums.comfDec  += g.getComfDec   (); 
}

void GenomeStats::setMeans (const SGenome &sums, const size_t &size) {
  lifespan.mean  = sums.lifespan.mean / size;
  sight.mean     = sums.sight.mean    / size;
  flee.mean      = sums.flee.mean     / size;
  pursue.mean    = sums.pursue.mean   / size;
  hunger.mean    = sums.hunger.mean   / size;
  thirst.mean    = sums.thirst.mean   / size;
  mate.mean      = sums.mate.mean     / size;
  comfInc.mean   = sums.comfInc.mean  / size;
  comfDec.mean   = sums.comfDec.mean  / size;
}

void GenomeStats::setVariances (const std::vector<Creature> &c,
                                const SGenome &sums, const size_t &size) {
  for (size_t i = 0; i < c.size(); i++) {
    Genome *genome = c.at(i).getGenome ();

    lifespan.variance += genome->getLifespan() - lifespan.mean;
    sight.variance    += genome->getSight()    - sight.mean;
    flee.variance     += genome->getFlee()     - flee.mean;
    pursue.variance   += genome->getPursue()   - pursue.mean;
    hunger.variance   += genome->getHunger()   - hunger.mean;
    thirst.variance   += genome->getThirst()   - thirst.mean;
    mate.variance     += genome->getMate()     - mate.mean;
    comfInc.variance  += genome->getComfInc()  - comfInc.mean;
    comfDec.variance  += genome->getComfDec()  - comfDec.mean;
  }

  lifespan.variance = sqrt (lifespan.variance) / size;
  sight.variance    = sqrt (sight.variance   ) / size;
  flee.variance     = sqrt (flee.variance    ) / size;
  pursue.variance   = sqrt (pursue.variance  ) / size;
  hunger.variance   = sqrt (hunger.variance  ) / size;
  thirst.variance   = sqrt (thirst.variance  ) / size;
  mate.variance     = sqrt (mate.variance    ) / size;
  comfInc.variance  = sqrt (comfInc.variance ) / size;
  comfDec.variance  = sqrt (comfDec.variance ) / size;
}

void GenomeStats::setStdDev (const size_t &size) {
  lifespan.stdDev = sqrt (lifespan.variance);
  sight.stdDev    = sqrt (sight.variance   ); 
  flee.stdDev     = sqrt (flee.variance    ); 
  pursue.stdDev   = sqrt (pursue.variance  ); 
  hunger.stdDev   = sqrt (hunger.variance  ); 
  thirst.stdDev   = sqrt (thirst.variance  ); 
  mate.stdDev     = sqrt (mate.variance    ); 
  comfInc.stdDev  = sqrt (comfInc.variance ); 
  comfDec.stdDev  = sqrt (comfDec.variance ); 
}
