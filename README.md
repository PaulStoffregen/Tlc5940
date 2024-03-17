Fork of https://github.com/PaulStoffregen/Tlc5940 with ESP32 support

Tested on ESP32, but should work on ESP32-S2, S3, etc. with no or minimal mods
Requires the following board support package in Arduino IDE (Tools -> Board -> Boards Manager…): esp32 by Espressif Systems

DOT Correction can be set for each channel individually.
Animation doesn't work as PROGMEM snippets for ESP32 were not finished.

# Tlc5940 Library

16 channel PWM LED driver based on the Texas Instruments TLC5940 chip.

http://www.pjrc.com/teensy/td_libs_Tlc5940.html

http://code.google.com/p/tlc5940arduino/

http://playground.arduino.cc/Learning/TLC5940

https://github.com/PaulStoffregen/Tlc5940

![Tlc5940 with Teensy 2.0](http://www.pjrc.com/teensy/td_libs_Tlc5940_1.jpg)

The TLC5940 chip SCK pin is sensitive to signal overshoot.  When used with
Teensy 4.0 or other high speed hardware, especially if using long wires, you
may need to add a resistor inline with the SCLK signal.

https://forum.pjrc.com/threads/71009?p=323766&viewfull=1#post323766
