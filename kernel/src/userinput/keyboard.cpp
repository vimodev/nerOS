#include "keyboard.h"

// Status of functional pressed keys
bool is_left_shift_pressed = false;
bool is_right_shift_pressed = false;

// handle a keyboard press based on scan_code
void handle_keyboard(uint8_t scan_code) {
    // Hndle special scan_codes
    switch (scan_code) {
        case LEFT_SHIFT:
            is_left_shift_pressed = true;
            return;
        case LEFT_SHIFT + RELEASED_DELTA:
            is_left_shift_pressed = false;
            return;
        case RIGHT_SHIFT:
            is_right_shift_pressed = true;
            return;
        case RIGHT_SHIFT + RELEASED_DELTA:
            is_right_shift_pressed = false;
            return;
        case ENTER:
            GlobalRenderer->next();
            return;
        case SPACEBAR:
            GlobalRenderer->put_char(' ');
            return;
        case BACKSPACE:
            GlobalRenderer->clear_char();
            return;     
    }
    // Look up ascii for the scan_code
    char ascii = QWERTYKeyboard::translate(scan_code, is_left_shift_pressed | is_right_shift_pressed);
    // Render the char
    if (ascii != 0) {
        GlobalRenderer->put_char(ascii);
    }
}