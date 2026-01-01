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
#include "schedule.h"
#include "sound.h"


uint32_t returnedArgument = 0;
uint32_t totalInterruptCount = 0;
uint32_t systemTimerInterruptCount = 0;
uint32_t keyboardInterruptCount = 0;
uint32_t otherInterruptCount = 0;
uint32_t currentFileDescriptor = 0;
uint32_t uptimeSeconds = 0;
uint32_t uptimeMinutes = 0;
struct task *SysHandlerTask;
struct task *newSysHandlerTask;
uint32_t sysHandlertaskStructLocation;
uint32_t newSysHandlertaskStructLocation;
uint32_t returnedPid;

void sysSound(struct soundParameter *SoundParameter)
{
    generateTone(SoundParameter->frequency);

    for (uint32_t iterations=0; iterations <= SoundParameter->duration; iterations++)
    {
        sysWaitOneInterrupt();
    }

 	stopTone();
}

void sysBeep()
{
    generateTone(1000);
    sysWaitOneInterrupt();
 	stopTone();
}

void sysOpen(struct fileParameter *FileParameter, uint32_t currentPid)
{
    uint8_t *newBinaryFilenameLoc = kMalloc(currentPid, FileParameter->fileNameLength);
    strcpyRemoveNewline(newBinaryFilenameLoc, FileParameter->fileName);

    if (FileParameter->requestedPermissions == RDWRITE)
    {
        if (!fileAvailableToBeLocked((uint8_t *)OPEN_FILE_TABLE, returnInodeofFileName(newBinaryFilenameLoc)))
        {
            printString(COLOR_RED, 3, 2, (uint8_t *)"File locked by another process!");
            sysBeep();
            sysWait();
        }
    }

    uint32_t taskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    struct task *Task = (struct task*)taskStructLocation;

    if (Task->nextAvailableFileDescriptor >= MAX_FILE_DESCRIPTORS) 
    {
        return; // reached the max open files
    }
    
    uint8_t *inodePage = requestAvailablePage(currentPid, PG_USER_PRESENT_RW);

    if (inodePage == 0)
    {
        panic((uint8_t *)"Syscalls.cpp:sysOpen() -> open request available page is null");
    }

    if (!fsFindFile(newBinaryFilenameLoc, (uint8_t *)inodePage))
    {
        printString(COLOR_RED, 2, 5, (uint8_t *)"File not found!");
        sysBeep();
        return;
    }

    struct inode *Inode = (struct inode*)inodePage;
    uint32_t pagesNeedForTmpBinary = ceiling(Inode->i_size, PAGE_SIZE);

    uint8_t *requestedBuffer = findBuffer(currentPid, pagesNeedForTmpBinary, PG_USER_PRESENT_RW);

    //request block of pages for temporary file storage to load file based on first available page above
    for (uint32_t pageCount = 0; pageCount < pagesNeedForTmpBinary; pageCount++)
    {                
        if (!requestSpecificPage(currentPid, (uint8_t *)((uint32_t)requestedBuffer + (pageCount * PAGE_SIZE)), PG_USER_PRESENT_RW))
        {
            clearScreen();
            printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
            panic((uint8_t *)"syscalls.cpp -> USER_TEMP_FILE_LOC page request");
        }

        // Zero each page in case it has been used previously
        fillMemory((uint8_t *)((uint32_t)requestedBuffer + (pageCount * PAGE_SIZE)), 0x0, PAGE_SIZE);
    }

    loadFileFromInodeStruct((uint8_t *)inodePage, requestedBuffer);

    Task->fileDescriptor[Task->nextAvailableFileDescriptor] = (openFileTableEntry *)insertOpenFileTableEntry((uint8_t *)OPEN_FILE_TABLE, (int)(returnInodeofFileName(newBinaryFilenameLoc)), currentPid, requestedBuffer, pagesNeedForTmpBinary, newBinaryFilenameLoc, 0, 0);
    
    if (FileParameter->requestedPermissions == RDWRITE)
    {
        if (!lockFile((uint8_t *)OPEN_FILE_TABLE, currentPid, returnInodeofFileName(newBinaryFilenameLoc)))
        {
            printString(COLOR_RED, 4, 2, (uint8_t *)"Unable to acquire file lock!");
            sysBeep();
        }
    }

    struct openBufferTable *openBufferTable = (struct openBufferTable*)OPEN_BUFFER_TABLE;

    freePage(currentPid, inodePage);
    storeValueAtMemLoc(CURRENT_FILE_DESCRIPTOR, ((int)Task->nextAvailableFileDescriptor));
    storeValueAtMemLoc((uint8_t *)&openBufferTable->buffers[Task->nextAvailableFileDescriptor], (int)requestedBuffer);

    Task->nextAvailableFileDescriptor++;

}

void sysWrite(uint32_t fileDescriptorToPrint, uint32_t currentPid)
{
    uint32_t taskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    struct task *Task = (struct task*)taskStructLocation;

    if ((uint32_t)Task->fileDescriptor[fileDescriptorToPrint] == 0x0)
    {
        printString(COLOR_RED, 3, 5, (uint8_t *)"No such file descriptor!");
        sysBeep();
        sysWait();
        sysWait();
        clearScreen();
    }
    else 
    {
        struct openFileTableEntry *OpenFileTableEntry = (openFileTableEntry*)(uint32_t)Task->fileDescriptor[fileDescriptorToPrint];
        printString(COLOR_WHITE, 0, 0, (uint8_t *)(OpenFileTableEntry->userspaceBuffer));        
    }

}

void sysClose(uint32_t fileDescriptor, uint32_t currentPid)
{
    
    // ASSIGNMENT 4 TO DO

}

