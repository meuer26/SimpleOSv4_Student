// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "simpleOSlibc.h"
#include "constants.h"
#include "vm.h"
#include "x86.h"
#include "sound.h"
#include "file.h"
#include "screen.h"
#include "keyboard.h"

void wait(uint32_t timeToWait)
{
    // Each syscall to SYS_WAIT waits one second. timeToWait is how many seconds to wait
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    for (uint32_t x = 0; x < timeToWait; x++)
    {
        sysCall(SYS_WAIT, 0x0, myPid);
    }

}

uint32_t strcmp(uint8_t *firstString, uint8_t *secondString)
{

    while (*firstString != 0x0)
    {
        if (*firstString == *secondString)
        {
            firstString++;
            secondString++;
        }
        else if (*secondString == 0x0)
        {
            return 1;
        }
        else
        {
            return 0xFFFFFFFF; // -1
        }


    }

    return 0;


}

uint32_t strlen(uint8_t *targetString)
{
    uint32_t stringLength = 0;

    while (*targetString != 0x0)
    {
        stringLength++;
        targetString++;
    }

    return stringLength;

}

void strcpy(uint8_t *destinationString, uint8_t *sourceString)
{

    while (*sourceString != 0x0)
    {
        *destinationString = *sourceString;
        sourceString++;
        destinationString++;
    }

    // add a null termination to destination
    *destinationString = 0x0;

}

void strcpyRemoveNewline(uint8_t *destinationString, uint8_t *sourceString)
{
    
    while (*sourceString != 0x0)
    {
        if (*sourceString != '\n')
        {
            *destinationString = *sourceString;
            destinationString++;
            sourceString++;
            continue;
        }
        else
        {
            break;
        }

    }

    // add a null termination to destination
    *destinationString = 0x0;

}

void bytecpy(uint8_t *destinationMemory, uint8_t *sourceMemory, uint32_t numberOfBytes)
{
    for (uint32_t x = 0; x < numberOfBytes; x++)
    {
        *destinationMemory = *sourceMemory;
        destinationMemory++;
        sourceMemory++;
    }
}

void itoa(uint32_t number, uint8_t *destinationMemory)
{
    uint8_t *startingMemoryLocation = destinationMemory;
    uint8_t byteToPreserve;
    
    // Using divide by 10 algorithm
    while ((number / 10) > 0 )
    {
        *destinationMemory = (number % 10) + 0x30;
        number = number / 10;
        destinationMemory++;
    }

    *destinationMemory = number + 0x30;

    // Now I need to reverse it
    while (startingMemoryLocation < destinationMemory)
    {
        byteToPreserve = *startingMemoryLocation;
        *startingMemoryLocation = *destinationMemory;
        *destinationMemory = byteToPreserve;
        startingMemoryLocation++;
        destinationMemory--;

    }

}

uint32_t atoi(uint8_t *sourceString)
{
    uint32_t number = 0;

    while (*sourceString != 0x0)
    {
        if ((*sourceString - 0x30) >= 0x0 && (*sourceString - 0x30) <= 0x9)
        
        number = (number * 10) + (*sourceString - 0x30);
        sourceString++;
        
    }

   return number;

}

uint32_t ceiling(uint32_t number, uint32_t base)
{
    return ((number + base - 1) / base);
}

uint8_t *malloc(uint32_t currentPid, uint32_t objectSize)
{  
    // Heap entries take up 16 bytes each, but 1 byte is the PID that owns it.
    // So, heap items cannot exceed 15 bytes (including null byte for strings)
    // If you need more than this, use mmap to get 4,096 bytes
    uint8_t heapObjectNumber = 0;
    struct heapObject *HeapObject = (struct heapObject*)USER_HEAP;

    if (objectSize > HEAP_OBJ_USABLE_SIZE)
    {
        return 0;
    }

    while (heapObjectNumber < MAX_HEAP_OBJECTS)
    {
        if (HeapObject->pid == 0)
        {
            HeapObject->pid = (uint8_t)currentPid;
            return &HeapObject->heapUseable[0];    
        }
        heapObjectNumber++;
        HeapObject++;
    }
    // return null pointer if unable to find object
    return 0;    
}

void free(uint8_t *heapObject)
{
    fillMemory((heapObject - 1), 0x0, HEAP_OBJ_SIZE); // minus 1 to remove the entire entry, including pid
}


void freeAll(uint32_t currentPid)
{
    uint32_t heapObjectNumber = 0;
    uint32_t heapObjectPid = 0;

    while (heapObjectNumber < MAX_HEAP_OBJECTS)
    {

        heapObjectPid = *(uint32_t *)(USER_HEAP + (HEAP_OBJ_SIZE * heapObjectNumber));

        if (heapObjectPid == currentPid)
        {
            fillMemory((uint8_t *)(USER_HEAP + (HEAP_OBJ_SIZE * heapObjectNumber)), 0x0, HEAP_OBJ_SIZE);
        }

        heapObjectNumber++;
    }
}


