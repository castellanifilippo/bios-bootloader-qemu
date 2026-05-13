#include "ata.h"
#include "ataInternal.h"
#include "../utilities/utilities.h"

/* We use ATA LBA28 standard*/

// ATA devices informations
ata_device* ata_devices = (ata_device*) ATA_DEVICES_RAM_ADDR;

// 4 devices: primary master, primary slave, secondary master, secondary slave

// Utility function to get the ATA register address based on the channel and offset
// channel: ATA_PRIMARY or ATA_SECONDARY
// offset: register offset
static uint16_t ata_get_register(uint8_t channel, uint16_t offset){
    if(channel == ATA_PRIMARY){
        return ATA_BASE_REG_1 + offset;
    } else {
        return ATA_BASE_REG_2 + offset;
    }
}

// Function to write to the ATA control register based on the channel
// This function write drirectly on the passed register, no offset
// channel: ATA_PRIMARY or ATA_SECONDARY
// data: data to write
static void ata_write_control_register(uint8_t channel, uint8_t data) {
    if (channel == ATA_PRIMARY) {
        outb(data, ATA_REG_CONTROL_1);
    } else {
        outb(data, ATA_REG_CONTROL_2);
    }
}

// Function to read a byte from the specified ATA register
// channel: ATA_PRIMARY or ATA_SECONDARY
// offset: register offset
static uint8_t ata_read_register(uint8_t channel, uint16_t offset){
    uint16_t reg = ata_get_register(channel, offset);
    return inb(reg);
}

// Function to write a byte to the specified ATA register
// channel: ATA_PRIMARY or ATA_SECONDARY
// offset: register offset
// data: data to write
static void ata_write_register(uint8_t channel, uint16_t offset, uint8_t data){
    uint16_t reg = ata_get_register(channel, offset);
    outb(data, reg);
}

// Function to introduce a delay after certain ATA operations
// The ATA PIO documentation recomends a delay of 400ns for the drive to process the command
// Reading from the status register a few times to achieve this delay
// channel: ATA_PRIMARY or ATA_SECONDARY
static void ata_delay(uint8_t channel){
    for(uint32_t i = 0; i < 4; i++)
        ata_read_register(channel, ATA_STATUS_OFFSET);
}

// Function to poll the ATA device until it's ready for data transfer
// channel: ATA_PRIMARY or ATA_SECONDARY
static uint8_t ata_polling(uint8_t channel, bool expect_drq){
    ata_delay(channel);

    uint8_t status;
    uint32_t timeout = ATA_TIMEOUT;

    // Wait for busy flag to clear
    while (--timeout > 0) {
        status = ata_read_register(channel, ATA_STATUS_OFFSET);
        
        if (status & ATA_STATUS_BSY) continue;
        
        if (status & ATA_STATUS_ERR) return ATA_RET_DEVICE_FAULT;
        if (status & ATA_STATUS_DF) return ATA_RET_ERROR;
        
        // Check for DRQ if needed
        if (!expect_drq) return ATA_RET_SUCCESS;
        if (status & ATA_STATUS_DRQ) return ATA_RET_SUCCESS;
    }

    // Timeout or DRQ not set
    return ATA_RET_TIMEOUT;
}

// Function to print ATA errors based on the error register
// channel: ATA_PRIMARY or ATA_SECONDARY
static void ata_print_error(uint8_t channel){
    uint8_t error = ata_read_register(channel, ATA_ERR_OFFSET);

    append("ATA Error: ",ERROR);

    if(error & ATA_ERR_AMNF)
        append("Address Mark Not Found\n",NORMAL);
    if(error & ATA_ERR_TK0NF)
        append("Track 0 Not Found\n",NORMAL);
    if(error & ATA_ERR_ABRT)
        append("Command Aborted\n",NORMAL);
    if(error & ATA_ERR_MCR)
        append("Media Change Request\n",NORMAL);
    if(error & ATA_ERR_IDNF)
        append("ID Not Found\n",NORMAL);
    if(error & ATA_ERR_MC)
        append("Media Changed\n",NORMAL);
    if(error & ATA_ERR_UNC)
        append("Uncorrectable Data Error\n",NORMAL);
    if(error & ATA_ERR_BBK)
        append("Bad Block Detected\n",NORMAL);
}

// Simplified function to read data from ATA device
// Used for IDENTIFY command
// buffer: an array of words
// count: the number of words to read
// channel: ATA_PRIMARY or ATA_SECONDARY
static uint8_t ata_read_data(uint8_t channel, uint16_t* buffer, uint32_t count){
    for(uint32_t i = 0; i < count; i++){
        if (ata_polling(channel, true) != ATA_RET_SUCCESS) return ATA_RET_ERROR; // Wait for data to be ready
        buffer[i] = inw(ata_get_register(channel, ATA_DATA_OFFSET));
    }
    return ATA_RET_SUCCESS;
}