void sysPs(uint32_t currentPid)
{
    uint32_t currentTaskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    struct task *currentTask = (struct task*)currentTaskStructLocation;
        
    uint32_t lastUsedPid = 0;
    uint32_t nextAvailPid = 0;
    uint32_t cursor = 1;
    uint32_t taskStructNumber = 0;

    uint32_t taskStructLocation = PROCESS_TABLE_LOC; 

    if (uint8_t *totalFramesCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor++, 2, (uint8_t *)"System Frames Used: ");
        itoa(totalFramesUsed((uint8_t *)PAGEFRAME_MAP_BASE), totalFramesCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 30, totalFramesCount); 
        kFree(totalFramesCount);
    }

    if (uint8_t *totalMemCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor-1, 38, (uint8_t *)"System Memory Used (Bytes): ");
        itoa((totalFramesUsed((uint8_t *)PAGEFRAME_MAP_BASE) * 4096), totalMemCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 70, totalMemCount); 
        kFree(totalMemCount);
    }

    if (uint8_t *processFramesCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor++, 2, (uint8_t *)"Current PID Frames Used: ");
        itoa((processFramesUsed(currentPid, (uint8_t *)PAGEFRAME_MAP_BASE)), processFramesCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 30, processFramesCount);  
        kFree(processFramesCount);
    }

    if (uint8_t *processMemCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor-1, 38, (uint8_t *)"Current PID Mem Used (Bytes): ");
        itoa((processFramesUsed(currentPid, (uint8_t *)PAGEFRAME_MAP_BASE) * 4096), processMemCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 70, processMemCount); 
        kFree(processMemCount);
    }

    if (uint8_t *userHeapObjectCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor++, 2, (uint8_t *)"Current PID Heap Objects: ");
        itoa(countHeapObjects((uint8_t *)USER_HEAP), userHeapObjectCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 30, userHeapObjectCount); 
        kFree(userHeapObjectCount);
    }

    if (uint8_t *interruptCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor-1, 38, (uint8_t *)"Total System Interrupts: ");
        itoa(totalInterruptCount, interruptCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 70, interruptCount); 
        kFree(interruptCount);
    }

    if (uint8_t *timerCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor++, 38, (uint8_t *)"Total Timer Interrupts: ");
        itoa(systemTimerInterruptCount, timerCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 70, timerCount); 
        kFree(timerCount);
    }
    
    if (uint8_t *keyboardCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor++, 38, (uint8_t *)"Total Keyboard Interrupts: ");
        itoa(keyboardInterruptCount, keyboardCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 70, keyboardCount); 
        kFree(keyboardCount);
    }
    
    if (uint8_t *otherCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor++, 38, (uint8_t *)"All Other Interrupts: ");
        itoa(otherInterruptCount, otherCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 70, otherCount); 
        kFree(otherCount);
    }

    if (uint8_t *openFilesCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor-1, 2, (uint8_t *)"System-wide Open Files: ");
        itoa(totalOpenFiles((uint8_t *)OPEN_FILE_TABLE), openFilesCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 30, openFilesCount); 
        kFree(openFilesCount);
    }

    if (uint8_t *kernelHeapObjectCount = kMalloc(currentPid, sizeof(int)))
    {
        printString(COLOR_GREEN, cursor++, 38, (uint8_t *)"Kernel Heap Objects: ");
        itoa(countHeapObjects((uint8_t *)KERNEL_HEAP), kernelHeapObjectCount);
        printString(COLOR_LIGHT_BLUE, cursor-1, 70, kernelHeapObjectCount); 
        kFree(kernelHeapObjectCount);
    }

    uint8_t *psUpperLeftCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psUpperLeftCorner = ASCII_UPPERLEFT_CORNER;

    printString(COLOR_WHITE, cursor, 1, psUpperLeftCorner);

    uint8_t *psHorizontalLine = kMalloc(currentPid, sizeof(uint8_t));
    *psHorizontalLine = ASCII_HORIZONTAL_LINE;
    for (uint32_t columnPos = 2; columnPos < 78; columnPos++)
    {
        printString(COLOR_WHITE, cursor, columnPos, psHorizontalLine);
    }
    
    uint8_t *psVerticalLine = kMalloc(currentPid, sizeof(uint8_t));
    *psVerticalLine = ASCII_VERTICAL_LINE;
    
    uint8_t *psUpperRightCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psUpperRightCorner = ASCII_UPPERRIGHT_CORNER;

    uint8_t *psLowerLeftCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psLowerLeftCorner = ASCII_LOWERLEFT_CORNER;

    uint8_t *psLowerRightCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psLowerRightCorner = ASCII_LOWERRIGHT_CORNER;

    printString(COLOR_WHITE, (cursor), 78, psUpperRightCorner);

    printString(COLOR_RED, 8, 3, (uint8_t *)"Processes");

    cursor++;

    while (taskStructNumber < MAX_PROCESSES)
    {
        taskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * taskStructNumber);;
    
        struct task *Task = (struct task*)taskStructLocation;
        
        lastUsedPid = *(uint32_t *)(PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * taskStructNumber));

        if ((uint32_t)lastUsedPid == 0)
        {
            nextAvailPid = (taskStructNumber + 1);
            taskStructNumber++;
        }
        else
        {
            printString(COLOR_WHITE, (cursor++), 1, psVerticalLine);
            printString(COLOR_WHITE, (cursor-1), 2, (uint8_t *)"PID:");
            printHexNumber(COLOR_WHITE, (cursor -1), 7, (uint8_t)Task->pid);
            printString(COLOR_WHITE, (cursor-1), 10, psVerticalLine);
            printString(COLOR_WHITE, (cursor -1), 12, (uint8_t *)"Name:");
            printString(COLOR_WHITE, (cursor -1), 18, (uint8_t *)Task->binaryName);
            printString(COLOR_WHITE, (cursor-1), 31, psVerticalLine);
            printString(COLOR_WHITE, (cursor -1), 33, (uint8_t *)"State:");
            if ((uint8_t)Task->state == PROC_SLEEPING) {printString(COLOR_WHITE, (cursor -1), 40, (uint8_t *)"SLEEPING");}
            if ((uint8_t)Task->state == PROC_RUNNING) {printString(COLOR_WHITE, (cursor -1), 40, (uint8_t *)"RUNNING");}
            if ((uint8_t)Task->state == PROC_ZOMBIE) {printString(COLOR_WHITE, (cursor -1), 40, (uint8_t *)"ZOMBIE");}
            if ((uint8_t)Task->state == PROC_KILLED) {printString(COLOR_WHITE, (cursor -1), 40, (uint8_t *)"KILLED");}
            printString(COLOR_WHITE, (cursor-1), 49, psVerticalLine);
            printString(COLOR_WHITE, (cursor -1), 51, (uint8_t *)"PPID:");
            printHexNumber(COLOR_WHITE, (cursor -1), 57, (uint8_t)Task->ppid);    
            printString(COLOR_WHITE, (cursor-1), 60, psVerticalLine);  
            printString(COLOR_WHITE, (cursor -1), 62, (uint8_t *)"P:");
            printHexNumber(COLOR_WHITE, (cursor -1), 65, (uint8_t)Task->priority);
            
            if (uint8_t *runtimeMemLoc = kMalloc(currentPid, sizeof(uint32_t)))
            {
                printString(COLOR_WHITE, (cursor-1), 68, psVerticalLine);
                printString(COLOR_WHITE, (cursor -1), 70, (uint8_t *)"R:");
                itoa(((Task->runtime) / SYSTEM_INTERRUPTS_PER_SECOND), runtimeMemLoc);
                printString(COLOR_WHITE, (cursor -1), 73, runtimeMemLoc);
                printString(COLOR_WHITE, (cursor-1), 78, psVerticalLine); 
                kFree(runtimeMemLoc);
            }
           
            taskStructNumber++;   
        }
        if ((unsigned int)(currentTask->nextAvailableFileDescriptor -1) <= 2)
        {
            returnedArgument = 0x0;
        }
        else
        {
            returnedArgument = (uint32_t)(currentTask->nextAvailableFileDescriptor -1);

        }

    }
    printSystemUptime(uptimeSeconds, uptimeMinutes);

    printString(COLOR_WHITE, cursor, 1, psLowerLeftCorner);
    printString(COLOR_WHITE, cursor, 78, psLowerRightCorner);

    for (uint32_t columnPos = 2; columnPos < 78; columnPos++)
    {
        printString(COLOR_WHITE, cursor, columnPos, psHorizontalLine);
    }

    kFree(psHorizontalLine);
    kFree(psVerticalLine);
    kFree(psUpperLeftCorner);
    kFree(psUpperRightCorner);
    kFree(psLowerLeftCorner);
    kFree(psLowerRightCorner);

}

