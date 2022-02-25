#ifndef TLC_Teensy_IMXRT_h
#define TLC_Teensy_IMXRT_h

// bitbang I/O is plenty fast on Teensy 4
// and avoids SPI sharing problems
#ifdef DATA_TRANSFER_MODE
#undef DATA_TRANSFER_MODE
#endif
#define DATA_TRANSFER_MODE TLC_BITBANG

// Teensy pin 6 -> SIN (TLC pin 26)
#define DEFAULT_BB_SIN_PIN	6
#define DEFAULT_BB_SIN_PORT	6
#define DEFAULT_BB_SIN_DDR	6

// Teensy pin 7 -> SCLK (TLC pin 25)
#define DEFAULT_BB_SCLK_PIN	7
#define DEFAULT_BB_SCLK_PORT	7
#define DEFAULT_BB_SCLK_DDR	7

// FlexPWM4_2_B (Teensy pin 3) -> XLAT (TLC pin 24)
#define XLAT_PIN		3
#define XLAT_PORT		3
#define XLAT_DDR		3

// FlexPWM4_2_A (Teensy pin 2) -> BLANK (TLC pin 23)
#define BLANK_PIN		2
#define BLANK_PORT		2
#define BLANK_DDR		2

// PWM (Teensy pin 5) -> GSCLK (TLC pin 18)
#define GSCLK_PIN		5
#define GSCLK_PORT		5
#define GSCLK_DDR		5


#endif
