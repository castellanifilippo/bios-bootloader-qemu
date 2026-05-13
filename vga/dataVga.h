#ifndef VGADATA
#define VGADATA
#include "../utilities/utilities.h"

/*====== REGISTERS ======*/
#define NDATAREGISTER 61
extern uint8_t textMode80x25[NDATAREGISTER];

/*====== FONT ======*/
#define NDATAFONT 4096
extern uint8_t font8x16[NDATAFONT];

/*====== PALETTE ======*/
#define COLOR 3
#define NCOLOR 64
#define NDATAPALETTE (NCOLOR * COLOR)
extern uint8_t palette[NDATAPALETTE];

#endif
