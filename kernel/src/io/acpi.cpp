#include "acpi.h"

namespace ACPI {

    // Find the given System Descriptor Table
    void *find_table(SDTHeader *sdt_header, char *signature) {
        int entries = (sdt_header->length - sizeof(ACPI::SDTHeader)) / 8;
        for (int t = 0; t < entries; t++) {
            // From xsdt base, add header size, and pick the t'th entry, and then cast it to SDTHeader *
            ACPI::SDTHeader *new_sdt_header = (ACPI::SDTHeader *) *(uint64_t *)((uint64_t)sdt_header + sizeof(ACPI::SDTHeader) + (t * 8));
            // Match signature with desired one
            for (int i = 0; i < 4; i++) {
                if (new_sdt_header->signature[i] != signature[i]) break;
                if (i == 3) return new_sdt_header;
            }
        }
        return 0;
    }

}