/*
    Fades a line down the channels, with max value and duration based on
    the voltage of analog pin 0.
    Try grounding analog 0: everything should turn off.
    Try putting +5V into analog 0: everything should turn on.

    See the BasicUse example for hardware setup.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "Tlc5940.h"
#include "tlc_fades.h"

TLC_CHANNEL_TYPE channel;

void setup()
{
  Tlc.init();
}

void loop()
{
  if (tlc_fadeBufferSize < TLC_FADE_BUFFER_LENGTH - 2) {
    if (!tlc_isFading(channel)) {
      uint16_t duration = analogRead(0) * 2;
      int maxValue = analogRead(0) * 2;
      uint32_t startMillis = millis() + 50;
      uint32_t endMillis = startMillis + duration;
      tlc_addFade(channel, 0, maxValue, startMillis, endMillis);
      tlc_addFade(channel, maxValue, 0, endMillis, endMillis + duration);
    }
    if (channel++ == NUM_TLCS * 16) {
      channel = 0;
    }
  }
  tlc_updateFades();
}

