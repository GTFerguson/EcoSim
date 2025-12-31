#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

/**
 *  Title     : Ecosim - File Handling
 *  Author    : Gary Ferguson
 *  Date      : May 18th, 2019
 *  Updated   : December 2024
 *  Purpose   : Handles saving and loading of simulation data to files.
 *              Now uses JSON serialization for Creature and Plant data.
 */

#include "objects/creature/creature.hpp"
#include "objects/gameObject.hpp"
#include "world/world.hpp"
#include "calendar.hpp"
#include "genetics/organisms/Plant.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <optional>
#include <chrono>

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

    //============================================================================
    //  File Format Field Counts (for validation) - Legacy CSV format
    //============================================================================
    static constexpr size_t WORLD_HEADER_FIELDS = 17;
    static constexpr size_t TILE_COORD_FIELDS = 2;
    static constexpr size_t CREATURE_FIELDS = 28;
    static constexpr size_t STATS_FIELDS = 13;

    std::string saveDir, statDir, genomeDir;

    //============================================================================
    //  Private Helper Methods - Legacy CSV format (deprecated)
    //============================================================================
    std::vector<std::string> parseCsvLine (const std::string &line);
    bool loadWorld     (World &w, Calendar &calendar);
    bool loadCreatures (std::vector<Creature> &c);
    bool loadStats     (Statistics &stats);

  public:
    //============================================================================
    //  JSON Save Format Constants
    //============================================================================
    /**
     * @brief Save format identification and versioning namespace.
     *
     * Magic header enables quick identification of valid save files.
     * Version number enables forward-compatible loading of older saves.
     */
    struct SaveFormat {
        static constexpr const char* MAGIC_HEADER = "ECOSIM";
        static constexpr int CURRENT_VERSION = 1;
        static constexpr int MIN_SUPPORTED_VERSION = 1;
    };
    
    /// Backwards-compatible alias for existing code
    static constexpr int SAVE_VERSION = SaveFormat::CURRENT_VERSION;
    
    //============================================================================
    //  Save Metadata Structure
    //============================================================================
    /**
     * @brief Metadata extracted from a save file without loading full state.
     *        Useful for displaying save file info in UI before loading.
     */
    struct SaveMetadata {
        int version;            ///< Save format version
        std::string savedAt;    ///< ISO 8601 timestamp of when saved
        unsigned tick;          ///< Simulation tick at time of save
        int creatureCount;      ///< Number of creatures in save
        int plantCount;         ///< Number of plants in save
    };

    //============================================================================
    //  Constructor
    //============================================================================
    FileHandling (const std::string &directory);

    //============================================================================
    //  Setters
    //============================================================================
    void changeDirectory (const std::string &directory); 

    //============================================================================
    //  Saving - Legacy Methods (statistics/genomes)
    //============================================================================
    bool saveStatsHeader ();
    bool appendStats (const std::string &str);
    bool saveGenomes (const std::string &filename,
                      const std::vector<Creature> &creatures);
    
    //============================================================================
    //  Saving - Legacy CSV (deprecated)
    //============================================================================
    [[deprecated("Use saveGameJson() instead")]]
    bool saveState   (const World &w,
                      const std::vector<Creature> &c,
                      const Calendar &calendar,
                      const Statistics &stats);
    
    //============================================================================
    //  Saving - New JSON Format
    //============================================================================
    /**
     * @brief Save game state to a JSON file.
     * 
     * Uses atomic save pattern: writes to temporary file first, then renames
     * on success to prevent partial/corrupted saves.
     *
     * @param filepath Path to save file (relative to save directory)
     * @param creatures Vector of all creatures
     * @param world World containing all plants in tiles
     * @param calendar Current in-game calendar state
     * @param currentTick Current simulation tick
     * @param mapWidth World map width
     * @param mapHeight World map height
     * @return true on success, false on failure
     */
    bool saveGameJson(
        const std::string& filepath,
        const std::vector<Creature>& creatures,
        const World& world,
        const Calendar& calendar,
        unsigned currentTick,
        int mapWidth,
        int mapHeight
    );

    //============================================================================
    //  Loading - Legacy CSV (deprecated)
    //============================================================================
    [[deprecated("Use loadGameJson() instead")]]
    GameObject loadGameObject  (const std::vector<std::string> &str,
                                unsigned int &start);
    [[deprecated("Use loadGameJson() instead")]]
    Calendar   loadCalendar    (const std::vector<std::string> &str, unsigned &start);
    [[deprecated("Use loadGameJson() instead")]]
    bool       loadState       (World &w,
                                std::vector<Creature> &c,
                                Calendar &calendar,
                                Statistics &stats);
    
    //============================================================================
    //  Loading - New JSON Format
    //============================================================================
    /**
     * @brief Load game state from a JSON file.
     *
     * @param filepath Path to save file (relative to save directory)
     * @param creatures Output vector to populate with loaded creatures
     * @param world World to populate with loaded plants
     * @param calendar Output calendar to set with loaded state
     * @param currentTick Output tick count
     * @param mapWidth Expected map width for validation
     * @param mapHeight Expected map height for validation
     * @return true on success, false on failure (file not found, parse error, version mismatch)
     */
    bool loadGameJson(
        const std::string& filepath,
        std::vector<Creature>& creatures,
        World& world,
        Calendar& calendar,
        unsigned& currentTick,
        int mapWidth,
        int mapHeight
    );
    
    //============================================================================
    //  Metadata Query
    //============================================================================
    /**
     * @brief Get metadata from a save file without loading full state.
     *
     * Reads only the metadata portion of the save file for quick info display.
     *
     * @param filepath Path to save file (relative to save directory)
     * @return SaveMetadata if file exists and is valid, std::nullopt otherwise
     */
    std::optional<SaveMetadata> getSaveMetadata(const std::string& filepath) const;
    
    //============================================================================
    //  Utility Methods
    //============================================================================
    /**
     * @brief Get the full path to a save file.
     * @param filename Save file name (without .json extension)
     * @return Full path: saves/<filename>.json
     */
    std::string getFullSavePath(const std::string& filename) const;
    
    /**
     * @brief Generate ISO 8601 timestamp string.
     * @return Current time in ISO 8601 format (e.g., "2025-12-28T19:00:00Z")
     */
    static std::string generateTimestamp();
    
    /**
     * @brief List all save files in the saves directory.
     * @return Vector of save file names (without path, with .json extension)
     */
    std::vector<std::string> listSaveFiles() const;
    
    /**
     * @brief Check if a save file exists.
     * @param filename Save file name (without .json extension)
     * @return true if the file exists
     */
    bool saveFileExists(const std::string& filename) const;
    
    /**
     * @brief Ensure the saves directory exists.
     */
    void ensureSaveDirectory() const;
};

#endif
