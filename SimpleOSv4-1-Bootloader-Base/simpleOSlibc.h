// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "constants.h"

/**
 * This is the heap object. It is used for both user heap objects and kernel heap objects.
 */
struct heapObject
{
    /** The PID is stored and used as a non-zero indicator to allow the heap allocator to know if that heap object position is used. This is done for user and kernel heap objects alike. */
    uint8_t pid; 
    /** The size of the heap object. If you need more use systemMMap() instead. */
    uint8_t heapUseable[HEAP_OBJ_USABLE_SIZE];
    /** Always left null to accommodate string null bytes */
    uint8_t nullByte; 
};

/**
 * The time structure. This is used in the Unix epoch conversion for convertToUnixTime() and convertFromUnixTime().
 */
struct time
{
    /** The four digit year */
    uint32_t year;
    /** The day of the year */
    uint32_t dayOfYear; 
    /** The hour of the day in the 24-hour clock */
    uint32_t hour;
    /** The minute */
    uint32_t min;
    /** The second */
    uint32_t sec;
};

/** 
 * Force the program into a busy loop for the number of interrupts in the parameter.
 * \param timeToWait The number of interrupts to wait.
 */
void wait(uint32_t timeToWait);

/**
 * Compare two strings and returns a value. 
 * Returns < 0 if firstString < secondString;
 * Returns 0 if firstString == secondString;
 * Returns > 0 if secondString > firstString.
 * \param firstString The first string.
 * \param secondString Compare this string to the first string.
 */
uint32_t strcmp(uint8_t *firstString, uint8_t *secondString);

/**
 * Returns the length of the string.
 * \param targetString The string to measure.
 */
uint32_t strlen(uint8_t *targetString);

/**
 * Copy a source string to a destination string.
 * \param destinationString The destination.
 * \param sourceString The source.
 */
void strcpy(uint8_t *destinationString, uint8_t *sourceString);

/**
 * Copies a string and stops when a newline character is detected.
 * \param destinationString The destination.
 * \param sourceString The source.
 */
void strcpyRemoveNewline(uint8_t *destinationString, uint8_t *sourceString);

/**
 * Copy bytes from one location to another in memory.
 * \param destinationMemory The target location in memory.
 * \param sourceMemory The source location in memory.
 * \param numberOfBytes How many bytes to copy from source to target.
 */
void bytecpy(uint8_t *destinationMemory, uint8_t *sourceMemory, uint32_t numberOfBytes);

/**
 * Convert an integer to an ASCII-equivalent string value of that integer.
 * \param number The integer to convert.
 * \param destinationMemory The target location for the string-converted integer (you may want to call malloc first to allocate space).
 */
void itoa(uint32_t number, uint8_t *destinationMemory);

/**
 * Convert an ASCII string of a number to an integer. The opposite of itoa(). Returns the integer.
 * \param sourceString The string to convert to an integer.
 */
uint32_t atoi(uint8_t *sourceString);

/**
 * Compute the ceiling of a number given some base. Returns ceiling value.
 * \param number The input number.
 * \param base The base.
 */
uint32_t ceiling(uint32_t number, uint32_t base);

/**
 * Allocate a user heap object.
 * \param currentPid The current pid. This is needed to show whether or not the heap object is allocated even though all objects in the same PID will have the same value.
 * \param objectSize The size of the object requested. If this requested size is larger than the heap object, a null pointer is returned. Be sure to check for that. If you need a larger object, use systemMMap().
 */
uint8_t *malloc(uint32_t currentPid, uint32_t objectSize);

/**
 * Free a heap object. See malloc().
 * \param heapObject The pointer to the heap object you want to free.
 */
void free(uint8_t *heapObject);

/**
 * Free all heap objects for a certain PID. Be very careful with this as it can lead to many memory problems, such as use-after-free (UAF) bugs.
 * \param currentPid The current PID.
 */
void freeAll(uint32_t currentPid);

/**
 * The kernel equivalent of malloc(). See kFree() also. The object size and parameters are the same as the userspace heap but the objects exist in kernel space and are for kernel use. Returns a pointer in kernel space if successful.
 * \param currentPid The pid associated with the object. This is important as you want to free all kernel heap objects associated with a killed/zombie/exited process.
 * \param objectSize The requested size of the heap object. Same as malloc().
 */
uint8_t *kMalloc(uint32_t currentPid, uint32_t objectSize);

/**
 * The kernel equivalent of free(). See kMalloc().
 * \param heapObject The pointer to the heap object you want to free in kernel space.
 */
void kFree(uint8_t *heapObject);

/**
 * Free all kernel heap objects for a certain PID.
 * \param currentPid The PID associated with kernel heap objects you want to free.
 */
void kFreeAll(uint32_t currentPid);

/**
 * Generates a simple hash based on an input string. Not cryptographically secure.
 * \param messageToHash The input string.
 */
uint32_t stringHash(uint8_t *messagetoHash);

/**
 * Plays a sound at a certain frequency for a certain duration.
 * \param frequency Frequency in Hz.
 * \param duration Duration in number of interrupts.
 */
void makeSound(uint32_t frequency, uint32_t duration);

