#ifndef BAR
#define BAR
#include "../utilities/utilities.h"

#define PROBE   0xFFFFFFFFU
#define NBAR    6
#define BAR0    0x10
#define BARDIM  4

#define BAR_TO_OFFSET(nbar) \
  (BAR0+(BARDIM*(nbar)))

typedef struct {
  /*bar info*/
  bool bit64;
  bool io;
  bool rom;
  uint64_t size;
  
  /*Address*/
  uint16_t dev;
  uint16_t func;
  uint8_t bus;
  uint8_t nBar;
} Bar;

void quickSortBars(Bar arr[], int low, int high);
inline void barInit(Bar* b){
  /*bar info*/
  b->rom=false;
  b->bit64=false;
  b->io=false;
  b->size=0;
  /*Address*/
  b->dev=0;
  b->func=0;
  b->bus=0;
  b->nBar=0;
}

#endif
