#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "buttons.h"

//time between two samples is 5 ms
#define SAMPLE_INTERVAL_MS 5u

//we want a total debounce time of 15-20 ms
//(3*5ms = 15ms between the 4 samples)
#define DEBOUNCE_SAMPLES 4u

// BOOSTXL-EDUMKII on BoosterPack 2: S1 = J4.33/PH1, S2 = J4.32/PK6.
//both buttons are active low
//the internal pull-up is enabled
#define BUTTON_S1_PORT GPIO_PORTH_BASE
#define BUTTON_S1_PIN GPIO_PIN_1
#define BUTTON_S2_PORT GPIO_PORTK_BASE
#define BUTTON_S2_PIN GPIO_PIN_6


//definition data type "ButtonState"
typedef struct {
    bool stable_pressed;          //state after debouncing
    uint8_t pending_count;        //consecutive samples that differ from stable_pressed
} ButtonState;


static ButtonState s1;
static ButtonState s2;


//initialize buttons
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
    s1 = (ButtonState){false, 0u};
    s2 = (ButtonState){false, 0u};
}



/*
 * Debouncing by counting consecutive samples:
 * The raw level must differ from the confirmed state for DEBOUNCE_SAMPLES
 * samples/times in a row before it is accepted.
 * If the raw level returns to the confirmed state in between (bounce or
 * noise), the counter restarts.
 * Returns true only once per press.
 */


/* ===== Part 1: Polling ===== */
/*
 * Polling means the CPU asks for the pin state itself, at a time of its
 * own choosing. Returns the RAW state (true = pressed). The level may
 * still be bouncing.
 */
static bool button_read_raw(uint32_t port, uint8_t pin) {
    // GPIOPinRead() returns the port value masked with 'pin' (bit set = high),
    // not 0 or 1. The buttons are active low: low level = pressed.
    return (GPIOPinRead(port, pin) & pin) == 0u;
}

/* ===== Part 2: Debouncing ===== */
/*
 * Debouncing by counting consecutive samples: the raw level must differ
 * from the confirmed state for DEBOUNCE_SAMPLES samples in a row before it
 * is accepted. If the raw level returns to the confirmed state in between
 * (bounce or noise), the counter restarts.
 * Returns true only once per press (press edge, not release).
 */
static bool button_debounce(ButtonState *state, bool raw_pressed) {
    // Raw level equals confirmed state: nothing pending
    if (raw_pressed == state->stable_pressed) {
        state->pending_count = 0u;
        return false;
    }
    // Raw level differs: count this sample
    if (++state->pending_count >= DEBOUNCE_SAMPLES) {
        state->stable_pressed = raw_pressed;   // accept the new state
        state->pending_count = 0u;
        return raw_pressed;                    // event only for a press
    }
    return false;
}

/* ===== Part 3: Fixed-rate sampling ===== */
uint32_t buttons_poll(uint32_t now_ms) {
    static uint32_t last_sample_ms = 0u;
    uint32_t pressed = 0u;

    // Sample only every SAMPLE_INTERVAL_MS (unsigned subtraction is overflow-safe)
    if ((uint32_t)(now_ms - last_sample_ms) < SAMPLE_INTERVAL_MS) {
        return 0u;
    }
    // Take over the current time, so a stalled main loop causes no catch-up burst
    last_sample_ms = now_ms;

    if (button_debounce(&s1, button_read_raw(BUTTON_S1_PORT, BUTTON_S1_PIN))) {
        pressed |= BUTTON_S1_PRESSED;
    }
    if (button_debounce(&s2, button_read_raw(BUTTON_S2_PORT, BUTTON_S2_PIN))) {
        pressed |= BUTTON_S2_PRESSED;
    }
    return pressed;
}

//melody_index() und play()
