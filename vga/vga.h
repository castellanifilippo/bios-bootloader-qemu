#ifndef VGA
#define VGA
#include "../utilities/utilities.h"

/*TEXT MODE 80X25 values*/
#define FONTHEIGHT 16
#define VIDEOMEM   0xB8000
#define COLS        80
#define ROWS        25

void vgaInit(void);
void screen(bool enable);/*Set screen on/off*/

#endif
