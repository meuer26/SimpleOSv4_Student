// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "screen.h"
#include "keyboard.h"
#include "vm.h"
#include "simpleOSlibc.h"
#include "constants.h"
#include "x86.h"
#include "file.h"
#include "sound.h"

void printPrompt(uint8_t myPid)
{
    printLogo(20);
    
    uint8_t *shellHorizontalLine = malloc(myPid, sizeof(uint8_t));
    *shellHorizontalLine = ASCII_HORIZONTAL_LINE;
    for (uint32_t columnPos = 0; columnPos < 80; columnPos++)
    {
        printString(COLOR_LIGHT_BLUE, 18, columnPos, shellHorizontalLine);
    }

    free(shellHorizontalLine);

    printString(COLOR_GREEN, 18, 3, (uint8_t *)"Simple OS Shell v2.0");
    printString(COLOR_LIGHT_BLUE, 19, 1, (uint8_t *)"/$");
    printString(COLOR_GREEN, 18, 62, (uint8_t *)"Current PID: ");
    printHexNumber(COLOR_GREEN, 18, 75, (uint8_t)myPid);

}



void main()
{
    uint32_t myPid;

    disableCursor();

    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    clearScreen();

    // clearing shared buffer area
    fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));
    uint8_t *commandArgument1 = malloc(myPid, 10);
    uint8_t *commandArgument2 = malloc(myPid, 10);

    while (true)
    {

        uint8_t *bufferMem = (uint8_t *)KEYBOARD_BUFFER;
        uint8_t *cursorMemory = (uint8_t *)SHELL_CURSOR_POS;

        myPid = readValueFromMemLoc(RUNNING_PID_LOC);     
        
        fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));

        printPrompt(myPid);     
        readCommand(bufferMem, cursorMemory);

        uint8_t *command = COMMAND_BUFFER;

        commandArgument1 = (uint8_t *)(COMMAND_BUFFER + strlen(COMMAND_BUFFER) + 1);
        commandArgument2 = (uint8_t *)(commandArgument1 + strlen(commandArgument1) + 1);

        // Any commands that don't take an argument, add "\n" to the end
        uint8_t *clearScreenCommand = (uint8_t *)"cls\n"; 
        uint8_t *uptimeCommand = (uint8_t *)"uptime\n";
        uint8_t *panicCommand = (uint8_t *)"panic\n";
        uint8_t *helpCommand = (uint8_t *)"help\n";
        uint8_t *musicCommand = (uint8_t *)"music\n";
        uint8_t *psCommand = (uint8_t *)"ps\n";
        uint8_t *runCommand = (uint8_t *)"run";
        uint8_t *deleteCommand = (uint8_t *)"rm";
        uint8_t *newCommand = (uint8_t *)"new";
        uint8_t *exitCommand = (uint8_t *)"exit\n";
        uint8_t *freeCommand = (uint8_t *)"free\n";
        uint8_t *mmapCommand = (uint8_t *)"mmap\n";
        uint8_t *killCommand = (uint8_t *)"kill";
        uint8_t *switchCommand = (uint8_t *)"switch";
        uint8_t *memCommand = (uint8_t *)"mem";
        uint8_t *parentCommand = (uint8_t *)"parent\n";
        uint8_t *dirCommand = (uint8_t *)"ls\n";
        uint8_t *schedCommand = (uint8_t *)"sched\n";

        if (strcmp(command, clearScreenCommand) == 0)
        {
            clearScreen();
        }
        else if (strcmp(command, uptimeCommand) == 0)
        {
            clearScreen();
            systemUptime();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, panicCommand) == 0)
        {
            asm volatile ("jmp $0x50,$0x0\n\t"); //picking a random far jump to cause a triple fault

        }
        else if (strcmp(command, musicCommand) == 0)
        {
            for (uint32_t x=0; x < 3; x++)
            {
                makeSound(262,6); // This is a libc function 
                makeSound(294,3);
                makeSound(262,3);
                makeSound(349,3);
                makeSound(330,3); // Makes no sound
                makeSound(262,6);
                makeSound(294,6);
                makeSound(330,18); // Makes no sound
            }

            clearScreen();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            printPrompt(myPid);

        }
        else if (strcmp(command, exitCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);  
            systemExit();            
        }
        else if (strcmp(command, helpCommand) == 0)
        {
            clearScreen();
            printString(COLOR_LIGHT_BLUE, 1, 0, (uint8_t *)"Commands available:");
            printString(COLOR_WHITE, 3, 3, (uint8_t *)"cls = Clear screen");
            printString(COLOR_WHITE, 4, 3, (uint8_t *)"uptime = System uptime"); 
            printString(COLOR_WHITE, 5, 3, (uint8_t *)"reboot = Reboot the system");
            printString(COLOR_WHITE, 6, 3, (uint8_t *)"music = Play a simple song"); 
            printString(COLOR_WHITE, 7, 3, (uint8_t *)"ps = Show processes");
            printString(COLOR_WHITE, 8, 3, (uint8_t *)"run = Launch new binary, with priority");
            printString(COLOR_WHITE, 9, 3, (uint8_t *)"exit = Exit the Shell");
            printString(COLOR_WHITE, 10, 3, (uint8_t *)"free = Free all zombie processes");
            printString(COLOR_WHITE, 11, 3, (uint8_t *)"mmap = Request a page of memory");
            printString(COLOR_WHITE, 12, 3, (uint8_t *)"kill = Kill a process");
            printString(COLOR_WHITE, 13, 3, (uint8_t *)"switch = Switch processes");
            printString(COLOR_WHITE, 3, 47, (uint8_t *)"mem = Show memory");
            printString(COLOR_WHITE, 4, 47, (uint8_t *)"parent = Switch to parent");
            printString(COLOR_WHITE, 5, 47, (uint8_t *)"ls = Show root directory");
            printString(COLOR_WHITE, 6, 47, (uint8_t *)"sched = Toggle kernel scheduler");
            printString(COLOR_WHITE, 7, 47, (uint8_t *)"rm = delete a file");
            printString(COLOR_WHITE, 8, 47, (uint8_t *)"new = create a new empty file");
            
        }
        else if (strcmp(command, freeCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);  

            systemFree();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            clearScreen();
            printString(COLOR_WHITE, 1, 5, (uint8_t *)"All zombie processes freed, along with their page frames...");

        }
        else if (strcmp(command, mmapCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);  

            systemMMap();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
        }
        else if (strcmp(command, killCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);             
            systemKill(commandArgument1);
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
        }
        else if (strcmp(command, switchCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);            

            systemTaskSwitch(commandArgument1);
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, parentCommand) == 0)
        {
            clearScreen();
            systemSwitchToParent();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, memCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);            
            printString(COLOR_WHITE, 1, 2, (uint8_t *)"Type the memory location (in decimal) you wish to see and press <Enter>...");
            fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));

            readCommand(bufferMem, cursorMemory);
            clearScreen();

            systemShowMemory(COMMAND_BUFFER);
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, psCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);  
            systemShowProcesses();

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, runCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);         
            
            if (atoi(commandArgument2) == 0x0)
            {
                clearScreen();
                printPrompt(myPid);             
                printString(COLOR_RED, 1, 3,(uint8_t *)"Please specify the run priority...");
                printString(COLOR_RED, 3, 3,(uint8_t *)"Ex: run myprog 30");
                myPid = readValueFromMemLoc(RUNNING_PID_LOC);

                fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));
                wait(3);

                clearScreen();

                freeAll(myPid);
                main();
            }  
            else if (atoi(commandArgument2) > 255)
            {
                clearScreen();
                printPrompt(myPid);             
                printString(COLOR_RED, 1, 3,(uint8_t *)"Run priority too high...must be <= 255.");
                myPid = readValueFromMemLoc(RUNNING_PID_LOC);

                fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));
                wait(2);

                clearScreen();

                freeAll(myPid);
                main();
            }

            systemForkExec(commandArgument1, atoi(commandArgument2));

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            freeAll(myPid);
            main(); // Seems to page fault when returning back from launched process without this

        }
        else if (strcmp(command, deleteCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);      

            uint8_t *commandArgument1NoNewLine = malloc(myPid, sizeof(int));
            strcpyRemoveNewline(commandArgument1NoNewLine, commandArgument1);

            systemDeleteFile(commandArgument1NoNewLine);

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            freeAll(myPid);
            main(); // Seems to page fault when returning back from launched process without this

        }
        else if (strcmp(command, newCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);       

            if (atoi(commandArgument2) == 0x0)
            {
                clearScreen();
                printPrompt(myPid);             
                printString(COLOR_RED, 1, 3,(uint8_t *)"Please specify size of the file in pages...");
                myPid = readValueFromMemLoc(RUNNING_PID_LOC);

                fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));
                wait(2);

                clearScreen();

                freeAll(myPid);
                main();
            }

            systemOpenEmptyFile(commandArgument1, atoi(commandArgument2));

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            freeAll(myPid);
            main(); // Seems to page fault when returning back from launched process without this

        }
        else if (strcmp(command, dirCommand) == 0)
        {
            clearScreen();
            printPrompt(myPid);  
            systemListDirectory();

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, schedCommand) == 0)
        {
            clearScreen();
            systemSchedulerToggle();

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else
        {
            clearScreen();
            printPrompt(myPid);             
            printString(COLOR_RED, 1, 3,(uint8_t *)"Command not recognized!");
            systemBeep();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));
            wait(1);

            clearScreen();

            freeAll(myPid);
            main();
        }    

    }


}