uint8_t *kMalloc(uint32_t currentPid, uint32_t objectSize)
{
    // Heap entries take up 16 bytes each, but 1 byte is the PID that owns it.
    // So, heap items cannot exceed 15 bytes (including null byte for strings)
    // If you need more than this, use mmap to get 4,096 bytes
    uint8_t heapObjectNumber = 0;
    struct heapObject *HeapObject = (struct heapObject*)KERNEL_HEAP;

    if (objectSize > HEAP_OBJ_USABLE_SIZE)
    {
        return 0;
    }

    while (heapObjectNumber < MAX_HEAP_OBJECTS)
    {
        if (HeapObject->pid == 0)
        {
            HeapObject->pid = (uint8_t)currentPid;
            return &HeapObject->heapUseable[0];    
        }
        heapObjectNumber++;
        HeapObject++;
    }
    // return null pointer if unable to find object
    return 0;
}

void kFree(uint8_t *heapObject)
{
    fillMemory((heapObject - 1), 0x0, HEAP_OBJ_SIZE); // minus 4 to remove the entire entry, including pid
}

void kFreeAll(uint32_t currentPid)
{
    uint32_t heapObjectNumber = 0;
    uint32_t heapObjectPid = 0;

    while (heapObjectNumber < MAX_HEAP_OBJECTS)
    {

        heapObjectPid = *(uint32_t *)(KERNEL_HEAP + (HEAP_OBJ_SIZE * heapObjectNumber));

        if (heapObjectPid == currentPid)
        {
            fillMemory((uint8_t *)(KERNEL_HEAP + (HEAP_OBJ_SIZE * heapObjectNumber)), 0x0, HEAP_OBJ_SIZE);
        }

        heapObjectNumber++;
    }
}

uint32_t stringHash(uint8_t *messagetoHash)
{
    uint32_t sum = 0;

    while (*messagetoHash != 0x0)
    { 
        sum += *messagetoHash;
        messagetoHash++;
    } 

    return sum;
}

void makeSound(uint32_t frequency, uint32_t duration)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    
    struct soundParameter *SoundParameter = (struct soundParameter *)(malloc(myPid, sizeof(soundParameter)));

    SoundParameter->frequency = frequency;
    SoundParameter->duration = duration;
    sysCall(SYS_SOUND, (uint32_t)SoundParameter, myPid); 

    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free((uint8_t *)SoundParameter);
}

void systemUptime()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_UPTIME, 0x0, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

void systemExit()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_EXIT, 0x0, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

void systemFree()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_FREE, 0x0, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

uint8_t *systemMMap()
{
    uint32_t returnedPage;
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_MMAP, 0x0, myPid);
    returnedPage = readValueFromMemLoc(RETURNED_MMAP_PAGE_LOC);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    return (uint8_t *)returnedPage;
}

void systemSwitchToParent()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    if (myPid == 1)
    {
        return; // There is no parent of PID 1
    }

    sysCall(SYS_SWITCH_TASK_TO_PARENT, 0x0, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

void systemBeep()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_BEEP, 1000, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

void systemShowProcesses()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_PS, 0x0, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

void systemListDirectory()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_DIR, 0x0, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

void systemSchedulerToggle()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_TOGGLE_SCHEDULER, 0x0, myPid);
    printString(COLOR_WHITE, 2, 5, (uint8_t *)"Scheduler toggled.");
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

void systemForkExec(uint8_t *fileName, uint32_t requestedRunPriority)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    struct fileParameter *FileParameter = (struct fileParameter *)(malloc(myPid, sizeof(fileParameter)));
    FileParameter->fileNameLength = strlen(fileName);
    FileParameter->fileName = fileName;
    FileParameter->requestedRunPriority = requestedRunPriority;

    sysCall(SYS_FORK_EXEC, (uint32_t)FileParameter, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free((uint8_t *)FileParameter);
}

void systemKill(uint8_t *pidToKill)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    uint8_t *pidToKillMem = malloc(myPid, sizeof(uint32_t));
    bytecpy(pidToKillMem, pidToKill, 1);

    if (atoi(pidToKillMem) == myPid)
    {
        printString(COLOR_RED, 1, 2, (uint8_t *)"You cannot kill yourself!");
        systemBeep();

        free(pidToKillMem);
        return;
    }

    sysCall(SYS_KILL, atoi(pidToKillMem), myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free(pidToKillMem);
}

void systemTaskSwitch(uint8_t *pidToSwitchTo)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    uint8_t *pidToSwitch = malloc(myPid, sizeof(uint32_t));
    bytecpy(pidToSwitch, pidToSwitchTo, 1);

    if (atoi(pidToSwitch) == myPid)
    { 
        printString(COLOR_RED, 1, 2, (uint8_t *)"You cannot switch to yourself!");
        systemBeep();

        return;
    }

    sysCall(SYS_SWITCH_TASK, atoi(pidToSwitch), myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free(pidToSwitch);
}

void systemShowMemory(uint8_t *memoryLocation)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    uint8_t *memLocationToDump = malloc(myPid, sizeof(uint32_t));
    bytecpy(memLocationToDump, memoryLocation, 15);

    sysCall(SYS_MEM_DUMP, atoi(memLocationToDump), myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free(memLocationToDump);
}