void sysForkExec(struct fileParameter *FileParameter, uint32_t currentPid)
{
    uint8_t *newBinaryFilenameLoc = kMalloc(currentPid, strlen(FileParameter->fileName)); 
    strcpyRemoveNewline(newBinaryFilenameLoc, FileParameter->fileName);

    if (!fsFindFile(newBinaryFilenameLoc, USER_TEMP_INODE_LOC))
    {
        printString(COLOR_RED, 2, 5, (uint8_t *)"File not found!");
        sysBeep();
        sysWait();
        sysWait();
        return;
    }
      
    uint32_t newPid = 0;

    newPid = initializeTask(currentPid, PROC_SLEEPING, STACK_START_LOC, newBinaryFilenameLoc, FileParameter->requestedRunPriority);
    initializePageTables(newPid);

    contextSwitch(newPid);

    if (!requestSpecificPage(newPid, (uint8_t *)(STACK_PAGE - PAGE_SIZE), PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"syscalls.cpp -> STACK_PAGE - PAGE_SIZE page request");
    }

    if (!requestSpecificPage(newPid, (uint8_t *)(STACK_PAGE), PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"syscalls.cpp -> STACK_PAGE page request");
    }

    if (!requestSpecificPage(newPid, (uint8_t *)(USER_HEAP), PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"syscalls.cpp -> USER_HEAP page request");
    }

    if (!requestSpecificPage(newPid, USER_TEMP_INODE_LOC, PG_USER_PRESENT_RW))
    {
        clearScreen();
        printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
        panic((uint8_t *)"syscalls.cpp -> USER_TEMP_INODE_LOC page request");
    }
   
    // we do this function twice. First one (above) to make sure the file exists.
    // this second one (below) is because there was a context switch and a different
    // userspace.
    fsFindFile(newBinaryFilenameLoc, USER_TEMP_INODE_LOC);

    struct inode *Inode = (struct inode*)USER_TEMP_INODE_LOC;

    uint32_t pagesNeedForTmpBinary = ceiling(Inode->i_size, PAGE_SIZE);

    //request pages for temporary file storage to load raw ELF file
    for (uint32_t tempFileLoc = 0; tempFileLoc < (pagesNeedForTmpBinary * PAGE_SIZE); tempFileLoc = tempFileLoc + PAGE_SIZE)
    {
        if (!requestSpecificPage(newPid, (USER_TEMP_FILE_LOC + tempFileLoc), PG_USER_PRESENT_RW))
        {
            clearScreen();
            printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
            panic((uint8_t *)"syscalls.cpp -> USER_TEMP_FILE_LOC page request");
        }
    }

    loadFileFromInodeStruct(USER_TEMP_INODE_LOC, USER_TEMP_FILE_LOC); 

    if (*(uint32_t *)USER_TEMP_FILE_LOC != MAGIC_ELF)
    {
        printString(COLOR_RED, 2, 5, (uint8_t *)"File is not an ELF file!");
        sysBeep();
        sysWait();
        sysWait();
        sysKill(newPid);
        contextSwitch(currentPid);
        return;
    }

    struct elfHeader *ELFHeader = (struct elfHeader*)USER_TEMP_FILE_LOC;
    struct pHeader *ProgHeaderTextSegment, *ProgHeaderDataSegment;
    ProgHeaderTextSegment = (struct pHeader*)((uint8_t *)(uint32_t)ELFHeader + ELFHeader->e_phoff + ELF_PROGRAM_HEADER_SIZE);
    ProgHeaderDataSegment = (struct pHeader*)((uint8_t *)(uint32_t)ELFHeader + (ELFHeader->e_phoff + (ELF_PROGRAM_HEADER_SIZE *2)));

    uint32_t totalTextSegmentPagesNeeded = ceiling(ProgHeaderTextSegment->p_memsz, PAGE_SIZE);
    uint32_t totalDataSegmentPagesNeeded = ceiling(ProgHeaderDataSegment->p_memsz, PAGE_SIZE);

    // request enough pages at the required virtual address to load and run binary
    for (uint32_t tempFileLoc = 0; tempFileLoc < ((totalTextSegmentPagesNeeded + totalDataSegmentPagesNeeded) * PAGE_SIZE); tempFileLoc = tempFileLoc + PAGE_SIZE)
    {
        if (!requestSpecificPage(newPid, (uint8_t *)(ProgHeaderTextSegment->p_vaddr + tempFileLoc), PG_USER_PRESENT_RW))
        {
            clearScreen();
            printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
            panic((uint8_t *)"syscalls.cpp -> USERPROG VADDR page requests");
        }

    }
    
    loadElfFile(USER_TEMP_FILE_LOC);

    struct elfHeader *ELFHeaderLaunch = (struct elfHeader*)USER_TEMP_FILE_LOC;

    updateTaskState(currentPid, PROC_SLEEPING);
    updateTaskState(newPid, PROC_RUNNING);

    // Letting the new process know its pid
    storeValueAtMemLoc(RUNNING_PID_LOC, newPid);

    enableInterrupts();
    
    switchToRing3LaunchBinary((uint8_t *)ELFHeaderLaunch->e_entry);

}

