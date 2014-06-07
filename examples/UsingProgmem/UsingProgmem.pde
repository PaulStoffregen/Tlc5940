/*
    Setting grayscale from progmem to save RAM.  If you want to play
    multiple "frames", (an animation), see the BasicAnimations Example.

    See the BasicUse example for hardware setup.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "Tlc5940.h"

// Extended functions (they start with tlc_...) require another include
#include "tlc_progmem_utils.h"

/*
    This is an array in program memory.  Program memory is 16kB instead of
    1024 bytes for regular variables in SRAM.

    The format for this array is
    GS_DUO(OUT15, OUT14), ... GS_DUO(OUT1, OUT0)

    If you have more than one TLC, the format is
    GS_DUO(TLC2.OUT15, TLC2.OUT14), ... GS_DUO(TLC2.OUT1, TLC2.OUT0),
    GS_DUO(TLC1.OUT15, TLC1.OUT14), ... GS_DUO(TLC1.OUT1, TLC1.OUT0)

    The pattern below will only work with 1 TLC.  Copy + Paste the 4 lines
    inside the curly brackets for each additional TLC. */
uint8_t gsArray1[NUM_TLCS * 24] PROGMEM = {
  GS_DUO((4095 * 16)/16, (4095 * 15)/16), GS_DUO((4095 * 14)/16, (4095 * 13)/16),
  GS_DUO((4095 * 12)/16, (4095 * 11)/16), GS_DUO((4095 * 10)/16, (4095 * 9)/16),
  GS_DUO((4095 * 8)/16, (4095 * 7)/16), GS_DUO((4095 * 6)/16, (4095 * 5)/16),
  GS_DUO((4095 * 4)/16, (4095 * 3)/16), GS_DUO((4095 * 2)/16, (4095 * 1)/16),
};

void setup()
{
  Tlc.init();
}

void loop()
{
  // Display the pattern (brightness ramp over the outputs)
  tlc_setGSfromProgmem(gsArray1);
  Tlc.update();

  // Fade each channel to zero
  for (TLC_CHANNEL_TYPE channel = 0; channel < NUM_TLCS * 16; channel++) {
    int16_t initialValue = Tlc.get(channel);
    while (initialValue > 0) {
      initialValue -= 5;
      if (initialValue < 0) {
        initialValue = 0;
      }
      Tlc.set(channel, initialValue);

      // wait until the data has been sent to the TLCs before continuing
      while (Tlc.update());
    }
  }
}

