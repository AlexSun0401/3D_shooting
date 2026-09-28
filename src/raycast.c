#include "raycast.h"
#include "fixed.h"
#include "world.h"
#include "target.h"

char View_player[LOG_H][LOG_W + 1];
q16 zBuffer[LOG_W];//record each wall's distance
q16 cameraXLUT[LOG_W];//representing each column from -1 to 1
//draw char on each column
void draw_line(int x, int y0, int y1, char ch)
{
    if (x < 0 || x >= LOG_W || y0 > y1) return;
    if (y0 < 0) y0 = 0;
    if (y1 >= LOG_H) y1 = LOG_H - 1;
    for (int y = y0; y <= y1; y++) View_player[y][x] = ch;
}
//using char to represent bg first
void ascii_bg(void)
{
    for (int y = 0; y < LOG_H; y++) {
        for (int x = 0; x < LOG_W; x++) {
            View_player[y][x] = (y < LOG_H / 2) ? '.' : '_';
        }
        View_player[y][LOG_W] = '\0';
    }
}
//using char to represent different wall
char shade(q16 dist, int side)
{
    const char pal[] = "@%#*+=-:. ";
    int d = q16_TO_INT(dist);
    if (d < 0) d = 0;
    if (d > 18) d = 18;
    int index = (d * 9) / 18;
    if (side == 1 && index < 9) index++;
    return pal[index];
}
//representing each column from -1 to 1
void init_cameraX_LUT(void)
{
    for (int x = 0; x < LOG_W; x++) {
        q16 a = q16_FROM_INT(2 * x);
        cameraXLUT[x] = q16_div(a, q16_FROM_INT(LOG_W)) - q16_ONE;
    }
}
// Shared grid traversal for rendering and shooting.
RayHit raycast_ray(const Player *p, q16 rayDirX, q16 rayDirY)
{
    //player's position
    int mapX = q16_TO_INT(p->posX);
    int mapY = q16_TO_INT(p->posY);
    //distance needed for each unit
    q16 deltaX = (rayDirX == 0) ? (q16)0x7fffffff : q16_abs(q16_div(q16_ONE, rayDirX));
    q16 deltaY = (rayDirY == 0) ? (q16)0x7fffffff : q16_abs(q16_div(q16_ONE, rayDirY));

    int stepX, stepY;//each step
    q16 sideX, sideY;//distance to next grid

    if (rayDirX < 0) {
        stepX = -1;
        sideX = q16_mul(p->posX - q16_FROM_INT(mapX), deltaX);//distance to next x grid
    } else {
        stepX = +1;
        sideX = q16_mul(q16_FROM_INT(mapX + 1) - p->posX, deltaX);
    }

    if (rayDirY < 0) {
        stepY = -1;
        sideY = q16_mul(p->posY - q16_FROM_INT(mapY), deltaY);//distance to next y grid
    } else {
        stepY = +1;
        sideY = q16_mul(q16_FROM_INT(mapY + 1) - p->posY, deltaY);
    }

    RayHit out;//recording on each ray of column, the nearest wall and target
    out.Dis_Tar = (q16)0x7fffffff;
    out.hitTarget = 0;
    out.side = 0;
    out.Dis_Wall = q16_ONE;
    out.targetX = -1;
    out.targetY = -1;

    while (1)
    {
        int side;
        if (sideX < sideY) {
            sideX += deltaX;
            mapX += stepX;
            side = 0;
        } else {
            sideY += deltaY;
            mapY += stepY;
            side = 1;//side == 1, color is darker
        }
        //calculate vertical distance, avoid fish eye affect
        q16 cellDist = (side == 0) ? (sideX - deltaX) : (sideY - deltaY);

        if (mapX >= 0 && mapX < MAP_W && mapY >= 0 && mapY < MAP_H && targetGrid[mapY][mapX])
        {
            // Shooting hits the first occupied cell, regardless of distance rounding.
            if (out.targetX < 0) {
                out.targetX = mapX;
                out.targetY = mapY;
            }
            if (cellDist < out.Dis_Tar) {
                out.Dis_Tar = cellDist;
                out.hitTarget = 1;
            }
        }

        if (is_wall(mapX, mapY))
        {
            out.side = side;
            out.Dis_Wall = cellDist;
            if (out.Dis_Wall < (q16_ONE / 256))
                out.Dis_Wall = (q16_ONE / 256);
            return out;
        }
    }
}

// Convert a screen column to a direction; all traversal stays in raycast_ray().
RayHit raycast_column(Player *p, int x)
{
    q16 cameraX = cameraXLUT[x];
    q16 rayDirX = p->dirX + q16_mul(p->planeX, cameraX);
    q16 rayDirY = p->dirY + q16_mul(p->planeY, cameraX);
    return raycast_ray(p, rayDirX, rayDirY);
}

void render_frame(Player *p)
{
    ascii_bg();//background
    for (int x = 0; x < LOG_W; x++)
    {
        RayHit h = raycast_column(p, x);
        zBuffer[x] = h.Dis_Wall;
        //record wall's height in each line
        int line_Height = q16_TO_INT(q16_div(q16_FROM_INT(LOG_H), h.Dis_Wall));
        int y0 = (LOG_H / 2) - (line_Height / 2);
        int y1 = (LOG_H / 2) + (line_Height / 2);

        draw_line(x, y0, y1, shade(h.Dis_Wall, h.side));
        //if targert closer, dar iw above the wall
        if (target_visible && h.hitTarget && h.Dis_Tar < zBuffer[x]) {
            int th = q16_TO_INT(q16_div(q16_FROM_INT(LOG_H), h.Dis_Tar));
            int ty0 = (LOG_H / 2) - (th / 4);
            int ty1 = (LOG_H / 2) + (th / 4);
            draw_line(x, ty0, ty1, 'T');
        }
    }
}
