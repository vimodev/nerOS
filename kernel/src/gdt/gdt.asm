[bits 64]
load_gdt:
    ; Fetch pointer to GDT from params
    lgdt [rdi]
    ; Load ax with index of kernel data segment in gdt
    mov ax, 0x10
    ; Fill data registers with index
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; Store load_gdt return address
    pop rdi
    ; Handle kernel code segment
    mov rax, 0x08
    push rax
    push rdi
    retfq
GLOBAL load_gdt