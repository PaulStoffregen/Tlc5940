/*
    A simple 1-d oscilliscope: scan all the channels, setting the PWM output
    value to 4x the analog pin 0 value (0 - 1024 * 4 = 4096).  The value will
    fade to zero as the channels keep scanning.

    See the BasicUse example for hardware setup.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "Tlc5940.h"
#include "tlc_fades.h"

// which analog pin to use
#define ANALOG_PIN      0

// how many millis to strobe over all the LEDs
#define SCOPE_PERIOD    (1000 * NUM_TLCS)
#define LED_PERIOD      SCOPE_PERIOD / (NUM_TLCS * 16)

TLC_CHANNEL_TYPE channel;

void setup()
{
  Tlc.init();
}

void loop()
{
  uint32_t lastMillis = millis();
  tlc_addFade(channel,                      // led channel
              analogRead(ANALOG_PIN) * 4,   // start fade value (0-4095)
              0,                            // end fade value (0-4095)
              lastMillis + 2,               // start millis
              lastMillis + (uint16_t)SCOPE_PERIOD / 4  // end millis
             );
  if (channel++ == NUM_TLCS * 16) {
    channel = 0;
  }
  uint32_t currentMillis;
  do {
    currentMillis = millis();
    tlc_updateFades(currentMillis);
  } while (currentMillis - lastMillis <= LED_PERIOD);
}

