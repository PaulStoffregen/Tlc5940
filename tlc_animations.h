/*  Copyright (c) 2009 by Alex Leone <acleone ~AT~ gmail.com>

    This file is part of the Arduino TLC5940 Library.

    The Arduino TLC5940 Library is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    The Arduino TLC5940 Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Arduino TLC5940 Library.  If not, see
    <http://www.gnu.org/licenses/>. */

#ifndef TLC_ANIMATIONS_H
#define TLC_ANIMATIONS_H

/** \file
    TLC Animation functions.  These play animations from PROGMEM. */

#include <avr/pgmspace.h>
#include <avr/io.h>

#include "tlc_config.h"
#include "Tlc5940.h"
#include "tlc_progmem_utils.h"

/** The currently playing animation */
prog_uint8_t *tlc_currentAnimation;
/** The number of frames in the current animation */
volatile uint16_t tlc_animationFrames;
/** The number of PWM periods to display each frame - 1 */
volatile uint16_t tlc_animationPeriodsPerFrame;
/** The current number of periods we've displayed this frame for */ 
volatile uint16_t tlc_animationPeriodsWait;

volatile void tlc_animationXLATCallback(void);
void tlc_playAnimation(prog_uint8_t *animation, uint16_t frames, uint16_t periodsPerFrame);

/** \addtogroup ExtendedFunctions
    \code #include "tlc_animations.h" \endcode
    - void tlc_playAnimation(prog_uint8_t *animation, uint16_t frames,
            uint16_t periodsPerFrame) - plays an animation from progmem. */
/* @{ */

/** Plays an animation from progmem in the "background" (with interrupts).
    \param animation A progmem array of grayscale data, length NUM_TLCS *
           24 * frames, in reverse order.  Ensure that there is not an update
           waiting to happen before calling this.
    \param frames the number of frames in animation
    \param periodsPerFrame number of PWM periods to wait between each frame
           (0 means play the animation as fast as possible).
           The default PWM period for a 16MHz clock is 1.024ms. */
void tlc_playAnimation(prog_uint8_t *animation, uint16_t frames, uint16_t periodsPerFrame)
{
    tlc_currentAnimation = animation;
    tlc_animationFrames = frames;
    tlc_animationPeriodsPerFrame = periodsPerFrame;
    tlc_animationPeriodsWait = 0;
    tlc_onUpdateFinished = tlc_animationXLATCallback;
    tlc_animationXLATCallback();
}

/** This is called by the XLAT interrupt every PWM period to do stuff. */
volatile void tlc_animationXLATCallback(void)
{
    if (tlc_animationPeriodsWait) {
        tlc_animationPeriodsWait--;
        set_XLAT_interrupt();
    } else {
        if (tlc_animationFrames) {
            tlc_setGSfromProgmem(tlc_currentAnimation +
                                (--tlc_animationFrames * NUM_TLCS * 24));
            tlc_animationPeriodsWait = tlc_animationPeriodsPerFrame;
            Tlc.update();
        } else { // animation is done
            tlc_onUpdateFinished = 0;
        }
    }
}

/* @} */

#endif
