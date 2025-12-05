/**
 *  Title   : Ecosim - Genome 
 *  Author  : Gary Ferguson
 *	Date	  : April 18th, 2020
 *	Purpose	: This class helps with the storage of any
 *            variables that are of an agents genetic makeup.
 */

// TODO Other Genes
// chance of wandering
// Shared at birth / incubation period?

#include "../../../include/objects/creature/genome.hpp"

using namespace std;

//================================================================================
//  Pseudo-Random Number Distributors
//================================================================================
uniform_int_distribution<unsigned short> Genome::spliceDist (1, 100);

//================================================================================
//  Gene Constants
//================================================================================
const unsigned  Genome::Limits::lifespan  [2] = { 100,    1000000 }; // max 1.9yrs
const unsigned  Genome::Limits::sight     [2] = { 5,      200     };
const unsigned  Genome::Limits::flock     [2] = { 2,      30      };
const unsigned  Genome::Limits::diet      [2] = { 0,      3       };
const float     Genome::Limits::needs     [2] = { 0.0f,   10.0f   };
const float     Genome::Limits::comfort   [2] = { 0.001f, 0.02f   };

const unsigned  Genome::Creep::lifespan = 1000;
const unsigned  Genome::Creep::flock    = 4;
const unsigned  Genome::Creep::sight    = 5;
const float     Genome::Creep::needs    = 0.05f;
const float     Genome::Creep::comfort  = 0.002f;

random_device Genome::_rd;
mt19937 Genome::_gen( Genome::_rd() );

//================================================================================
//  Constructors
//================================================================================
/**
 *  Our default constructor for making a generic starter creature
 */
Genome::Genome () {
  _lifespan       = 10000;
  _sight          = 50;
  _needs.hunger   = 5.0f;
  _needs.thirst   = 5.0f;
  _needs.fatigue  = 5.0f;
  _needs.mate     = 3.0f;
  _comfInc        = 0.004f;
  _comfDec        = 0.002f;
  _diet           = Diet::apple;  
  _flocks         = true;
  _flee           = 3;
  _pursue         = 6;
}

/**
 *  Parameterised Constructor for building a completely custom genome.
 *
 *  @param lifespan   The oldest a creature can get.
 *  @param t_hunger   The needs value before a creature can eat.
 *  @param t_thirst   The needs value before a creature can drink.
 *  @param t_mate     The needs value before a creature can seek a mate.
 *  @param comfInc    How much the mating variable increases when needs are met.
 *  @param comfDec    How much the mating variable decreases when needs are not met.
 *  @param sight      The maximum range a creature can see.
 *  @param diet       What the creature eats.
 *  @param flocks     Whether the creature exhibits flocking behaviour.
 *  @param flee       The range in which fleeing will be triggered.
 *  @param pursue     The range in which pursueing will be triggered.
 */
Genome::Genome (const unsigned int &lifespan,
                const float &t_hunger,
                const float &t_thirst,
                const float &t_fatigue,
                const float &t_mate,
                const float &comfInc,
                const float &comfDec,
                const unsigned int &sight,
                const Diet &diet,
                const bool &flocks,
                const unsigned int &flee, 
                const unsigned int &pursue) {
  _lifespan       = lifespan;
  _sight          = sight;
  _needs.hunger   = t_hunger;
  _needs.thirst   = t_thirst;
  _needs.fatigue  = t_fatigue;
  _needs.mate     = t_mate;
  _comfInc        = comfInc;
  _comfDec        = comfDec;
  _diet           = diet;
  _flocks         = flocks;
  _flee           = flee;
  _pursue         = pursue;
}

/**
 *  Parameterised Constructor for building a completely custom genome. This version
 *  takes the diet as a string, which is handy for when we load the genome from a file.
 *
 *  @param lifespan The oldest a creature can get.
 *  @param t_hunger The needs value before a creature can eat.
 *  @param t_thirst The needs value before a creature can drink.
 *  @param t_mate   The needs value before a creature can seek a mate.
 *  @param comfInc  How much the mating variable increases when needs are met.
 *  @param comfDec  How much the mating variable decreases when needs are not met.
 *  @param sight    The maximum range a creature can see.
 *  @param diet     What the creature eats.
 *  @param flocks   Whether the creature exhibits flocking behaviour.
 *  @param flee     The range in which fleeing will be triggered.
 *  @param pursue   The range in which pursueing will be triggered.
 */
