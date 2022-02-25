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

/** \file
    Tlc5940 class functions. */

#include "Tlc5940.h"
#include "pinouts/pin_functions.h"


/** This will be true (!= 0) if update was just called and the data has not
    been latched in yet. */
volatile uint8_t tlc_needXLAT;

/** Some of the extened library will need to be called after a successful
    update. */
volatile void (*tlc_onUpdateFinished)(void);

/** Packed grayscale data, 24 bytes (16 * 12 bits) per TLC.

    Format: Lets assume we have 2 TLCs, A and B, daisy-chained with the SOUT of
    A going into the SIN of B.
    - byte 0: upper 8 bits of B.15
    - byte 1: lower 4 bits of B.15 and upper 4 bits of B.14
    - byte 2: lower 8 bits of B.0
    - ...
    - byte 24: upper 8 bits of A.15
    - byte 25: lower 4 bits of A.15 and upper 4 bits of A.14
    - ...
    - byte 47: lower 8 bits of A.0

    \note Normally packing data like this is bad practice.  But in this
          situation, shifting the data out is really fast because the format of
          the array is the same as the format of the TLC's serial interface. */
uint8_t tlc_GSData[NUM_TLCS * 24];

/** Don't add an extra SCLK pulse after switching from dot-correction mode. */
static uint8_t firstGSInput;

/** Interrupt called after an XLAT pulse to prevent more XLAT pulses. */
static inline void Tlc5940_interrupt(void)
{
    disable_XLAT_pulses();
    clear_XLAT_interrupt();
    tlc_needXLAT = 0;
    if (tlc_onUpdateFinished) {
        sei();
        tlc_onUpdateFinished();
    }
}

#if defined(__AVR__)
ISR(TIMER1_OVF_vect)
{
    Tlc5940_interrupt();
}

#elif defined(__arm__) && defined(TEENSYDUINO)
#if defined(__IMXRT1062__)
void flexpwm42_isr(void)
{
    FLEXPWM4_SM2STS = FLEXPWM_SMSTS_RF;
    Tlc5940_interrupt();
}
#else
void ftm1_isr(void)
{
    uint32_t sc = FTM1_SC;
    if (sc & 0x80) FTM1_SC = sc & 0x7F;
    Tlc5940_interrupt();
}
#endif
#endif

/** \defgroup ReqVPRG_ENABLED Functions that Require VPRG_ENABLED
    Functions that require VPRG_ENABLED == 1.
    You can enable VPRG by changing
    \code #define VPRG_ENABLED    0 \endcode to
    \code #define VPRG_ENABLED    1 \endcode in tlc_config.h

    You will also have to connect Arduino digital pin 6 to TLC pin 27. (The
    pin to be used can be changed in tlc_config.h).  If VPRG is not enabled,
    the TLC pin should grounded (remember to unconnect TLC pin 27 from GND
    if you do enable VPRG). */
/* @{ */ /* @} */

/** \defgroup CoreFunctions Core Libary Functions
    These function are all prefixed with "Tlc." */
/* @{ */

/** Pin i/o and Timer setup.  The grayscale register will be reset to all
    zeros, or whatever initialValue is set to and the Timers will start.
    \param initialValue = 0, optional parameter specifing the inital startup
           value */
