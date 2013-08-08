#pragma once
#include <stdint.h>
#include <math.h>

// 3:13 fixed-point constants and utilities

#define FIX_PT 13
#define FIX_1 (1<<FIX_PT)
#define FIX_SQRT2 (int32_t)(sqrt(2) * FIX_1)
#define FIX_PI (int32_t)(3.141592653589793238463 * FIX_1)

inline static int32_t int_to_fix(int32_t x) {
	return x << FIX_PT;
}
inline static int32_t fix_to_int(int32_t x) {
	return x >> FIX_PT;
}

// convert a uint8 percentage where 0 = 0.0 and 255 = 1.0
inline static int32_t u8_to_fix(int32_t x) {
	return x << (FIX_PT - 8);
}

inline static int32_t fix_mul(int32_t a, int32_t b) {
	return (a * b) >> FIX_PT;
}
inline static int32_t fix_div(int32_t a, int32_t b) {
	return (a << FIX_PT) / b;
}

// TODO: optimise these (with lookup tables or approximations)

inline static int32_t fix_sin(int32_t x) {
	double y = (double)x / FIX_1;
	return sin(y) * FIX_1;
}
inline static int32_t fix_cos(int32_t x) {
	double y = (double)x / FIX_1;
	return cos(y) * FIX_1;
}
inline static int32_t fix_tan(int32_t x) {
	double y = (double)x / FIX_1;
	return tan(y) * FIX_1;
}

inline static int32_t fix_sqrt(int32_t x) {
	double y = (double)x / FIX_1;
	return sqrt(y) * FIX_1;
}