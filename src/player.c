#include "player.h"
#include "raycast.h"
#include "fixed.h"
#include "world.h"
#include "target.h"
#include "audio.h"

void rotate_player(Player *p, int left)
{
    q16 cosv = ROT_C;
    q16 sinv = left ? ROT_S : -ROT_S;
    q16 dir_x = p->dirX;
    q16 plane_x = p->planeX;

    p->dirX = q16_mul(p->dirX, cosv) - q16_mul(p->dirY, sinv);
    p->dirY = q16_mul(dir_x, sinv) + q16_mul(p->dirY, cosv);
    p->planeX = q16_mul(p->planeX, cosv) - q16_mul(p->planeY, sinv);
    p->planeY = q16_mul(plane_x, sinv) + q16_mul(p->planeY, cosv);
} //this function is to rotate player's point of view
//formula is as below
//x=xcos(theta)-ysin(theta) y=xsin(theta)+ycos(theta)

void move(Player *p, q16 dx, q16 dy)
{
    q16 new_x = p->posX + dx;
    q16 new_y = p->posY + dy;

    int mx = q16_TO_INT(new_x);
    int my = q16_TO_INT(p->posY);
    if (!is_wall(mx, my)) p->posX = new_x;

    mx = q16_TO_INT(p->posX);
    my = q16_TO_INT(new_y);
    if (!is_wall(mx, my)) p->posY = new_y;
}  //this function is to check the movement of the player as it traverses through the map

void fire_center(Player *p)
{
    if (p->cooldown > 0) return;
    p->cooldown = 6; //this is just a short duration time
    //to ensure that you don't fire so frequently

    RayHit hit = raycast_ray(p, p->dirX, p->dirY);
    if (!target_visible || hit.targetX < 0) return;

    targetGrid[hit.targetY][hit.targetX] = 0;

    for (int i = 0; i < MAX_TARGETS; i++) {
        if (targets[i].alive &&
            targets[i].x == hit.targetX &&
            targets[i].y == hit.targetY) {
            targets[i].alive = 0;
            break;
        }
    }

    p->score++;
    target_num++;
    target_visible = 0;

    shot_sound.playing = 1;
    shot_sound.index = 0; // Restart the hit sound; fire sound is handled by main().
}
