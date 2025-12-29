/**
 * @file IInputHandler.hpp
 * @brief Abstract interface for input handling
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file defines the abstract IInputHandler interface for platform-independent
 * input handling. It abstracts away ncurses getch() calls and allows different
 * backends to provide keyboard and mouse input in a uniform way.
 */

#ifndef ECOSIM_IINPUTHANDLER_HPP
#define ECOSIM_IINPUTHANDLER_HPP

#include <utility>

/**
 * @brief Platform-independent key codes
 * 
 * These key codes map common input keys to a platform-independent
 * enumeration. Backends translate their native key codes to these values.
 * 
 * Mapping from current ncurses keys in main.cpp (lines 454-520):
 *   KEY_UP    -> KEY_UP
 *   KEY_DOWN  -> KEY_DOWN
 *   KEY_LEFT  -> KEY_LEFT
 *   KEY_RIGHT -> KEY_RIGHT
 *   KEY_K     -> KEY_K (vim-style up)
 *   KEY_J     -> KEY_J (vim-style down)
 *   KEY_H     -> KEY_H (vim-style left)
 *   KEY_L     -> KEY_L (vim-style right)
 *   KEY_SPACE -> KEY_SPACE (pause)
 *   KEY_ESCAPE-> KEY_ESCAPE (quit)
 *   etc.
 */
enum class KeyCode {
    // Special keys
    KEY_NONE = 0,       ///< No key pressed
    KEY_UNKNOWN,        ///< Unknown key
    
    // Navigation keys
    KEY_UP,             ///< Up arrow
    KEY_DOWN,           ///< Down arrow
    KEY_LEFT,           ///< Left arrow
    KEY_RIGHT,          ///< Right arrow
    KEY_PAGE_UP,        ///< Page up
    KEY_PAGE_DOWN,      ///< Page down
    KEY_HOME,           ///< Home key
    KEY_END,            ///< End key
    
    // Letter keys (A-Z)
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,
    KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
    KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X,
    KEY_Y, KEY_Z,
    
    // Number keys (0-9)
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9,
    
    // Function keys
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    
    // Modifier keys
    KEY_SHIFT,          ///< Shift key
    KEY_CTRL,           ///< Control key
    KEY_ALT,            ///< Alt/Option key
    
    // Special characters
    KEY_SPACE,          ///< Space bar
    KEY_ENTER,          ///< Enter/Return key
    KEY_TAB,            ///< Tab key
    KEY_BACKSPACE,      ///< Backspace key
    KEY_DELETE,         ///< Delete key
    KEY_ESCAPE,         ///< Escape key
    
    // Symbols
    KEY_MINUS,          ///< Minus/hyphen key
    KEY_PLUS,           ///< Plus key
    KEY_EQUALS,         ///< Equals key
    KEY_COMMA,          ///< Comma key
    KEY_PERIOD,         ///< Period key
    KEY_SLASH,          ///< Forward slash
    KEY_BACKSLASH,      ///< Backslash
    KEY_BRACKET_LEFT,   ///< Left bracket [
    KEY_BRACKET_RIGHT,  ///< Right bracket ]
    
    COUNT               ///< Number of key codes
};

/**
 * @brief Mouse button enumeration
 */
enum class MouseButton {
    NONE = 0,           ///< No button
    LEFT,               ///< Left mouse button
    RIGHT,              ///< Right mouse button
    MIDDLE,             ///< Middle mouse button
    
    COUNT               ///< Number of button types
};

/**
 * @brief Input action enumeration for high-level input handling
 * 
 * These represent semantic actions that can be triggered by various
 * input combinations. Allows for key remapping without changing game logic.
 */
enum class InputAction {
    // No action
    ACTION_NONE = 0,
    
    // Navigation actions (map scrolling)
    MOVE_UP,            ///< Scroll map up
    MOVE_DOWN,          ///< Scroll map down
    MOVE_LEFT,          ///< Scroll map left
    MOVE_RIGHT,         ///< Scroll map right
    
    // Simulation control
    PAUSE,              ///< Pause/unpause simulation
    QUIT,               ///< Exit simulation
    TOGGLE_PAUSE_MENU,  ///< Toggle pause/ESC menu
    
