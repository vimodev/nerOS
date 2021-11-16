#include "interrupts.h"

__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *frame) {
    panic("Page fault detected.");
    while (true);
}

__attribute__((interrupt)) void double_fault_handler(struct interrupt_frame *frame) {
    panic("Double fault detected.");
    while (true);
}

__attribute__((interrupt)) void general_protection_fault_handler(struct interrupt_frame *frame) {
    panic("General protection fault detected.");
    while (true);
}

__attribute__((interrupt)) void keyboard_interrupt_handler(struct interrupt_frame *frame) {
    // PS2 keyboard is port 0x60
    // Get the scan code
    uint8_t scan_code = inb(0x60);
    // Handle the key press
    handle_keyboard(scan_code);
    // End the interrupt
    pic_end_master();
}

// Tell the master PIC chip to end the interrupt
void pic_end_master() {
    outb(PIC1_COMMAND, PIC_EOI);
}

// Tell the slave PIC chip to end the interrupt
void pic_end_slave() {
    outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

// Remap the PIC chip such that its interrupts do not collide with exceptions
void remap_pic() {
    uint8_t a1, a2;

    // Fetch current bitmasks
    a1 = inb(PIC1_DATA);
    io_wait();
    a2 = inb(PIC2_DATA);
    io_wait();

    // Initialize master PIC chip
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // Initialize slave PIC chip
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // Inform the PIC chips of their interrupts offsets
    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    // Tell the PIC chips of one another's existence
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    // Tell PIC's to operate in 80_86 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore previous bitmasks
    outb(PIC1_DATA, a1);
    io_wait();
    outb(PIC2_DATA, a2);
    io_wait();

}