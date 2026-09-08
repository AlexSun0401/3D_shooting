#ifndef FIXED_H
#define FIXED_H

#include "common.h"

#define q16_SHIFT 16
#define q16_ONE ((q16)1 << q16_SHIFT)
#define q16_HALF ((q16)1 << (q16_SHIFT - 1))
#define q16_FROM_INT(i) ((q16)((i) << q16_SHIFT))
#define q16_TO_INT(x) ((int)((x) >> q16_SHIFT))

q16 q16_abs(q16 x);
q16 q16_mul(q16 a, q16 b);
q16 q16_div(q16 a, q16 b);

#endif