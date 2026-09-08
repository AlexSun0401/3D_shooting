#include "render.h"
#include "common.h"
#include "fixed.h"
#include "hw.h"
#include "assets.h"
#include "raycast.h"

unsigned short gray_lut[256];//grat loop up tabel: storing different kinds of color gray
unsigned short bg_line[VGA_H];//background color in each line
int wall_y0[LOG_W], wall_y1[LOG_W];//wall's position of the viewr
unsigned short wall_w0[LOG_W], wall_e0[LOG_W];//wall's color and edge's color
int tar_y0[LOG_W], tar_y1[LOG_W];//target position of the viewer

int hud_score = 0, hud_px = 0, hud_py = 0;//socre and position of the player
int scope_on = 0, scope_handled = 0;
int center_x = LOG_W / 2;
int center_y = VGA_H / 2;
int radius_sq = 90 * 90;
int border_sq = 93 * 93;
int player_hp = 100;
int hp_timer = 0;

//----VGA Func----
static inline unsigned short rgb565(int r5, int g6, int b5)//comebining rgb565 format by shifting different bits
{
    return (unsigned short)(((r5 & 31) << 11) | ((g6 & 63) << 5) | (b5 & 31));
}

static inline int clamp(int v, int low, int high)//clamping v at range of [low, high]
{
    return (v < low) ? low : (v > high) ? high : v;
}

static inline unsigned char clamp255(int v)//clamping v at range of [0, 255]
{
    return (unsigned char)((v < 0) ? 0 : (v > 255) ? 255 : v);
}

static inline void vga_wait_swap(void)
{
    while (VGA[3] & 1u) {}
}

static inline void charbuf_putc(int x, int y, char c)//wrting character to the CHAR_BUF
{
    volatile unsigned char *cb = (volatile unsigned char *)CHAR_BUF_BASE;
    cb[((unsigned int)y << 7) + (unsigned int)x] = (unsigned char)c;//character buffer is 128 bytes wide
}

void charbuf_clear(void)//clear char buf
{
    for (int x = 0; x < 80; ++x)
        for (int y = 0; y < 60; ++y)
            charbuf_putc(x, y, ' ');
}

void charbuf_clear_line(int y)//clear a line of charbuf
{
    for (int x = 0; x < 80; ++x)
        charbuf_putc(x, y, ' ');
}

static void uitoa_dec(unsigned v, char *out) // ascii transformer
{
    char tmp[11]; // 32bit + 1
    int n = 0;
    do
    {
        tmp[n++] = (char)('0' + (v % 10));
        v /= 10;
    } while (v && n < 10);

    int i = 0;
    while (n > 0)
        out[i++] = tmp[--n];
    out[i] = '\0';
}
//printying word on char_buf
void charbuf_print(int x, int y, const char *s)
{
    while (*s && x < 80)
    {
        charbuf_putc(x, y, *s++);
        x++;
    }
}
//printing information on char_buf
void hud_draw(void)
{
    charbuf_clear_line(0);
    charbuf_clear_line(1);

    char buf[16];
    charbuf_print(0, 0, "Score:");
    uitoa_dec((unsigned)hud_score, buf);
    charbuf_print(6, 0, buf);

    charbuf_print(14, 0, "Pos:");
    uitoa_dec((unsigned)hud_px, buf);
    charbuf_print(18, 0, buf);
    charbuf_putc(18 + 10, 0, ',');
    uitoa_dec((unsigned)hud_py, buf);
    charbuf_print(20 + 10, 0, buf);

    charbuf_print(0, 1, "W/S move  Mouse/AD turn  L-Click/F fire  R-Click/Z Scope");
}

static void vga_color_table(void)
{
    for (int i = 0; i < 256; ++i)
    {
        gray_lut[i] = rgb565((unsigned short)(i >> 3), (unsigned short)(i >> 2), (unsigned short)(i >> 3));
    }
    const int half = VGA_H / 2;
    const unsigned short sky_top = rgb565(2, 6, 18), sky_horizon = rgb565(8, 14, 24);
    const unsigned short flr_horizon = rgb565(6, 18, 6), flr_bottom = rgb565(2, 10, 2);
//smooth vertical background gradient
    for (int y = 0; y < VGA_H; ++y)
    {
        int t;
        if (y < half)
        {
            t = (half <= 1) ? 0 : (y * 255) / (half - 1);
            int r = (((sky_top >> 11) & 31) * (255 - t) + ((sky_horizon >> 11) & 31) * t) / 255;
            int g = (((sky_top >> 5) & 63) * (255 - t) + ((sky_horizon >> 5) & 63) * t) / 255;
            int b = (((sky_top) & 31) * (255 - t) + ((sky_horizon) & 31) * t) / 255;
            bg_line[y] = rgb565(r, g, b);
        }
        else
        {
            t = (half <= 1) ? 0 : ((y - half) * 255) / (half - 1);
            int r = (((flr_horizon >> 11) & 31) * (255 - t) + ((flr_bottom >> 11) & 31) * t) / 255;
            int g = (((flr_horizon >> 5) & 63) * (255 - t) + ((flr_bottom >> 5) & 63) * t) / 255;
            int b = (((flr_horizon) & 31) * (255 - t) + ((flr_bottom) & 31) * t) / 255;
            bg_line[y] = rgb565(r, g, b);
        }
    }
}

