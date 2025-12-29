/**
 * @file SDL2InputHandler.cpp
 * @brief Implementation of SDL2InputHandler class
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file implements the SDL2-based input handler, providing keyboard
 * and mouse input handling for the ecological simulation.
 */

#include "../../../../include/rendering/backends/sdl2/SDL2InputHandler.hpp"
#include <algorithm>

// ImGui integration for event handling
#ifdef ECOSIM_HAS_IMGUI
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#endif

//==============================================================================
// Constructor / Destructor
//==============================================================================

SDL2InputHandler::SDL2InputHandler()
    : _initialized(false)
    , _blocking(false)
    , _mouseEnabled(true)
    , _quitRequested(false)
    , _inputDelayMs(0)
    , _lastKey(KeyCode::KEY_NONE)
    , _lastAction(InputAction::ACTION_NONE)
    , _mouseX(0)
    , _mouseY(0) {
    // Initialize key states to false
    _keyStates.fill(false);
}

SDL2InputHandler::~SDL2InputHandler() {
    if (_initialized) {
        shutdown();
    }
}

//==============================================================================
// Lifecycle Methods
//==============================================================================

bool SDL2InputHandler::initialize() {
    if (_initialized) {
        return true;
    }
    
    // Initialize default key mappings
    initializeDefaultMappings();
    
    _initialized = true;
    return true;
}

void SDL2InputHandler::shutdown() {
    if (!_initialized) {
        return;
    }
    
    _keyActionMap.clear();
    _keyStates.fill(false);
    _initialized = false;
}

//==============================================================================
// Input Polling Methods
//==============================================================================

InputEvent SDL2InputHandler::pollInput() {
    if (!_initialized) {
        return InputEvent();
    }
    
    // Poll for events (non-blocking)
    if (SDL_PollEvent(&_currentEvent)) {
        return processSDLEvent(_currentEvent);
    }
    
    // No event available
    _lastKey = KeyCode::KEY_NONE;
    _lastAction = InputAction::ACTION_NONE;
    return InputEvent();
}

InputEvent SDL2InputHandler::waitForInput(int timeoutMs) {
    if (!_initialized) {
        return InputEvent();
    }
    
    if (timeoutMs < 0) {
        // Blocking mode - wait indefinitely
        if (SDL_WaitEvent(&_currentEvent)) {
            return processSDLEvent(_currentEvent);
        }
    }
    else if (timeoutMs == 0) {
        // Non-blocking
        return pollInput();
    }
    else {
        // Wait with timeout
        if (SDL_WaitEventTimeout(&_currentEvent, timeoutMs)) {
            return processSDLEvent(_currentEvent);
        }
    }
    
    return InputEvent();
}

bool SDL2InputHandler::isKeyPressed(KeyCode key) const {
    if (key == KeyCode::KEY_NONE || key >= KeyCode::COUNT) {
        return false;
    }
    return _keyStates[static_cast<size_t>(key)];
}

bool SDL2InputHandler::isActionPressed(InputAction action) const {
    // Find the key mapped to this action and check if it's pressed
    for (const auto& pair : _keyActionMap) {
        if (pair.second == action) {
            return isKeyPressed(pair.first);
        }
    }
    return false;
}

//==============================================================================
// Mouse Methods
//==============================================================================

std::pair<int, int> SDL2InputHandler::getMousePosition() const {
    return std::make_pair(_mouseX, _mouseY);
}

bool SDL2InputHandler::supportsMouseInput() const {
    return true;
}

void SDL2InputHandler::enableMouseInput(bool enable) {
    _mouseEnabled = enable;
}

//==============================================================================
// Input Configuration Methods
//==============================================================================

void SDL2InputHandler::setInputDelay(unsigned int delayMs) {
    _inputDelayMs = delayMs;
}

void SDL2InputHandler::setBlockingMode(bool blocking) {
    _blocking = blocking;
}

void SDL2InputHandler::mapKeyToAction(KeyCode key, InputAction action) {
    _keyActionMap[key] = action;
}

