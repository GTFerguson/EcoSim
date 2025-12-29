/**
 *  Title     : Ecosim - File Handling
 *  Author    : Gary Ferguson
 *  Date      : May 18th, 2019
 *  Updated   : December 2024
 *  Purpose   : Handles saving and loading of simulation data to files.
 *
 *  NOTE: Legacy CSV-based save/load code is kept for backward compatibility
 *        but is deprecated. New code should use the JSON-based methods.
 */

#include "../include/fileHandling.hpp"
#include "../include/genetics/defaults/PlantGenes.hpp"
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <optional>

using namespace std;
using json = nlohmann::json;

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

  // Create directories if they don't already exist (creates parent directories too)
  fs::create_directories (saveDir);
  fs::create_directories (statDir);
  fs::create_directories (genomeDir);
}

//================================================================================
//  Utility Methods
//================================================================================
void FileHandling::ensureSaveDirectory() const {
  // Create saves/ directory if it doesn't exist
  fs::create_directories(SAVE_DIR);
}

string FileHandling::getFullSavePath(const string& filename) const {
  ensureSaveDirectory();
  // Simple path: saves/<filename>.json
  // If filename already has .json extension, use as-is
  if (filename.size() >= 5 && filename.substr(filename.size() - 5) == ".json") {
    return SAVE_DIR + filename;
  }
  return SAVE_DIR + filename + ".json";
}

std::vector<std::string> FileHandling::listSaveFiles() const {
  ensureSaveDirectory();
  std::vector<std::string> saveFiles;
  
  try {
    for (const auto& entry : fs::directory_iterator(SAVE_DIR)) {
      if (entry.is_regular_file()) {
        std::string filename = entry.path().filename().string();
        // Only include .json files
        if (filename.size() >= 5 && filename.substr(filename.size() - 5) == ".json") {
          saveFiles.push_back(filename);
        }
      }
    }
    
    // Sort alphabetically
    std::sort(saveFiles.begin(), saveFiles.end());
  } catch (const fs::filesystem_error& e) {
    std::cerr << "Error listing save files: " << e.what() << std::endl;
  }
  
  return saveFiles;
}

bool FileHandling::saveFileExists(const string& filename) const {
  string fullPath = getFullSavePath(filename);
  return fs::exists(fullPath);
}

string FileHandling::generateTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  
  std::tm tm_utc;
#ifdef _WIN32
  gmtime_s(&tm_utc, &time_t_now);
#else
  gmtime_r(&time_t_now, &tm_utc);
#endif
  
  std::ostringstream oss;
  oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%SZ");
  return oss.str();
}

//================================================================================
//  Saving - Statistics (kept from legacy system)
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
      // Use new genetics system - output genome chromosome data
      // Output basic genome stats for analysis
      file << creature.getLifespan() << ","
           << creature.getTHunger() << ","
           << creature.getTThirst() << ","
           << creature.getTFatigue() << ","
           << creature.getTMate() << ","
           << creature.getComfInc() << ","
           << creature.getComfDec() << ","
           << creature.getSightRange() << ","
           << static_cast<int>(creature.getDietType()) << ","
           << (creature.ifFlocks() ? 1 : 0) << ","
           << creature.getFlee() << ","
           << creature.getPursue() << endl;
    }

    file.close();
    return true;
  }
  std::cerr << "Error: Failed to open genome file: " << filepath << std::endl;
  return false;
}

