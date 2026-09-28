#ifndef RAYCAST_H
#define RAYCAST_H

#include "common.h"

extern char View_player[LOG_H][LOG_W + 1];
extern q16 zBuffer[LOG_W];
extern q16 cameraXLUT[LOG_W];

void init_cameraX_LUT(void);
void draw_line(int x, int y0, int y1, char ch);
void ascii_bg(void);
char shade(q16 dist, int side);
// Read-only DDA using the supplied Q16.16 direction, independent of camera/visibility.
// Records targets before checking the wall in each cell, including the wall cell.
RayHit raycast_ray(const Player *p, q16 rayDirX, q16 rayDirY);
RayHit raycast_column(Player *p, int x);
void render_frame(Player *p);

#endif