Genome::Genome (const unsigned int &lifespan,
                const float &t_hunger,
                const float &t_thirst,
                const float &t_fatigue,
                const float &t_mate,
                const float &comfInc,
                const float &comfDec,
                const unsigned &sight,
                const string &diet,
                const bool &flocks,
                const unsigned &flee, 
                const unsigned &pursue) {
  _lifespan       = lifespan;
  _sight          = sight;
  _needs.hunger   = t_hunger;
  _needs.thirst   = t_thirst;
  _needs.fatigue  = t_fatigue;
  _needs.mate     = t_mate;
  _comfInc        = comfInc;
  _comfDec        = comfDec;
  _diet           = stringToDiet (diet);
  _flocks         = flocks;
  _flee           = flee;
  _pursue         = pursue;
}

//================================================================================
//  Getters
//================================================================================
unsigned Genome::getLifespan () const { return _lifespan;      }
unsigned Genome::getSight    () const { return _sight;         }
float    Genome::getTHunger  () const { return _needs.hunger;  }
float    Genome::getTThirst  () const { return _needs.thirst;  }
float    Genome::getTFatigue () const { return _needs.fatigue; }
float    Genome::getTMate    () const { return _needs.mate;    }
float    Genome::getComfInc  () const { return _comfInc;       }
float    Genome::getComfDec  () const { return _comfDec;       }
Diet     Genome::getDiet     () const { return _diet;          }
bool     Genome::ifFlocks    () const { return _flocks;        }
unsigned Genome::getFlee     () const { return _flee;          }
unsigned Genome::getPursue   () const { return _pursue;        }

//================================================================================
//  Setters
//================================================================================
// CREATURE-024 fix: Add validation against Limits in setters to prevent invalid gene values
void Genome::setLifespan  (unsigned lifespan)   {
  _lifespan = (lifespan < Limits::lifespan[0]) ? Limits::lifespan[0] :
              (lifespan > Limits::lifespan[1]) ? Limits::lifespan[1] : lifespan;
}
void Genome::setSight     (unsigned sight)      {
  _sight = (sight < Limits::sight[0]) ? Limits::sight[0] :
           (sight > Limits::sight[1]) ? Limits::sight[1] : sight;
}
void Genome::setTHunger   (float t_hunger)      {
  _needs.hunger = (t_hunger < Limits::needs[0]) ? Limits::needs[0] :
                  (t_hunger > Limits::needs[1]) ? Limits::needs[1] : t_hunger;
}
void Genome::setTThirst   (float t_thirst)      {
  _needs.thirst = (t_thirst < Limits::needs[0]) ? Limits::needs[0] :
                  (t_thirst > Limits::needs[1]) ? Limits::needs[1] : t_thirst;
}
void Genome::setTFatigue  (float t_fatigue)     {
  _needs.fatigue = (t_fatigue < Limits::needs[0]) ? Limits::needs[0] :
                   (t_fatigue > Limits::needs[1]) ? Limits::needs[1] : t_fatigue;
}
void Genome::setTMate     (float t_mate)        {
  _needs.mate = (t_mate < Limits::needs[0]) ? Limits::needs[0] :
                (t_mate > Limits::needs[1]) ? Limits::needs[1] : t_mate;
}
void Genome::setComfInc   (float comfInc)       {
  _comfInc = (comfInc < Limits::comfort[0]) ? Limits::comfort[0] :
             (comfInc > Limits::comfort[1]) ? Limits::comfort[1] : comfInc;
}
void Genome::setComfDec   (float comfDec)       {
  _comfDec = (comfDec < Limits::comfort[0]) ? Limits::comfort[0] :
             (comfDec > Limits::comfort[1]) ? Limits::comfort[1] : comfDec;
}
void Genome::setDiet      (Diet diet)           { _diet           = diet;       }
void Genome::setFlocks    (bool flocks)         { _flocks         = flocks;     }
void Genome::setFlee      (unsigned int flee)   {
  _flee = (flee < Limits::flock[0]) ? Limits::flock[0] :
          (flee > Limits::flock[1]) ? Limits::flock[1] : flee;
}
void Genome::setPursue    (unsigned int pursue) {
  _pursue = (pursue < Limits::flock[0]) ? Limits::flock[0] :
            (pursue > Limits::flock[1]) ? Limits::flock[1] : pursue;
}