//================================================================================
//  Saving - Legacy CSV Format (deprecated)
//================================================================================
/**
 *  Saves the current state of the simulation to files.
 *  Uses atomic save pattern: writes to temporary files first, then renames
 *  them on success to prevent partial/corrupted saves.
 *
 *  @param w        The world object.
 *  @param c        A vector of Creature objects.
 *  @param calendar An object that tracks the in-game date and time.
 *  @return         If the state was successfully saved.
 *
 *  @deprecated Use saveGameJson() instead for new save/load functionality.
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
//  Saving - New JSON Format
//================================================================================
bool FileHandling::saveGameJson(
    const std::string& filepath,
    const std::vector<Creature>& creatures,
    const World& world,
    const Calendar& calendar,
    unsigned currentTick,
    int mapWidth,
    int mapHeight
) {
  try {
    // Build the root JSON object
    json saveData;
    
    // Version and metadata
    saveData["version"] = SAVE_VERSION;
    saveData["savedAt"] = generateTimestamp();
    
    // World state with generation parameters for terrain regeneration
    MapGen mg = world.getMapGen();
    OctaveGen og = world.getOctaveGen();
    
    saveData["world"] = {
      {"tick", currentTick},
      {"mapWidth", mapWidth},
      {"mapHeight", mapHeight},
      {"mapGen", {
        {"seed", mg.seed},
        {"scale", mg.scale},
        {"freq", mg.freq},
        {"exponent", mg.exponent},
        {"terraces", mg.terraces},
        {"rows", mg.rows},
        {"cols", mg.cols},
        {"isIsland", mg.isIsland}
      }},
      {"octaveGen", {
        {"quantity", og.quantity},
        {"minWeight", og.minWeight},
        {"maxWeight", og.maxWeight},
        {"freqInterval", og.freqInterval}
      }}
    };
    
    // Calendar state
    saveData["calendar"] = {
      {"minute", calendar.getMinute()},
      {"hour", calendar.getHour()},
      {"day", calendar.getDay()},
      {"month", calendar.getMonth()},
      {"year", calendar.getYear()}
    };
    
    // Serialize all creatures
    json creaturesArray = json::array();
    for (const auto& creature : creatures) {
      creaturesArray.push_back(creature.toJson());
    }
    saveData["creatures"] = creaturesArray;
    
    // Serialize all plants from all tiles
    json plantsArray = json::array();
    const auto& grid = const_cast<World&>(world).getGrid();
    for (unsigned x = 0; x < grid.size(); ++x) {
      for (unsigned y = 0; y < grid[x].size(); ++y) {
        const auto& plants = grid[x][y].getPlants();
        for (const auto& plantPtr : plants) {
          if (plantPtr && plantPtr->isAlive()) {
            plantsArray.push_back(plantPtr->toJson());
          }
        }
      }
    }
    saveData["plants"] = plantsArray;
    
    // Write to temporary file first (atomic save pattern)
    const string fullPath = getFullSavePath(filepath);
    const string tempPath = fullPath + ".tmp";
    
    ofstream file(tempPath);
    if (!file.is_open()) {
      std::cerr << "Error: Failed to open temp save file: " << tempPath << std::endl;
      return false;
    }
    
    // Write with pretty formatting for readability
    file << saveData.dump(2);
    file.close();
    
    if (file.fail()) {
      std::cerr << "Error: Failed to write save data to: " << tempPath << std::endl;
      fs::remove(tempPath);
      return false;
    }
    
    // Atomically rename temp file to final destination
    try {
      fs::rename(tempPath, fullPath);
    } catch (const fs::filesystem_error& e) {
      std::cerr << "Error: Failed to rename save file: " << e.what() << std::endl;
      fs::remove(tempPath);
      return false;
    }
    
    std::cout << "[Save] Game saved successfully to: " << fullPath << std::endl;
    std::cout << "       Creatures: " << creatures.size() 
              << ", Plants: " << plantsArray.size() 
              << ", Tick: " << currentTick << std::endl;
    
    return true;
    
  } catch (const json::exception& e) {
    std::cerr << "Error: JSON serialization failed: " << e.what() << std::endl;
    return false;
  } catch (const std::exception& e) {
    std::cerr << "Error: Save failed: " << e.what() << std::endl;
    return false;
  }
}

//================================================================================
//  Loading - Legacy CSV Format (deprecated)
//================================================================================
/**
 *  Loads the member variables of a general object into the parameters given.
 *
 *  @param str    The string to be parsed.
 *  @param start  Starting position for loading.
 *  @return       The game object parsed from the strings given.
 *
 *  @deprecated Use loadGameJson() instead.
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
 *  @deprecated Use loadGameJson() instead.
 */
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
//  Load Helper Methods - Legacy (deprecated)
//================================================================================
/**
 *  Loads world data from file.
 *  NOTE: Legacy Food/Spawner loading has been removed.
 *
 *  @param w        The world object to be loaded into.
 *  @param calendar The calendar object to be loaded into.
 *  @return         True if world was successfully loaded.
 *
 *  @deprecated Use loadGameJson() instead.
 */
