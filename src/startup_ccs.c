#include <stdint.h>

// Select the TM4C129ENCPDT interrupt numbers from the TivaWare headers.
#define PART_TM4C129ENCPDT
#include "inc/hw_ints.h"

#include "tone.h"

/*
 * GCC startup for the TM4C129ENCPDT. The referenced CCS example also keeps
 * its vector table in a startup file, but calls the CCS runtime at reset.
 * This project uses a small C reset routine with the existing GNU linker
 * script instead.
 */
// The linker defines the stack top and the flash/RAM boundaries of .data/.bss.
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

int main(void);
void Reset_Handler(void);
void SysTick_Handler(void);

static void Default_Handler(void) {
    // The mapped fault/system exceptions stop here for debugger inspection.
    while (1) {}
}

// Entry 0 is the initial stack pointer; entry 1 is the reset address.
// TivaWare's INT_* constants are vector indices, not timer register addresses.
// The GNU 'used' attribute and the linker's KEEP rule retain this table even
// when unused sections are removed. .isr_vector starts at flash address zero.
__attribute__((section(".isr_vector"), used))
const uintptr_t g_pfnVectors[] = {
    [0] = (uintptr_t)&_estack,
    [1] = (uintptr_t)Reset_Handler,
    [2] = (uintptr_t)Default_Handler,   // NMI
    [3] = (uintptr_t)Default_Handler,   // HardFault
    [4] = (uintptr_t)Default_Handler,   // MemManage
    [5] = (uintptr_t)Default_Handler,   // BusFault
    [6] = (uintptr_t)Default_Handler,   // UsageFault
    [11] = (uintptr_t)Default_Handler,  // SVCall
    [12] = (uintptr_t)Default_Handler,  // DebugMonitor
    [14] = (uintptr_t)Default_Handler,  // PendSV
    [15] = (uintptr_t)SysTick_Handler,
    [INT_TIMER4A] = (uintptr_t)ToneDuration_Handler,
    [INT_TIMER5A] = (uintptr_t)Timer5A_Handler
};

void Reset_Handler(void) {
    // .data has initial values stored in flash at _sidata; copy them into
    // RAM before any C global with an initializer is read.
    for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata; ) {
        *dst++ = *src++;
    }
    // .bss contains globals without explicit initial values. C requires
    // them to start at zero, including the millisecond and note counters.
    for (uint32_t *dst = &_sbss; dst < &_ebss; ) {
        *dst++ = 0u;
    }

    // CCS's _c_int00 is not linked in this GCC project. main() takes over
    // after the minimal C initialization; returning from main() is terminal.
    (void)main();
    while (1) {}
}
