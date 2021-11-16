#include "mouse.h"

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