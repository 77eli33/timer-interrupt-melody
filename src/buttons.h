#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

enum ButtonPress {
    BUTTON_S1_PRESSED = 1u << 0,
    BUTTON_S2_PRESSED = 1u << 1
};

void buttons_init(void);
uint32_t buttons_poll(uint32_t now_ms);

#endif
