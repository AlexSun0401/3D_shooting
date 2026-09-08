#include "player.h"
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

    q16 rayDirX = p->dirX;
    q16 rayDirY = p->dirY;

    int mapX = q16_TO_INT(p->posX);
    int mapY = q16_TO_INT(p->posY);  

    q16 deltaX = (rayDirX == 0) ? (q16)0x7fffffff : q16_abs(q16_div(q16_ONE, rayDirX));
    q16 deltaY = (rayDirY == 0) ? (q16)0x7fffffff : q16_abs(q16_div(q16_ONE, rayDirY));

    int stepX, stepY;
    q16 sideX, sideY;

    if (rayDirX < 0) {
        stepX = -1;
        sideX = q16_mul(p->posX - q16_FROM_INT(mapX), deltaX);
    } else {
        stepX = +1;
        sideX = q16_mul(q16_FROM_INT(mapX + 1) - p->posX, deltaX);
    }

    if (rayDirY < 0) {
        stepY = -1;
        sideY = q16_mul(p->posY - q16_FROM_INT(mapY), deltaY);
    } else {
        stepY = +1;
        sideY = q16_mul(q16_FROM_INT(mapY + 1) - p->posY, deltaY);
    }  //these if statements are to check in which direction we should move the ray

    while (1)
    {
        if (sideX < sideY) {
            sideX += deltaX;
            mapX += stepX;
        } else {
            sideY += deltaY;
            mapY += stepY;
        }

        if (mapX < 0 || mapX >= MAP_W || mapY < 0 || mapY >= MAP_H) return; //if the ray goes out of bound, stop checking

        if (targetGrid[mapY][mapX] && target_visible)
        {
            targetGrid[mapY][mapX] = 0;

            for (int i = 0; i < MAX_TARGETS; i++) {
                if (targets[i].alive &&
                    targets[i].x == mapX &&
                    targets[i].y == mapY) {
                    targets[i].alive = 0;
                    break;
                }
            } //This is to manually reset target.alive to 0 so that we don't need to 
            //move this target in the "target movement" section

            p->score++;
            target_num++;
            target_visible = 0; //increment score and "hide" the target permanently so
            //we don't have to shoot it again

            shot_sound.playing = 1;
            shot_sound.index = 0;  //start playing shooting sound effect
            return;
        }

        if (is_wall(mapX, mapY)) return; //if the ray hits wall, stop checking
    }
}