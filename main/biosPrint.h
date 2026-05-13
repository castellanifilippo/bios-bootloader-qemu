#ifndef BIOSPRINT
#define BIOSPRINT
#include "../utilities/utilities.h"

#define BIOSROWS 4
#define CENTERBIOS 25

extern const char * biosString[BIOSROWS];

static inline void biosPrint(void){
  int32_t i;
  for(i=0;i<BIOSROWS;i++)
    print(i,CENTERBIOS,biosString[i],NORMAL);
}

#endif
