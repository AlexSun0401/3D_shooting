#include "hw.h"

volatile unsigned int *const VGA = (volatile unsigned int *)VGA_BASE;
volatile unsigned int *const PS2_KB = (volatile unsigned int *)PS2_KB_BASE;
volatile unsigned int *const PS2_MOUSE = (volatile unsigned int *)PS2_MOUSE_BASE;
volatile int *audio_ptr = (volatile int *)AUDIO_BASE;
volatile int *HEX_ptr = (int *) HEX_BASE;