void Tlc5940::init(uint16_t initialValue)
{
    /* Pin Setup */
    output_pin(XLAT_DDR, XLAT_PIN);
    output_pin(BLANK_DDR, BLANK_PIN);
    output_pin(GSCLK_DDR, GSCLK_PIN);
#if VPRG_ENABLED
    output_pin(VPRG_DDR, VPRG_PIN);
    clear_pin(VPRG_PORT, VPRG_PIN);  // grayscale mode (VPRG low)
#endif
#if XERR_ENABLED
    pullup_pin(XERR_DDR, XERR_PORT, XERR_PIN); // XERR as input, enable pull-up resistor
#endif
    set_pin(BLANK_PORT, BLANK_PIN); // leave blank high (until the timers start)

    tlc_shift8_init();

    setAll(initialValue);
    update();
    disable_XLAT_pulses();
    clear_XLAT_interrupt();
    tlc_needXLAT = 0;
    pulse_pin(XLAT_PORT, XLAT_PIN);


    /* Timer Setup */
#if defined(__AVR__)
    /* Timer 1 - BLANK / XLAT */
    TCCR1A = _BV(COM1B1);  // non inverting, output on OC1B, BLANK
    TCCR1B = _BV(WGM13);   // Phase/freq correct PWM, ICR1 top
    OCR1A = 1;             // duty factor on OC1A, XLAT is inside BLANK
    OCR1B = 2;             // duty factor on BLANK (larger than OCR1A (XLAT))
    ICR1 = TLC_PWM_PERIOD; // see tlc_config.h

    /* Timer 2 - GSCLK */
#if defined(TLC_ATMEGA_8_H)
    TCCR2  = _BV(COM20)       // set on BOTTOM, clear on OCR2A (non-inverting),
           | _BV(WGM21);      // output on OC2B, CTC mode with OCR2 top
    OCR2   = TLC_GSCLK_PERIOD / 2; // see tlc_config.h
    TCCR2 |= _BV(CS20);       // no prescale, (start pwm output)
#elif defined(TLC_TIMER3_GSCLK)
    TCCR3A = _BV(COM3A1)      // set on BOTTOM, clear on OCR3A (non-inverting),
                              // output on OC3A
           | _BV(WGM31);      // Fast pwm with ICR3 top
    OCR3A = 0;                // duty factor (as short a pulse as possible)
    ICR3 = TLC_GSCLK_PERIOD;  // see tlc_config.h
    TCCR3B = _BV(CS30)        // no prescale, (start pwm output)
           | _BV(WGM32)       // Fast pwm with ICR3 top
           | _BV(WGM33);      // Fast pwm with ICR3 top
#else
    TCCR2A = _BV(COM2B1)      // set on BOTTOM, clear on OCR2A (non-inverting),
                              // output on OC2B
           | _BV(WGM21)       // Fast pwm with OCR2A top
           | _BV(WGM20);      // Fast pwm with OCR2A top
    TCCR2B = _BV(WGM22);      // Fast pwm with OCR2A top
    OCR2B = 0;                // duty factor (as short a pulse as possible)
    OCR2A = TLC_GSCLK_PERIOD; // see tlc_config.h
    TCCR2B |= _BV(CS20);      // no prescale, (start pwm output)
#endif
    TCCR1B |= _BV(CS10);      // no prescale, (start pwm output)

#elif defined(__arm__) && defined(TEENSYDUINO)
 #if defined(__IMXRT1062__)
    /* Teensy 4.0, 4.1, MicroMod */
    clear_pin(XLAT_DDR, XLAT_PIN);
    analogWriteFrequency(5, 4000000);
    analogWrite(5, 128);
    const uint32_t newdiv = (uint32_t)((float)F_BUS_ACTUAL / 4 / 1000 + 0.5f);
    FLEXPWM4_MCTRL |= FLEXPWM_MCTRL_CLDOK(4);
    FLEXPWM4_SM2CTRL = FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_PRSC(2); // 3146
    FLEXPWM4_SM2VAL0 = newdiv - 1;
    FLEXPWM4_SM2VAL1 = newdiv - 1;
    FLEXPWM4_SM2VAL2 = newdiv - 7; // pin 2 = FlexPWM4_2_A = BLANK
    FLEXPWM4_SM2VAL3 = newdiv - 1;
    FLEXPWM4_SM2VAL4 = newdiv - 6; // pin 3 = FlexPWM4_2_B = XLAT
    FLEXPWM4_SM2VAL5 = newdiv - 2;
    FLEXPWM4_OUTEN = FLEXPWM_OUTEN_PWMA_EN(4) | FLEXPWM_OUTEN_PWMB_EN(4);
    FLEXPWM4_MCTRL |= FLEXPWM_MCTRL_LDOK(4);
    CORE_PIN2_CONFIG = 1;
    CORE_PIN2_PADCONFIG = IOMUXC_PAD_DSE(7);
    CORE_PIN3_PADCONFIG = IOMUXC_PAD_DSE(7);
    FLEXPWM4_SM2INTEN = 0;
    FLEXPWM4_SM2STS = 0x3FFF;
    attachInterruptVector(IRQ_FLEXPWM4_2, flexpwm42_isr);
    NVIC_ENABLE_IRQ(IRQ_FLEXPWM4_2);
 #else
    /* Teensy 3.0, 3.1, 3.2, 3.5, 3.6 */
    clear_pin(XLAT_DDR, XLAT_PIN);
    SIM_SCGC4 |= SIM_SCGC4_CMT;
    CMT_MSC = 0;
    CMT_PPS = 0;
    CMT_CGH1 = TLC_TIMER_TEENSY3_NORMAL_CGH1;
    CMT_CGL1 = TLC_TIMER_TEENSY3_NORMAL_CGL1;
    CMT_CMD1 = 1;
    CMT_CMD2 = 0;
    CMT_CMD3 = 0;
    CMT_CMD4 = 0;
    CMT_OC = 0x60;
    CMT_MSC = 0x01;
    CORE_PIN5_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_DSE|PORT_PCR_SRE;
    FTM1_SC = 0;
    FTM1_MOD = TLC_TIMER_TEENSY3_NORMAL_MOD;
    FTM1_CNT = 0;
    FTM1_C0SC = 0x24;
    FTM1_C1SC = 0x24;
    FTM1_C0V = TLC_TIMER_TEENSY3_NORMAL_MOD - TLC_TIMER_TEENSY3_NORMAL_CV;
    FTM1_C1V = TLC_TIMER_TEENSY3_NORMAL_MOD - TLC_TIMER_TEENSY3_NORMAL_CV - 1;
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_CPWMS;
    NVIC_ENABLE_IRQ(IRQ_FTM1);
    CORE_PIN4_CONFIG = PORT_PCR_MUX(3)|PORT_PCR_DSE|PORT_PCR_SRE;
 #endif
#endif
    update();
}

