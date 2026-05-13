/* Documentation 
https://wiki.osdev.org/Multiboot
https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
*/

#include "../vga/vga.h"
#include "../utilities/utilities.h"
#include "bootPrint.h"
#include "mboot.h"

void main_boot(uint32_t eax, uint32_t ebx){
    clearScreen();
    bootloaderPrint();
    appendl("\n",NORMAL);

    char string[12];

    appendl("\xFE Multiboot: ",NORMAL);

    // Check if we were booted by a Multiboot-compliant bootloader
    if (eax != MULTIBOOT_BOOTLOADER_MAGIC) {
        appendl(" \x1A Not booted by a Multiboot-compliant bootloader",ERROR);
        return;
    }else{
        append("OK",SUCCESS);
    }

    // Get the address of the Multiboot information structure
    multiboot_info_t* info = (multiboot_info_t*)ebx;
    appendl(" \x1A Information structure address: ",NORMAL);
    appendInt((uint32_t)info, string, 16, NUMBER);

    // Get the boot device
    if(info->flags & MULTIBOOT_INFO_BOOTDEV){
        uint32_t boot_device = ((info->boot_device >> 24) - 0x80) & 0xFF;
        appendl(" \x1A Boot device: ",NORMAL);
        appendInt(boot_device, string, 16, NUMBER);
    }

    // Get memory information
    if (info->flags & MULTIBOOT_INFO_MEMORY) {
        appendl(" \x1A Memory information:",NORMAL);
        appendl("    Lower memory: ",NORMAL);
        appendInt(info->mem_lower, string, 10, NUMBER);
        append(" KB",NORMAL);
        appendl("    Upper memory: ",NORMAL);
        appendInt(info->mem_upper, string, 10, NUMBER);
        append(" KB\n",NORMAL);

    } else {
        appendl(" \x1A No memory information provided",ERROR);
    }

}