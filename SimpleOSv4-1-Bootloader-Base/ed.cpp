// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "screen.h"
#include "keyboard.h"
#include "vm.h"
#include "simpleOSlibc.h"
#include "constants.h"
#include "x86.h"
#include "file.h"

void printPrompt(uint8_t myPid)
{   
    uint8_t *shellHorizontalLine = malloc(myPid, sizeof(uint8_t));
    *shellHorizontalLine = ASCII_HORIZONTAL_LINE;
    for (uint32_t columnPos = 0; columnPos < 80; columnPos++)
    {
        printString(COLOR_LIGHT_BLUE, 18, columnPos, shellHorizontalLine);
    }

    free(shellHorizontalLine);

    printString(COLOR_GREEN, 18, 3, (uint8_t *)"Simple Editor v0.1");
    printString(COLOR_RED, 19, 2, (uint8_t *)"#");
    printString(COLOR_GREEN, 18, 62, (uint8_t *)"Current PID: ");
    printHexNumber(COLOR_GREEN, 18, 75, (uint8_t)myPid);

}

void printBufferToScreen(uint8_t *userSpaceBuffer)
{
    uint32_t linearPosition = 0;
    
    if (userSpaceBuffer == 0x0)
    {
        printString(COLOR_RED, 21, 3,(uint8_t *)"No such buffer!");
        return; //null pointer
    }
    
    
    uint32_t row = 0;
    uint32_t column = 0;

    while (row <= 15)
    {
        if (column > 79) { column = 0; row++; }
        if (*(uint8_t *)((uint32_t)userSpaceBuffer + linearPosition) == (uint8_t)0x0a) { row++; column=0; }
        if (*(uint8_t *)((uint32_t)userSpaceBuffer + linearPosition) == (uint8_t)0x0d) { column++; }
        if (*(uint8_t *)((uint32_t)userSpaceBuffer + linearPosition) == (uint8_t)0x09) { column=column+4; }

        if (row <= 15)
        {
            printCharacter(COLOR_WHITE, row, column, (uint8_t *)((int)userSpaceBuffer + linearPosition));
        }

        column++;
        linearPosition++;
    }

}

