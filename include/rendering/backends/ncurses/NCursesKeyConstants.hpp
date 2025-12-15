/**
 * @file NCursesKeyConstants.hpp
 * @brief NCurses key constant definitions
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file must be included ONLY in .cpp files BEFORE any other headers
 * to capture ncurses key macro values before they can conflict
 * with our KeyCode enum names.
 * 
 * WARNING: Do NOT include this header in any .hpp file, and do NOT
 * include any headers that define KeyCode before including this.
 */

#ifndef ECOSIM_NCURSES_KEY_CONSTANTS_HPP
#define ECOSIM_NCURSES_KEY_CONSTANTS_HPP

// Include ncurses to get the macro values
#include <ncurses.h>

// Namespace to hold captured ncurses key values
namespace NCKeys {
    // Navigation keys
    inline const int UP        = KEY_UP;
    inline const int DOWN      = KEY_DOWN;
    inline const int LEFT      = KEY_LEFT;
    inline const int RIGHT     = KEY_RIGHT;
    inline const int PPAGE     = KEY_PPAGE;
    inline const int NPAGE     = KEY_NPAGE;
    inline const int HOME      = KEY_HOME;
    inline const int END       = KEY_END;
    
    // Function keys
    inline const int F1        = KEY_F(1);
    inline const int F2        = KEY_F(2);
    inline const int F3        = KEY_F(3);
    inline const int F4        = KEY_F(4);
    inline const int F5        = KEY_F(5);
    inline const int F6        = KEY_F(6);
    inline const int F7        = KEY_F(7);
    inline const int F8        = KEY_F(8);
    inline const int F9        = KEY_F(9);
    inline const int F10       = KEY_F(10);
    inline const int F11       = KEY_F(11);
    inline const int F12       = KEY_F(12);
    
    // Editing keys
    inline const int BACKSPACE_KEY = KEY_BACKSPACE;
    inline const int DC        = KEY_DC;  // Delete character
    
    // ASCII values (not macros, but keeping here for consistency)
    inline const int ENTER_ASCII     = 10;
    inline const int ESCAPE_ASCII    = 27;
    inline const int SPACE_ASCII     = 32;
    inline const int TAB_ASCII       = 9;
    inline const int PLUS_ASCII      = 43;
    inline const int MINUS_ASCII     = 45;
    inline const int EQUAL_ASCII     = 61;
    inline const int BACKSPACE_ALT   = 127;
}

// Now undef ALL conflicting macros so KeyCode enum can be used
// Navigation keys
#undef KEY_UP
#undef KEY_DOWN
#undef KEY_LEFT
#undef KEY_RIGHT
#undef KEY_HOME
#undef KEY_END

// These might also be defined as macros in some ncurses versions
#undef KEY_BACKSPACE
#undef KEY_ENTER
#undef KEY_ESCAPE
#undef KEY_SPACE
#undef KEY_TAB
#undef KEY_DELETE

// Function keys (unlikely to conflict but just in case)
#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_F11
#undef KEY_F12

// Other potentially conflicting macros
#undef KEY_PPAGE
#undef KEY_NPAGE
#undef KEY_DC

// Letter/number keys are typically not macros in ncurses
// but undef them just in case some systems define them
#undef KEY_A
#undef KEY_B
#undef KEY_C
#undef KEY_D
#undef KEY_E
#undef KEY_F
#undef KEY_G
#undef KEY_H
#undef KEY_I
#undef KEY_J
#undef KEY_K
#undef KEY_L
#undef KEY_M
#undef KEY_N
#undef KEY_O
#undef KEY_P
#undef KEY_Q
#undef KEY_R
#undef KEY_S
#undef KEY_T
#undef KEY_U
#undef KEY_V
#undef KEY_W
#undef KEY_X
#undef KEY_Y
#undef KEY_Z

#undef KEY_0
#undef KEY_1
#undef KEY_2
#undef KEY_3
#undef KEY_4
#undef KEY_5
#undef KEY_6
#undef KEY_7
#undef KEY_8
#undef KEY_9

#undef KEY_NONE
#undef KEY_UNKNOWN
#undef KEY_SHIFT
#undef KEY_CTRL
#undef KEY_ALT
#undef KEY_MINUS
#undef KEY_PLUS
#undef KEY_EQUALS
#undef KEY_COMMA
#undef KEY_PERIOD
#undef KEY_SLASH
#undef KEY_BACKSLASH
#undef KEY_BRACKET_LEFT
#undef KEY_BRACKET_RIGHT
#undef KEY_PAGE_UP
#undef KEY_PAGE_DOWN

#endif // ECOSIM_NCURSES_KEY_CONSTANTS_HPP