void sysExit(uint32_t currentPid)
{
    if (currentPid == 1)
    {
        return; // PID 1 cannot exit
    }
    
    uint32_t currentTaskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    
    struct task *currentTask = (struct task*)currentTaskStructLocation;

    updateTaskState(currentPid, PROC_ZOMBIE);
    updateTaskState(currentTask->ppid, PROC_RUNNING);

    closeAllFiles((uint8_t *)OPEN_FILE_TABLE, currentPid);

    // Letting the new process know its pid
    storeValueAtMemLoc(RUNNING_PID_LOC, (currentTask->ppid));

    // switch CR3 back to parent's page directory
    contextSwitch(currentTask->ppid);

}

void sysFree(uint32_t currentPid)
{    
    
    // ASSIGNMENT 4 TO DO

}

void sysMmap(uint32_t currentPid)
{    
    uint32_t returnedPage = (unsigned int)requestAvailablePage(currentPid, PG_USER_PRESENT_RW);
    
    if (returnedPage == 0)
    {
        panic((uint8_t *)"syscalls.cpp -> MMAP - when requesting available page.");
    }

    storeValueAtMemLoc(RETURNED_MMAP_PAGE_LOC, returnedPage);
}

void sysKill(uint32_t pidToKill)
{   
    
    // ASSIGNMENT 4 TO DO

}

void sysSwitch(uint32_t pidToSwitchTo, uint32_t currentPid)
{
    
    // ASSIGNMENT 4 TO DO
    
}

void sysMemDump(uint32_t memAddressToDump)
{
    clearScreen();

    for (uint32_t row = 1; row < 17; row++ )
    {
        for (uint32_t column = 1; column < 48; column++)
        {
            printHexNumber(COLOR_WHITE, row, column, *(uint8_t *)memAddressToDump);
            memAddressToDump++;
            column++;
            column++;
        }
    }

}

void sysUptime()
{
    printSystemUptime(uptimeSeconds, uptimeMinutes);
}

void sysSwitchToParent(uint32_t currentPid)
{
    currentPid = readValueFromMemLoc(RUNNING_PID_LOC);
    
    uint32_t currentTaskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    
    struct task *currentTask = (struct task*)currentTaskStructLocation;

    updateTaskState(currentTask->pid, PROC_SLEEPING);
    updateTaskState(currentTask->ppid, PROC_RUNNING);

    // Letting the new process know its pid
    storeValueAtMemLoc(RUNNING_PID_LOC, currentTask->ppid);

    contextSwitch(currentTask->ppid);
}

void sysWait()
{
    // Waits one second and returns
    
    enableInterrupts();
    
    uint32_t futureSystemTimerInterruptCount = (systemTimerInterruptCount + SYSTEM_INTERRUPTS_PER_SECOND);
    
    while (systemTimerInterruptCount <= futureSystemTimerInterruptCount)
    {

    } 
    disableInterrupts();
}

void sysWaitOneInterrupt()
{
    // Waits one second and returns
    
    enableInterrupts();
    
    uint32_t futureSystemTimerInterruptCount = (systemTimerInterruptCount + 1);
    
    while (systemTimerInterruptCount <= futureSystemTimerInterruptCount)
    {

    } 
    disableInterrupts();
}

