/**
 * @file NCursesInputHandler.cpp
 * @brief Implementation of NCursesInputHandler class
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file implements the NCurses-based input handler, wrapping the existing
 * input handling code from main.cpp into the IInputHandler interface.
 * 
 * IMPORTANT: Include order matters here!
 * 1. NCursesKeyConstants.hpp - captures ncurses macros and undefs them
 * 2. Our headers (which define KeyCode enum)
 */

// This MUST be included first - it includes ncurses.h, captures key values,
// and undefs conflicting macros
#include "../../../../include/rendering/backends/ncurses/NCursesKeyConstants.hpp"

// Now safe to include our headers
#include "../../../../include/rendering/backends/ncurses/NCursesInputHandler.hpp"
#include "../../../../include/rendering/KeyMappings.hpp"

//==============================================================================
// Constructor / Destructor
//==============================================================================

NCursesInputHandler::NCursesInputHandler()
    : _initialized(false)
    , _blocking(false)
    , _lastRawKey(ERR)
    , _lastKey(KeyCode::KEY_NONE) {
}

NCursesInputHandler::~NCursesInputHandler() {
    if (_initialized) {
        shutdown();
    }
}

//==============================================================================
// Lifecycle Methods
//==============================================================================

bool NCursesInputHandler::initialize() {
    if (_initialized) {
        return true;
    }
    
    // Initialize default key mappings
    initializeDefaultMappings();
    
    // Set non-blocking mode by default
    nodelay(stdscr, TRUE);
    _blocking = false;
    
    _initialized = true;
    return true;
}

void NCursesInputHandler::shutdown() {
    if (!_initialized) {
        return;
    }
    
    _keyActionMap.clear();
    _initialized = false;
}

//==============================================================================
// Input Polling Methods
//==============================================================================

InputEvent NCursesInputHandler::pollInput() {
    if (!_initialized) {
        return InputEvent();
    }
    
    // Ensure non-blocking mode
    if (_blocking) {
        nodelay(stdscr, TRUE);
        _blocking = false;
    }
    
    int rawKey = getch();
    return processKey(rawKey);
}

InputEvent NCursesInputHandler::waitForInput(int timeoutMs) {
    if (!_initialized) {
        return InputEvent();
    }
    
    if (timeoutMs < 0) {
        // Blocking mode
        nodelay(stdscr, FALSE);
        _blocking = true;
        int rawKey = getch();
        return processKey(rawKey);
    } else if (timeoutMs == 0) {
        // Non-blocking
        return pollInput();
    } else {
        // Use halfdelay for timeout
        // halfdelay uses tenths of seconds
        int tenths = (timeoutMs + 99) / 100;  // Round up
        if (tenths > 255) tenths = 255;
        if (tenths < 1) tenths = 1;
        halfdelay(tenths);
        int rawKey = getch();
        // Restore nodelay mode
        nodelay(stdscr, TRUE);
        _blocking = false;
        return processKey(rawKey);
    }
}

bool NCursesInputHandler::isKeyPressed(KeyCode key) const {
    return _lastKey == key;
}

bool NCursesInputHandler::isActionPressed(InputAction action) const {
    if (_lastKey == KeyCode::KEY_NONE) {
        return false;
    }
    
    auto it = _keyActionMap.find(_lastKey);
    if (it != _keyActionMap.end()) {
        return it->second == action;
    }
    
    return false;
}

//==============================================================================
// Mouse Methods
//==============================================================================

std::pair<int, int> NCursesInputHandler::getMousePosition() const {
    // ncurses mouse support is limited, return (0, 0) for now
    return std::make_pair(0, 0);
}

bool NCursesInputHandler::supportsMouseInput() const {
    return false;
}

void NCursesInputHandler::enableMouseInput(bool /*enable*/) {
    // Mouse input not currently supported
}

//==============================================================================
// Input Configuration Methods
//==============================================================================

void NCursesInputHandler::setInputDelay(unsigned int delayMs) {
    if (!_initialized) {
        return;
    }
    
    if (delayMs == 0) {
        nodelay(stdscr, TRUE);
        _blocking = false;
    } else {
        int tenths = (delayMs + 99) / 100;
        if (tenths > 255) tenths = 255;
        if (tenths < 1) tenths = 1;
        halfdelay(tenths);
        _blocking = false;  // halfdelay is semi-blocking
    }
}

void NCursesInputHandler::setBlockingMode(bool blocking) {
    if (!_initialized) {
        return;
    }
    
    if (blocking) {
        nodelay(stdscr, FALSE);
    } else {
        nodelay(stdscr, TRUE);
    }
    _blocking = blocking;
}

void NCursesInputHandler::mapKeyToAction(KeyCode key, InputAction action) {
    _keyActionMap[key] = action;
}

InputAction NCursesInputHandler::getActionForKey(KeyCode key) const {
    auto it = _keyActionMap.find(key);
    if (it != _keyActionMap.end()) {
        return it->second;
    }
    return InputAction::ACTION_NONE;
}

void NCursesInputHandler::resetKeyMappings() {
    _keyActionMap.clear();
    initializeDefaultMappings();
}

//==============================================================================
// NCurses-Specific Methods
//==============================================================================

int NCursesInputHandler::getRawKeyCode() const {
    return _lastRawKey;
}

