#pragma once
#include <stdint.h>
#include <math.h>

// 7:25 fixed-point constants and utilities


#define FIX_1 (1<<25)
#define FIX_SQRT2 (int64_t)(sqrt(2) * FIX_1)
#define FIX_PI (int64_t)(3.141592653589793238463 * FIX_1)

inline static int64_t int_to_fix(int64_t x) {
	return x << 25;
}
inline static int64_t fix_to_int(int64_t x) {
	return x >> 25;
}

inline static int64_t fix_mul(int64_t a, int64_t b) {
	return (a * b) >> 25;
}
inline static int64_t fix_div(int64_t a, int64_t b) {
	return (a << 25) / b;
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