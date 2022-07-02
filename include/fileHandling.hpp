#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

/**
 *	Title	  : Ecosim - File Handling
 *	Author	: Gary Ferguson
 *	Date	  : May 18th, 2019
 *	Purpose	: Handles saving and loading of simulation data to files.
 */

#include "objects/creature/creature.hpp"
#include "objects/gameObject.hpp"
#include "objects/food.hpp"
#include "objects/spawner.hpp"
#include "world/world.hpp"
#include "calendar.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

class FileHandling {
  private:
    //============================================================================
    //  File Paths
    //============================================================================
    const static std::string SAVE_DIR;
    const static std::string STAT_DIR;
    const static std::string GS_FILEPATH;
    const static std::string GENOME_FILEPATH;
    const static std::string WORLD_FILEPATH;
    const static std::string CREATURES_FILEPATH;
    const static std::string TEMP_STATS_FILEPATH;

    //============================================================================
    //  File Headers
    //============================================================================
    const static std::string statsHeader;
    const static std::string genomeHeader;

    std::string saveDir, statDir, genomeDir;

  public:
    //============================================================================
    //  Constructor
    //============================================================================
    FileHandling (const std::string &directory);

    //============================================================================
    //  Setters
    //============================================================================
    void changeDirectory (const std::string &directory); 

    //============================================================================
    //  Saving
    //============================================================================
    bool saveStatsHeader ();
    bool appendStats (const std::string &str);
    bool saveGenomes (const std::string &filename,
                      const std::vector<Creature> &creatures);
    bool saveState   (const World &w,
                      const std::vector<Creature> &c,
                      const Calendar &calendar,
                      const Statistics &stats);

    //============================================================================
    //  Loading
    //============================================================================
    GameObject loadGameObject  (const std::vector<std::string> &str,
                                unsigned int &start);
    Food       loadFood        (const std::vector<std::string> &str,
                                unsigned int &start);
    Calendar   loadCalendar    (const std::vector<std::string> &str, unsigned &start);
    bool       loadState       (World &w,
                                std::vector<Creature> &c,
                                Calendar &calendar,
                                Statistics &stats);
};

#endif
