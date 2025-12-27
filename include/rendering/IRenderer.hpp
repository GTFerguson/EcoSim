/**
 * @file IRenderer.hpp
 * @brief Abstract interface for rendering operations
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file defines the abstract IRenderer interface that all rendering
 * backends must implement. This allows the simulation to be rendered
 * using different backends (ncurses, SDL2+ImGui, etc.) without modification
 * to the core simulation code.
 */

#ifndef ECOSIM_IRENDERER_HPP
#define ECOSIM_IRENDERER_HPP

#include "RenderTypes.hpp"
#include <string>
#include <vector>
#include <utility>  // for std::pair

// Forward declarations to minimize include dependencies
class World;
class Creature;
class Tile;
class Calendar;

/**
 * @brief Abstract interface for rendering backends
 * 
 * This interface defines the contract that all rendering implementations
 * must follow. Implementations include:
 * - NCursesRenderer: Terminal-based ASCII rendering
 * - SDLRenderer: SDL2-based graphical rendering (future)
 * - ImGuiRenderer: Dear ImGui integration (future)
 * 
 * Usage:
 * @code
 * IRenderer* renderer = new NCursesRenderer();
 * if (renderer->initialize()) {
 *     renderer->beginFrame();
 *     renderer->renderWorld(world, viewport);
 *     renderer->renderCreatures(creatures, viewport);
 *     renderer->renderHUD(hudData);
 *     renderer->endFrame();
 * }
 * renderer->shutdown();
 * @endcode
 */
class IRenderer {
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IRenderer() = default;

    //==========================================================================
    // Lifecycle Methods
    //==========================================================================
    
    /**
     * @brief Initialize the rendering system
     * 
     * Performs any necessary setup for the rendering backend, such as
     * creating windows, initializing graphics contexts, loading resources, etc.
     * 
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Shutdown the rendering system and cleanup resources
     * 
     * Releases all resources held by the renderer, closes windows,
     * and performs cleanup. Should be called before destroying the renderer.
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Begin a new rendering frame
     * 
     * Called at the start of each frame to prepare for rendering.
     * Typically clears buffers, resets state, etc.
     */
    virtual void beginFrame() = 0;
    
    /**
     * @brief End the current frame and present to display
     * 
     * Called at the end of each frame to finalize rendering
     * and present the result to the display.
     */
    virtual void endFrame() = 0;

    //==========================================================================
    // World Rendering Methods
    //==========================================================================
    
    /**
     * @brief Render the world grid within the specified viewport
     * 
     * Renders all terrain tiles visible within the viewport bounds.
     * This includes terrain, spawners (trees), and food items on tiles.
     * 
     * @param world The world to render
     * @param viewport The viewport configuration defining visible area
     */
    virtual void renderWorld(const World& world, const Viewport& viewport) = 0;
    
    /**
     * @brief Render a single tile at the specified screen position
     * 
     * Renders an individual tile including its terrain type and any
     * objects (food, spawners) on it.
     * 
     * @param tile The tile to render
     * @param screenX Screen X coordinate (pixels or characters)
     * @param screenY Screen Y coordinate (pixels or characters)
     */
    virtual void renderTile(const Tile& tile, int screenX, int screenY) = 0;
    
    /**
     * @brief Render all creatures within the viewport
     * 
     * Renders creature entities with appropriate visual representation
     * based on their behavior profile (hungry, thirsty, breeding, etc.).
     * 
     * @param creatures Vector of creatures to consider for rendering
     * @param viewport The viewport configuration for culling
     */
    virtual void renderCreatures(const std::vector<Creature>& creatures, 
                                 const Viewport& viewport) = 0;
    
    /**
     * @brief Render a single creature at its position
     * 
     * @param creature The creature to render
     * @param screenX Screen X coordinate
     * @param screenY Screen Y coordinate
     */
    virtual void renderCreature(const Creature& creature, 
                                int screenX, int screenY) = 0;

    //==========================================================================
    // UI Rendering Methods
    //==========================================================================
    
    /**
     * @brief Render the heads-up display with simulation statistics
     * 
     * Displays population counts, death statistics, time, and other
     * relevant simulation information.
     * 
     * @param data HUD data structure containing statistics to display
     */
    virtual void renderHUD(const HUDData& data) = 0;
    
    /**
     * @brief Render a menu and return the selected option
     * 
     * Displays a menu with the given title and options, handles user
     * selection input, and returns the selected index.
     * 
     * @param title Menu title to display
     * @param options Vector of menu options
     * @return Selected option index (0-based), or -1 if cancelled
     */
    virtual int renderMenu(const std::string& title,
                           const std::vector<MenuOption>& options) = 0;
    
