#ifndef ASSETS_H
#define ASSETS_H

#include "fixed.h"

#define GUN_W 256
#define GUN_H 128
#define TRANSPARENT 0xFFFF

extern const unsigned short gun[GUN_H][GUN_W];
extern const q16 soundfire[];
extern const q16 soundshot[];

extern const int soundfire_len;
extern const int soundshot_len;

#endif