static inline int pal_index(char c)
{
    switch (c)
    {
    case '@': return 0;
    case '%': return 1;
    case '#': return 2;
    case '*': return 3;
    case '+': return 4;
    case '=': return 5;
    case '-': return 6;
    case ':': return 7;
    case '.': return 8;
    case ' ': return 9;
    default: return -1;
    }
}
//determine the side
static inline int side_proving(int dist_q16, char character)
{
    int d = clamp((int)q16_TO_INT(dist_q16), 0, 18);
    int idx0 = (d * 9) / 18;
    if (pal_index(character) == (idx0 + 1) && idx0 < 9)
        return 1;
    return 0;
}
//determine wall's darkness according to distance: far means dark; side ==1 darker
static inline unsigned char wall_brightness(int dist_q16, int side)
{
    if (dist_q16 <= 0)
        dist_q16 = 256;
    unsigned int darkness = (unsigned int)q16_TO_INT(dist_q16 * WALL_FOG_K);
    if (darkness > 255)
        darkness = 255;
    int brightness = 255 - (int)darkness;
    if (side)
        brightness -= SIDE_DARKEN;
    return clamp255(brightness);
}

static void compute_columns(void)
{
    // target scan (from View_player)
    for (int x = 0; x < LOG_W; ++x)
    {
        tar_y0[x] = -1;
        tar_y1[x] = -1;
        int min = 999, max = -1;
        for (int y = 0; y < LOG_H; ++y)
        {
            if (View_player[y][x] == 'T')
            {
                if (y < min) min = y;
                if (y > max) max = y;
            }
        }
        if (max >= 0)
        {
            tar_y0[x] = clamp(min, 0, VGA_H);
            tar_y1[x] = clamp(max, 0, VGA_H);
        }
    }

    // wall columns
    for (int cx = 0; cx < LOG_W; ++cx)
    {
        int dist = (int)zBuffer[cx];
        if (dist < 256)
            dist = 256;
//wall's up and down position
        int height = clamp((int)(q16_FROM_INT(VGA_H) / dist), 1, VGA_H);
        int y0 = (VGA_H - height) / 2;
        wall_y0[cx] = clamp(y0, 0, VGA_H - 1);
        wall_y1[cx] = clamp(y0 + height - 1, 0, VGA_H - 1);

        char midc = View_player[LOG_H / 2][cx];
        int side = side_proving(dist, midc);

        unsigned char left_bri = wall_brightness(dist, side);
//wall's edge has brighter color
        wall_w0[cx] = (gray_lut[left_bri]);
        wall_e0[cx] = (gray_lut[clamp255((int)left_bri + EDGE_BRIGHTEN)]);
    }
}
//draw corsshair at the screen
static inline void draw_crosshair(unsigned int fb_base)
{
    const int cx = VGA_W / 2, cy = VGA_H / 2;
    const unsigned short c = rgb565(31, 63, 31);
    for (int dx = -6; dx <= 6; ++dx)
    {
        if (cx + dx >= 0 && cx + dx < VGA_W)
        {
            volatile unsigned short *row = (volatile unsigned short *)(unsigned long)(fb_base + ((unsigned int)cy << 10));
            row[cx + dx] = c;
        }
    }
    for (int dy = -4; dy <= 4; ++dy)
    {
        if (cy + dy >= 0 && cy + dy < VGA_H)
        {
            volatile unsigned short *row = (volatile unsigned short *)(unsigned long)(fb_base + ((unsigned int)(cy + dy) << 10));
            row[cx] = c;
        }
    }
}
//draw heathbar
static inline void draw_heathbar(unsigned int back)
{
    int bar_width = 50; // pixels
    int bar_height = 10;
    int start_x = 10;
    int start_y = 210;

    // compute health ratio
    int hp_pixels = (player_hp * bar_width) / MAX_HP;

    // colors
    unsigned short red = rgb565(31, 0, 0);
    unsigned short green = rgb565(0, 63, 0);

    for (int y = start_y; y < start_y + bar_height; y++)
    {
        volatile unsigned short *row =
            (volatile unsigned short *)(unsigned long)(back + ((unsigned int)y << 10));

        for (int x = 0; x < bar_width; x++)
        {
            if (x < hp_pixels)
                row[start_x + x] = red; // remaining health
            else
                row[start_x + x] = green; // lost health
        }
    }
}

