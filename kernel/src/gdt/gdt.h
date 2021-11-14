#pragma once

#include <stdint.h>

// Contains information about the GDT
struct GDTDescriptor {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed));

// One GDT entry
// https://wiki.osdev.org/Global_Descriptor_Table
struct GDTEntry {
    uint16_t limit_0;
    uint16_t base_0;
    uint8_t base_1;
    uint8_t access_byte;
    uint8_t limit_1_flags;
    uint8_t base_2;
}__attribute((packed));

// The global descriptor table
struct GDT {
    GDTEntry null;
    GDTEntry kernel_code;
    GDTEntry kernel_data;
    GDTEntry user_null;
    GDTEntry user_code;
    GDTEntry user_data;
}__attribute__((packed))__attribute__((aligned(0x1000)));

extern GDT DefaultGDT;

extern "C" void load_gdt(GDTDescriptor *gdt_descriptor);