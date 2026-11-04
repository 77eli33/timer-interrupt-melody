#ifndef TONE_H
#define TONE_H

#include <stdbool.h>
#include <stdint.h>

extern uint32_t system_clock_hz;
extern volatile uint32_t note_index;

void tone_init(uint32_t clock_hz, volatile const uint32_t *milliseconds);
void tone(uint32_t frequency_hz, uint32_t duration_ms);
bool tone_finished(void);
void tone_stop(void);
void Timer5A_Handler(void);
void ToneDuration_Handler(void);

#endif
