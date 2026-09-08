#ifndef TARGET_H
#define TARGET_H

#include "common.h"

extern int target_visible;
extern int target_timer;
extern int target_set;
extern int target_num;
extern int finish_game;

extern Target targets[MAX_TARGETS];

void initial_targets(void);
void move_targets(void);
void update_targets(void);

#endif