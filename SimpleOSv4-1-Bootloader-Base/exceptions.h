// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.

#include "constants.h"
/**
 * Panics the kernel with a blue screen and enters an infinite loop.
 * \param message The message to display in the blue screen.
 */
void panic(uint8_t *message);