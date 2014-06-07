/*
    A circular light buffer.  If you manage to construct a circle of LEDs,
    definitely send in pictures.  What this sketch does is take an analog
    reading off of analog pin 0 and add it to the current value of the last LED.
    If the resultant sum is greater than 4095, it turns the LED off,
    otherwise sets LED 0 to the value of the sum.

    If you ground pin 12, it will set LED 0 to zero.

    Then it shifts all the LED values up one (so LED 0 becomes LED 1) and sets
    LED 0 to the value shifted off the last LED (so if one LED is on, it will
    go in a circle forever).

    See the BasicUse example for hardware setup.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-04 */

#include "Tlc5940.h"
#include "tlc_shifts.h"

// which analog pin to use
#define ANALOG_PIN      0

// which pin to clear the LEDs with
#define CLEAR_PIN      12

// how many millis for one full revolution over all the LEDs
#define SCOPE_PERIOD    (2000 * NUM_TLCS)
#define LED_PERIOD      SCOPE_PERIOD / (NUM_TLCS * 16)

void setup()
{
  pinMode(CLEAR_PIN, INPUT);
  digitalWrite(CLEAR_PIN, HIGH); // enable pull-up
  Tlc.init();
}

void loop()
{
  // shiftUp returns the value shifted off the last pin
  uint16_t sum = tlc_shiftUp() + analogRead(ANALOG_PIN) * 4;
  if (digitalRead(CLEAR_PIN) == LOW || sum > 4095)
    sum = 0;
  Tlc.set(0, sum);
  Tlc.update();
  delay(LED_PERIOD);
}

