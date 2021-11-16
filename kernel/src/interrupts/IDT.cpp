#include "IDT.h"

// Set the IDT's offset
void IDTDescriptorEntry::set_offset(uint64_t offset) {
    this->offset_0 = (uint16_t)(offset & 0x000000000000ffff);
    this->offset_1 = (uint16_t)((offset & 0x00000000ffff0000) >> 16);
    this->offset_2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

// Get the IDT's offset
uint64_t IDTDescriptorEntry::get_offset() {
    uint64_t offset = 0;
    offset |= (uint64_t) this->offset_0;
    offset |= (uint64_t) this->offset_1 << 16;
    offset |= (uint64_t) this->offset_2 << 32;
    return offset;
}