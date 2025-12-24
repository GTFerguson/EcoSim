/**
 *	Title	  : Ecosim - File Handling
 *	Author	: Gary Ferguson
 *	Date	  : May 18th, 2019
 *	Purpose	: Handles saving and loading of simulation data to files.
 */

#include "../include/fileHandling.hpp"
#include <stdexcept>
#include <algorithm>

using namespace std;

namespace fs = std::filesystem;

//================================================================================
//  CSV Parsing Helper
//================================================================================
/**
 *  Parses a CSV line into a vector of string fields.
 *
 *  @param line   The CSV line to parse.
 *  @return       Vector of parsed fields.
 */
vector<string> FileHandling::parseCsvLine (const string &line) {
  vector<string> result;
  stringstream ss (line);
  string substr;
  
  while (getline(ss, substr, ',')) {
    result.push_back(substr);
  }
  
  return result;
}

//================================================================================
//  File Paths
//================================================================================
const string FileHandling::SAVE_DIR             = "saves/";
const string FileHandling::STAT_DIR             = "/stats";
const string FileHandling::GS_FILEPATH          = "/general_stats.csv";
const string FileHandling::GENOME_FILEPATH      = "/genomes/";
const string FileHandling::WORLD_FILEPATH       = "/world.txt";
const string FileHandling::CREATURES_FILEPATH   = "/creatures.txt";
const string FileHandling::TEMP_STATS_FILEPATH  = "/temp_stats.txt";

//================================================================================
//  File Headers
//================================================================================
const string FileHandling::statsHeader
  = "Time,Date,Population,Food Ate,Births,Old Age,Starved,Dehydrated,Discomfort,Predator";
const string FileHandling::genomeHeader
  = "Lifespan,Hunger,Thirst,Fatigue,Mate,Comfort Inc,Comf Dec,Sight Range,\
     Diet,If Flocks,Flee,Pursue";


//================================================================================
//  Constructor
//================================================================================
FileHandling::FileHandling (const string &directory) {
  changeDirectory (directory);
}

//================================================================================
//  Setters
//================================================================================
void FileHandling::changeDirectory (const string &directory) {
  saveDir   = SAVE_DIR  + directory;
  genomeDir = saveDir   + GENOME_FILEPATH; 
  statDir   = saveDir   + STAT_DIR;

  // Create directories if they don't already exist
  fs::create_directory (saveDir);
  fs::create_directory (statDir);
  fs::create_directory (genomeDir);
}

//================================================================================
//  Saving
//================================================================================
/**
 *  Used when creating a new statistics spreadsheet to insert the header row.
 *
 *  @return If the file was successfully opened and the header was added.
 */
bool FileHandling::saveStatsHeader () {
  ofstream file;
  file.open (statDir + GS_FILEPATH);

  if (file.is_open()) {
    file << statsHeader;
    file.close ();
    return true;
  }
  std::cerr << "Error: Failed to open stats header file: " << statDir + GS_FILEPATH << std::endl;
  return false;
}

/**
 *  This method appends to a given spreadsheet file a row of statistics.
 *
 *  @return If the file was successfully opened and statistics were appended.
 */
bool FileHandling::appendStats (const string &stats) {
  ofstream file;
  file.open (statDir + GS_FILEPATH, ofstream::out | ofstream::app);

  if (file.is_open()) {
    file << endl << stats;
    file.close ();
    return true;
  }
  std::cerr << "Error: Failed to open stats file for appending: " << statDir + GS_FILEPATH << std::endl;
  return false;
}

/**
 *  This saves all creatures genomes to a csv file so it can be viewed within
 *  spreadsheet software and analysed.
 *
 *  @param creatures  A vector of creatures who's genomes are to be saved.
 *  @return   If the genomes were successfully saved.
 */
bool FileHandling::saveGenomes (const string &filename,
                                const vector<Creature> &creatures) {
  const string filepath = genomeDir + filename;
  ofstream file (filepath);

  if (file.is_open()) {
    file << genomeHeader << endl;

    for (const Creature & creature : creatures) {
      file << creature.getGenome().toString() << endl;
    }

    file.close();
    return true;
  }
  std::cerr << "Error: Failed to open genome file: " << filepath << std::endl;
  return false;
}

