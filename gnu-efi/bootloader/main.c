#include <efi.h>
#include <efilib.h>
#include <elf.h>

typedef unsigned long long size_t;

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
	EFI_FILE *Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
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

	// Call Kernel entry _start
	int (*KernelStart)() = ((__attribute__((sysv_abi)) int (*)() ) header.e_entry);
	Print(L"%d\n\r", KernelStart());

	return EFI_SUCCESS; // Exit the UEFI application
}
