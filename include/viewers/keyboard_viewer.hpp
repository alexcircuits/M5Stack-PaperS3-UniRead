// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

// Not ready yet... not sure it will be used in the future...
// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once
#include "global.hpp"

#include "screen.hpp"
#include "viewers/page.hpp"
#include "controllers/event_mgr.hpp"

/**
 * @brief Keyboard presentation class
 * 
 * This class supply a screen keyboard to be used with a touch screen.
 * 
 */
class KeyboardViewer
{
  private:
    static constexpr char const * TAG = "KeyboardViewer";
    Page::Format fmt;
    
    enum class KBType : int8_t { ALFA, ALFA_SHIFTED, NUMBERS, SPECIAL };

    KBType         current_kb_type;
    uint16_t       width;
    const uint16_t HEIGHT = 240;

    static constexpr char const * alfa_line_1_low = "qwertyuiop" ;
    static constexpr char const * alfa_line_2_low = "asdfghjkl"  ;
    static constexpr char const * alfa_line_3_low = "\1zxcvbnm\b"; // \1 == 'shift', \b == 'backspace'
    static constexpr char const * alfa_line_4     = "\2 \r"      ; // \2 == '123', ' ' == 'space', \r == 'return'

    static constexpr char const * alfa_line_1_upp = "QWERTYUIOP" ;
    static constexpr char const * alfa_line_2_upp = "ASDFGHJKL"  ;
    static constexpr char const * alfa_line_3_upp = "\1ZXCVBNM\b"; // \1 == 'shift', \b == 'backspace'

    static constexpr char const * num_line_1      = "123" ;
    static constexpr char const * num_line_2      = "456";
    static constexpr char const * num_line_3      = "789"  ; 
    static constexpr char const * num_line_4      = " 0\b\r"      ; // ' ' = space, \b = backspace, \r = return/OK

    static constexpr char const * special_line_1  = "[]{}#%^*+=" ;
    static constexpr char const * special_line_2  = "_\\|~<>`/\"@";
    static constexpr char const * special_line_3  = "\2.,?!'";
    static constexpr char const * special_line_4  = "\4 \r";     ; // \4 == 'ABC', ' ' == 'space', \4 == 'return'

    void show_kb(KBType kb_type);
    void show_char(char           ch, uint16_t & x, uint16_t y);
    void show_line(const char * line, uint16_t & x, uint16_t y);
    
    bool load_font();
    
  public:
    KeyboardViewer() : current_kb_type(KBType::ALFA) {};

    typedef void (* UpdateHandler)(char ch);
    bool get_alfanum(char * str, uint16_t len, UpdateHandler handler);
    void get_num(); // Simplified for now, just shows the numpad
    
    bool event(const EventMgr::Event & event, char & char_code);
};

extern KeyboardViewer keyboard_viewer;

#if __KEYBOARD_VIEWER__
  KeyboardViewer keyboard_viewer;
#else
  extern KeyboardViewer keyboard_viewer;
#endif
