#ifndef TLC_Teensy_xxU4_h
#define TLC_Teensy_xxU4_h

#if DATA_TRANSFER_MODE == TLC_BITBANG
#error "If you want bitbang mode, insert pin defs here"
#endif

// MOSI (Teensy pin 11) -> SIN (TLC pin 26)
#define TLC_MOSI_PIN	11
#define TLC_MOSI_PORT	11
#define TLC_MOSI_DDR	11

// SCK (Teensy pin 13) -> SCLK (TLC pin 25)
#define TLC_SCK_PIN	13
#define TLC_SCK_PORT	13
#define TLC_SCK_DDR	13

// SS (Teensy pin 10)
#define TLC_SS_PIN	10
#define TLC_SS_DDR	10

// FTM1_CH0 (Teensy pin 3) -> XLAT (TLC pin 24)
#define XLAT_PIN	3
#define XLAT_PORT	3
#define XLAT_DDR	3

// FTM1_CH1 (Teensy pin 4) -> BLANK (TLC pin 23)
#define BLANK_PIN	4
#define BLANK_PORT	4
#define BLANK_DDR	4

// CMTOUT (Teensy pin 5) -> GSCLK (TLC pin 18)
#define GSCLK_PIN	5
#define GSCLK_PORT	5
#define GSCLK_DDR	5

#endif

