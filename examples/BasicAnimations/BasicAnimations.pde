/*
    Writes "Ardunio" with Persistance of Vision (POV) with 16 LEDs (output 0
    is on bottom, output 15 is top).  The animation below doesn't work with
    more than 1 TLC.

    I generated the animation with the included java code:
    <arduino folder>/hardware/libraries/Tlc5940/examples/BasicAnimations

    To use the code, run
        java AnimationCreator
    in the folder above and it will parse all images in the folder to
    .h files.  For best results use images that are 16 pixels high.

    See the BasicUse example for hardware setup.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "Tlc5940.h"
#include "tlc_animations.h"
#include "ani_arduino.h"

void setup()
{
  Tlc.init();
}

void loop()
{
  // checks to see if the animation is finished playing
  if (!tlc_onUpdateFinished) {

    delay(100);

    /*
      void tlc_playAnimation(prog_uint8_t *animation, uint16_t frames,
                             uint16_t periodsPerFrame);
      periods per frame is PWM periods, 1.024ms per frame (0 is valid - this
      will play the animation as fast as possible).

      Plays an animation in the "background".
      Don't call Tlc.update() while this is running.
      You can check if this is done with !tlc_onUpdateFinished */
    tlc_playAnimation(ani_arduino, ANI_ARDUINO_FRAMES, 3);


    // If you don't want to do anything until it's finished, use:
    // while (!tlc_onUpdateFinished);

  }

}

