// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "constants.h"
/** The main scheduler and dispatch function.
 * \param currentPid The pid at the time of the interrupt (which also runs the scheduler).
 */
void scheduler(uint32_t currentPid);