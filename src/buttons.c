#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "buttons.h"

#define DEBOUNCE_MS 20u

// BOOSTXL-EDUMKII on BoosterPack 2: S1 = J4.33/PH1, S2 = J4.32/PK6.
// Both buttons are active low and have pullups on the BoosterPack.
#define BUTTON_S1_PORT GPIO_PORTH_BASE
#define BUTTON_S1_PIN GPIO_PIN_1
#define BUTTON_S2_PORT GPIO_PORTK_BASE
#define BUTTON_S2_PIN GPIO_PIN_6

typedef struct {
    bool stable_pressed;
    bool last_raw_pressed;
    uint32_t changed_at_ms;
} ButtonState;

static ButtonState s1;
static ButtonState s2;

static bool button_pressed(uint32_t port, uint8_t pin, ButtonState *state,
                           uint32_t now_ms) {
    const bool raw_pressed = (GPIOPinRead(port, pin) & pin) == 0u;
    if (raw_pressed != state->last_raw_pressed) {
        state->last_raw_pressed = raw_pressed;
        state->changed_at_ms = now_ms;
    }
    if (raw_pressed != state->stable_pressed &&
        (uint32_t)(now_ms - state->changed_at_ms) >= DEBOUNCE_MS) {
        state->stable_pressed = raw_pressed;
        return raw_pressed; // Only the debounced press edge starts a melody.
    }
    return false;
}

void buttons_init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) {}
    GPIOPinTypeGPIOInput(BUTTON_S1_PORT, BUTTON_S1_PIN);
    GPIOPinTypeGPIOInput(BUTTON_S2_PORT, BUTTON_S2_PIN);
    GPIOPadConfigSet(BUTTON_S1_PORT, BUTTON_S1_PIN,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(BUTTON_S2_PORT, BUTTON_S2_PIN,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    s1 = (ButtonState){false, false, 0u};
    s2 = (ButtonState){false, false, 0u};
}

uint32_t buttons_poll(uint32_t now_ms) {
    uint32_t pressed = 0u;
    if (button_pressed(BUTTON_S1_PORT, BUTTON_S1_PIN, &s1, now_ms)) {
        pressed |= BUTTON_S1_PRESSED;
    }
    if (button_pressed(BUTTON_S2_PORT, BUTTON_S2_PIN, &s2, now_ms)) {
        pressed |= BUTTON_S2_PRESSED;
    }
    return pressed;
}