/**
 * The LibC wrapper for the SYS_UPTIME sysCall(). It displays the system uptime.
 */
void systemUptime();

/**
 * The LibC wrapper for the SYS_EXIT sysCall(). This terminates the current process and returns to the parent process.
 */
void systemExit();

/**
 * The LibC wrapper for the SYS_FREE sysCall(). This frees all data structures and memory associated with killed/zombie processes.
 */
void systemFree();

/**
 * The LibC wrapper for the SYS_MMAP sysCall(). This function will return a pointer (if successful) to a new and available page of user space memory somewhere in the address space of the process. Uses first-fit algorithm.
 */
uint8_t *systemMMap();

/**
 * The LibC wrapper for the SYS_SWITCH_TASK_TO_PARENT sysCall(). This will switch to the parent, putting the child to sleep without terminating the child process.
 */
void systemSwitchToParent();

/**
 * The LibC wrapper for the SYS_BEEP sysCall(). This will generate a one-interrupt error beep.
 */
void systemBeep();

/**
 * The LibC wrapper for the SYS_PS sysCall(). This will display the informational screen about processes, objects, memory, etc.
 */
void systemShowProcesses();

/**
 * The LibC wrapper for the SYS_DIR sysCall(). This will display the contents of the current directory.
 */
void systemListDirectory();

/**
 * The LibC wrapper for the SYS_TOGGLE_SCHEDULER sysCall(). This will turn on the scheduler/disbatch functionality if it is off, or it will turn it off if it is on.
 */
void systemSchedulerToggle();

/**
 * The Libc wrapper for the SYS_FORK_EXEC sysCall(). This will take a file name from the file system and a requested run priority and launch it.
 * \param fileName The string of a file name on the file system.
 * \param requestedRunPriority An 8-bit value to assign as the priority of the process. If left blank, the priority will be zero.
 */
void systemForkExec(uint8_t *fileName, uint32_t requestedRunPriority);

/**
 * The LibC wrapper for the SYS_KILL sysCall(). This will kill a process. You cannot kill yourself and you cannot kill PID=1.
 * \param pidToKill The PID to kill.
 */
void systemKill(uint8_t *pidToKill);

/**
 * The LibC wrapper for the SYS_SWITCH_TASK sysCall(). This will switch to a certain process and put the current process to sleep.
 * \param pidToSwitchTo The PID to switch to.
 */
void systemTaskSwitch(uint8_t *pidToSwitchTo);

/**
 * The LibC wrapper for the SYS_MEM_DUMP sysCall(). Takes the memory location in decimal and displays it.
 * \param memoryLocation Takes a memory location in decimal.
 */
void systemShowMemory(uint8_t *memoryLocation);

/**
 * The LibC wrapper for the SYS_SHOW_OPEN_FILES sysCall(). This will display the open files for the current process to the screen, along with their permissions.
 */
void systemShowOpenFiles();

/**
 * The LibC wrapper for the SYS_CREATE sysCall(). This will create a new file on the file system based on the contents of a file descriptor buffer.
 */
void systemCreateFile(uint8_t *fileName, uint32_t fileDescriptor);

/**
 * The LibC wrapper for the SYS_DELETE sysCall(). This will delete a file on the file system based on file name.
 */
void systemDeleteFile(uint8_t *fileName);

/**
 * The LibC wrapper for the SYS_OPEN sysCall(). This will open a filename on the file system with the requested permissions.
 * \param fileName The string value of the filename on the file system.
 * \param requestPermissions The permissions are constants in constants.h. They are typically RDWRITE for read/write or RDONLY for read only.
 */
void systemOpenFile(uint8_t *fileName, uint32_t requestedPermissions);

/**
 * The LibC wrapper for the SYS_OPEN_EMPTY sysCall(). This will open a buffer of the requested size in pages. Used for created brand new text files.
 * \param fileName The string value of the filename for the open file table.
 * \param requestSizeInPages The size of the buffer in pages.
 */
void systemOpenEmptyFile(uint8_t *fileName, uint32_t requestedSizeInPages);

/**
 * The LibC wrapper for the SYS_CLOSE sysCall(). This will close a file given a file descriptor number.
 * \param fileDescriptor The file descriptor to close.
 */
void systemCloseFile(uint8_t *fileDescriptor);

/**
 * Converts octal permissions in EXT2 to RWX string.
 * \param permissions The raw byte octal permissions.
 */
uint8_t *octalTranslation(uint8_t permissions);

/**
 * Converts directory entry types (e.g., file, directories, etc) to their string value.
 * \param type The raw type of the directory entry.
 */
uint8_t *directoryEntryTypeTranslation(uint8_t type);

/**
 * Counts the number of active heap objects. Returns the count.
 * \param heapLoc The pointer to the heap location.
 */
uint32_t countHeapObjects(uint8_t *heapLoc);

/** 
 * Returns the Unix Epoch value (seconds since 1970) given a time struct.
 * \param Time The time struct.
 */
uint32_t convertToUnixTime(struct time* Time);

/**
 * Given the Unix Epoch value (seconds since 1970), return the time struct.
 * \param unixTime Seconds since 1970.
 */
time *convertFromUnixTime(uint32_t unixTime);