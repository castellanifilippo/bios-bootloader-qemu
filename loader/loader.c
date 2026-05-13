#include "loader.h"
#include "loaderInternal.h"
#include "../ata/ata.h"
#include "../bootloader/mboot.h"
#include "../keyboard/keyboard.h"
#include "../utilities/utilities.h"
#include "../pci/conf.h"

multiboot_info_t* multiboot_info = (multiboot_info_t*)LOADER_MULTIBOOT_INFO_ADDRESS;
multiboot_memory_map_t* mmap_entry = (multiboot_memory_map_t*)LOADER_MULTIBOOT_MMAP_ADDRESS;

// We store the bootloader in the MBR GAP (LBA 1-2047)
// We search for a valid MBR with one bootable partition
// Assuming there is a bootloader in the MBR GAP

// Function that searches for the MBR on the ATA devices
// We stop at the first device that has a valid MBR with a bootable partition
// Use ata_devices array for valid disks
static uint8_t loader_search_mbr(void){
    // Buffer to store the MBR data (1 sector)
    uint16_t mbr[256];
    bool bootable_partition = false;
    uint8_t i;

    for(i = 0; i < 4; i++){
        if(ata_devices[i].Present){
            ata_read_sectors(ata_devices[i].Drive, ata_devices[i].Channel, 0, 1, mbr);

            // Divide offset by 2, mbr is an array of uint16_t
            uint16_t signature = mbr[MBR_SIGNATURE_OFFSET / 2];
            if(signature != MBR_SIGNATURE) continue;
            
            appendl(" \x1A Valid MBR found on device:\n ",NORMAL);
            char buffer[12];
            append("   Channel: ",NORMAL);
            appendInt(ata_devices[i].Channel, buffer, 10, NUMBER);
            append(" Drive: ",NORMAL);
            appendInt(ata_devices[i].Drive, buffer, 10, NUMBER);

            // Check for a bootable partition (status 0x80)
            for(uint32_t j = 0; j < 4; j++){
                uint16_t entry = MBR_PARTITION_TABLE_OFFSET / 2 + j * MBR_PARTITION_ENTRY_SIZE / 2;
                uint8_t status = mbr[entry + MBR_PARTITION_STATUS_OFFSET / 2] & 0xFF;
                if(status == MBR_BOOTABLE_PARTITION){
                    bootable_partition = true;
                    break;
                }
            }

            if(bootable_partition){
                appendl("    Bootable partition: ",NORMAL);
                append("OK",SUCCESS);
                break;
            }
        }
        
    }
    // Exit if there is no valid MBR with a bootable partition
    if(!bootable_partition){
        appendl(" \x1A No valid MBR found",ERROR);
        return LOADER_ERROR;
    }

    // Return the index of the device with the valid MBR
    return i;
}

// Function to load the bootloader from the MBR GAP
// device_index: index of the ATA device with the valid bootable MBR partition
// buffer: load address for the bootloader
// sector_count: number of sectors to read
// read_sectors: pointer to the number of sectors read so far, updated by the function
static uint8_t loader_load_bootloader(uint8_t device_index, uint16_t* buffer, uint32_t sector_count, uint32_t* read_sectors){
    uint32_t to_read;
    uint32_t current_lba = MBR_GAP_START_LBA + *read_sectors;

    while (sector_count > 0) {
        to_read = sector_count;
        // Max sectors per ATA command (0 means 256)
        if (to_read > 256) {
            to_read = 0;
            sector_count -= 256;
        }else{
            sector_count = 0;
        }

        ata_read_sectors(ata_devices[device_index].Drive, ata_devices[device_index].Channel, current_lba, to_read, buffer);

        // We change back to 256 if to_read is 0 to update variables
        if(to_read == 0) to_read = 256;

        // Update buffer and current LBA for the next read
        current_lba += to_read;
        buffer += (to_read * 256);
        *read_sectors += to_read;
    }
    return LOADER_SUCCESS;
}

