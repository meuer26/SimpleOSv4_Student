// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "syscalls.h"
#include "kernel.h"
#include "screen.h"
#include "fs.h"
#include "x86.h"
#include "vm.h"
#include "simpleOSlibc.h"
#include "interrupts.h"
#include "constants.h"
#include "frame-allocator.h"
#include "exceptions.h"
#include "keyboard.h"
#include "file.h"
#include "trap.h"
#include "schedule.h"

void pageFault()
{
    uint32_t cr2Value;
    asm volatile ("movl %%cr2, %0\n\t" : "=r" (cr2Value) : );

    // CR2 holds the virtual address that caused the page fault.
    // Printing the string representation of decimal value of the virtual address
    // in the panic message.
    strcpy((uint8_t *)0x3FFF00, (uint8_t *)"Page Fault:           ");
    itoa(cr2Value, (uint8_t *)0x3FFF0c);
    disableCursor();
    panic((uint8_t *)0x3FFF00);
}

void generalProtectionFault()
{
    disableCursor();
    panic((uint8_t *)"General Protection Fault!");
}