void systemShowOpenFiles()
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_SHOW_OPEN_FILES, 0x0, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}


void systemOpenFile(uint8_t *fileName, uint32_t requestedPermissions)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    struct fileParameter *FileParameter = (struct fileParameter *)(malloc(myPid, sizeof(fileParameter)));
    FileParameter->fileNameLength = strlen(fileName);
    FileParameter->requestedPermissions = requestedPermissions;
    FileParameter->fileName = fileName;
    sysCall(SYS_OPEN, (uint32_t)FileParameter, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free((uint8_t *)FileParameter);
}

void systemOpenEmptyFile(uint8_t *fileName, uint32_t requestedSizeInPages)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    struct fileParameter *FileParameter = (struct fileParameter *)(malloc(myPid, sizeof(fileParameter)));
    FileParameter->fileNameLength = strlen(fileName);
    FileParameter->requestedPermissions = RDWRITE;
    FileParameter->requestedSizeInPages = requestedSizeInPages;
    FileParameter->fileName = fileName;
    sysCall(SYS_OPEN_EMPTY, (uint32_t)FileParameter, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free((uint8_t *)FileParameter);
}

void systemCreateFile(uint8_t *fileName, uint32_t fileDescriptor)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    struct fileParameter *FileParameter = (struct fileParameter *)(malloc(myPid, sizeof(fileParameter)));
    FileParameter->fileNameLength = strlen(fileName);
    FileParameter->fileDescriptor = fileDescriptor;
    FileParameter->fileName = fileName;
    sysCall(SYS_CREATE, (uint32_t)FileParameter, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free((uint8_t *)FileParameter);
}

void systemDeleteFile(uint8_t *fileName)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    struct fileParameter *FileParameter = (struct fileParameter *)(malloc(myPid, sizeof(fileParameter)));
    FileParameter->fileNameLength = strlen(fileName);
    FileParameter->fileName = fileName;
    sysCall(SYS_DELETE, (uint32_t)FileParameter, myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    free((uint8_t *)FileParameter);
}

void systemCloseFile(uint8_t *fileDescriptor)
{
    uint32_t myPid = readValueFromMemLoc(RUNNING_PID_LOC);
    sysCall(SYS_CLOSE, atoi(fileDescriptor), myPid);
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
}

uint8_t *octalTranslation(uint8_t permissions)
{
    if (permissions == 0x0) { return (uint8_t *)"---"; }
    else if (permissions == 0x1) { return (uint8_t *)"--X"; }
    else if (permissions == 0x2) { return (uint8_t *)"-W-"; }
    else if (permissions == 0x3) { return (uint8_t *)"-WX"; }
    else if (permissions == 0x4) { return (uint8_t *)"R--"; }
    else if (permissions == 0x5) { return (uint8_t *)"R-X"; }
    else if (permissions == 0x6) { return (uint8_t *)"RW-"; }
    else if (permissions == 0x7) { return (uint8_t *)"RWX"; }
    else { return (uint8_t *)"UNK"; }
}

uint8_t *directoryEntryTypeTranslation(uint8_t type)
{
    if (type == EXT2_DIRECTORY_ENTRY_FILE) { return (uint8_t *)"FILE"; }
    else if (type == EXT2_DIRECTORY_ENTRY_DIR) { return (uint8_t *)"DIR"; }
    else { return (uint8_t *)"UNK"; }
}

uint32_t countHeapObjects(uint8_t *heapLoc)
{
    uint32_t heapObjectNumber = 0;
    uint8_t lastUsedObj = *(uint8_t *)(heapLoc + (HEAP_OBJ_SIZE * heapObjectNumber));
    uint32_t objectCount = 0;

    while (heapObjectNumber < MAX_HEAP_OBJECTS)
    {      
        lastUsedObj = *(uint8_t *)(heapLoc + (HEAP_OBJ_SIZE * heapObjectNumber));

        if ((uint8_t)lastUsedObj != 0)
        {
            objectCount++;
            
        }
        heapObjectNumber++;

    } 

    return objectCount;
}

uint32_t convertToUnixTime(struct time* Time)
{
    // https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_15

    uint32_t sec, min, hour, dayOfYear, year;

    sec = Time->sec;
    min = Time->min;
    hour = Time->hour;
    dayOfYear = Time->dayOfYear;
    year = Time->year;

    return (int)(sec + min*SECONDS_IN_MIN + hour*SECONDS_IN_HOUR + dayOfYear*SECONDS_IN_DAY + 
    (year-1970)*SECONDS_IN_YEAR + ((year-1969)/4)*SECONDS_IN_DAY - ((year-1)/100)*SECONDS_IN_DAY + ((year+299)/400)*SECONDS_IN_DAY);
}

time* convertFromUnixTime(uint32_t unixTime)
{
    struct time* Time;

    uint32_t year = (unsigned int)((unixTime / SECONDS_IN_YEAR) + 1970);
    uint32_t day = (unsigned int)((unixTime - (((year-1970) * SECONDS_IN_YEAR)) - ((year-1969)/4)*SECONDS_IN_DAY)/SECONDS_IN_DAY);

    Time->year = year;
    Time->dayOfYear = day;
    return Time;
}