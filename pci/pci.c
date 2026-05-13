#include "pci.h"
#include "../utilities/utilities.h"/*stdint,bool,video ...*/
#include "conf.h" /*PCI_IO_START PCI_MM_START*/
#include "pciIO.h" /*read/write in conf space*/
#include "bar.h" /*Bar struct and relative uitilities*/
#include "pciInternal.h" /* MACRO*/

/* ********************* *
 *  DISCOVERY & PROBING  *
 * ********************* */

uint32_t * const nMBars= (uint32_t *) SIZE_MM_ADDRESS;
Bar * const MBars= (Bar *) MMADDRESS;

uint32_t * const nIOBars= (uint32_t *)SIZE_IO_ADDRESS;
Bar * const IOBars= (Bar *) IOADDRESS;

static void probeFunction(uint8_t bus,uint16_t device, uint16_t func){
  Bar newBar;
  char buffer[10];
  uint8_t nbar;
  uint32_t original=0,probedBar=0;
  append(" Bar:",NORMAL);

  for(nbar=0; nbar<NBAR; nbar++){
    original=readConfSpace(bus,device,func,BAR_TO_OFFSET(nbar));
    writeConfSpace(bus,device,func,BAR_TO_OFFSET(nbar) ,PROBE);
    probedBar=readConfSpace(bus,device,func,BAR_TO_OFFSET(nbar));
    writeConfSpace(bus,device,func,BAR_TO_OFFSET(nbar),original);

    /*the BAR exist?*/
    if(probedBar == PROBE || !probedBar) 
      continue;

    barInit(&newBar);
    newBar.io=(probedBar & 0x1);

    /*is a shadow BAR?*/
    if(newBar.io && !(probedBar & IO_BAR_MASK))
      continue;
    else
      if(!(probedBar & MM_BAR_MASK_32))
         continue;

    append(" ",NORMAL);
    appendInt(nbar,buffer,10,NUMBER);

    newBar.dev=device;
    newBar.func=func;
    newBar.bus=bus;
    newBar.nBar=nbar;

    if(!newBar.io && (((probedBar) & TYPE_BAR_MASK) == BAR64BIT_BIT)){
      /*64 bit MM BAR*/
      newBar.bit64=true;
      uint32_t originalHigh=readConfSpace(bus,device,func,BAR_TO_OFFSET(nbar+1));
      writeConfSpace(bus,device,func,BAR_TO_OFFSET(nbar+1),PROBE);
      uint32_t probedBarHigh=readConfSpace(bus,device,func,BAR_TO_OFFSET(nbar+1));
      writeConfSpace(bus,device,func,BAR_TO_OFFSET(nbar+1),originalHigh);

      uint64_t probedBar64=(((uint64_t)probedBarHigh << 32) | probedBar);
      newBar.size= ~(probedBar64 & MM_BAR_MASK_64)+1;

      nbar++;
      MBars[*nMBars]=newBar;
      *nMBars+=1;
      append("M64",NORMAL);
    }else{
      if(newBar.io){
        /*IO BAR*/
        newBar.size= ~(probedBar & IO_BAR_MASK)+1;
        IOBars[*nIOBars]=newBar;
        *nIOBars+=1;
        append("IO",NORMAL);
      }else{
        /*32 bit MM BAR*/
        newBar.size= ~(probedBar & MM_BAR_MASK_32)+1;
        MBars[*nMBars]=newBar;
        *nMBars+=1;
        append("M32",NORMAL);
      }
      appendInt(newBar.size,buffer,16,NUMBER);
    }
  }
#ifdef MAP_EXP_ROM
  /*EXPANSION ROM*/
  nbar=8;
  original=readConfSpace(bus,device,func,BAR_TO_OFFSET(nbar));
  writeConfSpace(bus,device,func,BAR_TO_OFFSET(nbar) ,PROBE);
  probedBar=readConfSpace(bus,device,func,BAR_TO_OFFSET(nbar));
  writeConfSpace(bus,device,func,BAR_TO_OFFSET(nbar),original);
  if(probedBar == PROBE || !probedBar /* the BAR exist?*/
    || !(probedBar & EXP_ROM_BAR_MASK))/* is a shadow BAR?*/
    return;

  barInit(&newBar);
  newBar.rom=true;
  newBar.dev=device;
  newBar.func=func;
  newBar.bus=bus;
  newBar.nBar=nbar;
  newBar.size= ~(probedBar & EXP_ROM_BAR_MASK) + 1;
  MBars[*nMBars]=newBar;
  *nMBars+=1;
  append(" ExpROM",NORMAL);
  appendInt(newBar.size,buffer,16,NUMBER);
#endif
}

