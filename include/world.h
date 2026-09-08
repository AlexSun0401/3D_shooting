#ifndef WORLD_H
#define WORLD_H

#include "common.h"
//label wall and targets' position in 16 x 16 maps
extern const unsigned short wallMap[MAP_H][MAP_W];
extern unsigned short targetGrid[MAP_H][MAP_W];
//Determine whether it's a wall
int is_wall(int x, int y);

#endif