// Function to read the multiboot header from the loaded bootloader
// multiboot_header: multiboot header structure to fill
static uint8_t loader_read_multiboot_header(multiboot_header_t* multiboot_header){
    // We search the multiboot header in the loaded bootloader
    uint32_t* header = NULL;   
    uint32_t limit = (MBR_GAP_END_LBA - MBR_GAP_START_LBA + 1) * 512;
    
    // From specification, the multiboot header must be in the first 8KB of the bootloader
    for(uint32_t offset = 0; offset < limit && offset < MULTIBOOT_SEARCH; offset+=4){
        header = (uint32_t*)(LOADER_BOOTLOADER_ADDRESS + offset);
        // Check for the multiboot magic number and the checksum
        if(*header == MULTIBOOT_HEADER_MAGIC && header[1] + header[2] + header[0] == 0){
            appendl(" \x1A Multiboot header found in bootloader",NORMAL);
            break;
        }else if(*header == MULTIBOOT_HEADER_MAGIC){
            appendl(" \x1A Multiboot header found but checksum is invalid",ERROR);
            return LOADER_ERROR;
        }
    }
    if(*header != MULTIBOOT_HEADER_MAGIC){
        appendl(" \x1A Multiboot header not found in bootloader",ERROR);
        return LOADER_ERROR;
    }

    // Map the multiboot header fields to the structure
    multiboot_header->magic = header[0];
    multiboot_header->flags = header[1];
    multiboot_header->checksum = header[2];
    
    // If the AOUT_KLUDGE flag is set, we need to add the additional fields
    // This is the case we support
    if(multiboot_header->flags & MULTIBOOT_AOUT_KLUDGE){
        multiboot_header->header_addr = header[3];
        multiboot_header->load_addr = header[4];
        multiboot_header->load_end_addr = header[5];
        multiboot_header->bss_end_addr = header[6];
        multiboot_header->entry_addr = header[7];
    }else{
        appendl(" \x1A Multiboot header does not have AOUT_KLUDGE flag set",ERROR);
        return LOADER_ERROR;
    }

    // If the VIDEO_MODE flag is set, we need to add the additional fields
    if(multiboot_header->flags & MULTIBOOT_VIDEO_MODE){
        multiboot_header->mode_type = header[8];
        multiboot_header->width = header[9];
        multiboot_header->height = header[10];
        multiboot_header->depth = header[11];
    }

    return LOADER_SUCCESS;
}

// Function to preapare the multiboot information structure
// device_index: index of the ATA device with the valid bootable MBR partition
static uint8_t loader_prepare_info(uint8_t device_index){
    // Reset memory for the multiboot info structure
    memset((void*)LOADER_MULTIBOOT_INFO_ADDRESS, 0, sizeof(multiboot_info_t));

    // Since we are in a qemu emulator, we can get the RAM size from it
    // In order to support real hardware we should change method (like E820 ...)
    uint64_t ram_size = get_qemu_ram_size();
    
    multiboot_info->flags = MULTIBOOT_INFO_BOOTDEV | MULTIBOOT_INFO_MEM_MAP | MULTIBOOT_INFO_MEMORY;
    // 0x80 for the first ATA device, 0x81 for the second, etc.
    uint8_t bios_drive = 0x80 + device_index;
    // bios_drive is in the most significant byte of boot_device, the rest is for the partition number
    // 0x00FFFFFF since we don't support partitions
    multiboot_info->boot_device = (bios_drive << 24) | 0x00FFFFFF;
    
    // We prepare the memory information with the available memory regions
    multiboot_info->mem_lower = 640; // 640 KB of conventional memory
    multiboot_info->mem_upper = ram_size / 1024 - 1024; // RAM size in KB over 1 MB

    // We prepare the memory map with the available memory regions
    
    // Free Memory: 0 - 640 KB
    mmap_entry[0].size = sizeof(multiboot_memory_map_t) - sizeof(uint32_t);
    mmap_entry[0].addr = 0;
    mmap_entry[0].len = 0XA0000;
    mmap_entry[0].type = MULTIBOOT_MEMORY_AVAILABLE;
    multiboot_info->mmap_length = sizeof(multiboot_memory_map_t);

    // Reserved Memory: 640 KB - 1 MB
    mmap_entry[1].size = sizeof(multiboot_memory_map_t) - sizeof(uint32_t);
    mmap_entry[1].addr = 0xA0000;
    mmap_entry[1].len = 0x60000;
    mmap_entry[1].type = MULTIBOOT_MEMORY_RESERVED;
    multiboot_info->mmap_length += sizeof(multiboot_memory_map_t);
    
    // Free Memory: 1 MB +
    mmap_entry[2].size = sizeof(multiboot_memory_map_t) - sizeof(uint32_t);
    mmap_entry[2].addr = 0x100000;
    mmap_entry[2].len = ram_size - 0x100000;
    mmap_entry[2].type = MULTIBOOT_MEMORY_AVAILABLE;
    multiboot_info->mmap_length += sizeof(multiboot_memory_map_t);

    // If we have 3GB or more of RAM, we need to add a reserved memory region for the PCI MMIO space
    if(ram_size >= 3221225472){ // 3 GB
        mmap_entry[3].size = sizeof(multiboot_memory_map_t) - sizeof(uint32_t);
        mmap_entry[3].addr = PCI_MM_START;
        mmap_entry[3].len = PCI_MM_END - PCI_MM_START + 1;
        mmap_entry[3].type = MULTIBOOT_MEMORY_RESERVED;
        multiboot_info->mmap_length += sizeof(multiboot_memory_map_t);
    }

    multiboot_info->mmap_addr = LOADER_MULTIBOOT_MMAP_ADDRESS;

    return LOADER_SUCCESS;
}