/**
 *  Saves the current state of the simulation to files.
 *  Uses atomic save pattern: writes to temporary files first, then renames
 *  them on success to prevent partial/corrupted saves.
 *
 *  @param w        The world object.
 *  @param c        A vector of Creature objects.
 *  @param calendar An object that tracks the in-game date and time.
 *  @return         If the state was successfully saved.
 */
bool FileHandling::saveState (const World &w,
                              const vector<Creature> &creatures,
                              const Calendar &calendar,
                              const Statistics &stats) {
  //  Define temporary file paths
  const string worldTemp     = saveDir + WORLD_FILEPATH + ".tmp";
  const string creaturesTemp = saveDir + CREATURES_FILEPATH + ".tmp";
  const string statsTemp     = statDir + TEMP_STATS_FILEPATH + ".tmp";
  
  //  Save world data to temporary file
  ofstream file (worldTemp);
  if (file.is_open()) {
    file << calendar.toString() << ",";
    file << w.toString ();
    file.close ();
  } else {
    return false;
  }

  //  Save creature data to temporary file
  file.open (creaturesTemp);
  if (file.is_open()) {
    vector<Creature>::const_iterator it = creatures.begin();
    while (it != creatures.end()) {
      file << it++->toString();
      if (it != creatures.end()) file << endl;
    }
    file.close ();
  } else {
    //  Clean up partial save
    fs::remove (worldTemp);
    return false;
  }

  //  Save temporary statistics to temporary file
  file.open (statsTemp);
  if (file.is_open()) {
    file << stats.toString();
    file.close ();
  } else {
    //  Clean up partial save
    fs::remove (worldTemp);
    fs::remove (creaturesTemp);
    return false;
  }
  
  //  All files written successfully - now atomically rename them
  //  This ensures we either have a complete save or no changes at all
  try {
    fs::rename (worldTemp,     saveDir + WORLD_FILEPATH);
    fs::rename (creaturesTemp, saveDir + CREATURES_FILEPATH);
    fs::rename (statsTemp,     statDir + TEMP_STATS_FILEPATH);
  } catch (const fs::filesystem_error&) {
    //  Rename failed - clean up temporary files
    fs::remove (worldTemp);
    fs::remove (creaturesTemp);
    fs::remove (statsTemp);
    return false;
  }

  return true;
}

//================================================================================
//  Loading
//================================================================================
/**
 *  Loads the member variables of a general object into the parameters given.
 *
 *  @param str    The string to be parsed.
 *  @param start  Starting position for loading.
 *  @return       The game object parsed from the strings given.
 */
GameObject FileHandling::loadGameObject (const vector<string> &str,
                                         unsigned &start) {
  string   name   = str.at  (start++);
  string   desc   = str.at  (start++);
  
  //  Safe character extraction with bounds checking
  char ch = ' ';
  const string& charStr = str.at(start++);
  if (charStr.size() >= 2) {
    ch = charStr.at(1);
  } else if (charStr.size() == 1) {
    ch = charStr.at(0);
  } else {
    throw std::runtime_error("Invalid character field in save file");
  }
  
  unsigned colour = stoul   (str.at(start++));

  bool passable;
  //  Will set to false on invalid input
  istringstream (str.at(start++)) >> passable;

  // TODO better trimming
  //  Trimming whitespace and quotation marks
  if (name.size() >= 2) {
    name.erase (name.end()-1); name.erase(name.begin());
  }

  if (desc.size() >= 2) {
    desc.erase (desc.end()-1); desc.erase(desc.begin());
  }

  return GameObject (name, desc, passable, ch, colour);
}

/**
 *  Takes a string from a save file and loads a food object from it.
 *
 *  @param str    The string to be parsed.
 *  @param start  Starting position for loading.
 *  @return       The food object parsed from the strings given.
 */
