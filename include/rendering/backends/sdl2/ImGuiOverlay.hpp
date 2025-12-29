/**
 * @file ImGuiOverlay.hpp
 * @brief Dear ImGui overlay for professional debug UI panels
 * @author Gary Ferguson
 * @date December 2025
 *
 * This file defines the ImGuiOverlay class that provides beautiful,
 * professional UI panels for statistics, debugging, and controls
 * rendered on top of the SDL2 world view.
 */

#ifndef ECOSIM_IMGUI_OVERLAY_HPP
#define ECOSIM_IMGUI_OVERLAY_HPP

#include <SDL.h>
#include <vector>
#include <string>
#include <functional>
#include "../../RenderTypes.hpp"

// Forward declarations
struct ImGuiContext;
class World;
class Creature;
struct SaveFileInfo; // Defined in IRenderer.hpp

namespace EcoSim {
namespace Genetics {
    class Plant;
    enum class DietType;
}
}

/**
 * @brief Dear ImGui overlay for debug and statistics UI
 *
 * This class manages the ImGui integration with SDL2, providing
 * movable, resizable, and dockable UI panels for simulation
 * statistics, world information, and performance metrics.
 *
 * Features:
 * - Main menu bar with window toggles
 * - Statistics window with population graphs
 * - World information panel
 * - Performance metrics with FPS graph
 * - Creature list browser with filtering and sorting
 * - Creature inspector for detailed creature information
 * - Controls panel for simulation management
 */
class ImGuiOverlay {
public:
    /**
     * @brief Constructor
     */
    ImGuiOverlay();
    
    /**
     * @brief Destructor - cleans up ImGui resources
     */
    ~ImGuiOverlay();
    
    //==========================================================================
    // Lifecycle Methods
    //==========================================================================
    
    /**
     * @brief Initialize ImGui with SDL2 backend
     *
     * Sets up ImGui context, style, and SDL2/Renderer backends.
     *
     * @param window SDL2 window pointer
     * @param renderer SDL2 renderer pointer
     * @return true if initialization successful
     */
    bool initialize(SDL_Window* window, SDL_Renderer* renderer);
    
    /**
     * @brief Shutdown ImGui and cleanup resources
     */
    void shutdown();
    
    //==========================================================================
    // Rendering Methods
    //==========================================================================
    
    /**
     * @brief Begin a new ImGui frame
     *
     * Must be called at the start of each frame before any ImGui rendering.
     */
    void beginFrame();
    
    /**
     * @brief Render all ImGui windows with simulation data
     *
     * @param hudData HUD data containing simulation statistics
     * @param world Optional world pointer for additional info
     * @param creatures Optional pointer to creatures vector for creature panels
     */
    void render(const HUDData& hudData,
                const World* world = nullptr,
                const std::vector<Creature>* creatures = nullptr);
    
    /**
     * @brief End ImGui frame and render draw data
     *
     * Must be called after all ImGui rendering is complete.
     */
    void endFrame();
    
    //==========================================================================
    // Window Toggle Methods
    //==========================================================================
    
    /**
     * @brief Toggle statistics window visibility
     */
    void toggleStatistics() { _showStatistics = !_showStatistics; }
    
    /**
     * @brief Toggle world info window visibility
     */
    void toggleWorldInfo() { _showWorldInfo = !_showWorldInfo; }
    
    /**
     * @brief Toggle creature list window visibility
     */
    void toggleCreatureList() { _showCreatureList = !_showCreatureList; }
    
    /**
     * @brief Toggle performance window visibility
     */
    void togglePerformance() { _showPerformance = !_showPerformance; }
    
    /**
     * @brief Toggle creature inspector window visibility
     */
    void toggleCreatureInspector() { _showCreatureInspector = !_showCreatureInspector; }
    
    /**
     * @brief Toggle controls window visibility
     */
    void toggleControls() { _showControls = !_showControls; }
    
    //==========================================================================
    // State Query Methods
    //==========================================================================
    
    /**
     * @brief Check if ImGui is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return _initialized; }
    
    /**
     * @brief Check if statistics window is visible
     */
    bool isStatisticsVisible() const { return _showStatistics; }
    
    /**
     * @brief Check if world info window is visible
     */
    bool isWorldInfoVisible() const { return _showWorldInfo; }
    
    /**
     * @brief Check if performance window is visible
     */
    bool isPerformanceVisible() const { return _showPerformance; }
    
    /**
     * @brief Check if creature inspector window is visible
     */
    bool isCreatureInspectorVisible() const { return _showCreatureInspector; }
    
    /**
     * @brief Check if controls window is visible
     */
    bool isControlsVisible() const { return _showControls; }
    
    /**
     * @brief Get the currently selected creature ID
     * @return Selected creature ID, or -1 if none selected
     */
    int getSelectedCreatureId() const { return _selectedCreatureId; }
    
    /**
     * @brief Set the selected creature ID
     * @param id Creature ID to select (-1 to deselect)
     */
    void setSelectedCreatureId(int id) { _selectedCreatureId = id; }
    
