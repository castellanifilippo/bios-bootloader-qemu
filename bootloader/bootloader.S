#include "mboot.h"
#define STACK_SIZE 0x4000
#define MULTIBOOT_HEADER_FLAGS 0x00010003 
#define BOOTLOADER_LOAD_ADDR 0x00100000

.text
.code32
.align 4
# Multiboot header contains the magic number, flags, and checksum.
# Followed by the AOUT_KLUDGE fields
.global multiboot_header
multiboot_header:
    .long MULTIBOOT_HEADER_MAGIC  # Magic number
    .long MULTIBOOT_HEADER_FLAGS  # Flags
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)  # Checksum
    .long multiboot_header  # Header address
    .long BOOTLOADER_LOAD_ADDR  # Load address
    .long _edata  # End of data
    .long _end  # End of the binary
    .long .Lmultiboot_entry  # Entry point

.global _start
_start:
    jmp .Lmultiboot_entry

.Lmultiboot_entry:
    cli
    movl $stack_end, %esp  # Set up the stack pointer
    
    # Push parameters from loader
    pushl %ebx
    pushl %eax
    call main_boot

    # We should never reach this point
    hlt

.data

.bss
.align 16
    .global stack
stack:
    .space STACK_SIZE, 0
stack_end:

