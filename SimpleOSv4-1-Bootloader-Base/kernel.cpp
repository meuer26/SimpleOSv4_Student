// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "kernel.h"
#include "screen.h"
#include "fs.h"
#include "x86.h"
#include "vm.h"
#include "keyboard.h"
#include "interrupts.h"
#include "syscalls.h"
#include "simpleOSlibc.h"
#include "constants.h"
#include "frame-allocator.h"
#include "exceptions.h"
#include "vmmonitor.h"
#include "file.h"

uint32_t currentPid = 0;
uint32_t cursorRow = 0;
uint8_t *bufferMem = (uint8_t *)KEYBOARD_BUFFER;
uint8_t *cursorMemory = (uint8_t *)SHELL_CURSOR_POS;


void kInit()
{
    disableCursor();

    createSemaphore(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE, 1, 1);
    createSemaphore(KERNEL_OWNED, (uint8_t *)PROCESS_TABLE_LOC, 1, 1);
    createSemaphore(KERNEL_OWNED, (uint8_t *)PAGEFRAME_MAP_BASE, 1, 1);

    clearScreen();

    printString(COLOR_LIGHT_BLUE, cursorRow++, 0, (uint8_t *)"Starting Kernel Initialization:");

    // zero out process table memory, kernel heap and user heap
    fillMemory((uint8_t *)(PROCESS_TABLE_LOC) , (uint8_t)0x0, PAGE_SIZE);
    fillMemory((uint8_t *)(KERNEL_HEAP) , (uint8_t)0x0, PAGE_SIZE);
    fillMemory((uint8_t *)(USER_HEAP) , (uint8_t)0x0, PAGE_SIZE);
    
    startApplicationProcessor();

    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_block_usage, (uint8_t *)EXT2_BLOCK_USAGE_MAP);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_inode_usage, (uint8_t *)EXT2_INODE_USAGE_MAP);

    currentPid = initializeTask(currentPid, PROC_SLEEPING, STACK_START_LOC, (uint8_t *)"shell2", 100);
    createPageFrameMap((uint8_t *)PAGEFRAME_MAP_BASE, 0x400);

    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Initialized Task Struct -> PID: ");
    printHexNumber(COLOR_GREEN, (cursorRow - 1), 38, currentPid);
    
    initializePageTables(currentPid);
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Initialized Page Directory and Page Table -> PID: ");
    printHexNumber(COLOR_GREEN, (cursorRow - 1), 56, currentPid);
    
    contextSwitch(currentPid); 
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Paging Enabled");

    remapPIC(INTERRUPT_MASK_ALL_ENABLED, INTERRUPT_MASK_ALL_ENABLED);
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Programmable Interrupt Controller (PIC) Remapping Complete");

    //zero memory for the new IDT
    fillMemory((uint8_t *)INTERRUPT_DESC_TABLE, (uint8_t)0x0, PAGE_SIZE);
    fillMemory((uint8_t *)(INTERRUPT_DESC_TABLE + PAGE_SIZE), (uint8_t)0x0, PAGE_SIZE);
    loadIDT((uint8_t *)INTERRUPT_DESC_TABLE);
    loadIDTR((uint8_t *)INTERRUPT_DESC_TABLE, (uint8_t *)INTERRUPT_DESC_TABLE_REG);
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Interrupt Descriptor Table (IDT) Setup Complete");

    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Kernel Initialization Complete");
    cursorRow++;

    createOpenFileTable((uint8_t *)OPEN_FILE_TABLE);
    storeValueAtMemLoc(RUNNING_PID_LOC, currentPid);
    
    enableInterrupts();

    printLogo(20);

    printString(COLOR_LIGHT_BLUE, cursorRow++, 0, (uint8_t *)"Ready to load Shell into PID:   ....");
    printHexNumber(COLOR_GREEN, (cursorRow - 1), 30, currentPid);
}

void logonPrompt()
{    
    if (!fsFindFile((uint8_t *)"mpass", (uint8_t *)((int)KERNEL_HASH_LOC + 0x500)))
    {
        panic((uint8_t *)"kernel.cpp -> Cannot find mpass file");
    }

    loadFileFromInodeStruct((KERNEL_HASH_LOC + 0x500), KERNEL_HASH_LOC);
    cursorRow = 17;
    printString(COLOR_WHITE, cursorRow++, 0, (uint8_t *)"Type Root password and press <enter> to launch shell:");    
    printString(COLOR_LIGHT_BLUE, cursorRow+1, 2, (uint8_t *)"$");

    readCommand(bufferMem, cursorMemory);

    while (*(uint32_t *)KERNEL_HASH_LOC != stringHash((uint8_t *)COMMAND_BUFFER))
    {
        cursorRow++;
        printString(COLOR_RED, 12, 0, (uint8_t *)"Wrong Password!");
        printString(COLOR_RED, 13, 0, (uint8_t *)"Press <enter> to try again....");  
        fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, 0x200);   
        readCommand(bufferMem, cursorMemory);
    
    }
}