/** Clears the grayscale data array, #tlc_GSData, but does not shift in any
    data.  This call should be followed by update() if you are turning off
    all the outputs. */
void Tlc5940::clear(void)
{
    setAll(0);
}

/** Shifts in the data from the grayscale data array, #tlc_GSData.
    If data has already been shifted in this grayscale cycle, another call to
    update() will immediately return 1 without shifting in the new data.  To
    ensure that a call to update() does shift in new data, use
    \code while(Tlc.update()); \endcode
    or
    \code while(tlc_needXLAT); \endcode
    \returns 1 if there is data waiting to be latched, 0 if data was
             successfully shifted in */
uint8_t Tlc5940::update(void)
{
    if (tlc_needXLAT) {
        return 1;
    }
    disable_XLAT_pulses();
    if (firstGSInput) {
        // adds an extra SCLK pulse unless we've just set dot-correction data
        firstGSInput = 0;
    } else {
        pulse_pin(SCLK_PORT, SCLK_PIN);
    }
    uint8_t *p = tlc_GSData;
    while (p < tlc_GSData + NUM_TLCS * 24) {
        tlc_shift8(*p++);
        tlc_shift8(*p++);
        tlc_shift8(*p++);
    }
    tlc_needXLAT = 1;
    enable_XLAT_pulses();
    set_XLAT_interrupt();
    return 0;
}

/** Sets channel to value in the grayscale data array, #tlc_GSData.
    \param channel (0 to #NUM_TLCS * 16 - 1).  OUT0 of the first TLC is
           channel 0, OUT0 of the next TLC is channel 16, etc.
    \param value (0-4095).  The grayscale value, 4095 is maximum.
    \see get */
void Tlc5940::set(TLC_CHANNEL_TYPE channel, uint16_t value)
{
    TLC_CHANNEL_TYPE index8 = (NUM_TLCS * 16 - 1) - channel;
    uint8_t *index12p = tlc_GSData + ((((uint16_t)index8) * 3) >> 1);
    if (index8 & 1) { // starts in the middle
                      // first 4 bits intact | 4 top bits of value
        *index12p = (*index12p & 0xF0) | (value >> 8);
                      // 8 lower bits of value
        *(++index12p) = value & 0xFF;
    } else { // starts clean
                      // 8 upper bits of value
        *(index12p++) = value >> 4;
                      // 4 lower bits of value | last 4 bits intact
        *index12p = ((uint8_t)(value << 4)) | (*index12p & 0xF);
    }
}

/** Gets the current grayscale value for a channel
    \param channel (0 to #NUM_TLCS * 16 - 1).  OUT0 of the first TLC is
           channel 0, OUT0 of the next TLC is channel 16, etc.
    \returns current grayscale value (0 - 4095) for channel
    \see set */
uint16_t Tlc5940::get(TLC_CHANNEL_TYPE channel)
{
    TLC_CHANNEL_TYPE index8 = (NUM_TLCS * 16 - 1) - channel;
    uint8_t *index12p = tlc_GSData + ((((uint16_t)index8) * 3) >> 1);
    return (index8 & 1)? // starts in the middle
            (((uint16_t)(*index12p & 15)) << 8) | // upper 4 bits
            *(index12p + 1)                       // lower 8 bits
        : // starts clean
            (((uint16_t)(*index12p)) << 4) | // upper 8 bits
            ((*(index12p + 1) & 0xF0) >> 4); // lower 4 bits
    // that's probably the ugliest ternary operator I've ever created.
}

