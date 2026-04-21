#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stdint.h>

/* Fixed-point type */
typedef int fixed_t;

/* Scaling factor */
#define F (1 << 14)  // (17.14 format)

/* Conversion */
fixed_t int_to_fp(int n);
int fp_to_int_trunc(fixed_t x);
int fp_to_int_round(fixed_t x);

/* Arithmetic operations */
fixed_t add_fp(fixed_t x, fixed_t y);
fixed_t sub_fp(fixed_t x, fixed_t y);

fixed_t add_mixed(fixed_t x, int n);
fixed_t sub_mixed(fixed_t x, int n);

fixed_t mul_fp(fixed_t x, fixed_t y);
fixed_t mul_mixed(fixed_t x, int n);

fixed_t div_fp(fixed_t x, fixed_t y);
fixed_t div_mixed(fixed_t x, int n);

#endif