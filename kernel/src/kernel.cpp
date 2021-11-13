#include <stddef.h>
#include "BasicRenderer.h"
#include "cstr.h"
#include "efiMemory.h"

struct BootInfo {
	Framebuffer *framebuffer;
	PSF1_FONT *font;
	void *mMap;
	uint64_t mMapSize;
	uint64_t mMapDescSize;
};

extern "C" void _start(BootInfo *bootInfo) {

	// Initialize renderer with framebuffer and font
	BasicRenderer renderer(bootInfo->framebuffer, bootInfo->font);

	char str[] = "Hello kernel!\n";
	renderer.print(str);

	renderer.print("Hello kernel!\n");

	uint64_t mMapEntries = bootInfo->mMapSize / bootInfo->mMapDescSize;

	for (int i = 0; i < mMapEntries; i++) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)bootInfo->mMap + (i * bootInfo->mMapDescSize));
		//renderer.print(EFI_MEMORY_TYPE_STRINGS[desc->type]); renderer.print("\n");
	}

    return;
}