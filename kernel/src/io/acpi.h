#pragma once

#include <stdint.h>

namespace ACPI {
    
    // Root system descriptor pointer
    struct RSDP2 {
        unsigned char signature[8];
        uint8_t checksum;
        uint8_t oem_id[6];
        uint8_t revision;
        uint32_t rsdt_address;
        uint32_t length;
        uint64_t xsdt_address; // Extended system descriptor table
        uint8_t extended_checksum;
        uint8_t reserved[3];
    }__attribute__((packed));

    // System descriptor tables all have the same header format
    struct SDTHeader {
        unsigned char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        uint8_t oem_id[6];
        uint8_t oem_table_id[8];
        uint32_t oem_revision;
        uint32_t creator_id;
        uint32_t creator_revision;
    }__attribute__((packed));

    // MCFG header, header for table that contains all info about PCI bus
    struct MCFGHeader {
        SDTHeader header;
        uint64_t reserved;
    }__attribute__((packed));

    // All configuration of a PCI device
    struct DeviceConfig {
        uint64_t base_address;
        uint16_t pci_seg_group;
        uint8_t start_bus;
        uint8_t end_bus;
        uint32_t reserved;
    }__attribute__((packed));

    void *find_table(SDTHeader *sdt_header, char *signature);

}