InputAction SDL2InputHandler::getActionForKey(KeyCode key) const {
    auto it = _keyActionMap.find(key);
    if (it != _keyActionMap.end()) {
        return it->second;
    }
    return InputAction::ACTION_NONE;
}

void SDL2InputHandler::resetKeyMappings() {
    _keyActionMap.clear();
    initializeDefaultMappings();
}

//==============================================================================
// SDL2-Specific Methods
//==============================================================================

KeyCode SDL2InputHandler::mapSDLKey(SDL_Keycode sdlKey) {
    switch (sdlKey) {
        // Arrow keys
        case SDLK_UP:        return KeyCode::KEY_UP;
        case SDLK_DOWN:      return KeyCode::KEY_DOWN;
        case SDLK_LEFT:      return KeyCode::KEY_LEFT;
        case SDLK_RIGHT:     return KeyCode::KEY_RIGHT;
        
        // Navigation keys
        case SDLK_PAGEUP:    return KeyCode::KEY_PAGE_UP;
        case SDLK_PAGEDOWN:  return KeyCode::KEY_PAGE_DOWN;
        case SDLK_HOME:      return KeyCode::KEY_HOME;
        case SDLK_END:       return KeyCode::KEY_END;
        
        // Letter keys
        case SDLK_a:         return KeyCode::KEY_A;
        case SDLK_b:         return KeyCode::KEY_B;
        case SDLK_c:         return KeyCode::KEY_C;
        case SDLK_d:         return KeyCode::KEY_D;
        case SDLK_e:         return KeyCode::KEY_E;
        case SDLK_f:         return KeyCode::KEY_F;
        case SDLK_g:         return KeyCode::KEY_G;
        case SDLK_h:         return KeyCode::KEY_H;
        case SDLK_i:         return KeyCode::KEY_I;
        case SDLK_j:         return KeyCode::KEY_J;
        case SDLK_k:         return KeyCode::KEY_K;
        case SDLK_l:         return KeyCode::KEY_L;
        case SDLK_m:         return KeyCode::KEY_M;
        case SDLK_n:         return KeyCode::KEY_N;
        case SDLK_o:         return KeyCode::KEY_O;
        case SDLK_p:         return KeyCode::KEY_P;
        case SDLK_q:         return KeyCode::KEY_Q;
        case SDLK_r:         return KeyCode::KEY_R;
        case SDLK_s:         return KeyCode::KEY_S;
        case SDLK_t:         return KeyCode::KEY_T;
        case SDLK_u:         return KeyCode::KEY_U;
        case SDLK_v:         return KeyCode::KEY_V;
        case SDLK_w:         return KeyCode::KEY_W;
        case SDLK_x:         return KeyCode::KEY_X;
        case SDLK_y:         return KeyCode::KEY_Y;
        case SDLK_z:         return KeyCode::KEY_Z;
        
        // Number keys
        case SDLK_0:         return KeyCode::KEY_0;
        case SDLK_1:         return KeyCode::KEY_1;
        case SDLK_2:         return KeyCode::KEY_2;
        case SDLK_3:         return KeyCode::KEY_3;
        case SDLK_4:         return KeyCode::KEY_4;
        case SDLK_5:         return KeyCode::KEY_5;
        case SDLK_6:         return KeyCode::KEY_6;
        case SDLK_7:         return KeyCode::KEY_7;
        case SDLK_8:         return KeyCode::KEY_8;
        case SDLK_9:         return KeyCode::KEY_9;
        
        // Function keys
        case SDLK_F1:        return KeyCode::KEY_F1;
        case SDLK_F2:        return KeyCode::KEY_F2;
        case SDLK_F3:        return KeyCode::KEY_F3;
        case SDLK_F4:        return KeyCode::KEY_F4;
        case SDLK_F5:        return KeyCode::KEY_F5;
        case SDLK_F6:        return KeyCode::KEY_F6;
        case SDLK_F7:        return KeyCode::KEY_F7;
        case SDLK_F8:        return KeyCode::KEY_F8;
        case SDLK_F9:        return KeyCode::KEY_F9;
        case SDLK_F10:       return KeyCode::KEY_F10;
        case SDLK_F11:       return KeyCode::KEY_F11;
        case SDLK_F12:       return KeyCode::KEY_F12;
        
        // Special keys
        case SDLK_SPACE:     return KeyCode::KEY_SPACE;
        case SDLK_RETURN:    return KeyCode::KEY_ENTER;
        case SDLK_TAB:       return KeyCode::KEY_TAB;
        case SDLK_BACKSPACE: return KeyCode::KEY_BACKSPACE;
        case SDLK_DELETE:    return KeyCode::KEY_DELETE;
        case SDLK_ESCAPE:    return KeyCode::KEY_ESCAPE;
        
        // Modifier keys
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:    return KeyCode::KEY_SHIFT;
        case SDLK_LCTRL:
        case SDLK_RCTRL:     return KeyCode::KEY_CTRL;
        case SDLK_LALT:
        case SDLK_RALT:      return KeyCode::KEY_ALT;
        
        // Symbols
        case SDLK_MINUS:     return KeyCode::KEY_MINUS;
        case SDLK_PLUS:      return KeyCode::KEY_PLUS;
        case SDLK_EQUALS:    return KeyCode::KEY_EQUALS;
        case SDLK_COMMA:     return KeyCode::KEY_COMMA;
        case SDLK_PERIOD:    return KeyCode::KEY_PERIOD;
        case SDLK_SLASH:     return KeyCode::KEY_SLASH;
        case SDLK_BACKSLASH: return KeyCode::KEY_BACKSLASH;
        case SDLK_LEFTBRACKET:  return KeyCode::KEY_BRACKET_LEFT;
        case SDLK_RIGHTBRACKET: return KeyCode::KEY_BRACKET_RIGHT;
        
        default:
            return KeyCode::KEY_UNKNOWN;
    }
}

