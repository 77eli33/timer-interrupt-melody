#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "buttons.h"
#include "tone.h"

#define SYSTEM_CLOCK_HZ 120000000u
#define MILLISECONDS_PER_SECOND 1000u
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

typedef struct {
    uint32_t frequency_hz;
    uint32_t duration_ms;
} Tone;

// Frequenzen um 1 kHz und darüber waren mit dem Buzzer im Einzeltest hörbar.
static const Tone melody_s1[] = {
    {1047u, 220u}, {1175u, 220u}, {1319u, 220u}, {1397u, 220u},
    {1568u, 220u}, {1760u, 220u}, {1976u, 220u}, {2093u, 440u}
};

static const Tone melody_s2[] = {
    {1568u, 180u}, {0u, 60u}, {1976u, 180u}, {0u, 60u},
    {2093u, 360u}, {0u, 100u}, {1976u, 180u}, {1568u, 360u}
};

static volatile uint32_t milliseconds;

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
int main(void);
void Reset_Handler(void);
void SysTick_Handler(void);
static void Default_Handler(void) { while (1) {} }

__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[] = {
    (uintptr_t)&_estack, (uintptr_t)Reset_Handler,
    (uintptr_t)Default_Handler, (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler, (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler, 0, 0, 0, 0,
    (uintptr_t)Default_Handler, (uintptr_t)Default_Handler, 0,
    (uintptr_t)Default_Handler, (uintptr_t)SysTick_Handler
};

void Reset_Handler(void) {
    for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata; ) *dst++ = *src++;
    for (uint32_t *dst = &_sbss; dst < &_ebss; ) *dst++ = 0;
    (void)main();
    while (1) {}
}

void SysTick_Handler(void) {
    ++milliseconds;
}

int main(void) {
    const uint32_t system_clock_hz =
        SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                           SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
                           SYSTEM_CLOCK_HZ);
    if (system_clock_hz == 0u) while (1) {}

    buttons_init();
    tone_init(system_clock_hz, &milliseconds);

    SysTickPeriodSet(system_clock_hz / MILLISECONDS_PER_SECOND);
    SysTickIntEnable();
    SysTickEnable();
    IntMasterEnable();

    const Tone *melody = NULL;
    size_t melody_length = 0u;
    size_t note_index = 0u;

    while (1) {
        const uint32_t pressed = buttons_poll(milliseconds);
        if (pressed != 0u) {
            // Bei gleichzeitigem Druck erhält S2 Vorrang.
            const bool select_s2 = (pressed & BUTTON_S2_PRESSED) != 0u;
            melody = select_s2 ? melody_s2 : melody_s1;
            melody_length = select_s2 ? ARRAY_LENGTH(melody_s2)
                                      : ARRAY_LENGTH(melody_s1);
            note_index = 0u;
            tone(melody[note_index].frequency_hz,
                 melody[note_index].duration_ms);
        } else if (melody != NULL && tone_finished()) {
            ++note_index;
            if (note_index == melody_length) {
                tone_stop();
                melody = NULL;
            } else {
                tone(melody[note_index].frequency_hz,
                     melody[note_index].duration_ms);
            }
        }
    }
}
