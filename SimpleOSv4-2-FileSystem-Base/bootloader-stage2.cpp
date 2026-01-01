// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "screen.h"
#include "fs.h"
#include "constants.h"
#include "exceptions.h"
#include "vm.h"


void main()
{
    // Clear all upper memory areas for warm reboot
    fillMemory((uint8_t *)0x100000, 0x0, 0x29E000); 
    fillMemory((uint8_t *)KERNEL_SEMAPHORE_TABLE, 0x0, PAGE_SIZE); 
    fillMemory((uint8_t *)OPEN_FILE_TABLE, 0x0, PAGE_SIZE);

    // Load the superblock and block group descriptor table
    fillMemory(SUPERBLOCK_LOC, 0x0, PAGE_SIZE);
    readBlock(SUPERBLOCK, SUPERBLOCK_LOC);
    readBlock(GROUP_DESCRIPTOR_BLOCK, BLOCK_GROUP_DESCRIPTOR_TABLE);

    uint32_t cursorRow = 0;

    clearScreen();
    printString(COLOR_WHITE, cursorRow++, 0, (uint8_t *)"Bootloader stage 2...");
    printString(COLOR_WHITE, cursorRow++, 0, (uint8_t *)"Reading kernel binary from file system...");

    if (!fsFindFile((uint8_t *)"kernel", KERNEL_TEMP_INODE_LOC))
    {
        panic((uint8_t *)"Bootloader-stage2.cpp -> Cannot find kernel in root directory");
    }
    loadFileFromInodeStruct(KERNEL_TEMP_INODE_LOC, KERNEL_TEMP_FILE_LOC);
    loadElfFile(KERNEL_TEMP_FILE_LOC);

    printString(COLOR_WHITE, cursorRow++, 0, (uint8_t *)"Kernel binary loaded...jumping to kernel main...");  

    struct elfHeader *ELFHeader = (struct elfHeader*)KERNEL_TEMP_FILE_LOC;

    (*(void(*)())ELFHeader->e_entry)(); 

    panic((uint8_t *)"bootloader-stage2.cpp -> Error launching kernel binary");

}