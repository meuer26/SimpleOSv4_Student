// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "constants.h"
/**
 * The main system call handler routine that runs in the kernel during each system call interrupt.
 */
void syscallHandler();

/**
 * The kernel routine that executes a sound.
 * \param SoundParameter It takes the sound parameter structure pointer as the argument.
 */
void sysSound(struct soundParameter *SoundParameter);

/**
 * The kernel routine that executes an error beep.
 */
void sysBeep();

/** The kernel routine that prints a file descriptor to the screen.
 * \param fileDescriptorToPruint32_t The file descriptor to pruint32_t to the screen.
 * \param currentPid The pid of the process requesting this action.
 */
void sysWrite(uint32_t fileDescriptorToPrint, uint32_t currentPid);

/** The kernel routine to open a file.
 * \param FileParameter The file parameter structure with the file specifics.
 * \param currentPid The pid of the process requesting this action.
 */
void sysOpen(struct fileParameter *FileParameter, uint32_t currentPid);

/** The kernel routine to close a file.
 * \param fileDescriptor The file descriptor to close.
 * \param currentPid The pid of the process requesting this action.
 */
void sysClose(uint32_t fileDescriptor, uint32_t currentPid);

/** The kernel routine that prints the current processes to the screen.
 * \param currentPid The pid of the process requesting this action.
 */
void sysPs(uint32_t currentPid);

/** The kernel routine that runs a new program. 
 * \param FileParameter The file parameter structure with the file specifics.
 * \param currentPid The pid of the process requesting this action.
*/
void sysForkExec(struct fileParameter *FileParameter, uint32_t currentPid);

/** The kernel routine that exits the current process.
 * \param currentPid The pid of the process requesting this action.
 */
void sysExit(uint32_t currentPid);

/** The kernel routine that frees memory and frames for killed or zombie processes.
 * \param currentPid The pid of the process requesting this action.
 */
void sysFree(uint32_t currentPid);

/** The kernel routine that kills a process.
 * \param pidToKill The pid to kill.
 */
void sysKill(uint32_t pidToKill);

/** The kernel routine to switch to another process and puts the current process to sleep.
 * \param pidToSwitchTo The pid to switch to.
 * \param currentPid The pid of the process requesting this action.
 */
void sysSwitch(uint32_t pidToSwitchTo, uint32_t currentPid);

/** Display the contents of a memory address to the screen.
 * \param memAddressToDump The memory address to display.
 */
void sysMemDump(uint32_t memAddressToDump);

/** The main system interrupt handler. This function is called each time an interrupt is triggered. */
void systemInterruptHandler();

/** Prints the the uptime to the screen.
 * \param uptimeSeconds The uptime in seconds.
 * \param uptimeMinutes The uptime in minutes.
 */
void printSystemUptime(uint32_t uptimeSeconds, uint32_t uptimeMinutes);

/** The kernel routine wrapper for printSystemUptime() */
void sysUptime();

/** The kernel routine that switches to the parent of the running process. Puts the child to sleep.
 * \param currentPid The pid of the process requesting this action.
 */
void sysSwitchToParent(uint32_t currentPid);

/** The kernel routine that waits one second. This is computed based on number of interrupts per second. */
void sysWait();

/** The kernel routine that waits for one interrupt. The length of this action varies based on the system's interrupts per second. */
void sysWaitOneInterrupt();

/** The kernel routine that prints the directory contents to the screen.
 * \param currentPid The pid of the process requesting this action.
 */
void sysDirectory(uint32_t currentPid);

/** The kernel routine that toggles the schedule. This is done by modifying the kernelConfiguration structure. */
void sysToggleScheduler();

/** The kernel routine that shows the open files for that process.
 * \param currentPid The pid of the process requesting this action.
 */
void sysShowOpenFiles(uint32_t currentPid);

/** The kernel routine that creates a new file.
 * \param FileParameter The file parameter structure with the file specifics.
 * \param currentPid The pid of the process requesting this action.
 */
void sysCreate(struct fileParameter *FileParameter, uint32_t currentPid); 

/** The kernel routine that deletes a file.
 * \param FileParameter The file parameter structure with the file specifics.
 * \param currentPid The pid of the process requesting this action.
 */
void sysDelete(struct fileParameter *FileParameter, uint32_t currentPid);

/** The kernel routine that opens a new and empty file descriptor.
 * \param FileParameter The file parameter structure with the file specifics.
 * \param currentPid The pid of the process requesting this action. 
 */ 
void sysOpenEmpty(struct fileParameter *FileParameter, uint32_t currentPid);
