#include <efi.h>
#include <efilib.h>
#include <elf.h>

typedef unsigned long long size_t;

// Structure to hold framebuffer data
typedef struct {
	void* base_address; // Frame buffer address
	size_t buffer_size; // Size
	unsigned int width; 
	unsigned int height;
	unsigned int pixels_per_scanline; // width is more than screenwidth
} Framebuffer;

// PSF1 font format signature bytes
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

// PSF1 font file header
typedef struct {
	unsigned char magic[2]; // Magic signature bytes
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;
// PSF1 font
typedef struct {
	PSF1_HEADER* psf1_header;
	void* glyph_buffer; // Buffer with symbols
} PSF1_FONT;

// Globally addressable framebuffer object
Framebuffer framebuffer;

// Initialize GOP for rendering to the buffer
Framebuffer* initialize_gop() {
	// Fetch GOP info
	EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	// Attempt to call the EUFI GOP wrapper
	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void**)&gop);
	if(EFI_ERROR(status)){
		Print(L"Unable to locate GOP\n\r");
		return NULL;
	} else {
		Print(L"GOP located\n\r");
	}

	// Form the frame buffer struct
	framebuffer.base_address = (void*)gop->Mode->FrameBufferBase;
	framebuffer.buffer_size = gop->Mode->FrameBufferSize;
	framebuffer.width = gop->Mode->Info->HorizontalResolution;
	framebuffer.height = gop->Mode->Info->VerticalResolution;
	framebuffer.pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;

	return &framebuffer;
}

// Load a file from the image
EFI_FILE* load_file(EFI_FILE* directory, CHAR16* path, EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
	EFI_FILE* loaded_file;

	// Fetch the loaded image
	EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
	system_table->BootServices->HandleProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (void**)&loaded_image);

	// Fetch the file system from the image
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_system;
	system_table->BootServices->HandleProtocol(loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&file_system);

	// If directory is null open the root dir
	if (directory == NULL) {
		file_system->OpenVolume(file_system, &directory);
	}

	// Attempt to open the path
	EFI_STATUS s = directory->Open(directory, &loaded_file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (s != EFI_SUCCESS) {
		return NULL;
	}

	return loaded_file;
}

// Load a PSF1 font file
PSF1_FONT* load_psf1_font(EFI_FILE* directory, CHAR16* path, EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {

	// Load the font file
	EFI_FILE* font = load_file(directory, path, image_handle, system_table);
	if (font == NULL) return NULL;

	// Fetch header data
	PSF1_HEADER* font_header;
	system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&font_header);
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, font_header);

	// Check for the PSF1 signature bytes
	if (font_header->magic[0] != PSF1_MAGIC0 || font_header->magic[1] != PSF1_MAGIC1){
		return NULL;
	}

	// Fetch font file data
	UINTN glyph_buffer_size = font_header->charsize * 256;
	if (font_header->mode == 1) { //512 glyph mode
		glyph_buffer_size = font_header->charsize * 512;
	}

	// Fetch font file contents
	void* glyph_buffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		system_table->BootServices->AllocatePool(EfiLoaderData, glyph_buffer_size, (void**)&glyph_buffer);
		font->Read(font, &glyph_buffer_size, glyph_buffer);
	}

	// Form final font struct
	PSF1_FONT* finished_font;
	system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finished_font);
	finished_font->psf1_header = font_header;
	finished_font->glyph_buffer = glyph_buffer;

	return finished_font;
}

int memcmp(const void* aptr, const void* bptr, size_t n){
	const unsigned char* a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++){
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

typedef struct {
	Framebuffer* framebuffer;
	PSF1_FONT* psf1_Font;
	EFI_MEMORY_DESCRIPTOR* mMap;
	UINTN mMapSize;
	UINTN mMapDescSize;
} BootInfo;

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	InitializeLib(ImageHandle, SystemTable);
	Print(L"String blah blah blah \n\r");

	EFI_FILE* Kernel = load_file(NULL, L"kernel.elf", ImageHandle, SystemTable);
	if (Kernel == NULL){
		Print(L"Could not load kernel \n\r");
	}
	else{
		Print(L"Kernel Loaded Successfully \n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}

	if (
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	)
	{
		Print(L"kernel format is bad\r\n");
	}
	else
	{
		Print(L"kernel header successfully verified\r\n");
	}

	Elf64_Phdr* phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for (
		Elf64_Phdr* phdr = phdrs;
		(char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	)
	{
		switch (phdr->p_type){
			case PT_LOAD:
			{
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void*)segment);
				break;
			}
		}
	}

	Print(L"Kernel Loaded\n\r");
	

	PSF1_FONT* newFont = load_psf1_font(NULL, L"zap-light16.psf", ImageHandle, SystemTable);
	if (newFont == NULL){
		Print(L"Font is not valid or is not found\n\r");
	}
	else
	{
		Print(L"Font found. char size = %d\n\r", newFont->psf1_header->charsize);
	}
	

	Framebuffer* newBuffer = initialize_gop();

	Print(L"Base: 0x%x\n\rSize: 0x%x\n\rWidth: %d\n\rHeight: %d\n\rPixelsPerScanline: %d\n\r", 
	newBuffer->base_address, 
	newBuffer->buffer_size, 
	newBuffer->width, 
	newBuffer->height, 
	newBuffer->pixels_per_scanline);

	EFI_MEMORY_DESCRIPTOR* Map = NULL;
	UINTN MapSize, MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
	{
		
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void**)&Map);
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);

	}

	void (*KernelStart)(BootInfo*) = ((__attribute__((sysv_abi)) void (*)(BootInfo*) ) header.e_entry);

	BootInfo bootInfo;
	bootInfo.framebuffer = newBuffer;
	bootInfo.psf1_Font = newFont;
	bootInfo.mMap = Map;
	bootInfo.mMapSize = MapSize;
	bootInfo.mMapDescSize = DescriptorSize;

	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

	KernelStart(&bootInfo);

	return EFI_SUCCESS; // Exit the UEFI application
}
