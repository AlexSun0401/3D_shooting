#include "target.h"
#include "world.h"

int target_visible = 1;
int target_timer = 0;
int target_set = 4;
int target_num = 0;
int finish_game = 0;

Target targets[MAX_TARGETS];

void initial_targets(void)
{
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            targetGrid[y][x] = 0;

    targets[0] = (Target){2, 8, 1, 1};
    targets[1] = (Target){6, 12, -1, 1};
    targets[2] = (Target){12, 10, 1, 1};
    targets[3] = (Target){13, 5, -1, 1};

    for (int i = 0; i < MAX_TARGETS; i++) {
        targetGrid[targets[i].y][targets[i].x] = 1;//
    }
}     //we intialize each target (each struct)
//by giving it x,y, horizontal speed, alive or not


void move_targets(void)
{
    for (int i = 0; i < MAX_TARGETS; i++)
    {
        if(!targets[i].alive){continue;}
        //if it's being shot ignore it
        
        int x = targets[i].x;
        int y = targets[i].y;
        int dx = targets[i].dx;
        int nx = x + dx; //move in map x direction

        if (nx <= 0 || nx >= MAP_W - 1 || targetGrid[y][nx] != 0)//
        {
            targets[i].dx = -dx;
            nx = x + targets[i].dx;
            if (nx <= 0 || nx >= MAP_W - 1 || targetGrid[y][nx] != 0)//
                continue;
        } //check if it hits the wall, if it accidentally stuck between two walls
        //it just stays there forever (dream in the darkness)

        targetGrid[y][x] = 0;//mark the old posiiton as unoccupied
        targets[i].x = nx;
        targetGrid[y][nx] = 1;//make new position as occupied
    }
}

void update_targets(void)
{
    static int move_timer = 0;
    move_timer++;

    if (move_timer > 20) {
        move_targets();
        move_timer = 0;
    }  //move target every certain time, you can't keep it moving continuously
}