/**
 *  Completely randomises all the genes.
 */
void Genome::randomise () {
  _lifespan       = mutation (Limits::lifespan);
  _sight          = mutation (Limits::sight);
  _needs.hunger   = mutation (Limits::needs);
  _needs.thirst   = mutation (Limits::needs);
  _needs.fatigue  = mutation (Limits::needs);
  _needs.mate     = mutation (Limits::needs);
  _comfInc        = mutation (Limits::comfort);
  _comfDec        = mutation (Limits::comfort);
  _diet           = Diet     (mutation(Limits::diet));
  // CREATURE-012 fix: Use mt19937 engine instead of rand() for consistent random quality
  std::uniform_int_distribution<int> boolDist(0, 1);
  _flocks         = static_cast<bool>(boolDist(_gen));
  _flee           = mutation (Limits::flock);
  _pursue         = mutation (Limits::flock);
}

//================================================================================
//  Genome Splicing
//================================================================================
int Genome::mutation (const int (&limits)[2]) {
  uniform_int_distribution<int> distribution(limits[0], limits[1]);
  return distribution (Genome::_gen);
}

unsigned int Genome::mutation (const unsigned (&limits)[2]) {
  uniform_int_distribution<unsigned> distribution(limits[0], limits[1]);
  return distribution (Genome::_gen);
}

float Genome::mutation (const float (&limits)[2]) {
  uniform_real_distribution<float> distribution(limits[0], limits[1]);
  return distribution (Genome::_gen);
}

/**
 *  This method takes in one parent gene and will return an
 *  offspring gene that has been changed within the set creep.
 *
 *  @param gene   The parent gene.
 *  @param creep  The limit of creeping.
 *  @param limits The hard set upper and lower boundaries of the gene.
 *  @return       The offspring gene.
 */
unsigned Genome::geneCreep (const unsigned &gene, const unsigned &creep,
                            const unsigned (&limits)[2]) {
  unsigned lowerLimit, upperLimit;
  if ((long)gene-creep < limits[0])   lowerLimit = limits[0];
  else                                lowerLimit = gene - creep;
  if (gene+creep > limits[1])         upperLimit = limits[1];
  else                                upperLimit = gene + creep;

  std::uniform_int_distribution<unsigned> distribution(lowerLimit, upperLimit);
  return distribution (Genome::_gen);
}

/**
 *  This method takes in one parent gene and will return an
 *  offspring gene that has been changed within the set creep.
 *
 *  @param gene   The parent gene.
 *  @param creep  The limit of creeping.
 *  @param limits The hard set upper and lower boundaries of the gene.
 *  @return       The offspring gene.
 */
float Genome::geneCreep (const float &gene, const float &creep, 
                         const float (&limits)[2]) {
  float lowerLimit, upperLimit;
  if (gene-creep < limits[0]) lowerLimit = limits[0];
  else                        lowerLimit = gene - creep;
  if (gene+creep > limits[1]) upperLimit = limits[1];
  else                        upperLimit = gene + creep;

  std::uniform_real_distribution<float> distribution(lowerLimit, upperLimit);
  return distribution (Genome::_gen);
}



/**
 *  This method creates a value for a new gene based on the parent genes provided.
 *
 *  @param gene1   Value taken from this genome.
 *  @param gene2   Value from the genome of the second parent.
 *  @param limits  Lower limit value of the gene.
 *  @return        The value for the new gene.
 */
