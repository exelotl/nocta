#pragma once
#include "../include/nocta.h"
#include "fixedpoint.h"
#include "assert.h"

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

// keep n within the range min..max
#define CLAMP(n,min,max) ((n)<(min)?(min):((n)>(max)?(max):(n)))

// prevents a 16-bit sample from clipping
inline static int clip(int x) {
	return CLAMP(x, INT16_MIN, INT16_MAX);
}

// Convert cycles-per-second to cycles-per-sample
inline static int normalize_hz(int f, int sample_rate) {
	return int_to_fix(f) / sample_rate;
}


// allocates memory for a type, and initialises it at the same time
#define ialloc(t, ...) ialloc_impl(sizeof(t), &(t){ __VA_ARGS__ })

// example usage:
//   int* a = ialloc(int, 5);
//   float* b = ialloc(float[4], 1.0, 4.1, 12, 1.3);
//   my_type* c = ialloc(my_type, .foo="hello world", .bar=1337);

// helper function for ialloc
inline static void* ialloc_impl(int size, void* src) {
	void* dest = malloc(size);
	memcpy(dest, src, size);
	return dest;
}
