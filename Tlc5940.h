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

#ifndef TLC5940_H
#define TLC5940_H

/** \file
    Tlc5940 library header file. */

#include <Arduino.h>
#include "tlc_config.h"

#if defined(__AVR__)
#ifdef TLC_ATMEGA_8_H

/** Enables the Timer1 Overflow interrupt, which will fire after an XLAT
    pulse */
#define set_XLAT_interrupt()    TIFR |= _BV(TOV1); TIMSK = _BV(TOIE1)
/** Disables any Timer1 interrupts */
#define clear_XLAT_interrupt()  TIMSK = 0

#else

/** Enables the Timer1 Overflow interrupt, which will fire after an XLAT
    pulse */
#define set_XLAT_interrupt()    TIFR1 |= _BV(TOV1); TIMSK1 = _BV(TOIE1)
/** Disables any Timer1 interrupts */
#define clear_XLAT_interrupt()  TIMSK1 = 0

#endif

/** Enables the output of XLAT pulses */
#define enable_XLAT_pulses()    TCCR1A = _BV(COM1A1) | _BV(COM1B1)
/** Disables the output of XLAT pulses */
#define disable_XLAT_pulses()   TCCR1A = _BV(COM1B1)

#elif defined(__arm__) && defined(TEENSYDUINO)
#if defined(__IMXRT1062__)
#define set_XLAT_interrupt()    FLEXPWM4_SM2STS = FLEXPWM_SMSTS_RF; FLEXPWM4_SM2INTEN = FLEXPWM_SMINTEN_RIE;
#define clear_XLAT_interrupt()  FLEXPWM4_SM2INTEN = 0;
#define enable_XLAT_pulses()    CORE_PIN3_CONFIG = 1;
#define disable_XLAT_pulses()   CORE_PIN3_CONFIG = 5;
#else
#define set_XLAT_interrupt()    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_CPWMS | FTM_SC_TOIE | (FTM1_SC & FTM_SC_PS(7))
#define clear_XLAT_interrupt()  FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_CPWMS | (FTM1_SC & FTM_SC_PS(7))
#define enable_XLAT_pulses()    CORE_PIN3_CONFIG = PORT_PCR_MUX(3)|PORT_PCR_DSE|PORT_PCR_SRE
#define disable_XLAT_pulses()   CORE_PIN3_CONFIG = PORT_PCR_MUX(1)|PORT_PCR_DSE|PORT_PCR_SRE
#endif

#endif





extern volatile uint8_t tlc_needXLAT;
extern volatile void (*tlc_onUpdateFinished)(void);
extern uint8_t tlc_GSData[NUM_TLCS * 24];

/** The main Tlc5940 class for the entire library.  An instance of this class
    will be preinstantiated as Tlc. */
class Tlc5940
{
  public:
    void init(uint16_t initialValue = 0);
    void clear(void);
    uint8_t update(void);
    void set(TLC_CHANNEL_TYPE channel, uint16_t value);
    uint16_t get(TLC_CHANNEL_TYPE channel);
    void setAll(uint16_t value);
#if VPRG_ENABLED
    void setAllDC(uint8_t value);
#endif
#if XERR_ENABLED
    uint8_t readXERR(void);
#endif

};

void tlc_shift8_init(void);
void tlc_shift8(uint8_t byte);

#if VPRG_ENABLED
void tlc_dcModeStart(void);
void tlc_dcModeStop(void);
#endif

// for the preinstantiated Tlc variable.
extern Tlc5940 Tlc;

#endif