void sysDirectory(uint32_t currentPid)
{
    uint32_t cursor = 0;
    
    fillMemory((uint8_t *)KERNEL_TEMP_INODE_LOC, 0x0, (PAGE_SIZE * 2));
    readBlock(ROOTDIR_BLOCK, (uint8_t *)KERNEL_TEMP_INODE_LOC);
    readBlock(ROOTDIR_BLOCK +1, (uint8_t *)(KERNEL_TEMP_INODE_LOC + BLOCK_SIZE));
    readBlock(ROOTDIR_BLOCK +2, (uint8_t *)(KERNEL_TEMP_INODE_LOC + (BLOCK_SIZE *2)));
    readBlock(ROOTDIR_BLOCK +3, (uint8_t *)(KERNEL_TEMP_INODE_LOC + (BLOCK_SIZE *3)));

    struct directoryEntry *DirectoryEntry = (directoryEntry*)(KERNEL_TEMP_INODE_LOC);
    struct ext2SuperBlock *Ext2SuperBlock = (ext2SuperBlock*)SUPERBLOCK_LOC;

    printString(COLOR_WHITE, cursor++, 2, (uint8_t *)"Root Dir");

    uint8_t *psUpperLeftCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psUpperLeftCorner = ASCII_UPPERLEFT_CORNER;

    uint8_t *psHorizontalLine = kMalloc(currentPid, sizeof(uint8_t));
    *psHorizontalLine = ASCII_HORIZONTAL_LINE;
    cursor++;
    for (uint32_t columnPos = 3; columnPos < 77; columnPos++)
    {
        printString(COLOR_WHITE, cursor, columnPos, psHorizontalLine);
    }
    
    uint8_t *psVerticalLine = kMalloc(currentPid, sizeof(uint8_t));
    *psVerticalLine = ASCII_VERTICAL_LINE;
    
    uint8_t *psUpperRightCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psUpperRightCorner = ASCII_UPPERRIGHT_CORNER;

    uint8_t *psLowerLeftCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psLowerLeftCorner = ASCII_LOWERLEFT_CORNER;

    uint8_t *psLowerRightCorner = kMalloc(currentPid, sizeof(uint8_t));
    *psLowerRightCorner = ASCII_LOWERRIGHT_CORNER;

    cursor++;

    printString(COLOR_WHITE, 2, 2, psUpperLeftCorner);
    printString(COLOR_RED, 2, 4, (uint8_t *)"In");
    printString(COLOR_RED, 2, 8, (uint8_t *)"Filename");
    printString(COLOR_RED, 2, 20, (uint8_t *)"Bytes");
    printString(COLOR_RED, 2, 27, (uint8_t *)"Unix Create");
    printString(COLOR_RED, 2, 40, (uint8_t *)"Unix Modify");
    printString(COLOR_RED, 2, 53, (uint8_t *)"Type");
    printString(COLOR_RED, 2, 59, (uint8_t *)"Other");
    printString(COLOR_RED, 2, 65, (uint8_t *)"Group");
    printString(COLOR_RED, 2, 71, (uint8_t *)"User");
    printString(COLOR_WHITE, 2, 77, psUpperRightCorner);

    while ((int)DirectoryEntry->directoryInode != 0)
    {
        uint8_t *directoryFilename = kMalloc(currentPid, strlen((uint8_t *)&DirectoryEntry->fileName));
        uint8_t *directoryFileSize = kMalloc(currentPid, sizeof(uint32_t));
        uint8_t *fileCreateTimeUnix = kMalloc(currentPid, sizeof(uint32_t));
        //uint8_t *fileCreateTimeUnixYear = kMalloc(currentPid, sizeof(uint32_t));
        //uint8_t *fileCreateTimeUnixDay = kMalloc(currentPid, sizeof(uint32_t));
        uint8_t *fileModifyTimeUnix = kMalloc(currentPid, sizeof(uint32_t));
        struct time* Time = (struct time*)kMalloc(currentPid, sizeof(struct time));

        printString(COLOR_WHITE, (cursor++), 2, psVerticalLine);
        printHexNumber(COLOR_LIGHT_BLUE, (cursor-1), 4, (uint8_t)DirectoryEntry->directoryInode);

        memoryCopy((uint8_t *)&DirectoryEntry->fileName, directoryFilename, 6);
        printString(COLOR_WHITE, (cursor-1), 8, directoryFilename);
            
        fsFindFile(directoryFilename, EXT2_TEMP_INODE_STRUCTS);
        struct inode *Inode = (struct inode*)EXT2_TEMP_INODE_STRUCTS;

        itoa(Inode->i_size, directoryFileSize);
        printString(COLOR_LIGHT_BLUE, cursor-1, 20, directoryFileSize);

        itoa(Inode->i_mtime, fileCreateTimeUnix);
        printString(COLOR_GREEN, cursor-1, 27, fileCreateTimeUnix);

        // Time = convertFromUnixTime(Inode->i_ctime);
        // itoa(Time->year, fileCreateTimeUnixYear);
        // printString(COLOR_GREEN, cursor-1, 27, fileCreateTimeUnixYear);
        // printString(COLOR_GREEN, cursor-1, 31, (uint8_t*)"-");
        // itoa(Time->dayOfYear, fileCreateTimeUnixDay);
        // printString(COLOR_GREEN, cursor-1, 32, fileCreateTimeUnixDay);

        itoa(Inode->i_mtime, fileModifyTimeUnix);
        printString(COLOR_GREEN, cursor-1, 40, fileModifyTimeUnix);

        printString(COLOR_GREEN, cursor-1, 53, directoryEntryTypeTranslation((Inode->i_mode >> 12) & 0x000F));

        //Other Permissions
        printString(COLOR_GREEN, cursor-1, 60, octalTranslation(((Inode->i_mode >> 6) & 0b0000000000000111)));

        //Group Permissions
        printString(COLOR_GREEN, cursor-1, 66, octalTranslation(((Inode->i_mode >> 3) & 0b0000000000000111)));

        //User Permissions
        printString(COLOR_GREEN, cursor-1, 72, octalTranslation((Inode->i_mode & 0b0000000000000111)));

        printString(COLOR_WHITE, (cursor-1), 77, psVerticalLine);

        kFree(directoryFilename);
        kFree(directoryFileSize);
        kFree(fileCreateTimeUnix);
        // kFree(fileCreateTimeUnixYear);
        // kFree(fileCreateTimeUnixDay);
        kFree(fileModifyTimeUnix);
        kFree((uint8_t *)Time);

        DirectoryEntry = (directoryEntry *)((uint32_t)DirectoryEntry + DirectoryEntry->recLength);
    }

    printString(COLOR_WHITE, cursor, 2, psLowerLeftCorner);
    printString(COLOR_WHITE, cursor, 77, psLowerRightCorner);

    for (uint32_t columnPos = 3; columnPos < 77; columnPos++)
    {
        printString(COLOR_WHITE, cursor, columnPos, psHorizontalLine);
    }

    cursor++;

    uint8_t *volumeSizeBytes = kMalloc(currentPid, sizeof(uint32_t));
    itoa((Ext2SuperBlock->sb_total_blocks) * BLOCK_SIZE, volumeSizeBytes);
    printString(COLOR_GREEN, 0, 13, (uint8_t *)"FS Size:");
    printString(COLOR_LIGHT_BLUE, 0, 22, volumeSizeBytes);
    kFree(volumeSizeBytes);

    uint8_t *totalBlocksUsed = kMalloc(currentPid, sizeof(uint32_t));
    itoa(readTotalBlocksUsed(), totalBlocksUsed);
    printString(COLOR_GREEN, 1, 13, (uint8_t *)"Total Blocks Used:");
    printString(COLOR_LIGHT_BLUE, 1, 32, totalBlocksUsed);
    kFree(totalBlocksUsed);

    uint8_t *volumeRemainingBytes = kMalloc(currentPid, sizeof(uint32_t));
    itoa((Ext2SuperBlock->sb_total_blocks - readTotalBlocksUsed()) * BLOCK_SIZE, volumeRemainingBytes);
    printString(COLOR_GREEN, 0, 30, (uint8_t *)"Free:");
    printString(COLOR_LIGHT_BLUE, 0, 36, volumeRemainingBytes);
    kFree(volumeRemainingBytes);

    uint8_t *volumeTotalInodes = kMalloc(currentPid, sizeof(uint32_t));
    itoa((Ext2SuperBlock->sb_total_inodes) * BLOCK_SIZE, volumeTotalInodes);
    printString(COLOR_GREEN, 0, 44, (uint8_t *)"Total Inodes:");
    printString(COLOR_LIGHT_BLUE, 0, 58, volumeTotalInodes);
    kFree(volumeTotalInodes);

    uint8_t *nextAvailableInode = kMalloc(currentPid, sizeof(uint32_t));
    itoa(readNextAvailableInode(), nextAvailableInode);
    printString(COLOR_GREEN, 1, 44, (uint8_t *)"Next Avail Inode:");
    printString(COLOR_LIGHT_BLUE, 1, 62, nextAvailableInode);
    kFree(nextAvailableInode);

    uint8_t *volumeRemainingInodes = kMalloc(currentPid, sizeof(uint32_t));
    itoa((Ext2SuperBlock->sb_total_unallocated_inodes) * BLOCK_SIZE, volumeRemainingInodes);
    printString(COLOR_GREEN, 0, 65, (uint8_t *)"Free:");
    printString(COLOR_LIGHT_BLUE, 0, 71, volumeRemainingInodes);
    kFree(volumeRemainingInodes);

    kFree(psHorizontalLine);
    kFree(psVerticalLine);
    kFree(psUpperLeftCorner);
    kFree(psUpperRightCorner);
    kFree(psLowerLeftCorner);
    kFree(psLowerRightCorner);
}

