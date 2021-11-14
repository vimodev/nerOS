qemu:
	(cd gnu-efi && make bootloader)
	(cd kernel && make qemu)