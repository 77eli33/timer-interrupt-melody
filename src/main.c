#include <stdint.h>
#include <stdbool.h>

#define PART_TM4C129ENCPDT
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#define SYSTEM_CLOCK_HZ 120000000u
#define TEST_FREQUENCY_HZ 1000u
#define BUZZER_PORT GPIO_PORTG_BASE
#define BUZZER_PIN GPIO_PIN_1

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
int main(void);
void Reset_Handler(void);
static void TestTimer5A_Handler(void);
static void Default_Handler(void) { while (1) {} }

__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[] = {
    (uintptr_t)&_estack, (uintptr_t)Reset_Handler,
    (uintptr_t)Default_Handler, (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler, (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler, 0, 0, 0, 0,
    (uintptr_t)Default_Handler, (uintptr_t)Default_Handler, 0,
    (uintptr_t)Default_Handler, (uintptr_t)Default_Handler,
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

int main(void) {
    const uint32_t system_clock_hz =
        SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                           SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
                           SYSTEM_CLOCK_HZ);
    if (system_clock_hz == 0u) while (1) {}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER5)) {}

    GPIOPinTypeGPIOOutput(BUZZER_PORT, BUZZER_PIN);
    GPIOPinWrite(BUZZER_PORT, BUZZER_PIN, 0u);

    TimerClockSourceSet(TIMER5_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER5_BASE, TIMER_CFG_PERIODIC);
    const uint32_t half_period_ticks = system_clock_hz / (2u * TEST_FREQUENCY_HZ);
    TimerLoadSet(TIMER5_BASE, TIMER_A, half_period_ticks - 1u);
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER5A);
    IntMasterEnable();
    TimerEnable(TIMER5_BASE, TIMER_A);

    while (1) {}
}