void sysToggleScheduler()
{
    struct kernelConfiguration *KernelConfiguration = (struct kernelConfiguration*)KERNEL_CONFIGURATION;

    if (KernelConfiguration->runScheduler == 0)
    {
        KernelConfiguration->runScheduler = 1;
    }
    else if (KernelConfiguration->runScheduler == 1)
    {
        KernelConfiguration->runScheduler = 0;
    }
}
void sysShowOpenFiles(uint32_t currentPid)
{
    uint32_t currentTaskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    struct task *currentTask = (struct task*)currentTaskStructLocation;
    
    struct openFileTableEntry *OpenFileTableEntry = (struct openFileTableEntry*)OPEN_FILE_TABLE;
    uint32_t column = 0;
    uint32_t startingFileDescriptor = 3;

    while (startingFileDescriptor < MAX_FILE_DESCRIPTORS)
    {
        if (currentTask->fileDescriptor[startingFileDescriptor] != 0x0)
        {
            uint32_t addressOfOpenFile = (uint32_t)currentTask->fileDescriptor[startingFileDescriptor];
            
            OpenFileTableEntry = (struct openFileTableEntry*)addressOfOpenFile;
        
            printHexNumber(COLOR_LIGHT_BLUE, 23, column, startingFileDescriptor);
            if (OpenFileTableEntry->openedByPid == currentPid && OpenFileTableEntry->lockedForWriting == FILE_LOCKED)
            {
                printCharacter(COLOR_WHITE, 23, (column+2), (uint8_t *)":");
                printString(COLOR_LIGHT_BLUE, 23, (column+3), (uint8_t *)"RW");
            }
            else if (OpenFileTableEntry->openedByPid == currentPid && OpenFileTableEntry->lockedForWriting != FILE_LOCKED)
            {
                printCharacter(COLOR_WHITE, 23, (column+2), (uint8_t *)":");
                printString(COLOR_LIGHT_BLUE, 23, (column+3), (uint8_t *)"RO");
            }
            column = column + 6;
            printString(COLOR_WHITE, 23, column, (uint8_t *)(uint32_t)OpenFileTableEntry->fileName);
            column = column + 12; 
        }

        startingFileDescriptor++;
    }
}

void sysCreate(struct fileParameter *FileParameter, uint32_t currentPid)
{
    createFile(FileParameter->fileName, currentPid, FileParameter->fileDescriptor);
}

void sysDelete(struct fileParameter *FileParameter, uint32_t currentPid)
{
    deleteFile(FileParameter->fileName, currentPid);
}

