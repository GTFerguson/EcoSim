/**
 * @file genomeStats.cpp
 * @brief Implementation of the GenomeStats class for genetic trait analysis.
 *
 * This file implements statistical analysis of genome traits across a creature population.
 * It calculates descriptive statistics (min, max, mean, variance, standard deviation) for
 * all genetic traits, enabling analysis of genetic diversity and evolutionary trends.
 *
 * The implementation uses a two-pass algorithm:
 * 1. First pass: Find min/max values and accumulate sums for means
 * 2. Second pass: Calculate variances using computed means
 * 3. Finally: Compute standard deviations from variances
 */

#include "../../include/statistics/genomeStats.hpp"

using std::vector;
using std::string;
using std::ostringstream;
using std::endl;
using std::sqrt;

GenomeStats::GenomeStats (const vector<Creature> &c, const unsigned int &t) : time(t) {
  SGenome sumValues;

  size_t cSize = c.size ();
  
  if (cSize == 0) return;

  for (size_t i = 0; i < cSize; i++) {
    const Creature& creature = c.at(i);

    checkMaxValues  (creature);
    checkMinValues  (creature);
    accumulateSum   (sumValues, creature);
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

void GenomeStats::checkMaxValues (const Creature &c) {
  checkMaxValue (lifespan.max, c.getLifespan());
  checkMaxValue (sight.max,    c.getSightRange());
  checkMaxValue (flee.max,     c.getFlee());
  checkMaxValue (pursue.max,   c.getPursue());
  checkMaxValue (hunger.max,   c.getTHunger());
  checkMaxValue (thirst.max,   c.getTThirst());
  checkMaxValue (mate.max,     c.getTMate());
  checkMaxValue (comfInc.max,  c.getComfInc());
  checkMaxValue (comfDec.max,  c.getComfDec());
}

void GenomeStats::checkMinValues (const Creature &c) {
  checkMinValue (lifespan.min, c.getLifespan());
  checkMinValue (sight.min,    c.getSightRange());
  checkMinValue (flee.min,     c.getFlee());
  checkMinValue (pursue.min,   c.getPursue());
  checkMinValue (hunger.min,   c.getTHunger());
  checkMinValue (thirst.min,   c.getTThirst());
  checkMinValue (mate.min,     c.getTMate());
  checkMinValue (comfInc.min,  c.getComfInc());
  checkMinValue (comfDec.min,  c.getComfDec());
}

void GenomeStats::accumulateSum (SGenome &sums, const Creature &c) {
  sums.lifespan += c.getLifespan();
  sums.sight    += c.getSightRange();
  sums.flee     += c.getFlee();
  sums.pursue   += c.getPursue();
  sums.hunger   += c.getTHunger();
  sums.thirst   += c.getTThirst();
  sums.mate     += c.getTMate();
  sums.comfInc  += c.getComfInc();
  sums.comfDec  += c.getComfDec();
}

void GenomeStats::setMeans (const SGenome &sums, const size_t &size) {
  lifespan.mean  = sums.lifespan / size;
  sight.mean     = sums.sight    / size;
  flee.mean      = sums.flee     / size;
  pursue.mean    = sums.pursue   / size;
  hunger.mean    = sums.hunger   / size;
  thirst.mean    = sums.thirst   / size;
  mate.mean      = sums.mate     / size;
  comfInc.mean   = sums.comfInc  / size;
  comfDec.mean   = sums.comfDec  / size;
}

void GenomeStats::setVariances (const std::vector<Creature> &c,
                                const SGenome &sums, const size_t &size) {
  for (size_t i = 0; i < c.size(); i++) {
    const Creature& creature = c.at(i);

    // Variance = sum of squared differences from mean
    float lifespanDiff = creature.getLifespan() - lifespan.mean;
    float sightDiff    = creature.getSightRange() - sight.mean;
    float fleeDiff     = creature.getFlee()     - flee.mean;
    float pursueDiff   = creature.getPursue()   - pursue.mean;
    float hungerDiff   = creature.getTHunger()  - hunger.mean;
    float thirstDiff   = creature.getTThirst()  - thirst.mean;
    float mateDiff     = creature.getTMate()    - mate.mean;
    float comfIncDiff  = creature.getComfInc()  - comfInc.mean;
    float comfDecDiff  = creature.getComfDec()  - comfDec.mean;

    lifespan.variance += lifespanDiff * lifespanDiff;
    sight.variance    += sightDiff    * sightDiff;
    flee.variance     += fleeDiff     * fleeDiff;
    pursue.variance   += pursueDiff   * pursueDiff;
    hunger.variance   += hungerDiff   * hungerDiff;
    thirst.variance   += thirstDiff   * thirstDiff;
    mate.variance     += mateDiff     * mateDiff;
    comfInc.variance  += comfIncDiff  * comfIncDiff;
    comfDec.variance  += comfDecDiff  * comfDecDiff;
  }

  // Divide by count to get variance
  lifespan.variance = lifespan.variance / size;
  sight.variance    = sight.variance    / size;
  flee.variance     = flee.variance     / size;
  pursue.variance   = pursue.variance   / size;
  hunger.variance   = hunger.variance   / size;
  thirst.variance   = thirst.variance   / size;
  mate.variance     = mate.variance     / size;
  comfInc.variance  = comfInc.variance  / size;
  comfDec.variance  = comfDec.variance  / size;
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

string GenomeStats::toString(bool includeHeader) const {
  ostringstream ss;
  
  // Add CSV header if requested
  if (includeHeader) {
    ss << "Time,Trait,Min,Max,Mean,Variance,StdDev" << endl;
  }
  
  // Output statistics for each trait
  ss << time << ",Lifespan," << lifespan.min << "," << lifespan.max << ","
     << lifespan.mean << "," << lifespan.variance << "," << lifespan.stdDev << endl;
  ss << time << ",Sight," << sight.min << "," << sight.max << ","
     << sight.mean << "," << sight.variance << "," << sight.stdDev << endl;
  ss << time << ",Flee," << flee.min << "," << flee.max << ","
     << flee.mean << "," << flee.variance << "," << flee.stdDev << endl;
  ss << time << ",Pursue," << pursue.min << "," << pursue.max << ","
     << pursue.mean << "," << pursue.variance << "," << pursue.stdDev << endl;
  ss << time << ",Hunger," << hunger.min << "," << hunger.max << ","
     << hunger.mean << "," << hunger.variance << "," << hunger.stdDev << endl;
  ss << time << ",Thirst," << thirst.min << "," << thirst.max << ","
     << thirst.mean << "," << thirst.variance << "," << thirst.stdDev << endl;
  ss << time << ",Mate," << mate.min << "," << mate.max << ","
     << mate.mean << "," << mate.variance << "," << mate.stdDev << endl;
  ss << time << ",ComfInc," << comfInc.min << "," << comfInc.max << ","
     << comfInc.mean << "," << comfInc.variance << "," << comfInc.stdDev << endl;
  ss << time << ",ComfDec," << comfDec.min << "," << comfDec.max << ","
     << comfDec.mean << "," << comfDec.variance << "," << comfDec.stdDev;
  
  return ss.str();
}
