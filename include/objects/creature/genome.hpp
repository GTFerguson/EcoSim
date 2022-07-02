#ifndef GENOME_H
#define GENOME_H

/**
 *  Title   : Ecosim - Genome 
 *  Author  : Gary Ferguson
 *	Date	  : April 18th, 2020
 *	Purpose	: This class helps with the storlifespan of any
 *            genetic variables along with their manipulation.
 */

#include <cstdlib>
#include <cmath>
#include <string>
#include <random>
#include <sstream>

//  Made it an enum class to allow more diets to be added easily
enum class Diet { banana = 0, apple = 1, scavenger = 2, predator = 3 };

class Genome {
  private:
    //  Pseudo-random number engine
    static std::random_device _rd;
    static std::mt19937       _gen;

    //  Used for generating a number to select which operator to use
    static std::uniform_int_distribution<unsigned short> spliceDist;

    // How often operators are used, first number is actual percent odd
    // the previous chance is added on so it can be used directly in if-else.
    const static unsigned short DIET_MUTATION_CHANCE  = 4;
    const static unsigned short MUTATION_CHANCE       = 2;
    const static unsigned short AVG_CHANCE            = 2 + MUTATION_CHANCE;
    const static unsigned short AVG_CREEP_CHANCE      = 2 + AVG_CHANCE;
    const static unsigned short CREEP_ONE_CHANCE      = 2 + AVG_CREEP_CHANCE;
    const static unsigned short CREEP_TWO_CHANCE      = 2 + CREEP_ONE_CHANCE;
    const static unsigned short CROSSOVER_CHANCE      = 
      (100 - CREEP_TWO_CHANCE) / 2 + CREEP_TWO_CHANCE;
    //  Variant for bools as they only use mutation and crossover.
    const static unsigned short BOOL_CROSSOVER_CHANCE =
      (100 - MUTATION_CHANCE) / 2 + MUTATION_CHANCE;

    // These constants are the upper and lower bounds of genes.
    struct Limits {
      const static unsigned lifespan  [2];
      const static unsigned sight     [2];
      const static unsigned flock     [2];
      const static unsigned diet      [2];
      const static float    needs     [2]; 
      const static float    comfort   [2];
    };
    
    //  How much in either direction a gene can change from creep
    struct Creep {
      const static unsigned int lifespan, sight, flock;
      const static float needs, comfort;
    };

    unsigned int _lifespan, _sight;
    struct Needs { float hunger, thirst, fatigue, mate; } _needs;
    float _comfInc, _comfDec;
    Diet  _diet;

    bool          _flocks;
    unsigned int  _flee, _pursue;

  public:
    //============================================================================
    //  Constructor
    //============================================================================
    Genome ();
    Genome (const unsigned &lifespan,
            const float &t_hunger,
            const float &t_thirst,
            const float &t_fatigue,
            const float &t_mate,
            const float &comfortInc,
            const float &comfortDec,
            const unsigned &sight,
            const Diet &diet,
            const bool &flock,
            const unsigned &flee,
            const unsigned &pursue);
    Genome (const unsigned &lifespan,
            const float &t_hunger,
            const float &t_thirst,
            const float &t_fatigue,
            const float &t_mate,
            const float &comfortInc,
            const float &comfortDec,
            const unsigned &sight,
            const std::string &diet,
            const bool &flock,
            const unsigned &flee,
            const unsigned &pursue);

    //============================================================================
    //  Getters
    //============================================================================
    unsigned int  getLifespan () const;
    unsigned int  getSight    () const;
    float         getTHunger  () const;
    float         getTThirst  () const;
    float         getTFatigue () const;
    float         getTMate    () const;
    float         getComfInc  () const; 
    float         getComfDec  () const; 
    Diet          getDiet     () const;
    bool          ifFlocks    () const;
    unsigned int  getFlee     () const;
    unsigned int  getPursue   () const;

    //============================================================================
    //  Setters
    //============================================================================
    void setLifespan  (unsigned int lifespan); 
    void setSight     (unsigned int sight); 
    void setTHunger   (float t_hunger); 
    void setTThirst   (float t_thirst); 
    void setTFatigue  (float t_fatigue); 
    void setTMate     (float t_mate); 
    void setComfInc   (float comfInc); 
    void setComfDec   (float comfDec); 
    void setDiet      (Diet diet);
    void setFlocks    (bool flocks);
    void setFlee      (unsigned int flee);
    void setPursue    (unsigned int pursue);
    void randomise    ();

    //============================================================================
    //  Genome Splicing
    //============================================================================
    int       mutation (const int (&limits)[2]);
    unsigned  mutation (const unsigned int (&limits)[2]);
    float     mutation (const float (&limits)[2]);

    unsigned geneCreep    (const unsigned &gene, const unsigned &creep,
                           const unsigned (&limits)[2]);
    float geneCreep       (const float &gene, const float &creep, 
                           const float (&limits)[2]);
    unsigned spliceGene   (const unsigned &gene1, const unsigned &gene2, 
			                     const unsigned &creep,
                           const unsigned (&limits)[2]);
    float spliceGene      (const float &gene1, const float &gene2, 
			                     const float &creep, const float (&limits)[2]);
    bool spliceGene       (const bool &gene1, const bool &gene2);
    Diet spliceGene       (const Diet &diet, const unsigned (&limits)[2]);
    Genome spliceGenome   (const Genome &g2);
    float geneSimilarity  (const unsigned &g1, const unsigned &g2,
                           const unsigned (&limits)[2]) const;
    float geneSimilarity  (const float &g1, const float &g2, 
                           const float (&limits)[2]) const;
    float geneSimilarity  (const bool &g1, const bool &g2) const;
    float compare         (const Genome &g2) const;

    //============================================================================
    //  To String
    //============================================================================
    Diet stringToDiet (const std::string &str) const;
    std::string dietToString () const;
    std::string toString () const;
};

#endif
