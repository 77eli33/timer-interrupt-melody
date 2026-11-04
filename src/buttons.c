#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "buttons.h"

/*
 * Buttons are polled from main(), as required by the lab. SysTick provides
 * the millisecond time base, but it does not read the GPIO pins itself.
 * Sampling and debouncing therefore remain outside an interrupt handler.
 */
// Skip calls until at least 5 ms have passed since the previous sample.
#define SAMPLE_INTERVAL_MS 5u

// Four matching samples span 15 ms from the first to the fourth sample.
#define DEBOUNCE_SAMPLES 4u

// BOOSTXL-EDUMKII on BoosterPack 2: S1 = J4.33/PH1, S2 = J4.32/PK6.
// Both switches are active low; internal pull-ups hold released pins high.
#define BUTTON_S1_PORT GPIO_PORTH_BASE
#define BUTTON_S1_PIN GPIO_PIN_1
#define BUTTON_S2_PORT GPIO_PORTK_BASE
#define BUTTON_S2_PIN GPIO_PIN_6


// The stable state changes only after several matching raw readings.
typedef struct {
    bool stable_pressed;          // Debounced switch state.
    uint8_t pending_count;        // Consecutive samples that disagree.
} ButtonState;


static ButtonState s1;
static ButtonState s2;


// Configure the two BoosterPack pins as inputs with pull-up resistors.
void buttons_init(void) {
    // GPIO ports H and K need their peripheral clocks before pin setup.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) {}
    GPIOPinTypeGPIOInput(BUTTON_S1_PORT, BUTTON_S1_PIN);
    GPIOPinTypeGPIOInput(BUTTON_S2_PORT, BUTTON_S2_PIN);
    // A released button reads HIGH; pressing it connects the pin to LOW.
    GPIOPadConfigSet(BUTTON_S1_PORT, BUTTON_S1_PIN,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(BUTTON_S2_PORT, BUTTON_S2_PIN,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // Begin with both buttons released and no transition pending.
    s1 = (ButtonState){false, 0u};
    s2 = (ButtonState){false, 0u};
}



/*
 * Polling reads the raw pin level without a GPIO interrupt. The level may
 * still be bouncing; the caller passes it through button_debounce().
 */
static bool button_read_raw(uint32_t port, uint8_t pin) {
    // GPIOPinRead() returns the port value masked with 'pin' (bit set = high),
    // not 0 or 1. The buttons are active low: low level = pressed.
    return (GPIOPinRead(port, pin) & pin) == 0u;
}

/*
 * Debouncing requires DEBOUNCE_SAMPLES consecutive readings that disagree
 * with the confirmed state. A bounce back to the old state clears the count.
 * Both press and release update stable_pressed, but only a confirmed press
 * produces an event. Confirming the release arms the next press event.
 */
static bool button_debounce(ButtonState *state, bool raw_pressed) {
    // Raw level equals confirmed state: nothing pending.
    if (raw_pressed == state->stable_pressed) {
        state->pending_count = 0u;
        return false;
    }
    // Raw level differs: count this sample.
    if (++state->pending_count >= DEBOUNCE_SAMPLES) {
        state->stable_pressed = raw_pressed;   // Accept the new state.
        state->pending_count = 0u;
        return raw_pressed;                    // Report a press, not a release.
    }
    return false;
}

// Return one bit for each newly confirmed press; release produces no event.
uint32_t buttons_poll(uint32_t now_ms) {
    // Persist the sampling deadline across main-loop iterations.
    static uint32_t last_sample_ms = 0u;
    uint32_t pressed = 0u;

    // Unsigned subtraction also works across the millisecond counter wrap.
    if ((uint32_t)(now_ms - last_sample_ms) < SAMPLE_INTERVAL_MS) {
        return 0u;
    }
    // A stalled main loop resumes with one sample, not a catch-up burst.
    last_sample_ms = now_ms;

    // The return value is a bitmask, so presses on S1 and S2 can be reported
    // together and handled in a defined order by main().
    if (button_debounce(&s1, button_read_raw(BUTTON_S1_PORT, BUTTON_S1_PIN))) {
        pressed |= BUTTON_S1_PRESSED;
    }
    if (button_debounce(&s2, button_read_raw(BUTTON_S2_PORT, BUTTON_S2_PIN))) {
        pressed |= BUTTON_S2_PRESSED;
    }
    return pressed;
}
