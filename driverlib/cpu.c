//*****************************************************************************
//
// cpu.c - Instruction wrappers for special CPU instructions needed by the
//         drivers.
//
// Copyright (c) 2006-2020 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
// 
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the  
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This is part of revision 2.2.0.295 of the Tiva Peripheral Driver Library.
//
//*****************************************************************************

// NOTE: adapted for TI Arm Clang (CCS 20+). The original TivaWare cpu.c
// uses __attribute__((naked)) functions with C statements for the gcc
// variant, which clang rejects. Same functions and return values as in
// driverlib/cpu.h, written with extended inline assembly instead.

#include <stdint.h>
#include "driverlib/cpu.h"

// Disable interrupts, return the previous PRIMASK (1 = were disabled).
uint32_t
CPUcpsid(void)
{
    uint32_t ui32Ret;
    __asm volatile ("mrs %0, PRIMASK\n"
                    "cpsid i" : "=r" (ui32Ret) : : "memory");
    return(ui32Ret);
}

// Return the current PRIMASK (1 = interrupts disabled).
uint32_t
CPUprimask(void)
{
    uint32_t ui32Ret;
    __asm volatile ("mrs %0, PRIMASK" : "=r" (ui32Ret));
    return(ui32Ret);
}

// Enable interrupts, return the previous PRIMASK (1 = were disabled).
uint32_t
CPUcpsie(void)
{
    uint32_t ui32Ret;
    __asm volatile ("mrs %0, PRIMASK\n"
                    "cpsie i" : "=r" (ui32Ret) : : "memory");
    return(ui32Ret);
}

// Wait for interrupt.
void
CPUwfi(void)
{
    __asm volatile ("wfi" : : : "memory");
}

// Set the interrupt priority masking level.
void
CPUbasepriSet(uint32_t ui32NewBasepri)
{
    __asm volatile ("msr BASEPRI, %0" : : "r" (ui32NewBasepri) : "memory");
}

// Return the interrupt priority masking level.
uint32_t
CPUbasepriGet(void)
{
    uint32_t ui32Ret;
    __asm volatile ("mrs %0, BASEPRI" : "=r" (ui32Ret));
    return(ui32Ret);
}
