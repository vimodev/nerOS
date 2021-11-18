#pragma once

#include <stdint.h>

#include "efiMemory.h"

uint64_t get_memory_size(EFI_MEMORY_DESCRIPTOR* memory_map, uint64_t memory_map_entries, uint64_t memory_map_descriptor_size);
void memset(void* start, uint8_t value, uint64_t num);