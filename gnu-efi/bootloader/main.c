#include <efi.h>
#include <efilib.h>
#include <elf.h>

typedef unsigned long long size_t;

// Framebuffer structure for graphics rendering
typedef struct {
	void *BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanline;
} Framebuffer;
Framebuffer framebuffer;

// Font structures
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;
typedef struct {
	PSF1_HEADER *psf1_header;
	void *glyphBuffer;
} PSF1_FONT;

typedef struct {
	Framebuffer *framebuffer;
	PSF1_FONT *font;
	EFI_MEMORY_DESCRIPTOR *mMap;
	UINTN mMapSize;
	UINTN mMapDescSize;
} BootInfo;

// Initialize GOP
UINTN InitializeGOP() {
	// Get the GOP GUID
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status;

	// Attempt to call the GOP wrapper
	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void **) &gop);
	if (EFI_ERROR(status)) {
		return EFI_NOT_FOUND;
	}

	// Form framebuffer obj
	framebuffer.BaseAddress = (void *) gop->Mode->FrameBufferBase;
	framebuffer.BufferSize = gop->Mode->FrameBufferSize;
	framebuffer.Width = gop->Mode->Info->HorizontalResolution;
	framebuffer.Height = gop->Mode->Info->VerticalResolution;
	framebuffer.PixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;

	return EFI_SUCCESS;
}

// Load a file
EFI_FILE *LoadFile(EFI_FILE *Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
	EFI_FILE *LoadedFile;

	// Get the image we booted from
	EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**) &LoadedImage);

	// Get the file system from the image
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**) &FileSystem);

	// If Directory is NULL we open the root of the FS
	if (Directory == NULL) {
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	// Open path
	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (s != EFI_SUCCESS) {
		return NULL;
	}
	return LoadedFile;

}

// Load a PSF1 font file
PSF1_FONT *LoadPSF1Font(EFI_FILE *Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
	EFI_FILE* font = LoadFile(Directory, Path, ImageHandle, SystemTable);
	if (font == NULL) {
		return NULL;
	}

	// Load font header
	PSF1_HEADER *fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void **) &fontHeader);
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, fontHeader);

	// Check font validity
	if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1) {
		return NULL;
	}

	// Load glyph buffer
	UINTN glyphBufferSize = fontHeader->charsize * 256;
	if (fontHeader->mode == 1) { // 512 glyph mode
		glyphBufferSize = glyphBufferSize * 2;
	}
	void *glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void **) &glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);
	}

	// Create the font structure
	PSF1_FONT *finishedFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void **) &finishedFont);
	finishedFont->psf1_header = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;

	return finishedFont;

}

// memcmp implementation to compare 2 pieces of memory for n bytes
int memcmp(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

// EFI entry function
EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	
	// Allows access to useful EFI functionality
	InitializeLib(ImageHandle, SystemTable);

	// Load the kernel.elf file
	Print(L"Attempting to load kernel.\n\r");
	CHAR16 *kernelName = L"kernel.elf";
	EFI_FILE *Kernel = LoadFile(NULL, kernelName, ImageHandle, SystemTable);
	if (Kernel == NULL) {
		Print(L"ERROR Unable to locate kernel.\n\r");
		return EFI_NOT_FOUND;
	} else {
		Print(L"Kernel located successfully.\n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO *FileInfo;
		// Get the size of the kernel file info
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		// Allocate the kernel file info size worth of memory
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void **) &FileInfo);
		// Get the kernel file info
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void **) &FileInfo);

		// Read the kernel file header contents
		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}

	// Verify that it is a valid elf file
	if (
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	) {
		Print(L"ERROR Kernel format is bad.\n\r");
		return EFI_LOAD_ERROR;
	} else {
		Print(L"Kernel header verified.\n\r");
	}

	// Get the kernel program headers
	Elf64_Phdr *phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void **) &phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	// Iterate through headers
	for (
		Elf64_Phdr *phdr = phdrs;
		(char *) phdrs < (char *) phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr *) ((char *) phdr + header.e_phentsize)
	) {
		// If the header is of type PT_LOAD, we load
		if (phdr->p_type == PT_LOAD) {
			// Calculate necessary amount of pages, rounded up to be sure
			int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
			Elf64_Addr segment = phdr->p_paddr;
			// Allocate the pages
			SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);
			// Set the file pointer to the correct point in the kernel file
			Kernel->SetPosition(Kernel, phdr->p_offset);
			// Read the kernel file into the allocated segment
			UINTN size = phdr->p_filesz;
			Kernel->Read(Kernel, &size, (void *) segment);
			break;
		}
	}
	Print(L"Kernel loaded successfully.\n\r");

	// Locate and load font
	CHAR16 *fontName = L"zap-light16.psf";
	PSF1_FONT *font = LoadPSF1Font(NULL, fontName, ImageHandle, SystemTable);
	if (font == NULL) {
		Print(L"Unable to load valid font: %s.\n\r", fontName);
		return EFI_LOAD_ERROR;
	}
	Print(L"Loaded font %s with char size %d.\n\r", fontName, font->psf1_header->charsize);

	// Initialize GOP
	UINTN gopStatus = InitializeGOP();
	if (gopStatus == EFI_NOT_FOUND) {
		Print(L"Unable to locate GOP.\n\r");
		return EFI_NOT_FOUND;
	}

	// Get the memory map
	EFI_MEMORY_DESCRIPTOR *Map = NULL;
	UINTN MapSize, MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
	{
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void **) &Map);
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
	}

	// Form structure to pass information to kernel
	BootInfo bootInfo;
	bootInfo.framebuffer = &framebuffer;
	bootInfo.font = font;
	bootInfo.mMap = Map;
	bootInfo.mMapSize = MapSize;
	bootInfo.mMapDescSize = DescriptorSize;

	// Locate Kernel Entry
	void (*KernelStart)(BootInfo *) = ((__attribute__((sysv_abi)) void (*)(BootInfo *) ) header.e_entry);
	// Call the Kernel Entry
	KernelStart(&bootInfo);

	// Exit the EFI boot services
	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

	return EFI_SUCCESS; // Exit the UEFI application
}
