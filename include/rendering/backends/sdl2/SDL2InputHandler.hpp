/**
 * @file SDL2InputHandler.hpp
 * @brief SDL2 implementation of the IInputHandler interface
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file defines the SDL2InputHandler class that implements the
 * IInputHandler interface for event-based input handling using SDL2.
 */

#ifndef ECOSIM_SDL2_INPUT_HANDLER_HPP
#define ECOSIM_SDL2_INPUT_HANDLER_HPP

#include "../../../rendering/IInputHandler.hpp"
#include <SDL.h>
#include <map>
#include <array>

/**
 * @brief SDL2 implementation of the IInputHandler interface
 * 
 * This class provides keyboard and mouse input handling using the SDL2 library.
 * It wraps SDL2's event system to provide a clean interface for the RenderSystem.
 * 
 * Features:
 * - Full keyboard support including function keys
 * - Mouse input with position tracking and button states
 * - Key to action mapping (configurable)
 * - Non-blocking and blocking input modes
 * - Proper SDL2 event queue integration
 */
class SDL2InputHandler : public IInputHandler {
public:
    /**
     * @brief Constructor
     */
    SDL2InputHandler();
    
    /**
     * @brief Destructor
     */
    ~SDL2InputHandler() override;

    //==========================================================================
    // Lifecycle Methods
    //==========================================================================
    
    /**
     * @brief Initialize input handler
     * 
     * Sets up SDL2 event handling and initializes key mappings.
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
     * @param key Key code to check
     * @return true if key is currently held down
     */
    bool isKeyPressed(KeyCode key) const override;
    
    /**
     * @brief Check if an action is triggered
     * 
     * @param action Action to check
     * @return true if action's key binding is pressed
     */
    bool isActionPressed(InputAction action) const override;

    //==========================================================================
    // Mouse Methods
    //==========================================================================
    
    /**
     * @brief Get mouse position
     * 
     * @return Pair of (x, y) coordinates in pixels
     */
    std::pair<int, int> getMousePosition() const override;
    
    /**
     * @brief Check if mouse input is supported
     * 
     * @return true (SDL2 supports mouse)
     */
    bool supportsMouseInput() const override;
    
    /**
     * @brief Enable/disable mouse input
     * 
     * @param enable Enable flag
     */
    void enableMouseInput(bool enable) override;

    //==========================================================================
    // Input Configuration Methods
    //==========================================================================
    
    /**
     * @brief Set input delay for polling
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
    // SDL2-Specific Methods
    //==========================================================================
    
    /**
     * @brief Convert SDL keycode to platform-independent KeyCode
     * 
     * @param sdlKey SDL_Keycode value
     * @return Corresponding KeyCode
     */
    static KeyCode mapSDLKey(SDL_Keycode sdlKey);
    
    /**
     * @brief Check if window close was requested
     * 
     * @return true if SDL_QUIT event was received
     */
    bool isQuitRequested() const { return _quitRequested; }

private:
    // State
    bool _initialized;
    bool _blocking;
    bool _mouseEnabled;
    bool _quitRequested;
    unsigned int _inputDelayMs;
    
    // Current input state
    KeyCode _lastKey;
    InputAction _lastAction;
    int _mouseX;
    int _mouseY;
    
    // Key state tracking
    std::array<bool, static_cast<size_t>(KeyCode::COUNT)> _keyStates;
    
    // Key to action mapping table
    std::map<KeyCode, InputAction> _keyActionMap;
    
    // Current SDL event
    SDL_Event _currentEvent;
    
    // Initialize default key mappings
    void initializeDefaultMappings();
    
    // Process SDL event and create InputEvent
    InputEvent processSDLEvent(const SDL_Event& event);
    
    // Update key states from SDL keyboard state
    void updateKeyStates();
};

#endif // ECOSIM_SDL2_INPUT_HANDLER_HPP