//==============================================================================
// Private Methods
//==============================================================================

void SDL2InputHandler::initializeDefaultMappings() {
    // Navigation - arrow keys
    _keyActionMap[KeyCode::KEY_UP]    = InputAction::MOVE_UP;
    _keyActionMap[KeyCode::KEY_DOWN]  = InputAction::MOVE_DOWN;
    _keyActionMap[KeyCode::KEY_LEFT]  = InputAction::MOVE_LEFT;
    _keyActionMap[KeyCode::KEY_RIGHT] = InputAction::MOVE_RIGHT;
    
    // Navigation - vim-style (hjkl)
    _keyActionMap[KeyCode::KEY_H] = InputAction::MOVE_LEFT;
    _keyActionMap[KeyCode::KEY_J] = InputAction::MOVE_DOWN;
    _keyActionMap[KeyCode::KEY_K] = InputAction::MOVE_UP;
    _keyActionMap[KeyCode::KEY_L] = InputAction::MOVE_RIGHT;
    
    // Simulation control
    _keyActionMap[KeyCode::KEY_SPACE]  = InputAction::PAUSE;
    _keyActionMap[KeyCode::KEY_ESCAPE] = InputAction::TOGGLE_PAUSE_MENU;
    
    // UI actions
    _keyActionMap[KeyCode::KEY_F] = InputAction::TOGGLE_HUD;
    
    // Simulation actions
    _keyActionMap[KeyCode::KEY_A] = InputAction::ADD_CREATURES;
    _keyActionMap[KeyCode::KEY_S] = InputAction::SAVE_STATE;
    
    // Menu navigation
    _keyActionMap[KeyCode::KEY_ENTER] = InputAction::MENU_SELECT;
    
    // World editor actions
    _keyActionMap[KeyCode::KEY_PAGE_UP]   = InputAction::INCREASE_SCALE;
    _keyActionMap[KeyCode::KEY_PAGE_DOWN] = InputAction::DECREASE_SCALE;
    _keyActionMap[KeyCode::KEY_N] = InputAction::NEW_SEED;
    _keyActionMap[KeyCode::KEY_EQUALS] = InputAction::INCREASE_FREQ;
    _keyActionMap[KeyCode::KEY_MINUS]  = InputAction::DECREASE_FREQ;
    _keyActionMap[KeyCode::KEY_V] = InputAction::INCREASE_EXPONENT;
    _keyActionMap[KeyCode::KEY_C] = InputAction::DECREASE_EXPONENT;
    
    // Terrain selection (1-9)
    _keyActionMap[KeyCode::KEY_1] = InputAction::SELECT_TERRAIN_1;
    _keyActionMap[KeyCode::KEY_2] = InputAction::SELECT_TERRAIN_2;
    _keyActionMap[KeyCode::KEY_3] = InputAction::SELECT_TERRAIN_3;
    _keyActionMap[KeyCode::KEY_4] = InputAction::SELECT_TERRAIN_4;
    _keyActionMap[KeyCode::KEY_5] = InputAction::SELECT_TERRAIN_5;
    _keyActionMap[KeyCode::KEY_6] = InputAction::SELECT_TERRAIN_6;
    _keyActionMap[KeyCode::KEY_7] = InputAction::SELECT_TERRAIN_7;
    _keyActionMap[KeyCode::KEY_8] = InputAction::SELECT_TERRAIN_8;
    _keyActionMap[KeyCode::KEY_9] = InputAction::SELECT_TERRAIN_9;
    
    _keyActionMap[KeyCode::KEY_W] = InputAction::INCREASE_TERRAIN_LEVEL;
    _keyActionMap[KeyCode::KEY_Q] = InputAction::DECREASE_TERRAIN_LEVEL;
    
    // Zoom controls
    _keyActionMap[KeyCode::KEY_PLUS]   = InputAction::ZOOM_IN;
    _keyActionMap[KeyCode::KEY_EQUALS] = InputAction::ZOOM_IN;   // Unshifted + key
    _keyActionMap[KeyCode::KEY_MINUS]  = InputAction::ZOOM_OUT;
}