/** Sets all channels to value.
    \param value grayscale value (0 - 4095) */
void Tlc5940::setAll(uint16_t value)
{
    uint8_t firstByte = value >> 4;
    uint8_t secondByte = (value << 4) | (value >> 8);
    uint8_t *p = tlc_GSData;
    while (p < tlc_GSData + NUM_TLCS * 24) {
        *p++ = firstByte;
        *p++ = secondByte;
        *p++ = (uint8_t)value;
    }
}

#if VPRG_ENABLED

/** \addtogroup ReqVPRG_ENABLED
    From the \ref CoreFunctions "Core Functions":
    - \link Tlc5940::setAllDC Tlc.setAllDC(uint8_t value(0-63)) \endlink - sets
      all the dot correction data to value */
/* @{ */

/** Sets the dot correction for all channels to value.  The dot correction
    value correspondes to maximum output current by
    \f$\displaystyle I_{OUT_n} = I_{max} \times \frac{DCn}{63} \f$
    where
    - \f$\displaystyle I_{max} = \frac{1.24V}{R_{IREF}} \times 31.5 =
         \frac{39.06}{R_{IREF}} \f$
    - DCn is the dot correction value for channel n
    \param value (0-63) */
void Tlc5940::setAllDC(uint8_t value)
{
    tlc_dcModeStart();

    uint8_t firstByte = value << 2 | value >> 4;
    uint8_t secondByte = value << 4 | value >> 2;
    uint8_t thirdByte = value << 6 | value;

    for (TLC_CHANNEL_TYPE i = 0; i < NUM_TLCS * 12; i += 3) {
        tlc_shift8(firstByte);
        tlc_shift8(secondByte);
        tlc_shift8(thirdByte);
    }
    pulse_pin(XLAT_PORT, XLAT_PIN);

    tlc_dcModeStop();
}

/* @} */

#endif

#if XERR_ENABLED

/** Checks for shorted/broken LEDs reported by any of the TLCs.
    \returns 1 if a TLC is reporting an error, 0 otherwise. */
uint8_t Tlc5940::readXERR(void)
{
    return ((XERR_PINS & _BV(XERR_PIN)) == 0);
}

#endif

/* @} */

#if DATA_TRANSFER_MODE == TLC_BITBANG

/** Sets all the bit-bang pins to output */
void tlc_shift8_init(void)
{
    output_pin(SIN_DDR, SIN_PIN);   // SIN as output
    output_pin(SCLK_DDR, SCLK_PIN); // SCLK as output
    clear_pin(SCLK_PORT, SCLK_PIN);
}

/** Shifts a byte out, MSB first */
void tlc_shift8(uint8_t byte)
{
    for (uint8_t bit = 0x80; bit; bit >>= 1) {
        if (bit & byte) {
            set_pin(SIN_PORT, SIN_PIN);
        } else {
            clear_pin(SIN_PORT, SIN_PIN);
        }
        pulse_pin(SCLK_PORT, SCLK_PIN);
    }
}

#elif DATA_TRANSFER_MODE == TLC_SPI

/** Initializes the SPI module to double speed (f_osc / 2) */
void tlc_shift8_init(void)
{
    output_pin(SIN_DDR, SIN_PIN);       // SPI MOSI as output
    output_pin(SCLK_DDR, SCLK_PIN);     // SPI SCK as output
    output_pin(TLC_SS_DDR, TLC_SS_PIN); // SPI SS as output

    clear_pin(SCLK_PORT, SCLK_PIN);

    SPSR = _BV(SPI2X); // double speed (f_osc / 2)
    SPCR = _BV(SPE)    // enable SPI
         | _BV(MSTR);  // master mode
}

/** Shifts out a byte, MSB first */
void tlc_shift8(uint8_t byte)
{
    SPDR = byte; // starts transmission
    while (!(SPSR & _BV(SPIF)))
        ; // wait for transmission complete
}

#endif

#if VPRG_ENABLED

/** Switches to dot correction mode and clears any waiting grayscale latches.*/
void tlc_dcModeStart(void)
{
    disable_XLAT_pulses(); // ensure that no latches happen
    clear_XLAT_interrupt(); // (in case this was called right after update)
    tlc_needXLAT = 0;
    set_pin(VPRG_PORT, VPRG_PIN); // dot correction mode
}

