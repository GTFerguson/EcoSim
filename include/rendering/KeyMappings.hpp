/**
 * @file KeyMappings.hpp
 * @brief Shared key-to-action mappings for all input handlers
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file provides centralized default key mappings used by all rendering
 * backends. Backend-specific key translation (e.g., SDL_SCANCODE_W → KEY_W
 * or ncurses KEY_UP → KEY_UP) is handled by each backend, but the mapping
 * from our KeyCode enum to InputAction enum is shared here.
 */

#ifndef ECOSIM_KEY_MAPPINGS_HPP
#define ECOSIM_KEY_MAPPINGS_HPP

#include "IInputHandler.hpp"
#include <map>

/**
 * @brief Provides shared default key-to-action mappings for all backends
 * 
 * This namespace centralizes key mapping definitions to avoid duplication
 * between NCurses and SDL2 input handlers. Each backend translates their
 * native key codes to KeyCode values, then uses these shared mappings to
 * determine the InputAction.
 */
namespace KeyMappings {

/**
 * @brief Get the default key-to-action mapping table
 * 
 * Returns a map of KeyCode to InputAction representing the game's
 * default control scheme. Input handlers should copy this map during
 * initialization.
 * 
 * @return Unordered map of KeyCode to InputAction
 */
inline std::map<KeyCode, InputAction> getDefaultMappings() {
    std::map<KeyCode, InputAction> mappings;
    
    //==========================================================================
    // Navigation - Arrow keys
    //==========================================================================
    mappings[KeyCode::KEY_UP]    = InputAction::MOVE_UP;
    mappings[KeyCode::KEY_DOWN]  = InputAction::MOVE_DOWN;
    mappings[KeyCode::KEY_LEFT]  = InputAction::MOVE_LEFT;
    mappings[KeyCode::KEY_RIGHT] = InputAction::MOVE_RIGHT;
    
    //==========================================================================
    // Navigation - Vim-style (HJKL)
    //==========================================================================
    mappings[KeyCode::KEY_H] = InputAction::MOVE_LEFT;
    mappings[KeyCode::KEY_J] = InputAction::MOVE_DOWN;
    mappings[KeyCode::KEY_K] = InputAction::MOVE_UP;
    mappings[KeyCode::KEY_L] = InputAction::MOVE_RIGHT;
    
    //==========================================================================
    // Simulation control
    //==========================================================================
    mappings[KeyCode::KEY_SPACE]  = InputAction::PAUSE;
    mappings[KeyCode::KEY_ESCAPE] = InputAction::QUIT;
    
    //==========================================================================
    // UI actions
    //==========================================================================
    mappings[KeyCode::KEY_F] = InputAction::TOGGLE_HUD;
    
    //==========================================================================
    // Simulation actions
    //==========================================================================
    mappings[KeyCode::KEY_A] = InputAction::ADD_CREATURES;
    mappings[KeyCode::KEY_S] = InputAction::SAVE_STATE;
    
    //==========================================================================
    // Menu navigation
    //==========================================================================
    mappings[KeyCode::KEY_ENTER] = InputAction::MENU_SELECT;
    
    //==========================================================================
    // World editor actions
    //==========================================================================
    mappings[KeyCode::KEY_PAGE_UP]   = InputAction::INCREASE_SCALE;
    mappings[KeyCode::KEY_PAGE_DOWN] = InputAction::DECREASE_SCALE;
    mappings[KeyCode::KEY_N] = InputAction::NEW_SEED;
    mappings[KeyCode::KEY_EQUALS] = InputAction::INCREASE_FREQ;
    mappings[KeyCode::KEY_MINUS]  = InputAction::DECREASE_FREQ;
    mappings[KeyCode::KEY_V] = InputAction::INCREASE_EXPONENT;
    mappings[KeyCode::KEY_C] = InputAction::DECREASE_EXPONENT;
    
    //==========================================================================
    // Terrain selection (1-9)
    //==========================================================================
    mappings[KeyCode::KEY_1] = InputAction::SELECT_TERRAIN_1;
    mappings[KeyCode::KEY_2] = InputAction::SELECT_TERRAIN_2;
    mappings[KeyCode::KEY_3] = InputAction::SELECT_TERRAIN_3;
    mappings[KeyCode::KEY_4] = InputAction::SELECT_TERRAIN_4;
    mappings[KeyCode::KEY_5] = InputAction::SELECT_TERRAIN_5;
    mappings[KeyCode::KEY_6] = InputAction::SELECT_TERRAIN_6;
    mappings[KeyCode::KEY_7] = InputAction::SELECT_TERRAIN_7;
    mappings[KeyCode::KEY_8] = InputAction::SELECT_TERRAIN_8;
    mappings[KeyCode::KEY_9] = InputAction::SELECT_TERRAIN_9;
    
    //==========================================================================
    // Terrain level adjustment
    //==========================================================================
    mappings[KeyCode::KEY_W] = InputAction::INCREASE_TERRAIN_LEVEL;
    mappings[KeyCode::KEY_Q] = InputAction::DECREASE_TERRAIN_LEVEL;
    
    //==========================================================================
    // Zoom controls
    //==========================================================================
    mappings[KeyCode::KEY_PLUS] = InputAction::ZOOM_IN;
    
    return mappings;
}

} // namespace KeyMappings

#endif // ECOSIM_KEY_MAPPINGS_HPP
