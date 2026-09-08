#ifndef INPUT_H
#define INPUT_H

extern int key_w, key_a, key_s, key_d, key_z;
extern int key_q, key_e, key_f, key_x, key_r, key_g;
extern int break_flag, extended_flag;
extern int fire_handled, exit_handled;

extern int mouse_left, mouse_right, mouse_dx;

void mouse_init(void);
void ps2_poll_update_all(void);

#endif