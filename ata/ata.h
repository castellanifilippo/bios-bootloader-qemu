/* Documentation 
https://wiki.osdev.org/ATA_PIO_Mode
https://wiki.osdev.org/PCI_IDE_Controller 
PCI IDE Controller Specification Rev 1.0
*/

#ifndef ATA_H
#define ATA_H

#include "ataInternal.h"

// Returns
#define ATA_RET_SUCCESS 0
#define ATA_RET_ERROR 1
#define ATA_RET_DEVICE_FAULT 2
#define ATA_RET_TIMEOUT 3

// ATA DEVICE STRUCTURE
typedef struct {
   uint8_t  Present;     // 0 (empty), 1 (present)
   uint8_t  Channel;     // 0 (Primary Channel) or 1 (Secondary Channel)
   uint8_t  Drive;       // 0 (Master Drive) or 1 (Slave Drive)
   uint16_t Type;        // 0: ATA, 1:ATAPI
   uint16_t Signature;
   uint16_t Capabilities;// Features
   uint32_t CommandSets; // Command Sets Supported
   uint32_t Size;        // In Sectors
   uint8_t Model[41];
} ata_device;


extern ata_device* ata_devices;

void ata_init(void);

void ata_read_sectors(uint8_t drive, uint8_t channel, uint32_t lba, uint8_t sector_count, uint16_t* buffer);
void ata_write_sectors(uint8_t drive, uint8_t channel, uint32_t lba, uint8_t sector_count, uint16_t* buffer);

#endif