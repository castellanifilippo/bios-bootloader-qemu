#include "pciIO.h"
#include "../utilities/utilities.h"

void writeCAP(uint8_t bus, uint16_t deviceN, uint16_t funcN, uint8_t offset){
  uint32_t addr = 0x80000000; /* enable bit 31 */
  offset &= 0xFC;             /* dword alignment */
  funcN   &= 0x07;             /* 3 bits */
  deviceN &= 0x1F;             /* 5 bits */
  addr  |= ((uint32_t)bus << 16);
  addr  |= ((uint32_t)deviceN << 11);
  addr  |= ((uint32_t)funcN << 8);
  addr  |= offset;
  outl(addr, CAP);
}
