#include "io.h"

// Send a byte to the given port
void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Receive a byte from the given io port
uint8_t inb(uint16_t port) {
    uint8_t return_value;
    asm volatile ("inb %1, %0"
    : "=a"(return_value)
    : "Nd"(port));
    return return_value;
}

// Wait for one IO cycle
void io_wait() {
    // Send a 0 to an unused port to wait one IO cycle
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}