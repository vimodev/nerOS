#include "ahci.h"

#include "../graphics/BasicRenderer.h"

// Used to interact with SATA storage devices
namespace AHCI {

    // Initialize the driver with a PCI address
    AHCIDriver::AHCIDriver(PCI::PCIDeviceHeader *base_address) {
        this->pci_base_address = base_address;
        GlobalRenderer->println("AHCI Driver instance started.");
    }

    // Destroy everything
    AHCIDriver::~AHCIDriver() {

    }

}