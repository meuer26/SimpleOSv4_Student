// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "screen.h"
#include "fs.h"
#include "simpleOSlibc.h"
#include "constants.h"
#include "x86.h"
#include "vm.h"
#include "file.h"

void diskStatusCheck()
{
    //checks disk status and loops if not ready
    while ( ((inputIOPort(PRIMARY_ATA_COMMAND_STATUS_REGISTER) & 0xC0) != 0x40) ) {}   
}


void diskReadSector(uint32_t sectorNumber, uint8_t *destinationMemory)
{
    
    // ASSIGNMENT 2 TO DO

}

void diskWriteSector(uint32_t sectorNumber, uint8_t *sourceMemory)
{
    
    // ASSIGNMENT 2 TO DO

}

void readBlock(uint32_t blockNumber, uint8_t *destinationMemory)
{
    uint32_t sectorStart = (blockNumber * 2) + EXT2_SECTOR_START;
    diskReadSector(sectorStart, destinationMemory);
    diskReadSector(sectorStart + 1, destinationMemory + SECTOR_SIZE);
}

void writeBlock(uint32_t blockNumber, uint8_t *sourceMemory)
{
    uint32_t sectorStart = (blockNumber * 2) + EXT2_SECTOR_START;
    diskWriteSector(sectorStart, sourceMemory);
    diskWriteSector(sectorStart + 1, sourceMemory + SECTOR_SIZE);
}

uint32_t allocateFreeBlock()
{
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_block_usage, (uint8_t *)EXT2_BLOCK_USAGE_MAP);

    uint32_t lastUsedBlock = 0;
    uint32_t blockNumber = 0;
    uint32_t valueToWrite = 0;

    while (blockNumber < 0xF0) // Max byte of block bit flag for this 2 MB file system
    {
        if (*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)1) { lastUsedBlock++; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)3)) { lastUsedBlock=lastUsedBlock+2; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)7)) { lastUsedBlock=lastUsedBlock+3; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)15)) { lastUsedBlock=lastUsedBlock+4; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)31)) { lastUsedBlock=lastUsedBlock+5; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)63)) { lastUsedBlock=lastUsedBlock+6; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)127)) { lastUsedBlock=lastUsedBlock+7; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)255)) { lastUsedBlock=lastUsedBlock+8;}
        else if (*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)0) { break; }
        blockNumber++;
    }

    lastUsedBlock++;

    if ((lastUsedBlock % 8) == (uint8_t)1) {valueToWrite = (uint8_t)1;}
    else if ((lastUsedBlock % 8) == (uint8_t)2) {valueToWrite = (uint8_t)3;}
    else if ((lastUsedBlock % 8) == (uint8_t)3) {valueToWrite = (uint8_t)7;}
    else if ((lastUsedBlock % 8) == (uint8_t)4) {valueToWrite = (uint8_t)15;}
    else if ((lastUsedBlock % 8) == (uint8_t)5) {valueToWrite = (uint8_t)31;}
    else if ((lastUsedBlock % 8) == (uint8_t)6) {valueToWrite = (uint8_t)63;}
    else if ((lastUsedBlock % 8) == (uint8_t)7) {valueToWrite = (uint8_t)127;}
    else if ((lastUsedBlock % 8) == (uint8_t)0) {valueToWrite = (uint8_t)255;}

    *(uint8_t *)(EXT2_BLOCK_USAGE_MAP + blockNumber) = (uint8_t)valueToWrite;

    writeBlock(BlockGroupDescriptor->bgd_block_address_of_block_usage, (uint8_t *)EXT2_BLOCK_USAGE_MAP);

    return (uint32_t)lastUsedBlock;
}

