/* Documentation 
https://wiki.osdev.org/Multiboot
https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
https://wiki.osdev.org/MBR_(x86)
*/

#ifndef LOADER_H
#define LOADER_H

#include "../utilities/utilities.h"

// Return
#define LOADER_ERROR 255
#define LOADER_SUCCESS 0

typedef struct multiboot_header multiboot_header_t;

uint8_t loader_init(void);

#endif