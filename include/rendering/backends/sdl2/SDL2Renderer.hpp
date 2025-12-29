/**
 * @file SDL2Renderer.hpp
 * @brief SDL2 implementation of the IRenderer interface
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file defines the SDL2Renderer class that implements the IRenderer
 * interface for hardware-accelerated graphical rendering using SDL2.
 */

#ifndef ECOSIM_SDL2_RENDERER_HPP
#define ECOSIM_SDL2_RENDERER_HPP

#include "../../../rendering/IRenderer.hpp"
#include "../../../rendering/RenderTypes.hpp"
#include <SDL.h>
#include <string>
#include <vector>
#include <utility>  // for std::pair

// Forward declarations
class World;
class Creature;
class Tile;
class Calendar;
class ImGuiOverlay;

/**
 * @brief SDL2 implementation of the IRenderer interface
 * 
 * This class provides hardware-accelerated graphical rendering using SDL2.
 * It renders the simulation using colored rectangles for terrain and entities,
 * providing a visual upgrade from the terminal-based ncurses renderer.
 * 
 * Features:
 * - Hardware-accelerated rendering via SDL2
 * - Full color support (32-bit RGBA)
 * - Mouse input support
 * - Scalable tile size
 * - HUD display with simulation statistics
 * - Menu rendering with keyboard/mouse navigation
 */
class SDL2Renderer : public IRenderer {
public:
    /**
     * @brief Constructor
     */
    SDL2Renderer();
    
    /**
     * @brief Destructor - cleans up SDL2 resources
     */
    ~SDL2Renderer() override;

    //==========================================================================
    // Lifecycle Methods
    //==========================================================================
    
    /**
     * @brief Initialize SDL2 subsystem
     * 
     * Creates SDL2 window and renderer, sets up rendering context.
     * 
     * @return true if initialization successful
     */
    bool initialize() override;
    
    /**
     * @brief Shutdown SDL2 and cleanup
     * 
     * Destroys window, renderer, and quits SDL2.
     */
    void shutdown() override;
    
    /**
     * @brief Begin a new rendering frame
     * 
     * Clears the rendering target.
     */
    void beginFrame() override;
    
    /**
     * @brief End frame and present to display
     * 
     * Presents the rendered frame to the window.
     */
    void endFrame() override;

    //==========================================================================
    // World Rendering Methods
    //==========================================================================
    
    /**
     * @brief Render the world grid within the viewport
     * 
     * Renders terrain as colored rectangles.
     * 
     * @param world The world to render
     * @param viewport The viewport configuration
     */
    void renderWorld(const World& world, const Viewport& viewport) override;
    
    /**
     * @brief Render a single tile
     * 
     * @param tile The tile to render
     * @param screenX Screen X coordinate (pixels)
     * @param screenY Screen Y coordinate (pixels)
     */
    void renderTile(const Tile& tile, int screenX, int screenY) override;
    
    /**
     * @brief Render creatures within the viewport
     * 
     * @param creatures Vector of creatures
     * @param viewport The viewport configuration
     */
    void renderCreatures(const std::vector<Creature>& creatures, 
                        const Viewport& viewport) override;
    
    /**
     * @brief Render a single creature
     * 
     * @param creature The creature to render
     * @param screenX Screen X coordinate (pixels)
     * @param screenY Screen Y coordinate (pixels)
     */
    void renderCreature(const Creature& creature, 
                       int screenX, int screenY) override;

    //==========================================================================
    // UI Rendering Methods
    //==========================================================================
    
    /**
     * @brief Render the heads-up display
     * 
     * Displays population, deaths, time, and creature stats.
     * 
     * @param data HUD data structure
     */
    void renderHUD(const HUDData& data) override;
    
    /**
     * @brief Render a menu
     * 
     * @param title Menu title
     * @param options Menu options
     * @return Selected option index, or -1 if cancelled
     */
    int renderMenu(const std::string& title,
                   const std::vector<MenuOption>& options) override;
    
    /**
     * @brief Render world generation details
     * 
     * @param world World with generation parameters
     */
    void renderWorldDetails(const World& world) override;
    
