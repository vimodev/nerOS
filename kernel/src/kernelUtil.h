#pragma once

#include <stdint.h>
#include "BasicRenderer.h"
#include "cstr.h"
#include "efiMemory.h"
#include "memory.h"
#include "Bitmap.h"
#include "paging/PageFrameAllocator.h"
#include "paging/PageMapIndexer.h"
#include "paging/paging.h"
#include "paging/PageTableManager.h"
#include "gdt/gdt.h"

struct BootInfo {
	Framebuffer* framebuffer;
	PSF1_FONT* psf1_font;
	EFI_MEMORY_DESCRIPTOR* memory_map;
	uint64_t memory_map_size;
	uint64_t memory_map_descriptor_size;
} ;

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

struct KernelInfo {
    PageTableManager* page_table_manager;
};

KernelInfo initialize_kernel(BootInfo* boot_info);