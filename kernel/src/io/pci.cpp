#include "pci.h"

// https://wiki.osdev.org/PCI
// Each bus has 32 devices and each device has 8 functions
// We enumerate recursively over all of each
namespace PCI {

    // Enumerate over a function
    void enumerate_function(uint64_t device_address, uint64_t function) {
        uint64_t offset = function << 12;
        uint64_t function_address = device_address + offset;
        // Map the table to memory
        GlobalPageTableManager.map_memory((void *) function_address, (void *) function_address);
        // Get the device headers
        PCIDeviceHeader *pci_device_header = (PCIDeviceHeader *) function_address;
        // If not valid return
        if (pci_device_header->device_id == 0) return;
        if (pci_device_header->device_id == 0xFFFF) return;

        GlobalRenderer->print(to_hstring(pci_device_header->vendor_id));
        GlobalRenderer->print(" ");
        GlobalRenderer->println(to_hstring(pci_device_header->device_id));
    }

    // Enumerate over all function over all devices
    void enumerate_device(uint64_t bus_address, uint64_t device) {
        uint64_t offset = device << 15;
        uint64_t device_address = bus_address + offset;
        // Map the table to memory
        GlobalPageTableManager.map_memory((void *) device_address, (void *) device_address);
        // Get the device headers
        PCIDeviceHeader *pci_device_header = (PCIDeviceHeader *) device_address;
        // If not valid return
        if (pci_device_header->device_id == 0) return;
        if (pci_device_header->device_id == 0xFFFF) return;
        // There are 8 functions on a PCI device
        for (uint64_t function = 0; function < 8; function++) {
            enumerate_function(device_address, function);
        }
    }

    // Enumerate over all devices on a specific PCI bus
    void enumerate_bus(uint64_t base_address, uint64_t bus) {
        uint64_t offset = bus << 20;
        uint64_t bus_address = base_address + offset;
        // Map the table to memory
        GlobalPageTableManager.map_memory((void *) bus_address, (void *) bus_address);
        // Get the device headers
        PCIDeviceHeader *pci_device_header = (PCIDeviceHeader *) bus_address;
        // If not valid return
        if (pci_device_header->device_id == 0) return;
        if (pci_device_header->device_id == 0xFFFF) return;
        // There are 32 devices on a PCI bus
        for (uint64_t device = 0; device < 32; device++) {
            enumerate_device(bus_address, device);
        }
    }

    // Find all connected PCI devices
    void enumerate_pci(ACPI::MCFGHeader *mcfg) {
        // #entries = (MCFG size - MCFG header) / size(DeviceConfig)
        int entries = ((mcfg->header.length) - sizeof(ACPI::MCFGHeader)) / sizeof(ACPI::DeviceConfig);
        // Go over all configs
        for (int t = 0; t < entries; t++) {
            // Find the config as: base + header + size(DeviceConfig)*t
            ACPI::DeviceConfig *new_device_config = 
                (ACPI::DeviceConfig *) ((uint64_t) mcfg + sizeof(ACPI::MCFGHeader) + (sizeof(ACPI::DeviceConfig) * t));
            // Iterate over all busses in a device
            for (uint64_t bus = new_device_config->start_bus; bus < new_device_config->end_bus; bus++) {
                // Enumerate over all devices on the bus
                enumerate_bus(new_device_config->base_address, bus);
            }
        }
    }

}