uint32_t readNextAvailableBlock()
{
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_block_usage, (uint8_t *)EXT2_BLOCK_USAGE_MAP);

    uint32_t lastUsedBlock = 0;
    uint32_t blockNumber = 0;

    while (blockNumber < 0xF0) // Max byte of block bit flag for this 2 MB file system
    {
        if (*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)1) { lastUsedBlock++; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)3)) { lastUsedBlock=lastUsedBlock+2; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)7)) { lastUsedBlock=lastUsedBlock+3; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)15)) { lastUsedBlock=lastUsedBlock+4; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)31)) { lastUsedBlock=lastUsedBlock+5; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)63)) { lastUsedBlock=lastUsedBlock+6; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)127)) { lastUsedBlock=lastUsedBlock+7; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)255)) { lastUsedBlock=lastUsedBlock+8;}
        else if (*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)0) { break; }
        blockNumber++;
    }

    lastUsedBlock++;

    return (uint32_t)lastUsedBlock;
}

uint32_t readTotalBlocksUsed()
{
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_block_usage, (uint8_t *)EXT2_BLOCK_USAGE_MAP);

    uint32_t blocksInUse = 0;
    uint32_t blockNumber = 0;

    while (blockNumber < 0xF0) // Max byte of block bit flag for this 2 MB file system
    {
        if (*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)1) { blocksInUse++;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)3)) { blocksInUse=blocksInUse+2;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)7)) { blocksInUse=blocksInUse+3;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)15)) { blocksInUse=blocksInUse+4;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)31)) { blocksInUse=blocksInUse+5;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)63)) { blocksInUse=blocksInUse+6;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)127)) { blocksInUse=blocksInUse+7;}
        else if (((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockNumber) == (uint8_t)255)) { blocksInUse=blocksInUse+8;}
        blockNumber++;
    }

    return (uint32_t)blocksInUse;
}

void freeBlock(uint32_t blockNumber)
{
    uint32_t blockGroupByte = blockNumber / 8;
    uint32_t blockGroupBit = blockNumber % 8;
    uint32_t valueToWrite = 0;

    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_block_usage, (uint8_t *)EXT2_BLOCK_USAGE_MAP);

    if (blockGroupBit == (uint8_t)1) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -1;}
    else if (blockGroupBit == (uint8_t)2) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -2;}
    else if (blockGroupBit == (uint8_t)3) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -4;}
    else if (blockGroupBit == (uint8_t)4) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -8;}
    else if (blockGroupBit == (uint8_t)5) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -16;}
    else if (blockGroupBit == (uint8_t)6) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -32;}
    else if (blockGroupBit == (uint8_t)7) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -64;}
    else if (blockGroupBit == (uint8_t)0) { valueToWrite = ((uint8_t)*(uint8_t*)(EXT2_BLOCK_USAGE_MAP + blockGroupByte)) -128;}

    *(uint8_t *)(EXT2_BLOCK_USAGE_MAP + blockGroupByte) = (uint8_t)valueToWrite;

    writeBlock(BlockGroupDescriptor->bgd_block_address_of_block_usage, (uint8_t *)EXT2_BLOCK_USAGE_MAP);   
}

void freeAllBlocks(struct inode *inodeStructMemory)
{
    for (uint32_t x = 0; x < EXT2_NUMBER_OF_DIRECT_BLOCKS; x++)
    { 
        if (x < EXT2_NUMBER_OF_DIRECT_BLOCKS && inodeStructMemory->i_block[x] != 0)
        {
            freeBlock(inodeStructMemory->i_block[x]);
        }       
    }
    if (inodeStructMemory->i_block[EXT2_FIRST_INDIRECT_BLOCK] && inodeStructMemory->i_block[EXT2_FIRST_INDIRECT_BLOCK] != 0)
    {
        readBlock(inodeStructMemory->i_block[EXT2_FIRST_INDIRECT_BLOCK], EXT2_INDIRECT_BLOCK);
        uint32_t *indirectBlock = (uint32_t *)EXT2_INDIRECT_BLOCK;

        for (uint32_t y = 0; y < 256; y++)
        {
            if (indirectBlock[y] !=0)
            {
                freeBlock(indirectBlock[y]);
            }

        }
             
    }
}

