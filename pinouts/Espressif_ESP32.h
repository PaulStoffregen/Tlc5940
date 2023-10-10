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

#ifndef ESPRESSIF_ESP32_H
#define ESPRESSIF_ESP32_H

/** \file
    Default pins for the ESP32.  Don't edit these.  All
    changeable pins are defined through Tlc5940::init */

/** MOSI (ESP32 pin 33) -> SIN (TLC pin 26) */
#define DEFAULT_TLC_MOSI_PIN     33

/** MISO (ESP32 pin 25) -> SOUT (TLC pin 17) */
#define DEFAULT_TLC_MISO_PIN     25

/** SCK (ESP32 pin 32) -> SCLK (TLC pin 25) */
#define DEFAULT_TLC_SCK_PIN      32

/** XLAT (ESP32 pin 27) -> XLAT (TLC pin 24) */
#define DEFAULT_XLAT_PIN     27

/** BLANK (ESP32 pin 23) -> BLANK (TLC pin 23) */
#define DEFAULT_BLANK_PIN    23

/** GSCLK (ESP32 pin 12) -> GSCLK (TLC pin 18) */
#define DEFAULT_GSCLK_PIN    12

/** VPRG (Disabled by default) -> VPRG (TLC pin 27) */
#define DEFAULT_VPRG_PIN    -1

/** XERR (Disabled by default) -> XERR (TLC pin 16) */
#define DEFAULT_XERR_PIN    -1

#endif

