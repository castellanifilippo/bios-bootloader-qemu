#include "../utilities/utilities.h"
#include "keyboard.h"

// Function to pause the program until a key is pressed on the keyboard
// Reads make and break codes
void keyboard_wait_code()
{
    uint8_t c;
    for(uint32_t i = 0; i < 2; i++){
        do
            c = inb(KEYBOARD_STATUS_PORT);
        while (!(c & 0x01));
        inb(KEYBOARD_DATA_PORT);
    }
}