bool FileHandling::loadWorld (World &w, Calendar &calendar) {
  ifstream file (saveDir + WORLD_FILEPATH);

  if (!file.is_open()) {
    return false;
  }
  
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
      }
      // NOTE: Legacy Food/Spawner loading removed - new genetics system 
      // uses Plant objects which are handled separately

      lineCount++;
    }
  }
  file.close ();
  return true;
}

/**
 *  Loads creatures from file.
 *  NOTE: This now creates creatures with the new genetics system.
 *        Legacy save files may not load correctly.
 *
 *  @param c  The vector of creatures to be loaded into.
 *  @return   True if creatures were successfully loaded.
 *
 *  @deprecated Use loadGameJson() instead.
 */
bool FileHandling::loadCreatures (vector<Creature> &c) {
  ifstream file (saveDir + CREATURES_FILEPATH);
  
  if (!file.is_open()) {
    return false;
  }
  
  // Ensure gene registry is initialized
  Creature::initializeGeneRegistry();
  auto& registry = Creature::getGeneRegistry();
  
  string line;
  while (getline(file, line)) {
    //  Remove newline at end of string
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());

    if (!line.empty()) {
      vector<string> result = parseCsvLine(line);

      if (result.size() == CREATURE_FIELDS) {
        unsigned int index = 0;

        [[maybe_unused]] GameObject tempGO = loadGameObject (result, index);

        int       x           = stoi  (result.at(index++));
        int       y           = stoi  (result.at(index++));
        [[maybe_unused]] unsigned  age         = stoul (result.at(index++));
        [[maybe_unused]] string    direction   = result.at(index++);
        [[maybe_unused]] string    profile     = result.at(index++);
        float     hunger      = stof  (result.at(index++));
        float     thirst      = stof  (result.at(index++));
        [[maybe_unused]] float     fatigue     = stof  (result.at(index++));
        [[maybe_unused]] float     mate        = stof  (result.at(index++));
        [[maybe_unused]] float     metabolism  = stof  (result.at(index++));
        [[maybe_unused]] unsigned  speed       = stoul (result.at(index++));

        // Skip legacy genome fields - they're no longer used
        // Create creature with new genetics system
        auto genome = std::make_unique<EcoSim::Genetics::Genome>();
        Creature newC(x, y, hunger, thirst, std::move(genome));

        c.push_back(std::move(newC));
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
 *
 *  @deprecated Use loadGameJson() instead.
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

        Calendar cal        = loadCalendar (result, index);
        unsigned pop        = std::stoul (result.at(index++));
        unsigned foodAte    = std::stoul (result.at(index++));
        unsigned births     = std::stoul (result.at(index++));
        unsigned oldAge     = std::stoul (result.at(index++));
        unsigned starved    = std::stoul (result.at(index++));
        unsigned dehydrated = std::stoul (result.at(index++));
        unsigned discomfort = std::stoul (result.at(index++));
        unsigned predators  = std::stoul (result.at(index++));

        DeathStats ds   = { oldAge, starved, dehydrated, discomfort, predators };
        GeneralStats gs = { cal, pop, births, foodAte, 0, ds };  // 0 for feeding (legacy saves)

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
 *
 *  @deprecated Use loadGameJson() instead.
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

//================================================================================
//  Loading - New JSON Format
//================================================================================
bool FileHandling::loadGameJson(
    const std::string& filepath,
    std::vector<Creature>& creatures,
    World& world,
    Calendar& calendar,
    unsigned& currentTick,
    int mapWidth,
    int mapHeight
) {
  try {
    const string fullPath = getFullSavePath(filepath);
    
    // Check if file exists
    if (!fs::exists(fullPath)) {
      std::cerr << "Error: Save file not found: " << fullPath << std::endl;
      return false;
    }
    
    // Read the file
    ifstream file(fullPath);
    if (!file.is_open()) {
      std::cerr << "Error: Failed to open save file: " << fullPath << std::endl;
      return false;
    }
    
    // Parse JSON
    json saveData;
    try {
      file >> saveData;
    } catch (const json::parse_error& e) {
      std::cerr << "Error: Failed to parse save file JSON: " << e.what() << std::endl;
      return false;
    }
    file.close();
    
    // Version check
    int version = saveData.value("version", 0);
    if (version != SAVE_VERSION) {
      std::cerr << "Error: Incompatible save file version. Expected " 
                << SAVE_VERSION << ", got " << version << std::endl;
      return false;
    }
    
    // Validate world dimensions
    const auto& worldData = saveData["world"];
    int savedMapWidth = worldData.value("mapWidth", 0);
    int savedMapHeight = worldData.value("mapHeight", 0);
    
    if (savedMapWidth != mapWidth || savedMapHeight != mapHeight) {
      std::cerr << "Warning: Map dimensions mismatch. Save: " 
                << savedMapWidth << "x" << savedMapHeight
                << ", Current: " << mapWidth << "x" << mapHeight << std::endl;
      // Continue loading - positions will be clamped
    }
    
    // Load tick
    currentTick = worldData.value("tick", 0u);
    
    // Load calendar
    const auto& calData = saveData["calendar"];
    Time time;
    time.minute = calData.value("minute", static_cast<unsigned short>(0));
    time.hour = calData.value("hour", static_cast<unsigned short>(0));
    Date date;
    date.day = calData.value("day", static_cast<unsigned short>(1));
    date.month = calData.value("month", static_cast<unsigned short>(1));
    date.year = calData.value("year", 1u);
    calendar = Calendar(time, date);
    
    // Load and apply world generation parameters to regenerate identical terrain
    // Use current world dimensions for terrain generation to handle dimension mismatches
    // (seed and other parameters are preserved to generate identical terrain patterns)
    if (worldData.contains("mapGen")) {
      const auto& mapGenData = worldData["mapGen"];
      MapGen mg;
      mg.seed = mapGenData.value("seed", 0.0);
      mg.scale = mapGenData.value("scale", 0.01);
      mg.freq = mapGenData.value("freq", 1.0);
      mg.exponent = mapGenData.value("exponent", 1.0);
      mg.terraces = mapGenData.value("terraces", 20u);
      // Use current world dimensions, not saved ones (handles dimension mismatch safely)
      mg.rows = static_cast<unsigned>(mapHeight);
      mg.cols = static_cast<unsigned>(mapWidth);
      mg.isIsland = mapGenData.value("isIsland", false);
      world.setMapGen(mg);
    }
    
    if (worldData.contains("octaveGen")) {
      const auto& octaveGenData = worldData["octaveGen"];
      OctaveGen og;
      og.quantity = octaveGenData.value("quantity", 4u);
      og.minWeight = octaveGenData.value("minWeight", 0.1);
      og.maxWeight = octaveGenData.value("maxWeight", 0.5);
      og.freqInterval = octaveGenData.value("freqInterval", 1.0);
      world.setOctaveGen(og);
    }
    
    // Regenerate terrain with the saved parameters
    world.simplexGen();
    std::cout << "       Terrain regenerated with seed: " << world.getSeed() << std::endl;
    
    // Clear existing creatures
    creatures.clear();
    
    // Ensure gene registry is initialized
    Creature::initializeGeneRegistry();
    
    // Load creatures
    const auto& creaturesArray = saveData["creatures"];
    for (const auto& creatureJson : creaturesArray) {
      try {
        Creature creature = Creature::fromJson(creatureJson, mapWidth, mapHeight);
        creatures.push_back(std::move(creature));
      } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to load creature: " << e.what() << std::endl;
        // Continue loading other creatures
      }
    }
    
    // Initialize plant system if needed
    if (!world.hasGeneticsPlants()) {
      world.initializeGeneticsPlants();
    }
    
    // Get plant registry for loading
    auto plantRegistry = world.getPlantRegistry();
    if (!plantRegistry) {
      std::cerr << "Error: Plant registry not available" << std::endl;
      return false;
    }
    
    // Clear existing plants from all tiles
    auto& grid = world.getGrid();
    for (auto& column : grid) {
      for (auto& tile : column) {
        tile.getPlants().clear();
      }
    }
    
    // Load plants
    const auto& plantsArray = saveData["plants"];
    int plantsLoaded = 0;
    for (const auto& plantJson : plantsArray) {
      try {
        EcoSim::Genetics::Plant plant = EcoSim::Genetics::Plant::fromJson(plantJson, *plantRegistry);
        
        // Get plant position
        int x = plant.getX();
        int y = plant.getY();
        
        // Clamp to world bounds
        x = std::max(0, std::min(x, mapWidth - 1));
        y = std::max(0, std::min(y, mapHeight - 1));
        
        // Add to appropriate tile
        auto plantPtr = std::make_shared<EcoSim::Genetics::Plant>(std::move(plant));
        if (grid[x][y].addPlant(plantPtr)) {
          plantsLoaded++;
        }
      } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to load plant: " << e.what() << std::endl;
        // Continue loading other plants
      }
    }
    
    std::cout << "[Load] Game loaded successfully from: " << fullPath << std::endl;
    std::cout << "       Creatures: " << creatures.size() 
              << ", Plants: " << plantsLoaded 
              << ", Tick: " << currentTick << std::endl;
    
    return true;
    
  } catch (const json::exception& e) {
    std::cerr << "Error: JSON parsing failed: " << e.what() << std::endl;
    return false;
  } catch (const std::exception& e) {
    std::cerr << "Error: Load failed: " << e.what() << std::endl;
    return false;
  }
}

//================================================================================
//  Metadata Query
//================================================================================
std::optional<FileHandling::SaveMetadata> FileHandling::getSaveMetadata(
    const std::string& filepath
) const {
  try {
    const string fullPath = getFullSavePath(filepath);
    
    // Check if file exists
    if (!fs::exists(fullPath)) {
      return std::nullopt;
    }
    
    // Read the file
    ifstream file(fullPath);
    if (!file.is_open()) {
      return std::nullopt;
    }
    
    // Parse JSON
    json saveData;
    try {
      file >> saveData;
    } catch (const json::parse_error&) {
      return std::nullopt;
    }
    file.close();
    
    // Extract metadata
    SaveMetadata metadata;
    metadata.version = saveData.value("version", 0);
    metadata.savedAt = saveData.value("savedAt", "unknown");
    
    if (saveData.contains("world")) {
      metadata.tick = saveData["world"].value("tick", 0u);
    } else {
      metadata.tick = 0;
    }
    
    if (saveData.contains("creatures") && saveData["creatures"].is_array()) {
      metadata.creatureCount = static_cast<int>(saveData["creatures"].size());
    } else {
      metadata.creatureCount = 0;
    }
    
    if (saveData.contains("plants") && saveData["plants"].is_array()) {
      metadata.plantCount = static_cast<int>(saveData["plants"].size());
    } else {
      metadata.plantCount = 0;
    }
    
    return metadata;
    
  } catch (const std::exception&) {
    return std::nullopt;
  }
}
