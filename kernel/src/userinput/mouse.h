#pragma once

#include "../io/io.h"
#include "../utility/math.h"
#include "../graphics/BasicRenderer.h"

#define PS2_LEFT_BTN_MASK       0b00000001
#define PS2_RIGHT_BTN_MASK      0b00000010
#define PS2_MIDDLE_BTN_MASK     0b00000100

#define PS2_X_SIGN_MASK         0b00010000
#define PS2_Y_SIGN_MASK         0b00100000
#define PS2_X_OVERFLOW_MASK     0b01000000
#define PS2_Y_OVERFLOW_MASK     0b10000000

extern uint8_t MousePointer[];

void handle_ps2_mouse(uint8_t data);
void process_mouse_packet();
void init_ps2_mouse();

extern Point MousePosition;