    /**
     * @brief Render world generation details overlay
     * 
     * Displays world generation parameters (seed, scale, frequency, etc.)
     * during world editing/creation.
     * 
     * @param world World with generation parameters to display
     */
    virtual void renderWorldDetails(const World& world) = 0;
    
    /**
     * @brief Display a centered message on screen
     * 
     * @param message The message text to display
     * @param row Optional row to display on (-1 for vertical center)
     */
    virtual void renderMessage(const std::string& message, int row = -1) = 0;

    //==========================================================================
    // Screen Information Methods
    //==========================================================================
    
    /**
     * @brief Get screen width in native units (characters or pixels)
     * 
     * @return Screen width
     */
    virtual unsigned int getScreenWidth() const = 0;
    
    /**
     * @brief Get screen height in native units (characters or pixels)
     * 
     * @return Screen height
     */
    virtual unsigned int getScreenHeight() const = 0;
    
    /**
     * @brief Get maximum viewport width in tiles
     * 
     * @return Maximum number of tiles that can be displayed horizontally
     */
    virtual unsigned int getViewportMaxWidth() const = 0;
    
    /**
     * @brief Get maximum viewport height in tiles
     * 
     * @return Maximum number of tiles that can be displayed vertically
     */
    virtual unsigned int getViewportMaxHeight() const = 0;
    
    /**
     * @brief Get the center X coordinate of the screen
     * 
     * @return Center X position
     */
    virtual unsigned int getScreenCenterX() const = 0;
    
    /**
     * @brief Get the center Y coordinate of the screen
     *
     * @return Center Y position
     */
    virtual unsigned int getScreenCenterY() const = 0;

    //==========================================================================
    // Zoom Methods
    //==========================================================================
    
    /**
     * @brief Zoom in (increase tile size/scale)
     *
     * Increases the zoom level, showing less of the world but more detail.
     * Default implementation does nothing (for backends that don't support zoom).
     */
    virtual void zoomIn() {}
    
    /**
     * @brief Zoom out (decrease tile size/scale)
     *
     * Decreases the zoom level, showing more of the world but less detail.
     * Default implementation does nothing (for backends that don't support zoom).
     */
    virtual void zoomOut() {}
    
    /**
     * @brief Get current zoom level
     *
     * @return Current zoom level (tile size in pixels for SDL2, 1 for NCurses)
     */
    virtual int getZoomLevel() const { return 1; }

    //==========================================================================
    // Viewport Center Request Methods (for UI-driven viewport changes)
    //==========================================================================
    
    /**
     * @brief Check if there's a pending viewport center request
     *
     * Used by ImGui or other UI to request centering viewport on a position.
     * Default implementation returns false (no UI support).
     *
     * @return true if a centering request is pending
     */
    virtual bool hasViewportCenterRequest() const { return false; }
    
    /**
     * @brief Get the pending viewport center position (world tile coordinates)
     *
     * @return Pair of (x, y) tile coordinates to center on, or (-1, -1) if none
     */
    virtual std::pair<int, int> getViewportCenterRequest() const { return {-1, -1}; }
    
    /**
     * @brief Clear the pending viewport center request
     */
    virtual void clearViewportCenterRequest() {}

    //==========================================================================
    // Capability Query Methods
    //==========================================================================
    
    /**
     * @brief Get renderer capabilities
     * 
     * @return RendererCapabilities structure describing what this renderer supports
     */
    virtual RendererCapabilities getCapabilities() const = 0;
    
    /**
     * @brief Check if renderer supports color output
     * 
     * @return true if color is supported
     */
    virtual bool supportsColor() const = 0;
    
    /**
     * @brief Check if renderer supports sprite/texture rendering
     * 
     * @return true if sprites are supported
     */
    virtual bool supportsSprites() const = 0;
    
    /**
     * @brief Get the renderer name/description
     * 
     * @return Human-readable name of the renderer
     */
    virtual std::string getName() const = 0;

protected:
    /**
     * @brief Protected default constructor
     * 
     * Prevents direct instantiation of the interface.
     */
    IRenderer() = default;
    
    /**
     * @brief Deleted copy constructor
     */
    IRenderer(const IRenderer&) = delete;
    
    /**
     * @brief Deleted copy assignment operator
     */
    IRenderer& operator=(const IRenderer&) = delete;
};

#endif // ECOSIM_IRENDERER_HPP