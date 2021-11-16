#pragma once

#include <stdint.h>

namespace QWERTYKeyboard {

    // Special scan_codes
    #define LEFT_SHIFT 0x2a
    #define RIGHT_SHIFT 0x36
    #define ENTER 0x1c
    #define BACKSPACE 0x0e
    #define SPACEBAR 0x39

    // Released scan_code is this much higher than pressed
    #define RELEASED_DELTA 0x80

    extern const char ascii_table[];
    char translate(uint8_t scan_code, bool upper_case);
}