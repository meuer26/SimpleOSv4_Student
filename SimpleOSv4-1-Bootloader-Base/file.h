// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "constants.h"
/**
 * The file parameter structure. This is used to pass file-related information in a sysCall().
 */
struct fileParameter
{
    /** The requested permissions. Can be RDWRITE, RDONLY, etc. */
    uint32_t requestedPermissions;
    /** The requested run priority. This is an 8-bit value. If left blank, the value will be zero. */
    uint32_t requestedRunPriority; 
    /** The file descriptor associated with the file. This is needed when creating a new file on the file system. Blank otherwise. */
    uint32_t fileDescriptor;
    /** The requested size of a new, empty file. The size is in pages. */
    uint32_t requestedSizeInPages;
    /** The length of the filename string. */
    uint32_t fileNameLength; 
    /** The name of the file. */
    uint8_t *fileName;
};

/**
 * The open file table entry. This is used to track open files in the kernel.
 */
struct openFileTableEntry
{
    /** The PID that opened this file. */
    uint32_t openedByPid; 
    /** The inode of the open file. */
    uint32_t inode;
    /** A pointer to the user space buffer where the file begins. */
    uint8_t *userspaceBuffer; 
    /** The number of contiguous pages used for the file buffer, starting at the userspaceBuffer address. */
    uint32_t numberOfPagesForBuffer; 
    /** The name of the file. */
    uint8_t *fileName; 
    /** The current byte offset into the buffer, used for modification and the editor. */
    uint32_t offset;
    /** Is this file locked for writing (i.e., opened with RDWRITE permissions). */
    uint32_t lockedForWriting; 
};

struct openBufferTable
{
    uint8_t *buffers[MAX_FILE_DESCRIPTORS];
};

/**
 * Creates the open file table data structure at a particular address in kernel space.
 * \param openFileTable The pointer to the memory location to start building the open file table data structure.
 */
void createOpenFileTable(uint8_t *openFileTable);

/**
 * Creates an entry in the open file table data structure. Returns an openFileTableEntry pointer to the entry.
 * \param openFileTable The pointer to the beginning of the open file table data structure.
 * \param inode The inode of the open file.
 * \param pid The PID of the process that opened the file.
 * \param userspaceBuffer The pointer to the beginning of the contiguous buffer holding the file information.
 * \param numberOfPagesForBuffer The number of contiguous pages that form the buffer.
 * \param fileName The name of the file.
 * \param lockedForWriting The indicator as to whether the file is RDWRITE or not.
 * \param offset The current offset into the file, used in the editor.
 */
openFileTableEntry *insertOpenFileTableEntry(uint8_t *openFileTable, uint32_t inode, uint32_t pid, uint8_t *userspaceBuffer, uint32_t numberOfPagesForBuffer, uint8_t *fileName, uint32_t lockedForWriting, uint32_t offset);

/**
 * Returns the total number of open files in the open file table data structure.
 * \param openFileTable The pointer to the open file table data structure.
 */
uint32_t totalOpenFiles(uint8_t *openFileTable);

/**
 * Closes all files for a particular PID in the open file table data structure.
 * \param openFileTable The pointer to the open file table data structure.
 * \param pid The PID associated with closure of the files.
 */
void closeAllFiles(uint8_t *openFileTable, uint32_t pid);

/**
 * For a particular inode, return a bool as to whether it can be locked (RDWRITE) or not.
 * \param openFileTable The pointer to the open file table data structure.
 * \param inode The inode of the file in question.
 */
bool fileAvailableToBeLocked(uint8_t *openFileTable, uint32_t inode);

/**
 * Lock a file for RDWRITE access in the open file table data structure.
 * \param openFileTable The pointer to the open file table data structure. 
 * \param pid The PID that is locking the file.
 * \param inode The inode of the file being locked.
 */
bool lockFile(uint8_t *openFileTable, uint32_t pid, uint32_t inode);