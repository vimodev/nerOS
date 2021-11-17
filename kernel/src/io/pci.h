#pragma once

#include <stdint.h>

#include "acpi.h"
#include "../paging/PageTableManager.h"
#include "../graphics/BasicRenderer.h"
#include "../utility/cstr.h"

namespace PCI {

    // Header for a PCI device
    struct PCIDeviceHeader {
        uint16_t vendor_id;
        uint16_t device_id;
        uint16_t command;
        uint16_t status;
        uint8_t revision_id;
        uint8_t program_interface;
        uint8_t device_subclass;
        uint8_t device_class;
        uint8_t cache_line_size;
        uint8_t latency_timer;
        uint8_t header_type;
        uint8_t bist;
    };

    void enumerate_pci(ACPI::MCFGHeader *mcfg);

}