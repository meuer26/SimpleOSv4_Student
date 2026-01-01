// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.



#include "constants.h"
/** The function referenced in the interrupt descriptor table when a page fault is detected. */
void pageFault();

/** The function referenced in the interrupt descriptor table when a general protection fault is detected. */
void generalProtectionFault();