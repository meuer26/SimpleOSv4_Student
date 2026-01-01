// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "x86.h"
#include "constants.h"


void outputIOPort(uint16_t port, uint8_t data)
{
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

uint8_t inputIOPort(uint16_t port)
{
    uint8_t data;
    
    asm volatile("in %1,%0" : "=a" (data) : "d" (port));
    
    return data;
}

void ioPortWordToMem(uint16_t port, uint8_t *destinationMemory, uint32_t numberOfWords)
{
    asm volatile ("mov %0, %%dx\n\t" : : "r" (port));
    asm volatile ("movl %0, %%edi\n\t" : : "r" (destinationMemory));
    asm volatile ("cld\n\t");
    asm volatile ("movl %0, %%ecx\n\t" : : "r" (numberOfWords));
    asm volatile ("rep insw");
}

void memToIoPortWord(uint16_t destinationPort, uint8_t *sourceMemory, uint32_t numberOfWords)
{
    asm volatile ("mov %0, %%dx\n\t" : : "r" (destinationPort));
    asm volatile ("movl %0, %%esi\n\t" : : "r" (sourceMemory));
    asm volatile ("cld\n\t");
    asm volatile ("movl %0, %%ecx\n\t" : : "r" (numberOfWords));
    asm volatile ("rep outsw");
}

void memoryCopy(uint8_t *startingMemory, uint8_t *destinationMemory, uint32_t numberOfWords)
{
    asm volatile ("movl %0, %%esi\n\t" : : "r" (startingMemory));
    asm volatile ("movl %0, %%edi\n\t" : : "r" (destinationMemory));
    asm volatile ("cld\n\t");
    asm volatile ("movl %0, %%ecx\n\t" : : "r" (numberOfWords));                  
    asm volatile ("rep movsw\n\t");
}

void loadTaskRegister(uint16_t taskRegisterValue)
{
    asm volatile ("mov %0, %%ax\n\t" : : "r" (taskRegisterValue));
    asm volatile ("ltr %ax\n\t");
}

void storeValueAtMemLoc(uint8_t *destinationMemory, uint32_t value)
{
    asm volatile ("movl %0, %%ebx\n\t" : : "r" (destinationMemory));
    asm volatile ("movl %0, %%eax\n\t" : : "r" (value));
    asm volatile ("movl %eax, (%ebx)\n\t");
}

uint32_t readValueFromMemLoc(uint8_t *sourceMemory)
{
    uint32_t value;
    
    asm volatile ("movl %0, %%ebx\n\t" : : "r" (sourceMemory));
    asm volatile ("movl (%ebx), %eax\n\t");
    asm volatile ("movl %%eax, %0\n\t" : "=r" (value) : );

    return value;
}

uint32_t sysCall(uint32_t sysCallNumber, uint32_t arg1, uint32_t currentPid)
{
    uint32_t returnValue;
    
    asm volatile ("movl %0, %%ecx\n\t" : : "r" (currentPid)); 
    asm volatile ("movl %0, %%ebx\n\t" : : "r" (arg1)); 
    asm volatile ("movl %0, %%eax\n\t" : : "r" (sysCallNumber)); 
    asm volatile ("pusha\n\t");
    asm volatile ("int $0x80\n\t");
    asm volatile ("popa\n\t"); 
    asm volatile ("movl %%eax, %0\n\t" : "=r" (returnValue) : );

    return returnValue;
}

void startApplicationProcessor()
{
    
    // https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
    
    volatile unsigned int* lapic = (volatile unsigned int*)LAPIC_ADDR;

    // set AP address 0x1000 in ICR low
    lapic[0x310 >> 2] = 0;              // ICR high clear
    lapic[0x300 >> 2] = 0x000C4500;     // INIT IPI all except self

    for (volatile int i = 0; i < 10000; i++); // delay

    // send SIPI (start up IPI) to 0x1000
    lapic[0x310 >> 2] = 0;              // ICR high
    lapic[0x300 >> 2] = 0x000c4601;      // SIPI to vector 0x01 (0x1000)

    for (volatile int i = 0; i < 10000; i++); // delay


}