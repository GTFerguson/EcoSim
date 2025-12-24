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
#include "../../RenderTypes.hpp"

// Forward declarations
struct ImGuiContext;
class World;
class Creature;

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
     * @brief Get diet name string from Diet enum (legacy)
     * @param diet The Diet enum value
     * @return Human-readable diet name
     */
    const char* getDietName(int diet) const;
    
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