static void probeBus(uint8_t bus){
  char buffer[10];
  appendl("   BUS: ",NORMAL);
  appendInt(bus,buffer,10,NUMBER);

  for(uint16_t device=0; device<NDEVICES; device++){
    bool multifunction=false;
    /* device exist?*/
    uint16_t vendorID=(readConfSpace(bus,device,0,0) & VENDORID_MASK);
    if( vendorID == NO_FUNCTION)
      continue;

    /* is Multi-Function?*/
    uint8_t header = ((readConfSpace(bus,device,0,HEADER_OFFSET) >> 16) & 0xFF);
    multifunction= header & MULTIFUNCTION_BIT;

    uint16_t function=0;
    do{
      /* function exist?*/
      uint32_t id=readConfSpace(bus,device,function,0);
      if((id & VENDORID_MASK) == NO_FUNCTION)
        continue;
      /* vendorID:deviceID*/
      appendl("    ",NORMAL);
      appendInt(id & VENDORID_MASK,buffer,16,NUMBER);
      append(":",NORMAL);
      appendInt(id>>16,buffer,16,NUMBER);

      append(" Dev: ",NORMAL);
      appendInt(device,buffer,10,NUMBER);
      append(" Func: ",NORMAL);
      appendInt(function,buffer,10,NUMBER);

     /* device with special needs*/
      uint32_t classRow=readConfSpace(bus,device,function,CLASS_CODE_OFFSET);
      uint8_t classCode=(classRow &CLASS_CODE_MASK)>>24;
      uint8_t subclass=(classRow &SUBCLASS_MASK)>>16;

      if(classCode==BRIDGE && subclass==HOST_BRIDGE){
        /*host bridge dosen't need to be handle*/
        append(" host bridge",NORMAL);
        continue;
      }

      /* is a normal function?*/
      uint8_t headerNoMultifunctionBit = ((readConfSpace(bus,device,function,HEADER_OFFSET) >> 16) & (0xFF & ~MULTIFUNCTION_BIT));
      if(headerNoMultifunctionBit == NORMAL_DEVICE){
        probeFunction(bus,device,function);
      }/*else{
        * if we want ot handle bridge
        * add here the code needed
        * }*/
      
      /*write the APIC PIN in the INTR LINE*/
      uint32_t intrPinLine=readConfSpace(bus,device,function,INTR_PIN_LINE_OFFSET);
      uint8_t intrPin=(intrPinLine&INTR_PIN_MASK)>>8;
      if(intrPin!=NO_INT){
        intrPinLine&= ~INTR_LINE_MASK;
        intrPinLine|=INTX_TO_APIC_PIN(intrPin,device);
        writeConfSpace(bus,device,function,INTR_PIN_LINE_OFFSET,intrPinLine);
      }
    }while(multifunction && ++function<NFUNCTIONS);
  }
}

/* ****************** *
 *  ASSIGN ADDRESSES  *
 * ****************** */
