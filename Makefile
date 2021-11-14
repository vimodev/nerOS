qemu:
	(cd gnu-efi && make bootloader)
	(cd kernel && make qemu)

all:
	(cd gnu-efi && make bootloader)
	(cd kernel && make kernel)