// Function to write/read data to/from the ATA device using PIO mode
// drive: ATA_MASTER or ATA_SLAVE
// channel: ATA_PRIMARY or ATA_SECONDARY
// lba: Logical Block Addressing address to read/write
// sector_count: number of sectors to read/write (0-255 => 0 means 256)
// buffer: an array of words for data transfer
static uint8_t ata_access(uint8_t drive, uint8_t channel, uint32_t lba, uint8_t sector_count, uint16_t* buffer, uint8_t command){
    // Select drive and LBA mode
    // Bit 7 to select the drive, bit 6 to enable LBA mode, bit 4 to select master/slave
    uint8_t head = ATA_LBA_MODE | (drive << 4);

    // For LBA28 we pass the top 4 bits of the LBA in the head register
    head |= (lba >> 24) & 0x0F;
    ata_write_register(channel, ATA_HDDEVSEL_OFFSET, head);
    
    // Write LBA low, mid, and high bytes
    ata_write_register(channel, ATA_LBA0_OFFSET, lba & 0xFF);
    ata_write_register(channel, ATA_LBA1_OFFSET, (lba >> 8) & 0xFF);
    ata_write_register(channel, ATA_LBA2_OFFSET, (lba >> 16) & 0xFF);

    // Write sector count
    ata_write_register(channel, ATA_SECCOUNT_OFFSET, sector_count);

    // Send the command
    ata_write_register(channel, ATA_COMMAND_OFFSET, command);

    uint8_t error = 0;
    // Read/write data
    // If sector_count is 0, we need to read/write 256 sectors
    uint32_t limit = (sector_count == 0) ? 256 : sector_count;
    for(uint32_t i = 0; i < limit; i++){
        if((error = ata_polling(channel, true)) != ATA_RET_SUCCESS) {
            break;
        }
        for(uint32_t j = 0; j < 256; j++){
            // Read
            if(command == ATA_CMD_READ_PIO){
                buffer[i * 256 + j] = inw(ata_get_register(channel, ATA_DATA_OFFSET));
            // Write
            }else if(command == ATA_CMD_WRITE_PIO){
                outw(buffer[i * 256 + j], ata_get_register(channel, ATA_DATA_OFFSET));
            }  
        }
    }

    // Send cache flush command after writing
    if(!error && command == ATA_CMD_WRITE_PIO){
        ata_write_register(channel, ATA_COMMAND_OFFSET, ATA_CMD_CACHE_FLUSH);
        error = ata_polling(channel, false);
    }   
    return error;
}

// Wrapper function to read sectors from the ATA device into a buffer
// drive: ATA_MASTER or ATA_SLAVE 
// channel: ATA_PRIMARY or ATA_SECONDARY
// lba: Logical Block Addressing address to read/write
// sector_count: number of sectors to read/write (0-255 => 0 means 256)
// buffer: an array of words for data transfer
void ata_read_sectors(uint8_t drive, uint8_t channel, uint32_t lba, uint8_t sector_count, uint16_t* buffer){
    uint8_t error = ata_access(drive, channel, lba, sector_count, buffer, ATA_CMD_READ_PIO);
    if(error == ATA_RET_TIMEOUT){
        append("ATA Read Timeout\n",ERROR);
    } else if(error != ATA_RET_SUCCESS){
        ata_print_error(channel);
    }
}

// Wrapper function to write sectors from a buffer to the ATA device
// drive: ATA_MASTER or ATA_SLAVE
// channel: ATA_PRIMARY or ATA_SECONDARY
// lba: Logical Block Addressing address to read/write
// sector_count: number of sectors to read/write (0-255 => 0 means 256)
// buffer: an array of words for data transfer
void ata_write_sectors(uint8_t drive, uint8_t channel, uint32_t lba, uint8_t sector_count, uint16_t* buffer){
    uint8_t error = ata_access(drive, channel, lba, sector_count, buffer, ATA_CMD_WRITE_PIO);
    if(error == ATA_RET_TIMEOUT){
        append("ATA Write Timeout\n",ERROR);
    } else if(error != ATA_RET_SUCCESS){
        ata_print_error(channel);
    }
}

/* 
void test_ata_read() {
    uint16_t buffer[256*255];
    ata_read_sectors(0, 0, 0, 255, buffer);

    for(int j = 0; j < 255; j++){
        char str[12];
        appendInt(j, str, 10, NUMBER);
        append(": ", NORMAL);
        for(int i = 0; i < 16; i++){
            char str[12];
            appendInt(buffer[j * 256 + i], str, 16, NUMBER);
            append(" ", NORMAL);
        }
        append("\n", NORMAL);
    }
}*/

