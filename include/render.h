#ifndef RENDER_H
#define RENDER_H

#include "common.h"

extern int hud_score, hud_px, hud_py;
extern int scope_on, scope_handled;
extern int center_x, center_y, radius_sq, border_sq;
extern int player_hp, hp_timer;

void charbuf_clear(void);
void charbuf_print(int x, int y, const char *s);
void hud_draw(void);

void vga_init_once(void);
void vga_render_one_frame(void);

#endif