/** Switches back to grayscale mode. */
void tlc_dcModeStop(void)
{
    clear_pin(VPRG_PORT, VPRG_PIN); // back to grayscale mode
    firstGSInput = 1;
}

#endif

/** Preinstantiated Tlc variable. */
Tlc5940 Tlc;

/** \defgroup ExtendedFunctions Extended Library Functions
    These functions require an include statement at the top of the sketch. */
/* @{ */ /* @} */

/** \mainpage
    The <a href="http://www.ti.com/lit/gpn/TLC5940">Texas Instruments TLC5940
    </a> is a 16-channel, constant-current sink LED driver.  Each channel has
    an individually adjustable 4096-step grayscale PWM brightness control and
    a 64-step, constant-current sink (no LED resistors needed!).  This chip
    is a current sink, so be sure to use common anode RGB LEDs.

    Check the <a href="http://code.google.com/p/tlc5940arduino/">tlc5940arduino
    project</a> on Google Code for updates.  To install, unzip the "Tlc5940"
    folder to &lt;Arduino Folder&gt;/hardware/libraries/

    &nbsp;

    \section hardwaresetup Hardware Setup
    The basic hardware setup is explained at the top of the Examples.  A good
    place to start would be the BasicUse Example.  (The examples are in
    File->Sketchbook->Examples->Library-Tlc5940).

    All the options for the library are located in tlc_config.h, including
    #NUM_TLCS, what pins to use, and the PWM period.  After changing
    tlc_config.h, be sure to delete the Tlc5940.o file in the library folder
    to save the changes.

    &nbsp;

    \section libref Library Reference
    \ref CoreFunctions "Core Functions" (see the BasicUse Example and Tlc5940):
    - \link Tlc5940::init Tlc.init(int initialValue (0-4095))\endlink - Call this is
            to setup the timers before using any other Tlc functions.
            initialValue defaults to zero (all channels off).
    - \link Tlc5940::clear Tlc.clear()\endlink - Turns off all channels
            (Needs Tlc.update())
    - \link Tlc5940::set Tlc.set(uint8_t channel (0-(NUM_TLCS * 16 - 1)),
            int value (0-4095))\endlink - sets the grayscale data for channel.
            (Needs Tlc.update())
    - \link Tlc5940::setAll Tlc.setAll(int value(0-4095))\endlink - sets all
            channels to value. (Needs Tlc.update())
    - \link Tlc5940::get uint16_t Tlc.get(uint8_t channel)\endlink - returns
            the grayscale data for channel (see set).
    - \link Tlc5940::update Tlc.update()\endlink - Sends the changes from any
            Tlc.clear's, Tlc.set's, or Tlc.setAll's.

    \ref ExtendedFunctions "Extended Functions".  These require an include
    statement at the top of the sketch to use.

    \ref ReqVPRG_ENABLED "Functions that require VPRG_ENABLED".  These
    require VPRG_ENABLED == 1 in tlc_config.h

    &nbsp;

    \section expansion Expanding the Library
    Lets say we wanted to add a function like "tlc_goCrazy(...)".  The first
    thing to do is to create a source file in the library folder,
    "tlc_my_crazy_functions.h".
    A template for this .h file is
    \code
// Explination for my crazy function extension

#ifndef TLC_MY_CRAZY_FUNCTIONS_H
#define TLC_MY_CRAZY_FUNCTIONS_H

#include "tlc_config.h"
#include "Tlc5940.h"

void tlc_goCrazy(void);

void tlc_goCrazy(void)
{
    uint16_t crazyFactor = 4000;
    Tlc.clear();
    for (uint8_t channel = 4; channel < 9; channel++) {
        Tlc.set(channel, crazyFactor);
    }
    Tlc.update();
}

#endif
 * \endcode
 * Now to use your library in a sketch, just add
 * \code
#include "tlc_my_crazy_functions.h"

// ...

tlc_goCrazy();
    \endcode
    If you would like to share your extended functions for others to use,
    email me (acleone ~AT~ gmail.com) with the file and an example and I'll
    include them in the library.

    &nbsp;

    \section bugs Contact
    If you found a bug in the library, email me so I can fix it!
    My email is acleone ~AT~ gmail.com

    &nbsp;

    \section license License - GPLv3
    Copyright (c) 2009 by Alex Leone <acleone ~AT~ gmail.com>

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

