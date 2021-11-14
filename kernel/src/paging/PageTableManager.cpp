#include "PageTableManager.h"
#include "PageMapIndexer.h"
#include <stdint.h>
#include "PageFrameAllocator.h"
#include "../memory.h"

PageTableManager::PageTableManager(PageTable* pml4_address) {
    this->PML4 = pml4_address;
}

// Map the given virtual memory to the given physical memory
// PML4 -> 512 PDP -> 512 PD -> 512 PT
// We basically traverse down the recursive tables using the indexer's indices
void PageTableManager::map_memory(void* virtual_memory, void* physical_memory) {
    // Set up an indexer for the given address
    PageMapIndexer indexer = PageMapIndexer((uint64_t)virtual_memory);

    // Fetch the right PDP table for the given virtual address
    PageDirectoryEntry PDE;
    PDE = PML4->entries[indexer.PDP_i];
    PageTable* PDP;
    // If it is not present yet, we create it
    if (!PDE.get_flag(PT_Flag::Present)){
        // Allocate it some memory
        PDP = (PageTable*)GlobalAllocator.request_page();
        memset(PDP, 0, 0x1000);
        // Set the address and the flags
        PDE.set_address((uint64_t)PDP >> 12);
        PDE.set_flag(PT_Flag::Present, true);
        PDE.set_flag(PT_Flag::ReadWrite, true);
        PML4->entries[indexer.PDP_i] = PDE;
    } else {
        PDP = (PageTable*)((uint64_t)PDE.get_address() << 12);
    }
    
    // In that PDP table, fetch the correct PD table
    PDE = PDP->entries[indexer.PD_i];
    PageTable* PD;
    // If it does not exist, create it once more
    if (!PDE.get_flag(PT_Flag::Present)){
        PD = (PageTable*)GlobalAllocator.request_page();
        memset(PD, 0, 0x1000);
        PDE.set_address((uint64_t)PD >> 12);
        PDE.set_flag(PT_Flag::Present, true);
        PDE.set_flag(PT_Flag::ReadWrite, true);
        PDP->entries[indexer.PD_i] = PDE;
    } else {
        PD = (PageTable*)((uint64_t)PDE.get_address() << 12);
    }

    // In the found PD table, we fetch the correct PT table
    PDE = PD->entries[indexer.PT_i];
    PageTable* PT;
    // Create it if not there yet, as before
    if (!PDE.get_flag(PT_Flag::Present)){
        PT = (PageTable*)GlobalAllocator.request_page();
        memset(PT, 0, 0x1000);
        PDE.set_address((uint64_t)PT >> 12);
        PDE.set_flag(PT_Flag::Present, true);
        PDE.set_flag(PT_Flag::ReadWrite, true);
        PD->entries[indexer.PT_i] = PDE;
    } else {
        PT = (PageTable*)((uint64_t)PDE.get_address() << 12);
    }

    // Set the actual desired page table entry
    PDE = PT->entries[indexer.P_i];
    PDE.set_address((uint64_t)physical_memory >> 12);
    PDE.set_flag(PT_Flag::Present, true);
    PDE.set_flag(PT_Flag::ReadWrite, true);
    PT->entries[indexer.P_i] = PDE;
}