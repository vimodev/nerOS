#pragma once

#include <stdint.h>

#include "../io/pci.h"

// https://wiki.osdev.org/AHCI
// For interacting with mass storage devices through AHCI
namespace AHCI {

    class AHCIDriver {
        public:
            AHCIDriver(PCI::PCIDeviceHeader *base_address);
            ~AHCIDriver();
            PCI::PCIDeviceHeader *pci_base_address;
    };

}