void screenEditor(uint32_t myPid, uint32_t currentFileDescriptor)
{
    uint32_t row = 0;
    uint32_t column = 0;
    uint32_t rowBasedOnBuffer = 0;
    uint32_t columnBasedOnBuffer = 0;
    struct openBufferTable *openBufferTable = (struct openBufferTable*)OPEN_BUFFER_TABLE;

    uint32_t bufferPosition = (uint32_t)(uint8_t *)openBufferTable->buffers[currentFileDescriptor];
    uint32_t bufferPositionStart = bufferPosition;

    uint8_t *bufferPositionMem = malloc(myPid, sizeof(uint8_t *));
    uint8_t *rowPositionMem = malloc(myPid, sizeof(uint8_t *));
    uint8_t *columnPositionMem = malloc(myPid, sizeof(uint8_t *));

    while ((uint8_t)*(uint8_t *)KEYBOARD_BUFFER != 0x81) //esc
    {       
        moveCursor(row + rowBasedOnBuffer, column + columnBasedOnBuffer);

        readKeyboard((uint8_t *)KEYBOARD_BUFFER);

        if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x90) {
            *(uint8_t*)bufferPosition = 0x71;
            bufferPosition++;
            } //q
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x91) {
             *(uint8_t*)bufferPosition = 0x77;
             bufferPosition++;
             } //w
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x92) {
            *(uint8_t*)bufferPosition = 0x65;
            bufferPosition++;
            } //e
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x93) {
            *(uint8_t*)bufferPosition = 0x72;
            bufferPosition++;
            } //r
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x94) {
            *(uint8_t*)bufferPosition = 0x74;
            bufferPosition++;
            } //t
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x95) {
            *(uint8_t*)bufferPosition = 0x79;
            bufferPosition++;
            } //y
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x96) {
            *(uint8_t*)bufferPosition = 0x75;
            bufferPosition++;
            } //u
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x97) {
            *(uint8_t*)bufferPosition = 0x69;
            bufferPosition++;
            } //i
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x98) {
            *(uint8_t*)bufferPosition = 0x6f;
            bufferPosition++;
            } //o
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x99) {
            *(uint8_t*)bufferPosition = 0x70;
            bufferPosition++;
            } //p
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x9e) {
            *(uint8_t*)bufferPosition = 0x61;
            bufferPosition++;
            } //a
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x9f) {
            *(uint8_t*)bufferPosition = 0x73;
            bufferPosition++;
            } //s
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xa0) {
            *(uint8_t*)bufferPosition = 0x64;
            bufferPosition++;
            } //d
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xa1) {
            *(uint8_t*)bufferPosition = 0x66;
            bufferPosition++;
            } //f
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xa2) {
            *(uint8_t*)bufferPosition = 0x67;
            bufferPosition++;
            } //g
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xa3) {
            *(uint8_t*)bufferPosition = 0x68;
            bufferPosition++;
            } //h
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xa4) {
            *(uint8_t*)bufferPosition = 0x6a;
            bufferPosition++;
            } //j
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xa5) {
            *(uint8_t*)bufferPosition = 0x6b;
            bufferPosition++;
            } //k
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xa6) {
            *(uint8_t*)bufferPosition = 0x6c;
            bufferPosition++;
            } //l
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xac) {
            *(uint8_t*)bufferPosition = 0x7a;
            bufferPosition++;
            } //z
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xad) {
            *(uint8_t*)bufferPosition = 0x78;
            bufferPosition++;
            } //x
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xae) {
            *(uint8_t*)bufferPosition = 0x63;
            bufferPosition++;
            } //c
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xaf) {
            *(uint8_t*)bufferPosition = 0x76;
            bufferPosition++;
            } //v
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xb0) {
            *(uint8_t*)bufferPosition = 0x62;
            bufferPosition++;
            } //b
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xb1) {
            *(uint8_t*)bufferPosition = 0x6e;
            bufferPosition++;
            } //n
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xb2) {
            *(uint8_t*)bufferPosition = 0x6d;
            bufferPosition++;
            } //m
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x82) {
            *(uint8_t*)bufferPosition = 0x31;
            bufferPosition++;
            } //1
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x83) {
            *(uint8_t*)bufferPosition = 0x32;
            bufferPosition++;
            } //2
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x84) {
            *(uint8_t*)bufferPosition = 0x33;
            bufferPosition++;
            } //3
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x85) {
            *(uint8_t*)bufferPosition = 0x34;
            bufferPosition++;
            } //4
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x86) {
            *(uint8_t*)bufferPosition = 0x35;
            bufferPosition++;
            } //5
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x87) {
            *(uint8_t*)bufferPosition = 0x36;
            bufferPosition++;
            } //6
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x88) {
            *(uint8_t*)bufferPosition = 0x37;
            bufferPosition++;
            } //7
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x89) {
            *(uint8_t*)bufferPosition = 0x38;
            bufferPosition++;
            } //8
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x8a) {
            *(uint8_t*)bufferPosition = 0x39;
            bufferPosition++;
            } //9
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x8b) {
            *(uint8_t*)bufferPosition = 0x30;
            bufferPosition++;
            } //0
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x9c) {
            //*(uint8_t*)bufferPosition = 0x0a;
            //bufferPosition++;
            } // enter
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xb9) {
            *(uint8_t*)bufferPosition = 0x20;
            bufferPosition++;
            } // space
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0xb4) {
            *(uint8_t*)bufferPosition = 0x2e;
            bufferPosition++;
            } // period
        else if ((uint8_t)(*(uint8_t *)KEYBOARD_BUFFER) == (uint8_t)0x8e) {
            *(uint8_t*)bufferPosition = 0x20;

            if (bufferPosition <= bufferPositionStart)
            {
                bufferPosition = bufferPositionStart;
            }
            else
            {
                bufferPosition--;
            }
            } // backspace

        
        rowBasedOnBuffer = (bufferPosition - bufferPositionStart) / 80;
        columnBasedOnBuffer = (bufferPosition - bufferPositionStart) % 80;

        if ( (uint8_t)*(uint8_t *)KEYBOARD_BUFFER == 0xCD ) //right
        { 
            if (column >= 79)
            {
                column = 0;
                row++;
            }
            else
            {
                column++;
            } 
        }
        else if ( (uint8_t)*(uint8_t *)KEYBOARD_BUFFER == 0xCB ) //left
        { 
            if (column <= 0)
            {
                if (row == 0)
                {
                    row = 0;
                    column = 0;
                }
                else
                {
                    column = 79;
                    row--;
                }
            }
            else
            {
                column--;
            }

        }
        else if ( (uint8_t)*(uint8_t *)KEYBOARD_BUFFER == 0xD0 ) //down
        { 
            row++; 
        }
        else if ( (uint8_t)*(uint8_t *)KEYBOARD_BUFFER == 0xC8 ) //up
        { 
            if (row <= 0)
            {
                row = 0;
            }
            else
            {
                row--;
            }
        }


        if (row <= 1)
        {
            fillMemory(VIDEO_RAM, 0x0, 2720);
            
            if ((bufferPosition - 160) < bufferPosition)
            {
                printBufferToScreen((uint8_t *)(bufferPositionStart));
                moveCursor(row, column);
            }

        }

        if (row >= 2 && row < 15)
        {
            fillMemory(VIDEO_RAM, 0x0, 2720);
            printBufferToScreen((uint8_t *)(bufferPositionStart + (80 * row)));
            moveCursor(row, column);
        }

        if (row >= 15)
        {
            fillMemory(VIDEO_RAM, 0x0, 2720);
            printBufferToScreen((uint8_t *)(bufferPositionStart + (80 * row)));
            moveCursor(15, column);

        }

        printString(COLOR_WHITE, 22, 13, (uint8_t *)"        ");
        itoa((uint32_t)bufferPosition, bufferPositionMem);
        printString(COLOR_RED, 22, 1, (uint8_t *)"Buffer Pos:");
        printString(COLOR_WHITE, 22, 13, bufferPositionMem);

        printString(COLOR_WHITE, 21, 13, (uint8_t *)"        ");
        itoa((uint32_t)row + rowBasedOnBuffer, rowPositionMem);
        printString(COLOR_RED, 21, 1, (uint8_t *)"Row Pos:");
        printString(COLOR_WHITE, 21, 13, rowPositionMem);

        printString(COLOR_WHITE, 21, 43, (uint8_t *)"        ");
        itoa((uint32_t)column + columnBasedOnBuffer, columnPositionMem);
        printString(COLOR_RED, 21, 31, (uint8_t *)"Col Pos:");
        printString(COLOR_WHITE, 21, 43, columnPositionMem);

    }

    free(bufferPositionMem);
    free(rowPositionMem);
    free(columnPositionMem);

}


