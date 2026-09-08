#include "audio.h"
#include "assets.h"
#include "hw.h"

Sound fire_sound;
Sound shot_sound;

void fire_audio_init(void) {
    fire_sound.data = soundfire;
    fire_sound.length = soundfire_len;
    fire_sound.index = 0;
    fire_sound.playing = 0;
    fire_sound.loop = 0;
}  //initiate fire sound

void shot_audio_init(void) {
    shot_sound.data = soundshot;
    shot_sound.length = soundshot_len;
    shot_sound.index = 0;
    shot_sound.playing = 0;
    shot_sound.loop = 0;
} //initiate shot sound

void audio_update(void) {
    if (!fire_sound.playing && !shot_sound.playing) return;
     //don't do anything if there's no sound request
    int fifo = *(audio_ptr + 1);
    int space = (fifo >> 16) & 0xFF;
    //you must check fifo space 
    while (space > 0 && fire_sound.playing) {
        int sample = fire_sound.data[fire_sound.index];
        int sampleout = sample << 16;
        *(audio_ptr + 2) = sampleout;
        *(audio_ptr + 3) = sampleout;
        fire_sound.index++;
        if (fire_sound.index >= (int)(fire_sound.length / 8)) {
            fire_sound.playing = 0;
            fire_sound.index = 0; //one-shot event, stop when it reaches the end of array
            break;
        }
        space--;
    }   //output fire sound
    //I shrinked the array, otherwise it's too long
    //you must check fifo space 
    while (space > 0 && shot_sound.playing) {
        int sample = shot_sound.data[shot_sound.index];
        int sampleout = sample << 16;
        *(audio_ptr + 2) = sampleout;
        *(audio_ptr + 3) = sampleout;
        shot_sound.index++;
        if (shot_sound.index >= (int)(shot_sound.length / 16)) {
            shot_sound.playing = 0;
            shot_sound.index = 0; //one-shot event, stop when it reaches the end of array
            break;
        }
        space--;
    }
}  //output fire sound
    //Again I shrinked the array, otherwise it's too long