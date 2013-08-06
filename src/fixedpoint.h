#pragma once
#include <stdint.h>
#include <math.h>

// 8:24 fixed-point constants and utilities

#define FIX_P 24
#define FIX_1 (1<<FIX_P)
#define FIX_SQRT2 (int64_t)(sqrt(2) * FIX_1)
#define FIX_PI (int64_t)(3.141592653589793238463 * FIX_1)

inline static int64_t int_to_fix(int64_t x) {
	return x << FIX_P;
}
inline static int64_t fix_to_int(int64_t x) {
	return x >> FIX_P;
}

inline static int64_t fix_mul(int64_t a, int64_t b) {
	return (a * b) >> FIX_P;
}
inline static int64_t fix_div(int64_t a, int64_t b) {
	return (a << FIX_P) / b;
}

// TODO: optimise these (with lookup tables or approximations)

inline static int64_t fix_sin(int64_t x) {
	double y = (double)x / FIX_1;
	return sin(y) * FIX_1;
}
inline static int64_t fix_cos(int64_t x) {
	double y = (double)x / FIX_1;
	return cos(y) * FIX_1;
}
inline static int64_t fix_tan(int64_t x) {
	double y = (double)x / FIX_1;
	return tan(y) * FIX_1;
}