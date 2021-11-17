#include "mouse.h"

// Which byte from the mouse packet are we at(out of 3 or 4 usually)
// 4th byte would be for scroll wheel and mouse 4 and 5
uint8_t mouse_cycle = 0;
// The full mouse packet
uint8_t mouse_packet[4];
bool mouse_packet_ready = false;

// Pixel map for the mouse pointer image (16x16)
uint8_t MousePointer[] = {
    0b11111111, 0b11100000, 
    0b11111111, 0b10000000, 
    0b11111110, 0b00000000, 
    0b11111100, 0b00000000, 
    0b11111000, 0b00000000, 
    0b11110000, 0b00000000, 
    0b11100000, 0b00000000, 
    0b11000000, 0b00000000, 
    0b11000000, 0b00000000, 
    0b10000000, 0b00000000, 
    0b10000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 
};

// Mouse position
Point MousePosition;
Point old_mouse_position;

// Handle the mouse data bytes
// https://wiki.osdev.org/PS/2_Mouse
void handle_ps2_mouse(uint8_t data) {
    switch (mouse_cycle) {
        // First byte contains info on mouse buttons and 
        // details about the x and y values in the 2nd and 3rd bytes (sign, overflow etc)
        case 0:
            // If we have a good packet already, dont corrupt it and continue
            if (mouse_packet_ready) break;
            // 4th bit must always be 1, otherwise we are out of sync
            if (data & 0b00001000 == 0) break;
            mouse_packet[0] = data;
            mouse_cycle++;
            break;
        // Contains the x movement
        case 1:
            if (mouse_packet_ready) break;
            mouse_packet[1] = data;
            mouse_cycle++;
            break;
        // Y movement
        case 2:
            if (mouse_packet_ready) break;
            mouse_packet[2] = data;
            mouse_packet_ready = true;
            mouse_cycle = 0;
            break;
    }
}

// Process the current mouse packet if it is ready
void process_mouse_packet() {
    // Mouse packet must be ready
    if (!mouse_packet_ready) return;
    // Fetch conditions from the data
    bool x_negative, y_negative, x_overflow, y_overflow;
    x_negative = (mouse_packet[0] & PS2_X_SIGN_MASK);
    y_negative = (mouse_packet[0] & PS2_Y_SIGN_MASK);
    x_overflow = (mouse_packet[0] & PS2_X_OVERFLOW_MASK);
    y_overflow = (mouse_packet[0] & PS2_Y_OVERFLOW_MASK);
    // Handle data
    // Update x position
    if (!x_negative) {
        MousePosition.X += mouse_packet[1];
        if (x_overflow) MousePosition.X += 255;
    } else {
        mouse_packet[1] = 256 - mouse_packet[1];
        MousePosition.X -= mouse_packet[1];
        if (x_overflow) MousePosition.X -= 255;
    }
    // Update y position
    if (!y_negative) {
        MousePosition.Y -= mouse_packet[2];
        if (y_overflow) MousePosition.Y -= 255;
    } else {
        mouse_packet[2] = 256 - mouse_packet[2];
        MousePosition.Y += mouse_packet[2];
        if (y_overflow) MousePosition.Y += 255;
    }
    // Bound mouse position
    if (MousePosition.X < 0) MousePosition.X = 0;
    if (MousePosition.X > GlobalRenderer->target_frame_buffer->width - 1) MousePosition.X = GlobalRenderer->target_frame_buffer->width - 1;
    if (MousePosition.Y < 0) MousePosition.Y = 0;
    if (MousePosition.Y > GlobalRenderer->target_frame_buffer->height - 1) MousePosition.Y = GlobalRenderer->target_frame_buffer->height - 1;

    // Draw the mouse cursor
    GlobalRenderer->clear_mouse_cursor(MousePointer, old_mouse_position);
    GlobalRenderer->draw_overlay_mouse_cursor(MousePointer, MousePosition, 0xffffffff);

    // Handle button presses
    if (mouse_packet[0] & PS2_LEFT_BTN_MASK) {
        
    }

    if (mouse_packet[0] & PS2_RIGHT_BTN_MASK) {
        
    }

    if (mouse_packet[0] & PS2_MIDDLE_BTN_MASK) {
        
    }

    old_mouse_position = MousePosition;
    mouse_packet_ready = false;
}

// We must wait during communication with the mouse port
// https://wiki.osdev.org/Mouse_Input#Waiting_to_Send_Bytes_to_Port_0x60_and_0x64
void mouse_wait() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if ((inb(0x64) & 0b10) == 0) {
            return;
        }
    }
}

// Before reading from mouse port we must also wait
// https://wiki.osdev.org/Mouse_Input#Waiting_to_Send_Bytes_to_Port_0x60_and_0x64
void mouse_wait_inbound() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if (inb(0x64) & 0b1) {
            return;
        }
    }
}

void mouse_write(uint8_t value) {
    mouse_wait();
    // Tell the mouse we are sending it a command
    outb(0x64, 0xd4);
    mouse_wait();
    // Send the command
    outb(0x60, value);
}

uint8_t mouse_read() {
    mouse_wait_inbound();
    return inb(0x60);
}

// Initialize the mouse through PS2
void init_ps2_mouse() {
    // Enable the auxiliary mouse device
    outb(0x64, 0xa8);
    mouse_wait();
    // Tell the keyboard controller that we want to send mouse commands
    outb(0x64, 0x20);
    mouse_wait_inbound();
    // Check the mouse status, to see if it is ready to receive commands
    uint8_t status = inb(0x60);
    status |= 0b10;
    mouse_wait();
    outb(0x64, 0x60);
    mouse_wait();
    // Setting the correct bit, "compaq status byte"
    outb(0x60, status);
    // Write default setting to mouse
    mouse_write(0xf6);
    mouse_read();
    // Enable the mouse
    mouse_write(0xf4);
    mouse_read();
}