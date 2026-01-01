// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "simpleOSlibc.h"

void main()
{   
    for (uint32_t x=0; x <10; x++)
    {
        systemShowProcesses();
        wait(1);
    }

    systemExit();
}