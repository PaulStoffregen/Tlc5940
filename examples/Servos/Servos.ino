/*
    This sketch sweeps a servo on channel 0.

    To connect a servo:
     1. Put a 2k-5k pull-up resistor (R0 below; I've tried with 3.3k) between the
          servo control output pin and +5v.
     2. Connect that same pin to the servo's control line like so

        servo data pin
              |     _____
     OUTn ----+----[_____]---+5v
                      R0

    Steve Pomeroy <steve ~AT~ staticfree.info>, 2009-01-20 */

#include "Tlc5940.h"
#include "tlc_servos.h"

#define SERVO_CHANNEL   0
#define DELAY_TIME      20

void setup()
{
  tlc_initServos();  // Note: this will drop the PWM freqency down to 50Hz.
}

void loop()
{
  for (int angle = 0; angle < 180; angle++) {
    tlc_setServo(SERVO_CHANNEL, angle);
    Tlc.update();
    delay(DELAY_TIME);
  }
  for (int angle = 180; angle >= 0; angle--) {
    tlc_setServo(SERVO_CHANNEL, angle);
    Tlc.update();
    delay(DELAY_TIME);
  }
}

