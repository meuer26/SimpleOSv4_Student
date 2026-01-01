// Copyright (c) 2023-2026 Dan O’Malley
// This file is licensed under the MIT License. See LICENSE for details.


#include "constants.h"
/**
 *  Simple API to allow users to set the frequency (in Hz) and duration (in number of interrupts) for sound.
 */
struct soundParameter
{
    /** The frequency in Hz. */
    uint32_t frequency; 
    /** The duration in counts of interrupts. */
    uint32_t duration;
};


/**
 * This will generate a tone for one interrupt. You should not call this directly. Call makeSound() in SimpleOSLibc instead.
 * \param frequency The frequency in Hz.
 */
void generateTone(uint32_t frequency);

/**
 * This will stop a tone that is already playing. You should not call this directly. Call makeSound() in SimpleOSLibc instead.
 */
void stopTone();