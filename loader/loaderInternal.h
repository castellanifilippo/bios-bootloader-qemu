#include "../utilities/utilities.h"

#ifndef LOADER_INTERNAL_H
#define LOADER_INTERNAL_H

// 440 bytes: bootloader code
#define MBR_BOOT_OFFSET 0x00
// 4 bytes: unique disk signature
#define MBR_UNIQUE_DISK_SIGNATURE_OFFSET 0x1B8
// 16 bytes for entry: partition table
#define MBR_PARTITION_TABLE_OFFSET 0x1BE
#define MBR_PARTITION_ENTRY_SIZE 16
// 2 bytes: boot signature
#define MBR_SIGNATURE_OFFSET 0x1FE

// Magic number to identify a valid MBR
#define MBR_SIGNATURE 0xAA55

// Bootable partition
#define MBR_BOOTABLE_PARTITION 0x80

// Partition table entry offsets
#define MBR_PARTITION_STATUS_OFFSET 0x00
#define MBR_PARTITION_CHS_OFFSET 0x01
#define MBR_PARTITION_TYPE_OFFSET 0x04
#define MBR_PARTITION_CHS_END_OFFSET 0x05
#define MBR_PARTITION_LBA_OFFSET 0x08
#define MBR_PARTITION_SECTORS_OFFSET 0x0C

// Bootloader disk-memory address
#define MBR_GAP_START_LBA 1
#define MBR_GAP_END_LBA 2047
#define LOADER_BOOTLOADER_ADDRESS 0x100000

// Multiboot information structure address
#define LOADER_MULTIBOOT_INFO_ADDRESS 0x92000
#define LOADER_MULTIBOOT_MMAP_ADDRESS 0x93000

extern void bootloader_jump(uint32_t magic, uint32_t info_addr, uint32_t jump_addr);

#endif