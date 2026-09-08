#ifndef COMMON_H
#define COMMON_H

typedef int q16;

// map / logic screen
#define LOG_W 320
#define LOG_H 240
#define MAP_W 16
#define MAP_H 16

// vga
#define VGA_W 320
#define VGA_H 240
#define VGA_STRIDE_PIX 512
#define CELL_H (VGA_H / LOG_H)

#define WALL_FOG_K 12
#define SIDE_DARKEN 18
#define EDGE_BRIGHTEN 20

// player settings
#define PLANE_LEN_Q16 ((q16)43253)
#define ROT_C ((q16)65526)
#define ROT_S ((q16)1144)

// target settings
#define TARGET_VISIBLE_TIME 200
#define TARGET_HIDDEN_TIME 150
#define MAX_TARGETS 4

// hp
#define MAX_HP 100

typedef struct {
    q16 posX, posY;
    q16 dirX, dirY;
    q16 planeX, planeY;
    int score;
    int cooldown;
} Player;

typedef struct {
    q16 Dis_Wall;
    q16 Dis_Tar;
    int hitTarget;
    int side;
} RayHit;

typedef struct {
    int x;
    int y;
    int dx;
    int alive;
} Target;

typedef struct {
    const q16 *data;
    int length;
    int index;
    int playing;
    int loop;
} Sound;

#endif