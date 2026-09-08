#include "fixed.h"

q16 q16_abs(q16 x) {
    return x < 0 ? -x : x;
}

q16 q16_mul(q16 a, q16 b) {
    return (q16)(((long long)a * (long long)b) >> q16_SHIFT);
}

q16 q16_div(q16 a, q16 b) {
    return (b == 0) ? (q16)0x7fffffff
                    : (q16)(((long long)a << q16_SHIFT) / (long long)b);
}//this whole .c file is just there to implement some operations in q_16 format