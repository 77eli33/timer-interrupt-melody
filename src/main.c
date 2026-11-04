#include <stdint.h>
#include <stdbool.h>

#define PART_TM4C129ENCPDT
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "buttons.h"
#include "melodies.h"
#include "tone.h"

#define SYSTEM_CLOCK_HZ 120000000u
#define MILLISECONDS_PER_SECOND 1000u
#define MELODY_COUNT (sizeof(melodies) / sizeof(melodies[0]))
#define BUZZER_PORT GPIO_PORTG_BASE
#define BUZZER_PIN GPIO_PIN_1

uint32_t system_clock_hz;
static volatile uint32_t milliseconds;

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
int main(void);
void Reset_Handler(void);
void SysTick_Handler(void);
static void TestTimer5A_Handler(void);
static void Default_Handler(void) { while (1) {} }

__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[] = {
    [0] = (uintptr_t)&_estack,
    [1] = (uintptr_t)Reset_Handler,
    [2] = (uintptr_t)Default_Handler,
    [3] = (uintptr_t)Default_Handler,
    [4] = (uintptr_t)Default_Handler,
    [5] = (uintptr_t)Default_Handler,
    [6] = (uintptr_t)Default_Handler,
    [11] = (uintptr_t)Default_Handler,
    [12] = (uintptr_t)Default_Handler,
    [14] = (uintptr_t)Default_Handler,
    [15] = (uintptr_t)SysTick_Handler,
    [INT_TIMER4A] = (uintptr_t)ToneDuration_Handler,
    [INT_TIMER5A] = (uintptr_t)TestTimer5A_Handler
};

void Reset_Handler(void) {
    for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata; ) *dst++ = *src++;
    for (uint32_t *dst = &_sbss; dst < &_ebss; ) *dst++ = 0;
    (void)main();
    while (1) {}
}

static void TestTimer5A_Handler(void) {
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    const uint8_t level = (uint8_t)GPIOPinRead(BUZZER_PORT, BUZZER_PIN);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, level ^ BUZZER_PIN);
}

void SysTick_Handler(void) {
    ++milliseconds;
}

int main(void) {
    system_clock_hz = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
                                        SYSTEM_CLOCK_HZ);
    if (system_clock_hz == 0u) while (1) {}

    // GPIO und Timer initialisieren
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER4) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER5)) {}

    GPIOPinTypeGPIOOutput(BUZZER_PORT, BUZZER_PIN);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);

    // Timer für Frequenz und Dauer konfigurieren
    TimerClockSourceSet(TIMER5_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER5_BASE, TIMER_CFG_PERIODIC);
    TimerClockSourceSet(TIMER4_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER4_BASE, TIMER_CFG_ONE_SHOT);

    buttons_init();
    SysTickPeriodSet(system_clock_hz / MILLISECONDS_PER_SECOND);
    SysTickIntEnable();
    SysTickEnable();
    IntMasterEnable();

    size_t selected_melody = 0u;
    const Melody *active_melody = NULL;
    uint32_t started_index = 0u;

    while (1) {
        const uint32_t pressed = buttons_poll(milliseconds);
        if ((pressed & BUTTON_S1_PRESSED) != 0u) {
            selected_melody = (selected_melody + 1u) % MELODY_COUNT;
        }

        if ((pressed & BUTTON_S2_PRESSED) != 0u) {
            if (active_melody != NULL) {
                tone(0u, 0u);
                active_melody = NULL;
            } else {
                tone(0u, 0u);
                note_index = 0u;
                started_index = 0u;
                active_melody = &melodies[selected_melody];
                tone(active_melody->notes[0].frequency_hz,
                     active_melody->notes[0].duration_ms);
            }
        }

        if (active_melody != NULL && note_index != started_index) {
            started_index = note_index;
            if (started_index < active_melody->length) {
                tone(active_melody->notes[started_index].frequency_hz,
                     active_melody->notes[started_index].duration_ms);
            } else {
                tone(0u, 0u);
                active_melody = NULL;
            }
        }
    }
}