    /**
     * @brief Display a centered message
     * 
     * @param message The message to display
     * @param row Row to display on (-1 for center)
     */
    void renderMessage(const std::string& message, int row = -1) override;

    //==========================================================================
    // Screen Information Methods
    //==========================================================================
    
    /**
     * @brief Get screen width in pixels
     */
    unsigned int getScreenWidth() const override;
    
    /**
     * @brief Get screen height in pixels
     */
    unsigned int getScreenHeight() const override;
    
    /**
     * @brief Get maximum viewport width in tiles
     */
    unsigned int getViewportMaxWidth() const override;
    
    /**
     * @brief Get maximum viewport height in tiles
     */
    unsigned int getViewportMaxHeight() const override;
    
    /**
     * @brief Get screen center X
     */
    unsigned int getScreenCenterX() const override;
    
    /**
     * @brief Get screen center Y
     */
    unsigned int getScreenCenterY() const override;

    //==========================================================================
    // Capability Query Methods
    //==========================================================================
    
    /**
     * @brief Get renderer capabilities
     */
    RendererCapabilities getCapabilities() const override;
    
    /**
     * @brief Check color support (always true for SDL2)
     */
    bool supportsColor() const override;
    
    /**
     * @brief Check sprite support (true for SDL2)
     */
    bool supportsSprites() const override;
    
    /**
     * @brief Get renderer name
     */
    std::string getName() const override;

    //==========================================================================
    // SDL2-Specific Methods
    //==========================================================================
    
    /**
     * @brief Get the SDL window pointer
     * 
     * @return Pointer to SDL_Window
     */
    SDL_Window* getWindow() const { return _window; }
    
    /**
     * @brief Get the SDL renderer pointer
     * 
     * @return Pointer to SDL_Renderer
     */
    SDL_Renderer* getSDLRenderer() const { return _renderer; }
    
    /**
     * @brief Set the tile size in pixels
     * 
     * @param size Tile size (width and height) in pixels
     */
    void setTileSize(int size);
    
    /**
     * @brief Get the current tile size
     * 
     * @return Tile size in pixels
     */
    int getTileSize() const { return _tileSize; }
    
    /**
     * @brief Handle window resize event
     */
    void handleResize();
    
    /**
     * @brief Zoom in (increase tile size)
     *
     * Increases tile size by 2 pixels up to maximum.
     */
    void zoomIn() override;
    
    /**
     * @brief Zoom out (decrease tile size)
     *
     * Decreases tile size by 2 pixels down to minimum.
     */
    void zoomOut() override;
    
    /**
     * @brief Get current zoom level (tile size)
     *
     * @return Current tile size in pixels
     */
    int getZoomLevel() const override { return _tileSize; }
    
    /**
     * @brief Check if there's a pending viewport center request from ImGui
     *
     * @return true if a centering request is pending
     */
    bool hasViewportCenterRequest() const override;
    
    /**
     * @brief Get the pending viewport center position (world tile coordinates)
     *
     * @return Pair of (x, y) tile coordinates to center on
     */
    std::pair<int, int> getViewportCenterRequest() const override;
    
    /**
     * @brief Clear the pending viewport center request
     */
    void clearViewportCenterRequest() override;
    
    //==========================================================================
    // Pause Menu Methods
    //==========================================================================
    
    /**
     * @brief Toggle the pause menu visibility
     */
    void togglePauseMenu() override;
    
    /**
     * @brief Check if pause menu is currently open
     *
     * @return true if pause menu is visible
     */
    bool isPauseMenuOpen() const override;
    
    /**
     * @brief Check if quit was requested from pause menu
     *
     * @return true if quit was requested
     */
    bool shouldQuit() const override;
    
    /**
     * @brief Check if save was requested from pause menu
     *
     * @return true if save was requested
     */
    bool shouldSave() const override;
    
    /**
     * @brief Check if load was requested from pause menu
     *
     * @return true if load was requested
     */
    bool shouldLoad() const override;
    
