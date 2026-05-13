#ifndef PCIIO
#define PCIIO
#include "../utilities/utilities.h"

/*===== REGISTERS ======*/
#define CAP 0xCF8
#define CDP 0xCFC

/*===== UTILITIES ======*/
void writeCAP(uint8_t bus, uint16_t deviceN, uint16_t funcN, uint8_t offset);

static inline uint32_t readConfSpace(uint8_t bus, uint16_t device, uint16_t func, uint8_t offset){
  writeCAP(bus, device, func, offset);
  return inl(CDP);
}

static inline void writeConfSpace(uint8_t bus, uint16_t device, uint16_t func, uint8_t offset, uint32_t element){
  writeCAP(bus, device, func, offset);
  outl(element,CDP);
}

#endif
