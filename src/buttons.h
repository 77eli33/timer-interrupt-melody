#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

// Each bit represents one newly debounced press; release has no event bit.
enum ButtonPress {
    BUTTON_S1_PRESSED = 1u << 0,
    BUTTON_S2_PRESSED = 1u << 1
};

// Initialize active-low S1 and S2 on BoosterPack connector 2.
void buttons_init(void);
// Call repeatedly from main() with the SysTick millisecond count. The
// function samples no faster than every 5 ms and returns a press-event mask.
uint32_t buttons_poll(uint32_t now_ms);

#endif
