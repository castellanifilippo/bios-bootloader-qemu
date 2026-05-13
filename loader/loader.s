.globl bootloader_jump
.type bootloader_jump, @function

# Set the magic in eax and the address of the multiboot info structure in ebx, 
# then jump to the entry point of the bootloader
# extern void bootloader_jump(uint32_t magic, uint32_t info_addr, uint32_t jump_addr)
bootloader_jump:
    push %ebp
    mov %esp, %ebp
    mov 8(%ebp), %eax  # magic
    mov 12(%ebp), %ebx # info_addr
    mov 16(%ebp), %ecx # jump_addr
    jmp *%ecx
    # We should never reach this point
    pop %ebp
    ret