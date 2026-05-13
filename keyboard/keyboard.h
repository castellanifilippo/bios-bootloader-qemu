#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../utilities/utilities.h"

#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

void keyboard_wait_code(void);

#endif