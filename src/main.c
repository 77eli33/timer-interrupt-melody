#include <stdbool.h>
#include <stdint.h>

#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "buttons.h"
#include "melodies.h"
#include "tone.h"

/*
 * Application control stays in the main loop. SysTick supplies a millisecond
 * clock for button polling, while tone.c uses two hardware timers to play a
 * note without blocking that loop.
 */
#define SYSTEM_CLOCK_HZ 120000000u
#define MILLISECONDS_PER_SECOND 1000u
#define MELODY_COUNT (sizeof(melodies) / sizeof(melodies[0]))

// The SysTick ISR writes this value; buttons_poll() reads it in main().
static volatile uint32_t milliseconds;

// One interrupt per millisecond; note length is timed by Timer4A instead.
void SysTick_Handler(void) {
    ++milliseconds;
}

int main(void) {
    // Configure the PLL and use the frequency actually returned by TivaWare
    // when calculating the SysTick and tone-timer periods.
    const uint32_t system_clock_hz =
        SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                           SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
                           SYSTEM_CLOCK_HZ);
    // A zero return means the requested system clock could not be set.
    if (system_clock_hz == 0u) while (1) {}

    // Each module enables and configures its own BoosterPack pins and timers.
    buttons_init();
    tone_init(system_clock_hz);

    // A 1 ms SysTick lets buttons_poll() sample the active-low buttons
    // every 5 ms without using button interrupts.
    SysTickPeriodSet(system_clock_hz / MILLISECONDS_PER_SECOND);
    SysTickIntEnable();
    SysTickEnable();
    // Enable CPU interrupts only after all peripherals and SysTick are ready.
    IntMasterEnable();

    // Selection may change during playback; active_melody keeps the melody
    // already playing until it ends or S2 stops it.
    size_t selected_melody = 0u;
    const Melody *active_melody = NULL;
    // Last note index launched by main(); note_index is advanced in Timer4A.
    uint32_t started_index = 0u;

    while (1) {
        // buttons_poll() returns press events, not a continuously held state.
        // Process S1 first if both events arrive in the same iteration.
        const uint32_t pressed = buttons_poll(milliseconds);
        if ((pressed & BUTTON_S1_PRESSED) != 0u) {
            // S1 selects the next melody for the next start. A melody that
            // is already playing continues until S2 stops it or it ends.
            selected_melody = (selected_melody + 1u) % MELODY_COUNT;
        }

        if ((pressed & BUTTON_S2_PRESSED) != 0u) {
            // S2 toggles playback. Starting always begins at the first note.
            if (active_melody != NULL) {
                // Zero duration cancels both timers and drives PG1 low.
                tone(0u, 0u);
                active_melody = NULL;
            } else {
                // Cancel any pending timeout before resetting the shared
                // counter and starting the selected melody at note zero.
                tone(0u, 0u);
                note_index = 0u;
                started_index = 0u;
                active_melody = &melodies[selected_melody];
                tone(active_melody->notes[0].frequency_hz,
                     active_melody->notes[0].duration_ms);
            }
        }

        if (active_melody != NULL && note_index != started_index) {
            // Timer4A increments note_index when the current note ends.
            // The main loop starts the next note outside the ISR, so button
            // polling and melody transitions share one control path.
            started_index = note_index;
            if (started_index < active_melody->length) {
                tone(active_melody->notes[started_index].frequency_hz,
                     active_melody->notes[started_index].duration_ms);
            } else {
                // After the final note, leave the output low and mark the
                // player idle so the next S2 press restarts from the start.
                tone(0u, 0u);
                active_melody = NULL;
            }
        }
    }
}