    /**
     * @brief Reset the save flag after handling
     */
    void resetSaveFlag() override;
    
    /**
     * @brief Reset the load flag after handling
     */
    void resetLoadFlag() override;
    
    /**
     * @brief Check if save dialog is open
     */
    bool isSaveDialogOpen() const override;
    
    /**
     * @brief Check if load dialog is open
     */
    bool isLoadDialogOpen() const override;
    
    /**
     * @brief Set the list of save files for display in dialogs
     */
    void setSaveFiles(const std::vector<SaveFileInfo>& files) override;
    
    /**
     * @brief Set callback for checking if a file exists
     */
    void setFileExistsChecker(std::function<bool(const std::string&)> checker) override;
    
    /**
     * @brief Get the filename entered in save dialog
     */
    std::string getSaveFilename() const override;
    
    /**
     * @brief Get the filename selected in load dialog
     */
    std::string getLoadFilename() const override;
    
    /**
     * @brief Clear the save filename after handling
     */
    void clearSaveFilename() override;
    
    /**
     * @brief Clear the load filename after handling
     */
    void clearLoadFilename() override;
    
    /**
     * @brief Open the load dialog directly (for start screen use)
     */
    void openLoadDialog() override;
    
    /**
     * @brief Render only dialogs (save/load) without the full HUD
     *
     * Used during start screen when we want to show just the load dialog
     * without statistics panels and other HUD elements.
     */
    void renderDialogsOnly() override;
    
    /**
     * @brief Get the ImGui overlay pointer
     *
     * @return Pointer to ImGuiOverlay (can be null if not initialized)
     */
    ImGuiOverlay* getImGuiOverlay() const { return _imguiOverlay; }

    /**
     * @brief Render a selected/highlighted creature
     *
     * Draws the creature with a highlight effect to indicate selection
     *
     * @param creature The creature to render
     * @param screenX Screen X coordinate (pixels)
     * @param screenY Screen Y coordinate (pixels)
     */
    void renderSelectedCreature(const Creature& creature,
                               int screenX, int screenY);
    
    /**
     * @brief Render ImGui overlay with creature data
     *
     * Should be called after creatures are rendered so ImGui has access to creature data
     *
     * @param data HUD data structure
     * @param world World pointer for world info
     */
    void renderImGuiOverlay(const HUDData& data, const World* world = nullptr);

private:
    // SDL2 objects
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    
    // ImGui overlay
    ImGuiOverlay* _imguiOverlay;
    
    // Current creatures reference for ImGui
    const std::vector<Creature>* _currentCreatures;
    
    // Current world reference for ImGui
    const World* _currentWorld;
    
    // Screen state
    bool _initialized;
    int _screenWidth;
    int _screenHeight;
    int _tileSize;  // Pixels per tile
    
    // UI layout constants
    static constexpr int DEFAULT_TILE_SIZE = 8;      // Smaller for more zoomed-out default view
    static constexpr int MIN_TILE_SIZE = 4;          // Minimum zoom (most zoomed out)
    static constexpr int MAX_TILE_SIZE = 32;         // Maximum zoom (most zoomed in)
    static constexpr int DEFAULT_SCREEN_WIDTH = 1920;  // Will be overridden by display mode
    static constexpr int DEFAULT_SCREEN_HEIGHT = 1080; // Will be overridden by display mode
    static constexpr int HUD_HEIGHT = 150;
    static constexpr int HUD_PADDING = 10;
    
    // Helper methods for rendering
    void drawFilledRect(int x, int y, int w, int h, SDL_Color color);
    void drawRect(int x, int y, int w, int h, SDL_Color color);
    void drawText(const std::string& text, int x, int y, SDL_Color color);
    
    // Color helper methods
    SDL_Color getTerrainColor(TerrainType terrain) const;
    SDL_Color getProfileColor(BehaviorProfile profile) const;
    SDL_Color getProfileColor(const Creature& creature) const;
    SDL_Color getEntityColor(EntityType entity) const;
};

#endif // ECOSIM_SDL2_RENDERER_HPP