Food FileHandling::loadFood (const vector<string> &str, unsigned &start) {
  GameObject  tempGO    = loadGameObject (str, start);
  unsigned    id        = stoul (str.at(start++));
  float       calories  = stof  (str.at(start++));
  unsigned    lifespan  = stoul (str.at(start++));
  unsigned    decay     = stoul (str.at(start++));

  return Food (id, tempGO.getName(), tempGO.getDesc(), tempGO.getPassable(), 
               tempGO.getChar(), tempGO.getColour(), calories, lifespan, decay);
}

Calendar FileHandling::loadCalendar (const vector<string> &str, unsigned &start) {
  Time time; Date date;
  time.hour   = stoul (str.at(start++));
  time.minute = stoul (str.at(start++));
  date.day    = stoul (str.at(start++));
  date.month  = stoul (str.at(start++));
  date.year   = stoul (str.at(start++));
  
  return Calendar (time, date);
}

//================================================================================
//  Load Helper Methods
//================================================================================
/**
 *  Loads world data from file.
 *
 *  @param w        The world object to be loaded into.
 *  @param calendar The calendar object to be loaded into.
 *  @return         True if world was successfully loaded.
 */
bool FileHandling::loadWorld (World &w, Calendar &calendar) {
  ifstream file (saveDir + WORLD_FILEPATH);

  if (!file.is_open()) {
    return false;
  }
  
  //  Coordinates of current objects being loaded
  int x = 0, y = 0;
  unsigned int lineCount = 0;

  string line;
  while (getline(file, line)) {
    if (!line.empty()) {
      vector<string> result = parseCsvLine(line);

      //  First line is general world data
      if (lineCount == 0 && result.size() == WORLD_HEADER_FIELDS) {
        unsigned index = 0;

        calendar = loadCalendar (result, index);

        MapGen mg;
        mg.seed     = (stod  (result.at(index++)));
        mg.scale    = (stod  (result.at(index++)));
        mg.freq     = (stod  (result.at(index++)));
        mg.exponent = (stod  (result.at(index++)));
        mg.terraces = (stoul (result.at(index++)));
        mg.rows     = (stoul (result.at(index++)));
        mg.cols     = (stoul (result.at(index++)));
        istringstream (result.at(index++)) >> mg.isIsland;
        w.setMapGen (mg);

        OctaveGen og;
        og.quantity     = stoul (result.at(index++));
        og.minWeight    = stod  (result.at(index++));
        og.maxWeight    = stod  (result.at(index++));
        og.freqInterval = stod  (result.at(index++));
        w.setOctaveGen (og);
        w.simplexGen ();

      } else {
        //  Tile coordinate
        if (result.size() == TILE_COORD_FIELDS) {
          x = stoi (result.at(0));
          y = stoi (result.at(1));

        //  Food object
        } else if (result.size() == FOOD_FIELDS) {
          unsigned int index = 0;
          w.addFood (x, y, loadFood(result, index));

        //  Spawner Object
        } else if (result.size() == SPAWNER_FIELDS) {
          unsigned index = 0;

          GameObject tempGO = loadGameObject (result, index);

          unsigned timer      = stoul (result.at(index++));
          unsigned rate       = stoul (result.at(index++));
          unsigned minRadius  = stoul (result.at(index++));
          unsigned maxRadius  = stoul (result.at(index++));

          Food food   (loadFood (result, index));
          Spawner spn (tempGO.getName(), tempGO.getDesc(), tempGO.getPassable(),
                       tempGO.getChar(), tempGO.getColour(), rate, timer,
                       minRadius, maxRadius, food);

          w.addSpawner (x, y, spn);
        }
      }

      lineCount++;
    }
  }
  file.close ();
  return true;
}

/**
 *  Loads creatures from file.
 *
 *  @param c  The vector of creatures to be loaded into.
 *  @return   True if creatures were successfully loaded.
 */