KeyCode NCursesInputHandler::mapNcursesKey(int ncursesKey) {
    // Handle ERR (no input)
    if (ncursesKey == ERR) {
        return KeyCode::KEY_NONE;
    }
    
    // Arrow keys (using NCKeys namespace from NCursesKeyConstants.hpp)
    if (ncursesKey == NCKeys::UP)        return KeyCode::KEY_UP;
    if (ncursesKey == NCKeys::DOWN)      return KeyCode::KEY_DOWN;
    if (ncursesKey == NCKeys::LEFT)      return KeyCode::KEY_LEFT;
    if (ncursesKey == NCKeys::RIGHT)     return KeyCode::KEY_RIGHT;
    if (ncursesKey == NCKeys::PPAGE)     return KeyCode::KEY_PAGE_UP;
    if (ncursesKey == NCKeys::NPAGE)     return KeyCode::KEY_PAGE_DOWN;
    if (ncursesKey == NCKeys::HOME)      return KeyCode::KEY_HOME;
    if (ncursesKey == NCKeys::END)       return KeyCode::KEY_END;
    
    // Function keys
    if (ncursesKey == NCKeys::F1)        return KeyCode::KEY_F1;
    if (ncursesKey == NCKeys::F2)        return KeyCode::KEY_F2;
    if (ncursesKey == NCKeys::F3)        return KeyCode::KEY_F3;
    if (ncursesKey == NCKeys::F4)        return KeyCode::KEY_F4;
    if (ncursesKey == NCKeys::F5)        return KeyCode::KEY_F5;
    if (ncursesKey == NCKeys::F6)        return KeyCode::KEY_F6;
    if (ncursesKey == NCKeys::F7)        return KeyCode::KEY_F7;
    if (ncursesKey == NCKeys::F8)        return KeyCode::KEY_F8;
    if (ncursesKey == NCKeys::F9)        return KeyCode::KEY_F9;
    if (ncursesKey == NCKeys::F10)       return KeyCode::KEY_F10;
    if (ncursesKey == NCKeys::F11)       return KeyCode::KEY_F11;
    if (ncursesKey == NCKeys::F12)       return KeyCode::KEY_F12;
    
    // Other special keys (using renamed constants to avoid conflicts)
    if (ncursesKey == NCKeys::BACKSPACE_KEY) return KeyCode::KEY_BACKSPACE;
    if (ncursesKey == NCKeys::DC)            return KeyCode::KEY_DELETE;
    
    // ASCII special keys
    if (ncursesKey == NCKeys::ENTER_ASCII)     return KeyCode::KEY_ENTER;
    if (ncursesKey == NCKeys::ESCAPE_ASCII)    return KeyCode::KEY_ESCAPE;
    if (ncursesKey == NCKeys::SPACE_ASCII)     return KeyCode::KEY_SPACE;
    if (ncursesKey == NCKeys::TAB_ASCII)       return KeyCode::KEY_TAB;
    if (ncursesKey == NCKeys::BACKSPACE_ALT)   return KeyCode::KEY_BACKSPACE;
    
    // Number keys (0-9)
    if (ncursesKey >= '0' && ncursesKey <= '9') {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_0) + (ncursesKey - '0'));
    }
    
    // Letter keys (a-z, lowercase)
    if (ncursesKey >= 'a' && ncursesKey <= 'z') {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (ncursesKey - 'a'));
    }
    
    // Letter keys (A-Z, uppercase - map to same as lowercase)
    if (ncursesKey >= 'A' && ncursesKey <= 'Z') {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (ncursesKey - 'A'));
    }
    
    // Symbols
    if (ncursesKey == NCKeys::MINUS_ASCII)     return KeyCode::KEY_MINUS;
    if (ncursesKey == NCKeys::PLUS_ASCII)      return KeyCode::KEY_PLUS;
    if (ncursesKey == NCKeys::EQUAL_ASCII)     return KeyCode::KEY_EQUALS;
    if (ncursesKey == ',')                     return KeyCode::KEY_COMMA;
    if (ncursesKey == '.')                     return KeyCode::KEY_PERIOD;
    if (ncursesKey == '/')                     return KeyCode::KEY_SLASH;
    if (ncursesKey == '\\')                    return KeyCode::KEY_BACKSLASH;
    if (ncursesKey == '[')                     return KeyCode::KEY_BRACKET_LEFT;
    if (ncursesKey == ']')                     return KeyCode::KEY_BRACKET_RIGHT;
    
    return KeyCode::KEY_UNKNOWN;
}

//==============================================================================
// Private Methods
//==============================================================================

void NCursesInputHandler::initializeDefaultMappings() {
    // Use shared default mappings from KeyMappings.hpp
    _keyActionMap = KeyMappings::getDefaultMappings();
}

InputEvent NCursesInputHandler::processKey(int rawKey) {
    _lastRawKey = rawKey;
    
    if (rawKey == ERR) {
        _lastKey = KeyCode::KEY_NONE;
        return InputEvent();
    }
    
    // Map ncurses key to KeyCode
    KeyCode key = mapNcursesKey(rawKey);
    _lastKey = key;
    
    // Look up action for this key
    InputAction action = InputAction::ACTION_NONE;
    auto it = _keyActionMap.find(key);
    if (it != _keyActionMap.end()) {
        action = it->second;
    }
    
    return InputEvent(key, action);
}