static void assignList(Bar * toAssign, uint32_t nBars, uint64_t addr,const uint64_t lastAddr){
  char buffer[10];
  const uint64_t startAddr=addr;
  uint32_t origHigh=0;
  
  for (uint32_t i = 0; i < nBars; i++){
    uint64_t mask = toAssign->size - 1;

    if (addr & mask){
      uint64_t padding = toAssign->size - (addr & mask);
      appendl("     Ops! There is a hole in pci space at: ",ERROR);
      appendInt(addr,buffer,16,ERROR);
      appendl(" of: ",ERROR);
      appendInt(padding,buffer,16,ERROR);
      append("B",ERROR);

      addr = (addr + mask) & ~mask;
    }

    appendl("    Dev: ",NORMAL);
    appendInt(toAssign->dev,buffer,10,NUMBER);
    append(" Func: ",NORMAL);
    appendInt(toAssign->func,buffer,10,NUMBER);
    append(" BAR: ", NORMAL);
    appendInt(toAssign->nBar,buffer,10,NUMBER);
    
    uint32_t orig=readConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar));
    writeConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar),(uint32_t)((toAssign->rom)? addr|ENABLE_ROM_BIT :addr));//if is rom -> enable rom
    if(toAssign->bit64){
      /* 64bit BAR */
      append(" M64", NORMAL);
      origHigh=readConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar+1));
      writeConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar+1), (uint32_t)(addr >> 32));
    }else
      if(toAssign->rom)
        append(" ExpROM",NORMAL);
      else
        if(toAssign->io)
          append(" io",NORMAL);
        else
          append(" M32",NORMAL);
    
    /* Readback*/
    if(/*ExpRom*/   (toAssign->rom && (uint32_t)addr != (readConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar)) & EXP_ROM_BAR_MASK)) ||
      /*64 bit BAR*/(toAssign->bit64 && (uint32_t)(addr>>32) != (readConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar+1)))) ||
      /*io BAR*/    (!toAssign->rom && toAssign->io && (uint32_t)addr != (readConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar)) & IO_BAR_MASK)) ||
      /*32 bit BAR*/(!toAssign->rom && !toAssign->io && (uint32_t)addr != (readConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar)) & MM_BAR_MASK_32)) ){

      /* readback has failed*/
      writeConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar),orig);
      if(toAssign->bit64)
        writeConfSpace(toAssign->bus,toAssign->dev,toAssign->func,BAR_TO_OFFSET(toAssign->nBar+1),origHigh);
      
      append("BAR NOT MAPPED, error on write",ERROR);
    }
    /* MM space of a device that failed the readback is wasted for safety*/
    append(" Addr: ",NORMAL);
    appendInt(addr,buffer,16,NUMBER);
    append(" Size: ",NORMAL);
    appendInt(toAssign->size,buffer,16,NUMBER);

    addr+=toAssign->size;
    if(addr>=lastAddr || addr<startAddr /*overflow*/){
      appendl("    PCI MEM terminate!",ERROR);
      append(" Addr: ",ERROR);
      appendInt(addr,buffer,16,ERROR);
      return;
    }
    
    /*COMAND*/
    orig=readConfSpace(toAssign->bus, toAssign->dev, toAssign->func, COMMAND_OFFSET);
    orig |= ENABLE_BUS_MASTER_BIT;
    if (toAssign->io)
      orig |= ENABLE_IO_MAP_BIT;
    else
      orig |= ENABLE_MM_MAP_BIT;
    writeConfSpace(toAssign->bus, toAssign->dev, toAssign->func, COMMAND_OFFSET, orig);

    toAssign++;
  }
}

/* ****************** *
 *        INIT        *
 * ****************** */

#define BARS_ARRAY_INIT() \
  *nMBars=*nIOBars=0

void PCIinit(void){
  appendl("\xFE PCI:",NORMAL);
  
  /*====== PROBE ======*/
  appendl(" \x1A Probe device:",NORMAL);
  BARS_ARRAY_INIT();
  probeBus(0);

  /*====== SORT ======*/
  appendl(" \x1A Sort BAR",NORMAL);
  quickSortBars(MBars,0,*nMBars-1);
  quickSortBars(IOBars,0,*nIOBars-1);
  
  /*====== ASSIGN ADDRESSES ======*/
  appendl(" \x1A Assign addresses:",NORMAL);
  assignList(MBars, *nMBars, PCI_MM_START, PCI_MM_END);
  assignList(IOBars, *nIOBars, PCI_IO_START, PCI_IO_END);
}
