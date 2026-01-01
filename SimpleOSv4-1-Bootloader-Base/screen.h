// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "constants.h"

/**
 * Clears the screen.
 */
void clearScreen();

/**
 * Prints a character to a location on the screen.
 * \param color This is a color constant for the attribute byte.
 * \param row The row number of an 80 column by 25 row display.
 * \param colum The column number of an 80 column by 25 row display.
 * \param message This is a pointer to the character you want to display.
 */
void printCharacter(uint32_t color, uint32_t row, uint32_t column, uint8_t *message);

/**
 * Prints a null-terminated string to a location on the screen.
 * \param color This is a color constant for the attribute byte.
 * \param row The row number of an 80 column by 25 row display.
 * \param colum The column number of an 80 column by 25 row display.
 * \param message This is a pointer to the string you want to display.
 */
void printString(uint32_t color, uint32_t row, uint32_t column, uint8_t *message);

/**
 * Prints the hexadecimal value of an 8-bit (unsigned) number to a location on the screen.
 * \param color This is a color constant for the attribute byte.
 * \param row The row number of an 80 column by 25 row display.
 * \param colum The column number of an 80 column by 25 row display.
 * \param number The unsigned uint8_t value of the number to display.
 */
void printHexNumber(uint32_t color, uint32_t row, uint32_t column, uint8_t number);

/**
 * Prints the SimpleOS logo starting at a line number.
 * \param lineNumber The row to begin printing the logo.
 */
void printLogo(uint32_t lineNumber);

/**
 * Enable the blinking cursor.
 */
void enableCursor();

/**
 * Disable the blinking cursor.
 */
void disableCursor();

/**
 * Move the blinking cursor to a location on the screen.
 * \param row The row number of an 80 column by 25 row display.
 * \param colum The column number of an 80 column by 25 row display.
 */
void moveCursor(uint32_t row, uint32_t column);