InputEvent SDL2InputHandler::processSDLEvent(const SDL_Event& event) {
    InputEvent inputEvent;
    
    // Pass event to ImGui first
#ifdef ECOSIM_HAS_IMGUI
    ImGui_ImplSDL2_ProcessEvent(&event);
    
    // Check if ImGui wants to capture this event
    ImGuiIO& io = ImGui::GetIO();
    bool imguiWantsMouse = io.WantCaptureMouse;
    bool imguiWantsKeyboard = io.WantCaptureKeyboard;
#else
    bool imguiWantsMouse = false;
    bool imguiWantsKeyboard = false;
#endif
    
    switch (event.type) {
        case SDL_QUIT:
            _quitRequested = true;
            inputEvent.key = KeyCode::KEY_ESCAPE;
            inputEvent.action = InputAction::QUIT;
            break;
            
        case SDL_KEYDOWN:
            {
                KeyCode key = mapSDLKey(event.key.keysym.sym);
                _lastKey = key;
                
                // Update key state
                if (key != KeyCode::KEY_NONE && key < KeyCode::COUNT) {
                    _keyStates[static_cast<size_t>(key)] = true;
                }
                
                // If ImGui wants keyboard, don't process for game (except Escape)
                if (imguiWantsKeyboard && key != KeyCode::KEY_ESCAPE) {
                    break;
                }
                
                // Look up action for this key
                InputAction action = getActionForKey(key);
                _lastAction = action;
                
                inputEvent.key = key;
                inputEvent.action = action;
            }
            break;
            
        case SDL_KEYUP:
            {
                KeyCode key = mapSDLKey(event.key.keysym.sym);
                
                // Update key state
                if (key != KeyCode::KEY_NONE && key < KeyCode::COUNT) {
                    _keyStates[static_cast<size_t>(key)] = false;
                }
            }
            break;
            
        case SDL_MOUSEMOTION:
            if (_mouseEnabled) {
                _mouseX = event.motion.x;
                _mouseY = event.motion.y;
            }
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            if (_mouseEnabled && !imguiWantsMouse) {
                inputEvent.hasMouseEvent = true;
                inputEvent.mouseEvent.x = event.button.x;
                inputEvent.mouseEvent.y = event.button.y;
                inputEvent.mouseEvent.pressed = true;
                inputEvent.mouseEvent.released = false;
                
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        inputEvent.mouseEvent.button = MouseButton::LEFT;
                        break;
                    case SDL_BUTTON_RIGHT:
                        inputEvent.mouseEvent.button = MouseButton::RIGHT;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        inputEvent.mouseEvent.button = MouseButton::MIDDLE;
                        break;
                    default:
                        inputEvent.mouseEvent.button = MouseButton::NONE;
                        break;
                }
                
                _mouseX = event.button.x;
                _mouseY = event.button.y;
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (_mouseEnabled && !imguiWantsMouse) {
                inputEvent.hasMouseEvent = true;
                inputEvent.mouseEvent.x = event.button.x;
                inputEvent.mouseEvent.y = event.button.y;
                inputEvent.mouseEvent.pressed = false;
                inputEvent.mouseEvent.released = true;
                
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        inputEvent.mouseEvent.button = MouseButton::LEFT;
                        break;
                    case SDL_BUTTON_RIGHT:
                        inputEvent.mouseEvent.button = MouseButton::RIGHT;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        inputEvent.mouseEvent.button = MouseButton::MIDDLE;
                        break;
                    default:
                        inputEvent.mouseEvent.button = MouseButton::NONE;
                        break;
                }
            }
            break;
            
        case SDL_MOUSEWHEEL:
            if (_mouseEnabled && !imguiWantsMouse) {
                inputEvent.hasMouseEvent = true;
                inputEvent.mouseEvent.scrollDelta = event.wheel.y;
            }
            break;
            
        case SDL_WINDOWEVENT:
            // Could handle window resize, focus, etc. here
            break;
            
        default:
            break;
    }
    
    return inputEvent;
}