void main()
{
    uint32_t myPid;
    bool fileContentsOnScreen = false;

    disableCursor();
    clearScreen();

    myPid = readValueFromMemLoc(RUNNING_PID_LOC);

    // clearing shared buffer area
    fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));

    systemShowOpenFiles();
    myPid = readValueFromMemLoc(RUNNING_PID_LOC);
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

        uint8_t *command = (uint8_t *)COMMAND_BUFFER;
        commandArgument1 = (uint8_t*)(COMMAND_BUFFER + strlen(COMMAND_BUFFER) + 1);
        commandArgument2 = (uint8_t*)(commandArgument1 + strlen(commandArgument1) + 1);

        // Any commands that don't take an argument, add "\n" to the end
        uint8_t *clearScreenCommand = (uint8_t *)"cls\n";
        uint8_t *openFileCommand = (uint8_t *)"open";
        uint8_t *saveFileCommand = (uint8_t *)"save";
        uint8_t *closeFileCommand = (uint8_t *)"close";
        uint8_t *viewFileCommand = (uint8_t *)"view";
        uint8_t *helpCommand = (uint8_t *)"help\n";
        uint8_t *psCommand = (uint8_t *)"ps\n";
        uint8_t *exitCommand = (uint8_t *)"exit\n";
        uint8_t *dirCommand = (uint8_t *)"ls\n";
        uint8_t *editFileCommand = (uint8_t *)"edit";
        uint8_t *switchCommand = (uint8_t *)"switch";
        uint8_t *insertCommand = (uint8_t *)"ins\n";

        if (strcmp(command, clearScreenCommand) == 0)
        {
            disableCursor();
            clearScreen();

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
        }
        else if (strcmp(command, insertCommand) == 0)
        {
            printString(COLOR_WHITE, 19, 3,(uint8_t *)"                                ");
            fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));
            
            uint32_t currentFileDescriptor;
            currentFileDescriptor = readValueFromMemLoc(CURRENT_FILE_DESCRIPTOR);

            if (fileContentsOnScreen)
            {
                screenEditor(myPid, currentFileDescriptor);
            }

        }
        else if (strcmp(command, openFileCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printPrompt(myPid);    
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            clearScreen();

            systemOpenFile(commandArgument1, RDONLY);

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            uint32_t currentFileDescriptor;
            currentFileDescriptor = readValueFromMemLoc(CURRENT_FILE_DESCRIPTOR);

            struct openBufferTable *openBufferTable = (struct openBufferTable*)OPEN_BUFFER_TABLE;
            printBufferToScreen((uint8_t *)openBufferTable->buffers[currentFileDescriptor]);

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            currentFileDescriptor = readValueFromMemLoc(CURRENT_FILE_DESCRIPTOR);
        }
        else if (strcmp(command, saveFileCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printPrompt(myPid);    
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            clearScreen();

            systemCreateFile(commandArgument1, atoi(commandArgument2));

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            uint32_t currentFileDescriptor;
            currentFileDescriptor = readValueFromMemLoc(CURRENT_FILE_DESCRIPTOR);

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            currentFileDescriptor = readValueFromMemLoc(CURRENT_FILE_DESCRIPTOR);
        }
        else if (strcmp(command, viewFileCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printPrompt(myPid);           

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            struct openBufferTable *openBufferTable = (struct openBufferTable*)OPEN_BUFFER_TABLE;
            printBufferToScreen((uint8_t *)openBufferTable->buffers[atoi(commandArgument1)]);

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            fileContentsOnScreen = true;
        }
        else if (strcmp(command, editFileCommand) == 0)
        {
            clearScreen();
            enableCursor();

            moveCursor(1, 1);

            printPrompt(myPid);    

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            systemOpenFile(commandArgument1, RDWRITE);

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            uint32_t currentFileDescriptor;
            currentFileDescriptor = readValueFromMemLoc(CURRENT_FILE_DESCRIPTOR);

            struct openBufferTable *openBufferTable = (struct openBufferTable*)OPEN_BUFFER_TABLE;
            printBufferToScreen((uint8_t *)openBufferTable->buffers[currentFileDescriptor]);

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            fileContentsOnScreen = true;

        }
        else if (strcmp(command, closeFileCommand) == 0)
        {
            disableCursor();
            clearScreen();

            fileContentsOnScreen = false;

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            printPrompt(myPid);           

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            systemCloseFile(commandArgument1);
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            clearScreen();

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, switchCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printPrompt(myPid);            

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            
            clearScreen();

            systemTaskSwitch(commandArgument1);
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, exitCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printPrompt(myPid);  

            systemExit();       
        }
        else if (strcmp(command, helpCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printString(COLOR_LIGHT_BLUE, 1, 0, (uint8_t *)"Commands available:");
            printString(COLOR_WHITE, 3, 3, (uint8_t *)"cls = Clear screen");
            printString(COLOR_WHITE, 4, 3, (uint8_t *)"open = Open a file as read-only"); 
            printString(COLOR_WHITE, 5, 3, (uint8_t *)"close = Close file"); 
            printString(COLOR_WHITE, 6, 3, (uint8_t *)"view = View a file");
            printString(COLOR_WHITE, 7, 3, (uint8_t *)"help = This menu"); 
            printString(COLOR_WHITE, 8, 3, (uint8_t *)"ps = Show processes");
            printString(COLOR_WHITE, 9, 3, (uint8_t *)"exit = Exit the Editor");
            printString(COLOR_WHITE, 10, 3, (uint8_t *)"ls = Show root directory");
            printString(COLOR_WHITE, 11, 3, (uint8_t *)"edit = Edit a file");
            printString(COLOR_WHITE, 12, 3, (uint8_t *)"switch = Switch processes");
            printString(COLOR_WHITE, 13, 3, (uint8_t *)"ins = Insert in the screen editor");
            printString(COLOR_WHITE, 14, 3, (uint8_t *)"save = Save an open file to disk");
            
        }
        else if (strcmp(command, psCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printPrompt(myPid);  
            systemShowProcesses();

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else if (strcmp(command, dirCommand) == 0)
        {
            disableCursor();
            clearScreen();
            printPrompt(myPid);  
            systemListDirectory();

            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

        }
        else
        {
            printPrompt(myPid);             
            printString(COLOR_RED, 21, 3,(uint8_t *)"Command not recognized!");
            systemBeep();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);

            fillMemory((uint8_t *)KEYBOARD_BUFFER, (uint8_t)0x0, (KEYBOARD_BUFFER_SIZE * 2));
            fillMemory((uint8_t *)SHELL_CURSOR_POS, (uint8_t)0x0, 40);
            wait(1);
            printString(COLOR_WHITE, 21, 3,(uint8_t *)"                                ");

            systemShowOpenFiles();
            myPid = readValueFromMemLoc(RUNNING_PID_LOC);
            freeAll(myPid);
            main();

        }    
    }


}