// Function to initialize ATA devices and populate the ata_devices array with detected devices
void ata_init(){
    uint16_t identify_data[256];

    appendl("\xFE ATA:",NORMAL);

    // Disable IRQs for both channels
    ata_write_control_register(ATA_PRIMARY, 0x02);
    ata_write_control_register(ATA_SECONDARY, 0x02);


    // Detect devices on both channels
    for(uint32_t channel = 0; channel < 2; channel++){
        for(uint32_t drive = 0; drive < 2; drive++){
            uint32_t count = channel * 2 + drive;
            ata_devices[count].Present = 0;

            // Select drive: bit 4 set to 0 for master and 1 for slave
            // bit 7 must be set to 1 to select the drive, and bit 6 must be set to 1 to enable LBA mode
            // We use LBA28
            ata_write_register(channel, ATA_HDDEVSEL_OFFSET, 0xE0 | (drive << 4));
            
            // Wait 400ns for the drive to process the command
            ata_delay(channel);

            // Preliminary check to see if the drive is present on the bus
            // If 0XFF or 0 there is no device
            uint8_t status = ata_read_register(channel, ATA_STATUS_OFFSET);
            if(status == 0xFF || status == 0){
                continue;
            }

            // ATA Identify command

            // Reset sector count and LBA registers to 0 before sending the command
            ata_write_register(channel, ATA_LBA0_OFFSET, 0);
            ata_write_register(channel, ATA_LBA1_OFFSET, 0);
            ata_write_register(channel, ATA_LBA2_OFFSET, 0);
            ata_write_register(channel, ATA_SECCOUNT_OFFSET, 0);    
            ata_write_register(channel, ATA_COMMAND_OFFSET, ATA_CMD_IDENTIFY);
            
            // Delay 400ns for the drive to process the command
            ata_delay(channel);

            // If Status = 0, there is no drive
            if (ata_read_register(channel, ATA_STATUS_OFFSET) == 0) continue;

            // Wait for BSY to clear
            uint32_t timeout = ATA_TIMEOUT;
            while ((status = ata_read_register(channel, ATA_STATUS_OFFSET)) & ATA_STATUS_BSY) {
                if (--timeout <= 0) {
                    break;
                }
            }

            // Check if we have a timeout
            if(timeout <= 0) continue;
            
            // Check if driver is ATA
            // For ATA devices, LBA1 and LBA2 should be 0 after the IDENTIFY command
            // If not it's an ATAPI device or no device
            uint8_t lba1 = ata_read_register(channel, ATA_LBA1_OFFSET);
            uint8_t lba2 = ata_read_register(channel, ATA_LBA2_OFFSET);
            if(lba1 != 0 || lba2 != 0) continue;

            ata_devices[count].Type = ATA_TYPE_ATA;

            bool deviceReady = false;
            bool error = false;
            
            // Wait for the device to set DRQ or an error flag
            for(uint32_t i = 0; i < ATA_TIMEOUT; i++){
                status = ata_read_register(channel, ATA_STATUS_OFFSET);

                if (status & ATA_STATUS_ERR || status & ATA_STATUS_DF) {
                    ata_print_error(channel);
                    error = true;
                    break;
                }

                // Data is ready to be read
                if(status & ATA_STATUS_DRQ){
                    deviceReady = true;
                    break;
                }
            }

            // Check for error or timeout and continue to the next device
            if(error || !deviceReady) continue;

            // Read device information
            if (ata_read_data(channel, &identify_data[0], 256) != 0) continue;
            identify_data[255] = '\0';

            
            // Store device information
            ata_devices[count].Present = 1;
            ata_devices[count].Channel = channel;
            ata_devices[count].Drive = drive;
            ata_devices[count].Signature = identify_data[ATA_IDENTIFY_DEVICETYPE_OFFSET];
            ata_devices[count].Capabilities = identify_data[ATA_IDENTIFY_CAPABILITIES_OFFSET];
            ata_devices[count].CommandSets = ((uint32_t)identify_data[ATA_IDENTIFY_COMMANDSETS_OFFSET+1] << 16) | identify_data[ATA_IDENTIFY_COMMANDSETS_OFFSET];
            ata_devices[count].Size = ((uint32_t)identify_data[ATA_IDENTIFY_SIZE_OFFSET+1] << 16) | identify_data[ATA_IDENTIFY_SIZE_OFFSET];

            // Copy model string
            // The model string is stored in the identify data as 40 bytes
            // Data are stored in big-endian format We need to swap the bytes to get the correct string
            for (uint32_t k = 0; k < 40; k += 2) {
                ata_devices[count].Model[k] = ((int8_t*)identify_data)[ATA_IDENTIFY_MODEL_OFFSET * 2 + k + 1];
                ata_devices[count].Model[k + 1] = ((int8_t*)identify_data)[ATA_IDENTIFY_MODEL_OFFSET * 2 + k];
            }
            ata_devices[count].Model[40] = '\0';
        }
    }

    // Print detected ATA devices
    for(uint32_t i = 0; i < 4; i++){
        if(ata_devices[i].Present){
            char buffer[12];
            appendl(" \x1A Detected ATA Device:\n ",NORMAL);
            append("   Channel: ",NORMAL);
            appendInt(ata_devices[i].Channel, buffer, 10, NUMBER);
            append(" Drive: ",NORMAL);
            appendInt(ata_devices[i].Drive, buffer, 10, NUMBER);
            appendl("    Model: ",NORMAL);
            append((char*)ata_devices[i].Model, NUMBER);
            appendl("    Size: ", NORMAL);
            char sizeStr[12];
            appendInt(ata_devices[i].Size * 512 / 1024 / 1024, sizeStr, 10, NUMBER);
            append(" MB", NORMAL);
        }
    }
}
    