#pragma once

#include <stdint.h>

#include "../graphics/Framebuffer.h"
#include "../graphics/simpleFonts.h"
#include "../memory/efiMemory.h"
#include "../io/acpi.h"
#include "../paging/PageTableManager.h"

struct BootInfo {
	Framebuffer* framebuffer;
	PSF1_FONT* psf1_font;
	EFI_MEMORY_DESCRIPTOR* memory_map;
	uint64_t memory_map_size;
	uint64_t memory_map_descriptor_size;
	ACPI::RSDP2 *rsdp;
} ;

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

struct KernelInfo {
    PageTableManager* page_table_manager;
};

KernelInfo initialize_kernel(BootInfo* boot_info);