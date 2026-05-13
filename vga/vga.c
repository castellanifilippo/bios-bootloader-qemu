#include "vga.h"
#include "dataVga.h" /*value for registers, font & palette*/
#include "vgaIO.h" /*To read/write easly vga register*/
#include "../utilities/utilities.h" /*memcpy & in/out function*/

/* ********************** *
 *    STATIC UTILITIES    *
 * ********************** */

/*
 * true -> Ram Access palette
 * flase -> Display Access palette
 */
static void accessPalette(bool enable){
  CLEAR_FLIPFLOP();
  if(enable){
    outb(inb(ACR_READ_REG) & 0x20, ACR_WRITE_REG);/*0-> RAM*/
  }else{
    outb(inb(ACR_READ_REG) | 0x20, ACR_WRITE_REG);/*1-> Display*/
  }
}

/*Enable/Disable crtc write protection on 0-7 registers*/
static void CRTCProtection(bool enable){
  if(!enable){
    maskIndexDataReg(0x80,0,EHBR_INDEX,CRTC_INDEX_REG);
    maskIndexDataReg(0,0x80,LPHR_INDEX,CRTC_INDEX_REG);
  }else{
    maskIndexDataReg(0x80,0,EHBR_INDEX,CRTC_INDEX_REG);
    maskIndexDataReg(0x80,0,LPHR_INDEX,CRTC_INDEX_REG);
    maskIndexDataReg(0,0x80,EHBR_INDEX,CRTC_INDEX_REG);
  }
}

static void setPlane(uint8_t p){
  uint8_t mask;
  p&=3;
  mask= 1 << p;
  outIndexDataReg(p,READ_MAP_SELECT_INDEX,GC_INDEX_REG); /*Read plane*/
  outIndexDataReg(mask,MAP_MASK_INDEX,SR_INDEX_REG); /*Write plane*/
}

/* ********************** *
 *   PUBLIC  UTILITIES    *
 * ********************** */

/*Set screen on/off*/
void screen(bool enable){
  if(enable)
    maskIndexDataReg(0,0x20,CLOCKING_MODE_INDEX,SR_INDEX_REG);/* 0 -> Screen on*/
  else
    maskIndexDataReg(0x20,0,CLOCKING_MODE_INDEX,SR_INDEX_REG);
}

/* ******************* *
 *       VGA INIT      * 
 * ******************* */

static void writeFont(void){
  uint8_t sr2, sr4, gc4, gc5, gc6;
  uint8_t *src=font8x16;
  uint32_t i;

  /*save ragister modified by setPlane() and disable odd/even and chain four */
  
  sr2=inIndexDataReg(MAP_MASK_INDEX,SR_INDEX_REG);
 
  sr4=inIndexDataReg(MEMORY_MODE_INDEX,SR_INDEX_REG);
  maskIndexDataReg(0x04,0,MEMORY_MODE_INDEX,SR_INDEX_REG);/* 1 -> chain four, to disable it*/
  
  gc4=inIndexDataReg(READ_MAP_SELECT_INDEX,GC_INDEX_REG);

  gc5=inIndexDataReg(MODE_REGISTER_INDEX,GC_INDEX_REG);
  maskIndexDataReg(0,0x10,MODE_REGISTER_INDEX,GC_INDEX_REG);/* 0 -> odd/even register */

  gc6=inIndexDataReg(MISCELLANEOUS_INDEX,GC_INDEX_REG);
  maskIndexDataReg(0,0x02,MISCELLANEOUS_INDEX,GC_INDEX_REG);/* 0 -> Chain Odd-even register */

  setPlane(2);

  /*fonts are 16*8 bit but vga reserve a 32*8 space for each font charchter*/
  for(i = 0; i < 256; i++){
    memcpy((uint8_t *)(VIDEOMEM + (i * 32)), src, FONTHEIGHT);
		src += FONTHEIGHT;
	}

  /*Restore registers*/
  outb(MAP_MASK_INDEX, SR_INDEX_REG);
  outb(sr2, SR_DATA_REG);
  outb(MEMORY_MODE_INDEX, SR_INDEX_REG);
  outb(sr4, SR_DATA_REG);
  outb(READ_MAP_SELECT_INDEX, GC_INDEX_REG);
  outb(gc4, GC_DATA_REG);
	outb(MODE_REGISTER_INDEX, GC_INDEX_REG);
	outb(gc5, GC_DATA_REG);
	outb(MISCELLANEOUS_INDEX, GC_INDEX_REG);
	outb(gc6, GC_DATA_REG);
}

void vgaInit(void){
  uint32_t i;
  uint8_t *regs=textMode80x25;

  /*======= LOAD PALETTE =========*/
  outb(0xff,DAC_MASK_REG);
  outb(0,DAC_INDEX_WRITE_REG);
  for(i=0; i<NDATAPALETTE;i++)
    outb(palette[i],DAC_DATA_REG);

  /*======== DISABLE DISAPLAY ========*/
  screen(false);

  /*======== LOAD RREGISTERS ========*/
  
  /*Attribute Controller Registers*/
  for(i=0;i<NACR;i++)
    writeACR(*(regs++),i);
  
  /*Sequencer registers*/
  for(i=0;i<NSR;i++)
    outIndexDataReg(*(regs++),i,SR_INDEX_REG);

  /*Graphics Controller Registers*/
  for(i=0;i<NGCR;i++)
    outIndexDataReg(*(regs++),i,GC_INDEX_REG);

  /*General Regiser*/
  outb(*(regs++),MOR);

  /*Disable crtc protection*/
  CRTCProtection(false);
  regs[EHBR_INDEX]|= 0x80;
  regs[LPHR_INDEX]&= ~0x80;

  /*CRTC register*/
  for(i=0;i<NCRTC;i++)
    outIndexDataReg(*(regs++),i,CRTC_INDEX_REG);

  /*Lock 16-color palette and unblank display*/
  accessPalette(false);

  /*======== CLEAR SCREEN ========*/
  clearScreen();

  /*======== LOAD FONTS ========*/
  writeFont();

  /*======== EANBLE DSIPLAY & LOCK CRTC========*/
  screen(true);
  CRTCProtection(true);
}





