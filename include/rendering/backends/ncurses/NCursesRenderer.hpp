/**
 * @file NCursesRenderer.hpp
 * @brief NCurses implementation of the IRenderer interface
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file defines the NCursesRenderer class that implements the IRenderer
 * interface for terminal-based ASCII rendering using ncurses.
 */

#ifndef ECOSIM_NCURSES_RENDERER_HPP
#define ECOSIM_NCURSES_RENDERER_HPP

#include "../../../rendering/IRenderer.hpp"
#include "../../../rendering/RenderTypes.hpp"
#include <string>
#include <vector>

// Forward declarations
class World;
class Creature;
class Tile;
class Calendar;

/**
 * @brief NCurses implementation of the IRenderer interface
 * 
 * This class provides terminal-based ASCII rendering using the ncurses library.
 * It wraps the existing rendering code from main.cpp to provide a clean
 * interface for the RenderSystem.
 * 
 * Features:
 * - Terminal-based ASCII graphics
 * - Color support (256 colors if terminal supports it)
 * - HUD display with simulation statistics
 * - Menu rendering with keyboard navigation
 */
class NCursesRenderer : public IRenderer {
public:
    /**
     * @brief Constructor
     */
    NCursesRenderer();
    
    /**
     * @brief Destructor - cleans up ncurses state
     */
    ~NCursesRenderer() override;

    //==========================================================================
    // Lifecycle Methods
    //==========================================================================
    
    /**
     * @brief Initialize ncurses subsystem
     * 
     * Calls initscr(), start_color(), and initializes all color pairs.
     * Sets up keypad, raw mode, noecho, and cursor visibility.
     * 
     * @return true if initialization successful
     */
    bool initialize() override;
    
    /**
     * @brief Shutdown ncurses and cleanup
     * 
     * Calls endwin() to restore terminal state.
     */
    void shutdown() override;
    
    /**
     * @brief Begin a new rendering frame
     * 
     * Clears the screen using erase().
     */
    void beginFrame() override;
    
    /**
     * @brief End frame and present to display
     * 
     * Calls refresh() to update the terminal.
     */
    void endFrame() override;

    //==========================================================================
    // World Rendering Methods
    //==========================================================================
    
    /**
     * @brief Render the world grid within the viewport
     * 
     * Renders terrain, spawners, and food items.
     * 
     * @param world The world to render
     * @param viewport The viewport configuration
     */
    void renderWorld(const World& world, const Viewport& viewport) override;
    
    /**
     * @brief Render a single tile
     * 
     * @param tile The tile to render
     * @param screenX Screen X coordinate (characters)
     * @param screenY Screen Y coordinate (characters)
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
     * @param screenX Screen X coordinate
     * @param screenY Screen Y coordinate
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
     * @brief Get screen width in characters
     */
    unsigned int getScreenWidth() const override;
    
    /**
     * @brief Get screen height in characters
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
     * @brief Check color support
     */
    bool supportsColor() const override;
    
    /**
     * @brief Check sprite support (always false for ncurses)
     */
    bool supportsSprites() const override;
    
    /**
     * @brief Get renderer name
     */
    std::string getName() const override;

    //==========================================================================
    // NCurses-Specific Methods
    //==========================================================================
    
    /**
     * @brief Set half-delay mode for input timing
     * 
     * @param tenths Delay in tenths of a second
     */
    void setHalfDelay(int tenths);
    
    /**
     * @brief Update screen dimensions after resize
     */
    void updateDimensions();

private:
    // Screen state
    bool _initialized;
    int _rows;
    int _cols;
    bool _hasColors;
    
    // UI layout constants
    static constexpr unsigned int MAP_HORI_BORDER = 2;
    static constexpr unsigned int MAP_VERT_BORDER = 4;
    
    // Helper methods
    void printCentered(const std::string& str, int y);
    int getColorPairForProfile(const Creature& creature) const;
};

#endif // ECOSIM_NCURSES_RENDERER_HPP