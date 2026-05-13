#ifndef VGAIO
#define VGAIO
#include "../utilities/utilities.h"

/* ************ *
 * VGA REGISTER * 
 * ************ */

/*======= General Regisers =======*/
#define MOR 0x3C2
#define FC 0x3DA
#define IS1 FC

/*======= Sequencer Registers =======*/
#define SR_INDEX_REG 0x3C4
#define SR_DATA_REG 0x3C5

#define NSR 5

#define CLOCKING_MODE_INDEX 1
#define MAP_MASK_INDEX 0x02 
#define MEMORY_MODE_INDEX 0x04

/*======= CRTC Registers ========*/
#define CRTC_INDEX_REG 0x3D4
#define CRTC_DATA_REG 0x3D5

#define NCRTC 25

#define EHBR_INDEX 0x03 /*End Horizontal Blanking Register*/
#define LPHR_INDEX 0x11 /*Light Pen High register*/

/*======= Graphics Controller Registers =======*/
#define GC_INDEX_REG 0x3CE
#define GC_DATA_REG 0x3CF

#define NGCR 9

#define READ_MAP_SELECT_INDEX  0x04
#define MODE_REGISTER_INDEX 0x05
#define MISCELLANEOUS_INDEX 0x06

/*======= Attribute Controller Registers =======*/
#define ACR_WRITE_REG  0x3C0
#define ACR_READ_REG   0x3C1

#define NACR 21

/*======= DAC Registers=======*/
#define DAC_INDEX_WRITE_REG 0x3C8
#define DAC_DATA_REG 0x3C9
#define DAC_INDEX_READ_REG 0x3C7
#define DAC_MASK_REG 0x3c6

/* ************* *
 *     VGA IO    * 
 * ************* */

#define CLEAR_FLIPFLOP() \
  inb(IS1)

/*write the value in the register indexed by AddrReg (for CRTC GCR SR)*/
static inline void outIndexDataReg(uint8_t value, uint8_t index, uint16_t AddrReg){
  outw( (value << 8) | index, AddrReg);
}

/*read the register indexed by AddrReg (for CRTC GCR SR)*/
static inline uint8_t inIndexDataReg(uint8_t index, uint16_t AddrReg){
  outb(index,AddrReg);
  return inb(AddrReg+1);
}

/*for CRTC GCR SR*/
static inline void maskIndexDataReg(uint8_t set,uint8_t reset, uint8_t index ,uint16_t AddrReg){
  outb( (inIndexDataReg(index, AddrReg) & ~reset ) | set , AddrReg+1 );
}

/*Read ACR, does reset PAS bit in the read, unless in index PAS bit is enabled*/
static inline uint8_t readACR(uint8_t index){
  CLEAR_FLIPFLOP();
  outb(index, ACR_WRITE_REG);
  return inb(ACR_READ_REG);
}

/*write ACR, does reset PAS bit in the write, unless in index PAS bit is enabled*/
static inline void writeACR(uint8_t value, uint8_t index){
  CLEAR_FLIPFLOP();
  outb(index, ACR_WRITE_REG);
  outb(value, ACR_WRITE_REG);
}

#endif
