#ifndef HW_H
#define HW_H

#define SDRAM_BASE      0x00000000u
#define SDRAM_END       0x03FFFFFFu
#define VGA_BASE        0xFF203020u
#define CHAR_BUF_BASE   0x09000000u
#define PS2_KB_BASE     0xFF200100u
#define PS2_MOUSE_BASE  0xFF200108u
#define AUDIO_BASE      0xFF203040
#define HEX_BASE        0xFF200020

extern volatile unsigned int *const VGA;
extern volatile unsigned int *const PS2_KB;
extern volatile unsigned int *const PS2_MOUSE;
extern volatile int *audio_ptr;
extern volatile int *HEX_ptr;

#endif