void deleteFile(uint8_t *fileName, uint32_t currentPid)
{
    uint8_t *inodePage = requestAvailablePage(currentPid, PG_USER_PRESENT_RW);
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    
    fsFindFile(fileName, inodePage);

    if (!fsFindFile(fileName, inodePage))
    {
        // File not found
        return;
    }
    freeAllBlocks((struct inode *)inodePage);

    // Load all inodes of a directory up to max number of files per directory. This requires 16KB of memory.
    for (uint32_t blocksOfInodes=0; blocksOfInodes < (MAX_FILES_PER_DIRECTORY / INODES_PER_BLOCK); blocksOfInodes++)
    {
        readBlock(BlockGroupDescriptor->bgd_starting_block_of_inode_table + blocksOfInodes, (uint8_t *)(uint32_t)EXT2_TEMP_INODE_STRUCTS + (BLOCK_SIZE * blocksOfInodes));
    }

    // Zero out the inode
    fillMemory((EXT2_TEMP_INODE_STRUCTS + ((returnInodeofFileName(fileName)-1) * INODE_SIZE)), 0x0, INODE_SIZE);

    for (uint32_t blocksOfInodes=0; blocksOfInodes < (MAX_FILES_PER_DIRECTORY / INODES_PER_BLOCK); blocksOfInodes++)
    {
        writeBlock(BlockGroupDescriptor->bgd_starting_block_of_inode_table + blocksOfInodes, (uint8_t *)(uint32_t)EXT2_TEMP_INODE_STRUCTS + (BLOCK_SIZE * blocksOfInodes));
    }

    deleteDirectoryEntry(fileName);
    freePage(currentPid, inodePage);
}

uint32_t allocateInode()
{
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_inode_usage, (uint8_t *)EXT2_INODE_USAGE_MAP);

    uint32_t lastUsedInode = 0;
    uint32_t inodeNumber = 0;
    uint32_t valueToWrite = 0;

    while (inodeNumber < 0x1E) // Max byte of inode bit flag for this 2 MB file system
    {
        if (*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)1) { lastUsedInode++; break; }
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)3)) { lastUsedInode=lastUsedInode+2; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)7)) { lastUsedInode=lastUsedInode+3; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)15)) { lastUsedInode=lastUsedInode+4; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)31)) { lastUsedInode=lastUsedInode+5; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)63)) { lastUsedInode=lastUsedInode+6; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)127)) { lastUsedInode=lastUsedInode+7; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)255)) { lastUsedInode=lastUsedInode+8; }
        else if (*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)0) { break; }
        inodeNumber++;
    }

    lastUsedInode++;

    if ((lastUsedInode % 8) == (uint8_t)1) {valueToWrite = (uint8_t)1;}
    else if ((lastUsedInode % 8) == (uint8_t)2) {valueToWrite = (uint8_t)3;}
    else if ((lastUsedInode % 8) == (uint8_t)3) {valueToWrite = (uint8_t)7;}
    else if ((lastUsedInode % 8) == (uint8_t)4) {valueToWrite = (uint8_t)15;}
    else if ((lastUsedInode % 8) == (uint8_t)5) {valueToWrite = (uint8_t)31;}
    else if ((lastUsedInode % 8) == (uint8_t)6) {valueToWrite = (uint8_t)63;}
    else if ((lastUsedInode % 8) == (uint8_t)7) {valueToWrite = (uint8_t)127;}
    else if ((lastUsedInode % 8) == (uint8_t)0) {valueToWrite = (uint8_t)255;}

    *(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) = (uint8_t)valueToWrite;

    writeBlock(BlockGroupDescriptor->bgd_block_address_of_inode_usage, (uint8_t *)EXT2_INODE_USAGE_MAP);

    return (uint32_t )lastUsedInode;
}

