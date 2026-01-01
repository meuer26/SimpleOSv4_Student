// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "keyboard.h"
#include "constants.h"
#include "x86.h"
#include "screen.h"


void readKeyboard(uint8_t *ioMemory)
{
    uint8_t scanCode;

    //status check
    while ((inputIOPort(KEYBOARD_STATUS_PORT) & 0x1) == 0) {}

    // I need to read twice to avoid duplicate scan codes
    inputIOPort(KEYBOARD_DATA_PORT); // Discarding first read

    //status check
    while ((inputIOPort(KEYBOARD_STATUS_PORT) & 0x1) == 0) {}

    scanCode = inputIOPort(KEYBOARD_DATA_PORT);
    *ioMemory = ((uint8_t)scanCode);
}

void readCommand(uint8_t *ioMemory, uint8_t *vgaMemory)
{
    uint32_t cursorPosition = 4;

    while ((uint8_t)(*(ioMemory -1)) != (uint8_t)0x9c) //enter 
    {
        *vgaMemory = 0x0;    // need a seed value
        *(vgaMemory +1) = 15;

        enableCursor();
        moveCursor(19, cursorPosition);

        *(vgaMemory + 2) = 0x0;
        *(vgaMemory + 3) = 0x0;

        readKeyboard(ioMemory);

        if ((uint8_t)(*ioMemory) == (uint8_t)0x90) {
            *vgaMemory = 0x71;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x71;
            } //q
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x91) {
            *vgaMemory = 0x77;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x77;
            } //w
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x92) {
            *vgaMemory = 0x65;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x65;
            } //e
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x93) {
            *vgaMemory = 0x72;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x72;
            } //r
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x94) {
            *vgaMemory = 0x74;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x74;
            } //t
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x95) {
            *vgaMemory = 0x79;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x79;
            } //y
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x96) {
            *vgaMemory = 0x75;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x75;
            } //u
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x97) {
            *vgaMemory = 0x69;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x69;
            } //i
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x98) {
            *vgaMemory = 0x6f;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x6f;
            } //o
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x99) {
            *vgaMemory = 0x70;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x70;
            } //p
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x9e) {
            *vgaMemory = 0x61;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x61;
            } //a
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x9f) {
            *vgaMemory = 0x73;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x73;
            } //s
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xa0) {
            *vgaMemory = 0x64;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x64;
            } //d
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xa1) {
            *vgaMemory = 0x66;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x66;
            } //f
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xa2) {
            *vgaMemory = 0x67;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x67;
            } //g
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xa3) {
            *vgaMemory = 0x68;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x68;
            } //h
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xa4) {
            *vgaMemory = 0x6a;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x6a;
            } //j
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xa5) {
            *vgaMemory = 0x6b;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x6b;
            } //k
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xa6) {
            *vgaMemory = 0x6c;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x6c;
            } //l
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xac) {
            *vgaMemory = 0x7a;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x7a;
            } //z
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xad) {
            *vgaMemory = 0x78;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x78;
            } //x
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xae) {
            *vgaMemory = 0x63;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x63;
            } //c
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xaf) {
            *vgaMemory = 0x76;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x76;
            } //v
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xb0) {
            *vgaMemory = 0x62;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x62;
            } //b
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xb1) {
            *vgaMemory = 0x6e;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x6e;
            } //n
        else if ((uint8_t)(*ioMemory) == (uint8_t)0xb2) {
            *vgaMemory = 0x6d;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x6d;
            } //m
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x82) {
            *vgaMemory = 0x31;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x31;
            } //1
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x83) {
            *vgaMemory = 0x32;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x32;
            } //2
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x84) {
            *vgaMemory = 0x33;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x33;
            } //3
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x85) {
            *vgaMemory = 0x34;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x34;
            } //4
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x86) {
            *vgaMemory = 0x35;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x35;
            } //5
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x87) {
            *vgaMemory = 0x36;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x36;
            } //6
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x88) {
            *vgaMemory = 0x37;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x37;
            } //7
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x89) {
            *vgaMemory = 0x38;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x38;
            } //8
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x8a) {
            *vgaMemory = 0x39;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x39;
            } //9
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x8b) {
            *vgaMemory = 0x30;
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x30;
            } //0
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x9c) {
            *(ioMemory + KEYBOARD_BUFFER_SIZE) = 0x0a;
            } // enter
        
        else if ((uint8_t)(*ioMemory) == (uint8_t)0x8e) //backspace
        {
            if (ioMemory == (uint8_t *)KEYBOARD_BUFFER)
            {
                // Do nothing, we are at the beginning of the buffer
            }
            else
            {
                *(vgaMemory -1) = 0x0;
                *(vgaMemory -2) = 0x0;
                vgaMemory = vgaMemory - 4;

                *(ioMemory -1) = 0x0;
                *(ioMemory -2) = 0x0;
                ioMemory = ioMemory - 2;
                cursorPosition = cursorPosition-2;
            }

        }

        vgaMemory++;
        *vgaMemory = COLOR_WHITE;
        vgaMemory++;
        ioMemory++;
        cursorPosition++;

    }
}