void sysOpenEmpty(struct fileParameter *FileParameter, uint32_t currentPid)
{
    uint8_t *newBinaryFilenameLoc = kMalloc(currentPid, FileParameter->fileNameLength);
    strcpyRemoveNewline(newBinaryFilenameLoc, FileParameter->fileName);

    uint32_t taskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    struct task *Task = (struct task*)taskStructLocation;

    if (Task->nextAvailableFileDescriptor >= MAX_FILE_DESCRIPTORS) 
    {
        return; // reached the max open files
    }

    uint32_t pagesNeedForTmpBinary = FileParameter->requestedSizeInPages;

    uint8_t *requestedBuffer = findBuffer(currentPid, pagesNeedForTmpBinary, PG_USER_PRESENT_RW);

    //request block of pages for temporary file storage to load file based on first available page above
    for (uint32_t pageCount = 0; pageCount < pagesNeedForTmpBinary; pageCount++)
    {                
        if (!requestSpecificPage(currentPid, (uint8_t *)((uint32_t)requestedBuffer + (pageCount * PAGE_SIZE)), PG_USER_PRESENT_RW))
        {
            clearScreen();
            printString(COLOR_RED, 2, 2, (uint8_t *)"Requested page is not available");
            panic((uint8_t *)"syscalls.cpp -> USER_TEMP_FILE_LOC page request");
        }

        // Zero each page in case it has been used previously
        fillMemory((uint8_t *)((uint32_t)requestedBuffer + (pageCount * PAGE_SIZE)), 0x0, PAGE_SIZE);
    }

    Task->fileDescriptor[Task->nextAvailableFileDescriptor] = (openFileTableEntry *)insertOpenFileTableEntry((uint8_t *)OPEN_FILE_TABLE, (uint32_t)(0xFFFF), currentPid, requestedBuffer, pagesNeedForTmpBinary, newBinaryFilenameLoc, 0, 0);

    struct openBufferTable *openBufferTable = (struct openBufferTable*)OPEN_BUFFER_TABLE;

    storeValueAtMemLoc(CURRENT_FILE_DESCRIPTOR, ((uint32_t)Task->nextAvailableFileDescriptor));
    storeValueAtMemLoc((uint8_t *)&openBufferTable->buffers[Task->nextAvailableFileDescriptor], (int)requestedBuffer);

    createFile(newBinaryFilenameLoc, currentPid, Task->nextAvailableFileDescriptor);

    sysClose(Task->nextAvailableFileDescriptor, currentPid);

}

void syscallHandler()
{     
    // This is very sensitive (guru code below).
    // Don't Touch!
    asm volatile ("pusha\n\t");
   
    uint32_t syscallNumber;
    uint32_t arg1;
    uint32_t currentPid;
    asm volatile ("movl %%eax, %0\n\t" : "=r" (syscallNumber) : );
    asm volatile ("movl %%ebx, %0\n\t" : "=r" (arg1) : );
    asm volatile ("movl %%ecx, %0\n\t" : "=r" (currentPid) : );

    sysHandlertaskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    SysHandlerTask = (struct task*)sysHandlertaskStructLocation;

    // These manually grab from the stack are very sensitive to local variables.
    // The stack prior to iret should be:
    // (top to bottom) -> EIP -> CS -> UKNOWN (EAX) -> ESP

    asm volatile ("movl %esp, %edx\n\t");
    asm volatile ("add $8, %edx\n\t");
    asm volatile ("movl (%edx), %edx\n\t");
    asm volatile ("movl %%edx, %0\n\t" : "=r" ((uint32_t)(SysHandlerTask->ebp)) : );

    asm volatile ("movl %esp, %edx\n\t");
    asm volatile ("add $60, %edx\n\t");
    asm volatile ("movl (%edx), %edx\n\t");
    asm volatile ("movl %%edx, %0\n\t" : "=r" ((uint32_t)(SysHandlerTask->eip)) : );

    asm volatile ("movl %esp, %edx\n\t");
    asm volatile ("add $64, %edx\n\t");
    asm volatile ("movl (%edx), %edx\n\t");
    asm volatile ("mov %%dx, %0\n\t" : "=r" ((uint16_t)(SysHandlerTask->cs)) : );

    asm volatile ("movl %esp, %edx\n\t");
    asm volatile ("add $68, %edx\n\t");
    asm volatile ("movl (%edx), %edx\n\t");
    asm volatile ("movl %%edx, %0\n\t" : "=r" ((uint32_t)(SysHandlerTask->eax)) : );

    asm volatile ("movl %esp, %edx\n\t");
    asm volatile ("add $72, %edx\n\t");
    asm volatile ("movl (%edx), %edx\n\t");
    asm volatile ("movl %%edx, %0\n\t" : "=r" ((uint32_t)(SysHandlerTask->esp)) : );
  
         if ((unsigned int)syscallNumber == SYS_SOUND)                  { sysSound((struct soundParameter *)arg1); }
    else if ((unsigned int)syscallNumber == SYS_OPEN)                   { sysOpen((struct fileParameter *)arg1, currentPid); }
    else if ((unsigned int)syscallNumber == SYS_WRITE)                  { sysWrite(arg1, currentPid); }
    else if ((unsigned int)syscallNumber == SYS_CLOSE)                  { sysClose(arg1, currentPid); }
    else if ((unsigned int)syscallNumber == SYS_PS)                     { sysPs(currentPid); }
    else if ((unsigned int)syscallNumber == SYS_FORK_EXEC)              { sysForkExec((struct fileParameter *)arg1, currentPid); }
    else if ((unsigned int)syscallNumber == SYS_EXIT)                   { sysExit(currentPid); }
    else if ((unsigned int)syscallNumber == SYS_FREE)                   { sysFree(currentPid); }
    else if ((unsigned int)syscallNumber == SYS_MMAP)                   { sysMmap(currentPid); }
    else if ((unsigned int)syscallNumber == SYS_KILL)                   { sysKill(arg1); }
    else if ((unsigned int)syscallNumber == SYS_SWITCH_TASK)            { sysSwitch(arg1, currentPid); }
    else if ((unsigned int)syscallNumber == SYS_MEM_DUMP)               { sysMemDump(arg1); }
    else if ((unsigned int)syscallNumber == SYS_UPTIME)                 { sysUptime(); }
    else if ((unsigned int)syscallNumber == SYS_SWITCH_TASK_TO_PARENT)  { sysSwitchToParent(currentPid); }
    else if ((unsigned int)syscallNumber == SYS_WAIT)                   { sysWait(); }
    else if ((unsigned int)syscallNumber == SYS_DIR)                    { sysDirectory(currentPid); }
    else if ((unsigned int)syscallNumber == SYS_TOGGLE_SCHEDULER)       { sysToggleScheduler(); }
    else if ((unsigned int)syscallNumber == SYS_SHOW_OPEN_FILES)        { sysShowOpenFiles(currentPid); }
    else if ((unsigned int)syscallNumber == SYS_BEEP)                   { sysBeep(); }
    else if ((unsigned int)syscallNumber == SYS_CREATE)                 { sysCreate((struct fileParameter *)arg1, currentPid); }
    else if ((unsigned int)syscallNumber == SYS_DELETE)                 { sysDelete((struct fileParameter *)arg1, currentPid); }
    else if ((unsigned int)syscallNumber == SYS_OPEN_EMPTY)             { sysOpenEmpty((struct fileParameter *)arg1, currentPid); }

    scheduler(currentPid);

    returnedPid = readValueFromMemLoc(RUNNING_PID_LOC);
    newSysHandlertaskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (returnedPid - 1));
    newSysHandlerTask = (struct task*)newSysHandlertaskStructLocation;

    asm volatile ("popa\n\t");
    asm volatile ("leave\n\t");

    // pop off the important registers from the last task and save

    asm volatile ("pop %edx\n\t"); // burn
    asm volatile ("pop %edx\n\t"); // burn
    asm volatile ("pop %edx\n\t"); // burn
    asm volatile ("pop %edx\n\t"); // burn

    // Now recreate the stack for the IRET after saving the registers
    // Pushing these in reverse order so they are ready for the iret

    asm volatile ("movl %0, %%edx\n\t" : : "r" ((uint32_t)newSysHandlerTask->esp));
    asm volatile ("pushl %edx\n\t");

    asm volatile ("movl %0, %%edx\n\t" : : "r" ((uint32_t)newSysHandlerTask->eax));
    asm volatile ("pushl %edx\n\t");

    asm volatile ("mov %0, %%dx\n\t" : : "r" ((uint16_t)newSysHandlerTask->cs));
    asm volatile ("pushl %eax\n\t");

    asm volatile ("movl %0, %%edx\n\t" : : "r" ((uint32_t)newSysHandlerTask->eip));
    asm volatile ("pushl %edx\n\t");

    asm volatile ("movl %0, %%ebp\n\t" : : "r" ((uint32_t)newSysHandlerTask->ebp));

    asm volatile ("iret\n\t");

}

