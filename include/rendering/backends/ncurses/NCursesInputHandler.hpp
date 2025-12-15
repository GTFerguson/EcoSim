/**
 * @file NCursesInputHandler.hpp
 * @brief NCurses implementation of the IInputHandler interface
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file defines the NCursesInputHandler class that implements the
 * IInputHandler interface for terminal-based keyboard input using ncurses.
 */

#ifndef ECOSIM_NCURSES_INPUT_HANDLER_HPP
#define ECOSIM_NCURSES_INPUT_HANDLER_HPP

#include "../../../rendering/IInputHandler.hpp"
#include <array>
#include <map>

/**
 * @brief NCurses implementation of the IInputHandler interface
 * 
 * This class provides keyboard input handling using the ncurses library.
 * It wraps the existing input handling code from main.cpp to provide a
 * clean interface for the RenderSystem.
 * 
 * Features:
 * - Non-blocking and blocking input modes
 * - Key to action mapping (configurable)
 * - Support for arrow keys, function keys, and special keys
 * - Vim-style navigation keys (h/j/k/l)
 */
class NCursesInputHandler : public IInputHandler {
public:
    /**
     * @brief Constructor
     */
    NCursesInputHandler();
    
    /**
     * @brief Destructor
     */
    ~NCursesInputHandler() override;

    //==========================================================================
    // Lifecycle Methods
    //==========================================================================
    
    /**
     * @brief Initialize input handler
     * 
     * Sets up ncurses input modes (keypad, nodelay, etc.)
     * 
     * @return true if initialization successful
     */
    bool initialize() override;
    
    /**
     * @brief Shutdown input handler
     */
    void shutdown() override;

    //==========================================================================
    // Input Polling Methods
    //==========================================================================
    
    /**
     * @brief Poll for input (non-blocking)
     * 
     * @return InputEvent with key and action, or empty if no input
     */
    InputEvent pollInput() override;
    
    /**
     * @brief Wait for input (blocking)
     * 
     * @param timeoutMs Timeout in milliseconds (-1 for infinite)
     * @return InputEvent with key and action
     */
    InputEvent waitForInput(int timeoutMs = -1) override;
    
    /**
     * @brief Check if a key is currently pressed
     * 
     * Note: ncurses doesn't track key state, this is based on last input
     * 
     * @param key Key code to check
     * @return true if key was last pressed
     */
    bool isKeyPressed(KeyCode key) const override;
    
    /**
     * @brief Check if an action is triggered
     * 
     * @param action Action to check
     * @return true if action's key was last pressed
     */
    bool isActionPressed(InputAction action) const override;

    //==========================================================================
    // Mouse Methods
    //==========================================================================
    
    /**
     * @brief Get mouse position
     * 
     * ncurses mouse support is limited
     * 
     * @return Pair of (x, y) coordinates
     */
    std::pair<int, int> getMousePosition() const override;
    
    /**
     * @brief Check if mouse input is supported
     * 
     * @return false (mouse not currently supported)
     */
    bool supportsMouseInput() const override;
    
    /**
     * @brief Enable/disable mouse input
     * 
     * @param enable Enable flag (currently ignored)
     */
    void enableMouseInput(bool enable) override;

    //==========================================================================
    // Input Configuration Methods
    //==========================================================================
    
    /**
     * @brief Set input delay for polling
     * 
     * Uses ncurses halfdelay() or nodelay() modes
     * 
     * @param delayMs Delay in milliseconds
     */
    void setInputDelay(unsigned int delayMs) override;
    
    /**
     * @brief Set blocking mode
     * 
     * @param blocking True for blocking, false for non-blocking
     */
    void setBlockingMode(bool blocking) override;
    
    /**
     * @brief Map a key to an action
     * 
     * @param key Key code
     * @param action Action to map to
     */
    void mapKeyToAction(KeyCode key, InputAction action) override;
    
    /**
     * @brief Get action for a key
     * 
     * @param key Key code
     * @return Mapped action
     */
    InputAction getActionForKey(KeyCode key) const override;
    
    /**
     * @brief Reset key mappings to defaults
     */
    void resetKeyMappings() override;

    //==========================================================================
    // NCurses-Specific Methods
    //==========================================================================
    
    /**
     * @brief Get raw ncurses key code
     * 
     * @return Last raw key code from getch()
     */
    int getRawKeyCode() const;
    
    /**
     * @brief Convert ncurses key code to KeyCode enum
     * 
     * @param ncursesKey Raw ncurses key code
     * @return Corresponding KeyCode
     */
    static KeyCode mapNcursesKey(int ncursesKey);

private:
    // State
    bool _initialized;
    bool _blocking;
    int _lastRawKey;
    KeyCode _lastKey;
    
    // Key to action mapping table
    std::map<KeyCode, InputAction> _keyActionMap;
    
    // Initialize default key mappings
    void initializeDefaultMappings();
    
    // Process a raw ncurses key and create an InputEvent
    InputEvent processKey(int rawKey);
};

#endif // ECOSIM_NCURSES_INPUT_HANDLER_HPP