unsigned int Genome::spliceGene (const unsigned &gene1, const unsigned &gene2, 
			                           const unsigned &creep, const unsigned (&limits)[2]) {
  unsigned method = spliceDist (Genome::_gen);

  //  Mutation
 if (method <= MUTATION_CHANCE) {
    return mutation(limits);

  } else if (method <= AVG_CHANCE) {
    unsigned avg = (gene1 + gene2) / 2;
   	return avg;

  } else if (method <= AVG_CREEP_CHANCE) {
    unsigned avg = (gene1 + gene2) / 2;
    return geneCreep (avg, creep, limits);

  } else if (method <= CREEP_ONE_CHANCE) {
    return geneCreep (gene1, creep, limits); 

  } else if (method <= CREEP_TWO_CHANCE) {
    return geneCreep (gene2, creep, limits); 

  //  Crossover
  }	else if (method <= CROSSOVER_CHANCE) {
    return gene1; 
  }	

  return gene2; 
}

/**
 *  This method creates a value for a new gene based on the parent genes provided.
 *
 *  @param gene1   Value taken from this genome.
 *  @param gene2   Value from the genome of the second parent.
 *  @param limits  Lower limit value of the gene.
 *  @return        The value for the new gene.
 */
float Genome::spliceGene (const float &gene1, const float &gene2, 
			 	                  const float &creep, const float (&limits)[2]) {
  unsigned method = spliceDist (Genome::_gen);

	if (method <= MUTATION_CHANCE) {
	   return mutation(limits);

	 } else if (method <= AVG_CHANCE) {
	   return (gene1 + gene2) / 2;

  } else if (method <= AVG_CREEP_CHANCE) {
    float avg = (gene1 + gene2) / 2;
    return geneCreep(avg, creep, limits); 

  } else if (method <= CREEP_ONE_CHANCE) {
    return geneCreep(gene1, creep, limits); 

  } else if (method <= CREEP_TWO_CHANCE) {
    return geneCreep(gene2, creep, limits); 

  //  Crossover Parent 1
  }	else if (method <= CROSSOVER_CHANCE) {
    return gene1; 
  }

  return gene2; 
}

/**
 *  This method creates a value for a new gene based on the parent genes provided.
 *
 *  @param gene1   Value taken from this genome.
 *  @param gene2   Value from the genome of the second parent.
 *  @param limits  Lower limit value of the gene.
 *  @return        The value for the new gene.
 */
bool Genome::spliceGene (const bool &gene1, const bool &gene2) {
  uniform_int_distribution<unsigned> disBool  (0, 1);
  unsigned method = spliceDist (Genome::_gen);
  if      (method <= MUTATION_CHANCE)       return (bool) disBool (Genome::_gen);
  else if (method <= BOOL_CROSSOVER_CHANCE) return gene1;
  else                                      return gene2;
}

/**
 *  This method technically doesn't splice the parents gene as there
 *  is no interbreeding between species with conflicting diets. This
 *  has kept the naming convention of similar methods for simplicities
 *  sake.
 *
 *  This simply allows for a small chance of a diet changing.
 *
 *  @return The value for the new gene.
 */
Diet Genome::spliceGene (const Diet &gene, const unsigned (&limits)[2]) {
  unsigned method = spliceDist (Genome::_gen);
  if (method <= DIET_MUTATION_CHANCE)   return Diet (mutation(limits));
  else                                  return gene;
}

/**
 *  This method takes two genomes and uses GA's to create a new 
 *  genome from them.
 *
 *  @param g2   The chosen mate.
 *  @return     Offspring genome.
 */
Genome Genome::spliceGenome (const Genome &g2) {
  Genome genome;

  genome.setLifespan (spliceGene (_lifespan,       g2.getLifespan(),
                                  Creep::lifespan, Limits::lifespan));
  genome.setTHunger  (spliceGene (_needs.hunger,   g2.getTHunger(),
                                  Creep::needs,    Limits::needs));
  genome.setTThirst  (spliceGene (_needs.thirst,   g2.getTThirst(),
                                  Creep::needs,    Limits::needs));
  genome.setTFatigue (spliceGene (_needs.fatigue,  g2.getTFatigue(),
                                  Creep::needs,    Limits::needs));
  genome.setTMate    (spliceGene (_needs.mate,     g2.getTMate(),
                                  Creep::needs,    Limits::needs));
  genome.setComfInc  (spliceGene (_comfInc,        g2.getComfInc(),
                                  Creep::comfort,  Limits::comfort));
  genome.setComfDec  (spliceGene (_comfDec,        g2.getComfDec(),
                                  Creep::comfort,  Limits::comfort));
  genome.setSight    (spliceGene (_sight,          g2.getSight(),
                                  Creep::sight,    Limits::sight));
  genome.setDiet     (spliceGene (_diet,           Limits::diet));
  genome.setFlocks   (spliceGene (_flocks,         g2.ifFlocks()));
  genome.setFlee     (spliceGene (_flee,           g2.getFlee(),
                                  Creep::flock,    Limits::flock));
  genome.setPursue   (spliceGene (_pursue,         g2.getPursue(),
                                  Creep::flock,    Limits::flock));

  return genome;
}