bool FileHandling::loadCreatures (vector<Creature> &c) {
  ifstream file (saveDir + CREATURES_FILEPATH);
  
  if (!file.is_open()) {
    return false;
  }
  
  string line;
  while (getline(file, line)) {
    //  Remove newline at end of string
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());

    if (!line.empty()) {
      vector<string> result = parseCsvLine(line);

      if (result.size() == CREATURE_FIELDS) {
        unsigned int index = 0;

        GameObject tempGO = loadGameObject (result, index);

        int       x           = stoi  (result.at(index++));
        int       y           = stoi  (result.at(index++));
        unsigned  age         = stoul (result.at(index++));
        string    direction   = result.at(index++);
        string    profile     = result.at(index++);
        float     hunger      = stof  (result.at(index++));
        float     thirst      = stof  (result.at(index++));
        float     fatigue     = stof  (result.at(index++));
        float     mate        = stof  (result.at(index++));
        float     metabolism  = stof  (result.at(index++));
        unsigned  speed       = stoul (result.at(index++));

        //  Genome Variables
        unsigned  lifespan    = stoul (result.at(index++));
        float     tHunger     = stof  (result.at(index++));
        float     tThirst     = stof  (result.at(index++));
        float     tFatigue    = stof  (result.at(index++));
        float     tMate       = stof  (result.at(index++));
        float     comfInc     = stof  (result.at(index++));
        float     comfDec     = stof  (result.at(index++));
        unsigned  sightRange  = stoul (result.at(index++));
        string    diet        = result.at(index++);
        bool      flocks;     istringstream (result.at(index++)) >> flocks;
        unsigned  flee        = stoul (result.at(index++));
        unsigned  pursue      = stoul (result.at(index));

        Genome g      (lifespan, tHunger, tThirst, tFatigue, tMate, comfInc, comfDec,
                       sightRange, diet, flocks, flee, pursue);
        Creature newC (tempGO.getName(), tempGO.getDesc(), tempGO.getPassable(),
                       tempGO.getChar(), tempGO.getColour(), x, y, age, profile,
                       direction, hunger, thirst, fatigue, mate, metabolism, speed, g);

        c.push_back (newC);
      }
    }
  }
  file.close ();
  return true;
}

/**
 *  Loads statistics from file.
 *
 *  @param stats  The statistics object to be loaded into.
 *  @return       True if statistics were successfully loaded.
 */
bool FileHandling::loadStats (Statistics &stats) {
  std::ifstream file (statDir + TEMP_STATS_FILEPATH);
  
  if (!file.is_open()) {
    return false;
  }
  
  std::string line;
  while (std::getline(file, line)) {
    //  Remove newline at end of string
    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

    if (!line.empty()) {
      std::vector<std::string> result = parseCsvLine(line);

      if (result.size() == STATS_FIELDS) {
        unsigned index = 0;

        Calendar c          = loadCalendar (result, index);
        unsigned pop        = std::stoul (result.at(index++));
        unsigned foodAte    = std::stoul (result.at(index++));
        unsigned births     = std::stoul (result.at(index++));
        unsigned oldAge     = std::stoul (result.at(index++));
        unsigned starved    = std::stoul (result.at(index++));
        unsigned dehydrated = std::stoul (result.at(index++));
        unsigned discomfort = std::stoul (result.at(index++));
        unsigned predators  = std::stoul (result.at(index++));

        DeathStats ds   = { oldAge, starved, dehydrated, discomfort, predators };
        GeneralStats gs = { c, pop, births, foodAte, 0, ds };  // 0 for feeding (legacy saves)

        stats.addRecord (gs);
      }
    }
  }
  file.close ();
  return true;
}

/**
 *  Loads a world state from a file.
 *
 *  @param w        The world object to be loaded into.
 *  @param c        The vector of creatures to be loaded into.
 *  @param calendar How many ticks the simulation has been running.
 *  @param stats    The statistics object to be loaded into.
 *  @return         True if state was successfully loaded.
 */
bool FileHandling::loadState (World &w,
                              std::vector<Creature> &c,
                              Calendar &calendar,
                              Statistics &stats) {
  //  Load world data first
  if (!loadWorld(w, calendar)) {
    return false;
  }

  //  Load creatures
  if (!loadCreatures(c)) {
    return false;
  }

  //  Load statistics
  if (!loadStats(stats)) {
    return false;
  }

  return true;
}
