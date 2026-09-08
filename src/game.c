#include "game.h"
#include "fixed.h"
#include "input.h"
#include "render.h"
#include "target.h"

void refresh(Player *p)
{
    player_hp = 100;
    hp_timer = 0;
    target_visible = 1;
    target_timer = 0;
    target_num = 0;
    finish_game = 0;
    hud_score = 0;

    key_w = 0; key_a = 0; key_s = 0; key_d = 0; key_z = 0;
    key_q = 0; key_e = 0; key_f = 0; key_x = 0; key_r = 0; key_g = 0;
    fire_handled = 0;
    exit_handled = 0;
    scope_on = 0;
    scope_handled = 0;

    mouse_dx = 0;
    mouse_left = 0;
    mouse_right = 0;

    charbuf_clear();
    hud_draw();
    initial_targets();

    p->posX = q16_FROM_INT(3) + q16_HALF;
    p->posY = q16_FROM_INT(3) + q16_HALF;
    p->dirX = q16_ONE;
    p->dirY = 0;
    p->planeX = 0;
    p->planeY = PLANE_LEN_Q16;
    p->score = 0;
    p->cooldown = 0;
}//this function is for refresh function, when you press R it'll jump to here
//it'll initialize all state variables, call all intialized functions to update any struct 
//it'll also try to start any effect