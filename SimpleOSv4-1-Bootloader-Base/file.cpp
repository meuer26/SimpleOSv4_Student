// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "constants.h"
#include "file.h"
#include "exceptions.h"
#include "vm.h"

void createOpenFileTable(uint8_t *openFileTable)
{
    while (!acquireLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
    
    struct openFileTableEntry *OpenFileTableEntry = (struct openFileTableEntry*)openFileTable;

    OpenFileTableEntry->openedByPid = KERNEL_OWNED; //STDIN
    OpenFileTableEntry->userspaceBuffer = 0; //undefined
    OpenFileTableEntry++;

    OpenFileTableEntry->openedByPid = KERNEL_OWNED; //STDOUT
    OpenFileTableEntry->userspaceBuffer = VIDEO_RAM;
    OpenFileTableEntry++;

    OpenFileTableEntry->openedByPid = KERNEL_OWNED; //STDERR
    OpenFileTableEntry->userspaceBuffer = 0; //undefined

    while (!releaseLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}

}

openFileTableEntry *insertOpenFileTableEntry(uint8_t *openFileTable, uint32_t inode, uint32_t pid, uint8_t *userspaceBuffer, uint32_t numberOfPagesForBuffer, uint8_t *fileName, uint32_t lockedForWriting, uint32_t offset)
{
    while (!acquireLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
    
    uint32_t lastUsedEntry = 0;
    uint32_t nextAvailEntry = 0;
    uint32_t openFileTableEntryNumber = 0;

    while (nextAvailEntry == 0 && openFileTableEntryNumber < MAX_SYSTEM_OPEN_FILES)
    {
        lastUsedEntry = *(uint32_t *)(OPEN_FILE_TABLE + (sizeof(openFileTableEntry) * openFileTableEntryNumber));

        if ((uint32_t)lastUsedEntry == 0)
        {
            nextAvailEntry = (openFileTableEntryNumber + 1);
        }

        if (openFileTableEntryNumber == (MAX_SYSTEM_OPEN_FILES - 1))
        {
            while (!releaseLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
            panic((uint8_t *)"file.cpp:insertOpenFileTableEntry() -> reached max system open files");
        }

        openFileTableEntryNumber++;
    }

    uint32_t availableOpenFileTableLocation = OPEN_FILE_TABLE + (sizeof(openFileTableEntry) * (nextAvailEntry - 1));
    struct openFileTableEntry *OpenFileTableEntry = (struct openFileTableEntry*)availableOpenFileTableLocation;

    OpenFileTableEntry->openedByPid = pid;
    OpenFileTableEntry->inode = inode;
    OpenFileTableEntry->userspaceBuffer = userspaceBuffer;
    OpenFileTableEntry->numberOfPagesForBuffer = numberOfPagesForBuffer;
    OpenFileTableEntry->fileName = fileName;
    OpenFileTableEntry->offset = offset;
    OpenFileTableEntry->lockedForWriting = lockedForWriting;

    while (!releaseLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}

    return OpenFileTableEntry;
}

uint32_t totalOpenFiles(uint8_t *openFileTable)
{
    uint32_t lastUsedEntry = 0;
    uint32_t totalSystemOpenFiles = 0;
    uint32_t openFileTableEntryNumber = 0;

    while (openFileTableEntryNumber < MAX_SYSTEM_OPEN_FILES)
    {
        lastUsedEntry = *(uint32_t *)(OPEN_FILE_TABLE + (sizeof(openFileTableEntry) * openFileTableEntryNumber));

        if ((uint32_t)lastUsedEntry != 0)
        {
            totalSystemOpenFiles++;
        }

        openFileTableEntryNumber++;
    }

    return totalSystemOpenFiles;

}

void closeAllFiles(uint8_t *openFileTable, uint32_t pid)
{
    while (!acquireLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
    
    struct openFileTableEntry *OpenFileTableEntry = (struct openFileTableEntry*)openFileTable;
    uint32_t openFileTableEntryNumber = 0;

    while (openFileTableEntryNumber < MAX_SYSTEM_OPEN_FILES)
    {
        if (OpenFileTableEntry->openedByPid == pid)
        {
            fillMemory((uint8_t *)OpenFileTableEntry, 0x0, sizeof(openFileTableEntry));
        }
        
        OpenFileTableEntry++;
        openFileTableEntryNumber++;
    }

    while (!releaseLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
}

bool fileAvailableToBeLocked(uint8_t *openFileTable, uint32_t inode)
{
    struct openFileTableEntry *OpenFileTableEntry = (struct openFileTableEntry*)openFileTable;
    uint32_t openFileTableEntryNumber = 0;

    while (openFileTableEntryNumber < MAX_SYSTEM_OPEN_FILES)
    {
        if (OpenFileTableEntry->inode == inode)
        {
            if (OpenFileTableEntry->lockedForWriting == FILE_LOCKED)
            {
                return false; // File already locked
            }

        }
        
        OpenFileTableEntry++;
        openFileTableEntryNumber++;
    }

    return true;
}

bool lockFile(uint8_t *openFileTable, uint32_t pid, uint32_t inode)
{
    while (!acquireLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
    
    struct openFileTableEntry *OpenFileTableEntry = (struct openFileTableEntry*)openFileTable;
    uint32_t openFileTableEntryNumber = 0;

    // Do complete scan of table to make sure no one else has it locked
    while (openFileTableEntryNumber < MAX_SYSTEM_OPEN_FILES)
    {
        if (OpenFileTableEntry->inode == inode)
        {
            if (OpenFileTableEntry->lockedForWriting == FILE_LOCKED)
            {
                while (!releaseLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
                return false; // File already locked
            }

        }
        
        OpenFileTableEntry++;
        openFileTableEntryNumber++;
    }

    // Do another scan of the table to lock the file for the requesting pid.
    OpenFileTableEntry = (struct openFileTableEntry*)openFileTable;
    openFileTableEntryNumber = 0;
    while (openFileTableEntryNumber < MAX_SYSTEM_OPEN_FILES)
    {
        if (OpenFileTableEntry->inode == inode && OpenFileTableEntry->openedByPid == pid)
        {
            OpenFileTableEntry->lockedForWriting = FILE_LOCKED;
            while (!releaseLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
            return true;
        }
        
        OpenFileTableEntry++;
        openFileTableEntryNumber++;
    }

    while (!releaseLock(KERNEL_OWNED, (uint8_t *)OPEN_FILE_TABLE)) {}
    return false;
}