void loadShell()
{
    if (!requestSpecificPage(currentPid, (uint8_t *)(STACK_PAGE - PAGE_SIZE), PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"kernel.cpp -> STACK_PAGE - PAGE_SIZE page request");
    }

    if (!requestSpecificPage(currentPid, (uint8_t *)(STACK_PAGE), PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"kernel.cpp -> STACK_PAGE page request");
    }

    if (!requestSpecificPage(currentPid, (uint8_t *)(USER_HEAP), PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"kernel.cpp -> USER_HEAP page request");
    }

    cursorRow++;
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Loading binary to Temp File Storage");

    if (!requestSpecificPage(currentPid, USER_TEMP_INODE_LOC, PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"kernel.cpp -> USER_TEMP_INODE_LOC page request");
    }

    if (!fsFindFile((uint8_t *)"shell2", USER_TEMP_INODE_LOC))
    {
        panic((uint8_t *)"kernel.cpp -> Cannot find shell in root directory");
    }

    struct inode *Inode = (struct inode*)USER_TEMP_INODE_LOC;
    uint32_t pagesNeedForTmpBinary = ceiling(Inode->i_size, PAGE_SIZE);

    //request pages for temporary file storage to load raw ELF file
    for (uint32_t tempFileLoc = 0; tempFileLoc < (pagesNeedForTmpBinary * PAGE_SIZE); tempFileLoc = tempFileLoc + PAGE_SIZE)
    {
        if (!requestSpecificPage(currentPid, (USER_TEMP_FILE_LOC + tempFileLoc), PG_USER_PRESENT_RW))
        {
            clearScreen();
            printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
            panic((uint8_t *)"kernel.cpp -> USER_TEMP_FILE_LOC page request");
        }
    }
    
    loadFileFromInodeStruct(USER_TEMP_INODE_LOC, USER_TEMP_FILE_LOC); 
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Raw binary loaded to 0x31000");
    
    struct elfHeader *ELFHeader = (struct elfHeader*)USER_TEMP_FILE_LOC;
    struct pHeader *ProgHeaderTextSegment, *ProgHeaderDataSegment;
    ProgHeaderTextSegment = (struct pHeader*)((uint8_t * )(int)ELFHeader + ELFHeader->e_phoff + ELF_PROGRAM_HEADER_SIZE);
    ProgHeaderDataSegment = (struct pHeader*)((uint8_t * )(int)ELFHeader + (ELFHeader->e_phoff + (ELF_PROGRAM_HEADER_SIZE * 2)));

    uint32_t totalTextSegmentPagesNeeded = ceiling(ProgHeaderTextSegment->p_memsz, PAGE_SIZE);
    uint32_t totalDataSegmentPagesNeeded = ceiling(ProgHeaderDataSegment->p_memsz, PAGE_SIZE);

    // request enough pages at the required virtual address to load and run binary
    for (uint32_t tempFileLoc = 0; tempFileLoc < ((totalTextSegmentPagesNeeded + totalDataSegmentPagesNeeded) * PAGE_SIZE); tempFileLoc = tempFileLoc + PAGE_SIZE)
    {
        if (!requestSpecificPage(currentPid, (uint8_t *)(ProgHeaderTextSegment->p_vaddr + tempFileLoc), PG_USER_PRESENT_RW))
        {
            clearScreen();
            printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
            panic((uint8_t *)"kernel.cpp -> USERPROG TEXT and DATA VADDR page requests");
        }

    }
    
    loadElfFile(USER_TEMP_FILE_LOC);
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Binary loaded from 0x31000 to desired location");
}

void launchShell()
{
    struct elfHeader *ELFHeaderLaunch = (struct elfHeader*)USER_TEMP_FILE_LOC;

    // RUNNING_PID_LOC will store some information for the currently running process, such as
    // what its PID is
    storeValueAtMemLoc(RUNNING_PID_LOC, currentPid);
    printString(COLOR_GREEN, cursorRow++, 0, (uint8_t *)"   -> Switching to Ring 3 and launching binary");

    // setting current process state to running
    updateTaskState(currentPid, PROC_RUNNING);
    setSystemTimer(SYSTEM_INTERRUPTS_PER_SECOND);

    loadTaskRegister(0x28);
    switchToRing3LaunchBinary((uint8_t *)ELFHeaderLaunch->e_entry);
}


void main()
{
    kInit();

    // Disabling logon prompt to save time while iterating through code.
    // logonPrompt();
    
    loadShell();
    launchShell();

    panic((uint8_t *)"kernel.cpp -> Unable to launch shell, returned to kernel.cpp");
}