#pragma once

#include <stdint.h>

#define IDT_TA_InterruptGate    0b10001110
#define IDT_TA_CallGate         0b10001100
#define IDT_TA_TrapGate         0b10001111

// Contains what to do on an interrupt
struct IDTDescriptorEntry {
    uint16_t offset_0;
    uint16_t selector; // Segment selector
    uint8_t ist;
    uint8_t type_attr; // What kind of IDT
    uint16_t offset_1;
    uint32_t offset_2;
    uint32_t ignore;
    void set_offset(uint64_t offset);
    uint64_t get_offset();
};

// IDT Register information
struct IDTR {
    uint16_t limit;
    uint64_t offset;
}__attribute__((packed));