//----draw gun----
static inline void draw_gun(unsigned int back)
{
    int x_offset = VGA_W - GUN_W;
    int y_offset = VGA_H - GUN_H;
    for (int y = 0; y < GUN_H; y++)
    {
        int pos_y = y + y_offset;
        if (pos_y < 0 || pos_y >= VGA_H) continue;
        
        volatile unsigned short *row = (volatile unsigned short *)(unsigned long)(back + ((unsigned int)pos_y << 10));
        for (int x = 0; x < GUN_W; x++)
        {
            int pos_x = x + x_offset;
            if (pos_x < 0 || pos_x >= VGA_W) continue;
            
            unsigned short color = gun[y][x];
            if (color != TRANSPARENT)
            {
                row[pos_x] = color;
            }
        }
    }
}

//----draw scope----
static inline void draw_scope_crosshair(unsigned int back)
{
    const unsigned short black = rgb565(0, 0, 0);

    volatile unsigned short *row = (volatile unsigned short *)(unsigned long)(back + ((unsigned int)center_y << 10));
    for (int x = 0; x < VGA_W; x++)
    {
        row[x] = black;
    }
    for (int y = 0; y < VGA_H; y++)
    {
        volatile unsigned short *r = (volatile unsigned short *)(unsigned long)(back + ((unsigned int)y << 10));
        r[center_x] = black;
    }
}

void vga_init_once(void)
{
    vga_color_table();//initializing color look up table
    charbuf_clear();//clear charbuf
    hud_draw();//draw basic information on charbuf

    const unsigned int fb_bytes = (unsigned int)(VGA_STRIDE_PIX * VGA_H * 2u);//512 * 240 * 2
    unsigned int fb1 = ((SDRAM_END + 1u) - 0x400000 - fb_bytes) & ~3u;
    unsigned int fb0 = (fb1 - fb_bytes) & ~3u;//... 1111 1000 address align

    VGA[1] = fb0;//f0b0 to back register
    VGA[0] = 1;//ready to swap
    vga_wait_swap();
    VGA[1] = fb1;//fb1 to back register
}

void vga_render_one_frame(void)
{
    compute_columns();
    unsigned int back = VGA[1];//stroing back reg function
    const unsigned short tar16 = (rgb565(31, 0, 0));
    const unsigned short black = rgb565(0, 0, 0);
    const unsigned short border = rgb565(5, 5, 5);

    for (int y = 0; y < VGA_H; ++y)
    {
        unsigned int bg16 = (bg_line[y]);
        volatile unsigned short *row16 = (volatile unsigned short *)(back + ((unsigned int)y << 10));
        int dy = y - center_y;
        int dy_sq = dy * dy;
        for (int cx = 0; cx < LOG_W; ++cx)
        {
            unsigned short color;
            int dx = cx - center_x;
            int dist_sq = dx * dx + dy_sq;
            if (scope_on && dist_sq > radius_sq)//rendering scope
            {
                if (dist_sq <= border_sq)
                {
                    color = border;
                }
                else
                {
                    color = black;
                }
            }
            else//rendering target, wall
            {
                if (tar_y0[cx] >= 0 && y >= tar_y0[cx] && y <= tar_y1[cx])
                {
                    color = tar16;
                }
                else if (y < wall_y0[cx] || y > wall_y1[cx])
                {
                    color = bg16;
                }
                else if (y == wall_y0[cx] || y == wall_y1[cx])
                {
                    color = wall_e0[cx];
                }
                else
                {
                    color = wall_w0[cx];
                }
            }
            row16[cx] = color;
        }
    }
    if (scope_on)
    {
        draw_scope_crosshair(back);
    }
    else
    {
        draw_gun(back);
        draw_crosshair(back);
    }
    draw_heathbar(back);
    VGA[0] = 1;
    vga_wait_swap();
}