#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "fixed.h"
#include "player.h"
#include "raycast.h"
#include "target.h"
#include "world.h"

// Only software state is linked: these tests never access audio or other MMIO.
Sound fire_sound;
Sound shot_sound;

static Player reset_scene(void)
{
    memset(targetGrid, 0, sizeof targetGrid);
    memset(targets, 0, sizeof targets);
    target_visible = 1;
    target_timer = 37;
    target_num = 0;
    fire_sound = (Sound){.length = 20, .index = 3, .playing = 1};
    shot_sound = (Sound){.length = 30, .index = 7};
    return (Player){
        .posX = q16_FROM_INT(3) + q16_HALF,
        .posY = q16_FROM_INT(3) + q16_HALF,
        .dirX = q16_ONE,
        .planeY = PLANE_LEN_Q16,
        .score = 10
    };
}

static void put_target(int i, int x, int y)
{
    targets[i] = (Target){x, y, 1, 1};
    targetGrid[y][x] = 1;
}

static void assert_hit_state(const Player *p, int i)
{
    assert(p->score == 11 && p->cooldown == 6);
    assert(target_num == 1 && target_visible == 0 && target_timer == 37);
    assert(!targets[i].alive && !targetGrid[targets[i].y][targets[i].x]);
    assert(shot_sound.playing == 1 && shot_sound.index == 0);
    assert(shot_sound.length == 30);
    assert(fire_sound.playing == 1 && fire_sound.index == 3);
}

static void test_directions_and_projection(void)
{
    Player p = reset_scene();
    const q16 dirs[][2] = {
        {q16_ONE, 0}, {-q16_ONE, 0}, {0, q16_ONE}, {0, -q16_ONE}
    };
    const q16 distances[] = {
        q16_FROM_INT(3) + q16_HALF, q16_FROM_INT(2) + q16_HALF,
        q16_ONE + q16_HALF, q16_FROM_INT(2) + q16_HALF
    };
    for (int i = 0; i < 4; ++i) {
        RayHit h = raycast_ray(&p, dirs[i][0], dirs[i][1]);
        assert(h.Dis_Wall == distances[i] && h.side == (i >= 2));
        assert(!h.hitTarget && h.Dis_Tar == (q16)0x7fffffff);
        assert(h.targetX == -1 && h.targetY == -1);
    }

    // Known projection distances also catch ignoring cameraX or camera plane.
    RayHit center = raycast_column(&p, LOG_W / 2);
    RayHit left = raycast_column(&p, 0);
    RayHit right = raycast_column(&p, LOG_W - 1);
    assert(center.Dis_Wall == 229376 && center.side == 0);
    assert(left.Dis_Wall == 248245 && left.side == 1); // Top boundary at y=0.
    assert(right.Dis_Wall == 349734 && right.side == 1); // Interior wall at y=7.

    // Grid-corner ties advance in y first, as in the original implementations.
    put_target(0, 3, 4);
    put_target(1, 4, 3);
    RayHit tie = raycast_ray(&p, q16_ONE, q16_ONE);
    assert(tie.targetX == 3 && tie.targetY == 4 && tie.Dis_Tar == q16_HALF);
    p.dirY = q16_ONE;
    fire_center(&p);
    assert_hit_state(&p, 0);
    assert(targets[1].alive);

    p = reset_scene();
    p.posX = q16_FROM_INT(1);
    RayHit edge = raycast_ray(&p, -q16_ONE, 0);
    assert(edge.Dis_Wall == q16_ONE / 256 && edge.side == 0);
    p.posX = q16_FROM_INT(MAP_W) + q16_HALF;
    edge = raycast_ray(&p, q16_ONE, 0);
    assert(edge.Dis_Wall == q16_HALF && edge.targetX == -1);
}

static void test_first_target_and_read_only_cast(void)
{
    Player p = reset_scene();
    put_target(0, 4, 3);
    put_target(1, 5, 3);
    RayHit h = raycast_ray(&p, p.dirX, p.dirY);
    assert(h.hitTarget && h.Dis_Tar == q16_HALF);
    assert(h.targetX == 4 && h.targetY == 3);
    assert(targets[0].alive && targets[1].alive && targetGrid[3][4]);
    assert(p.score == 10 && p.cooldown == 0 && target_num == 0);
    assert(target_visible == 1 && target_timer == 37 && shot_sound.index == 7);

    fire_center(&p);
    assert_hit_state(&p, 0);
    assert(targets[1].alive && targetGrid[3][5]);

    // Cooldown blocks a second shot, even when visibility is restored.
    target_visible = 1;
    shot_sound.index = 9;
    fire_center(&p);
    assert(p.score == 11 && p.cooldown == 6 && target_num == 1);
    assert(targets[1].alive && target_visible == 1 && shot_sound.index == 9);
}

static void test_occlusion_visibility_and_misses(void)
{
    Player p = reset_scene();
    put_target(0, 9, 3); // Wall at x=7 blocks this target.
    RayHit h = raycast_ray(&p, p.dirX, p.dirY);
    assert(!h.hitTarget && h.targetX == -1);
    fire_center(&p);
    assert(p.score == 10 && p.cooldown == 6 && target_num == 0);
    assert(targets[0].alive && target_visible == 1);
    assert(!shot_sound.playing && shot_sound.index == 7);

    p = reset_scene();
    put_target(0, 4, 3);
    target_visible = 0;
    h = raycast_ray(&p, p.dirX, p.dirY);
    assert(h.hitTarget && h.targetX == 4); // Visibility belongs to the caller.
    render_frame(&p);
    assert(View_player[LOG_H / 2][LOG_W / 2] != 'T');
    fire_center(&p);
    assert(p.score == 10 && p.cooldown == 6 && target_num == 0);
    assert(targets[0].alive && targetGrid[3][4] && target_visible == 0);
    assert(!shot_sound.playing && shot_sound.index == 7 && target_timer == 37);
    target_visible = 1;
    render_frame(&p);
    assert(View_player[LOG_H / 2][LOG_W / 2] == 'T');

    p = reset_scene();
    put_target(0, 7, 3); // Preserve the existing target-before-wall shot rule.
    h = raycast_ray(&p, p.dirX, p.dirY);
    assert(h.targetX == 7 && h.Dis_Tar == h.Dis_Wall);
    render_frame(&p);
    assert(View_player[LOG_H / 2][LOG_W / 2] != 'T');
    fire_center(&p);
    assert_hit_state(&p, 0);

    p = reset_scene();
    put_target(0, 3, 3); // Traversal starts by stepping out of the player's cell.
    fire_center(&p);
    assert(p.score == 10 && p.cooldown == 6 && targets[0].alive);
    assert(!shot_sound.playing && shot_sound.index == 7);
    assert(fire_sound.playing == 1 && fire_sound.index == 3);
}

int main(void)
{
    init_cameraX_LUT();
    test_directions_and_projection();
    test_first_target_and_read_only_cast();
    test_occlusion_visibility_and_misses();
    puts("DDA and shooting regression checks passed.");
    return 0;
}
