#include "common.h"
#include "fixed.h"
#include "input.h"
#include "audio.h"
#include "target.h"
#include "player.h"
#include "raycast.h"
#include "render.h"
#include "game.h"
#include <stdarg.h>
#include "cover.h"
#include "HEX.h"
int main(void)
{
    //initializing
    init_cameraX_LUT();
    initial_targets();
    vga_init_once();
    fire_audio_init();
    shot_audio_init();
    mouse_init();

    Player p;
cover://cover page, waiting for the beginning of game
    charbuf_clear();
    draw_title();
    clear_score();
    key_x = 0;
    key_r = 0;
    key_r = 0;
    while (1)
    {
        ps2_poll_update_all();//reading keyboard for message

        if (key_g) {
            while (key_g) ps2_poll_update_all();
            break;
        }
    }

begin:
    refresh(&p);//refreshing game state
    display_score(p.score);
    for (;;)
    {
        hp_timer++;
        if (hp_timer > 30) {
            hp_timer = 0;
            if (player_hp > 0) player_hp--;
        }
        if (player_hp <= 0) goto exit_game;

        if (target_num == target_set) {
            finish_game = 1;
            goto exit_game;
        }

        target_timer++;
        if (target_visible) {
            if (target_timer > TARGET_VISIBLE_TIME) {
                target_visible = 0;
                target_timer = 0;
            }
        } else {
            if (target_timer > TARGET_HIDDEN_TIME) {
                target_visible = 1;
                target_timer = 0;
            }
        }

        ps2_poll_update_all();

        q16 step = (q16)3932;
        q16 strafeX = -p.dirY;
        q16 strafeY = p.dirX;

        if (key_x && !exit_handled) {
            exit_handled = 1;
            goto exit_game;
        }
        if (!key_x) exit_handled = 0;

        if (key_w) move(&p, q16_mul(p.dirX, step), q16_mul(p.dirY, step));
        if (key_s) move(&p, -q16_mul(p.dirX, step), -q16_mul(p.dirY, step));
        if (key_d) move(&p, q16_mul(strafeX, step), q16_mul(strafeY, step));
        if (key_a) move(&p, -q16_mul(strafeX, step), -q16_mul(strafeY, step));
        if (key_e) rotate_player(&p, 1);
        if (key_q) rotate_player(&p, 0);

        if (mouse_dx != 0) {
            int turn_left = (mouse_dx < 0) ? 1 : 0;
            int turn_amount = q16_TO_INT(q16_abs(q16_FROM_INT(mouse_dx))) >> 4;//adjusting sensitivity by shifting 4 to right
            if (turn_amount == 0) turn_amount = 1;

            for (int i = 0; i < turn_amount; i++) {
                rotate_player(&p, turn_left);
            }
            mouse_dx = 0;
        }

        if ((key_z || mouse_right) && !scope_handled) {
            scope_handled = 1;
            scope_on = !scope_on;

            if (scope_on) {//narrow down the view, to make the target conspicuous
                p.planeX = p.planeX / 2;
                p.planeY = p.planeY / 2;
            } else {
                p.planeX = p.planeX * 2;
                p.planeY = p.planeY * 2;
            }
        }
        if (!key_z && !mouse_right) scope_handled = 0;

        if ((key_f || mouse_left) && !fire_handled && !fire_sound.playing) {
        fire_center(&p);
        fire_handled = 1;
        fire_sound.playing = 1;
        fire_sound.index = 0;

        if (target_num == target_set) {
            finish_game = 1;//
    }
}
        if (!key_f && !mouse_left) fire_handled = 0;

        if (p.cooldown > 0) p.cooldown--;

        audio_update();
        update_targets();
        render_frame(&p);

        hud_score = p.score;
        hud_px = q16_TO_INT(p.posX);
        hud_py = q16_TO_INT(p.posY);
        hud_draw();
        display_score(p.score);
        vga_render_one_frame();
    }

exit_game:
    if (player_hp <= 0) {
        charbuf_print(34, 12, "  GAME OVER(Die)  ");
    } else if (finish_game == 1) {
        charbuf_print(34, 12, "   SUCCESS!   ");
    } else {
        charbuf_print(34, 12, "   End!   ");
    }
    charbuf_print(28, 14, "Press R to return to cover");
    while (1)
    {
        ps2_poll_update_all();
        if (key_r) {
            while (key_r) ps2_poll_update_all();
            goto cover;
        }
    }
}