    /**
     * @brief Toggle plant inspector window visibility
     */
    void togglePlantInspector() { _showPlantInspector = !_showPlantInspector; }
    
    /**
     * @brief Check if plant inspector window is visible
     */
    bool isPlantInspectorVisible() const { return _showPlantInspector; }
    
    /**
     * @brief Get the currently selected plant ID
     * @return Selected plant ID, or -1 if none selected
     */
    int getSelectedPlantId() const { return _selectedPlantId; }
    
    /**
     * @brief Set the selected plant ID
     * @param id Plant ID to select (-1 to deselect)
     */
    void setSelectedPlantId(int id) { _selectedPlantId = id; }
    
    //==========================================================================
    // Viewport Centering Request Methods
    //==========================================================================
    
    /**
     * @brief Check if there's a pending viewport center request
     * @return true if a centering request is pending
     */
    bool hasPendingCenterRequest() const { return _pendingCenterX >= 0 && _pendingCenterY >= 0; }
    
    /**
     * @brief Get the pending center position (world tile coordinates)
     * @return Pair of (x, y) tile coordinates to center on
     */
    std::pair<int, int> getPendingCenterPosition() const { return {_pendingCenterX, _pendingCenterY}; }
    
    /**
     * @brief Clear the pending center request
     */
    void clearCenterRequest() { _pendingCenterX = -1; _pendingCenterY = -1; }
    
    //==========================================================================
    // Pause Menu Methods
    //==========================================================================
    
    /**
     * @brief Toggle the pause menu visibility
     */
    void togglePauseMenu();
    
    /**
     * @brief Check if pause menu is currently open
     * @return true if pause menu is visible
     */
    bool isPauseMenuOpen() const { return _showPauseMenu; }
    
    /**
     * @brief Check if quit was requested from pause menu
     * @return true if quit was requested
     */
    bool shouldQuit() const { return _shouldQuit; }
    
    /**
     * @brief Check if save was requested from pause menu
     * @return true if save was requested
     */
    bool shouldSave() const { return _shouldSave; }
    
    /**
     * @brief Check if load was requested from pause menu
     * @return true if load was requested
     */
    bool shouldLoad() const { return _shouldLoad; }
    
    /**
     * @brief Reset the save flag after handling
     */
    void resetSaveFlag() { _shouldSave = false; }
    
    /**
     * @brief Reset the load flag after handling
     */
    void resetLoadFlag() { _shouldLoad = false; }
    
    //==========================================================================
    // Save/Load Dialog Methods
    //==========================================================================
    
    /**
     * @brief Check if save dialog is open
     */
    bool isSaveDialogOpen() const { return _showSaveDialog; }
    
    /**
     * @brief Check if load dialog is open
     */
    bool isLoadDialogOpen() const { return _showLoadDialog; }
    
    /**
     * @brief Set the list of available save files for display in dialogs
     * @param files Vector of SaveFileInfo with metadata
     */
    void setSaveFiles(const std::vector<SaveFileInfo>& files) { _saveFiles = files; }
    
    /**
     * @brief Get the filename to save to (set when user confirms save)
     * @return Filename without .json extension, empty if no save pending
     */
    std::string getSaveFilename() const { return _pendingSaveFilename; }
    
    /**
     * @brief Get the filename to load from (set when user confirms load)
     * @return Filename without .json extension, empty if no load pending
     */
    std::string getLoadFilename() const { return _pendingLoadFilename; }
    
    /**
     * @brief Clear the pending save filename after handling
     */
    void clearSaveFilename() { _pendingSaveFilename.clear(); }
    
    /**
     * @brief Clear the pending load filename after handling
     */
    void clearLoadFilename() { _pendingLoadFilename.clear(); }
    
    /**
     * @brief Check if there's a save filename ready for processing
     */
    bool hasPendingSave() const { return !_pendingSaveFilename.empty(); }
    
    /**
     * @brief Check if there's a load filename ready for processing
     */
    bool hasPendingLoad() const { return !_pendingLoadFilename.empty(); }
    
    /**
     * @brief Set overwrite check callback - returns true if file exists
     * @param checker Function that checks if a save file exists
     */
    void setFileExistsChecker(std::function<bool(const std::string&)> checker) {
        _fileExistsChecker = checker;
    }
    
    /**
     * @brief Open the load dialog directly (for start screen use)
     */
    void openLoadDialog();
    
    /**
     * @brief Render only dialogs (save/load) without the full HUD
     *
     * Used during start screen when we want to show just the load dialog
     * without statistics panels and other HUD elements.
     */
    void renderDialogsOnly();

private:
    // SDL2 references (not owned)
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    
    // ImGui context
    ImGuiContext* _context;
    
    // State
    bool _initialized;
    
    // Window visibility flags
    bool _showStatistics;
    bool _showWorldInfo;
    bool _showCreatureList;
    bool _showPerformance;
    bool _showCreatureInspector;
    bool _showPlantInspector;
    bool _showControls;
    bool _showDemo;  // ImGui demo window (for development)
    
    // Creature selection state
    int _selectedCreatureId;
    
    // Plant selection state
    int _selectedPlantId;
    