void systemInterruptHandler()
{    
    asm volatile ("pusha\n\t");

    uint8_t currentInterrupt = 0;
    uint32_t currentPid = readValueFromMemLoc(RUNNING_PID_LOC);

    disableInterrupts();

    totalInterruptCount++;

    uint32_t currentTaskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    struct task *currentTask;
    currentTask = (struct task*)currentTaskStructLocation;

    currentTask->runtime++;
    currentTask->recentRuntime++;

    uint32_t taskStructNumber = 0;
    struct task *Task = (struct task*)PROCESS_TABLE_LOC;

    while (taskStructNumber < MAX_PROCESSES)
    {
        if (Task->state == PROC_SLEEPING)
        {
            Task->sleepTime++;
        }
        taskStructNumber++;
        Task++;
    }


    outputIOPort(MASTER_PIC_COMMAND_PORT, 0xA);
    outputIOPort(MASTER_PIC_COMMAND_PORT, 0xB);
    currentInterrupt = inputIOPort(MASTER_PIC_COMMAND_PORT);
    
    if ((currentInterrupt & 0b0000001) == 0x1) // system timer IRQ 0
    {
        systemTimerInterruptCount++;
        
        if (totalInterruptCount % SYSTEM_INTERRUPTS_PER_SECOND)
        {
            uptimeSeconds = totalInterruptCount / SYSTEM_INTERRUPTS_PER_SECOND;
            if (uptimeSeconds == 60) { uptimeSeconds = 0; }
            uptimeMinutes = ((totalInterruptCount / SYSTEM_INTERRUPTS_PER_SECOND) / 60);
        }
    }
    else if ((currentInterrupt & 0b0000010) == 0x2) // keyboard interrupt IRQ 1
    {
        keyboardInterruptCount++;

    }
    else
    {
        otherInterruptCount++; // capture any other interrupts
    }

    outputIOPort(MASTER_PIC_COMMAND_PORT, INTERRUPT_END_OF_INTERRUPT);
    outputIOPort(SLAVE_PIC_COMMAND_PORT, INTERRUPT_END_OF_INTERRUPT);

    enableInterrupts();

    asm volatile ("popa\n\t");
    asm volatile ("leave\n\t");
    asm volatile ("iret\n\t");

}

void printSystemUptime(uint32_t uptimeSeconds, uint32_t uptimeMinutes)
{
    uint8_t *uptimeSecondsLoc = kMalloc(KERNEL_OWNED, sizeof(int));
    uint8_t *uptimeMinutesLoc = kMalloc(KERNEL_OWNED, sizeof(int));

    itoa((unsigned int)(uptimeSeconds), uptimeSecondsLoc);
    itoa((unsigned int)(uptimeMinutes), uptimeMinutesLoc);

    printString(COLOR_GREEN, 4, 2, (uint8_t *)"Uptime (mins):");
    printString(COLOR_LIGHT_BLUE, 4, 30, uptimeMinutesLoc);
    printString(COLOR_GREEN, 5, 2, (uint8_t *)"Uptime (secs):");
    printString(COLOR_LIGHT_BLUE, 5, 30, uptimeSecondsLoc);

    kFree(uptimeSecondsLoc);
    kFree(uptimeMinutesLoc);
}