// Main function to initialize the loader, search for the bootloader, 
// load it and jump to its entry point
uint8_t loader_init(void){
    multiboot_header_t multiboot_header;

    // Keep track of how many sectors whe have read
    uint32_t read_sectors = 0;

    appendl("\xFE LOADER:",NORMAL);

    uint8_t device_index = loader_search_mbr();
    if(device_index == LOADER_ERROR){
        // No valid MBR found
        return LOADER_ERROR; 
    }
    
    // We calculate how many sectors to read based on the gap size and the search size for the multiboot header
    uint32_t to_read = (MBR_GAP_END_LBA - MBR_GAP_START_LBA + 1) > (MULTIBOOT_SEARCH / 512)
        ? MULTIBOOT_SEARCH / 512 : (MBR_GAP_END_LBA - MBR_GAP_START_LBA + 1);

    
    // We load at 0x100000 the first part of the bootloader, where we expect to find the multiboot header
    loader_load_bootloader(device_index, (uint16_t*)LOADER_BOOTLOADER_ADDRESS, to_read, &read_sectors);



    // We read the multiboot header from the loaded bootloader
    uint8_t status = loader_read_multiboot_header(&multiboot_header);
    if(status != 0){
        // Multiboot header not found
        return LOADER_ERROR;
    }

    // If the multiboot header is valid, we need to load all the bootloader
    // We copy the first part of the bootloader that we already loaded to the final load address specified in the multiboot header
    memcpy((void*)multiboot_header.load_addr, (void*)LOADER_BOOTLOADER_ADDRESS, read_sectors * 512);
    
    to_read = (multiboot_header.load_end_addr - multiboot_header.load_addr -  1) / 512 + 1;
    // Check if we need to read more sectors from the bootloader
    if (to_read > read_sectors) {
        to_read -= read_sectors;
        loader_load_bootloader(device_index, (uint16_t*)(multiboot_header.load_addr + read_sectors * 512), to_read, &read_sectors);
    }

    // We set the bss section to 0
    memset((void*)multiboot_header.load_end_addr, 0, multiboot_header.bss_end_addr - multiboot_header.load_end_addr);
    
    // We print the multiboot header information
    char string[12];
    appendl("    Load address: ",NORMAL);
    appendInt(multiboot_header.load_addr, string, 16, NUMBER);
    appendl("    Entry point: ",NORMAL);
    appendInt(multiboot_header.entry_addr, string, 16, NUMBER);

    appendl("\xFE Press any key to load the bootloader...\n",NORMAL);

    // Prepare the multiboot info structure to pass to the bootloader
    loader_prepare_info(device_index);

    keyboard_wait_code();

    // We get the entry point from the multiboot header and we jump to it
    bootloader_jump( MULTIBOOT_BOOTLOADER_MAGIC, LOADER_MULTIBOOT_INFO_ADDRESS, multiboot_header.entry_addr);

    // We should never reach this point
    return LOADER_ERROR;
}