    // UI actions
    TOGGLE_HUD,         ///< Toggle HUD visibility
    
    // Simulation actions
    ADD_CREATURES,      ///< Add more creatures
    SAVE_STATE,         ///< Save current state
    LOAD_STATE,         ///< Load saved state
    
    // Menu navigation
    MENU_UP,            ///< Navigate menu up
    MENU_DOWN,          ///< Navigate menu down
    MENU_SELECT,        ///< Select menu item
    MENU_CANCEL,        ///< Cancel/back from menu
    
    // World editor actions
    INCREASE_SCALE,     ///< Increase world scale
    DECREASE_SCALE,     ///< Decrease world scale
    NEW_SEED,           ///< Generate new world seed
    INCREASE_SEED,      ///< Increment world seed
    DECREASE_SEED,      ///< Decrement world seed
    INCREASE_FREQ,      ///< Increase noise frequency
    DECREASE_FREQ,      ///< Decrease noise frequency
    INCREASE_EXPONENT,  ///< Increase redistribution exponent
    DECREASE_EXPONENT,  ///< Decrease redistribution exponent
    INCREASE_TERRACES,  ///< Increase terrace count
    DECREASE_TERRACES,  ///< Decrease terrace count
    SELECT_TERRAIN_1,   ///< Select terrain level 1
    SELECT_TERRAIN_2,   ///< Select terrain level 2
    SELECT_TERRAIN_3,   ///< Select terrain level 3
    SELECT_TERRAIN_4,   ///< Select terrain level 4
    SELECT_TERRAIN_5,   ///< Select terrain level 5
    SELECT_TERRAIN_6,   ///< Select terrain level 6
    SELECT_TERRAIN_7,   ///< Select terrain level 7
    SELECT_TERRAIN_8,   ///< Select terrain level 8
    SELECT_TERRAIN_9,   ///< Select terrain level 9
    INCREASE_TERRAIN_LEVEL, ///< Increase selected terrain level
    DECREASE_TERRAIN_LEVEL, ///< Decrease selected terrain level
    CONFIRM_WORLD_EDIT, ///< Confirm world editing
    
    // Zoom control
    ZOOM_IN,            ///< Zoom in (increase tile size)
    ZOOM_OUT,           ///< Zoom out (decrease tile size)
    
    COUNT               ///< Number of actions
};

/**
 * @brief Mouse event data structure
 */
struct MouseEvent {
    int x;                  ///< X coordinate (pixels or characters)
    int y;                  ///< Y coordinate (pixels or characters)
    MouseButton button;     ///< Button involved
    bool pressed;           ///< True if button pressed down
    bool released;          ///< True if button released
    int scrollDelta;        ///< Scroll wheel delta (positive = up)
    
    /** @brief Default constructor */
    MouseEvent() 
        : x(0), y(0), button(MouseButton::NONE)
        , pressed(false), released(false), scrollDelta(0) {}
};

/**
 * @brief Input event structure
 * 
 * Represents a single input event, which can be either a key press
 * or a mouse event.
 */
struct InputEvent {
    KeyCode key;            ///< Key code (if keyboard event)
    InputAction action;     ///< High-level action (if mapped)
    bool hasMouseEvent;     ///< True if mouseEvent is valid
    MouseEvent mouseEvent;  ///< Mouse event data
    
    /** @brief Default constructor */
    InputEvent() 
        : key(KeyCode::KEY_NONE), action(InputAction::ACTION_NONE)
        , hasMouseEvent(false), mouseEvent() {}
    
    /** @brief Construct from key code */
    explicit InputEvent(KeyCode k, InputAction act = InputAction::ACTION_NONE)
        : key(k), action(act), hasMouseEvent(false), mouseEvent() {}
    
    /** @brief Construct from mouse event */
    explicit InputEvent(const MouseEvent& me)
        : key(KeyCode::KEY_NONE), action(InputAction::ACTION_NONE)
        , hasMouseEvent(true), mouseEvent(me) {}
    
    /** @brief Check if this is a mouse event */
    bool isMouseEvent() const { return hasMouseEvent; }
    
    /** @brief Check if this is a keyboard event */
    bool isKeyEvent() const { return key != KeyCode::KEY_NONE; }
    
