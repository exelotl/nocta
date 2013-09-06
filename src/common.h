#pragma once
#include "../include/nocta.h"
#include "fixedpoint.h"
#include "assert.h"

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

// keep n within the range min..max
#define CLAMP(n,min,max) ((n)<(min)?(min):((n)>(max)?(max):(n)))

// allocate memory for a type, and initialise it at the same time.
#define ialloc(t, ...) ialloc_impl(sizeof(t), &(t){ __VA_ARGS__ })

inline static void* ialloc_impl(int size, void* data) {
	void* bytes = malloc(size);
	memcpy(bytes, data, size);
	return bytes;
}

inline static int clip(int x) {
	return CLAMP(x, INT16_MIN, INT16_MAX);
}
