#include "PageMapIndexer.h"

// From a virtual address, compute the Page Table indices 
// for easy repeated access
PageMapIndexer::PageMapIndexer(uint64_t virtual_address){
    // Skip attribute and A bits
    virtual_address >>= 12;
    // Lower 9 bits dictate last index
    P_i = virtual_address & 0x1ff;
    virtual_address >>= 9;
    // After those 9, 9 dictate second to last
    PT_i = virtual_address & 0x1ff;
    virtual_address >>= 9;
    // etc
    PD_i = virtual_address & 0x1ff;
    virtual_address >>= 9;
    PDP_i = virtual_address & 0x1ff;
}

