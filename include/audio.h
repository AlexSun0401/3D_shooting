#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"

extern Sound fire_sound;
extern Sound shot_sound;

void fire_audio_init(void);
void shot_audio_init(void);
void audio_update(void);

#endif