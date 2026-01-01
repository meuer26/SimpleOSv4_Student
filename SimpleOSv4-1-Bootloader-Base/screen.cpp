// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.

// https://wiki.osdev.org/Printing_To_Screen

#include "screen.h"
#include "x86.h"
#include "constants.h"

void clearScreen()
{       
    
    // ASSIGNMENT 1 TO DO

}


void printCharacter(uint32_t color, uint32_t row, uint32_t column, uint8_t *message)
{
    
    // ASSIGNMENT 1 TO DO

}

void printString(uint32_t color, uint32_t row, uint32_t column, uint8_t *message)
{

    // ASSIGNMENT 1 TO DO

}


void printHexNumber(uint32_t color, uint32_t row, uint32_t column, uint8_t number)
{

    // ASSIGNMENT 1 TO DO
}


void printLogo(uint32_t lineNumber)
{
        printString(12, lineNumber++, 0,(uint8_t *)"                                                __                      _   __");
        printString(12, lineNumber++, 0,(uint8_t *)"                                               (_  o ._ _  ._  |  _    / \\ (_ ");
        printString(12, lineNumber++, 0,(uint8_t *)"                                               __) | | | | |_) | (/_   \\_/ __)");
        printString(12, lineNumber, 0,(uint8_t *)"                                                           |                  ");
        printString(9, lineNumber, 65, (uint8_t *)"version 4.0");
}

void enableCursor()
{
    outputIOPort(0x3D4, 0x0A);
    outputIOPort(0x3D5, (inputIOPort(0x3D5) & 0xC0) | 0xD); //scan line start

    outputIOPort(0x3D4, 0x0B);
    outputIOPort(0x3D5, (inputIOPort(0x3D5) & 0xE0) | 0xF); //scan line stop
}

void disableCursor()
{
    outputIOPort(0x3D4, 0x0A);
    outputIOPort(0x3D5, 0x20);
}

void moveCursor(uint32_t row, uint32_t column)
{
    uint16_t cursorPosition = row * 80 + column;

    outputIOPort(0x3D4, 0x0F);
	outputIOPort(0x3D5, (uint8_t)((uint16_t)cursorPosition & 0xFF));
    outputIOPort(0x3D4, 0x0E);
	outputIOPort(0x3D5, (uint8_t)(((uint16_t)cursorPosition >> 8) & 0xFF));

}