    // Creature list filter/sort state
    char _creatureFilterText[64];
    int _creatureSortMode;
    
    // Last HUD data for controls panel
    HUDData _lastHudData;
    
    // Performance tracking
    static constexpr int FRAME_TIME_HISTORY_SIZE = 120;
    float _frameTimes[FRAME_TIME_HISTORY_SIZE];
    int _frameTimeIndex;
    
    // Population dynamics history for graphing
    static constexpr int HISTORY_SIZE = 120;
    float _populationHistory[HISTORY_SIZE];
    float _birthsHistory[HISTORY_SIZE];
    float _deathsHistory[HISTORY_SIZE];
    int _historyIndex;
    
    // Previous frame cumulative values for delta calculation
    unsigned int _lastBirths;
    unsigned int _lastDeaths;
    
    // Viewport centering request (set by double-click in creature list)
    int _pendingCenterX;
    int _pendingCenterY;
    
    // Pause menu state
    bool _showPauseMenu = false;
    bool _shouldQuit = false;
    bool _shouldSave = false;
    bool _shouldLoad = false;
    
    // Save/Load dialog state
    bool _showSaveDialog = false;
    bool _showLoadDialog = false;
    bool _showOverwriteConfirm = false;
    char _saveNameInput[128] = {0};                   ///< Input buffer for save name
    int _selectedSaveIndex = -1;                      ///< Selected save file in list
    std::string _pendingSaveFilename;                 ///< Filename to save to once confirmed
    std::string _pendingLoadFilename;                 ///< Filename to load from once confirmed
    std::string _pendingOverwriteFilename;            ///< Filename pending overwrite confirmation
    std::vector<SaveFileInfo> _saveFiles;             ///< Available save files
    std::function<bool(const std::string&)> _fileExistsChecker; ///< Callback to check file existence
    bool _showPostSaveDialog = false;                 ///< Show post-save Continue/Quit dialog
    
    //==========================================================================
    // Private Rendering Methods
    //==========================================================================
    
    /**
     * @brief Render the main menu bar
     */
    void renderMainMenuBar();
    
    /**
     * @brief Render the statistics window
     * @param hudData HUD data with simulation statistics
     */
    void renderStatisticsWindow(const HUDData& hudData);
    
    /**
     * @brief Render the world information window
     * @param world World pointer (can be null)
     * @param creatures Creatures pointer for population stats
     */
    void renderWorldInfoWindow(const World* world, const std::vector<Creature>* creatures = nullptr);
    
    /**
     * @brief Render the performance metrics window
     */
    void renderPerformanceWindow();
    
    /**
     * @brief Render the creature list window
     * @param creatures Pointer to creatures vector
     */
    void renderCreatureListWindow(const std::vector<Creature>* creatures);
    
    /**
     * @brief Render the creature inspector window
     * @param creature Pointer to the selected creature
     */
    void renderCreatureInspectorWindow(const Creature* creature);
    
    /**
     * @brief Render the controls panel
     */
    void renderControlsWindow();
    
    /**
     * @brief Render the pause menu
     */
    void renderPauseMenu();
    
    /**
     * @brief Render the save dialog
     */
    void renderSaveDialog();
    
    /**
     * @brief Render the load dialog
     */
    void renderLoadDialog();
    
    /**
     * @brief Render the overwrite confirmation dialog
     */
    void renderOverwriteConfirmDialog();
    
    /**
     * @brief Render the post-save Continue/Quit dialog
     */
    void renderPostSaveDialog();
    
    /**
     * @brief Render a semi-transparent dark overlay behind modal dialogs
     *
     * DRY helper used by pause menu and modal popups to create a consistent
     * dark overlay effect that dims the background content.
     */
    void renderModalDarkOverlay();
    
    /**
     * @brief Render the save file list (shared by save and load dialogs)
     * @param clickAction 0 = save mode (triggers overwrite), 1 = load mode (double-click loads)
     */
    void renderSaveFileList(int clickAction);
    
    /**
     * @brief Render the plant inspector window
     * @param plant Pointer to the selected plant
     */
    void renderPlantInspectorWindow(const EcoSim::Genetics::Plant* plant);
    
    /**
     * @brief Set up custom ImGui style
     */
    void setupStyle();
    
    /**
     * @brief Get profile name string from Profile enum
     * @param profile The Profile enum value
     * @return Human-readable profile name
     */
    const char* getProfileName(int profile) const;
    
    /**
     * @brief Get diet name string from DietType enum
     * @param diet The DietType enum value
     * @return Human-readable diet name
     */
    const char* getDietName(EcoSim::Genetics::DietType diet) const;
    
    /**
     * @brief Get emergent diet type name from new genetics system
     * @param type The DietType enum value
     * @return Human-readable diet type name
     */
    const char* getEmergentDietName(EcoSim::Genetics::DietType type) const;
    
    /**
     * @brief Get dispersal strategy name
     * @param strategy The DispersalStrategy enum value
     * @return Human-readable strategy name
     */
    const char* getDispersalStrategyName(int strategy) const;
};

#endif // ECOSIM_IMGUI_OVERLAY_HPP