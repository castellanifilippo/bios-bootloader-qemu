#include "../utilities/utilities.h"
#include "../vga/vga.h"
#include "../pci/pci.h"
#include "biosPrint.h"
#include "../ata/ata.h"
#include "../loader/loader.h"
#include "../keyboard/keyboard.h"


void main(void){
  /*====== VGA INIT ======*/
  vgaInit();
  biosPrint();
  appendl("\xFE VGA",NORMAL);
  append(" ON",SUCCESS);


  /*====== PCI INIT ======*/
  PCIinit();
  appendl("\xFE PCI DEVICES",NORMAL);
  append(" MAPPED",SUCCESS);

  
  /*====== WAIT FOR KEY ======*/
  appendl("\xFE Press any key to continue...",NORMAL);
  keyboard_wait_code();
  clearScreen();


  /*====== ATA INIT ======*/
  biosPrint();
  ata_init();
  appendl("\xFE ATA DEVICES",NORMAL);
  append(" MAPPED",SUCCESS);
  

  /*====== LOADER ====== */
  loader_init();
  
}
