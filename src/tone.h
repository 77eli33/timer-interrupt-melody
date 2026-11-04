#ifndef TONE_H
#define TONE_H

#include <stdint.h>

// Shared note counter: Timer4A increments it at the end of each tone or rest;
// main() watches it to launch the next entry in the active melody.
extern volatile uint32_t note_index;

// Configure PG1 and both timers using the actual configured system clock.
void tone_init(uint32_t clock_hz);
// Play a tone for duration_ms without blocking. Frequency 0 gives a timed
// rest; duration 0 stops immediately without advancing note_index.
void tone(uint32_t frequency_hz, uint32_t duration_ms);

// The startup vector table assigns these handlers to Timer5A and Timer4A.
void Timer5A_Handler(void);
void ToneDuration_Handler(void);

#endif
