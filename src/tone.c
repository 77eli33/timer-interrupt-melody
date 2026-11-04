#include <stdint.h>
#include <stdbool.h>

#define PART_TM4C129ENCPDT
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "tone.h"

#define TIMER_MIN_PERIOD_TICKS 2u

// BOOSTXL-EDUMKII on BoosterPack 2: buzzer J4.40 = PG1.
#define BUZZER_PORT GPIO_PORTG_BASE
#define BUZZER_PIN GPIO_PIN_1

volatile uint32_t note_index;

static uint32_t timer_clock_hz;
static volatile const uint32_t *milliseconds;
static uint32_t tone_started_ms;
static uint32_t tone_duration_ms;
static volatile bool tone_active;

void Timer5A_Handler(void) {
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    if (!tone_active) return;

    const uint8_t level = (uint8_t)GPIOPinRead(BUZZER_PORT, BUZZER_PIN);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, level ^ BUZZER_PIN);
}

void ToneDuration_Handler(void) {
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    TimerDisable(TIMER5_BASE, TIMER_A);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);
    ++note_index;
}

/*
void tone_stop(void) {
    tone_active = false;
    TimerDisable(TIMER5_BASE, TIMER_A);
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    IntPendClear(INT_TIMER5A);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);
}*/

void tone_init(uint32_t clock_hz, volatile const uint32_t *time_ms) {
    timer_clock_hz = clock_hz;
    milliseconds = time_ms;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER5)) {}
    GPIOPinTypeGPIOOutput(BUZZER_PORT, BUZZER_PIN);
    TimerClockSourceSet(TIMER5_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER5_BASE, TIMER_CFG_PERIODIC);
    tone_stop();
    TimerIntEnable(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER5A);
}

// Earlier version with a note duration, kept for comparison during the test.
/*void tone(uint32_t frequency_hz, uint32_t duration_ms) {
    tone_started_ms = *milliseconds;
    tone_duration_ms = duration_ms;
    tone_stop();

    if (frequency_hz == 0u || duration_ms == 0u) return;

    const uint32_t half_period_ticks = timer_clock_hz / frequency_hz / 2u;
    if (half_period_ticks < TIMER_MIN_PERIOD_TICKS) return;

    TimerLoadSet(TIMER5_BASE, TIMER_A, half_period_ticks - 1u);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, BUZZER_PIN);
    tone_active = true;
    TimerEnable(TIMER5_BASE, TIMER_A);
}*/

void tone(uint32_t frequency_hz, uint32_t duration_ms) {
    TimerDisable(TIMER4_BASE, TIMER_A);
    TimerDisable(TIMER5_BASE, TIMER_A);
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    IntPendClear(INT_TIMER4A);
    IntPendClear(INT_TIMER5A);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);

    if (frequency_hz == 0u || duration_ms == 0u) return;

    const uint32_t half_period_ticks = system_clock_hz / frequency_hz / 2u;
    const uint32_t ticks_per_ms = system_clock_hz / 1000u;
    if (half_period_ticks < TIMER_MIN_PERIOD_TICKS ||
        ticks_per_ms == 0u || duration_ms > UINT32_MAX / ticks_per_ms) return;
    const uint32_t duration_ticks = ticks_per_ms * duration_ms;

    TimerLoadSet(TIMER5_BASE, TIMER_A, half_period_ticks - 1u);
    TimerLoadSet(TIMER4_BASE, TIMER_A, duration_ticks - 1u);
    TimerIntEnable(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER5A);
    IntEnable(INT_TIMER4A);
    IntMasterEnable();
    TimerEnable(TIMER4_BASE, TIMER_A);
    TimerEnable(TIMER5_BASE, TIMER_A);
}

bool tone_finished(void) {
    return (uint32_t)(*milliseconds - tone_started_ms) >= tone_duration_ms;
}