void SDL2InputHandler::updateKeyStates() {
    // This could be used to poll SDL keyboard state directly
    // For now, we rely on event-based updates
    const Uint8* sdlKeyState = SDL_GetKeyboardState(nullptr);
    
    // Update states for common keys based on SDL scan codes
    _keyStates[static_cast<size_t>(KeyCode::KEY_UP)] = sdlKeyState[SDL_SCANCODE_UP];
    _keyStates[static_cast<size_t>(KeyCode::KEY_DOWN)] = sdlKeyState[SDL_SCANCODE_DOWN];
    _keyStates[static_cast<size_t>(KeyCode::KEY_LEFT)] = sdlKeyState[SDL_SCANCODE_LEFT];
    _keyStates[static_cast<size_t>(KeyCode::KEY_RIGHT)] = sdlKeyState[SDL_SCANCODE_RIGHT];
    _keyStates[static_cast<size_t>(KeyCode::KEY_SPACE)] = sdlKeyState[SDL_SCANCODE_SPACE];
    _keyStates[static_cast<size_t>(KeyCode::KEY_ESCAPE)] = sdlKeyState[SDL_SCANCODE_ESCAPE];
    _keyStates[static_cast<size_t>(KeyCode::KEY_ENTER)] = sdlKeyState[SDL_SCANCODE_RETURN];
    
    // Letter keys
    _keyStates[static_cast<size_t>(KeyCode::KEY_A)] = sdlKeyState[SDL_SCANCODE_A];
    _keyStates[static_cast<size_t>(KeyCode::KEY_W)] = sdlKeyState[SDL_SCANCODE_W];
    _keyStates[static_cast<size_t>(KeyCode::KEY_S)] = sdlKeyState[SDL_SCANCODE_S];
    _keyStates[static_cast<size_t>(KeyCode::KEY_D)] = sdlKeyState[SDL_SCANCODE_D];
    // ... additional keys as needed
}