uint32_t readNextAvailableInode()
{
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    readBlock(BlockGroupDescriptor->bgd_block_address_of_inode_usage, (uint8_t *)EXT2_INODE_USAGE_MAP);

    uint32_t lastUsedInode = 0;
    uint32_t inodeNumber = 0;

    while (inodeNumber < 0x1E) // Max byte of inode bit flag for this 2 MB file system
    {
        if (*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)1) { lastUsedInode++; break; }
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)3)) { lastUsedInode=lastUsedInode+2; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)7)) { lastUsedInode=lastUsedInode+3; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)15)) { lastUsedInode=lastUsedInode+4; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)31)) { lastUsedInode=lastUsedInode+5; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)63)) { lastUsedInode=lastUsedInode+6; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)127)) { lastUsedInode=lastUsedInode+7; break;}
        else if (((uint8_t)*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)255)) { lastUsedInode=lastUsedInode+8; }
        else if (*(uint8_t*)(EXT2_INODE_USAGE_MAP + inodeNumber) == (uint8_t)0) { break; }
        inodeNumber++;
    }

    lastUsedInode++;

    return (uint32_t )lastUsedInode;
}

void deleteDirectoryEntry(uint8_t *fileName)
{
    uint32_t inodeToRemove = returnInodeofFileName(fileName);
    
    fillMemory((uint8_t *)KERNEL_TEMP_INODE_LOC, 0x0, PAGE_SIZE);
    readBlock(ROOTDIR_BLOCK, (uint8_t *)KERNEL_TEMP_INODE_LOC);

    struct directoryEntry *DirectoryEntry = (directoryEntry*)(KERNEL_TEMP_INODE_LOC);
    uint8_t *savedDirectoryEntry;
    uint8_t *previousDirectoryEntry;
    uint32_t savedDirectoryEntryRecLength;
    uint32_t previousDirectoryEntryRecLength;

    while ((uint32_t)DirectoryEntry->directoryInode != 0)
    {
        // This is due to the last entry in a directory having a very long record length
        // It is the way to identify the last entry in a directory so we can add a new entry
        if (DirectoryEntry->directoryInode == inodeToRemove)
        {
            savedDirectoryEntry = (uint8_t *)DirectoryEntry;
            savedDirectoryEntryRecLength = DirectoryEntry->recLength;

            if (DirectoryEntry->recLength > 255)
            {
                fillMemory(savedDirectoryEntry, 0x0, savedDirectoryEntryRecLength);
                memoryCopy((uint8_t *)(savedDirectoryEntry + savedDirectoryEntryRecLength), savedDirectoryEntry, (BLOCK_SIZE *3)/2);
                *(uint16_t*)(previousDirectoryEntry + 4) = (uint16_t)0x100; //make it the last entry in the directory
            }
            else
            {
                fillMemory(savedDirectoryEntry, 0x0, savedDirectoryEntryRecLength);
                memoryCopy((uint8_t *)(savedDirectoryEntry + savedDirectoryEntryRecLength), savedDirectoryEntry, (BLOCK_SIZE *3)/2);
            }
            
            break;
        }
        else
        {
            previousDirectoryEntry = (uint8_t *)DirectoryEntry;
            previousDirectoryEntryRecLength = DirectoryEntry->recLength;
            
            DirectoryEntry = (directoryEntry *)((uint32_t)DirectoryEntry + DirectoryEntry->recLength);
        }
        
    }

    writeBlock(ROOTDIR_BLOCK, (uint8_t *)KERNEL_TEMP_INODE_LOC);
}

