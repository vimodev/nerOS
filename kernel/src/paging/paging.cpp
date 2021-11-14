#include "paging.h"

// Set the given flag to enabled
void PageDirectoryEntry::set_flag(PT_Flag flag, bool enabled){
    uint64_t bit_selector = (uint64_t)1 << flag;
    value &= ~bit_selector;
    if (enabled){
        value |= bit_selector;
    }
}

// Get the value of the given flag
bool PageDirectoryEntry::get_flag(PT_Flag flag){
    uint64_t bit_selector = (uint64_t)1 << flag;
    return value & bit_selector > 0 ? true : false;
}

// Get the address of the PDE
uint64_t PageDirectoryEntry::get_address(){
    return (value & 0x000ffffffffff000) >> 12;
}

// Set the address to the given one
void PageDirectoryEntry::set_address(uint64_t address){
    address &= 0x000000ffffffffff;
    value &= 0xfff0000000000fff;
    value |= (address << 12);
}