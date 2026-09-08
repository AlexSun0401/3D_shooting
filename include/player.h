#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"

void rotate_player(Player *p, int left);
void move(Player *p, q16 dx, q16 dy);
void fire_center(Player *p);

#endif