void createFile(uint8_t *fileName, uint32_t currentPid, uint32_t fileDescriptor)
{
    uint32_t taskStructLocation = PROCESS_TABLE_LOC + (TASK_STRUCT_SIZE * (currentPid - 1));
    struct task *Task = (struct task*)taskStructLocation;
    
    fillMemory((uint8_t *)KERNEL_TEMP_INODE_LOC, 0x0, PAGE_SIZE);
    readBlock(ROOTDIR_BLOCK, (uint8_t *)KERNEL_TEMP_INODE_LOC);

    struct directoryEntry *DirectoryEntry = (directoryEntry*)(KERNEL_TEMP_INODE_LOC);

    while ((uint32_t)DirectoryEntry->directoryInode != 0)
    {
        // This is due to the last entry in a directory having a very long record length
        // It is the way to identify the last entry in a directory so we can add a new entry
        if (DirectoryEntry->recLength > 255) { break; }
        else 
        {
            DirectoryEntry = (directoryEntry *)((uint32_t)DirectoryEntry + DirectoryEntry->recLength);
        }
        
    }

    // Have to update the record length of the last directory entry before adding another file to the directory
    DirectoryEntry->recLength = (ceiling(DirectoryEntry->nameLength+1, 4)*4) + 8;

    DirectoryEntry = (directoryEntry *)((uint32_t)DirectoryEntry + DirectoryEntry->recLength);

    DirectoryEntry->directoryInode = allocateInode();
    strcpy((uint8_t *)&DirectoryEntry->fileName, fileName);
    DirectoryEntry->fileType = (uint8_t)1;
    DirectoryEntry->nameLength = (uint8_t)(strlen(fileName));
    DirectoryEntry->recLength = (uint16_t)0x100; //make it the last entry in the directory

    writeInodeEntry((int)DirectoryEntry->directoryInode, 0x81b6, (struct openFileTableEntry *)Task->fileDescriptor[fileDescriptor]);

    writeBlock(ROOTDIR_BLOCK, (uint8_t *)KERNEL_TEMP_INODE_LOC);
}

void writeInodeEntry(uint32_t inodeEntry, uint16_t mode, struct openFileTableEntry *openFile)
{
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);
    uint32_t blocksOfInodes = 0;

    while(blocksOfInodes < (MAX_FILES_PER_DIRECTORY / INODES_PER_BLOCK))
    {
        readBlock(BlockGroupDescriptor->bgd_starting_block_of_inode_table + blocksOfInodes, (uint8_t *)(int)EXT2_TEMP_INODE_STRUCTS + (BLOCK_SIZE * blocksOfInodes));
        blocksOfInodes++;
    }

    struct inode *Inode = (struct inode*)(EXT2_TEMP_INODE_STRUCTS + (INODE_SIZE * (inodeEntry - 1)));

    Inode->i_mode = mode;

    writeBufferToDisk(openFile, inodeEntry);

    blocksOfInodes = 0;

    while(blocksOfInodes < (MAX_FILES_PER_DIRECTORY / INODES_PER_BLOCK))
    {
        writeBlock(BlockGroupDescriptor->bgd_starting_block_of_inode_table + blocksOfInodes, (uint8_t *)(int)EXT2_TEMP_INODE_STRUCTS + (BLOCK_SIZE * blocksOfInodes));
        blocksOfInodes++;
    }

}

void writeBufferToDisk(struct openFileTableEntry *openFile, uint32_t inodeEntry)
{
    struct inode *Inode = (struct inode*)(EXT2_TEMP_INODE_STRUCTS + (INODE_SIZE * (inodeEntry - 1)));

    fillMemory((uint8_t *)EXT2_INDIRECT_BLOCK_TMP_LOC, 0x0, BLOCK_SIZE);

    uint32_t blockArrayDirect[13];
    uint32_t *blockArraySinglyIndirect = (uint32_t *)EXT2_INDIRECT_BLOCK_TMP_LOC;
    uint32_t totalBlocksNeeded = ceiling((openFile->numberOfPagesForBuffer * PAGE_SIZE), BLOCK_SIZE);
    uint32_t currentDirectBlock = 0;
    uint32_t currentIndirectBlock = 0;

    while (currentDirectBlock <= 12 && (currentDirectBlock < totalBlocksNeeded))
    {
        if (currentDirectBlock < 12)
        {
            blockArrayDirect[currentDirectBlock] = allocateFreeBlock();
            writeBlock(blockArrayDirect[currentDirectBlock], (uint8_t *)(openFile->userspaceBuffer + (currentDirectBlock * BLOCK_SIZE)));
            Inode->i_block[currentDirectBlock] = blockArrayDirect[currentDirectBlock];
            currentDirectBlock++;
        }
        else if (currentDirectBlock == 12)
        {
            blockArrayDirect[12] = allocateFreeBlock(); //write the indirect block
            Inode->i_block[12] = blockArrayDirect[12];
            currentDirectBlock++;
        }
    }

    if (totalBlocksNeeded > 12)
    {

        for (currentIndirectBlock=0; (currentIndirectBlock + currentDirectBlock) < totalBlocksNeeded; currentIndirectBlock++)
        {
            blockArraySinglyIndirect[currentIndirectBlock] = allocateFreeBlock();
            writeBlock(blockArraySinglyIndirect[currentIndirectBlock], (uint8_t *)(openFile->userspaceBuffer + ((currentIndirectBlock + currentDirectBlock) * BLOCK_SIZE)));
        }

        // Write the indirect block to disk
        writeBlock(Inode->i_block[12], (uint8_t *)EXT2_INDIRECT_BLOCK_TMP_LOC);
    }

    Inode->i_size = (currentDirectBlock + currentIndirectBlock) * BLOCK_SIZE;
}


