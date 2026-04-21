#include "threads/fixed-point.h"

/* Convert int to fixed-point */
fixed_t int_to_fp(int n) {
    return n * F;
}

/* Convert fixed-point to int (truncate) */
int fp_to_int_trunc(fixed_t x) {
    return x / F;
}

/* Convert fixed-point to int (round) */
int fp_to_int_round(fixed_t x) {
    if (x >= 0)
        return (x + F / 2) / F;
    else
        return (x - F / 2) / F;
}

/* Add two fixed-point numbers */
fixed_t add_fp(fixed_t x, fixed_t y) {
    return x + y;
}

/* Subtract two fixed-point numbers */
fixed_t sub_fp(fixed_t x, fixed_t y) {
    return x - y;
}

/* Add fixed-point and int */
fixed_t add_mixed(fixed_t x, int n) {
    return x + n * F;
}

/* Subtract int from fixed-point */
fixed_t sub_mixed(fixed_t x, int n) {
    return x - n * F;
}

/* Multiply two fixed-point numbers */
fixed_t mul_fp(fixed_t x, fixed_t y) {
    return (fixed_t)(((int64_t) x) * y / F);
}

/* Multiply fixed-point by int */
fixed_t mul_mixed(fixed_t x, int n) {
    return x * n;
}

/* Divide two fixed-point numbers */
fixed_t div_fp(fixed_t x, fixed_t y) {
    return (fixed_t)(((int64_t) x) * F / y);
}

/* Divide fixed-point by int */
fixed_t div_mixed(fixed_t x, int n) {
    return x / n;
}