float Genome::geneSimilarity (const unsigned &g1, const unsigned &g2,
                              const unsigned (&limits)[2]) const {
  unsigned int oldRange = limits[1] - limits[0];
  float f_g1 = (float) (g1 - limits[0]) / oldRange;
  float f_g2 = (float) (g2 - limits[0]) / oldRange;

  return abs (f_g1 - f_g2);
}

float Genome::geneSimilarity (const float &g1, const float &g2,
                              const float (&limits)[2]) const {
  float oldRange = limits[1] - limits[0];
  float f_g1 = (g1-limits[0]) / oldRange;
  float f_g2 = (g2-limits[0]) / oldRange;
 
  return abs (f_g1-f_g2);
}

float Genome::geneSimilarity (const bool &g1, const bool &g2) const {
  if (g1 == g2) return 0.0f;
  else          return 1.0f;
}

/**
 *  Checks how similar this gene is to the one given. The similarity of the two
 *  genes is represented as a float value between 0 and 1. 
 *  0 being not at all alike, to 1 being identical.
 *
 *  @param g2 The gene to be compared to.
 *  @return   How similar they are as a value between 0 and 1.
 */
float Genome::compare (const Genome &g2) const {
  float similarity  = 0.0f;
        similarity += geneSimilarity (_lifespan,      g2.getLifespan(), Limits::lifespan);
        similarity += geneSimilarity (_needs.hunger,  g2.getTHunger(),  Limits::needs);
        similarity += geneSimilarity (_needs.thirst,  g2.getTThirst(),  Limits::needs);
        similarity += geneSimilarity (_needs.fatigue, g2.getTFatigue(), Limits::needs);
        similarity += geneSimilarity (_needs.mate,    g2.getTMate(),    Limits::needs);
        similarity += geneSimilarity (_comfInc,       g2.getComfInc(),  Limits::comfort);
        similarity += geneSimilarity (_comfDec,       g2.getComfDec(),  Limits::comfort);
        similarity += geneSimilarity (_sight,         g2.getSight(),    Limits::sight);
        similarity += geneSimilarity (_flocks,        g2.ifFlocks());
        similarity += geneSimilarity (_flee,          g2.getFlee(),     Limits::flock);
        similarity += geneSimilarity (_pursue,        g2.getPursue(),   Limits::flock);

  return  1.0f - similarity / 11;
}

//================================================================================
//  To String Methods
//================================================================================
Diet Genome::stringToDiet (const string &str) const {
  if      (str.compare("banana")    == 0)   return Diet::banana;
  else if (str.compare("apple")     == 0)   return Diet::apple;
  else if (str.compare("scavenger") == 0)   return Diet::scavenger;
  else                                      return Diet::predator;
}

string Genome::dietToString () const {
  switch (_diet) {
    case Diet::banana:    return "banana";
    case Diet::apple:     return "apple";
    case Diet::scavenger: return "scavenger";
    case Diet::predator:  return "predator";
    default:              return "error";
  }
}

/**
 *  Returns all data in the genome as a string separated by commas.
 *  @return The genome as string.
 */
string Genome::toString () const {
  ostringstream ss;
  ss  << _lifespan        << ","
      << _needs.hunger    << ","
      << _needs.thirst    << "," 
      << _needs.fatigue   << "," 
      << _needs.mate      << ","
      << _comfInc         << "," 
      << _comfDec         << ","
      << _sight           << "," 
      << dietToString ()  << ","
      << _flocks          << "," 
      << _flee            << ","
      << _pursue;

  return ss.str();
}