void loadElfFile(uint8_t *elfHeaderLocation)
{

    // ASSIGNMENT 2 TO DO

}

void loadFileFromInodeStruct(uint8_t *inodeStructMemory, uint8_t *fileBuffer)
{
    
    // ASSIGNMENT 2 TO DO
    
}


bool fsFindFile(uint8_t *fileName, uint8_t *destinationMemory)
{ 
    fillMemory((uint8_t *)KERNEL_TEMP_INODE_LOC, 0x0, PAGE_SIZE);
    readBlock(ROOTDIR_BLOCK, (uint8_t *)KERNEL_TEMP_INODE_LOC);

    struct directoryEntry *DirectoryEntry = (directoryEntry*)(KERNEL_TEMP_INODE_LOC);
    struct blockGroupDescriptor *BlockGroupDescriptor = (blockGroupDescriptor*)(BLOCK_GROUP_DESCRIPTOR_TABLE);

    while ((uint32_t)DirectoryEntry->directoryInode != 0)
    {
        if (strcmp((uint8_t *)(&DirectoryEntry->fileName), fileName) == '\0')
        {      
            // Load all inodes of a directory up to max number of files per directory. This requires 16KB of memory.
            for (uint32_t blocksOfInodes=0; blocksOfInodes < (MAX_FILES_PER_DIRECTORY / INODES_PER_BLOCK); blocksOfInodes++)
            {
                readBlock(BlockGroupDescriptor->bgd_starting_block_of_inode_table + blocksOfInodes, (uint8_t *)(int)EXT2_TEMP_INODE_STRUCTS + (BLOCK_SIZE * blocksOfInodes));
            }
            memoryCopy( (uint8_t *)((int)EXT2_TEMP_INODE_STRUCTS + ((DirectoryEntry->directoryInode -1) * INODE_SIZE)), destinationMemory, INODE_SIZE/2);
            
            return true;
        }

        DirectoryEntry = (directoryEntry *)((int)DirectoryEntry + DirectoryEntry->recLength);

    }

    return false;

}

uint32_t returnInodeofFileName(uint8_t *fileName)
{ 
    fillMemory((uint8_t *)KERNEL_TEMP_INODE_LOC, 0x0, PAGE_SIZE);
    readBlock(ROOTDIR_BLOCK, (uint8_t *)KERNEL_TEMP_INODE_LOC);

    struct directoryEntry *DirectoryEntry = (directoryEntry*)(KERNEL_TEMP_INODE_LOC);

    while ((int)DirectoryEntry->directoryInode != 0)
    {
        if (strcmp((uint8_t *)(&DirectoryEntry->fileName), fileName) == '\0')
        {      
            return DirectoryEntry->directoryInode;
        }

        DirectoryEntry = (directoryEntry *)((int)DirectoryEntry + DirectoryEntry->recLength);

    }

    return 0;
}