    /** @brief Check if any event occurred */
    bool hasEvent() const { return isMouseEvent() || isKeyEvent(); }
};

/**
 * @brief Abstract interface for input handling
 * 
 * This interface defines the contract for input handling implementations.
 * Different backends (ncurses, SDL2, etc.) implement this interface to
 * provide platform-specific input handling.
 * 
 * Usage:
 * @code
 * IInputHandler* input = new NCursesInputHandler();
 * input->initialize();
 * 
 * while (running) {
 *     InputEvent event = input->pollInput();
 *     if (event.action == InputAction::QUIT) {
 *         running = false;
 *     }
 * }
 * 
 * input->shutdown();
 * @endcode
 */
class IInputHandler {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~IInputHandler() = default;

    //==========================================================================
    // Lifecycle Methods
    //==========================================================================
    
    /**
     * @brief Initialize the input system
     * 
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Shutdown the input system and cleanup
     */
    virtual void shutdown() = 0;

    //==========================================================================
    // Input Polling Methods
    //==========================================================================
    
    /**
     * @brief Poll for input (non-blocking)
     * 
     * Checks for pending input and returns immediately.
     * Returns an event with KEY_NONE if no input available.
     * 
     * @return InputEvent containing the input, or empty event if none
     */
    virtual InputEvent pollInput() = 0;
    
    /**
     * @brief Wait for input (blocking)
     * 
     * Blocks until input is received or timeout expires.
     * 
     * @param timeoutMs Timeout in milliseconds (-1 for infinite)
     * @return InputEvent containing the input
     */
    virtual InputEvent waitForInput(int timeoutMs = -1) = 0;
    
    /**
     * @brief Check if a specific key is currently pressed
     * 
     * @param key The key code to check
     * @return true if the key is currently held down
     */
    virtual bool isKeyPressed(KeyCode key) const = 0;
    
    /**
     * @brief Check if a specific action is triggered
     * 
     * @param action The action to check
     * @return true if the action's key binding is pressed
     */
    virtual bool isActionPressed(InputAction action) const = 0;

    //==========================================================================
    // Mouse Methods
    //==========================================================================
    
    /**
     * @brief Get current mouse position
     * 
     * @return Pair of (x, y) coordinates
     */
    virtual std::pair<int, int> getMousePosition() const = 0;
    
    /**
     * @brief Check if mouse input is supported
     * 
     * @return true if mouse input is available
     */
    virtual bool supportsMouseInput() const = 0;
    
    /**
     * @brief Enable or disable mouse input
     * 
     * @param enable true to enable, false to disable
     */
    virtual void enableMouseInput(bool enable) = 0;

    //==========================================================================
    // Input Configuration Methods
    //==========================================================================
    
    /**
     * @brief Set input delay/timeout for polling
     * 
     * @param delayMs Delay in milliseconds (0 for immediate)
     */
    virtual void setInputDelay(unsigned int delayMs) = 0;
    
    /**
     * @brief Set blocking or non-blocking input mode
     * 
     * @param blocking true for blocking mode
     */
    virtual void setBlockingMode(bool blocking) = 0;
    
    /**
     * @brief Map a key to an action
     * 
     * Allows remapping of key bindings at runtime.
     * 
     * @param key The key code
     * @param action The action to map to
     */
    virtual void mapKeyToAction(KeyCode key, InputAction action) = 0;
    
    /**
     * @brief Get the action mapped to a key
     * 
     * @param key The key code to look up
     * @return The mapped action, or ACTION_NONE if not mapped
     */
    virtual InputAction getActionForKey(KeyCode key) const = 0;
    
    /**
     * @brief Reset all key mappings to defaults
     */
    virtual void resetKeyMappings() = 0;

protected:
    /**
     * @brief Protected default constructor
     */
    IInputHandler() = default;
    
    /**
     * @brief Deleted copy constructor
     */
    IInputHandler(const IInputHandler&) = delete;
    
    /**
     * @brief Deleted copy assignment
     */
    IInputHandler& operator=(const IInputHandler&) = delete;
};

#endif // ECOSIM_IINPUTHANDLER_HPP