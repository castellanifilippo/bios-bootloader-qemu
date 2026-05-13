#ifndef BOOTPRINT
#define BOOTPRINT
#include "../utilities/utilities.h"

#define BOOTROWS 5
#define CENTERBOOT 9

extern const char * bootloaderString[BOOTROWS];

static inline void bootloaderPrint(void){
  int32_t i;
  for(i=0;i<BOOTROWS;i++)
    print(i,CENTERBOOT,bootloaderString[i],NORMAL);
}

#endif
