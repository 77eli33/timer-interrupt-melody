#include <stdbool.h>
#include <stdint.h>

#ifndef PART_TM4C129ENCPDT
#define PART_TM4C129ENCPDT  // also set by the CCS project
#endif
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "tone.h"

/*
 * Timer5A generates the buzzer's square wave by changing PG1 once per
 * half-period. Timer4A is a separate one-shot timer for the note length.
 * A silent rest uses only Timer4A, so the melody can advance without sound.
 */
#define TIMER_MIN_PERIOD_TICKS 2u
#define MILLISECONDS_PER_SECOND 1000u

// BOOSTXL-EDUMKII on BoosterPack connector 2: buzzer J4.40 is PG1.
#define BUZZER_PORT GPIO_PORTG_BASE
#define BUZZER_PIN GPIO_PIN_1

// Shared with main(): volatile makes every read observe the ISR-updated value.
volatile uint32_t note_index;

// Set from the system clock reported by SysCtlClockFreqSet() in main().
static uint32_t timer_clock_hz;
// A late Timer5A interrupt must not toggle PG1 after a note has stopped.
static volatile bool tone_active;

void Timer5A_Handler(void) {
    // A timeout represents one half-period. Acknowledge it before toggling
    // so the next timeout can produce another interrupt.
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    if (!tone_active) return;

    // GPIOPinRead() returns the masked PG1 bit. XOR flips only that bit;
    // GPIOPinWrite() writes only the selected pin, leaving other pins alone.
    const uint8_t level = (uint8_t)GPIOPinRead(BUZZER_PORT, BUZZER_PIN);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, level ^ BUZZER_PIN);
}

void ToneDuration_Handler(void) {
    // Timer4A runs once per note or rest. Clear its timeout first, then
    // prevent another frequency toggle and leave the buzzer at LOW.
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    tone_active = false;
    TimerDisable(TIMER5_BASE, TIMER_A);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);
    // Advancing only the counter keeps melody selection out of the ISR.
    ++note_index;
}

void tone_init(uint32_t clock_hz) {
    timer_clock_hz = clock_hz;
    tone_active = false;

    // Peripheral clocks must be enabled before their registers are used.
    // The ready check waits for all three modules to become accessible.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER4) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER5)) {}

    // PG1 is the BoosterPack 2 buzzer pin; start with an inactive LOW level.
    GPIOPinTypeGPIOOutput(BUZZER_PORT, BUZZER_PIN);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);

    // Timer5A repeats every half-period; Timer4A expires once after the
    // requested duration. Both count system-clock ticks.
    TimerClockSourceSet(TIMER5_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER5_BASE, TIMER_CFG_PERIODIC);
    TimerClockSourceSet(TIMER4_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER4_BASE, TIMER_CFG_ONE_SHOT);

    // Arm timeout sources and NVIC lines here. main() enables global CPU
    // interrupts after initialization; tone() starts the timers per note.
    TimerIntEnable(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER5A);
    IntEnable(INT_TIMER4A);
}

void tone(uint32_t frequency_hz, uint32_t duration_ms) {
    // Stop the old note before changing either timer load. Clear both the
    // peripheral timeout flags and NVIC pend bits so an old interrupt cannot
    // be mistaken for a timeout belonging to the new note.
    tone_active = false;
    TimerDisable(TIMER4_BASE, TIMER_A);
    TimerDisable(TIMER5_BASE, TIMER_A);
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    IntPendClear(INT_TIMER4A);
    IntPendClear(INT_TIMER5A);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);

    // The main loop calls tone(0, 0) to stop; no completion event is raised.
    if (duration_ms == 0u) return;

    // Convert milliseconds to timer ticks. A 32-bit load cannot hold every
    // possible duration, so reject values that would overflow the product.
    const uint32_t ticks_per_ms = timer_clock_hz / MILLISECONDS_PER_SECOND;
    if (ticks_per_ms == 0u || duration_ms > UINT32_MAX / ticks_per_ms) return;
    const uint32_t duration_ticks = ticks_per_ms * duration_ms;
    // The counter includes its zero state; load N - 1 for N clock ticks.
    TimerLoadSet(TIMER4_BASE, TIMER_A, duration_ticks - 1u);

    if (frequency_hz != 0u) {
        // Two GPIO transitions make one full tone cycle. Timer5A therefore
        // interrupts at twice the requested audio frequency.
        const uint32_t half_period_ticks = timer_clock_hz / frequency_hz / 2u;
        if (half_period_ticks < TIMER_MIN_PERIOD_TICKS) return;
        TimerLoadSet(TIMER5_BASE, TIMER_A, half_period_ticks - 1u);
        tone_active = true;
    }

    // A zero frequency is a silent timed rest: only Timer4A runs. Its ISR
    // still increments note_index, allowing main() to start the next note.
    TimerEnable(TIMER4_BASE, TIMER_A);
    if (tone_active) TimerEnable(TIMER5_BASE, TIMER_A);
}
