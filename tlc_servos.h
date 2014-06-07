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

#ifndef TLC_SERVOS_H
#define TLC_SERVOS_H

/** \file
    TLC servo functions. */

#include <avr/io.h>
#include "Tlc5940.h"

#ifndef SERVO_MAX_ANGLE
/** The maximum angle of the servo. */
#define SERVO_MAX_ANGLE     180
#endif
#ifndef SERVO_MIN_WIDTH
/** The 1ms pulse width for zero degrees (0 - 4095). */
#define SERVO_MIN_WIDTH     204
#endif
#ifndef SERVO_MAX_WIDTH
/** The 2ms pulse width for 180 degrees (0 - 4095). */
#define SERVO_MAX_WIDTH     410
#endif
#ifndef SERVO_TIMER1_TOP
/** The top value for XLAT and BLANK pulses.  This is with the div8 prescale,
    so
    \f$\displaystyle f_{PWM} = \frac{f_{osc}}{2 * 8 * SERVO\_TIMER1\_TOP} \f$
    The default is 20000, which corresponds to 50Hz. */
#define SERVO_TIMER1_TOP    20000
#endif
#ifndef SERVO_TIMER2_TOP
/** The top value for GSCLK pulses.  Related to SERVO_TIMER1_TOP by
    \f$\displaystyle SERVO\_TIMER2\_TOP =
        \frac{2 * 8 * SERVO\_TIMER1\_TOP}{4096} - 1 \f$
    The default is 77. */
#define SERVO_TIMER2_TOP    77
#endif

void tlc_initServos(uint8_t initAngle = 0);
void tlc_setServo(TLC_CHANNEL_TYPE channel, uint8_t angle);
uint8_t tlc_getServo(TLC_CHANNEL_TYPE channel);
uint16_t tlc_angleToVal(uint8_t angle);
uint8_t tlc_valToAngle(uint16_t value);

/** \addtogroup ExtendedFunctions
    \code #include "tlc_servos.h" \endcode
    - void tlc_initServos(uint8_t initAngle = 0) - initializes the tlc for
            servos.
    - void tlc_setServo(TLC_CHANNEL_TYPE channel, uint8_t angle) - sets a
            servo to an angle
    - uint8_t tlc_getServo(TLC_CHANNEL_TYPE channel) - gets the currently set
            servo angle */
/* @{ */

/** Initializes the tlc.
    \param initAngle the initial angle to set all servos to
            (0 - SERVO_MAX_ANGLE). */
void tlc_initServos(uint8_t initAngle)
{
    Tlc.init(tlc_angleToVal(initAngle));
    TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10)); // stop timer1
    ICR1 = SERVO_TIMER1_TOP;
    TCNT1 = 0;
#ifdef TLC_ATMEGA_8_H
    uint8_t oldTCCR2 = TCCR2;
    TCCR2 = 0;
    TCNT2 = 0;
    OCR2 = SERVO_TIMER2_TOP / 2;
    TCCR2 = oldTCCR2;
#else
    uint8_t oldTCCR2B = TCCR2B;
    TCCR2B = 0;
    TCNT2 = 0;
    OCR2A = SERVO_TIMER2_TOP;
    TCCR2B = oldTCCR2B;
#endif
    TCCR1B |= _BV(CS11); // start timer1 with div 8 prescale
}

/** Sets a servo on channel to angle.
    \param channel which channel to set
    \param angle (0 - SERVO_MAX_ANGLE) */
void tlc_setServo(TLC_CHANNEL_TYPE channel, uint8_t angle)
{
    Tlc.set(channel, tlc_angleToVal(angle));
}

/** Gets the current angle that channel is set to.
    \param channel which channel to get */
uint8_t tlc_getServo(TLC_CHANNEL_TYPE channel)
{
    return tlc_valToAngle(Tlc.get(channel));
}

/** Converts and angle (0 - SERVO_MAX_ANGLE) to the inverted tlc channel value
    (4095 - 0). */
uint16_t tlc_angleToVal(uint8_t angle)
{
    return 4095 - SERVO_MIN_WIDTH - (
            ((uint16_t)(angle) * (uint16_t)(SERVO_MAX_WIDTH - SERVO_MIN_WIDTH))
            / SERVO_MAX_ANGLE);
}

/** Converts an inverted tlc channel value (4095 - 0) into an angle (0 -
    SERVO_MAX_ANGLE). */
uint8_t tlc_valToAngle(uint16_t value)
{
    return SERVO_MAX_ANGLE * (4095 - SERVO_MIN_WIDTH - value)
            / (SERVO_MAX_WIDTH - SERVO_MIN_WIDTH);
}

/* @} */

#endif

