#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
// pin_map.h selects names using the part macro; PlatformIO sets only the MCU name.
#define PART_TM4C129ENCPDT
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"

#include "tone.h"

#define PWM_CLOCK_DIVISOR 16u
#define PWM_MIN_PERIOD_TICKS 2u
#define PWM_MAX_PERIOD_TICKS UINT16_MAX

// BOOSTXL-EDUMKII on BoosterPack 2: buzzer J4.40 = PG1/M0PWM5.
#define BUZZER_PORT GPIO_PORTG_BASE
#define BUZZER_PIN GPIO_PIN_1

static uint32_t pwm_clock_hz;
static volatile const uint32_t *milliseconds;
static uint32_t tone_started_ms;
static uint32_t tone_duration_ms;

void tone_stop(void) {
    PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT, false);
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);
    GPIOPinTypeGPIOOutput(BUZZER_PORT, BUZZER_PIN);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);
}

void tone_init(uint32_t clock_hz, volatile const uint32_t *time_ms) {
    pwm_clock_hz = clock_hz / PWM_CLOCK_DIVISOR;
    milliseconds = time_ms;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) {}
    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_16);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2,
                    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    tone_stop();
}

// Start a hardware PWM tone without blocking. Frequency 0 is a timed rest.
void tone(uint32_t frequency_hz, uint32_t duration_ms) {
    tone_started_ms = *milliseconds;
    tone_duration_ms = duration_ms;
    tone_stop();

    if (frequency_hz == 0u || duration_ms == 0u) return;

    const uint32_t period_ticks = pwm_clock_hz / frequency_hz;
    if (period_ticks < PWM_MIN_PERIOD_TICKS ||
        period_ticks > PWM_MAX_PERIOD_TICKS) return;

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, period_ticks);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, period_ticks / 2u);
    GPIOPinConfigure(GPIO_PG1_M0PWM5);
    GPIOPinTypePWM(BUZZER_PORT, BUZZER_PIN);
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
    PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT, true);
}

bool tone_finished(void) {
    return (uint32_t)(*milliseconds - tone_started_ms) >= tone_duration_ms;
}
