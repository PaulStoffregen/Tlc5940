#ifndef TLC_pin_functions_h
#define TLC_pin_functions_h

#if defined(__AVR__)
  #define pulse_pin(port, pin)        port |= _BV(pin); port &= ~_BV(pin)
  #define set_pin(port, pin)          port |= _BV(pin)
  #define clear_pin(port, pin)        port &= ~_BV(pin)
  #define output_pin(ddr, pin)        ddr |= _BV(pin)
  #define pullup_pin(ddr, port, pin)  ddr &= ~_BV(pin); port |= _BV(pin)
#elif defined(TEENSYDUINO)
  #if F_CPU > 48000000
  #define pulse_pin(port, pin)        digitalWriteFast(pin, HIGH); asm("nop"); digitalWriteFast(pin, LOW)
  #else
  #define pulse_pin(port, pin)        digitalWriteFast(pin, HIGH); digitalWriteFast(pin, LOW)
  #endif
  #define set_pin(port, pin)          digitalWriteFast(pin, HIGH)
  #define clear_pin(port, pin)        digitalWriteFast(pin, LOW)
  #define output_pin(ddr, pin)        pinMode(pin, OUTPUT)
  #define pullup_pin(ddr, port, pin)  pinMode(pin, INPUT_PULLUP)
#else
  #define pulse_pin(port, pin)        digitalWrite(pin, HIGH); digitalWrite(pin, LOW)
  #define set_pin(port, pin)          digitalWrite(pin, HIGH)
  #define clear_pin(port, pin)        digitalWrite(pin, LOW)
  #define output_pin(ddr, pin)        pinMode(pin, OUTPUT)
  #define pullup_pin(ddr, port, pin)  pinMode(pin, INPUT_PULLUP)
#endif

#endif
