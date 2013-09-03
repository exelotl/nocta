#pragma once
#include "../include/nocta.h"
#include "fixedpoint.h"
#include "assert.h"

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

// keep n within the range min..max
#define CLAMP(n,min,max) ((n)<(min)?(min):((n)>(max)?(max):(n)))


inline static int clip(int x) {
	return CLAMP(x, INT16_MIN, INT16_MAX);
}
